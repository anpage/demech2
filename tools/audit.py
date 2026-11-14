#!/usr/bin/env python3
"""Classify what the witnesses don't prove: constructively, or not at all.

`tools/certify.py` reports a function unproven when neither its source nor a witness compiles to
the original's bytes. Some of those still differ only by compiler entropy that no witness reached.
This tool says so only constructively: a function is `classified` when a composition of the
transformations below, applied to the rebuilt function, gives the original's code instruction for
instruction. Each kind is a rewrite that keeps what the code computes:

  slots        a one-to-one map of the named locals' stack homes (whole objects: a local the
               original keeps in two places is not one)
  mirror       a comparison with its operands swapped and its condition mirrored (jl/jg, setb/seta,
               the loads that feed it swapped with it); x87: the operands of fcom and the AH test
               (the forms the compiler gives a C comparison and its mirror, a <= b and b >= a,
               which differ only for unordered operands, where C doesn't tell them apart either)
  commutative  the operands of add, imul, and, or, xor swapped (with the loads that feed them);
               x87: fadd and fmul, and fsub/fsubr, fdiv/fdivr with theirs

Each rewrite also checks that what it changes is dead afterwards: a register the swapped loads
leave holding the other value, and the flags a mirrored comparison sets (written again before
they're read, conservatively within the straight line). No kind changes an operation's width or signedness, a constant, a displacement of a member, a
referenced global or function, or the number of instructions. At /Od those kinds are all the
entropy there is (CLAUDE.md, "Codegen Patterns"): an instruction moved, merged, added or missing is
a source difference, and so is everything this tool can't reproduce. Optimized code (/Ox) has
more (registers, scheduling), which this tool doesn't reproduce yet: its functions are
`unexplained` unless the kinds above suffice.

What remains `unexplained` is either a difference to fix or one to record in
`tools/known_diffs.txt`: a line per target and function, with a hash of the function's rebuilt and
original code (so another diff of the same function no longer matches it) and the reason. With
`--check`, the tool fails on an unexplained function without a matching line, on a line whose hash
no longer matches, and on a line whose function is proven or classified now (remove it). The reason
`undiagnosed` marks a line nobody has looked at: it keeps the unexplained weight.

Costs (summed per target): 0 byte-identical, 1 a witness by padding, 2 one with renamed locals, 25
classified, 250 a known difference with a reason, 1000 unexplained or undiagnosed.

The corpus (`tools/audit_corpus.txt`, `--corpus`) holds real source differences that once passed for
entropy: a revision, a target, a function at that revision, and the verdict the tool must give
there (`unexplained`). `--corpus` compiles each revision (git archive) with the build's commands
and fails when the verdict differs: a classifier that calls a real difference entropy is wrong.
`--find-corpus REV TARGET` lists candidates: the functions the commit REV turned from unproven into
byte-identical or witnessed, with the verdict at REV^.

usage (from the repository root, with reccmp's environment):
  python tools/audit.py --target MW2 [--build build-wine] [--wine PATH] [--function NAME ...]
                        [--check | --write] [--json FILE] [--markdown FILE] [-v]
  python tools/audit.py --target MW2 --objects DIR ...   (objects and listings from certify.py)
  python tools/audit.py --corpus [--wine PATH]
  python tools/audit.py --find-corpus REV --target MW23DFX [--wine PATH]
"""

import argparse
import hashlib
import json
import os
import struct
import sys
from collections import Counter
from pathlib import Path

from capstone import CS_ARCH_X86, CS_GRP_CALL, CS_GRP_JUMP, CS_GRP_RET, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

sys.path.insert(0, str(Path(__file__).resolve().parent))
from certify import COST, Certify, _Unit, unit_annotations  # noqa: E402
from funccmp import ends, read_yaml_paths  # noqa: E402
from unitbuild import IMAGE_SYM_CLASS_EXTERNAL, NUMBERED, ROOT, TARGETS, Image, Object  # noqa: E402
from witness import read_listing, slot_plan  # noqa: E402

KNOWN = ROOT / "tools" / "known_diffs.txt"
WEIGHTS = dict(COST, classified=25, known=250, unexplained=1000)

DISASSEMBLER = Cs(CS_ARCH_X86, CS_MODE_32)
DISASSEMBLER.detail = True

MIRROR = {"jl": "jg", "jg": "jl", "jle": "jge", "jge": "jle", "jb": "ja", "ja": "jb", "jbe": "jae", "jae": "jbe",
          "je": "je", "jne": "jne"}
MIRROR.update({"set" + k[1:]: "set" + v[1:] for k, v in list(MIRROR.items())})
COMMUTATIVE = {"add", "imul", "and", "or", "xor"}
X87_COMMUTATIVE = {"fadd": "fadd", "fmul": "fmul", "fsub": "fsubr", "fsubr": "fsub", "fdiv": "fdivr", "fdivr": "fdiv",
                   "faddp": "faddp", "fmulp": "fmulp", "fsubp": "fsubrp", "fsubrp": "fsubp", "fdivp": "fdivrp", "fdivrp": "fdivp"}
