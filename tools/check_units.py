#!/usr/bin/env python3
"""Check that functions and globals sit in the translation units the original's layout implies.

The linker lays out each section object by object, in link order, so every
object file's functions (and, separately, its initialized data and its
uninitialized data) form one run in the original. reccmp and datacmp match by
name and don't care which source file defines a symbol; this tool does. For
every annotated function and global it takes the object the recompiled build
put it in (from the linker map next to the recompiled binary) and checks the
original's layout against that split:

- order: sorted by original address, each object's symbols must form one run,
  and the runs must follow the recompiled link order. The ordered sequence of
  runs that keeps the most objects in place is the reference; the runs left
  out are reported as misplaced, adjacent ones merged into one block. Checked
  separately for code and for each data group (.rdata, .data, .bss).
- split: an object boundary in the original (a 16-aligned function start with
  0xCC padding before it) between two functions of the same object: the
  source file holds two of the original's objects.
- start: an object that begins at a function the original has in the middle
  of another object (unaligned start, right after the previous object's
  function). The previous function is only known to end there when its
  recompiled size says so, or when the expected-findings file declares the
  target complete (every game function annotated). A 16-aligned start without
  padding before it is ambiguous (the previous object may have ended flush)
  and is not reported; so is a 4-aligned start of a MASM unit (.asm), whose
  object ML aligns to 4 and which starts flush after the previous one.
- section: a global in the original's .rdata that the build puts in writable
  data, or the reverse (a missing or extra `const`).
- static: a C tentative definition (a communal variable, allocated after all
  objects' .bss in the recompiled build) that the original allocates among
  the objects' own .bss: it was `static` there.

Findings the project knows about are listed in tools/check_units_expected.txt
with a reason; the check fails on any other finding, and on listed entries
that no longer occur (remove them once fixed).
"""

import argparse
import logging
import re
import sys
from pathlib import Path

from reccmp.compare import Compare
from reccmp.project.detect import (
    RecCmpProjectException,
    argparse_add_project_target_args,
    argparse_parse_project_target,
)
from reccmp.types import EntityType, ImageId

# Ignore all compare-db messages.
logging.getLogger("reccmp.compare").addHandler(logging.NullHandler())

logger = logging.getLogger()

DEFAULT_EXPECTED = Path(__file__).with_name("check_units_expected.txt")

VARIABLE_TYPES = (EntityType.DATA, EntityType.POINTER)

# ` 0003:0001ac70 00021384H .bss                    DATA`
_group_regex = re.compile(r"^\s*([0-9a-f]{4}):([0-9a-f]{8}) ([0-9a-f]{8})H (\S+)\s+(?:CODE|DATA)\s*$")
# ` 0003:0001f8a4       _g_pWnd                    100678a4   shellmain.cpp.obj`
# ` 0001:00000000       _FUN_10001000              10001000 f   debrief.cpp.obj`
_symbol_regex = re.compile(r"^\s*([0-9a-f]{4}):([0-9a-f]{8})\s+(\S+)\s+([0-9a-f]{8})(?:\s+[fi])*\s+(\S+)\s*$")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--expected", type=Path, default=DEFAULT_EXPECTED, help="List of known findings")
    parser.add_argument("--verbose", action="store_true", help="Also list the ambiguous object starts")
    argparse_add_project_target_args(parser)
    return parser.parse_args()


class LinkerMap:
    """The recompiled build's object file for each symbol, and its section groups."""

    def __init__(self, path: Path, recomp_bin):
        self.groups: list[tuple[int, int, str]] = []  # (start, end, name)
        self.objects: dict[int, str] = {}
        with open(path, encoding="latin-1") as file:
            for line in file:
                if (match := _group_regex.match(line)) is not None:
                    start = recomp_bin.get_abs_addr(int(match.group(1), 16), int(match.group(2), 16))
                    self.groups.append((start, start + int(match.group(3), 16), match.group(4)))
                elif (match := _symbol_regex.match(line)) is not None:
                    addr = int(match.group(4), 16)
                    obj = match.group(5)
                    # Prefer the defining object over an alias from elsewhere.
                    if addr not in self.objects or self.objects[addr].startswith("<"):
                        self.objects[addr] = obj

    def group(self, addr: int) -> str | None:
        for start, end, name in self.groups:
            if start <= addr < end:
                return name
        return None


class Symbol:
    def __init__(self, entity, obj: str):
        self.orig = entity.orig_addr
        self.recomp = entity.recomp_addr
        self.name = entity.best_name() or f"0x{self.orig:08x}"
        self.size = entity.size(ImageId.RECOMP) or 0
        self.obj = obj

    @property
    def unit(self) -> str:
        return self.obj.removesuffix(".obj")


