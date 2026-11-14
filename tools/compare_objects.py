#!/usr/bin/env python3
"""Check that a change leaves every function of every target as it was: the byte gate.

Compiles each unit of each target from two source trees, with the target's own compiler and flags
(from the build tree's NMake files), and compares the objects outside their debug sections: every
section in order (its name, flags and bytes), every relocation (its offset, type and the symbol
it names), and the symbol table. The compiler's own counters (string literals `$SG123`, labels,
temporaries, a nested scope's `i$99`) are renumbered by order of appearance first, so that what
they number is compared, not the counter. A change that only renames macros, moves text between
branches a target doesn't compile, edits comments or shifts lines passes; a function whose bytes
change is a finding: its tokens changed, or the symbols declared ahead of it did.

BASE and CHANGE are directories (a copy of the repository) or git revisions, which are exported
to a scratch directory (`git archive`). The working tree is `.`.

usage (from the repository root):
  python tools/compare_objects.py BASE CHANGE [--target MW2 ...] [--unit ai.c ...]
         [--build build-wine] [--wine PATH] [--jobs N] [--json FILE] [-v]

Exit status 1 when anything differs or doesn't compile.
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from unitbuild import IMAGE_SYM_CLASS_FILE, IMAGE_SYM_CLASS_FUNCTION, ROOT, TARGETS, Compiler, Object, Request, Target  # noqa: E402

SIMULATORS = ["MW2", "MW2MATROX", "MW23DFX"]


def tree_of(spec, scratch, label):
    """A directory as is, or a git revision exported to scratch/label."""
    path = Path(spec)
    if path.is_dir():
        return path.resolve()
    out = scratch / label
    shutil.rmtree(out, ignore_errors=True)
    out.mkdir(parents=True)
    archive = subprocess.run(["git", "-C", str(ROOT), "archive", spec], check=True, capture_output=True).stdout
    subprocess.run(["tar", "-x", "-C", str(out)], input=archive, check=True)
    return out


def relocations(section, start=0, end=None):
    end = section.size if end is None else end
    return [(o - start, s.canonical, t) for o, s, t in section.relocations if start <= o < end]


def symbol_table(obj):
    """The symbols outside the debug sections, by canonical name, with their section's ordinal."""
    ordinal, n = {}, 0
    for section in obj.sections:
        if not section.debug:
            ordinal[section.number] = n
            n += 1
    out = []
    for s in obj.symbols:
        if s.section > 0 and s.section not in ordinal:
            continue
        if s.storage in (IMAGE_SYM_CLASS_FUNCTION, IMAGE_SYM_CLASS_FILE):
            continue
        out.append((s.canonical, ordinal.get(s.section, s.section), s.value, s.kind, s.storage))
    return out


def ranges(obj, section):
    """(start, end, name) of the symbols in a section, in order."""
    starts = sorted((s.value, s.canonical) for s in obj.symbols
                    if s.section == section.number and s.name != section.name and s.storage != IMAGE_SYM_CLASS_FUNCTION)
    out = []
    for k, (start, name) in enumerate(starts):
        end = starts[k + 1][0] if k + 1 < len(starts) else section.size
        out.append((start, end, name))
    return out


