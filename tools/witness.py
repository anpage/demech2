#!/usr/bin/env python3
"""Search for witnesses that a function differs from the original only by compiler entropy.

At /Od the compilers give every named local a stack slot in the order of a hash of its name (and
within one bucket of the hash, mostly in the order of declaration), then the temporaries ($T123) in
the order they're created, outside the hash; and they number the duplicates of a nested
scope (`i$99`) with a counter that every symbol declared ahead of the function advances. Neither is
recoverable from the original: its locals' names are lost. So a function can be right and still
differ from the original in its `[ebp-N]` operands. Optimized code (/Ox: the 3Dfx edition, the
Matrox edition's renderer and depth sort) moves with the symbols declared ahead of it too, in its
registers and its order.

This tool looks for a witness: a variant of the unit that changes only the locals' names or the
number of symbols declared ahead of it (`--pads N`: up to N typedefs ahead of the unit), and
compiles the function to the original's bytes. A witness proves that the function's tokens are
right. The source keeps its readable names; the witness is a record in
`tools/witnesses/<TARGET>.txt`, which `tools/certify.py` checks. This tool is the finder: nothing
rests on it, and it may be as clever as it needs to be.

For each unit it compiles the source as the build does (tools/unitbuild.py), compares each
annotated function with the original (tools/funccmp.py), and reports one of:

  exact      the bytes are the original's (relocated fields compared by their targets)
  witness    exact once the locals are renamed and/or with N symbols declared ahead
  slots      the only differences are stack slots, but no renaming tried reproduces them
  other      the first difference isn't a stack slot: it is printed

The renaming (unoptimized units): where the only differences are `[ebp-N]` operands, the order in
which the original allocated the named locals follows from them (each local must map to one place,
without overlap). The locals are renamed, in a scratch copy, to names the compiler allocates in
that order: a probe compiled twice (its locals declared forward, then reversed) sorts a pool of
names into the hash's buckets and finds the order of the buckets, and within a bucket whether the
first declared comes first, without knowing the hash. A C++ unit's probe is a member function,
which places `this` (a name that can't change) among the buckets. Optimized units are searched by
padding alone.

usage (from the repository root, with reccmp's environment):
  python tools/witness.py --target MW2 [--build build-wine] [--wine PATH] [--unit ai.c ...]
                          [--function NAME ...] [--pads N] [--jobs N] [--write] [--json FILE] [-v]

--write records the witnesses in tools/witnesses/<TARGET>.txt: the records of the units and
functions searched are replaced, the others kept.
"""

import argparse
import json
import os
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from certify import TOKEN, pads_text, read_records, recipe, rename, unit_annotations  # noqa: E402
from funccmp import Annotations, Comparison, read_yaml_paths, source_roots, symbol_name  # noqa: E402
from unitbuild import ROOT, TARGETS, Compiler, Image, Object, Request, Target, Unit  # noqa: E402

FIXED = ("this",)  # locals the compiler names itself: they can't be renamed


def optimized(unit):
    """Whether a unit is compiled with the optimizer (`/Ox`, `/O1`, `/O2`; `/Ob1` alone isn't)."""
    return bool(re.search(r"(?:^|\s)[/-]O[x12]", unit.arguments))


def read_listing(path):
    """{function symbol: [(local name, offset, renameable)]} from a /Fa listing."""
    result, pending = {}, []
    for line in Path(path).read_text(errors="replace").split("\n"):
        m = re.match(r"^__\$\w+\$ = (-?\d+)\s*$", line)
        if m:  # the C++ EH record: a fixed area at the top of the frame
            continue
        m = re.match(r"^_(\S+)\$ = (-?\d+)\s*$", line)
        if m:
            name = m.group(1)
            pending.append((name, int(m.group(2)), "$" not in name and name not in FIXED))
            continue
        m = re.match(r"^_?(\S+) = (-?\d+)\s*$", line)
        if m:  # a nested scope's duplicate (_i$99) or a named temporary ($T123)
            pending.append((m.group(1), int(m.group(2)), False))
            continue
        m = re.match(r"^(\S+)\s+PROC NEAR", line)
        if m:
            result[m.group(1)] = pending
            pending = []
    return result