def runs_of(symbols: list[Symbol]) -> list[list[Symbol]]:
    """Consecutive symbols (in original address order) of the same object."""
    runs: list[list[Symbol]] = []
    for symbol in symbols:
        if runs and runs[-1][0].obj == symbol.obj:
            runs[-1].append(symbol)
        else:
            runs.append([symbol])
    return runs


def ordered_runs(runs: list[list[Symbol]], rank: dict[str, int]) -> set[int]:
    """Indices of the sequence of runs, ranks not decreasing, that keeps the most objects in
    order (then the most symbols). Counting objects rather than symbols keeps a small object
    at its place in the link order when a big block of misplaced data surrounds it."""
    best = [(1, len(run)) for run in runs]
    prev = [-1] * len(runs)
    for i, run in enumerate(runs):
        for j in range(i):
            if rank[runs[j][0].obj] > rank[run[0].obj]:
                continue
            new = rank[runs[j][0].obj] < rank[run[0].obj]
            score = (best[j][0] + new, best[j][1] + len(run))
            if score > best[i]:
                best[i] = score
                prev[i] = j
    kept: set[int] = set()
    i = max(range(len(runs)), key=lambda k: best[k], default=-1)
    while i >= 0:
        kept.add(i)
        i = prev[i]
    return kept


def describe(symbols: list[Symbol]) -> str:
    units = list(dict.fromkeys(symbol.unit for symbol in symbols))
    if len(units) > 1:
        return f"a block of {len(symbols)} symbols from {', '.join(units)}"
    names = ", ".join(symbol.name for symbol in symbols[:3])
    if len(symbols) > 3:
        names += f", ... ({len(symbols)} in all)"
    return f"{names} from {units[0]}"


def check_order(what: str, symbols: list[Symbol], findings: list[tuple[str, int, str]], hint=None) -> set[int]:
    """Report the runs out of link order, adjacent ones merged into one block. Returns the
    original addresses of the symbols in order."""
    rank: dict[str, int] = {}
    for symbol in symbols:
        rank[symbol.obj] = min(rank.get(symbol.obj, symbol.recomp), symbol.recomp)
    runs = runs_of(symbols)
    kept = ordered_runs(runs, rank)
    ordered = {symbol.orig for i in kept for symbol in runs[i]}

    blocks: list[tuple[int, list[Symbol]]] = []
    for i, run in enumerate(runs):
        if i in kept:
            continue
        if blocks and blocks[-1][0] + len(blocks[-1][1]) == sum(len(r) for r in runs[:i]):
            blocks[-1][1].extend(run)
        else:
            blocks.append((sum(len(r) for r in runs[:i]), list(run)))

    for start, block in blocks:
        before = next((s.unit for s in reversed(symbols[:start]) if s.orig in ordered), "the start")
        after = next((s.unit for s in symbols[start + len(block):] if s.orig in ordered), "the end")
        message = (
            f"{what}: {describe(block)} (0x{block[0].orig:08x}-0x{block[-1].orig:08x}) "
            f"sit between {before} and {after} in the original"
        )
        if hint is not None:
            message += hint(block, ordered)
        findings.append(("order", block[0].orig, message))
    return ordered


def check_code(compare: Compare, functions: list[Symbol], findings, ambiguous: list[str], complete: bool):
    check_order("code", functions, findings)

    for prev, cur in zip(functions, functions[1:]):
        # ML aligns an assembled object's .text to 4, VC++ to 16
        aligned = cur.orig % (4 if cur.unit.lower().endswith(".asm") else 16) == 0
        padded = aligned and compare.orig_bin.read(cur.orig - 1, 1)[0] == 0xCC
        if prev.obj == cur.obj and padded:
            findings.append(
                (
                    "split",
                    cur.orig,
                    f"code: the original starts an object at {cur.name} (0x{cur.orig:08x}), "
                    f"inside {cur.unit}",
                )
            )
        elif prev.obj != cur.obj and not aligned and (complete or prev.orig + prev.size == cur.orig):
            findings.append(
                (
                    "start",
                    cur.orig,
                    f"code: {cur.unit} begins at {cur.name} (0x{cur.orig:08x}), in the middle of "
                    f"an object in the original (after {prev.name} from {prev.unit})",
                )
            )
        elif prev.obj != cur.obj and not aligned:
            ambiguous.append(f"{cur.unit} begins at 0x{cur.orig:08x}, possibly after unannotated code")
        elif prev.obj != cur.obj and not padded:
            ambiguous.append(f"{cur.unit} begins at 0x{cur.orig:08x} flush against {prev.unit}")


