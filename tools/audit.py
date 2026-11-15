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
               a chain of one of add, and, or, xor (mov r, a; add r, b; add r, c) in any order of
               its operands, none of which reads r; x87: fadd and fmul, and fsub/fsubr,
               fdiv/fdivr with theirs
  chains       a comparison of two chains of loads (mov eax, [a]; mov eax, [eax*4+g]; mov ecx, [b];
               ... cmp [eax+8], ecx), its sides evaluated in the other order or registers, the
               final dereference moved between a chain and the comparison, the condition mirrored
               with the comparison's operands; or two chains that give an address's base and index
               (p->m_items[i]), evaluated in the other order or registers; the chains only load
               (no stores or calls) and read no register but their own and ebp

Each rewrite also checks that what it changes is dead afterwards: a register the swapped loads
leave holding the other value, and the flags a mirrored comparison sets (written again before
they're read on every path, following the function's jumps; with the callee's decorated name, a
call reads only its register arguments), and that no operand moved into or out of a load reads the
register the load writes. No kind changes an operation's width or
signedness, a constant, a displacement of a member, a referenced global or function, or the number
of instructions. At /Od those kinds are all the entropy there is (CLAUDE.md, "Codegen Patterns"):
an instruction moved, merged, added or missing is a source difference, and so is everything this
tool can't reproduce.

