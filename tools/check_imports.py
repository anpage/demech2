#!/usr/bin/env python3
"""Check the imports the build links, and the imports each function uses, against the original's.

reccmp and certify compare a function's code; a relocated field compares by the annotation of the
symbol it names. Imports fall through both: an import slot (`__imp__DeleteFileA@4`) has no
annotation, so a call through one is never checked, and a library function is whatever its
`// LIBRARY:` annotation calls it, so game code that calls the wrong one still compares equal
(MW2's `/E` switch called `_rmdir` where the original calls `remove`: the two differ only in the
import they call, and the annotation named the original's `remove` `_rmdir`). This tool checks
what they leave:

- missing, extra: an import (DLL and name, or ordinal) that only the original's import table has,
  or only the build's.
- function: a function matched in both images, game code or library, whose code doesn't use the
  same imports. A use is a relocated field that holds an import slot's address
  (`call dword ptr [slot]`, `mov esi, dword ptr [slot]`) or a call or jump to an import's `jmp`
  thunk. The original's function runs to the next annotated function, or to the first function
  start before it (a `ret`, any padding and another /Od frame: a target annotated in part has
  functions no annotation names yet); a library function, which the original may follow with functions no
  annotation names, runs as far as the build's does.
- order: a function that uses the same imports, but not in the same order or not as often. In
  unoptimized code that is two calls swapped (their bytes are the same but for the slot). In
  optimized code it is also an import's address kept in a register in one image and read from
  its slot at each call in the other: a function that isn't the original's yet.

Findings the project knows about are listed in tools/check_imports_expected.txt with a reason; the
check fails on any other finding, and on listed entries that no longer occur (remove them once
fixed).
"""

import argparse
import bisect
import logging
import struct
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

DEFAULT_EXPECTED = Path(__file__).with_name("check_imports_expected.txt")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    argparse_add_project_target_args(parser)
    parser.add_argument("--expected", type=Path, default=DEFAULT_EXPECTED, help="The known findings")
    parser.add_argument("-v", "--verbose", action="store_true", help="List every function that uses imports")
    return parser.parse_args()


class Imports:
    """An image's imports: the slots of its import table, the `jmp` thunks to them, and the uses in
    a range of its code."""

    def __init__(self, image):
        self.image = image
        self.slots = {}  # a slot's address -> "dll!name"
        for entry in image.imports:
            self.slots[entry.addr] = "%s!%s" % (entry.module.lower(), entry.name or "#%d" % entry.ordinal)
        self.relocations = sorted(image.relocations)
        # `jmp dword ptr [slot]` (ff 25): the thunk an import library gives a function that is
        # declared without __declspec(dllimport).
        self.thunks = {}
        for address in self.relocations:
            slot = self.dword(address)
            if slot in self.slots and self.read(address - 2, 2) == b"\xff\x25":
                self.thunks[address - 2] = self.slots[slot]

    def read(self, address: int, size: int) -> bytes:
        try:
            return self.image.read(address, size) or b""
        except Exception:  # outside the image's sections
            return b""

    def function_end(self, start: int, limit: int) -> int:
        """Where the original's function at start ends, at limit at the latest: the first `ret`
        (and any padding) followed by another function's /Od frame."""
        code = self.read(start, limit - start)
        for i in range(3, len(code) - 2):
            if code[i : i + 3] != b"\x55\x8b\xec":
                continue
            j = i
            while j > 3 and code[j - 1] in (0xCC, 0x90):
                j -= 1
            if code[j - 1] == 0xC3 or code[j - 3] == 0xC2:
                return start + i
        return limit

    def dword(self, address: int):
        data = self.read(address, 4)
        return struct.unpack("<I", data)[0] if len(data) == 4 else None

    def uses(self, start: int, end: int) -> list[str]:
        """The imports the code in [start, end) uses, in the order of their places in the code."""
        found = {}
        first = bisect.bisect_left(self.relocations, start)
        last = bisect.bisect_left(self.relocations, end)
        relocated = set()
        for address in self.relocations[first:last]:
            relocated.update(range(address, address + 4))
            slot = self.dword(address)
            if slot in self.slots and address - 2 not in self.thunks:
                found[address] = self.slots[slot]
        code = self.read(start, end - start)
        for offset in range(max(0, len(code) - 4)):
            # call rel32, jmp rel32: an operand that lands on a thunk (a relocated field is an
            # address, not a displacement).
            if code[offset] in (0xE8, 0xE9) and start + offset + 1 not in relocated:
                target = (start + offset + 5 + struct.unpack_from("<i", code, offset + 1)[0]) & 0xFFFFFFFF
                if target in self.thunks:
                    found[start + offset + 1] = self.thunks[target]
        return [found[address] for address in sorted(found)]


