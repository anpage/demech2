#!/usr/bin/env python3
"""Transcribe a MASM object of an original DLL into MASM 6.11 source.

Prints the source of the object's code from START up to END, and optionally its data (--data),
for ML 6.11 (`.model flat, c`) to assemble back to the original's bytes. See CLAUDE.md,
"Hand-written Assembly", form 4.

- Routines start at the module's annotated functions (// FUNCTION, // STUB, // LIBRARY, and the
  by-name markers in headers) and at --split addresses (named FUN_<address>). Annotated globals
  inside the range are data the object keeps in .text.
- A routine whose frame is the one ML generates (push ebp; mov ebp, esp; add esp, -N; push the
  USES registers ... pop them; leave; ret at every ret) becomes `Name proc uses ..., p_a:dword`
  with `local`s, its parameters named after the C declaration when a header has one. Other
  routines are transcribed instruction for instruction.
- Addresses are named from the relocation table: annotated functions and globals by name (or
  name+offset), addresses inside the object by label. What it can't name is reported, and
  printed as UNK_<address>.
- ML's alignment padding (90 / 8b c0 / 2e 8b c0 up to a 4-aligned jump target) becomes
  `align 4`.
- Data arrays are wrapped in a STRUCT, which gives them their full size in the debug
  information (ML types a plain definition as one element).

--check assembles the output with ML and compares its code and data with the original byte for
byte, including the relocation targets, and lists the differences.

usage: python tools/asm2masm.py --target MW2|MW2SHELL|MW2MATROX START END [--data START END] [--split A,B,...]
                                [--check --ml PATH/ML.EXE] [-o FILE]
Needs capstone (installed with reccmp).
"""

import argparse
import bisect
import glob
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_MEM
from capstone.x86 import X86_OP_IMM, X86_OP_MEM

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# ML's padding for `align 4`, by gap size
ALIGN_PAD = {1: b"\x90", 2: b"\x8b\xc0", 3: b"\x2e\x8b\xc0"}
FRAME_REGS = ("eax", "ebx", "ecx", "edx", "esi", "edi")
PUSH_REG = {0x50 + i: r for i, r in enumerate(("eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"))}
POP_REG = {0x58 + i: r for i, r in enumerate(("eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"))}
STRING_OP = re.compile(r"^(rep[a-z]* )?(movs|stos|lods|cmps|scas|ins|outs)[bwd]\b")
ELEMENT = {
    "MechS32": 4, "MechU32": 4, "undefined4": 4, "int": 4, "long": 4, "MechFloat": 4, "float": 4,
    "MechS16": 2, "MechU16": 2, "undefined2": 2, "short": 2,
    "MechS8": 1, "MechU8": 1, "MechChar": 1, "undefined": 1, "char": 1,
}
DIRECTIVE = {1: "db", 2: "dw", 4: "dd"}


# --- The original image ---