Optimized code (/Ox units) has more, and four more kinds, checked together:

  registers    each side's registers named by their webs (the definitions and the uses they
               reach, over the function's control flow); a web keeps its register where the
               register is fixed (entry and exit: the parameters, the return value by the
               definition's return type, the preserved registers; a call's register arguments, by
               the callee's decorated name, and what it clobbers; an implicit operand, a shift
               count, a string instruction; esp), and the free webs map one to one
  scheduling   our instructions reordered within their basic blocks, each only past instructions
               it is independent of (registers, the x87 stack, memory: two accesses commute only
               when both read, or they are provably apart: different globals, different stack
               slots, one base register's offsets; a call commutes with no memory access), and
               every condition still reading the flags of the same instructions
  frame        our stack variables (the compiler's listing names them) each moved by one shift
               into what the original's `sub esp` allocates, everything under them (saved
               registers, outgoing arguments) by one shift, the rest of the frame and the
               parameters not at all, with esp followed through pushes, pops and each callee's
               `ret N`; one shift for all variables when an address is taken
  mirror, commutative   as above, inside the same search (a cmp's operands swapped with every
               condition its flags reach; test's; an address's base and index)

An operand nothing names (an unannotated symbol, an import the original's import table doesn't
list) matches nothing.

In optimized code, a source difference whose only effect is scheduling still looks like entropy.
`--coverage` narrows that (the plan's rule 5, tools/coverage.py): an optimized function is
covered when an /Od target proves the same function (exact or witnessed) and both targets'
preprocessed units give it the same tokens and the same declarations, every one it reaches
transitively (types, struct layouts, prototypes, inline bodies; macros are expanded). The 3Dfx
edition is covered by the Matrox edition only, the Matrox edition's optimized units by 1.1. The
report and the JSON (`covered_by`, or `uncovered` and why) say which; it changes no function's
verdict or cost. It needs the compilers (cl /EP) and the covering targets' states
(`--cover-states`, their certify.py or audit.py --json, or certified here).

What remains `unexplained` is either a difference to fix or one to record in
`tools/known_diffs.txt`: a line per target and function, with a hash of the function's rebuilt and
original code (so another diff of the same function no longer matches it) and the reason. With
`--check`, the tool fails on an unexplained function without a matching line, on a line whose hash
no longer matches, and on a line whose function is proven or classified now (remove it). The reason
`undiagnosed` marks a line nobody has looked at, and a reason that says `to investigate` one whose
cause isn't settled yet (it says what was found so far): both keep the unexplained weight.

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
                        [--coverage [--cover-states JSON ...]]
  python tools/audit.py --target MW2 --objects DIR ...   (objects and listings from certify.py)
  python tools/audit.py --corpus [--wine PATH]
  python tools/audit.py --find-corpus REV --target MW23DFX [--wine PATH]
"""

import argparse
import hashlib
import json
import os
import re
import struct
import sys
from collections import Counter
from pathlib import Path

from capstone import CS_ARCH_X86, CS_GRP_CALL, CS_GRP_JUMP, CS_GRP_RET, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

sys.path.insert(0, str(Path(__file__).resolve().parent))
from certify import COST, Certify, _Unit, unit_annotations  # noqa: E402
from funccmp import ends, read_yaml_paths  # noqa: E402
from unitbuild import IMAGE_REL_I386_DIR32, IMAGE_SYM_CLASS_EXTERNAL, NUMBERED, ROOT, TARGETS, Image, Object  # noqa: E402
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

    def __init__(self, offset, mnemonic, ops, text, kind="code", reads=(), writes=(), access=None, raw=((), ())):
        self.offset, self.mnemonic, self.ops, self.text, self.kind = offset, mnemonic, ops, text, kind
        self.reads, self.writes = set(reads), set(writes)  # register families (eax for al), "flags"
        self.access = access or [0] * len(ops)  # per operand: capstone's CS_AC_READ (1) | CS_AC_WRITE (2)
        self.raw = raw  # the registers read and written, by their own names (al, not eax)

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


def dead_after(code, position, name, callees=None):
    """Whether a register (or "flags") is dead after `position`: on every path, written before it
    is read (following jumps to positions in the function; a call may read it, a return or a jump
    out of the function does). Conservative. With `callees` (address: decorated name), a call
    reads only the registers its callee's name says it takes, and clobbers eax, ecx and edx."""
    seen = set()
    work = [position + 1]
    while work:
        k = work.pop()
        while True:
            if k in seen:
                break
            seen.add(k)
            if k >= len(code) or code[k].kind != "code":
                return False
            ins = code[k]
            zeroing = ins.mnemonic in ("xor", "sub") and len(ins.ops) == 2 and ins.ops[0] == ins.ops[1] and ins.ops[0][0] == "reg"
            if name in ins.reads and not (zeroing and name == family(ins.ops[0][1])):  # xor r, r reads nothing
                return False
            if ins.mnemonic == "call":
                if name in ("eax", "flags"):  # a call clobbers eax and the flags; /Gr passes ecx, edx
                    break
                if callees is not None and name in CLOBBERED and name not in call_reads(callee_name(ins, callees)):
                    break
                return False
            if name in ins.writes:
                break
            if ins.mnemonic.startswith("ret") and name == "flags":
                break  # no convention returns anything in the flags
            if ins.mnemonic.startswith(("ret", "loop")):
                return False
            if ins.mnemonic.startswith("j"):
                targets = [op[1] for op in ins.ops if op[0] == "label"]
                if not targets:
                    return False  # an indirect jump or one out of the function
                work += targets
                if ins.mnemonic == "jmp":
                    break
            k += 1
    return True


def register(x, reg):
    return x.reg_name(reg) if reg else None


def lift(code, start, relocate, table_at, extent, tables=()):
    """The instructions of a function's code, decoded from offset 0: `relocate(offset)` gives the
    value of a relocated field (or None), `table_at(offset)` the entries of a jump table that starts
    there (or None). Decoding stops after a terminator with no branch target beyond it."""
    out, offset, targets = [], 0, set(tables)  # tables: where the jump tables a jmp reads start
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
                    if jump and 0 <= op.imm < extent:
                        # a call's target is another function, not more of this one, and so is
                        # a jump past the function's extent (a tail jump to a later function)
                        targets.add(op.imm)
                ops.append(value if value is not None else ("imm", op.imm))
            elif op.type == X86_OP_MEM:
                disp = relocate(offset + e.disp_offset) if e.disp_size == 4 else None
                if disp is None:
                    disp = op.mem.disp
                elif disp[0] == "label" and x.group(CS_GRP_JUMP):
                    targets.add(disp[1])  # a jump table in the function: decode up to it
                ops.append(("mem", register(x, op.mem.segment), register(x, op.mem.base), register(x, op.mem.index), op.mem.scale, disp, op.size))
        read, written = x.regs_access()
        raw = (tuple(x.reg_name(r) for r in read), tuple(x.reg_name(r) for r in written))
        reads = {family(r) for r in raw[0]}
        writes = {family(r) for r in raw[1]}
        out.append(Ins(offset, x.mnemonic, ops, x.op_str, "code", reads, writes, [op.access for op in x.operands], raw))
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
        self.callees = {}  # address: the symbol our code names there (its calling convention)
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
            if symbol.section in (-1, 0xFFFF):  # an absolute symbol
                return ("addr", (symbol.value + addend) & 0xFFFFFFFF)
            if symbol.name == "__except_list":  # the CRT's absolute 0: the SEH chain at fs:[0]
                return ("addr", addend & 0xFFFFFFFF)
            target = annotations.address_of(symbol.name)
            if target is not None:
                self.callees[(target + addend) & 0xFFFFFFFF] = symbol.name
                return ("addr", (target + addend) & 0xFFFFFFFF)
            if symbol.name.startswith("__imp__"):  # an import: its entry in the original's table
                bare = symbol.name[len("__imp_"):]  # _Name@N: a DLL may export the decorated name
                entry = image.imports.get(bare[1:].split("@")[0], image.imports.get(bare))
                if entry is not None:
                    self.callees[(entry + addend) & 0xFFFFFFFF] = symbol.name
                    return ("addr", (entry + addend) & 0xFFFFFFFF)
            data = obj.data_at(symbol, addend) if symbol.section > 0 else None
            if data:
                return ("data", bytes(data))
            data = relocated_data(symbol, addend)
            if data:
                return ("data", data)
            self.unverified.append(symbol.name)
            return ("unknown", symbol.name)

        def relocated_data(symbol, addend, limit=256):
            """The bytes of a data symbol that holds relocations (an SEH scope table: its filters
            and handlers are labels of this function), each relocated field resolved to the
            original's address, or None when one names nothing the original's addresses show."""
            section = obj.section_of(symbol)
            if section is None or section.data is None or symbol.section == function.symbol.section:
                return None
            start = symbol.value + addend
            later = [s.value for s in obj.symbols if s.section == symbol.section and s.value > start]
            end = min(min(later) if later else len(section.data), len(section.data), start + limit)
            data = bytearray(section.data[start:end])
            for o, target, kind in section.relocations:
                if not start <= o < end:
                    continue
                if kind != IMAGE_REL_I386_DIR32 or o + 4 > end:
                    return None
                inner = struct.unpack_from("<i", data, o - start)[0]
                if target is function.symbol or (target.section == function.symbol.section and target.storage != IMAGE_SYM_CLASS_EXTERNAL):
                    offset = target.value + inner - function.start
                    if not 0 <= offset <= len(function.code):
                        return None
                    value = address + offset
                else:
                    known = annotations.address_of(target.name)
                    if known is None:
                        return None
                    value = known + inner
                struct.pack_into("<I", data, o - start, value & 0xFFFFFFFF)
            return bytes(data)

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
            lifted = lift(original, 0, theirs_value, theirs_table, extent, tables)
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
        if op[0] == "imm" and address <= op[1] < address + extent:
            # an address inside the function that isn't a branch (a recursive function pushing
            # itself, an SEH handler's label): our relocation names it as a label too
            return ("label", op[1] - address)
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
        # An import called through its thunk (jmp [entry]); anything else nothing names, so nothing
        # shows it is the original's.
        value = b[1] if isinstance(b, tuple) and b[0] in ("addr", "imm") else b
        bare = a[1].lstrip("_")
        entry = image.imports.get(bare.split("@")[0], image.imports.get("_" + bare))
        return isinstance(value, int) and entry is not None and image.at(value, 6) == b"\xff\x25" + struct.pack("<I", entry)
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


def is_local(op):
    """A slot, or an element of a local array (`[ebp+i*4-720]`: the displacement names the array)."""
    return op[0] == "mem" and op[2] == "ebp" and isinstance(op[5], int) and op[5] < 0


def apply_slots(ours, theirs, local_list, image):
    """Rewrite our [ebp-N] by a one-to-one map of whole locals, if one maps us onto the original:
    (the rewritten code, None), or (None, why there is none: None when no slot differs)."""
    pairs = []
    for a, b in zip(ours, theirs):
        if a.mnemonic != b.mnemonic or len(a.ops) != len(b.ops):
            continue
        for x, y in zip(a.ops, b.ops):
            if is_local(x) and is_local(y) and x[3:5] == y[3:5]:
                pairs.append((x[5], y[5]))
    if not any(x != y for x, y in pairs):
        return None, None
    # Each local's place by majority: a pair from a comparison the original mirrors, or from
    # operands it swaps, names the other operand's slot. Any one-to-one map of whole locals is a
    # rewrite that keeps what the code computes, and the rest must still match exactly.
    named_starts = sorted({offset for _, offset, _ in local_list if offset < 0})
    if named_starts:
        votes = {}
        for ours_disp, theirs_disp in pairs:
            if ours_disp >= named_starts[0]:
                index = max(k for k, start in enumerate(named_starts) if start <= ours_disp)
                votes.setdefault(index, Counter())[theirs_disp - (ours_disp - named_starts[index])] += 1
        best = {index: counts.most_common(1)[0][0] for index, counts in votes.items()}
        pairs = [(o, t) for o, t in pairs if o < named_starts[0] or
                 t - (o - named_starts[max(k for k, start in enumerate(named_starts) if start <= o)]) == best[max(k for k, start in enumerate(named_starts) if start <= o)]]
    order, reason = slot_plan(local_list, pairs)
    if order is None:
        return None, reason
    named = sorted((offset, name) for name, offset, _ in local_list if offset < 0)
    starts = [offset for offset, _ in named]
    shift = {}
    for ours_disp, theirs_disp in pairs:
        if ours_disp < starts[0]:
            continue
        k = max(i for i, s in enumerate(starts) if s <= ours_disp)
        base = theirs_disp - (ours_disp - starts[k])
        if shift.setdefault(k, base - starts[k]) != base - starts[k]:
            return None, "%s is at two places in the original" % named[k][1]
    if not shift:
        return None, None

    def remap(op):
        if not is_local(op) or op[5] < starts[0]:
            return op
        k = max(i for i, s in enumerate(starts) if s <= op[5])
        return op[:5] + (op[5] + shift.get(k, 0),) + op[6:]

    return [copy(i, ops=[remap(op) for op in i.ops]) for i in ours], None


def copy(ins, mnemonic=None, ops=None):
    return Ins(ins.offset, mnemonic or ins.mnemonic, list(ins.ops if ops is None else ops), ins.text, ins.kind, ins.reads, ins.writes,
               ins.access, ins.raw)


def rewrites(ours, k, callees=None):
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
                and a[0] == "reg" and dead_after(ours, c + 1, family(a[1])) \
                and family(a[1]) not in operand_registers(b) | operand_registers(ours[c - 1].ops[1]):
            load = ours[c - 1]
            if b[0] == "mem" and load.ops[1][0] == "mem" and b[6] == load.ops[1][6] or b[0] != "mem" and load.mnemonic == "mov":
                out.append(("mirror", c - 1, [copy(load, ops=[a, b]), copy(cmp, ops=[a, load.ops[1]]), mirrored]))
        if c >= 1 and ours[c - 1].mnemonic == "mov" and b[0] == "reg" and ours[c - 1].ops[0] == b and a[0] == "mem" \
                and ours[c - 1].ops[1][0] == "mem" and ours[c - 1].ops[1][6] == a[6] and dead_after(ours, c + 1, family(b[1])) \
                and family(b[1]) not in operand_registers(a) | operand_registers(ours[c - 1].ops[1]):
            # mov r, x; cmp y, r  becomes  mov r, y; cmp x, r (the condition mirrored)
            load = ours[c - 1]
            out.append(("mirror", c - 1, [copy(load, ops=[b, a]), copy(cmp, ops=[load.ops[1], b]), mirrored]))
    # A commutative operation and the load that feeds it.
    for c in (k - 1, k, k + 1):
        if not 1 <= c < n:
            continue
        op, load = ours[c], ours[c - 1]
        if op.mnemonic in COMMUTATIVE and len(op.ops) == 2 and load.mnemonic == "mov" and load.ops[0] == op.ops[0]:
            source, other = load.ops[1], op.ops[1]
            if source[0] == "mem" and other[0] == "mem" and source[6] != other[6]:
                continue
            dest = load.ops[0]
            if dest[0] == "reg" and family(dest[1]) in operand_registers(source) | operand_registers(other) or \
                    dest[0] == "mem" and "mem" in (source[0], other[0]):
                continue  # an operand that reads what the load writes isn't the same value after it
            out.append(("commutative", c - 1, [copy(load, ops=[load.ops[0], other]), copy(op, ops=[op.ops[0], source])]))
        if c >= 2 and op.mnemonic in COMMUTATIVE and len(op.ops) == 2 and op.ops[1][0] == "reg":
            first, second = ours[c - 2], ours[c - 1]
            if first.mnemonic == second.mnemonic == "mov" and {first.ops[0], second.ops[0]} == {op.ops[0], op.ops[1]} \
                    and dead_after(ours, c, family(op.ops[1][1])) \
                    and not {family(first.ops[0][1]), family(second.ops[0][1])} & (operand_registers(first.ops[1]) | operand_registers(second.ops[1])):
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
    out += load_chains(ours, k, callees)
    return out


# A comparison of two chains of loads, evaluated in the other order.
#
# At /Od the compilers order the sides of a comparison by the symbol table, and a side can be a
# chain of loads: g_players[a]->m_team == g_players[b]->m_team is
#     mov eax, [a]; mov eax, [eax*4+g_players]; mov ecx, [b]; mov ecx, [ecx*4+g_players];
#     mov ecx, [ecx+8]; cmp [eax+8], ecx; jne
# and the same source can evaluate b's side first. Each side is a chain of loads into one register
# (every instruction writes it, the first reads no register but ebp, the others only their own),
# and its value is what the comparison's operand on that side reads: the register, or memory
# through it (the final dereference, which can move between the chain and the comparison). The
# rewrite evaluates the sides in either order, in either register, with the final dereference on
# either side, and the condition mirrored when the comparison's operands are swapped. Sound when
# the chains only load (no stores or calls: their reads commute), neither reads the other's
# register, both registers are dead after the condition and the flags are read only by it.

CHAIN_LOADS = ("mov", "movsx", "movzx", "lea")
SIZED = {("eax", 4): "eax", ("eax", 2): "ax", ("eax", 1): "al", ("ecx", 4): "ecx", ("ecx", 2): "cx", ("ecx", 1): "cl",
         ("edx", 4): "edx", ("edx", 2): "dx", ("edx", 1): "dl", ("ebx", 4): "ebx", ("ebx", 2): "bx", ("ebx", 1): "bl",
         ("esi", 4): "esi", ("edi", 4): "edi"}
REGISTER_SIZE = {name: size for (_, size), name in SIZED.items()}


def operand_registers(op):
    """The register families an operand reads (an address's base and index, or the register)."""
    if op[0] == "reg":
        return {family(op[1])}
    if op[0] == "mem":
        return {family(r) for r in (op[2], op[3]) if r}
    return set()


def chain_back(ours, end, fam):
    """The chain of loads into `fam` that ends just before position `end`: its first position, or
    None. Every instruction writes the whole register from a source that reads no register but ebp
    and fam (the first: ebp only)."""
    k = end - 1
    while k >= 0:
        ins = ours[k]
        if ins.kind != "code" or ins.mnemonic not in CHAIN_LOADS or len(ins.ops) != 2:
            return None
        dest, source = ins.ops
        if dest[0] != "reg" or dest[1] != fam or source[0] != "mem" and not (source[0] in ("reg", "imm", "addr") and ins.mnemonic == "mov"):
            return None
        if ins.mnemonic == "lea" and source[0] != "mem" or source[0] == "mem" and source[1] not in (None, "ds"):
            return None
        reads = operand_registers(source) - {"ebp"}
        if not reads <= {fam}:
            return None
        if not reads:
            return k
        k -= 1
    return None


def renamed(op, mapping):
    """An operand with its registers' families renamed (al to cl with eax to ecx)."""
    def one(name):
        if name is None or family(name) not in mapping:
            return name
        fam, part = PART[name]
        target = mapping[fam]
        return {"e": target, "x": target[1:], "l": target[1] + "l", "h": target[1] + "h"}[part]
    if op[0] == "reg":
        return ("reg", one(op[1]))
    if op[0] == "mem":
        return op[:2] + (one(op[2]), one(op[3])) + op[4:]
    return op


def address_chains(ours, k, callees=None):
    """An address whose base and index are two chains of loads (p->m_items[i]: mov eax, [p];
    mov eax, [eax+16]; mov ecx, [i]; mov eax, [eax+ecx*4]), the chains evaluated in the other
    order or registers. The instruction that uses the address reads no other register and writes
    at most a whole register; both registers are dead after it but the one it writes."""
    out = []
    n = len(ours)
    for c in range(k + 1, min(n, k + 40)):
        use = ours[c]
        if use.kind != "code":
            continue
        memory = [j for j, op in enumerate(use.ops) if op[0] == "mem"]
        if len(memory) != 1:
            continue
        m = use.ops[memory[0]]
        if not m[2] or not m[3] or m[1] not in (None, "ds"):
            continue
        fa, fb = family(m[2]), family(m[3])
        if fa == fb or "ebp" in (fa, fb) or "esp" in (fa, fb):
            continue
        written = None
        others = [(op, access) for j, (op, access) in enumerate(zip(use.ops, use.access)) if j != memory[0]]
        if any(op[0] == "reg" and (access & 1 or use.mnemonic not in CHAIN_LOADS or op[1] not in REGISTER_SIZE or REGISTER_SIZE[op[1]] != 4)
               for op, access in others) or any(op[0] not in ("reg", "imm") for op, _ in others):
            continue
        if others and others[0][0][0] == "reg":
            written = family(others[0][0][1])
        if use.reads - {fa, fb, "ebp", "esp"} or "flags" in use.reads:
            continue
        last = family(ours[c - 1].ops[0][1]) if ours[c - 1].ops and ours[c - 1].ops[0][0] == "reg" else None
        if last not in (fa, fb):
            continue
        second = chain_back(ours, c, last)
        first_fam = fb if last == fa else fa
        first = chain_back(ours, second, first_fam) if second is not None else None
        if first is None or first > k:
            break
        if any(f != written and not dead_after(ours, c, f, callees) for f in (fa, fb)):
            break
        chains = {last: [ours[x] for x in range(second, c)], first_fam: [ours[x] for x in range(first, second)]}
        for one, two in ((first_fam, last), (last, first_fam)):
            for r1, r2 in ((first_fam, last), (last, first_fam)):
                new = []
                for fam, reg in ((one, r1), (two, r2)):
                    for ins in chains[fam]:
                        dest, source = ins.ops
                        source = renamed(source, {fam: reg})
                        new.append(Ins(ins.offset, ins.mnemonic, [("reg", reg), source], "", "code", operand_registers(source), {reg}, [2, 1], ((), (reg,))))
                address = renamed(m, {one: r1, two: r2})
                for operand in [address] + ([address[:2] + (address[3], address[2]) + address[4:]] if m[4] == 1 else []):
                    ops = list(use.ops)
                    ops[memory[0]] = operand  # [a+b] is [b+a]
                    candidate = new + [copy(use, ops=ops)]
                    if [show(i) for i in candidate] != [show(i) for i in ours[first : c + 1]]:
                        out.append(("chains", first, candidate))
        break
    return out


def load_chains(ours, k, callees=None):
    out = address_chains(ours, k, callees)
    n = len(ours)
    for c in range(k + 1, min(n - 1, k + 40)):
        cmp = ours[c]
        if cmp.kind != "code" or cmp.mnemonic != "cmp" or len(cmp.ops) != 2:
            continue
        reader = ours[c + 1]
        if reader.mnemonic not in MIRROR or not dead_after(ours, c + 1, "flags"):
            continue
        a, b = cmp.ops
        if a[0] not in ("reg", "mem") or b[0] not in ("reg", "mem") or a[0] == b[0] == "mem":
            continue
        fa, fb = operand_registers(a) - {"ebp"}, operand_registers(b) - {"ebp"}
        if len(fa) != 1 or len(fb) != 1 or fa == fb:
            continue
        fa, fb = fa.pop(), fb.pop()
        if a[0] == "mem" and (a[3] and family(a[3]) != fa or a[1] not in (None, "ds")) or \
                b[0] == "mem" and (b[3] and family(b[3]) != fb or b[1] not in (None, "ds")):
            continue
        # The two chains: the second ends at the comparison, the first just before it.
        last = family(ours[c - 1].ops[0][1]) if c >= 1 and ours[c - 1].ops and ours[c - 1].ops[0][0] == "reg" else None
        if last not in (fa, fb):
            continue
        second = chain_back(ours, c, last)
        first_fam = fb if last == fa else fa
        first = chain_back(ours, second, first_fam) if second is not None else None
        if first is None or first > k:
            break
        if any(not dead_after(ours, c + 1, f, callees) for f in (fa, fb)):
            break
        # Each side: (its chain of (mnemonic, source) loads, the operand the comparison reads).
        sides = {}
        for fam, start, end in ((first_fam, first, second), (last, second, c)):
            sides[fam] = [(ours[x].mnemonic, ours[x].ops[1]) for x in range(start, end)]
        values = {fa: (sides[fa], a), fb: (sides[fb], b)}
        forms = {}
        for fam, (chain, final) in values.items():
            # The side's forms: as written; its final dereference moved into the chain (a register
            # operand); its chain's last load moved into the comparison (a memory operand).
            options = [(chain, final)]
            size = REGISTER_SIZE.get(final[1]) if final[0] == "reg" else final[6]
            if final[0] == "mem":
                options.append((chain + [("mov", final)], ("reg", SIZED.get((fam, size)))))
            elif chain and chain[-1][0] == "mov" and chain[-1][1][0] == "mem" and len(chain) > 1 and final[1] == fam \
                    and chain[-1][1][6] == 4:
                options.append((chain[:-1], chain[-1][1]))
            forms[fam] = [(ch, fi, size) for ch, fi in options if fi[0] == "reg" and fi[1] or fi[0] == "mem"]
        original_order = (fa, fb)  # the comparison's operands: a's side, then b's
        for one, two in ((first_fam, last), (last, first_fam)):  # which side is evaluated first
            for r1, r2 in ((first_fam, last), (last, first_fam)):  # the registers they're evaluated in
                for c1, f1, s1 in forms[one]:
                    for c2, f2, s2 in forms[two]:
                        if f1[0] == f2[0] == "mem" or len(c1) + len(c2) != c - first or s1 != s2:
                            continue
                        new = []
                        for fam, reg, chain in ((one, r1, c1), (two, r2, c2)):
                            mapping = {fam: reg}
                            for mnemonic, source in chain:
                                source = renamed(source, mapping)
                                dest = ("reg", reg if mnemonic != "mov" or source[0] != "mem" or source[6] == 4 else SIZED.get((reg, source[6])))
                                new.append(Ins(ours[first].offset, mnemonic, [dest, source], "", "code", operand_registers(source),
                                               {reg}, [2, 1], ((), (dest[1],))))
                        finals = {one: renamed(f1, {one: r1}), two: renamed(f2, {two: r2})}
                        for left, right in ((original_order[0], original_order[1]), (original_order[1], original_order[0])):
                            x, y = finals[left], finals[right]
                            if x[0] == "mem" and y[0] == "mem" or ("reg", None) in (x, y):
                                continue
                            swapped = left != original_order[0]
                            compare = copy(cmp, ops=[x, y])
                            compare.reads = operand_registers(x) | operand_registers(y)
                            candidate = new + [compare, copy(reader, MIRROR[reader.mnemonic]) if swapped else reader]
                            if [show(i) for i in candidate] != [show(i) for i in ours[first : c + 2]]:
                                out.append(("chains", first, candidate))
        break
    return out


# ---------------------------------------------------------------------------------------------
# Optimized code: register allocation and scheduling
#
# Each side's registers are named by their webs (the definitions and uses reaching definitions
# connect, computed over the function's control flow): a program with its registers replaced by
# web names computes what the program computes. A web keeps its physical register where the
# register is fixed: at entry and exit (parameters, the return value, the registers a function
# preserves), at a call (its register arguments, what it clobbers), where an instruction names it
# implicitly or requires it (cdq, a shift by cl, the string instructions, fnstsw ax), and esp
# always. Our code, reordered within its basic blocks only past instructions it is independent
# of, must then equal the original's under a one-to-one map of the free webs.

GPR = ("eax", "ecx", "edx", "ebx", "esi", "edi", "ebp", "esp")
PART = {}
for _fam, (_x, _l, _h) in {"eax": ("ax", "al", "ah"), "ecx": ("cx", "cl", "ch"), "edx": ("dx", "dl", "dh"), "ebx": ("bx", "bl", "bh")}.items():
    PART.update({_fam: (_fam, "e"), _x: (_fam, "x"), _l: (_fam, "l"), _h: (_fam, "h")})
for _fam, _x in {"esi": "si", "edi": "di", "ebp": "bp", "esp": "sp"}.items():
    PART.update({_fam: (_fam, "e"), _x: (_fam, "x")})
STRING = ("movs", "stos", "lods", "scas", "cmps", "ins", "outs", "xlat")
SHIFTS = {"sal", "sar", "shl", "shr", "rol", "ror", "rcl", "rcr", "shld", "shrd"}
FIXED_OPERANDS = {"fnstsw", "in", "out"}
FULL_FLAGS = {"cmp", "test", "add", "sub", "and", "or", "xor", "neg", "adc", "sbb", "sahf", "call"}
PRESERVED = ("ebx", "esi", "edi", "ebp", "esp")
EXIT_READS = ("eax", "edx") + PRESERVED
# Return types that come back in eax alone (a pointer does too); a float comes back in st0.
SCALARS = {"int", "long", "unsigned", "signed", "char", "short", "MechS8", "MechU8", "MechS16", "MechU16", "MechS32", "MechU32",
           "MechChar", "MechBool", "BOOL", "DWORD", "WORD", "BYTE", "UINT", "LONG", "ULONG", "HANDLE", "HWND", "HDC", "HRESULT",
           "LRESULT", "HINSTANCE", "HMODULE", "HGLOBAL", "LPARAM", "WPARAM", "MMRESULT", "undefined", "undefined2", "undefined4",
           "size_t", "FxBool", "FxU32", "FxI32", "GrMipMapId_t", "MCIERROR", "HBITMAP", "HPALETTE", "HFONT", "HMENU", "ATOM"}
FLOATS = {"float", "double", "MechFloat", "MechDouble", "FxFloat"}
QUALIFIERS = {"static", "extern", "__inline", "inline", "__cdecl", "__stdcall", "__fastcall", "WINAPI", "CALLBACK", "APIENTRY",
              "const", "volatile", "__declspec", "naked", "AILCALLBACK", "AILCALL", "FAR", "PASCAL"}


def exit_reads(annotation):
    """The registers a function's caller reads when it returns, from its definition's return type:
    the preserved ones, and eax (and edx: a 64-bit value or a small struct) for a value. Anything
    not recognized counts as both."""
    try:
        lines = Path(annotation.filename).read_text(encoding="utf-8", errors="replace").split("\n")[annotation.line_number - 1 : annotation.line_number + 4]
    except (OSError, AttributeError, TypeError):
        return EXIT_READS
    head = " ".join(lines).split("(")[0]
    words = [w for w in re.findall(r"[A-Za-z_]\w*|\*", head) if w not in QUALIFIERS]
    if not words:
        return EXIT_READS
    kind = words[:-1]  # the last word is the function's name (Class::Name leaves the class in front)
    if kind == ["void"]:
        return PRESERVED
    if "*" in kind:
        return ("eax",) + PRESERVED
    if kind and set(kind) <= FLOATS:
        return PRESERVED
    if kind and set(kind) <= SCALARS:
        return ("eax",) + PRESERVED
    return EXIT_READS
CLOBBERED = ("eax", "ecx", "edx")
UNKNOWN_MEMORY = ("call", "ret", "leave", "enter", "pushal", "popal", "pushad", "popad", "pushfd", "popfd")


def is_terminator(ins):
    return ins.kind != "code" or ins.mnemonic.startswith(("j", "ret", "loop")) or ins.mnemonic == "(the original goes on)"


def callee_name(ins, callees):
    """The decorated name a call's target has in our code, or None."""
    target = ins.ops[0] if ins.ops else None
    if target and target[0] == "addr":
        return callees.get(target[1])
    if target and target[0] == "unknown":
        return target[1]
    if target and target[0] == "mem" and isinstance(target[5], tuple) and target[5][0] == "addr" and target[2] is None:
        return callees.get(target[5][1])
    return None


def call_reads(name):
    """The registers a call passes arguments in, from the callee's decorated name (/Gr: __fastcall,
    @name@N, the first two dword arguments in ecx and edx; _name and _name@N take none)."""
    if name is None:
        return ("ecx", "edx")
    if name.startswith("@") and name.rsplit("@", 1)[1].isdigit():
        size = int(name.rsplit("@", 1)[1])
        return () if size == 0 else ("ecx",) if size == 4 else ("ecx", "edx")
    if name.startswith("_"):
        return ()
    return ("ecx", "edx")


class Flow:
    """One side's code: blocks, webs and dependencies."""

    def __init__(self, code, callees, exits=EXIT_READS):
        self.code = code
        # Per instruction: registers read and written (families), operand tokens, fixed occurrences.
        self.reads, self.writes, self.fixed_reads, self.fixed_writes = [], [], [], []
        self.rw = []  # families both read and written by one operand (or written in part): one web
        self.operands = []  # per operand: list of (family, part, "r" or "w") in it
        for ins in code:
            reads, writes, fixed_r, fixed_w, rw, operands = set(), set(), set(), set(), set(), []
            if ins.kind == "code":
                explicit = set()
                fix_all = ins.mnemonic.startswith(STRING) or ins.mnemonic.startswith("rep") or ins.mnemonic in FIXED_OPERANDS
                zero = ins.mnemonic in ("xor", "sub") and len(ins.ops) == 2 and ins.ops[0] == ins.ops[1] and ins.ops[0][0] == "reg"
                for k, (op, access) in enumerate(zip(ins.ops, ins.access)):
                    found = []
                    if op[0] == "reg" and op[1] in PART:
                        fam, part = PART[op[1]]
                        if zero:
                            access = 2
                        if access & 2:
                            found.append((fam, part, "w"))
                            writes.add(fam)
                            if access & 1 or part != "e":
                                rw.add(fam)
                                reads.add(fam)
                        elif access & 1 or access == 0:
                            found.append((fam, part, "r"))
                            reads.add(fam)
                        if fix_all or ins.mnemonic in SHIFTS and k > 0:
                            (fixed_w if access & 2 else fixed_r).add(fam)
                        explicit.add(fam)
                    elif op[0] == "mem":
                        for name in (op[2], op[3]):
                            if name in PART:
                                fam, part = PART[name]
                                found.append((fam, part, "r"))
                                reads.add(fam)
                                explicit.add(fam)
                                if fix_all:
                                    fixed_r.add(fam)
                    operands.append(found)
                raw_r = {family(r) for r in ins.raw[0]} & set(GPR)
                raw_w = {family(r) for r in ins.raw[1]} & set(GPR)
                for fam in raw_r - explicit:
                    reads.add(fam)
                    fixed_r.add(fam)
                for fam in raw_w - explicit:
                    writes.add(fam)
                    fixed_w.add(fam)
                for name in ins.raw[1]:
                    if name in PART and PART[name][1] != "e" and PART[name][0] not in explicit:
                        rw.add(PART[name][0])  # an implicit partial write (fnstsw ax) keeps the rest
                        reads.add(PART[name][0])
                        fixed_r.add(PART[name][0])
                if ins.mnemonic == "call":
                    target = ins.ops[0] if ins.ops else None
                    name = None
                    if target and target[0] == "addr":
                        name = callees.get(target[1])
                    elif target and target[0] == "unknown":
                        name = target[1]
                    elif target and target[0] == "mem" and isinstance(target[5], tuple) and target[5][0] == "addr" and target[2] is None:
                        name = callees.get(target[5][1])
                    for fam in call_reads(name):
                        reads.add(fam)
                        fixed_r.add(fam)
                    for fam in CLOBBERED:
                        writes.add(fam)
                        fixed_w.add(fam)
                elif ins.mnemonic.startswith("ret"):
                    reads |= set(exits)
                    fixed_r |= set(exits)
                elif ins.mnemonic == "jmp" and not (ins.ops and ins.ops[0][0] == "label") and not self._table_jump(ins):
                    reads |= set(GPR)  # a tail call or an indirect jump: everything may be read
                    fixed_r |= set(GPR)
            self.reads.append(reads)
            self.writes.append(writes)
            self.fixed_reads.append(fixed_r)
            self.fixed_writes.append(fixed_w)
            self.rw.append(rw)
            self.operands.append(operands)
        self._blocks()
        self._webs()

    def _table_jump(self, ins):
        op = ins.ops[0] if ins.ops else None
        return op is not None and op[0] == "mem" and isinstance(op[5], tuple) and op[5][0] == "label" \
            and op[5][1] < len(self.code) and self.code[op[5][1]].kind == "table"

    def _blocks(self):
        code, n = self.code, len(self.code)
        leaders = {0}
        for k, ins in enumerate(code):
            if is_terminator(ins) or ins.kind != "code":
                leaders.add(k + 1)
            if ins.kind != "code" and k > 0:
                leaders.add(k)
            for op in ins.ops:
                if op[0] == "label":
                    leaders.add(op[1])
        self.leaders = sorted(l for l in leaders if l < n)
        self.blocks = [(s, e) for s, e in zip(self.leaders, self.leaders[1:] + [n])]
        self.block_of = {}
        for b, (s, e) in enumerate(self.blocks):
            for k in range(s, e):
                self.block_of[k] = b
        self.successors = []
        for s, e in self.blocks:
            last = code[e - 1]
            out = []
            if last.kind != "code" or last.mnemonic == "(the original goes on)":
                pass
            elif last.mnemonic.startswith("ret"):
                pass
            elif last.mnemonic == "jmp":
                if last.ops and last.ops[0][0] == "label":
                    out.append(last.ops[0][1])
                elif self._table_jump(last):
                    out += [op[1] for op in code[last.ops[0][5][1]].ops if op[0] == "label"]
            elif last.mnemonic.startswith(("j", "loop")):
                out += [op[1] for op in last.ops if op[0] == "label"]
                out.append(e)
            else:
                out.append(e)
            self.successors.append([self.block_of[t] for t in out if t in self.block_of and t in self.leaders])

    def _webs(self):
        parent = {}

        def find(x):
            parent.setdefault(x, x)
            while parent[x] != x:
                parent[x] = parent[parent[x]]
                x = parent[x]
            return x

        def union(a, b):
            a, b = find(a), find(b)
            if a != b:
                parent[a] = b

        n = len(self.blocks)
        entry = {fam: frozenset([("entry", fam)]) for fam in GPR}
        ins_state = [None] * n
        ins_state[0] = entry
        changed, work = True, 0
        while changed and work < 100:
            changed, work = False, work + 1
            outs = []
            for b, (s, e) in enumerate(self.blocks):
                state = dict(ins_state[b] or {})
                for k in range(s, e):
                    for fam in self.writes[k]:
                        state[fam] = frozenset([("d", k, fam)])
                outs.append(state)
            for b in range(n):
                for t in self.successors[b]:
                    merged = dict(ins_state[t] or {})
                    for fam, defs in outs[b].items():
                        merged[fam] = merged.get(fam, frozenset()) | defs
                    if merged != (ins_state[t] or {}):
                        ins_state[t] = merged
                        changed = True
        fixed = {("entry", fam) for fam in GPR}
        self.read_node, self.write_node = [], []
        for b, (s, e) in enumerate(self.blocks):
            state = dict(ins_state[b] or {})
            for k in range(s, e):
                reads = {}
                for fam in self.reads[k]:
                    defs = state.get(fam) or frozenset([("u", k, fam)])
                    if ("u", k, fam) in defs:
                        fixed.add(("u", k, fam))  # unreachable: no definition reaches it
                    first = next(iter(defs))
                    for d in defs:
                        union(first, d)
                    reads[fam] = first
                    if fam in self.fixed_reads[k] or fam == "esp":
                        fixed.add(first)
                writes = {}
                for fam in self.writes[k]:
                    node = ("d", k, fam)
                    find(node)
                    writes[fam] = node
                    if fam in self.fixed_writes[k] or fam == "esp":
                        fixed.add(node)
                    if fam in self.rw[k] and fam in reads:
                        union(node, reads[fam])
                    state[fam] = frozenset([node])
                self.read_node.append(reads)
                self.write_node.append(writes)
        self.find = find
        self.fixed = {find(x) for x in fixed}

    def tokens(self, k, j):
        """The web tokens of operand j of instruction k: [(web, part, fixed)]."""
        out = []
        for fam, part, role in self.operands[k][j]:
            node = self.write_node[k][fam] if role == "w" else self.read_node[k][fam]
            web = self.find(node)
            out.append((web, part, web in self.fixed, fam))
        return out

    # Dependencies (in our order), for reordering.

    def regs(self, k):
        ins = self.code[k]
        reads, writes = set(self.reads[k]), set(self.writes[k])
        if ins.kind == "code" and (ins.mnemonic.startswith(("f", "wait")) or ins.mnemonic == "call" or ins.mnemonic.startswith("ret")):
            reads.add("st")
            writes.add("st")
        return reads, writes

    def memory(self, k):
        """(location, size, write) of each memory access."""
        ins = self.code[k]
        if ins.kind != "code" or ins.mnemonic in ("lea", "nop"):
            return []
        if ins.mnemonic in UNKNOWN_MEMORY or ins.mnemonic.startswith(STRING) or ins.mnemonic.startswith("rep"):
            return [(("all",), 0, True)]
        out = []
        if ins.mnemonic == "push":
            out.append((("stack", -4), 4, True))
        elif ins.mnemonic == "pop":
            out.append((("stack", 0), 4, False))
        for op, access in zip(ins.ops, ins.access):
            if op[0] != "mem":
                continue
            seg, base, index, scale, disp, size = op[1:7]
            write = bool(access & 2)
            if base == "esp" and index is None and isinstance(disp, int):
                out.append((("stack", disp), size, write))
            elif base is None and index is None and isinstance(disp, tuple) and disp[0] == "addr":
                out.append((("global", disp[1]), size, write))
            elif base is None and index is None:
                out.append((("any_global",), size, write))
            elif isinstance(disp, int):
                out.append((("ptr", base, index, scale, disp), size, write))
            else:
                out.append((("global_index",), size, write))
            if not access:
                out[-1] = (out[-1][0], size, True)  # unknown access: assume a write
        return out

    def written_between(self, k, i, fams):
        return any(self.writes[x] & fams for x in range(k + 1, i))

    def disjoint(self, a, b, k, i):
        (la, sa, _), (lb, sb, _) = a, b
        ka, kb = la[0], lb[0]
        if "all" in (ka, kb):
            return False
        if {ka, kb} == {"stack"}:
            return not self.written_between(k, i, {"esp"}) and (la[1] + sa <= lb[1] or lb[1] + sb <= la[1])
        if "stack" in (ka, kb):
            return {ka, kb} <= {"stack", "global", "any_global", "global_index"}
        if ka == kb == "global":
            return la[1] + sa <= lb[1] or lb[1] + sb <= la[1]
        if ka == kb == "ptr" and la[1:4] == lb[1:4]:
            fams = {PART[r][0] for r in la[1:3] if r in PART}
            return not self.written_between(k, i, fams) and (la[4] + sa <= lb[4] or lb[4] + sb <= la[4])
        return False

    def independent(self, k, i):
        a, b = self.code[k], self.code[i]
        if is_terminator(a) or is_terminator(b):
            return False
        ra, wa = self.regs(k)
        rb, wb = self.regs(i)
        if wa & (rb | wb) or ra & wb:
            return False
        for x in self.memory(k):
            for y in self.memory(i):
                if (x[2] or y[2]) and not self.disjoint(x, y, k, i):
                    return False
        return True


def flag_chains(code, blocks, order_of):
    """For each instruction that reads the flags: the writers whose flags reach it, back to one that
    writes them all (by position), in the given order of each block."""
    out = {}
    for s, e in blocks:
        chain = ["entry"]
        for k in order_of(s, e):
            ins = code[k]
            if "flags" in ins.reads:
                out[k] = tuple(chain)
            if "flags" in ins.writes or ins.mnemonic == "call":
                chain = [k] if ins.mnemonic in FULL_FLAGS else chain + [k]
        out[("end", s)] = tuple(chain)  # what the block's successors read
    return out


def callee_pops(ins, image, callees, code, cache={}):
    """The bytes a call's callee pops (its ret N), or None when unknown."""
    target = ins.ops[0] if ins.ops else None
    name, address = None, None
    if target == ("label", 0):  # the function itself
        pops = {x.ops[0][1] if x.ops else 0 for x in code if x.kind == "code" and x.mnemonic.startswith("ret")}
        return pops.pop() if len(pops) == 1 else None
    if target and target[0] == "addr":
        address = target[1]
        name = callees.get(address)
    elif target and target[0] == "unknown":
        name = target[1]
    elif target and target[0] == "mem" and target[2] is None and target[3] is None and isinstance(target[5], tuple) and target[5][0] == "addr":
        name = callees.get(target[5][1])
        if name is None:
            return None
    else:
        return None
    if name is not None:
        bare = name[len("__imp_"):] if name.startswith("__imp_") else name
        if bare.startswith("_"):
            tail = bare.rsplit("@", 1)
            if len(tail) == 2 and tail[1].isdigit():
                return int(tail[1])  # _name@N pops N
            if not bare.startswith("__"):
                return 0  # _name: __cdecl (a helper named __x is read from its code)
    if address is None:
        return None
    if address not in cache:
        cache[address] = None
        code = image.at(address, 0x4000)
        for x in DISASSEMBLER.disasm(bytes(code), address):
            if x.group(CS_GRP_RET):
                cache[address] = x.operands[0].imm if x.operands else 0
                break
    return cache[address]


def esp_deltas(code, flow, image, callees):
    """Each instruction's esp, relative to the entry's (the return address's slot), or None (an esp
    no rule follows, or two paths that disagree)."""
    blocks, successors = flow.blocks, flow.successors
    deltas = [None] * len(code)
    entry = [None] * len(blocks)
    entry[0] = 0
    work = [0]
    while work:
        b = work.pop()
        d = entry[b]
        s, e = blocks[b]
        for k in range(s, e):
            deltas[k] = d
            ins = code[k]
            if ins.kind != "code":
                d = None
            elif ins.mnemonic.startswith("push"):
                d = d - 4 if ins.mnemonic in ("push", "pushfd", "pushal") else None
            elif ins.mnemonic in ("pop", "popfd"):
                d += 4
            elif ins.mnemonic in ("sub", "add") and ins.ops and ins.ops[0] == ("reg", "esp") and ins.ops[1][0] == "imm":
                d += ins.ops[1][1] if ins.mnemonic == "add" else -ins.ops[1][1]
            elif ins.mnemonic == "call":
                pops = callee_pops(ins, image, callees, code)
                d = None if pops is None else d + pops
            elif ins.mnemonic.startswith("ret"):
                if d != 0:
                    return [None] * len(code)
            elif "esp" in ins.writes:
                d = None
            if d is None:
                break
        if d is None:
            continue
        for t in successors[b]:
            if entry[t] is None:
                entry[t] = d
                work.append(t)
            elif entry[t] != d:
                return [None] * len(code)
    return deltas


class Frame:
    """Our stack variables (from the compiler's listing) mapped onto the original's frame, each by
    one shift: (start, end) relative to the entry's esp, end the next variable's start."""

    def __init__(self, local_list, ours, theirs, mine, other, image, callees):
        self.mine_deltas = esp_deltas(ours, mine, image, callees)
        self.other_deltas = esp_deltas(theirs, other, image, callees)
        # An optimized (esp-framed) listing gives a local's offset from the entry's esp itself
        # (`_height$ = -24`: [entry esp - 24]); a parameter's from a virtual ebp, 4 below.
        starts = sorted({offset for _, offset, _ in local_list if offset < 0})
        self.objects = [(s, e) for s, e in zip(starts, starts[1:] + [0])]
        # The original's frame: what its `sub esp, N` allocate.
        self.allocated = []
        for k, ins in enumerate(theirs):
            d = self.other_deltas[k]
            if d is not None and ins.mnemonic == "sub" and ins.ops and ins.ops[0] == ("reg", "esp") and ins.ops[1][0] == "imm":
                self.allocated.append((d - ins.ops[1][1], d))
        self.usable = bool(self.objects) and all(d is not None for d in self.mine_deltas + self.other_deltas)
        # An ebp frame's listing counts from ebp: not this convention.
        if any(ins.mnemonic == "mov" and ins.ops == [("reg", "ebp"), ("reg", "esp")] for ins in ours):
            self.usable = False
        # A variable whose address is taken can be reached past the next one's start (variables of
        # other scopes overlap): then every variable moves by the same shift.
        self.rigid = any(ins.mnemonic == "lea" and op[0] == "mem" and op[2] == "esp" for ins in ours for op in ins.ops[1:]) or \
            any(op[0] == "mem" and op[2] == "esp" and op[3] is not None for ins in ours for op in ins.ops)

    def image(self, k, shift):
        """Where variable k (or "below": everything under our lowest variable) lands."""
        if k == "below":
            return (float("-inf"), self.objects[0][0] + shift)
        return (self.objects[k][0] + shift, self.objects[k][1] + shift)

    def bind(self, shifts, i, j, ours_disp, theirs_disp):
        """The shifts extended so that ours[i]'s [esp+ours_disp] is theirs[j]'s, or None. Our
        variables each move by one shift into what the original's `sub esp` allocate; what is
        under them (the saved registers, outgoing arguments: pushes) moves by one shift too; the
        rest of the frame stays where it is, and the return address and parameters do."""
        f_ours, f_theirs = self.mine_deltas[i] + ours_disp, self.other_deltas[j] + theirs_disp
        if f_ours >= 0 or f_theirs >= 0:
            return shifts if f_ours == f_theirs else None
        if f_ours < self.objects[0][0]:
            k = "below"
        else:
            k = next((n for n, (s, e) in enumerate(self.objects) if s <= f_ours < e), None)
        shift = f_theirs - f_ours
        if k is None:
            if shift:
                return None
            key = ("same", f_ours)
            if any(a <= f_ours < b for a, b in (self.image(n, sh) for n, sh in shifts.items() if not isinstance(n, tuple))):
                return None
            return {**shifts, key: 0}
        if self.rigid and k != "below":
            fixed = {sh for n, sh in shifts.items() if not isinstance(n, tuple) and n != "below"}
            if fixed and fixed != {shift}:
                return None
        if k in shifts:
            return shifts if shifts[k] == shift else None
        a, b = self.image(k, shift)
        if k != "below" and shift and not any(x <= a and b <= y for x, y in self.allocated):
            return None
        if any(a <= n[1] < b for n in shifts if isinstance(n, tuple)):
            return None
        for n, other_shift in shifts.items():
            if isinstance(n, tuple):
                continue
            c, d = self.image(n, other_shift)
            if a < d and c < b:
                return None
        return {**shifts, k: shift}


def schedule(lifted, local_list=(), budget=20000):
    """Our code reordered within its blocks and its registers renamed, if that gives the original's:
    (kinds, None), else (None, the first difference). A comparison may also be mirrored (operands
    swapped, every condition that reads it mirrored), a commutative test's operands and an address's
    base and index (scale 1) swapped."""
    ours, theirs, image = lifted.ours, lifted.theirs, lifted.image
    if len(ours) != len(theirs):
        return None, inserted(ours, theirs)
    exits = getattr(lifted, "exit_reads", EXIT_READS)
    mine, other = Flow(ours, lifted.callees, exits), Flow(theirs, lifted.callees, exits)
    if mine.leaders != other.leaders:
        first = next((a for a, b in zip(mine.leaders, other.leaders) if a != b), None)
        return None, "the blocks differ (at #%s)" % first

    frame = Frame(local_list, ours, theirs, mine, other, image, lifted.callees)

    def bind(state, x, y):
        """state: (forward, backward, shifts), dicts; returns the extended state or None."""
        (wx, px, fx, fam_x), (wy, py, fy, fam_y) = x, y
        if px != py or fx != fy or fx and fam_x != fam_y:
            return None
        forward, backward, shifts = state
        if wx in forward:
            return state if forward[wx] == wy else None
        if wy in backward:
            return None
        return ({**forward, wx: wy}, {**backward, wy: wx}, shifts)

    def bind_all(state, xs, ys):
        if len(xs) != len(ys):
            return None
        for x, y in zip(xs, ys):
            state = bind(state, x, y)
            if state is None:
                return None
        return state

    def operand(state, i, j, n, x, y, ty):
        tx = mine.tokens(i, n)
        if x[0] == "reg" and y[0] == "reg":
            if (x[1] in PART) != (y[1] in PART) or x[1] not in PART and x[1] != y[1]:
                return None
            return bind_all(state, tx, ty)
        if x[0] == "mem" and y[0] == "mem" and frame.usable and x[2] == y[2] == "esp" and x[3] is None and y[3] is None \
                and isinstance(x[5], int) and isinstance(y[5], int):
            if x[1] != y[1] or x[6] != y[6]:
                return None
            shifts = frame.bind(state[2], i, j, x[5], y[5])
            return None if shifts is None else (state[0], state[1], shifts)
        if x[0] == "mem" and y[0] == "mem":
            if x[1] != y[1] or x[6] != y[6] or not same_value(x[5], y[5], image):
                return None
            options = [(x[2:5], ty)]
            if x[4] == 1 and x[2] and x[3]:
                options.append(((x[3], x[2], 1), ty[::-1]))  # [a+b] is [b+a]
            for (base, index, scale), tokens in options:
                if (base is None) != (y[2] is None) or (index is None) != (y[3] is None) or scale != y[4]:
                    continue
                result = bind_all(state, tx if (base, index) == (x[2], x[3]) else tx[::-1], tokens)
                if result is not None:
                    return result
            return None
        return state if same_operand(x, y, image) else None

    def unify(state, i, j):
        """[(state, mirrored)] for the ways ours[i] can be theirs[j]."""
        a, b = ours[i], theirs[j]
        if a.kind != b.kind:
            return []
        if a.kind != "code":
            return [(state, False)] if same(a, b, image) else []
        out = []
        if frame.usable and a.mnemonic == b.mnemonic in ("sub", "add") and a.ops[0] == b.ops[0] == ("reg", "esp") \
                and a.ops[1][0] == b.ops[1][0] == "imm":
            return [(state, False)]  # the frame's size: the deltas account for it
        if a.mnemonic == b.mnemonic and len(a.ops) == len(b.ops):
            orders = [(list(range(len(a.ops))), False)]
            if a.mnemonic in ("cmp", "test") and len(a.ops) == 2:
                orders.append(([1, 0], a.mnemonic == "cmp"))
        elif MIRROR.get(a.mnemonic) == b.mnemonic:
            orders = [(list(range(len(a.ops))), "reader")]
        else:
            return []
        for order, mirrored in orders:
            s = state
            for n, m in zip(range(len(a.ops)), order):
                s = s and operand(s, i, j, n, a.ops[n], b.ops[m], other.tokens(j, m))
            if s is not None and frame.usable and a.mnemonic in ("push", "pop", "call"):
                # The slot it writes or reads implicitly maps like any other: so the stack is as
                # deep on both sides wherever it is used outside our variables.
                disp = 0 if a.mnemonic == "pop" else -4
                shifts = frame.bind(s[2], i, j, disp, disp)
                s = None if shifts is None else (s[0], s[1], shifts)
            if s is not None:
                out.append((s, mirrored))
        return out

    nodes, deepest = [0], [0]
    order, mirrored = {}, {}

    def block(state, s, pending, j, e):
        deepest[0] = max(deepest[0], j)
        if j == e:
            return state
        nodes[0] += 1
        if nodes[0] > budget:
            return None
        for n, i in enumerate(pending):
            if any(not mine.independent(k, i) for k in pending[:n]):
                continue
            for new, mirror in unify(state, i, j):
                order[s].append(i)
                mirrored[i] = mirror
                done = block(new, s, pending[:n] + pending[n + 1 :], j + 1, e)
                if done is not None:
                    return done
                order[s].pop()
                del mirrored[i]
        return None

    state = ({}, {}, {})
    for s, e in mine.blocks:
        order[s] = []
        result = block(state, s, list(range(s, e)), s, e)
        if result is None:
            j = deepest[0]
            return None, describe(ours, theirs, j, "no instruction of ours in its block reaches it" if nodes[0] <= budget else "too many ways")
        state = result
    before = flag_chains(ours, mine.blocks, lambda s, e: range(s, e))
    after = flag_chains(ours, mine.blocks, lambda s, e: order[s])
    for k, chain in before.items():
        if after.get(k) != chain:
            return None, describe(ours, theirs, k, "the flags it reads come from elsewhere once reordered")
    # A mirrored comparison: every condition its flags reach is mirrored, and nothing else reads them.
    ends = {}
    for b, (s, e) in enumerate(mine.blocks):
        writers = [k for k in range(s, e) if "flags" in ours[k].writes or ours[k].mnemonic == "call"]
        ends[b] = writers[-1] if writers else None
    predecessors = {b: [p for p in range(len(mine.blocks)) if b in mine.successors[p]] for b in range(len(mine.blocks))}
    for k, chain in before.items():
        if isinstance(k, tuple):
            continue
        if chain[0] == "entry":
            sources = [ends[p] for p in predecessors[mine.block_of[k]]]
            if any(w is None for w in sources):
                sources = None  # flags from further back: only unmirrored readers pass
        else:
            sources = [chain[0]]
        flips = {mirrored.get(w) is True for w in sources or []} | ({mirrored.get(c) is True for c in chain[1:]} if chain[0] != "entry" else set())
        reader = mirrored.get(k)
        same_either_way = MIRROR.get(ours[k].mnemonic) == ours[k].mnemonic
        if sources is None or len(chain) > 1:
            if reader == "reader" and not same_either_way or True in flips:
                return None, describe(ours, theirs, k, "a mirrored comparison's flags reach it")
            continue
        if len(flips) > 1:
            return None, describe(ours, theirs, k, "the flags it reads are mirrored on one path only")
        flipped = True in flips
        if not same_either_way and (reader == "reader") != flipped:
            return None, describe(ours, theirs, k, "its condition and its comparison disagree")
    kinds = Counter()
    if any(order[s] != list(range(s, e)) for s, e in mine.blocks):
        kinds["scheduling"] += 1
    if any(web[-1] != theirs_web[-1] for web, theirs_web in state[0].items()):  # a web's nodes end in its register
        kinds["registers"] += 1
    if any(m is True for m in mirrored.values()):
        kinds["mirror"] += 1
    if any(shift for n, shift in state[2].items() if not isinstance(n, tuple)):
        kinds["frame"] += 1
    return kinds, None


def reassociate(ours, theirs, k, image):
    """A chain `mov r, a; op r, b; op r, c ...` (one commutative, associative op: add, imm... and,
    or, xor) around position k with its operands in the original's order, or None. No operand may
    read r, and the flags the last op sets must be dead."""
    n = len(ours)
    start = k
    while start > 0 and not (ours[start].mnemonic == "mov" and ours[start].ops and ours[start].ops[0][0] == "reg"):
        start -= 1
    first = ours[start]
    if first.mnemonic != "mov" or not first.ops or first.ops[0][0] != "reg":
        return None
    r = first.ops[0]
    end = start + 1
    while end < n and ours[end].mnemonic in ("add", "and", "or", "xor") and len(ours[end].ops) == 2 and ours[end].ops[0] == r:
        end += 1
    if end - start < 3 or not start <= k < end or end > len(theirs):
        return None
    op = ours[start + 1].mnemonic
    if any(ours[x].mnemonic != op for x in range(start + 1, end)) or theirs[start].mnemonic != "mov" or theirs[start].ops[0] != r:
        return None
    if any(theirs[x].mnemonic != op or theirs[x].ops[0] != r for x in range(start + 1, end)):
        return None
    family_r = family(r[1])

    def reads_r(o):
        return o[0] == "reg" and family(o[1]) == family_r or o[0] == "mem" and family_r in (o[2] and family(o[2]), o[3] and family(o[3]))

    mine = [ours[x].ops[1] for x in range(start, end)]
    other = [theirs[x].ops[1] for x in range(start, end)]
    if any(reads_r(o) for o in mine) or not dead_after(ours, end - 1, "flags"):
        return None
    sizes = {o[6] for o in mine if o[0] == "mem"}
    if len(sizes) > 1:
        return None
    left, order = list(mine), []
    for o in other:
        match = next((m for m in left if same_operand(m, o, image)), None)
        if match is None:
            return None
        left.remove(match)
        order.append(match)
    return start, [copy(ours[x], ops=[r, order[x - start]]) for x in range(start, end)]


def classify(lifted, local_list, optimized_unit, steps=None):
    """(kinds Counter, None) when the kinds reproduce the original, else (kinds so far, the first
    remaining difference). `steps` collects each rewrite: (kind, position, before, after)."""
    ours, theirs, image = lifted.ours, lifted.theirs, lifted.image
    kinds = Counter()
    steps = [] if steps is None else steps
    if len(ours) != len(theirs):
        return kinds, inserted(ours, theirs)
    scheduled = None
    if optimized_unit:
        found, scheduled = schedule(lifted, local_list)
        if found is not None:
            return found, None
    unmapped = None
    if not optimized_unit:
        remapped, unmapped = apply_slots(ours, theirs, local_list, image)
        if remapped is not None:
            ours = remapped
            kinds["slots"] += 1
    for _ in range(len(ours) + 1):
        bad = [k for k, (a, b) in enumerate(zip(ours, theirs)) if not same(a, b, image)]
        if not bad:
            return kinds, None
        k = bad[0]
        chain = reassociate(ours, theirs, k, image)
        candidates = rewrites(ours, k, lifted.callees) + ([("commutative", chain[0], chain[1])] if chain else [])
        for kind, first, new in candidates:
            window = range(first, first + len(new))
            if all(same(new[i - first], theirs[i], image) for i in window):
                steps.append((kind, first, [show(i) for i in ours[first : first + len(new)]], [show(i) for i in new]))
                ours = ours[:first] + new + ours[first + len(new) :]
                kinds[kind] += 1
                break
        else:
            return kinds, scheduled or describe(ours, theirs, k, unmapped and "no map of the slots: " + unmapped)
    return kinds, scheduled or describe(ours, theirs, 0, "no progress")


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
    text = "#%d: %s    the original's: %s" % (k, a and show(a), b and show(b))
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
            text = lambda side, a, b: "; ".join(show(x) for x in side[a : min(b, a + 3)]) or "nothing"
            return "%d instructions, the original's %d; #%d: %s    the original's (#%d): %s" % (
                len(ours), len(theirs), i1, text(ours, i1, i2), j1, text(theirs, j1, j2))
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
              "# a line nobody has looked at yet, `to investigate` one whose cause isn't settled (both weigh as",
              "# unexplained). tools/audit.py --check fails on an unexplained function",
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
        symbol, annotation = next((s, a) for s, a in unit_annotations(certify.annotations, _Unit(entry), obj, Path(tree)).items() if a.offset == address)
        later = [a for a in certify.annotations.addresses if a > address]
        lifted = Lifted(obj.functions[symbol], obj, image, address, certify.annotations, min(later) if later else None)
        lifted.exit_reads = exit_reads(annotation)
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
    parser.add_argument("--show", action="store_true", help="Print both sides of each --function, as lifted")
    parser.add_argument("--check", action="store_true", help="Fail on what tools/known_diffs.txt doesn't cover")
    parser.add_argument("--write", action="store_true", help="Rewrite the target's lines of tools/known_diffs.txt")
    parser.add_argument("--coverage", action="store_true", help="Report which optimized functions an /Od proof covers (rule 5; needs the compilers)")
    parser.add_argument("--cover-states", nargs="*", metavar="JSON", help="For --coverage: the covering targets' certify.py or audit.py --json (else certified here)")
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
        symbol, annotation = next((s, a) for s, a in unit_annotations(certify.annotations, _unit(manifest, state["unit"]), obj, tree).items() if a.offset == address)
        later = [a for a in addresses if a > address]
        lifted = Lifted(obj.functions[symbol], obj, image, address, certify.annotations, min(later) if later else None)
        lifted.exit_reads = exit_reads(annotation)
        if args.show:
            for k in range(max(len(lifted.ours), len(lifted.theirs))):
                a = show(lifted.ours[k]) if k < len(lifted.ours) else ""
                b = show(lifted.theirs[k]) if k < len(lifted.theirs) else ""
                print("%4d  %-48s %s %s" % (k, a, "  " if a == b else "| ", b))
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
            elif k["reason"] != "undiagnosed" and "to investigate" not in k["reason"]:
                r["verdict"], r["reason"] = "known", k["reason"]
            else:
                r["reason"] = k["reason"]
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
    coverage_summary = None
    if args.coverage:
        wanted = cover(args, args.target, tree, manifest, results, certify.annotations)
        coverage_summary = {}
        for verdict in ("exact", "witness", "classified", "known", "unexplained"):
            group = [r for r in wanted if r["verdict"] == verdict]
            if group:
                coverage_summary[verdict] = {"covered": sum("covered_by" in r for r in group), "functions": len(group)}
        if args.verbose:
            for r in wanted:
                if r["verdict"] not in ("exact", "witness"):
                    what = "covered by %(target)s %(address)s (%(state)s)" % r["covered_by"] if "covered_by" in r else "not covered: " + r["uncovered"]
                    print("%s %-11s %-32s %s" % (r["address"], r["verdict"], r["name"], what))
        print("%s: coverage of %d optimized functions by an /Od proof (report-only): %s" % (args.target, len(wanted), ", ".join(
            "%s %d of %d" % (v, c["covered"], c["functions"]) for v, c in coverage_summary.items()) or "none"))
    if args.json:
        data = {"target": args.target, "cost": total, "functions": results}
        if coverage_summary is not None:
            data["coverage"] = coverage_summary
        Path(args.json).write_text(json.dumps(data, indent=1) + "\n")
    return 1 if args.check and failures else 0


def _unit(manifest, name):
    return _Unit(manifest["units"][name])


def cover(args, target, tree, manifest, results, annotations):
    """Rule 5 (tools/coverage.py): sets each optimized function's `covered_by` (the target, address
    and state of the /Od proof that covers it) or `uncovered` (why not). Report-only."""
    import coverage
    from funccmp import Annotations, source_roots
    from unitbuild import Target

    optimized = {u for u, e in manifest["units"].items() if e.get("optimized")}
    wanted = [r for r in results if r["unit"] in optimized]
    if not wanted:
        return []
    build = ROOT / args.build
    mine = Target(target, build)
    scratch = build / "certify" / (target + "-coverage")
    names = {}  # address -> (name, the definition's file and line)
    for r in wanted:
        found = [a for a in annotations.functions.get(r["name"], []) if a.offset == int(r["address"], 16)]
        if found:
            names[r["address"]] = (str(Path(found[0].filename).resolve()), found[0].line_number)
    proofs = {}  # (file, line) -> (target, address, state, unit name) of a covering /Od proof
    reasons = {}  # (file, line) -> why a coverer doesn't cover
    coverer_units = {}  # coverer target -> {unit name: Unit}
    for other in coverage.COVERERS.get(target, []):
        theirs = Target(other, build)
        units = {u.name: u for u in theirs.units if u.language in ("C", "CXX")}
        unit_of = {str(p.resolve()): u for u in units.values() for p in u.files(tree).values()}
        states = None
        for path in args.cover_states or []:
            data = json.loads(Path(path).read_text())
            if (data.get("summary") or data).get("target") == other:
                states = {s["address"]: (s.get("verdict", s["state"]), s.get("unit")) for s in data["functions"]}
        if states is None:
            print("coverage: certifying %s" % other)
            certifier = Certify(other, build, args.wine, args.jobs, tree)
            out = build / "certify" / (other + "-cover")
            certifier.compile(out, listings=False)
            states = {s["address"]: (s["state"], s["unit"]) for s in certifier.compare(out, Image(read_yaml_paths(tree).get(other)))}
        other_annotations = Annotations(other, source_roots(other, tree), tree)
        for name, found in other_annotations.functions.items():
            for a in found:
                key = (str(Path(a.filename).resolve()), a.line_number)
                if key in proofs:
                    continue
                state, unit = states.get("%#010x" % a.offset, (None, None))
                unit = units.get(unit) or unit_of.get(key[0])  # a fragment two units include: certify's
                if unit is None or state is None:
                    reasons.setdefault(key, "%s doesn't compile it" % other)
                elif re.search(r"(?:^|\s)[/-]O[x12]", unit.arguments):
                    reasons.setdefault(key, "%s compiles it optimized too" % other)
                elif state not in ("exact", "witness"):
                    reasons.setdefault(key, "%s's /Od function is %s" % (other, state))
                else:
                    proofs[key] = (other, "%#010x" % a.offset, state, unit.name)
                    coverer_units.setdefault(other, {})[unit.name] = unit
    # The preprocessed units, each target's own.
    own_units = {u.name: u for u in mine.units if u.name in {r["unit"] for r in wanted}}
    texts = {target: coverage.preprocess(mine, list(own_units.values()), tree, scratch / target, args.wine, args.jobs)}
    for other, units in coverer_units.items():
        texts[other] = coverage.preprocess(Target(other, build), list(units.values()), tree, scratch / other, args.wine, args.jobs)
    parsed = {}

    def closure(t, unit, name):
        if (t, unit) not in parsed:
            text = texts[t].get(unit)
            parsed[(t, unit)] = coverage.Items(text) if text else None
        items = parsed[(t, unit)]
        return items.closure(name) if items else None

    for r in wanted:
        key = names.get(r["address"])
        proof = proofs.get(key) if key else None
        if proof is None:
            r["uncovered"] = reasons.get(key, "no /Od target compiles it") if key else "no annotation in a source file"
            continue
        other, address, state, unit = proof
        a, b = closure(target, r["unit"], r["name"]), closure(other, unit, r["name"])
        if a is None or b is None:
            r["uncovered"] = "its definition isn't found in the preprocessed unit"
        elif a != b:
            r["uncovered"] = "%s: %s" % (other, coverage.difference(a, b))
        else:
            r["covered_by"] = {"target": other, "address": address, "state": state, "tokens": coverage.fingerprint(a)}
    return wanted


if __name__ == "__main__":
    sys.exit(main())