def check_data(group: str, variables: list[Symbol], findings):
    commons = [symbol for symbol in variables if symbol.obj == "<common>"]
    own = [symbol for symbol in variables if symbol.obj != "<common>"]
    if group != ".bss":
        check_order(group, own, findings)
        return

    # The original allocates its communal variables after every object's .bss.
    def communal(block, ordered):
        if ordered and block[0].orig > max(ordered):
            return " (the original's communal area: C tentative definitions?)"
        return ""

    kept = check_order(group, own, findings, communal)
    if not kept:
        return
    end = max(kept)
    for symbol in commons:
        if symbol.orig < end:
            findings.append(
                (
                    "static",
                    symbol.orig,
                    f".bss: {symbol.name} (0x{symbol.orig:08x}) is communal in the build, but the "
                    f"original allocates it among the objects' own .bss: static?",
                )
            )


def orig_section(orig_bin, addr: int) -> str | None:
    for section in orig_bin.sections:
        if section.virtual_address <= addr < section.virtual_address + section.virtual_size:
            return section.name
    return None


def load_expected(path: Path, target_id: str) -> tuple[dict[tuple[str, int], str], bool]:
    """The known findings for the target, and whether the file declares the target complete."""
    expected: dict[tuple[str, int], str] = {}
    complete = False
    with open(path, encoding="utf-8") as file:
        for line in file:
            line, _, reason = line.partition("#")
            fields = line.split()
            if not fields or fields[0] != target_id:
                continue
            if fields[1:] == ["complete"]:
                complete = True
                continue
            _, kind, addr = fields
            expected[(kind, int(addr, 16))] = reason.strip()
    return expected, complete


def main():
    args = parse_args()
    try:
        target = argparse_parse_project_target(args=args)
    except RecCmpProjectException as e:
        logger.error(e.args[0])
        return 1

    print(f"Checking the source split against the original's layout: {target.target_id}")

    compare = Compare.from_target(target)
    linker_map = LinkerMap(target.recompiled_path.with_suffix(".map"), compare.recomp_bin)

    functions: list[Symbol] = []
    variables: dict[str, list[Symbol]] = {}
    findings: list[tuple[str, int, str]] = []
    for entity in compare.get_all():
        if entity.orig_addr is None or entity.recomp_addr is None or entity.get("library"):
            continue
        obj = linker_map.objects.get(entity.recomp_addr)
        # Skip library objects (LIBCMT:x.obj) and linker-made symbols.
        if obj is None or ":" in obj or (obj.startswith("<") and obj != "<common>"):
            continue
        if entity.entity_type == EntityType.FUNCTION:
            functions.append(Symbol(entity, obj))
        elif entity.entity_type in VARIABLE_TYPES:
            # Group by the original's section; data in the original's .text belongs to
            # hand-written assembly and is skipped. Uninitialized data is told apart by the
            # build, since the original's raw .data can run past its initialized part.
            group = orig_section(compare.orig_bin, entity.orig_addr)
            if group is None or group == ".text":
                continue
            symbol = Symbol(entity, obj)
            recomp_group = "<common>" if obj == "<common>" else linker_map.group(entity.recomp_addr)
            if (group == ".rdata") != (recomp_group == ".rdata"):
                findings.append(
                    (
                        "section",
                        symbol.orig,
                        f"{symbol.name} (0x{symbol.orig:08x}) from {symbol.unit} is in the original's "
                        f"{group} but the build's {recomp_group}",
                    )
                )
                continue
            if recomp_group in (".bss", "<common>"):
                group = ".bss"
            variables.setdefault(group, []).append(symbol)

    ambiguous: list[str] = []
    expected, complete = load_expected(args.expected, target.target_id)
    functions.sort(key=lambda symbol: symbol.orig)
    check_code(compare, functions, findings, ambiguous, complete)
    for group in sorted(variables):
        check_data(group, sorted(variables[group], key=lambda symbol: symbol.orig), findings)

    if args.verbose:
        for line in ambiguous:
            print("  ambiguous: " + line)

    found = {(kind, addr) for kind, addr, _ in findings}
    unexpected = [finding for finding in findings if (finding[0], finding[1]) not in expected]
    stale = sorted(key for key in expected if key not in found)

    for kind, addr, message in sorted(findings, key=lambda finding: finding[1]):
        mark = "known" if (kind, addr) in expected else "error"
        print(f"{mark}: {kind} 0x{addr:08x}: {message}")
    for kind, addr in stale:
        print(f"error: {kind} 0x{addr:08x} is listed in {args.expected.name} but no longer occurs; remove it")

    print(
        f"{len(functions)} functions and {sum(len(v) for v in variables.values())} globals checked; "
        f"{len(findings)} findings ({len(findings) - len(unexpected)} known), {len(stale)} stale entries"
    )
    return 1 if unexpected or stale else 0


if __name__ == "__main__":
    sys.exit(main())