class Image:
    def __init__(self, path):
        self.data = open(path, "rb").read()
        pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        count = struct.unpack_from("<H", self.data, pe + 6)[0]
        optional_size = struct.unpack_from("<H", self.data, pe + 20)[0]
        self.base = struct.unpack_from("<I", self.data, pe + 24 + 28)[0]
        self.sections = []
        for i in range(count):
            offset = pe + 24 + optional_size + 40 * i
            virtual_size, virtual_address, raw_size, raw_pointer = struct.unpack_from("<IIII", self.data, offset + 8)
            self.sections.append((self.base + virtual_address, virtual_size, raw_size, raw_pointer))
        reloc_rva, reloc_size = struct.unpack_from("<II", self.data, pe + 24 + 96 + 5 * 8)
        self.relocs = set()
        offset = self.file_offset(self.base + reloc_rva)
        end = offset + reloc_size
        while offset < end:
            page, block = struct.unpack_from("<II", self.data, offset)
            if block == 0:
                break
            for i in range((block - 8) // 2):
                entry = struct.unpack_from("<H", self.data, offset + 8 + 2 * i)[0]
                if entry >> 12 == 3:  # IMAGE_REL_BASED_HIGHLOW
                    self.relocs.add(self.base + page + (entry & 0xFFF))
            offset += block

    def section(self, address):
        for start, virtual_size, raw_size, raw in self.sections:
            if start <= address < start + max(virtual_size, raw_size):
                return start, virtual_size, raw_size, raw
        raise ValueError("0x%08x is outside the image" % address)

    def file_offset(self, address):
        start, _, _, raw = self.section(address)
        return raw + address - start

    def initialized(self, address):
        start, _, raw_size, _ = self.section(address)
        return address < start + raw_size

    def read(self, address, size):
        start, _, raw_size, raw = self.section(address)
        present = max(0, min(size, start + raw_size - address))
        offset = raw + address - start
        return self.data[offset : offset + present] + b"\0" * (size - present)

    def dword(self, address):
        return struct.unpack("<I", self.read(address, 4))[0]


# --- Annotations ---


class Symbol:
    def __init__(self, address, name, kind, declaration=None):
        self.address = address
        self.name = name
        self.kind = kind  # "code" or "data"
        self.declaration = declaration


# Source directories and original DLL of the modules whose names aren't their directory's
SOURCE_DIRS = {"MW2MATROX": ("MW2", "common")}
ORIGINALS = {"MW2MATROX": "MW2_MATROX.DLL"}


def module_sources(module):
    paths = []
    for directory in SOURCE_DIRS.get(module, (module,)):
        for pattern in ("*.c", "*.cpp", "*.h"):
            paths += glob.glob(os.path.join(ROOT, directory, "**", pattern), recursive=True)
    return sorted(paths)


def annotated_symbols(module):
    """Address -> Symbol for the module's annotated functions and globals."""
    symbols = {}
    marker = re.compile(r"\s*// (FUNCTION|STUB|GLOBAL|LIBRARY|SYNTHETIC): %s 0x([0-9a-f]{8})" % module)
    for path in module_sources(module):
        lines = open(path, errors="replace").read().split("\n")
        for i, line in enumerate(lines):
            m = marker.match(line)
            if not m:
                continue
            address = int(m.group(2), 16)
            kind = "data" if m.group(1) == "GLOBAL" else "code"
            for following in lines[i + 1 : i + 4]:
                by_name = re.match(r"\s*// ([A-Za-z_?@$][\w?@$]*)\s*$", following)
                if by_name and (m.group(1) == "LIBRARY" or path.endswith(".h")):
                    symbols[address] = Symbol(address, by_name.group(1), kind)
                    symbols[address].by_name = True
                    break
                if following.lstrip().startswith("#") or following.lstrip().startswith("//") or not following.strip():
                    continue
                k = lines.index(following, i + 1)
                joined = " ".join(l.strip() for l in lines[k : k + 3])  # a declaration can span lines
                n = re.search(r"([A-Za-z_]\w*)\s*(\(|\[|=|;)", joined.replace("__declspec(naked)", ""))
                if n:
                    symbols[address] = Symbol(address, n.group(1), kind, joined)
                break
    return symbols


def declared_params(module, name):
    """Parameter names of `name` from its C declaration, or None."""
    pattern = re.compile(r"\b%s\s*\(([^)]*)\)\s*[;{]" % re.escape(name), re.S)
    for path in module_sources(module):
        text = open(path, errors="replace").read()
        m = pattern.search(text)
        if m:
            params = [p.strip() for p in m.group(1).split(",") if p.strip()]
            if params in ([], ["void"]):
                return []
            names = [re.search(r"(\w+)\s*(\[[^]]*\])?$", p) for p in params]
            return [n.group(1) for n in names] if all(names) else None  # None: a function pointer parameter
    return None


def declared_global(module, name):
    """The C declaration of the global `name`, or None."""
    pattern = re.compile(r"^\s*(?:extern\s+)?[\w\s*()]*?\b%s\s*(\[[^]]*\])?\s*[;=]" % re.escape(name), re.M)
    for path in module_sources(module):
        m = pattern.search(open(path, errors="replace").read())
        if m:
            return m.group(0).strip()
    return None


def declared_size(symbol):
    """A global's size from its C declaration, or None."""
    if not symbol.declaration:
        return None
    before = symbol.declaration.replace("extern ", "").split(symbol.name)[0]
    words = before.split()
    element = 4 if "*" in before else ELEMENT.get(words[-1] if words else "", None)
    if element is None:
        return None
    m = re.match(r"\s*\[\s*(0x[0-9a-fA-F]+|\d+)\s*\]", symbol.declaration.split(symbol.name, 1)[1])
    return element * (int(m.group(1), 0) if m else 1)


def data_layout(symbol, size):
    """(element size, count, is_char) for a global from its C declaration, filling `size` bytes."""
    element, is_char = 1, False
    if symbol.declaration:
        declaration = symbol.declaration.replace("extern ", "")
        if "*" in declaration.split(symbol.name)[0]:
            element = 4
        else:
            words = declaration.split(symbol.name)[0].split()
            if words and words[-1] in ELEMENT:
                element = ELEMENT[words[-1]]
                is_char = words[-1] in ("MechChar", "char")
    if size % element:
        element = 1
    return element, size // element, is_char


# --- Rendering ---


def hexnum(value):
    """A number in MASM notation."""
    if value < 0:
        return "-" + hexnum(-value)
    if value < 10:
        return str(value)
    text = "%xh" % value
    return "0" + text if text[0] in "abcdef" else text


class Namer:
    """Names addresses: annotated symbols, labels inside the object, and unknowns."""

    def __init__(self, symbols, labels):
        self.symbols = symbols
        self.starts = sorted(a for a in symbols)
        self.labels = labels  # address -> label inside the object
        self.unknown = set()
        self.used = set()
        self.pointer_types = set()

    def name(self, address, exact=False):
        if address in self.labels:
            return self.labels[address]
        if address in self.symbols:
            self.used.add(address)
            return self.symbols[address].name
        if not exact:
            i = bisect.bisect_right(self.starts, address) - 1
            if i >= 0:
                start = self.starts[i]
                symbol = self.symbols[start]
                size = declared_size(symbol)
                if symbol.kind == "data" and address - start < (size if size else 0x10000):
                    self.used.add(start)
                    return "%s+%s" % (symbol.name, hexnum(address - start))
        self.unknown.add(address)
        return "UNK_%08x" % address


class Instruction:
    def __init__(self, ins):
        self.address = ins.address
        self.size = ins.size
        self.bytes = bytes(ins.bytes)
        self.mnemonic = ins.mnemonic
        self.op_str = ins.op_str
        self.operands = list(ins.operands)
        self.disp_offset = ins.disp_offset if ins.disp_size == 4 else None
        self.imm_offset = ins.imm_offset if ins.imm_size == 4 else None

    @property
    def text(self):
        return (self.mnemonic + " " + self.op_str).strip()

    def is_branch(self):
        b = self.bytes
        return b[0] in (0xE8, 0xE9, 0xEB, 0xE0, 0xE1, 0xE2, 0xE3) or 0x70 <= b[0] <= 0x7F or (b[0] == 0x0F and 0x80 <= b[1] <= 0x8F)

    def branch_target(self):
        return int(self.op_str, 16) if self.is_branch() else None

    def ends_flow(self):
        return self.mnemonic in ("ret", "jmp", "retf", "iretd")


def unbased_index(ins):
    """Whether the memory operand is a SIB with an index and no base register."""
    for op in ins.operands:
        if op.type == X86_OP_MEM:
            return op.mem.base == 0 and op.mem.index != 0
    return False


def render_memory(ins, text, namer, image, frame):
    """Rewrite capstone's memory operands and relocated immediates in MASM syntax."""
    relocated = {}
    for offset in (ins.disp_offset, ins.imm_offset):
        if offset is not None and ins.address + offset in image.relocs:
            relocated[image.dword(ins.address + offset)] = offset == ins.imm_offset

    def memory(m):
        size, segment, inner = (m.group(1) or "").strip(), m.group(2) or "", m.group(3)
        terms = [t.strip() for t in re.split(r"(?=[+-])", inner.replace(" ", "")) if t.strip()]
        regs, disp, symbol = [], 0, None
        for term in terms:
            sign = -1 if term.startswith("-") else 1
            body = term.lstrip("+-")
            if re.match(r"^(0x[0-9a-f]+|\d+)$", body):
                value = int(body, 0) * sign
                if value & 0xFFFFFFFF in relocated and not relocated[value & 0xFFFFFFFF]:
                    symbol = namer.name(value & 0xFFFFFFFF)
                else:
                    disp += value
            else:
                regs.append(body)
        # A lone index with scale 1 (SIB, no base) is written *1, or ML encodes it as the base
        if len(regs) == 1 and "*" not in regs[0] and unbased_index(ins):
            regs[0] += "*1"
        # ML takes the last register of an unscaled pair as the base
        if len(regs) == 2 and "*" not in regs[0] and "*" not in regs[1]:
            regs.reverse()
        if frame and regs == ["ebp"] and symbol is None:
            named = frame.name(disp)
            if named:
                return (size + " " + named).strip()
        parts = ([symbol] if symbol else []) + regs
        inner = "+".join(parts)
        if disp or not parts:
            inner += ("+" if disp >= 0 and parts else "") + hexnum(disp) if parts else hexnum(disp & 0xFFFFFFFF)
        return ("%s %s[%s]" % (size, segment, inner)).strip()

    text = re.sub(r"((?:byte|word|dword|qword|tbyte|xword) ptr )?(\w{2}:)?\[([^]]*)\]", memory, text)

    def immediate(m):
        value = int(m.group(0), 16)
        if value in relocated and relocated[value]:
            return "offset " + namer.name(value)
        return m.group(0)

    return re.sub(r"(?<![\w\[+])0x[0-9a-f]+(?![\w\]])", immediate, text)


def masm_numbers(text):
    return re.sub(r"\b0x([0-9a-f]+)\b", lambda m: hexnum(int(m.group(1), 16)), text)


def render(ins, namer, image, frame, start, end):
    b = ins.bytes
    mnemonic = {"pushal": "pushad", "popal": "popad"}.get(ins.mnemonic, ins.mnemonic)
    if ins.is_branch():
        target = ins.branch_target()
        name = namer.name(target, exact=True)
        return "%s %s" % (mnemonic, name)
    if STRING_OP.match(mnemonic):
        return re.sub(r" .*", "", mnemonic) if not mnemonic.startswith("rep") else " ".join(mnemonic.split()[:2])
    text = render_memory(ins, ins.text, namer, image, frame)
    if mnemonic in ("rol", "ror", "rcl", "rcr", "shl", "sal", "shr", "sar") and "," not in ins.op_str:
        text += ", 1"  # capstone leaves out the implicit count of the D0/D1 forms
    if mnemonic in ("xchg", "test") and len(b) == 2 and b[1] >= 0xC0 and b[0] in (0x84, 0x85, 0x86, 0x87):
        # ML puts the first operand in the ModRM reg field
        a, c = [s.strip() for s in ins.op_str.split(",")]
        text = "%s %s, %s" % (mnemonic, c, a)
    return masm_numbers(text.replace(ins.mnemonic, mnemonic, 1))


# --- Routines ---


class Frame:
    """An ML-generated frame: USES registers, parameters and locals."""

    def __init__(self, uses, has_ebp, locals_size, params):
        self.uses = uses
        self.has_ebp = has_ebp
        self.locals_size = locals_size
        self.params = params

    def name(self, disp):
        if not self.has_ebp:
            return None
        if disp >= 8 and (disp - 8) % 4 == 0 and (disp - 8) // 4 < len(self.params):
            return self.params[(disp - 8) // 4]
        if -self.locals_size <= disp < 0:
            slot = (-disp + 3) // 4 * 4
            return "l_unk0x%02x" % slot + ("+%d" % (slot + disp) if slot + disp else "")
        return None

    def header(self):
        parts = []
        if self.uses:
            parts.append("uses " + " ".join(self.uses))
        params = ", ".join("%s:dword" % p for p in self.params)
        head = " ".join(parts)
        if params:
            head = (head + ", " if head else "") + params
        return head


def find_frame(instructions, params_hint):
    """The routine's ML frame (and the instructions it generates), or None."""
    texts = [i.text for i in instructions]
    i = 0
    has_ebp = texts[:2] == ["push ebp", "mov ebp, esp"]
    if has_ebp:
        i = 2
    locals_size = 0
    if has_ebp and i < len(texts) and re.match(r"^add esp, -(0x[0-9a-f]+|\d+)$", texts[i]):
        locals_size = int(texts[i].split("-")[1], 0)
        i += 1
    uses = []
    while i < len(instructions) and instructions[i].bytes[0] in PUSH_REG and len(instructions[i].bytes) == 1:
        reg = PUSH_REG[instructions[i].bytes[0]]
        if reg not in FRAME_REGS or reg in uses:
            break
        uses.append(reg)
        i += 1
    prologue = i
    epilogue = ["pop " + r for r in reversed(uses)] + (["leave"] if has_ebp else [])
    exits = []
    for k, ins in enumerate(instructions):
        if ins.mnemonic in ("ret", "retf") or (ins.mnemonic == "leave" and has_ebp):
            if ins.text != "ret" and ins.mnemonic != "leave":
                return None
            if ins.mnemonic == "leave":
                continue
            if texts[k - len(epilogue) : k] != epilogue:
                return None
            exits.append(k)
    leaves = sum(1 for t in texts if t == "leave")
    if has_ebp and leaves != len(exits):
        return None
    if not exits:
        return None
    # parameters: ML only builds the ebp frame for a proc with parameters or locals
    highest = -1
    for ins in instructions:
        for m in re.finditer(r"\[ebp \+ (0x[0-9a-f]+|\d+)\]", ins.op_str):
            disp = int(m.group(1), 0)
            if disp >= 8 and disp % 4 == 0:
                highest = max(highest, (disp - 8) // 4)
    count = highest + 1
    names = list(params_hint or [])
    if len(names) < count:
        names += ["p_unk0x%02x" % (4 * n) for n in range(len(names), count)]
    if has_ebp and not names and not locals_size:
        return None
    if not has_ebp and not uses:
        return None
    frame = Frame(uses, has_ebp, locals_size, names if has_ebp else [])
    generated = set(range(prologue))
    for k in exits:
        generated.update(range(k - len(epilogue), k))
    return frame, generated


def existing_comments(path):
    """Comments to carry over from an existing transcription: (file header, {name: lines}).

    From a .asm: the `;` lines before each `Name proc` and before each `public name`. From a
    .c/.h: the `//` lines before each function's or global's annotation (or #ifdef COMPAT_MODE)."""
    lines = open(path, errors="replace").read().split("\n")
    comments = {}
    header = []
    if path.endswith(".asm"):
        k = 0
        while k < len(lines) and lines[k].startswith(";"):
            header.append(lines[k])
            k += 1
        for i, line in enumerate(lines):
            m = re.match(r"^(\w+) proc\b", line) or re.match(r"^\s*public (\w+)", line)
            if not m:
                continue
            j = i - 1
            if not line.startswith("\t") and False:
                pass
            block = []
            while j >= 0 and lines[j].startswith(";") and j >= len(header):
                block.insert(0, lines[j])
                j -= 1
            if block:
                comments[m.group(1)] = block
        return header, comments
    text = "\n".join(lines)
    m = re.match(r"\s*/\*(.*?)\*/", text, re.S)
    if m:
        header = ["; " + l.strip() if l.strip() else ";" for l in m.group(1).strip().split("\n")]
    for i, line in enumerate(lines):
        if not (re.match(r"\s*// (FUNCTION|STUB|GLOBAL):", line) or line.startswith("#ifdef COMPAT_MODE")):
            continue
        name = None
        for following in lines[i + 1 : i + 6]:
            n = re.search(r"(\w+)\s*(\(|\[|=|;)", following.replace("__declspec(naked)", ""))
            if n and not following.lstrip().startswith("//"):
                name = n.group(1)
                break
        j = i - 1
        block = []
        while j >= 0 and lines[j].lstrip().startswith("//") and not re.match(r"\s*// (FUNCTION|STUB|GLOBAL|SIZE)", lines[j]):
            block.insert(0, "; " + lines[j].lstrip()[3:])
            j -= 1
        if name and block and name not in comments:
            comments[name] = block
    return header, comments


def decode(image, start, end):
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = True
    instructions = [Instruction(i) for i in md.disasm(image.read(start, end - start), start)]
    reached = instructions[-1].address + instructions[-1].size if instructions else start
    if reached != end:
        raise SystemExit("the decode from 0x%08x stops at 0x%08x, not 0x%08x: embedded data? (annotate it as a global)" % (start, reached, end))
    return instructions


def is_padding(instructions, k, labels):
    """Whether instructions[k:] starts ML's align 4 padding: returns the count of instructions."""
    ins = instructions[k]
    if k == 0:
        return 0
    for size, pad in ALIGN_PAD.items():
        if ins.bytes == pad and (ins.address + size) % 4 == 0 and ins.address + size in labels:
            return 1
    return 0


class Transcriber:
    def __init__(self, module, image, symbols, start, end, splits):
        self.module = module
        self.image = image
        self.start, self.end = start, end
        self.items = []  # (address, name, kind) in the object's .text
        starts = {a: s for a, s in symbols.items() if start <= a < end}
        for a in splits:
            if a not in starts:
                starts[a] = Symbol(a, "FUN_%08x" % a, "code")
            elif starts[a].kind == "data":
                starts[a] = Symbol(a, starts[a].name, "code")  # code the annotations call data
        if start not in starts:
            raise SystemExit("0x%08x (START) is not an annotated function: annotate it or pass it with --split" % start)
        ordered = sorted(starts)
        for i, a in enumerate(ordered):
            self.items.append((a, ordered[i + 1] if i + 1 < len(ordered) else end, starts[a]))
        self.labels = {a: s.name for a, s in starts.items()}
        self.decoded = {}
        for a, b, s in self.items:
            if s.kind == "code":
                self.decoded[a] = decode(image, a, b)
        for instructions in self.decoded.values():
            for ins in instructions:
                target = ins.branch_target()
                if target is not None and start <= target < end and target not in self.labels:
                    self.labels[target] = "jmp_%08x" % target
        # relocated addresses inside the object that aren't item starts get labels as well; one
        # inside an instruction (self-modifying code patching an operand) is named after the
        # instruction's label
        starts = sorted(ins.address for instructions in self.decoded.values() for ins in instructions)
        inside = {}
        values = []
        for instructions in self.decoded.values():
            for ins in instructions:
                for offset in (ins.disp_offset, ins.imm_offset):
                    if offset is not None and ins.address + offset in image.relocs:
                        values.append(image.dword(ins.address + offset))
        for a, b, s in self.items:
            if s.kind == "data":
                values += [image.dword(r) for r in image.relocs if a <= r < b]
        for value in values:
            if not start <= value < end or value in self.labels or self.in_data(value):
                continue
            i = bisect.bisect_right(starts, value) - 1
            if starts[i] == value:
                self.labels[value] = "jmp_%08x" % value
            else:
                if starts[i] not in self.labels:
                    self.labels[starts[i]] = "jmp_%08x" % starts[i]
                inside[value] = starts[i]
        for value, at in inside.items():
            self.labels[value] = "%s+%d" % (self.labels[at], value - at)
        self.inside = set(inside)
        self.namer = Namer(symbols, self.labels)

    def in_data(self, address):
        return any(a <= address < b for a, b, s in self.items if s.kind == "data")

    def routine(self, address, symbol):
        instructions = self.decoded[address]
        params = declared_params(self.module, symbol.name)
        found = find_frame(instructions, params)
        generated = set()
        frame = None
        if found:
            frame, generated = found
            # a jump into the generated code other than to the start of an epilogue can't be expressed
            for k in generated:
                ins = instructions[k]
                if k and ins.address in self.labels and not (ins.text.startswith("pop") and k - 1 not in generated):
                    if not (ins.text == "leave" and k - 1 not in generated):
                        frame, generated = None, set()
                        break
            if frame and frame.locals_size % 4:
                frame, generated = None, set()
        lines = []
        head = frame.header() if frame else ""
        lines.append(("%s proc %s" % (symbol.name, head)).rstrip())
        if frame and frame.locals_size:
            slots = ["l_unk0x%02x:dword" % (4 * (n + 1)) for n in range(frame.locals_size // 4)]
            for n in range(0, len(slots), 6):
                lines.append("\tlocal " + ", ".join(slots[n : n + 6]))
        k = 0
        while k < len(instructions):
            ins = instructions[k]
            if ins.address in self.labels and k:
                lines.append(self.labels[ins.address] + ":")
            if k in generated:
                k += 1
                continue
            pad = is_padding(instructions, k, self.labels)
            if pad:
                lines.append("\talign 4")
                k += pad
                continue
            lines.append("\t" + render(ins, self.namer, self.image, frame, self.start, self.end))
            k += 1
        lines.append("%s endp" % symbol.name)
        return lines

    def text_data(self, address, end, symbol):
        return data_lines(self.image, self.namer, symbol, address, end)

    def code(self):
        lines = []
        for address, end, symbol in self.items:
            lines.append("")
            if symbol.kind == "code":
                lines += self.routine(address, symbol)
            else:
                lines += self.text_data(address, end, symbol)
        return lines


def data_lines(image, namer, symbol, address, end):
    """A global's definition: a STRUCT around arrays so the debug information has its size.

    Relocated dwords are addresses, typed as pointers (CODEPTR/DATAPTR): datacmp compares a typed
    pointer by the symbol it points to, and an untyped one by its value, which differs."""
    size = end - address
    element, count, is_char = data_layout(symbol, size)
    initialized = image.initialized(address)
    pointers = sorted(a for a in image.relocs if address <= a < end) if initialized else []
    if pointers:
        if any((a - address) % 4 for a in pointers) or size % 4:
            raise SystemExit("%s holds addresses at unaligned offsets: split it" % symbol.name)
        members = []
        for at in range(address, end, 4):
            if at in pointers:
                target = image.dword(at)
                name = namer.name(target)
                kind = "CODEPTR" if (namer.labels.get(target) or namer.symbols.get(target, Symbol(0, "", "data")).kind == "code") else "DATAPTR"
                namer.pointer_types.add(kind)
                members.append((at - address, kind, name))
            else:
                members.append((at - address, "dd", hexnum(image.dword(at))))
        lines = ["\tpublic %s" % symbol.name]
        if len(members) == 1:
            lines.append("%s %s %s" % (symbol.name, members[0][1], members[0][2]))
            return lines
        type_name = symbol.name + "_t"
        lines.append("%s struct" % type_name)
        k = 0
        while k < len(members):
            offset, kind, value = members[k]
            if kind != "dd":
                lines.append("m_unk0x%02x %s %s" % members[k])
                k += 1
                continue
            run = []
            while k < len(members) and members[k][1] == "dd":
                run.append(members[k][2])
                k += 1
            body = pack_values("dd", run, None)
            lines.append("m_unk0x%02x %s" % (offset, body[0].strip()))
            lines += body[1:]
        lines.append("%s ends" % type_name)
        lines.append("%s %s <>" % (symbol.name, type_name))
        return lines
    directive = DIRECTIVE[element]
    values = []
    for n in range(count):
        at = address + n * element
        if not initialized:
            values.append("?")
        else:
            values.append(hexnum(int.from_bytes(image.read(at, element), "little")))
    body = pack_values(directive, values, image.read(address, size) if is_char and initialized else None)
    lines = ["\tpublic %s" % symbol.name]
    if count == 1:
        lines.append("%s %s" % (symbol.name, body[0].strip()))
        return lines
    type_name = symbol.name + "_t"
    lines.append("%s struct" % type_name)
    lines.append("m_data " + body[0].strip())
    lines += body[1:]
    lines.append("%s ends" % type_name)
    lines.append("%s %s <>" % (symbol.name, type_name))
    return lines


def pack_values(directive, values, text):
    """`directive v, v, ...` lines, with runs of one value as `n dup (v)`, and text quoted."""
    if text is not None:
        items, run = [], ""
        for byte in text:
            if 0x20 <= byte < 0x7F and byte != 0x22:
                run += chr(byte)
            else:
                if run:
                    items.append('"%s"' % run)
                    run = ""
                items.append(hexnum(byte))
        if run:
            items.append('"%s"' % run)
        values = items
    groups = []
    i = 0
    while i < len(values):
        j = i
        while j < len(values) and values[j] == values[i]:
            j += 1
        groups.append("%s dup (%s)" % (hexnum(j - i), values[i]) if j - i > 2 else ", ".join(values[i:j]))
        i = j
    lines, current = [], []
    for g in groups:
        current.append(g)
        if len(", ".join(current)) > 80:
            lines.append("\t%s %s" % (directive, ", ".join(current)))
            current = []
    if current:
        lines.append("\t%s %s" % (directive, ", ".join(current)))
    return lines


def data_section(image, namer, symbols, start, end):
    items = sorted(a for a, s in symbols.items() if start <= a < end and s.kind == "data")
    if not items or items[0] != start:
        raise SystemExit("0x%08x (the data START) is not an annotated global" % start)
    lines, current = [], None
    for i, a in enumerate(items):
        b = items[i + 1] if i + 1 < len(items) else end
        section = "\t.data" if image.initialized(a) else "\t.data?"
        if section != current:
            lines += ["", section]
            current = section
        if not image.initialized(a) and image.initialized(b - 1):
            raise SystemExit("0x%08x straddles the end of the raw data" % a)
        lines.append("")
        lines += data_lines(image, namer, symbols[a], a, b)
    return lines


# --- Checking against the original ---


class Coff:
    def __init__(self, data):
        self.data = data
        count = struct.unpack_from("<H", data, 2)[0]
        symbols_at, symbol_count = struct.unpack_from("<II", data, 8)
        strings_at = symbols_at + 18 * symbol_count
        self.symbols = []
        i = 0
        while i < symbol_count:
            raw = data[symbols_at + 18 * i : symbols_at + 18 * i + 18]
            if raw[:4] == b"\0\0\0\0":
                offset = struct.unpack_from("<I", raw, 4)[0]
                name = data[strings_at + offset : data.index(b"\0", strings_at + offset)].decode()
            else:
                name = raw[:8].rstrip(b"\0").decode()
            value, section, _, storage, aux = struct.unpack_from("<IhHBB", raw, 8)
            self.symbols.append((name, value, section, storage))
            self.symbols += [None] * aux
            i += 1 + aux
        self.sections = {}
        for n in range(count):
            at = 20 + 40 * n
            name = data[at : at + 8].rstrip(b"\0").decode()
            size, pointer, relocs_at = struct.unpack_from("<III", data, at + 16)
            reloc_count = struct.unpack_from("<H", data, at + 32)[0]
            relocs = [struct.unpack_from("<IIH", data, relocs_at + 10 * r) for r in range(reloc_count)]
            self.sections[n + 1] = (name, data[pointer : pointer + size] if pointer else b"\0" * size, size, relocs)


def run_ml(ml, wine, source):
    work = tempfile.mkdtemp(prefix="asm2masm")
    try:
        open(os.path.join(work, "check.asm"), "w").write(source)
        command = [ml, "/nologo", "/c", "/coff", "/Fl", "/Focheck.obj", "check.asm"]
        if sys.platform != "win32":
            command = [wine] + command
        result = subprocess.run(command, cwd=work, capture_output=True, text=True, errors="replace",
                                env=dict(os.environ, WINEDEBUG="-all"))
        obj = os.path.join(work, "check.obj")
        if result.returncode or not os.path.exists(obj):
            lines = source.split("\n")
            report = []
            for m in re.finditer(r"check\.asm\((\d+)\) : (.*)", result.stdout + result.stderr):
                n = int(m.group(1))
                report.append("line %d: %s\n    %s" % (n, m.group(2), lines[n - 1].strip() if n <= len(lines) else ""))
            raise SystemExit("ML failed:\n" + ("\n".join(report) or result.stdout + result.stderr))
        return open(obj, "rb").read()
    finally:
        shutil.rmtree(work, ignore_errors=True)


def check(image, symbols, obj, bases, limit=40):
    """Compare the object's sections and relocation targets with the original. Returns the count of problems."""
    by_name = {s.name: a for a, s in symbols.items()}
    coff = Coff(obj)
    problems = []
    section_base = {}
    for number, (name, data, size, relocs) in coff.sections.items():
        if name in bases:
            section_base[number] = bases[name]
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    for number, (name, data, size, relocs) in coff.sections.items():
        if name not in bases:
            if name in (".text", ".data", ".bss") and size:
                problems.append((0, "%s: %d bytes, but no original range was given for it" % (name, size)))
            continue
        base = bases[name]
        masked = set()
        for field, index, kind in relocs:
            masked.update(range(field, field + 4))
            target_name, value, target_section, _ = coff.symbols[index]
            addend = struct.unpack_from("<I", data, field)[0]
            if target_section > 0:
                target = section_base.get(target_section, 0) + value
            else:
                target = by_name.get(target_name[1:] if target_name.startswith("_") else target_name)
                if target is None:
                    problems.append((base + field, "relocation to %s, which has no annotated address" % target_name))
                    continue
            if kind == 6:  # DIR32
                ours = (target + addend) & 0xFFFFFFFF
                theirs = image.dword(base + field)
            elif kind == 0x14:  # REL32
                ours = (target + addend) & 0xFFFFFFFF
                theirs = (base + field + 4 + image.dword(base + field)) & 0xFFFFFFFF
            else:
                problems.append((base + field, "unhandled relocation type %d" % kind))
                continue
            if ours != theirs:
                problems.append((base + field, "relocation to 0x%08x (%s), the original has 0x%08x" % (ours, target_name, theirs)))
        ours_fields = {base + field for field, _, _ in relocs}
        theirs_fields = {a for a in image.relocs if base <= a < base + size}
        for a in sorted(theirs_fields - ours_fields):
            problems.append((a, "the original relocates this address, ours doesn't (a constant where the original has an address)"))
        for a in sorted(ours_fields - theirs_fields):
            problems.append((a, "ours relocates this address, the original doesn't"))
        if name == ".bss":
            continue
        original = image.read(base, size)
        if name == ".text":
            listing = list(md.disasm(original, base))
            for ins in listing:
                k = ins.address - base
                ours = data[k : k + ins.size]
                if any(ours[i] != ins.bytes[i] for i in range(len(ours)) if k + i not in masked):
                    problems.append((ins.address, "ours %s, the original %s (%s %s)" % (ours.hex(" "), bytes(ins.bytes).hex(" "), ins.mnemonic, ins.op_str)))
        else:
            for k in range(size):
                if k not in masked and data[k] != original[k]:
                    problems.append((base + k, "(%s+0x%x) ours %02x, the original %02x" % (name, k, data[k], original[k])))
    problems.sort()
    for address, message in problems[:limit]:
        print("0x%08x: %s" % (address, message))
    if len(problems) > limit:
        print("... and %d more" % (len(problems) - limit))
    return len(problems)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", required=True, help="MW2, MW2SHELL or MW2MATROX")
    parser.add_argument("--dll", help="The original DLL (default: <target>.DLL in the repository root)")
    parser.add_argument("start", help="The object's first code address (hex)")
    parser.add_argument("end", help="The address after its code (hex)")
    parser.add_argument("--data", nargs=2, metavar=("START", "END"), help="The object's data (hex), .data and .bss")
    parser.add_argument("--split", default="", help="Routine starts that have no annotation yet, comma-separated (hex)")
    parser.add_argument("-o", "--output", help="Write the source here instead of printing it")
    parser.add_argument("--comments", help="Carry the file header and the routines' and globals' comments over from this .asm or .c")
    parser.add_argument("--check", action="store_true", help="Assemble the output and compare it with the original")
    parser.add_argument("--ml", help="ML.EXE for --check")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"), help="Wine for --check off Windows")
    args = parser.parse_args()

    image = Image(args.dll or os.path.join(ROOT, ORIGINALS.get(args.target, args.target + ".DLL")))
    symbols = annotated_symbols(args.target)
    for symbol in symbols.values():
        if symbol.kind == "data" and symbol.declaration is None:
            symbol.declaration = declared_global(args.target, symbol.name)
    start, end = int(args.start, 16), int(args.end, 16)
    while end > start and image.read(end - 1, 1) == b"\xcc":
        end -= 1  # the linker's padding before the next object
    splits = [int(a, 16) for a in args.split.split(",") if a]
    transcriber = Transcriber(args.target, image, symbols, start, end, splits)
    code = transcriber.code()
    data = []
    data_start = data_end = None
    if args.data:
        data_start, data_end = int(args.data[0], 16), int(args.data[1], 16)
        data = data_section(image, transcriber.namer, symbols, data_start, data_end)

    defined = {s.name for _, _, s in transcriber.items}
    if args.data:
        defined |= {s.name for a, s in symbols.items() if data_start <= a < data_end}
    externs = []
    for address in sorted(transcriber.namer.used):
        symbol = symbols[address]
        if symbol.name not in defined:
            externs.append("\textern %s:%s" % (symbol.name, "near" if symbol.kind == "code" else "byte"))
    lines = ["\t.386", "\t.model flat, c", "\toption noscoped", "\toption casemap:none"]
    if transcriber.namer.pointer_types:
        lines += ["", "CODE typedef proto", "CODEPTR typedef ptr CODE", "DATAPTR typedef ptr byte"]
    if externs:
        lines += [""] + sorted(set(externs))
    lines += data
    lines += ["", "\t.code"] + code + ["", "\tend", ""]
    if args.comments:
        header, comments = existing_comments(args.comments)
        merged = header + ([""] if header else [])
        for line in lines:
            m = re.match(r"^(\w+) proc\b", line) or re.match(r"^\tpublic (\w+)$", line)
            if m and m.group(1) in comments:
                merged += comments[m.group(1)]
            merged.append(line)
        lines = merged
    source = "\n".join(lines)

    status = 0
    for address in sorted(transcriber.namer.unknown):
        print("can't name 0x%08x: annotate it" % address, file=sys.stderr)
        status = 1
    if args.output:
        open(args.output, "w").write(source)
    else:
        print(source)
    if args.check:
        if not args.ml:
            raise SystemExit("--check needs --ml")
        obj = run_ml(args.ml, args.wine, source)
        bases = {".text": start}
        if args.data:
            initialized = [a for a in sorted(symbols) if data_start <= a < data_end and image.initialized(a)]
            uninitialized = [a for a in sorted(symbols) if data_start <= a < data_end and not image.initialized(a)]
            if initialized:
                bases[".data"] = initialized[0]
            if uninitialized:
                bases[".bss"] = uninitialized[0]
        problems = check(image, symbols, obj, bases)
        print("%d problem%s" % (problems, "" if problems == 1 else "s"), file=sys.stderr)
        status = status or (1 if problems else 0)
    return status


if __name__ == "__main__":
    sys.exit(main())