# The AH tests after fnstsw that read a comparison, and the test of the same comparison mirrored:
# st < src (C0) is src > st, st <= src (C0|C3) is src >= st; equality reads the same both ways.
X87_MIRROR = {(0x01, "jne"): (0x41, "je"), (0x41, "je"): (0x01, "jne"), (0x01, "je"): (0x41, "jne"),
              (0x41, "jne"): (0x01, "je"), (0x40, "jne"): (0x40, "jne"), (0x40, "je"): (0x40, "je"),
              (0x05, "jp"): (0x45, "jnp"), (0x45, "jnp"): (0x05, "jp"), (0x05, "jnp"): (0x45, "jp"),
              (0x45, "jp"): (0x05, "jnp"), (0x44, "jp"): (0x44, "jp"), (0x44, "jnp"): (0x44, "jnp")}


# ---------------------------------------------------------------------------------------------
# Lifting: both sides into the same instruction form


class Ins:
    """One instruction (or a table of data inside the function). `ops`: tuples comparable across
    the two sides: ("reg", name), ("imm", value), ("mem", segment, base, index, scale, disp, size),
    where a relocated value is ("addr", address), ("data", bytes) or ("label", position)."""

    def __init__(self, offset, mnemonic, ops, text, kind="code", reads=(), writes=()):
        self.offset, self.mnemonic, self.ops, self.text, self.kind = offset, mnemonic, ops, text, kind
        self.reads, self.writes = set(reads), set(writes)  # register families (eax for al), "flags"

    def __repr__(self):
        return "%s %s" % (self.mnemonic, self.text)


FAMILIES = {}
for _wide, _names in {"eax": "al ah ax", "ecx": "cl ch cx", "edx": "dl dh dx", "ebx": "bl bh bx", "esi": "si", "edi": "di",
                      "ebp": "bp", "esp": "sp"}.items():
    for _name in _names.split():
        FAMILIES[_name] = _wide


def family(name):
    """A register's 32-bit family; the flags register as "flags"."""
    return "flags" if name in ("eflags", "flags") else FAMILIES.get(name, name)


def dead_after(code, position, name):
    """Whether a register (or "flags") is dead after `position`: written before it is read, before
    anything leaves the straight line (a branch, a call that may read it, a return). Conservative."""
    for ins in code[position + 1 :]:
        if ins.kind != "code":
            return False
        if name in ins.reads:
            return False
        if ins.mnemonic == "call":
            return name in ("eax", "flags")  # a call clobbers eax and the flags; /Gr passes ecx, edx
        if name in ins.writes:
            return True
        if ins.mnemonic.startswith(("j", "ret", "loop")):
            return False
    return False


def register(x, reg):
    return x.reg_name(reg) if reg else None


def lift(code, start, relocate, table_at, extent):
    """The instructions of a function's code, decoded from offset 0: `relocate(offset)` gives the
    value of a relocated field (or None), `table_at(offset)` the entries of a jump table that starts
    there (or None). Decoding stops after a terminator with no branch target beyond it."""
    out, offset, targets = [], 0, set()
    while offset < len(code) and offset < extent:
        entries = table_at(offset)
        if entries is not None:
            out.append(Ins(offset, "table", [("label", e) for e in entries], "", "table"))
            targets.update(entries)
            offset += 4 * len(entries)
            later = [t for t in targets if t >= offset]
            resume = min(later) if later else offset
            if resume > offset:
                out.append(Ins(offset, "bytes", [("bytes", bytes(code[offset:resume]))], "", "bytes"))
                offset = resume
            continue
        x = next(DISASSEMBLER.disasm(bytes(code[offset : offset + 16]), offset, 1), None)
        if x is None:
            # Data in the code (a MASM table): the rest of the function, as bytes.
            out.append(Ins(offset, "bytes", [("bytes", bytes(code[offset : min(len(code), extent)]).rstrip(b"\xcc"))], "", "bytes"))
            break
        ops = []
        e = x.encoding
        branch = x.group(CS_GRP_JUMP) or x.group(CS_GRP_CALL)
        for op in x.operands:
            if op.type == X86_OP_REG:
                ops.append(("reg", x.reg_name(op.reg)))
            elif op.type == X86_OP_IMM:
                value = relocate(offset + e.imm_offset) if e.imm_size == 4 else None
                jump = x.group(CS_GRP_JUMP)
                if value is not None and value[0] == "label" and jump:
                    targets.add(value[1])
                if value is None and branch:
                    value = ("label", op.imm)
                    if jump:
                        targets.add(op.imm)  # a call's target is another function, not more of this one
                ops.append(value if value is not None else ("imm", op.imm))
            elif op.type == X86_OP_MEM:
                disp = relocate(offset + e.disp_offset) if e.disp_size == 4 else None
                if disp is None:
                    disp = op.mem.disp
                ops.append(("mem", register(x, op.mem.segment), register(x, op.mem.base), register(x, op.mem.index), op.mem.scale, disp, op.size))
        read, written = x.regs_access()
        reads = {family(x.reg_name(r)) for r in read}
        writes = {family(x.reg_name(r)) for r in written}
        out.append(Ins(offset, x.mnemonic, ops, x.op_str, "code", reads, writes))
        offset += x.size
        if (x.group(CS_GRP_RET) or x.mnemonic == "jmp") and not any(t >= offset for t in targets):
            break
    return out


