#!/usr/bin/env python3
"""Check the original binary's block operations against the recompiled layout.

The original reads, writes and copies some runs of globals as one block. reccmp
and datacmp compare symbol by symbol, so a run declared as separate globals
passes both even though the block runs into unrelated data in the recompiled
build. This tool walks the original game code and checks, for each block
operation whose pointer is a known global, that the recompiled object at that
address has room for the size the original uses:

- calls to the CRT and Win32 block functions (fread, fwrite, memcpy, memset,
  memmove, memcmp, strncpy, _read, _write, ReadFile, WriteFile), with their
  arguments read from the pushes before the call (the /Od code pushes every
  argument right before the call);
- /Oi's inline memset/memcpy: `rep stos`/`rep movs` with a constant count;
- as warnings only: array indexing (`[index*scale + global]`) of a global the
  original has more room for, before the next known object, than the
  recompiled declaration. The room is often data nobody has annotated yet, so
  these need a look rather than fail the check.
It also checks every annotated global's recompiled size against the room the
original has for it before the next known address: a declaration larger than
that overlaps its neighbour, so code reaching its last elements reads or
writes the neighbour in the original and something else in the recompiled
build (the modifier bits kept in the last word of the key states, a NULL
terminator added to a table the original ends without one).

tools/check_block_sizes.py checks the same calls in the source; this tool also
covers calls whose size the source computes (`sizeof` of the wrong object).
"""

import argparse
import bisect
import logging
import sys

import capstone
from capstone import x86

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

# Block functions: (argument count, pointer argument indices, size argument
# indices). The size is the product of the size arguments.
BLOCK_FUNCTIONS = {
    "memcpy": (3, (0, 1), (2,)),
    "memmove": (3, (0, 1), (2,)),
    "memset": (3, (0,), (2,)),
    "memcmp": (3, (0, 1), (2,)),
    "strncpy": (3, (0,), (2,)),
    "fread": (4, (0,), (1, 2)),
    "fwrite": (4, (0,), (1, 2)),
    "_read": (3, (1,), (2,)),
    "_write": (3, (1,), (2,)),
    "ReadFile": (5, (1,), (2,)),
    "WriteFile": (5, (1,), (2,)),
}

VARIABLE_TYPES = (EntityType.DATA, EntityType.POINTER)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--verbose", action="store_true", help="Also list the checked block operations")
    argparse_add_project_target_args(parser)
    return parser.parse_args()


def plain_name(name: str | None) -> str | None:
    """`_fread`, `KERNEL32.dll::ReadFile`, `_ReadFile@20` -> the C name."""
    if name is None:
        return None
    name = name.split("::")[-1]
    if name.startswith("__imp_"):
        name = name[len("__imp_"):]
    name = name.split("@")[0]
    while name.startswith("_") and name.lstrip("_") in BLOCK_FUNCTIONS:
        name = name[1:]
    return name


class Layout:
    """The known objects of the original, by address, with their recompiled size."""

    def __init__(self, compare: Compare):
        self.by_addr = {}
        self.variables = []
        for entity in compare.get_all():
            if entity.orig_addr is None:
                continue
            self.by_addr[entity.orig_addr] = entity
            if entity.entity_type in VARIABLE_TYPES and entity.matched and entity.size(ImageId.RECOMP):
                self.variables.append(entity)
        self.variables.sort(key=lambda e: e.orig_addr)
        self.starts = [e.orig_addr for e in self.variables]
        # Every known address (strings and imports included) and the section
        # ends bound the room the original has after an object.
        self.all_starts = sorted(self.by_addr)
        self.section_ends = sorted(r.stop for r in compare.orig_bin.vaddr_ranges)

    def orig_room(self, entity) -> int:
        """Bytes from an object's start to the next known address or section end."""
        start = entity.orig_addr
        i = bisect.bisect_right(self.all_starts, start)
        limit = self.all_starts[i] if i < len(self.all_starts) else start + entity.size(ImageId.RECOMP)
        j = bisect.bisect_right(self.section_ends, start)
        if j < len(self.section_ends):
            limit = min(limit, self.section_ends[j])
        return limit - start

    def variable_at(self, addr: int):
        """The known variable at or before addr, if addr lies in its original room."""
        i = bisect.bisect_right(self.starts, addr) - 1
        if i < 0:
            return None
        entity = self.variables[i]
        if addr >= entity.orig_addr + max(self.orig_room(entity), entity.size(ImageId.RECOMP)):
            return None
        return entity

    def room(self, addr: int):
        """(entity, bytes the recompiled object has from addr), or None when addr is
        not inside a known object. Past the end of one, the original's data is not
        modeled yet (not annotated), so there is nothing to compare against."""
        entity = self.variable_at(addr)
        if entity is None or addr >= entity.orig_addr + entity.size(ImageId.RECOMP):
            return None
        return entity, entity.size(ImageId.RECOMP) - (addr - entity.orig_addr)


