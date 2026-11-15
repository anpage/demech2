#!/usr/bin/env python3
"""Search the declaration order of a function's locals that the original's code implies.

At /Od the compilers decide which side of a comparison goes in a register, and the operand order
of some commutative operations, from the order of the symbols: reordering two locals' declarations
flips a comparison between them (CLAUDE.md, "Codegen Patterns"), and padding the unit (the symbols
declared ahead of it) flips one between a local and a global. Within one bucket of the slot hash,
the compilers also allocate the locals in their order of declaration. The original's declaration
order is lost with its source, so a function whose comparisons come out mirrored, or whose locals
outnumber the hash's buckets, can be right in every token and still not reach a witness
(tools/witness.py), which renames locals and pads the unit but never moves a declaration.

This tool is a finder that edits the source: it moves the declaration lines of each function it is
given (never a line inside an #if group, which stays where it is) and keeps an order that the
original's code supports, in two stages:

  order    each step tries every declaration line at every other position, and every padding for
           the current order (each compiled as the build does), and keeps the change that leaves
           the fewest rewrites in tools/audit.py's classification (mirrored comparisons,
           commutative operands, sums); it stops when only stack slots differ and the slots give an
           allocation order (or the function is byte-identical), or when no move helps
  buckets  for a function whose locals outnumber the buckets: every way of sharing buckets that
           the allocation order allows, fewest shares first, each giving the order of declaration
           that sharing needs; the first whose renamed variant (tools/witness.py's renaming)
           compiles to the original's bytes is kept

Every target that compiles the unit sees the same declarations. `--also TARGET ...` names the other
targets that annotate the function: where one of them compiles it byte-identical, or to only slots
with an allocation order (its best over the paddings, `--also-pads`), an order is written only if
it still does, so that tools/witness.py and the gate (tools/certify.py) can keep what they had. After --write, run tools/witness.py --write on the unit
for every target that builds it, with as many pads as its old records used.

usage (from the repository root, with reccmp's environment):
  python tools/declorder.py --target MW2 --unit shots.c FUNCTION ... [--also MW2MATROX MW23DFX]
                            [--pads N] [--rounds N] [--write] [--wine PATH] [-v]
"""

import argparse
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from audit import Lifted, classify, exit_reads  # noqa: E402
from certify import pads_text, rename  # noqa: E402
from funccmp import symbol_name  # noqa: E402
from unitbuild import ROOT, Object, Request  # noqa: E402
from witness import Search, ladder, majority, optimized, slot_plan  # noqa: E402

DECL = re.compile(r"^\t(?!return\b)[A-Za-z_][\w ]*?[\s*]+(\w+)(\[[^\]]*\])*;$")
EXACT, SLOTS = 2, 1


class Target:
    """One target's search over the unit: its compiler, and the function's state in a compile."""

    def __init__(self, args, name):
        ns = argparse.Namespace(target=name, build=args.build, original=None, wine=args.wine, unit=[args.unit],
                                function=None, pads=0, jobs=args.jobs, write=False, json=None, verbose=False)
        self.name = name
        self.search = Search(ns, name)
        units = [u for u in self.search.units if u.name == args.unit]
        if not units:
            raise SystemExit("%s doesn't build %s" % (name, args.unit))
        self.unit = units[0]
        self.optimized = bool(optimized(self.unit))
        self.later = sorted(self.search.annotations.addresses)

    def compile(self, texts_and_pads, tag):
        """[(text of the unit's own file, pads)] -> the requests, compiled (discard them after)."""
        requests = []
        for k, (text, pads) in enumerate(texts_and_pads):
            request = Request(self.unit, ROOT, "%s%d" % (tag, k), {self.unit.name: text}, prefix=pads_text(pads), listing=True)
            request.k = k
            requests.append(request)
        self.search.compiler.run(self.search.target, requests)
        return requests

    def state(self, request, function):
        """(rank, -rewrites) of the function in a compile: EXACT, SLOTS (only slots differ, and they
        give an allocation order; unoptimized units only), or 0 with the rewrites the audit needs
        (1000 when it can't classify). Slots the majority vote reconciles (tools/witness.py) still
        count their rewrites here: the vote can't undo a swapped operand."""
        compared = self.search.compare(request)
        if isinstance(compared, str):
            return (-1, 0)
        for symbol, (comparison, local_list, annotation) in compared.items():
            if symbol_name(symbol) != function:
                continue
            if comparison.exact:
                return (EXACT, 0)
            if comparison.slots_only and not self.optimized:
                if slot_plan(local_list, comparison.slots)[0]:
                    return (SLOTS, 0)
            obj = Object(request.obj)
            later = [a for a in self.later if a > annotation.offset]
            lifted = Lifted(obj.functions[symbol], obj, self.search.image, annotation.offset, self.search.annotations,
                            later[0] if later else None)
            lifted.exit_reads = exit_reads(annotation)
            kinds, difference = classify(lifted, local_list, self.optimized)
            return (0, -1000 if difference is not None else -sum(n for kind, n in kinds.items() if kind != "slots"))
        return (-1, 0)

    def best_over_pads(self, text, function, pads):
        """(the best state over 0..pads, its padding, the state at padding 0: the one tools/audit.py
        classifies where no witness record pads the unit)."""
        requests = self.compile([(text, q) for q in range(pads + 1)], "p")
        scored = [(self.state(r, function), r.k) for r in requests]
        best = max(scored, key=lambda x: (x[0], -x[1]))
        self.search.compiler.discard(requests)
        return best + (next(st for st, k in scored if k == 0),)