def label_positions(instructions, base):
    """The instructions with branch targets turned from offsets into instruction positions. A
    target outside the decoded function (another routine of the object) becomes the address it has
    in the original's layout, `base` + offset, on both sides."""
    position = {ins.offset: k for k, ins in enumerate(instructions)}

    def place(offset):
        return ("label", position[offset]) if offset in position else ("addr", (base + offset) & 0xFFFFFFFF)

    def fix(op):
        if op[0] == "label":
            return place(op[1])
        if op[0] == "mem" and isinstance(op[5], tuple) and op[5][0] == "label":
            return op[:5] + (place(op[5][1]),) + op[6:]
        return op

    for ins in instructions:
        ins.ops = [fix(op) for op in ins.ops]
    return instructions


class Lifted:
    """A rebuilt function and the original's, lifted."""

    def __init__(self, function, obj, image, address, annotations, next_address):
        self.unverified = []
        self.image = image
        relocations = function.relocations

        def ours_value(at):
            if at not in relocations:
                return None
            symbol, kind = relocations[at]
            addend = struct.unpack("<i", bytes(function.code[at : at + 4]))[0]
            if symbol.section == function.symbol.section and symbol.section > 0:
                # Both DIR32 and REL32 name the address symbol + addend (funccmp.py).
                offset = symbol.value + addend - function.start
                if 0 <= offset <= len(function.code) and (symbol.storage != IMAGE_SYM_CLASS_EXTERNAL or symbol is function.symbol):
                    return ("label", offset)
            target = annotations.address_of(symbol.name)
            if target is not None:
                return ("addr", (target + addend) & 0xFFFFFFFF)
            data = obj.data_at(symbol, addend) if symbol.section > 0 else None
            if data:
                return ("data", bytes(data))
            self.unverified.append(symbol.name)
            return ("unknown", symbol.name)

        def ours_table(at):
            if at not in relocations or not self.at_table(function.code, relocations, at):
                return None
            entries = []
            while at in relocations:
                entries.append(ours_value(at)[1])
                at += 4
            return entries

        code = function.code
        ours = lift(code, 0, ours_value, ours_table, len(code))
        # What the rebuilt function has after its last instruction (data a MASM routine keeps in
        # its code) is compared as bytes, at the same place after the original's last instruction.
        end = ours[-1].offset + self.size_of(ours[-1], code) if ours else 0
        tail = bytes(code[end:]).rstrip(b"\xcc")
        if tail:
            ours.append(Ins(end, "bytes", [("bytes", tail)], "", "bytes"))
        self.ours = label_positions(ours, address)
        extent = (next_address - address) if next_address else 0x10000
        original = image.at(address, extent)
        tables = {}

        def theirs_value(at):
            return None

        def theirs_table(at):
            return tables.get(at)

        # The original's jump tables: the `jmp [reg*4 + table]` operands inside the function.
        self.theirs = None
        for _ in range(4):  # each pass knows the tables the one before found
            lifted = lift(original, 0, theirs_value, theirs_table, extent)
            found = {}
            for ins in lifted:
                if ins.mnemonic == "jmp" and ins.ops and ins.ops[0][0] == "mem" and ins.ops[0][4] == 4:
                    disp = ins.ops[0][5]
                    if isinstance(disp, int) and address <= disp < address + extent:
                        at, entries = disp - address, []
                        while at + 4 <= extent:
                            value = struct.unpack("<I", original[at : at + 4])[0] - address
                            if not 0 <= value < extent:
                                break
                            entries.append(value)
                            at += 4
                        found[disp - address] = entries
            if found == tables:
                break
            tables = found
        if tail and lifted:
            end = lifted[-1].offset + self.size_of(lifted[-1], original)
            lifted.append(Ins(end, "bytes", [("bytes", bytes(original[end : end + len(tail)]))], "", "bytes"))
        # The original must end where its last instruction does (funccmp.py's rule).
        if lifted and lifted[-1].kind == "code":
            end = lifted[-1].offset + self.size_of(lifted[-1], original)
            if end < extent and not ends(original[end : end + 16]):
                lifted.append(Ins(end, "(the original goes on)", [("bytes", bytes(original[end : end + 4]))], "", "bytes"))
        # The original's addresses: a branch inside it is a label, any other value an address.
        for ins in lifted:
            ins.ops = [self.theirs_operand(op, address, extent) for op in ins.ops]
        self.theirs = label_positions(lifted, address)
        self.address = address

    @staticmethod
    def size_of(ins, code):
        if ins.kind == "table":
            return 4 * len(ins.ops)
        if ins.kind == "bytes":
            return len(ins.ops[0][1])
        x = next(DISASSEMBLER.disasm(bytes(code[ins.offset : ins.offset + 16]), ins.offset, 1), None)
        return x.size if x else 1

    @staticmethod
    def at_table(code, relocations, at):
        """Whether a relocation at an instruction boundary starts a jump table (an instruction
        can't have a relocated field at its first byte)."""
        return at in relocations

    def theirs_operand(self, op, address, extent):
        if op[0] == "label":
            return op  # label_positions places it
        if op[0] == "mem" and isinstance(op[5], int) and address <= op[5] < address + extent:
            return op[:5] + (("label", op[5] - address),) + op[6:]  # a jump table inside the function
        if op[0] == "mem" and isinstance(op[5], int) and op[2] is None and op[5] >= 0x10000:
            return op[:5] + (("addr", op[5] & 0xFFFFFFFF),) + op[6:]
        if op[0] == "imm" and op[1] >= 0x10000:
            return ("imm", op[1])
        return op


