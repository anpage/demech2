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

  order    each step tries every declaration line at every other position, and the locals of each
           rewrite tools/audit.py still needs declared together in the order the original
           evaluates them (and its rotations), each at the paddings 0..--pads (operand order
           follows a symbol's place in the compiler's blocks of 17, so a move can pay off at
           another padding than the current order's best; past --compiles a step, only at the
           paddings where the current order does best), and keeps the change that leaves the
           fewest rewrites (mirrored comparisons, commutative operands, sums); it stops when only
           stack slots differ and the slots give an allocation order (or the function is
           byte-identical), or when nothing helps. -v prints each order's state at every padding
           (`s` only slots, `=` identical, else the rewrites left)
  buckets  for a function whose locals outnumber the buckets: first the sharing that moves no two
           locals the code orders past each other (the operands of one comparison or commutative
           operation), compiled at every padding, then every way of sharing buckets that the
           allocation order allows, fewest shares first, each giving the order of declaration
           that sharing needs; the first whose renamed variant (tools/witness.py's renaming)
           compiles to the original's bytes is kept

Where an edition's own body under #else declares the locals again, the lines moved are those of the
group the target compiles (found by compiling). `--early DECL N` keeps one global redeclared ahead
of the unit in every compile, as a witness record spells it: for a function tools/witness.py's
--early search leaves at "slots ... (with pad 11 early int~g_localPlayerId 17)", whose locals need
their buckets shared before any renaming works.

Every target that compiles the unit sees the same declarations. `--also TARGET ...` names the other
targets that annotate the function: where one of them compiles it byte-identical, or to only slots
with an allocation order (its best over the paddings, `--also-pads`), an order is written only if
it still does (of the orders that settle the function, the first that costs them nothing is taken), so that tools/witness.py and the gate (tools/certify.py) can keep what they had. After --write, run tools/witness.py --write on the unit
for every target that builds it, with as many pads as its old records used.

usage (from the repository root, with reccmp's environment):
  python tools/declorder.py --target MW2 --unit shots.c FUNCTION ... [--also MW2MATROX MW23DFX]
                            [--file FRAGMENT] [--pads N] [--early DECL N] [--rounds N] [--write] [--wine PATH] [-v]
"""

import argparse
import difflib
import os
import re
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from audit import Lifted, classify, exit_reads, show  # noqa: E402
from certify import pads_text, rename  # noqa: E402
from funccmp import symbol_name  # noqa: E402
from unitbuild import ROOT, Object, Request  # noqa: E402
from witness import Search, bucket_plans, ladder, majority, optimized, slot_plan  # noqa: E402

DECL = re.compile(r"^\t(?!return\b)[A-Za-z_][\w ]*?[\s*]+(\w+)(\[[^\]]*\])*;$")
EXACT, SLOTS = 2, 1
UNEXPLAINED = 1000  # the rewrites charged for code the audit can't classify, plus its distance()


class Target:
    """One target's search over the unit: its compiler, and the function's state in a compile."""

    def __init__(self, args, name, early=()):
        self.early = early  # the global redeclared ahead of the unit in every compile (--early)
        ns = argparse.Namespace(target=name, build=args.build, original=None, wine=args.wine, unit=[args.unit],
                                function=None, pads=0, jobs=args.jobs, write=False, json=None, verbose=False)
        self.name = name
        self.search = Search(ns, name)
        units = [u for u in self.search.units if u.name == args.unit]
        if not units:
            raise SystemExit("%s doesn't build %s" % (name, args.unit))
        self.unit = units[0]
        self.file = args.file or self.unit.name  # the file of the unit's that defines the functions
        self.optimized = bool(optimized(self.unit))
        self.later = sorted(self.search.annotations.addresses)

    def compile(self, texts_and_pads, tag):
        """[(text of the unit's own file, pads)] -> the requests, compiled (discard them after)."""
        requests = []
        for k, (text, pads) in enumerate(texts_and_pads):
            request = Request(self.unit, ROOT, "%s%d" % (tag, k), {self.file: text}, prefix=pads_text(pads, self.early), listing=True)
            request.k = k
            requests.append(request)
        self.search.compiler.run(self.search.target, requests)
        return requests

    def state(self, request, function):
        """(rank, -rewrites) of the function in a compile: EXACT, SLOTS (only slots differ, and they
        give an allocation order; unoptimized units only), or 0 with the rewrites the audit needs
        (UNEXPLAINED plus its distance() when it can't classify). Slots the majority vote reconciles (tools/witness.py) still
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
            if difference is not None:
                return (0, -UNEXPLAINED - distance(lifted))
            return (0, -sum(n for kind, n in kinds.items() if kind != "slots"))
        return (-1, 0)

    def chains(self, text, function, pads):
        """The locals of each operand-order rewrite the audit needs at one padding, in the order the
        original evaluates them: [[names]] (a mirrored comparison's two sides, a commutative chain's
        or a sum's operands), read from the original's slots through the slots most of each local's
        accesses pair it with."""
        requests = self.compile([(text, pads)], "c")
        compared = self.search.compare(requests[0])
        found = []
        for symbol, (comparison, local_list, annotation) in ({} if isinstance(compared, str) else compared).items():
            if symbol_name(symbol) != function or comparison.difference is not None:
                continue
            starts = sorted((offset, name) for name, offset, _ in local_list if offset < 0 and not name.startswith("$"))
            votes = {}
            for ours, theirs in comparison.slots:
                at = [name for offset, name in starts if offset == ours]
                if at:
                    votes.setdefault(at[0], Counter())[theirs] += 1
            theirs_name = {counts.most_common(1)[0][0]: name for name, counts in votes.items()}
            obj = Object(requests[0].obj)
            later = [a for a in self.later if a > annotation.offset]
            lifted = Lifted(obj.functions[symbol], obj, self.search.image, annotation.offset, self.search.annotations,
                            later[0] if later else None)
            lifted.exit_reads = exit_reads(annotation)
            steps = []
            _, difference = classify(lifted, local_list, self.optimized, steps)
            if difference is not None:
                continue
            for kind, _, _, after in steps:
                if kind == "slots":
                    continue
                names = []
                for offset in re.findall(r"\[ebp\+(-\d+)\]", " ; ".join(after)):
                    name = theirs_name.get(int(offset))
                    if name and name not in names:
                        names.append(name)
                if len(names) > 1:
                    found.append(names)
        self.search.compiler.discard(requests)
        return found

    def over_pads(self, text, function, pads):
        """The function's state at each padding 0..pads."""
        requests = self.compile([(text, q) for q in range(pads + 1)], "p")
        states = [self.state(r, function) for r in requests]
        self.search.compiler.discard(requests)
        return states

    def best_over_pads(self, text, function, pads):
        """(the best state over 0..pads, its padding, the state at padding 0: the one tools/audit.py
        classifies where no witness record pads the unit)."""
        states = self.over_pads(text, function, pads)
        best = max(range(len(states)), key=lambda q: (states[q], -q))
        return (states[best], best, states[0])

    def compiled_groups(self, lines, block):
        """The #if groups of the function's declarations that the target compiles: those where an
        #error in place of a declaration stops the compile."""
        groups = [g for g in block.groups if g]
        texts = []
        for g in groups:
            broken = list(lines)
            broken[block.groups[g][0]] = "#error declorder"
            texts.append(("\n".join(broken), 0))
        requests = self.compile(texts, "e")
        found = [g for g, r in zip(groups, requests) if r.error]
        self.search.compiler.discard(requests)
        return found


class Block:
    """A function's declaration lines: the ones that may move (outside #if groups, or the lines of
    the #if group the target compiles where the function has none outside), by name."""

    def __init__(self, lines, function, target):
        self.lines = lines
        # The definition the target's annotation marks (an edition may define the function apart).
        starts = [k for k, line in enumerate(lines) if re.match(r"^[A-Za-z].*[ *]%s\(" % re.escape(function), line)]
        if not starts:
            raise SystemExit("no definition of %s" % function)

        def marked(k):
            while k > 0 and lines[k - 1].startswith("//"):
                k -= 1
                if re.match(r"// (FUNCTION|STUB): %s 0x" % re.escape(target), lines[k]):
                    return True
            return False

        start = next((k for k in starts if marked(k)), starts[0])
        while lines[start].strip() != "{":
            start += 1
        self.first = start  # the line of `{`
        end = next(k for k in range(start, len(lines)) if lines[k] == "}")
        # The declaration lines by #if group: those right after `{`, and those after an #else or
        # #elif of the function's own (an edition's whole body). Group 0 is outside any group.
        self.groups, count = {}, 0
        for k in [start] + [k for k in range(start + 1, end) if re.match(r"#\s*(else|elif)", lines[k])]:
            stack, k = [0 if k == start else -1], k + 1
            if stack[0]:
                count += 1
                stack = [count]
            while k < end and (DECL.match(lines[k]) or lines[k].startswith("#")):
                if re.match(r"#\s*if", lines[k]):
                    count += 1
                    stack.append(count)
                elif re.match(r"#\s*endif", lines[k]):
                    if len(stack) == 1:
                        break
                    stack.pop()
                elif re.match(r"#\s*(else|elif)", lines[k]):
                    count += 1
                    stack[-1] = count
                elif DECL.match(lines[k]):
                    self.groups.setdefault(stack[-1], []).append(k)
                k += 1
        self.choose(0)

    def choose(self, group):
        """Move the lines of one group; the others stay."""
        self.movable = self.groups.get(group, [])
        self.fixed = {}
        for other, ks in self.groups.items():
            if other != group:
                for k in ks:
                    self.fixed[DECL.match(self.lines[k]).group(1)] = k

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
    """Every order with one line moved, once each."""
    out, seen = [], {tuple(order)}
    for k in range(len(order)):
        rest = order[:k] + order[k + 1 :]
        for t in range(len(order)):
            moved = rest[:t] + [order[k]] + rest[t:]
            if tuple(moved) not in seen:
                seen.add(tuple(moved))
                out.append(moved)
    return out


def chain_moves(order, chains):
    """The orders that declare a rewrite's locals together, last evaluated first (the compilers
    evaluate the latest declared of one block of symbols first), and each rotation of that (a block
    boundary among them): what several single moves reach only through orders no better than the
    current one."""
    out = []
    for names in chains:
        lines = {name_of(line): line for line in order}
        if any(name not in lines for name in names):
            continue
        rest = [line for line in order if name_of(line) not in names]
        at = min(order.index(lines[name]) for name in names)
        at = sum(1 for line in order[:at] if name_of(line) not in names)
        wanted = [lines[name] for name in reversed(names)]
        for turn in range(len(wanted)):
            moved = rest[:at] + wanted[turn:] + wanted[:turn] + rest[at:]
            if moved != order and moved not in out:
                out.append(moved)
    return out


def pad_map(states):
    """The states over the paddings on one line: `=` exact, `s` only slots, else the rewrites
    (`?` unexplained)."""
    return " ".join("%d:%s" % (q, "=" if st[0] == EXACT else "s" if st[0] == SLOTS else "?" if st[1] <= -UNEXPLAINED else str(-st[1])) for q, st in enumerate(states))


def search_order(target, lines, block, function, args, accept=None):
    """The order stage: (order, pads, state). A move can pay off at another padding than the
    current order's best (the padding places the block boundary that orders two symbols), so each
    move is compiled at every padding while --compiles allows, and at the paddings where the
    current order does best otherwise. Of the orders that leave only slots, the first that
    `accept` takes (the --also targets keep what they had) is kept, if any is."""
    order = block.order(lines)
    states = target.over_pads(block.text(lines, order), function, args.pads)
    pads = max(range(len(states)), key=lambda q: (states[q], -q))
    state = states[pads]
    if args.verbose:
        print("  %s: %s" % (function, pad_map(states)))
    for step in range(args.rounds):
        if state[0] >= SLOTS:
            break
        candidates = moves(order)
        candidates += [m for m in chain_moves(order, target.chains(block.text(lines, order), function, pads)) if m not in candidates]
        if not candidates:
            break
        count = max(1, min(len(states), args.compiles // len(candidates)))
        paddings = sorted(range(len(states)), key=lambda q: (states[q], -q), reverse=True)[:count]
        jobs = [(k, q) for k in range(len(candidates)) for q in paddings]
        requests = target.compile([(block.text(lines, candidates[k]), q) for k, q in jobs], "m")
        scored = [(target.state(r, function), -jobs[r.k][1], -jobs[r.k][0]) for r in requests]
        target.search.compiler.discard(requests)
        best_state, _, k = max(scored)
        if accept and best_state[0] >= SLOTS:
            ties, tried = sorted((x for x in scored if x[0] == best_state), reverse=True), []
            for _, _, other in ties:
                if other not in tried and len(tried) < args.also_tries:
                    tried.append(other)
                    if accept(candidates[-other]):
                        k = other
                        break
        if best_state <= state:
            if args.verbose:
                print("  %s: no move helps (%s)" % (function, state))
            break
        order = candidates[-k]
        states = target.over_pads(block.text(lines, order), function, args.pads)
        pads = max(range(len(states)), key=lambda q: (states[q], -q))
        state = states[pads]
        if args.verbose:
            print("  %s step %d: %s" % (function, step + 1, pad_map(states)))
    return order, pads, state


def ordered_pairs(listing, function, names):
    """The pairs of locals whose relative order the code shows: operands of one comparison or one
    commutative operation (a load of one, then an operation with the other), from the listing."""
    m = re.search(r"^\S*\b%s\b\S*\s+PROC\b" % re.escape(function), listing, re.M)
    if not m:
        return set()
    end = listing.find("ENDP", m.end())
    instructions = [line for line in listing[m.end() : end if end > 0 else len(listing)].split("\n") if line.startswith("\t")]
    local = re.compile(r"_(\w+)\$\[ebp")
    pairs = set()
    for first, second in zip(instructions, instructions[1:]):
        x, y = local.findall(first), local.findall(second)
        if x and y and x[0] != y[0] and x[0] in names and y[0] in names:
            if second.split()[0] not in ("mov", "movsx", "movzx", "lea", "push", "fld", "fild", "fst", "fstp", "fistp"):
                pairs.add(frozenset((x[0], y[0])))
    return pairs


def careful_plan(alloc, buckets, current, pairs):
    """One way to put the locals into the buckets, [(bucket index, [locals])], that moves no two
    locals of `pairs` past each other if it can (a bucket's locals are declared in its direction,
    which a pair already in that order allows), the buckets filled evenly; None if they don't fit."""
    place = {name: k for k, name in enumerate(current)}

    def cost(b, i, j):
        direction = buckets[b][2]
        return sum(1 for x in range(i, j) for y in range(x + 1, j)
                   if frozenset((alloc[x], alloc[y])) in pairs and direction * (place[alloc[y]] - place[alloc[x]]) < 0)

    best = {(0, 0): (0, 0, None)}  # (locals placed, buckets used) -> (pairs moved, sum of squares, locals placed before)
    for b in range(len(buckets)):
        for j in range(1, len(alloc) + 1):
            for i in range(max(0, j - min(8, len(buckets[b][1]))), j):
                if (i, b) in best:
                    option = (best[(i, b)][0] + cost(b, i, j), best[(i, b)][1] + (j - i) ** 2, i)
                    if (j, b + 1) not in best or option < best[(j, b + 1)]:
                        best[(j, b + 1)] = option
    ends = [b for b in range(1, len(buckets) + 1) if (len(alloc), b) in best]
    if not ends:
        return None
    b = min(ends, key=lambda b: best[(len(alloc), b)][:2])
    plan, j = [], len(alloc)
    while j:
        i = best[(j, b)][2]
        plan.append((b - 1, alloc[i:j]))
        j, b = i, b - 1
    return plan[::-1]


def search_buckets(target, lines, block, function, pads, args):
    """The buckets stage: [(declaration order, padding)] whose renamed variants are exact, best
    first. The first sharing tried keeps the locals the code orders (ordered_pairs) as they are,
    and is compiled at every padding (moving declarations moves the locals against the globals);
    the others are every sharing the allocation order allows, at the order stage's padding."""
    requests = target.compile([(block.text(lines, block.order(lines)), pads)], "b")
    compared = target.search.compare(requests[0])
    listing = Path(requests[0].asm).read_text(encoding="utf-8", errors="replace") if not isinstance(compared, str) else ""
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
    careful = careful_plan(alloc, buckets, current, ordered_pairs(listing, function, set(current)))
    for plan in ([careful] if careful else []) + bucket_plans(alloc, buckets):
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
        candidates.append((0 if plan is careful else 1, len(before), moved, names, [by_name[n] for n in new]))
    candidates.sort(key=lambda c: c[:3])
    candidates = candidates[: args.bucket_candidates]
    jobs = []
    for k, (rest, _, _, names, new_order) in enumerate(candidates):
        text = rename(block.text(lines, new_order), annotation.line_number, annotation.end_line, names)
        jobs += [(k, q, text) for q in ([pads] if rest else [pads] + [q for q in range(args.pads + 1) if q != pads])]
    requests = target.compile([(text, q) for _, q, text in jobs], "r")
    found, seen = [], set()
    for r in sorted(requests, key=lambda r: r.k):
        k, q, _ = jobs[r.k]
        if k not in seen and target.state(r, function)[0] == EXACT:
            seen.add(k)
            found.append((candidates[k][4], q))
    target.search.compiler.discard(requests)
    return found


def distance(lifted):
    """How far an unexplained function is from the original, below UNEXPLAINED: the instructions
    outside the longest matching runs, once each of our stack slots is renamed to the original's
    slot most of the aligned instructions pair it with (so that a mirrored comparison between two
    locals counts, where masking the slots would hide it). The order stage climbs this until the
    audit can classify what is left."""
    slot = re.compile(r"\[ebp\+(-?\d+)\]")
    ours, theirs = [show(i) for i in lifted.ours], [show(i) for i in lifted.theirs]
    votes = {}
    for a, b in zip(ours, theirs):
        x, y = slot.findall(a), slot.findall(b)
        if len(x) == len(y) and slot.sub("", a) == slot.sub("", b):
            for u, v in zip(x, y):
                votes.setdefault(u, Counter())[v] += 1
    mapping = {u: c.most_common(1)[0][0] for u, c in votes.items()}
    ours = [slot.sub(lambda m: "[ebp+%s]" % mapping.get(m.group(1), "?" + m.group(1)), a) for a in ours]
    matched = sum(b.size for b in difflib.SequenceMatcher(None, ours, theirs, autojunk=False).get_matching_blocks())
    return min(UNEXPLAINED - 1, len(ours) + len(theirs) - 2 * matched)


def worse_on(before, after):
    """Whether another target loses what it had (each a best_over_pads result): byte-identical or
    only slots at some padding (what a witness needs), or a classification at padding 0 (what
    tools/audit.py explains)."""
    if before[0][0] >= SLOTS:
        return after[0][0] < before[0][0]
    return before[2][1] > -UNEXPLAINED and after[0][0] < SLOTS and after[2][0] < SLOTS and after[2][1] <= -UNEXPLAINED


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", required=True)
    parser.add_argument("--unit", required=True)
    parser.add_argument("--file", help="The file the unit includes that defines the functions (a fragment: shapebounds.c)")
    parser.add_argument("functions", nargs="+")
    parser.add_argument("--also", nargs="*", default=[], help="Other targets that annotate the functions: none may get worse")
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"))
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--pads", type=int, default=16, help="Paddings tried for each order (0..N; operand order repeats every 17)")
    parser.add_argument("--also-pads", type=int, default=16, help="Paddings tried on the --also targets (0..N)")
    parser.add_argument("--early", nargs=2, metavar=("DECL", "N"), help="A global redeclared ahead of the unit after the paddings, and N typedefs after it, as a witness record spells them (int~g_playerCount 4): tools/witness.py's --early search names it for a function it leaves with only slots")
    parser.add_argument("--also-tries", type=int, default=12, help="Orders that leave only slots checked against the --also targets, until one costs them nothing")
    parser.add_argument("--compiles", type=int, default=3000, help="Compiles of one step of the order stage at most: the paddings each move is tried at")
    parser.add_argument("--rounds", type=int, default=12, help="Steps of the order stage")
    parser.add_argument("--bucket-candidates", type=int, default=400, help="Bucket sharings compiled at most")
    parser.add_argument("--write", action="store_true", help="Write the orders found into the source")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    main_target = Target(args, args.target, ((args.early[0], int(args.early[1])),) if args.early else ())
    others = [Target(args, name) for name in args.also]
    path = (ROOT / main_target.unit.source).parent / main_target.file
    lines = path.read_text(encoding="utf-8").split("\n")
    changed = False
    for function in args.functions:
        block = Block(lines, function, args.target)
        if len(block.movable) < 2 and len(block.groups) > 1:
            compiled = main_target.compiled_groups(lines, block)
            if compiled:
                block.choose(max(compiled, key=lambda g: len(block.groups[g])))
        old_text = "\n".join(lines)
        before = {t.name: t.best_over_pads(old_text, function, args.also_pads) for t in others}
        start = main_target.best_over_pads(old_text, function, args.pads)[:2]
        accept = lambda new: not any(worse_on(before[t.name], t.best_over_pads(block.text(lines, new), function, args.also_pads)) for t in others)
        order, pads, state = search_order(main_target, lines, block, function, args, accept if others else None)
        how = {EXACT: "exact", SLOTS: "only slots"}.get(state[0], "%d rewrites" % -state[1])
        orders = [(order, how, pads)]
        if state[0] == SLOTS:
            shared = search_buckets(main_target, block.text(lines, order).split("\n"), block, function, pads, args)
            orders = [(o, "exact once renamed (buckets shared)", q) for o, q in shared] + orders
        if orders[-1][0] == block.order(lines) and len(orders) == 1 or (len(orders) == 1 and state <= start[0]):
            print("%-28s no better order (%s)" % (function, how))
            continue
        for order, how, pads in orders:
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