def describe(entity, addr: int) -> str:
    offset = addr - entity.orig_addr
    return entity.name + ("+%#x" % offset if offset else "")


def check_overlaps(layout, findings, checked):
    """Each annotated global's recompiled size against the original's room for it."""
    for entity in layout.variables:
        size = entity.size(ImageId.RECOMP)
        room = layout.orig_room(entity)
        checked.append("%#x: %s: %#x bytes declared, %#x available" % (entity.orig_addr, entity.name, size, room))
        if size > room:
            neighbour = layout.by_addr.get(entity.orig_addr + room)
            after = neighbour.name if neighbour is not None else "the end of the section"
            findings.append(
                "%#x: %s is declared %#x bytes, but the original has %#x before %s"
                % (entity.orig_addr, entity.name, size, room, after)
            )


def scan_function(md, orig_bin, layout, function, findings, warnings, checked):
    # The original's function sizes are unknown (no PDB): take the recompiled
    # size, which a match makes equal, capped by the next known address. The
    # distance to the next known address alone can run into code nobody has
    # annotated yet.
    start = function.orig_addr
    size = function.size(ImageId.RECOMP) or 0
    if function.get("orig_max_size"):
        size = min(size, function.get("orig_max_size"))
    if not size:
        return
    code = orig_bin.read(start, size)

    regs: dict[int, int | None] = {}
    pushes: list[int | None] = []

    def value_of(op):
        if op.type == x86.X86_OP_IMM:
            return op.imm
        if op.type == x86.X86_OP_REG:
            return regs.get(op.reg)
        return None

    for ins in md.disasm(code, start):
        mnemonic = ins.mnemonic

        # Array-style indexing of a global: [index*scale + global].
        for op in ins.operands:
            if op.type != x86.X86_OP_MEM or op.mem.index == 0 or op.mem.base != 0:
                continue
            entity = layout.variable_at(op.mem.disp)
            if entity is None or mnemonic == "lea":
                continue
            recomp_size = entity.size(ImageId.RECOMP)
            orig_room = layout.orig_room(entity)
            if op.mem.disp - entity.orig_addr < recomp_size and orig_room >= recomp_size + max(op.mem.scale, 1) * 2:
                warnings.append(
                    "%#x: indexes %s (%s) like an array: the original has %#x bytes before the next known "
                    "object, the recompiled object %#x"
                    % (ins.address, describe(entity, op.mem.disp), ins.op_str, orig_room, recomp_size)
                )

        if mnemonic == "push":
            pushes.append(value_of(ins.operands[0]))
            continue

        if mnemonic == "call":
            op = ins.operands[0]
            target = None
            if op.type == x86.X86_OP_IMM:
                target = layout.by_addr.get(op.imm)
            elif op.type == x86.X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0:
                target = layout.by_addr.get(op.mem.disp)
            name = plain_name(target.name if target is not None else None)
            if name in BLOCK_FUNCTIONS:
                count, pointer_args, size_args = BLOCK_FUNCTIONS[name]
                if len(pushes) >= count:
                    args = list(reversed(pushes[-count:]))
                    check_block(ins.address, name, args, pointer_args, size_args, layout, findings, checked)
            pushes.clear()
            regs.clear()
            continue

        if mnemonic in ("rep stosd", "rep stosb", "rep movsd", "rep movsb"):
            count = regs.get(x86.X86_REG_ECX)
            if count is not None:
                scale = 4 if mnemonic.endswith("d") else 1
                args = [regs.get(x86.X86_REG_EDI), regs.get(x86.X86_REG_ESI), count * scale]
                pointer_args = (0, 1) if "movs" in mnemonic else (0,)
                check_block(ins.address, "inline " + mnemonic.split()[1], args, pointer_args, (2,), layout,
                            findings, checked)

        # Track constants and addresses loaded into registers.
        if mnemonic == "mov" and len(ins.operands) == 2 and ins.operands[0].type == x86.X86_OP_REG:
            dst, src = ins.operands
            if src.type == x86.X86_OP_IMM:
                regs[dst.reg] = src.imm
                continue
            if src.type == x86.X86_OP_REG:
                regs[dst.reg] = regs.get(src.reg)
                continue
        if mnemonic == "lea" and ins.operands[1].mem.base == 0 and ins.operands[1].mem.index == 0:
            regs[ins.operands[0].reg] = ins.operands[1].mem.disp
            continue
        _, written = ins.regs_access()
        for reg in written:
            regs[reg] = None
            # A write to a 32-bit register's low part changes it too.
            for full, parts in FULL_REGISTERS.items():
                if reg in parts:
                    regs[full] = None