# ---------------------------------------------------------------------------------------------
# Equality


def same_value(a, b, image):
    """Our operand value against the original's."""
    if a == b:
        return True
    if isinstance(a, tuple) and a[0] == "addr":
        value = b[1] if isinstance(b, tuple) and b[0] in ("addr", "imm") else b
        return isinstance(value, int) and (a[1] == value or thunk(image, value) == a[1])
    if isinstance(a, tuple) and a[0] == "data":
        value = b[1] if isinstance(b, tuple) and b[0] in ("addr", "imm") else b
        return isinstance(value, int) and image.at(value, len(a[1])) == a[1]
    if isinstance(a, tuple) and a[0] == "unknown":
        return True
    return False


def thunk(image, address):
    data = image.at(address, 5)
    if data[:1] != b"\xe9":
        return None
    return (address + 5 + struct.unpack("<i", data[1:5])[0]) & 0xFFFFFFFF


def same_operand(a, b, image):
    if a[0] == "mem" and b[0] == "mem":
        return a[1:5] == b[1:5] and a[6] == b[6] and same_value(a[5], b[5], image)
    if a[0] in ("addr", "data", "unknown"):
        return same_value(a, b, image)
    if a[0] == "imm" and b[0] in ("imm", "addr"):
        return a[1] == b[1]
    return a == b


def same(a, b, image):
    if a.mnemonic != b.mnemonic or len(a.ops) != len(b.ops):
        return False
    return all(same_operand(x, y, image) for x, y in zip(a.ops, b.ops))


# ---------------------------------------------------------------------------------------------
# The kinds


def is_slot(op):
    return op[0] == "mem" and op[2] == "ebp" and op[3] is None and isinstance(op[5], int) and op[5] < 0


def apply_slots(ours, theirs, local_list, image):
    """Rewrite our [ebp-N] by a one-to-one map of whole locals, if one maps us onto the original."""
    pairs = []
    for a, b in zip(ours, theirs):
        if a.mnemonic != b.mnemonic or len(a.ops) != len(b.ops):
            continue
        for x, y in zip(a.ops, b.ops):
            if is_slot(x) and is_slot(y):
                pairs.append((x[5], y[5]))
    if not any(x != y for x, y in pairs):
        return None
    order, reason = slot_plan(local_list, pairs)
    if order is None:
        return None
    named = sorted((offset, name) for name, offset, _ in local_list if offset < 0)
    starts = [offset for offset, _ in named]
    shift = {}
    for ours_disp, theirs_disp in pairs:
        if ours_disp < starts[0]:
            continue
        k = max(i for i, s in enumerate(starts) if s <= ours_disp)
        base = theirs_disp - (ours_disp - starts[k])
        if shift.setdefault(k, base - starts[k]) != base - starts[k]:
            return None
    if not shift:
        return None

    def remap(op):
        if not is_slot(op) or op[5] < starts[0]:
            return op
        k = max(i for i, s in enumerate(starts) if s <= op[5])
        return op[:5] + (op[5] + shift.get(k, 0),) + op[6:]

    return [Ins(i.offset, i.mnemonic, [remap(op) for op in i.ops], i.text, i.kind) for i in ours]


def copy(ins, mnemonic=None, ops=None):
    return Ins(ins.offset, mnemonic or ins.mnemonic, list(ins.ops if ops is None else ops), ins.text, ins.kind, ins.reads, ins.writes)