def compare(a, b):
    """The differences between two objects: a list of strings, empty when they are the same."""
    found = []
    sa = [s for s in a.sections if not s.debug]
    sb = [s for s in b.sections if not s.debug]
    if [s.name for s in sa] != [s.name for s in sb]:
        found.append("sections: %s, then %s" % (" ".join(s.name for s in sa), " ".join(s.name for s in sb)))
        return found
    for x, y in zip(sa, sb):
        same = x.flags == y.flags and x.size == y.size and x.data == y.data and relocations(x) == relocations(y)
        if same:
            continue
        # Which symbols' bytes differ (functions in code, variables and literals in data).
        rx, ry = ranges(a, x), ranges(b, y)
        names = []
        if [n for _, _, n in rx] == [n for _, _, n in ry]:
            for (s0, e0, name), (s1, e1, _) in zip(rx, ry):
                bx = x.data[s0:e0] if x.data is not None else e0 - s0
                by = y.data[s1:e1] if y.data is not None else e1 - s1
                if bx != by or relocations(x, s0, e0) != relocations(y, s1, e1):
                    names.append(name)
            what = ", ".join(names) if names else "bytes outside any symbol"
        else:
            what = "other symbols (%d, then %d)" % (len(rx), len(ry))
        found.append("%s #%d: %s" % (x.name, x.number, what))
    ta, tb = symbol_table(a), symbol_table(b)
    if ta != tb:
        only_a = sorted(set(ta) - set(tb))
        only_b = sorted(set(tb) - set(ta))
        detail = []
        if only_a:
            detail.append("only in base: " + ", ".join(t[0] for t in only_a[:8]))
        if only_b:
            detail.append("only in change: " + ", ".join(t[0] for t in only_b[:8]))
        found.append("symbols: " + ("; ".join(detail) if detail else "the same symbols in another order"))
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("base", help="Directory or git revision")
    parser.add_argument("change", help="Directory or git revision ('.' is the working tree)")
    parser.add_argument("--target", nargs="*", choices=sorted(TARGETS), help="Default: the three simulators")
    parser.add_argument("--unit", nargs="*", help="Only these units (file names: ai.c)")
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build",
                        help="The configured build tree whose commands are used, relative to the repository")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"), help="Wine, off Windows")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--scratch", help="Scratch directory (default: a temporary one)")
    parser.add_argument("--json", help="Write the differences to this file")
    parser.add_argument("-v", "--verbose", action="store_true", help="List the units compared")
    args = parser.parse_args()

    scratch = Path(args.scratch) if args.scratch else Path(tempfile.mkdtemp(prefix="compare_objects-"))
    scratch.mkdir(parents=True, exist_ok=True)
    base = tree_of(args.base, scratch, "base")
    change = tree_of(args.change, scratch, "change")
    compiler = Compiler(scratch / "work", args.wine, args.jobs)
    build = ROOT / args.build

    report, failed = {}, False
    for name in args.target or SIMULATORS:
        target = Target(name, build)
        units = [u for u in target.units if not args.unit or u.name in args.unit]
        result = {"units": len(units), "identical": 0, "differ": {}, "errors": {}}
        report[name] = result
        # Compile and compare a chunk of units at a time: a chunk's objects are scratch files that
        # the next chunk's compiles replace.
        chunk = max(1, compiler.jobs) * 8
        for i in range(0, len(units), chunk):
            part, objects = [], {}
            for unit in units[i : i + chunk]:
                for side, tree in (("base", base), ("change", change)):
                    if not unit.path(tree).exists():
                        result["errors"].setdefault(unit.source, []).append("no source in %s" % side)
                    else:
                        part.append(Request(unit, tree, side))
            compiler.run(target, part)
            for request in part:
                if request.error:
                    result["errors"].setdefault(request.unit.source, []).append("%s: %s" % (request.tag, request.error))
                else:
                    objects[(request.unit.source, request.tag)] = Object(request.obj)
            compiler.discard(part)
            for unit in {r.unit.source: r.unit for r in part}.values():
                a, b = objects.get((unit.source, "base")), objects.get((unit.source, "change"))
                if a is None or b is None:
                    continue
                found = compare(a, b)
                if found:
                    result["differ"][unit.source] = found
                else:
                    result["identical"] += 1
                if args.verbose:
                    print("%s %s: %s" % (name, unit.source, "differs" if found else "identical"), flush=True)
        failed = failed or result["differ"] or result["errors"]
        print("%s: %d units, %d identical, %d differ, %d failed" % (name, result["units"], result["identical"], len(result["differ"]), len(result["errors"])))
        for source, found in sorted(result["differ"].items()):
            for line in found:
                print("  %s: %s" % (source, line))
        for source, errors in sorted(result["errors"].items()):
            for line in errors:
                print("  %s: %s" % (source, line))
    print("%d compiles" % compiler.count)
    if args.json:
        Path(args.json).write_text(json.dumps(report, indent=1) + "\n")
    if not args.scratch:
        shutil.rmtree(scratch, ignore_errors=True)
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