# ---------------------------------------------------------------------------------------------
# Renaming


def slot_plan(local_list, slots):
    """The named locals in the order the original allocates them, or a reason there is none.
    local_list: (name, offset, renameable) from the listing; slots: (ours, the original's)."""
    named = sorted((offset, name, renameable) for name, offset, renameable in local_list if offset < 0)
    if not named:
        return None, "no named locals"
    starts = [offset for offset, _, _ in named]
    bases = {}
    for ours, theirs in slots:
        if ours < starts[0]:
            if ours != theirs:
                return None, "an unnamed temporary moved (ebp%+d, the original's ebp%+d)" % (ours, theirs)
            continue
        index = max(k for k, start in enumerate(starts) if start <= ours)
        base = theirs - (ours - starts[index])
        if bases.setdefault(index, base) != base:
            return None, "%s is at two places in the original" % named[index][1]
    sizes = [(starts[k + 1] if k + 1 < len(starts) else 0) - starts[k] for k in range(len(starts))]
    taken = sorted((base, base + sizes[k], k) for k, base in bases.items())
    for (_, end, k), (start, _, n) in zip(taken, taken[1:]):
        if end > start:
            return None, "%s and %s overlap in the original" % (named[k][1], named[n][1])
    if taken and (taken[0][0] < starts[0] or taken[-1][1] > 0):
        return None, "%s is outside the named locals' area in the original" % named[taken[0][2]][1]
    # Locals the code never addresses go into the gaps, the largest first.
    free, at = [], starts[0]
    for start, end, _ in taken:
        if start > at:
            free.append([at, start])
        at = end
    if at < 0:
        free.append([at, 0])
    for k in sorted((k for k in range(len(named)) if k not in bases), key=lambda k: -sizes[k]):
        gap = next((g for g in free if g[1] - g[0] >= sizes[k]), None)
        if gap is None:
            return None, "no room for %s in the original's frame" % named[k][1]
        bases[k] = gap[1] - sizes[k]
        gap[1] -= sizes[k]
    order = sorted(bases, key=lambda k: -bases[k])
    return [named[k][1] for k in order], None


class ProbeUnit(Unit):
    """A probe compiled with a unit's compiler and options, from a scratch file of its own."""

    def __init__(self, unit, tag):
        super().__init__(unit.target, unit.compiler, unit.language, unit.arguments, unit.source, unit.root)
        self.source = str(Path(unit.source).parent / ("zqprobe%s%s" % (tag, Path(unit.source).suffix))).replace("\\", "/")
        self.name = Path(self.source).name

    def files(self, tree):
        return {}