def rewrites(ours, k):
    """Candidate rewrites of our code around position k: (kind, first position, new instructions)."""
    out = []
    n = len(ours)
    # A comparison and the condition that reads it, mirrored.
    for c in (k - 1, k, k + 1):
        if not 0 <= c < n or ours[c].mnemonic != "cmp" or c + 1 >= n:
            continue
        reader = ours[c + 1]
        if reader.mnemonic not in MIRROR or not dead_after(ours, c + 1, "flags"):
            continue
        cmp, mirrored = ours[c], copy(reader, MIRROR[reader.mnemonic])
        a, b = cmp.ops
        if a[0] != "imm" and b[0] != "imm" or a[0] == "reg":
            # cmp a, b becomes cmp b, a (one of them a register: the encodings allow both)
            if a[0] == "reg" or b[0] == "reg":
                out.append(("mirror", c, [copy(cmp, ops=[b, a]), mirrored]))
        if c >= 1 and ours[c - 1].mnemonic in ("mov", "movsx", "movzx") and ours[c - 1].ops[0] == a and b[0] in ("mem", "reg", "imm", "addr") \
                and a[0] == "reg" and dead_after(ours, c + 1, family(a[1])):
            load = ours[c - 1]
            if b[0] == "mem" and load.ops[1][0] == "mem" and b[6] == load.ops[1][6] or b[0] != "mem" and load.mnemonic == "mov":
                out.append(("mirror", c - 1, [copy(load, ops=[a, b]), copy(cmp, ops=[a, load.ops[1]]), mirrored]))
    # A commutative operation and the load that feeds it.
    for c in (k - 1, k, k + 1):
        if not 1 <= c < n:
            continue
        op, load = ours[c], ours[c - 1]
        if op.mnemonic in COMMUTATIVE and len(op.ops) == 2 and load.mnemonic == "mov" and load.ops[0] == op.ops[0]:
            source, other = load.ops[1], op.ops[1]
            if source[0] == "mem" and other[0] == "mem" and source[6] != other[6]:
                continue
            if source[0] == "mem" and other[0] == "mem":
                continue  # two memory operands can't both be the op's
            out.append(("commutative", c - 1, [copy(load, ops=[load.ops[0], other]), copy(op, ops=[op.ops[0], source])]))
        if c >= 2 and op.mnemonic in COMMUTATIVE and len(op.ops) == 2 and op.ops[1][0] == "reg":
            first, second = ours[c - 2], ours[c - 1]
            if first.mnemonic == second.mnemonic == "mov" and {first.ops[0], second.ops[0]} == {op.ops[0], op.ops[1]} \
                    and dead_after(ours, c, family(op.ops[1][1])):
                out.append(("commutative", c - 2, [copy(first, ops=[first.ops[0], second.ops[1]]), copy(second, ops=[second.ops[0], first.ops[1]]), op]))
    # x87: fld a; fop b  becomes  fld b; fop' a.
    for c in (k - 1, k, k + 1):
        if not 1 <= c < n:
            continue
        op, load = ours[c], ours[c - 1]
        if op.mnemonic in X87_COMMUTATIVE and load.mnemonic in ("fld", "fild") and len(op.ops) == 1 and op.ops[0][0] == "mem":
            if load.mnemonic == "fld" and load.ops[0][0] == "mem" and load.ops[0][6] == op.ops[0][6]:
                out.append(("commutative", c - 1, [copy(load, ops=[op.ops[0]]), copy(op, X87_COMMUTATIVE[op.mnemonic], [load.ops[0]])]))
        if c >= 2 and op.mnemonic in X87_COMMUTATIVE and op.mnemonic.endswith("p") and ours[c - 2].mnemonic == ours[c - 1].mnemonic == "fld":
            out.append(("commutative", c - 2, [ours[c - 1], ours[c - 2], copy(op, X87_COMMUTATIVE[op.mnemonic])]))
    # x87 comparisons: fld a; fcomp b; fnstsw ax; test ah, m; jcc.
    for c in range(max(1, k - 4), min(n - 3, k + 2)):
        load, cmp, status, test = ours[c - 1], ours[c], ours[c + 1], ours[c + 2]
        jcc = ours[c + 3] if c + 3 < n else None
        if load.mnemonic != "fld" or cmp.mnemonic not in ("fcomp", "fcom") or status.mnemonic != "fnstsw" or test.mnemonic != "test" or jcc is None:
            continue
        if len(cmp.ops) != 1 or cmp.ops[0][0] != "mem" or load.ops[0][0] != "mem" or load.ops[0][6] != cmp.ops[0][6]:
            continue
        key = (test.ops[1][1] if test.ops[1][0] == "imm" else None, jcc.mnemonic)
        if key in X87_MIRROR and dead_after(ours, c + 3, "flags") and dead_after(ours, c + 3, "eax"):
            mask, condition = X87_MIRROR[key]
            out.append(("mirror", c - 1, [copy(load, ops=[cmp.ops[0]]), copy(cmp, ops=[load.ops[0]]), status,
                                         copy(test, ops=[test.ops[0], ("imm", mask)]), copy(jcc, condition)]))
    return out


def classify(lifted, local_list, optimized_unit, steps=None):
    """(kinds Counter, None) when the kinds reproduce the original, else (kinds so far, the first
    remaining difference). `steps` collects each rewrite: (kind, position, before, after)."""
    ours, theirs, image = lifted.ours, lifted.theirs, lifted.image
    kinds = Counter()
    steps = [] if steps is None else steps
    if len(ours) != len(theirs):
        return kinds, inserted(ours, theirs)
    if not optimized_unit:
        remapped = apply_slots(ours, theirs, local_list, image)
        if remapped is not None:
            ours = remapped
            kinds["slots"] += 1
    for _ in range(len(ours) + 1):
        bad = [k for k, (a, b) in enumerate(zip(ours, theirs)) if not same(a, b, image)]
        if not bad:
            return kinds, None
        k = bad[0]
        for kind, first, new in rewrites(ours, k):
            window = range(first, first + len(new))
            if all(same(new[i - first], theirs[i], image) for i in window):
                steps.append((kind, first, [show(i) for i in ours[first : first + len(new)]], [show(i) for i in new]))
                ours = ours[:first] + new + ours[first + len(new) :]
                kinds[kind] += 1
                break
        else:
            return kinds, describe(ours, theirs, k)
    return kinds, describe(ours, theirs, 0, "no progress")