FULL_REGISTERS = {
    x86.X86_REG_EAX: (x86.X86_REG_AX, x86.X86_REG_AL, x86.X86_REG_AH),
    x86.X86_REG_EBX: (x86.X86_REG_BX, x86.X86_REG_BL, x86.X86_REG_BH),
    x86.X86_REG_ECX: (x86.X86_REG_CX, x86.X86_REG_CL, x86.X86_REG_CH),
    x86.X86_REG_EDX: (x86.X86_REG_DX, x86.X86_REG_DL, x86.X86_REG_DH),
    x86.X86_REG_ESI: (x86.X86_REG_SI,),
    x86.X86_REG_EDI: (x86.X86_REG_DI,),
}


def check_block(addr, name, args, pointer_args, size_args, layout, findings, checked):
    size = 1
    for i in size_args:
        if args[i] is None:
            return
        size *= args[i]
    if size <= 0:
        return
    for i in pointer_args:
        pointer = args[i]
        if pointer is None:
            continue
        room = layout.room(pointer)
        if room is None:
            continue
        entity, available = room
        if len(pointer_args) == 1:
            role = "buffer"
        else:
            role = "destination" if i == pointer_args[0] else "source"
        checked.append("%#x: %s(%s %s, %#x): %#x bytes available" % (addr, name, role, describe(entity, pointer),
                                                                     size, available))
        if size > available:
            findings.append(
                "%#x: %s uses %#x bytes through its %s %s, but the recompiled object has %#x bytes from there"
                % (addr, name, size, role, describe(entity, pointer), available)
            )


def main():
    args = parse_args()
    try:
        target = argparse_parse_project_target(args=args)
    except RecCmpProjectException as e:
        logger.error(e.args[0])
        return 1

    print(f"Checking block operations against the recompiled layout: {target.target_id}")

    compare = Compare.from_target(target)
    layout = Layout(compare)

    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True

    findings: list[str] = []
    warnings: list[str] = []
    checked: list[str] = []
    overlaps: list[str] = []
    check_overlaps(layout, findings, overlaps)
    for entity in layout.by_addr.values():
        if entity.entity_type != EntityType.FUNCTION or entity.get("library") or entity.get("stub"):
            continue
        scan_function(md, compare.orig_bin, layout, entity, findings, warnings, checked)

    if args.verbose:
        for line in sorted(overlaps):
            print("  sized " + line)
        for line in sorted(checked):
            print("  checked " + line)
    for line in sorted(set(warnings)):
        print("warning: " + line)
    for line in sorted(findings):
        print(line)
    print(
        "%d globals sized, %d block operations checked, %d problems; %d array accesses to review"
        % (len(overlaps), len(checked), len(findings), len(set(warnings)))
    )
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