def ladder(compiler, target, unit, extras=()):
    """The compiler's allocation order as a list of entries: ("bucket", names, direction) for one
    bucket of the hash (names that all hash there; direction +1 if the first declared is allocated
    first, -1 if last, 0 for a single name) and ("fixed", name) for a name that can't be renamed
    (`this`; `extras`: a nested scope's duplicates, `i$99`, declared in the probe under their own
    names, after its pool), in the order the buckets are allocated.
    Found by compiling a probe twice."""
    alphabet = "abcdefghijklmnopqrstuvwxyz0123456789"
    pool = ["zq" + a + b for a in alphabet[:12] for b in alphabet]
    member = unit.language == "CXX"

    def probe(names):
        names = names + list(extras)  # after the locals, as a nested scope's are
        body = "".join("\tint %s;\n" % n for n in names) + "".join("\t%s = 0;\n" % n for n in names)
        if member:
            return "struct ZqProbeClass { int m_zq; void ZqProbe(); };\nvoid ZqProbeClass::ZqProbe()\n{\n%s\tm_zq = 0;\n}\n" % body
        return "void ZqProbe(void)\n{\n%s}\n" % body

    forward, backward = ProbeUnit(unit, "a"), ProbeUnit(unit, "b")
    requests = [Request(forward, ROOT, "probe", {forward.name: probe(pool)}, listing=True),
                Request(backward, ROOT, "probe", {backward.name: probe(pool[::-1])}, listing=True)]
    compiler.run(target, requests)
    orders = []
    for request in requests:
        if request.error:
            raise SystemExit("the probe doesn't compile: " + request.error)
        local_list = next(iter(read_listing(request.asm).values()))
        orders.append([name for _, name in sorted(((offset, name) for name, offset, _ in local_list), reverse=True)])
    rank = {name: k for k, name in enumerate(orders[1])}
    index = {name: k for k, name in enumerate(pool)}
    groups = [[orders[0][0]]]
    for name in orders[0][1:]:
        if rank[groups[-1][-1]] < rank[name]:  # the same order both ways: another bucket
            groups.append([name])
        else:
            groups[-1].append(name)
    entries = []
    for group in groups:
        fixed = [n for n in group if n not in index]
        if fixed:
            entries.append(("fixed", fixed[0]))
            continue
        direction = 0 if len(group) < 2 else (1 if index[group[1]] > index[group[0]] else -1)
        entries.append(("bucket", sorted(group, key=lambda n: index[n]), direction))
    return entries


def declaration_order(text, first, last, names):
    """The position of each local's first appearance in the function's text: its declaration."""
    body = "\n".join(text.split("\n")[first - 1 : last])
    body = body[body.find("{") :]
    seen, previous = {}, ""
    for m in TOKEN.finditer(body):
        token = m.group(0)
        if token in names and token not in seen and previous not in (".", "->", "::"):
            seen[token] = len(seen)
        if not token.startswith(("//", "/*")):
            previous = token
    return seen


def assign(order, entries, declared, fixed, share=False):
    """{local: new name} that the compiler allocates in `order`, or (None, a reason). Each local
    takes the next bucket, a fixed name (`this`, `i$99`) is where the compiler puts it, and with
    `share` (more locals than buckets), a local stays in the current bucket when the order of
    declaration allows it. Sharing is the fallback, not the default: the order within a bucket
    doesn't always follow the declarations as `declaration_order` reads them, and sharing whenever
    it was allowed lost 14 of MW2's witnesses that one local per bucket finds."""
    mapping, placed = {}, set()
    position, last = -1, None  # the current entry, the last local put in it
    for local in order:
        if local in fixed:
            k = next((k for k in range(position + 1, len(entries)) if entries[k] == ("fixed", local)), None)
            if k is None:
                return None, "%s can't come this late" % local
            position, last = k, None
            placed.add(local)
            continue
        entry = entries[position] if position >= 0 else None
        used = sum(1 for v in mapping.values() if entry and entry[0] == "bucket" and v in entry[1])
        if share and entry and entry[0] == "bucket" and last and used < len(entry[1]) and entry[2] * (declared.get(local, -1) - declared.get(last, -1)) > 0:
            mapping[local], last = entry[1][used], local
            continue
        k = position + 1
        while k < len(entries) and entries[k][0] != "bucket":
            if entries[k][1] in order and entries[k][1] not in placed:
                return None, "%s would come before %s" % (entries[k][1], local)
            k += 1
        if k >= len(entries):
            return None, "%d locals, and the hash has %d buckets" % (len(order), sum(1 for e in entries if e[0] == "bucket"))
        position, last = k, local
        mapping[local] = entries[k][1][0]
    return mapping, None


# ---------------------------------------------------------------------------------------------
# The search