def show(ins):
    def op(o):
        if o[0] == "reg":
            return o[1]
        if o[0] == "imm":
            return hex(o[1])
        if o[0] == "mem":
            base = "+".join(x for x in (o[2], o[3] and "%s*%d" % (o[3], o[4])) if x)
            disp = o[5] if isinstance(o[5], int) else "%s:%s" % (o[5][0], hex(o[5][1]) if isinstance(o[5][1], int) else o[5][1])
            return "[%s%s]" % (base + "+" if base else "", disp)
        return "%s:%s" % (o[0], hex(o[1]) if isinstance(o[1], int) else o[1])
    return "%s %s" % (ins.mnemonic, ", ".join(op(o) for o in ins.ops))


def describe(ours, theirs, k, prefix=None):
    a = ours[k] if k < len(ours) else None
    b = theirs[k] if k < len(theirs) else None
    text = "#%d: %s    the original's: %s" % (k, a, b)
    return (prefix + "; " if prefix else "") + text


def skeleton(ins):
    """An instruction with its stack slots and addresses masked: for aligning two versions."""
    def mask(op):
        if op[0] == "mem":
            return ("mem", op[2], op[3], op[6], "slot" if is_slot(op) else "value" if isinstance(op[5], tuple) or abs(op[5]) >= 0x10000 else op[5])
        if op[0] in ("addr", "data", "unknown") or op[0] == "imm" and abs(op[1]) >= 0x10000:
            return ("value",)
        return (op[0],) if op[0] == "label" else op
    return (ins.mnemonic,) + tuple(mask(op) for op in ins.ops)


def inserted(ours, theirs):
    """The first instructions one side has and the other doesn't, by aligning the two."""
    import difflib
    matcher = difflib.SequenceMatcher(None, [skeleton(i) for i in ours], [skeleton(i) for i in theirs], autojunk=False)
    for tag, i1, i2, j1, j2 in matcher.get_opcodes():
        if tag != "equal":
            show = lambda side, a, b: "; ".join(repr(x) for x in side[a : min(b, a + 3)]) or "nothing"
            return "%d instructions, the original's %d; #%d: %s    the original's (#%d): %s" % (
                len(ours), len(theirs), i1, show(ours, i1, i2), j1, show(theirs, j1, j2))
    return "%d instructions, the original's %d" % (len(ours), len(theirs))


def fingerprint(lifted):
    """A hash of both sides' code, as lifted: another diff of the function gives another. The
    compiler's counters in the names of unannotated symbols ($SG123) are left out: they move with
    any declaration ahead of the function."""
    text = repr([(i.mnemonic, i.ops) for i in lifted.ours]) + "|" + repr([(i.mnemonic, i.ops) for i in lifted.theirs])
    text = NUMBERED.sub("$#", text)
    return hashlib.sha1(text.encode()).hexdigest()[:12]


# ---------------------------------------------------------------------------------------------
# known_diffs.txt


def read_known(path=KNOWN):
    known = {}
    if not path.exists():
        return known
    for number, line in enumerate(path.read_text().split("\n"), 1):
        if not line.strip() or line.startswith("#"):
            continue
        target, address, name, digest, *reason = line.split()
        known[(target, int(address, 16))] = {"name": name, "hash": digest, "reason": " ".join(reason), "line": number}
    return known


def write_known(path, target, results, known):
    """Rewrite the target's lines: keep a reason that still applies (same hash), else undiagnosed."""
    lines = [l for l in (path.read_text().split("\n") if path.exists() else []) if l.strip() and not l.startswith("#") and l.split()[0] != target]
    for r in results:
        if r["verdict"] in ("unexplained", "known"):
            old = known.get((target, int(r["address"], 16)))
            reason = old["reason"] if old and old["hash"] == r["hash"] else "undiagnosed"
            lines.append("%s %s %s %s %s" % (target, r["address"], r["name"], r["hash"], reason))
    header = ["# Functions that are neither byte-identical, proven by a witness nor classified (tools/audit.py),",
              "# and why: target, address, function, a hash of both sides' code, the reason. `undiagnosed` marks",
              "# a line nobody has looked at yet. tools/audit.py --check fails on an unexplained function",
              "# without a line, on a hash that no longer matches, and on a line whose function no longer needs it.", ""]
    order = {t: k for k, t in enumerate(TARGETS)}
    lines.sort(key=lambda l: (order.get(l.split()[0], 99), l.split()[1]))
    path.write_text("\n".join(header + lines) + "\n")


# ---------------------------------------------------------------------------------------------