def load_expected(path: Path, target_id: str) -> dict[tuple[str, str], str]:
    expected: dict[tuple[str, str], str] = {}
    with open(path, encoding="utf-8") as file:
        for line in file:
            line, _, reason = line.partition("#")
            fields = line.split()
            if len(fields) == 3 and fields[0] == target_id:
                expected[(fields[1], fields[2])] = reason.strip()
    return expected


def first_difference(ours: list[str], theirs: list[str]) -> str:
    for index, (a, b) in enumerate(zip(ours, theirs)):
        if a != b:
            return f"use {index + 1} of {len(theirs)}: the build's {a}, the original's {b}"
    if len(ours) > len(theirs):
        return f"the build uses {len(ours)} ({ours[len(theirs)]} next), the original {len(theirs)}"
    return f"the original uses {len(theirs)} ({theirs[len(ours)]} next), the build {len(ours)}"


def main():
    args = parse_args()
    try:
        target = argparse_parse_project_target(args=args)
    except RecCmpProjectException as e:
        logger.error(e.args[0])
        return 1

    print(f"Checking the imports against the original's: {target.target_id}")

    compare = Compare.from_target(target)
    original, build = Imports(compare.orig_bin), Imports(compare.recomp_bin)
    findings: list[tuple[str, str, str]] = []

    theirs, ours = set(original.slots.values()), set(build.slots.values())
    for name in sorted(theirs - ours):
        findings.append(("missing", name, f"the original imports {name}, the build doesn't"))
    for name in sorted(ours - theirs):
        findings.append(("extra", name, f"the build imports {name}, the original doesn't"))

    functions = [
        entity
        for entity in compare.get_all()
        if entity.entity_type == EntityType.FUNCTION and entity.orig_addr is not None and not entity.get("stub")
    ]
    starts = sorted({entity.orig_addr for entity in functions})
    build_starts = sorted({entity.recomp_addr for entity in compare.get_all() if entity.entity_type == EntityType.FUNCTION and entity.recomp_addr is not None})
    checked = using = 0
    for entity in sorted(functions, key=lambda entity: entity.orig_addr):
        if entity.recomp_addr is None:
            continue
        index = bisect.bisect_right(starts, entity.orig_addr)
        following = starts[index] if index < len(starts) else None
        size = entity.size(ImageId.RECOMP)
        if not size:
            index = bisect.bisect_right(build_starts, entity.recomp_addr)
            size = build_starts[index] - entity.recomp_addr if index < len(build_starts) else 0
        if not size:
            continue
        end = entity.orig_addr + size
        if following is not None and (not entity.get("library") or following < end):
            end = following
        if end - entity.orig_addr > 0x10000:  # the last annotated function: no further than the build's
            end = entity.orig_addr + size
        if not entity.get("library"):
            end = original.function_end(entity.orig_addr, end)
        wanted = original.uses(entity.orig_addr, end)
        found = build.uses(entity.recomp_addr, entity.recomp_addr + size)
        checked += 1
        using += bool(wanted or found)
        name = entity.best_name() or f"0x{entity.orig_addr:08x}"
        if args.verbose and (wanted or found):
            print(f"  0x{entity.orig_addr:08x} {name}: {', '.join(wanted)}")
        if wanted != found:
            kind = "order" if set(wanted) == set(found) else "function"
            findings.append((kind, f"0x{entity.orig_addr:08x}", f"{name}: {first_difference(found, wanted)}"))

    expected = load_expected(args.expected, target.target_id)
    present = {(kind, key) for kind, key, _ in findings}
    unexpected = [finding for finding in findings if (finding[0], finding[1]) not in expected]
    stale = sorted(key for key in expected if key not in present)

    for kind, key, message in findings:
        mark = "known" if (kind, key) in expected else "error"
        print(f"{mark}: {kind} {key}: {message}")
    for kind, key in stale:
        print(f"error: {kind} {key} is listed in {args.expected.name} but no longer occurs; remove it")

    print(
        f"{len(theirs)} imports and {checked} functions checked ({using} use imports); "
        f"{len(findings)} findings ({len(findings) - len(unexpected)} known), {len(stale)} stale entries"
    )
    return 1 if unexpected or stale else 0


if __name__ == "__main__":
    sys.exit(main())