class Entry:
    def __init__(self, unit, name, annotation):
        self.unit, self.name, self.annotation = unit, name, annotation
        self.address = annotation.offset
        self.status = None
        self.detail = ""
        self.renames = {}
        self.pads = 0
        self.unverified = []

    def note(self, status, detail, pads):
        """Keep the closest verdict: slots-only (at the lowest padding) over another difference,
        and the unpadded compile's difference over a padded one's."""
        rank = {"slots": 0, "other": 1}
        if self.status not in rank or rank[status] < rank[self.status] or rank[status] == rank[self.status] and not self.detail:
            self.status, self.detail = status, detail + (" (with %d pads)" % pads if pads else "")

    def recipe(self):
        return recipe({"pads": self.pads, "renames": self.renames})


class Search:
    def __init__(self, args, target):
        self.args, self.target_name = args, target
        original = args.original or read_yaml_paths().get(target)
        if not original or not Path(original).exists():
            raise SystemExit("no original for %s: pass --original" % target)
        self.image = Image(original)
        self.annotations = Annotations(target, source_roots(target))
        self.target = Target(target, ROOT / args.build)
        self.units = [u for u in self.target.units if u.path(ROOT).exists() and u.language != "ASM_MASM"]
        scratch = ROOT / args.build / "witness" / target
        self.compiler = Compiler(scratch, args.wine, args.jobs)
        self.ladders = {}
        self.entries = []
        self.pads = 0

    def compare(self, request, wanted=None):
        """{symbol: (Comparison, the function's locals, annotation)} of one compile, or an error."""
        if request.error:
            return request.error
        obj, local_lists = Object(request.obj), read_listing(request.asm)
        out = {}
        for symbol, annotation in unit_annotations(self.annotations, request.unit, obj).items():
            if wanted is None or symbol in wanted:
                comparison = Comparison(obj.functions[symbol], obj, self.image, annotation.offset, self.annotations)
                out[symbol] = (comparison, local_lists.get(symbol, []), annotation)
        return out

    def plan(self, unit, compared, pending):
        """The renamed files of the unit for the functions that differ only in slots."""
        texts, plans, orders = {}, {}, {}
        pads = self.pads
        for symbol, entry in pending.items():
            comparison, local_list, annotation = compared[symbol]
            if comparison.difference:
                entry.note("other", comparison.difference, pads)
                continue
            if optimized(unit):
                entry.note("slots", "an optimized unit: padding only", pads)
                continue
            order, reason = slot_plan(local_list, comparison.slots)
            if order is None:
                entry.note("slots", reason, pads)
                continue
            # The compiler's temporaries ($T123) come after the named locals, in the order it
            # creates them, outside the hash: they must end the order.
            while order and order[-1].startswith("$T"):
                order = order[:-1]
            fixed = {name for name, _, renameable in local_list if not renameable} & set(order)
            if any(name.startswith("$T") for name in fixed):
                entry.note("slots", "a temporary among the named locals", pads)
                continue
            orders[symbol] = (order, fixed)
        if not orders:
            return texts, plans
        extras = sorted({n for _, fixed in orders.values() for n in fixed} - set(FIXED))
        key = (unit.language, unit.compiler, tuple(extras))
        if key not in self.ladders:
            self.ladders[key] = ladder(self.compiler, self.target, unit, extras)
        entries = self.ladders[key]
        for symbol, (order, fixed) in orders.items():
            entry, annotation = pending[symbol], compared[symbol][2]
            path = Path(annotation.filename)
            text = texts.get(path.name) or path.read_text(encoding="utf-8")
            declared = declaration_order(path.read_text(encoding="utf-8"), annotation.line_number, annotation.end_line, set(order))
            names, reason = assign(order, entries, declared, fixed)
            if names is None:
                names, reason = assign(order, entries, declared, fixed, share=True)
            if names is None:
                entry.note("slots", reason, pads)
                continue
            plans[symbol] = names
            texts[path.name] = rename(text, annotation.line_number, annotation.end_line, names)
        return texts, plans

    def run(self):
        units = [u for u in self.units if not self.args.unit or u.name in self.args.unit]
        base = {r.unit.name: r for r in self.compiler.run(self.target, [Request(u, ROOT, "base", listing=True) for u in units])}
        pending = {}  # unit name -> {symbol: Entry}
        exact = defaultdict(set)
        for unit in units:
            compared = self.compare(base[unit.name])
            if isinstance(compared, str):
                print("%s: %s" % (unit.name, compared))
                continue
            for symbol, (comparison, _, annotation) in compared.items():
                name = symbol_name(symbol)
                if self.args.function and name not in self.args.function:
                    continue
                entry = Entry(unit, name, annotation)
                entry.unverified = comparison.unverified
                self.entries.append(entry)
                if comparison.exact:
                    entry.status = "exact"
                    exact[unit.name].add(symbol)
                else:
                    entry.note("other" if comparison.difference else "slots", comparison.difference or "", 0)
                    pending.setdefault(unit.name, {})[symbol] = entry
        by_name = {u.name: u for u in units}
        for pads in range(0, self.args.pads + 1):
            self.pads = pads
            todo = [by_name[n] for n, entries in pending.items() if entries]
            if not todo:
                break
            if pads:
                padded = {r.unit.name: r for r in self.compiler.run(self.target, [Request(u, ROOT, "pad", prefix=pads_text(pads), listing=True) for u in todo])}
            requests, plans = [], {}
            for unit in todo:
                compared = self.compare(padded[unit.name] if pads else base[unit.name], pending[unit.name])
                if isinstance(compared, str):
                    continue
                for symbol, entry in list(pending[unit.name].items()):
                    if pads and symbol in compared and compared[symbol][0].exact:
                        entry.status, entry.pads, entry.unverified, entry.detail = "witness", pads, compared[symbol][0].unverified, ""
                        del pending[unit.name][symbol]
                texts, plans[unit.name] = self.plan(unit, compared, {s: e for s, e in pending[unit.name].items() if s in compared})
                if texts:
                    requests.append(Request(unit, ROOT, "rename", texts, prefix=pads_text(pads), listing=True))
            self.compiler.run(self.target, requests)
            for request in requests:
                unit = request.unit
                compared = self.compare(request)
                if isinstance(compared, str):
                    for symbol in plans[unit.name]:
                        pending[unit.name][symbol].note("slots", "renamed: " + compared, pads)
                    continue
                for symbol in exact[unit.name]:
                    if not pads and symbol in compared and not compared[symbol][0].exact:
                        print("warning: %s: renaming other functions' locals changed %s" % (unit.name, symbol))
                for symbol, names in plans[unit.name].items():
                    entry = pending[unit.name][symbol]
                    comparison = compared[symbol][0] if symbol in compared else None
                    if comparison is not None and comparison.exact:
                        entry.status, entry.renames, entry.pads, entry.unverified = "witness", names, pads, comparison.unverified
                        entry.detail = ""
                        del pending[unit.name][symbol]
                    else:
                        entry.note("slots", "renamed, still differs" + (": " + comparison.difference if comparison is not None and comparison.difference else ""), pads)
            self.compiler.discard(requests)
            if pads:
                self.compiler.discard(padded.values())
        self.compiler.discard(base.values())