def audit_tree(target, tree, build, wine, jobs, scratch, names=None):
    """{function name: (state, verdict, difference)} of a target compiled from `tree`."""
    certify = Certify(target, ROOT / build, wine, jobs, tree)
    certify.records = []  # the revision's own records, if any, are read below
    from certify import read_records
    certify.records = read_records(Path(tree) / "tools" / "witnesses" / (target + ".txt"))
    certify.compile(scratch)
    image = Image(read_yaml_paths(ROOT).get(target))
    states = certify.compare(scratch, image)
    manifest = json.loads((scratch / "manifest.json").read_text())
    out = {}
    for state in states:
        if names and state["name"] not in names:
            continue
        if state["state"] != "unproven":
            out[state["name"]] = (state["state"], state["state"], "")
            continue
        entry = manifest["units"][state["unit"]]
        obj = Object(scratch / entry["obj"])
        listing = read_listing(scratch / entry["asm"]) if entry.get("asm") else {}
        address = int(state["address"], 16)
        symbol = next(s for s, a in unit_annotations(certify.annotations, _Unit(entry), obj, Path(tree)).items() if a.offset == address)
        later = [a for a in certify.annotations.addresses if a > address]
        lifted = Lifted(obj.functions[symbol], obj, image, address, certify.annotations, min(later) if later else None)
        kinds, difference = classify(lifted, listing.get(symbol, []), entry.get("optimized"))
        out[state["name"]] = ("unproven", "classified" if difference is None else "unexplained", difference or "")
    return out


def run_corpus(args):
    from compare_objects import tree_of
    scratch = ROOT / args.build / "audit-corpus"
    groups = {}
    for line in (ROOT / "tools" / "audit_corpus.txt").read_text().split("\n"):
        line = line.split("#")[0].strip()
        if line:
            rev, target, name, expected = line.split()
            groups.setdefault((rev, target), []).append((name, expected))
    failures = 0
    for (rev, target), cases in groups.items():
        tree = tree_of(rev, scratch, "tree-" + rev.replace("^", "p").replace("~", "t"))
        found = audit_tree(target, tree, args.build, args.wine, args.jobs, scratch / ("objects-" + target), {n for n, _ in cases})
        for name, expected in cases:
            state, verdict, difference = found.get(name, ("missing", "missing", ""))
            ok = verdict == expected
            failures += not ok
            print("%-4s %s %s %s: %s (expected %s)%s" % ("ok" if ok else "FAIL", rev, target, name, verdict, expected, "" if ok else ": " + difference))
    print("%d corpus cases, %d failures" % (sum(len(c) for c in groups.values()), failures))
    return 1 if failures else 0


def find_corpus(args):
    """The functions commit REV changed (their rebuilt code differs between REV^ and REV) that are
    proven or classified today, with the verdict at REV^: a fix's parent holds a real difference."""
    from compare_objects import tree_of
    scratch = ROOT / args.build / "audit-corpus"
    today = {f["name"]: f["verdict"] for f in json.loads(Path(args.today).read_text())["functions"]} if args.today else {}
    codes = {}
    for label, rev in (("after", args.find_corpus), ("before", args.find_corpus + "^")):
        tree = tree_of(rev, scratch, label)
        certify = Certify(args.target, ROOT / args.build, args.wine, args.jobs, tree)
        certify.records = []
        certify.compile(scratch / (label + "-objects"))
        manifest = json.loads((scratch / (label + "-objects") / "manifest.json").read_text())
        code = {}
        for unit, entry in manifest["units"].items():
            obj = Object(scratch / (label + "-objects") / entry["obj"])
            for symbol, annotation in unit_annotations(certify.annotations, _Unit(entry), obj, tree).items():
                f = obj.functions[symbol]
                code[Annotations_name(certify, annotation)] = (bytes(f.code), sorted((o, s.canonical) for o, (s, _) in f.relocations.items()), body_tokens(annotation))
        codes[label] = code
    # The commit changed the function's code and its own tokens (not only declarations ahead of
    # it, which move code by entropy).
    # With --declarations (a commit that fixes a type the functions use), any code change.
    changed = {n for n, c in codes["after"].items() if n in codes["before"] and codes["before"][n][:2] != c[:2]
               and (args.declarations or codes["before"][n][2] != c[2])}
    names = {n for n in changed if today.get(n, "exact") in ("exact", "witness", "classified")}
    before = audit_tree(args.target, tree_of(args.find_corpus + "^", scratch, "before"), args.build, args.wine, args.jobs, scratch / "before-audit", names)
    for name in sorted(names):
        state, verdict, difference = before.get(name, ("missing", "missing", ""))
        print("%s^ %s %s %s    # today: %s; %s" % (args.find_corpus, args.target, name, verdict, today.get(name, "?"), difference[:110]))
    return 0


def body_tokens(annotation):
    """The tokens of a function's definition, comments and preprocessor lines left out."""
    from certify import TOKEN
    lines = Path(annotation.filename).read_text(encoding="utf-8").split("\n")[annotation.line_number - 1 : annotation.end_line]
    text = "\n".join(l for l in lines if not l.lstrip().startswith("#"))
    return [t for t in TOKEN.findall(text) if not t.startswith(("//", "/*"))]