class Block:
    """A function's declaration lines: the ones that may move (outside #if groups), by name."""

    def __init__(self, lines, function):
        start = next((k for k, line in enumerate(lines) if re.match(r"^[A-Za-z].*[ *]%s\(" % re.escape(function), line)), None)
        if start is None:
            raise SystemExit("no definition of %s" % function)
        while lines[start].strip() != "{":
            start += 1
        self.first = start  # the line of `{`
        self.movable, self.fixed, depth, k = [], {}, 0, start + 1
        while DECL.match(lines[k]) or lines[k].startswith("#"):
            if re.match(r"#\s*if", lines[k]):
                depth += 1
            elif re.match(r"#\s*endif", lines[k]):
                depth -= 1
            elif DECL.match(lines[k]):
                if depth:
                    self.fixed[DECL.match(lines[k]).group(1)] = k
                else:
                    self.movable.append(k)
            k += 1

    def order(self, lines):
        return [lines[k] for k in self.movable]

    def text(self, lines, order):
        new = list(lines)
        for k, line in zip(self.movable, order):
            new[k] = line
        return "\n".join(new)


def name_of(line):
    return DECL.match(line).group(1)


def moves(order):
    out = []
    for k in range(len(order)):
        rest = order[:k] + order[k + 1 :]
        for t in range(len(order)):
            if t != k:
                out.append(rest[:t] + [order[k]] + rest[t:])
    return out


def search_order(target, lines, block, function, args):
    """The order stage: (order, pads, state)."""
    order = block.order(lines)
    state, pads, _ = target.best_over_pads(block.text(lines, order), function, args.pads)
    for step in range(args.rounds):
        if state[0] >= SLOTS:
            break
        candidates = moves(order)
        requests = target.compile([(block.text(lines, m), pads) for m in candidates], "m")
        scored = [(target.state(r, function), r.k) for r in requests]
        target.search.compiler.discard(requests)
        best_state, k = max(scored)
        if best_state <= state:
            if args.verbose:
                print("  %s: no move helps (%s)" % (function, state))
            break
        order = candidates[k]
        state, pads, _ = target.best_over_pads(block.text(lines, order), function, args.pads)
        if args.verbose:
            print("  %s step %d: %s at pad %d" % (function, step + 1, state, pads))
    return order, pads, state


def bucket_plans(order, buckets, limit=20000):
    """Ways to put the locals, in their allocation order, into the buckets in order (a bucket
    holding up to its names' count), fewest locals per bucket first: [[(bucket index, [locals])]]."""
    out = []

    def go(pos, b, acc):
        if len(out) >= limit:
            return
        if pos == len(order):
            out.append(list(acc))
            return
        if b >= len(buckets) or len(order) - pos > sum(len(e[1]) for e in buckets[b:]):
            return
        for take in range(1, min(len(buckets[b][1]), len(order) - pos) + 1):
            acc.append((b, order[pos : pos + take]))
            go(pos + take, b + 1, acc)
            acc.pop()
        go(pos, b + 1, acc)

    go(0, 0, [])
    return out