def claims(entry):
    """What the comments above the function's annotations say its diff is: 'slots' for a stack-slot
    permutation, 'other' for another explanation, '' for none."""
    lines = Path(entry.annotation.filename).read_text(encoding="utf-8").split("\n")
    at = entry.annotation.line_number - 2
    text = []
    while at >= 0 and lines[at].lstrip().startswith("//"):
        text.append(lines[at])
        at -= 1
    comment = " ".join(line for line in text if not re.match(r"\s*// (FUNCTION|STUB|LIBRARY|SYNTHETIC|GLOBAL):", line))
    if re.search(r"slot", comment, re.I):
        return "slots"
    if re.search(r"operand order|register|matches but|only diff|differs|scheduling|entropy", comment, re.I):
        return "other"
    return ""


def write_records(path, target, entries, searched):
    """Replace the records of the searched functions, keep the others."""
    kept = [r for r in read_records(path) if (r["unit"], r["address"]) not in searched]
    lines = {r["address"]: "%#010x %s %s %s" % (r["address"], r["name"], r["unit"], recipe(r)) for r in kept}
    for e in entries:
        if e.status == "witness":
            lines[e.address] = "%#010x %s %s %s" % (e.address, e.name, e.unit.name, e.recipe())
    header = ["# %s: witnesses that a function differs from the original only by compiler entropy." % target,
              "# address, function, unit, then the recipe: `pad N` symbols declared ahead of the unit, and/or",
              "# `rename old=new ...` of the function's locals. Found by tools/witness.py, checked by tools/certify.py.", ""]
    path.parent.mkdir(exist_ok=True)
    path.write_text("\n".join(header + [lines[a] for a in sorted(lines)]) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", required=True, choices=sorted(TARGETS))
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build", help="The build tree, relative to the repository")
    parser.add_argument("--original", help="The original binary (default: reccmp-user.yml's)")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"), help="Wine, off Windows")
    parser.add_argument("--unit", nargs="*", help="Only these units (ai.c)")
    parser.add_argument("--function", nargs="*", help="Only these functions")
    parser.add_argument("--pads", type=int, default=0, help="Also try up to N symbols declared ahead of the unit")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--write", action="store_true", help="Record the witnesses in tools/witnesses/<TARGET>.txt")
    parser.add_argument("--json", help="Write every function's result to this file")
    parser.add_argument("-v", "--verbose", action="store_true", help="List every function that isn't exact")
    args = parser.parse_args()

    search = Search(args, args.target)
    search.run()
    entries = sorted(search.entries, key=lambda e: e.address)
    counts = Counter(e.status for e in entries)
    if args.verbose:
        for entry in entries:
            if entry.status != "exact":
                print("%#010x %-8s %-32s %s" % (entry.address, entry.status, entry.name, entry.recipe() or entry.detail))
    print("%s: %d functions in %d units (%d compiles)" % (args.target, len(entries), len({e.unit.name for e in entries}), search.compiler.count))
    for status in ("exact", "witness", "slots", "other"):
        print("  %-8s %d" % (status, counts[status]))
    said = Counter((claims(e), e.status) for e in entries if e.status != "exact")
    for claim in ("slots", "other", ""):
        row = {s: said[(claim, s)] for s in ("witness", "slots", "other")}
        if sum(row.values()):
            print("  comment says %-6s %s" % ((claim or "nothing") + ":", ", ".join("%s %d" % item for item in row.items())))
    unverified = Counter(name for e in entries if e.status in ("exact", "witness") for name in e.unverified)
    if unverified:
        print("  relocation targets no annotation names (unverified): %d symbols, %d uses" % (len(unverified), sum(unverified.values())))
        if args.verbose:
            print("    " + ", ".join("%s (%d)" % item for item in unverified.most_common(20)))
    if args.json:
        Path(args.json).write_text(json.dumps([
            {"address": "%#010x" % e.address, "name": e.name, "unit": e.unit.name, "status": e.status, "detail": e.detail,
             "pads": e.pads, "renames": e.renames, "unverified": e.unverified, "claim": claims(e) if e.status != "exact" else ""}
            for e in entries], indent=1) + "\n")
    if args.write:
        record_file = ROOT / "tools" / "witnesses" / (args.target + ".txt")
        write_records(record_file, args.target, entries, {(e.unit.name, e.address) for e in entries})
        print("wrote %s" % record_file.relative_to(ROOT))
    return 0


if __name__ == "__main__":
    sys.exit(main())