def Annotations_name(certify, annotation):
    from funccmp import Annotations as A
    return A.name_of(annotation)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", choices=sorted(TARGETS))
    parser.add_argument("--corpus", action="store_true", help="Check tools/audit_corpus.txt")
    parser.add_argument("--find-corpus", metavar="REV", help="List the functions commit REV fixed, and their verdicts before it")
    parser.add_argument("--today", help="For --find-corpus: an --json of the current tree (which functions are right today)")
    parser.add_argument("--declarations", action="store_true", help="For --find-corpus: the commit fixed a declaration, not the functions' tokens")
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build")
    parser.add_argument("--original")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"))
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--objects", help="certify.py's objects (with listings): no compile")
    parser.add_argument("--tree", default=str(ROOT))
    parser.add_argument("--function", nargs="*")
    parser.add_argument("--check", action="store_true", help="Fail on what tools/known_diffs.txt doesn't cover")
    parser.add_argument("--write", action="store_true", help="Rewrite the target's lines of tools/known_diffs.txt")
    parser.add_argument("--json")
    parser.add_argument("--markdown")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()
    if args.corpus:
        return run_corpus(args)
    if args.find_corpus:
        return find_corpus(args)
    if not args.target:
        parser.error("--target is required")

    tree = Path(args.tree).resolve()
    certify = Certify(args.target, None if args.objects else ROOT / args.build, args.wine, args.jobs, tree)
    objects = Path(args.objects) if args.objects else ROOT / args.build / "certify" / (args.target + "-audit")
    if not args.objects:
        certify.compile(objects, listings=True)
    original = args.original or read_yaml_paths(tree).get(args.target)
    image = Image(original)
    states = certify.compare(objects, image)
    manifest = json.loads((objects / "manifest.json").read_text())
    objs, listings = {}, {}
    for unit_name, entry in manifest["units"].items():
        objs[unit_name] = Object(objects / entry["obj"])
        if entry.get("asm"):
            listings[unit_name] = read_listing(objects / entry["asm"])
    optimized_units = {u for u, e in manifest["units"].items() if e.get("optimized")}
    addresses = certify.annotations.addresses
    results = []
    for state in states:
        result = dict(state)
        if state["state"] != "unproven" or args.function and state["name"] not in args.function:
            result["verdict"] = state["state"]
            results.append(result)
            continue
        obj = objs[state["unit"]]
        address = int(state["address"], 16)
        symbol = next(s for s, a in unit_annotations(certify.annotations, _unit(manifest, state["unit"]), obj, tree).items() if a.offset == address)
        later = [a for a in addresses if a > address]
        lifted = Lifted(obj.functions[symbol], obj, image, address, certify.annotations, min(later) if later else None)
        steps = []
        kinds, difference = classify(lifted, listings.get(state["unit"], {}).get(symbol, []), state["unit"] in optimized_units, steps)
        if steps:
            result["steps"] = steps
        result["hash"] = fingerprint(lifted)
        if difference is None:
            result["verdict"], result["kinds"] = "classified", dict(kinds)
        else:
            result["verdict"], result["difference"] = "unexplained", difference
            if kinds:
                result["kinds"] = dict(kinds)
        results.append(result)

    known_path = tree / "tools" / "known_diffs.txt"  # the checked tree's, like its annotations
    known = read_known(known_path)
    failures = []
    for r in results:
        k = known.get((args.target, int(r["address"], 16)))
        if r["verdict"] == "unexplained" and k:
            if k["hash"] != r["hash"]:
                failures.append("%s %s: its code changed since tools/known_diffs.txt's line %d (rerun --write and review the reason)" % (r["address"], r["name"], k["line"]))
            elif k["reason"] != "undiagnosed":
                r["verdict"], r["reason"] = "known", k["reason"]
            else:
                r["reason"] = "undiagnosed"
        elif r["verdict"] == "unexplained":
            failures.append("%s %s: unexplained, and not in tools/known_diffs.txt" % (r["address"], r["name"]))
        elif k:
            failures.append("%s %s: %s now: remove line %d of tools/known_diffs.txt" % (r["address"], r["name"], r["verdict"], k["line"]))
        # exact and witness keep certify's cost (0; 1 or 2)
        if r["verdict"] in ("classified", "known", "unexplained"):
            r["cost"] = WEIGHTS[r["verdict"]]
    counts = Counter(r["verdict"] for r in results)
    total = sum(r["cost"] for r in results)
    if args.verbose:
        for r in results:
            if r["verdict"] in ("classified", "unexplained", "known"):
                print("%s %-11s %-32s %s" % (r["address"], r["verdict"], r["name"], r.get("difference") or r.get("reason") or " ".join("%s %d" % kv for kv in sorted(r.get("kinds", {}).items()))))
    if args.write:
        write_known(known_path, args.target, results, known)
        print("wrote %s" % known_path)
    elif args.check:
        for failure in failures:
            print(failure)
    print("%s: %d functions: %s; cost %d" % (args.target, len(results), ", ".join("%d %s" % (counts[v], v) for v in ("exact", "witness", "classified", "known", "unexplained") if counts[v]), total))
    if args.json:
        Path(args.json).write_text(json.dumps({"target": args.target, "cost": total, "functions": results}, indent=1) + "\n")
    return 1 if args.check and failures else 0


def _unit(manifest, name):
    return _Unit(manifest["units"][name])


if __name__ == "__main__":
    sys.exit(main())