def search_buckets(target, lines, block, function, pads, args):
    """The buckets stage: the declaration orders whose renamed variants are exact, best first."""
    requests = target.compile([(block.text(lines, block.order(lines)), pads)], "b")
    compared = target.search.compare(requests[0])
    target.search.compiler.discard(requests)
    if isinstance(compared, str):
        return []
    symbol, (comparison, local_list, annotation) = next((s, e) for s, e in compared.items() if symbol_name(s) == function)
    alloc, reason = slot_plan(local_list, comparison.slots)
    if alloc is None:
        alloc, reason = slot_plan(local_list, majority(local_list, comparison.slots))
    if alloc is None:
        return []
    while alloc and (alloc[-1].startswith("$T") or alloc[-1] == "this"):
        alloc = alloc[:-1]
    if any("$" in n for n in alloc):
        return []
    buckets = [e for e in ladder(target.search.compiler, target.search.target, target.unit) if e[0] == "bucket"]
    if len(alloc) <= len(buckets):
        return []
    current = [name_of(line) for line in block.order(lines)]
    by_name = {name_of(line): line for line in block.order(lines)}
    if any(n not in by_name and n not in block.fixed for n in alloc):
        return []
    candidates = []
    for plan in bucket_plans(alloc, buckets):
        names, before = {}, []
        for b, group in plan:
            entry = buckets[b]
            for k, local in enumerate(group):
                names[local] = entry[1][k]
            for x, y in zip(group, group[1:]):
                before.append((x, y) if entry[2] > 0 else (y, x))
        # The movable lines in an order meeting `before`, closest to the current one.
        after = {n: set() for n in current}
        for x, y in before:
            if x in after and y in after:
                after[y].add(x)
        new, done = [], set()
        while len(new) < len(current):
            ready = [n for n in current if n not in done and after[n] <= done]
            if not ready:
                break
            new.append(ready[0])
            done.add(ready[0])
        if len(new) < len(current):
            continue
        place = dict(block.fixed)
        place.update({n: k for k, n in zip(block.movable, new)})
        if any(place[x] > place[y] for x, y in before):
            continue
        moved = sum(1 for x, y in zip(new, current) if x != y)
        candidates.append((len(before), moved, names, [by_name[n] for n in new]))
    candidates.sort(key=lambda c: c[:2])
    candidates = candidates[: args.bucket_candidates]
    texts = []
    for _, _, names, new_order in candidates:
        text = block.text(lines, new_order)
        texts.append((rename(text, annotation.line_number, annotation.end_line, names), pads))
    requests = target.compile(texts, "r")
    found = [candidates[r.k][3] for r in sorted(requests, key=lambda r: r.k) if target.state(r, function)[0] == EXACT]
    target.search.compiler.discard(requests)
    return found


def worse_on(before, after):
    """Whether another target loses what it had (each a best_over_pads result): byte-identical or
    only slots at some padding (what a witness needs), or a classification at padding 0 (what
    tools/audit.py explains)."""
    if before[0][0] >= SLOTS:
        return after[0][0] < before[0][0]
    return before[2][1] > -1000 and after[0][0] < SLOTS and after[2][0] < SLOTS and after[2][1] == -1000


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", required=True)
    parser.add_argument("--unit", required=True)
    parser.add_argument("functions", nargs="+")
    parser.add_argument("--also", nargs="*", default=[], help="Other targets that annotate the functions: none may get worse")
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"))
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--pads", type=int, default=24, help="Paddings tried for each order (0..N)")
    parser.add_argument("--also-pads", type=int, default=64, help="Paddings tried on the --also targets (0..N)")
    parser.add_argument("--rounds", type=int, default=12, help="Steps of the order stage")
    parser.add_argument("--bucket-candidates", type=int, default=400, help="Bucket sharings compiled at most")
    parser.add_argument("--write", action="store_true", help="Write the orders found into the source")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    main_target = Target(args, args.target)
    others = [Target(args, name) for name in args.also]
    path = ROOT / main_target.unit.source
    lines = path.read_text(encoding="utf-8").split("\n")
    changed = False
    for function in args.functions:
        block = Block(lines, function)
        old_text = "\n".join(lines)
        before = {t.name: t.best_over_pads(old_text, function, args.also_pads) for t in others}
        start = main_target.best_over_pads(old_text, function, args.pads)[:2]
        order, pads, state = search_order(main_target, lines, block, function, args)
        how = {EXACT: "exact", SLOTS: "only slots"}.get(state[0], "%d rewrites" % -state[1])
        orders = [(order, how)]
        if state[0] == SLOTS:
            shared = search_buckets(main_target, block.text(lines, order).split("\n"), block, function, pads, args)
            orders = [(o, "exact once renamed (buckets shared)") for o in shared] + orders
        if orders[-1][0] == block.order(lines) and len(orders) == 1 or (len(orders) == 1 and state <= start[0]):
            print("%-28s no better order (%s)" % (function, how))
            continue
        for order, how in orders:
            text = block.text(lines, order)
            worse = []
            for t in others:
                after = t.best_over_pads(text, function, args.also_pads)
                if worse_on(before[t.name], after):
                    worse.append("%s %s -> %s" % (t.name, before[t.name][0], after[0]))
                    break
            names = " ".join(name_of(line) for line in order)
            if worse:
                print("%-28s %s at pad %d, but worse on %s: %s" % (function, how, pads, ", ".join(worse), names))
                continue
            print("%-28s %s at pad %d: %s" % (function, how, pads, names))
            lines = text.split("\n")
            changed = True
            break
    if args.write and changed:
        path.write_text("\n".join(lines), encoding="utf-8")
        print("wrote %s" % path.relative_to(ROOT))
    return 0


if __name__ == "__main__":
    sys.exit(main())
