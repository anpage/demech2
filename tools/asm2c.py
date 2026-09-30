#!/usr/bin/env python3
"""Transcribe a hand-written assembly routine of an original DLL into VC++ 4.1 inline-asm text.

Prints the body of an `__asm { ... }` block for the routine from START up to END (trailing int3
padding is dropped), ready for a __declspec(naked) function or, with --body, for a C function
whose /Od frame the compiler supplies. It applies the inline-assembler rules from CLAUDE.md
("Hand-written Assembly"):

- short (rel8) conditional jumps become _emit pairs (the inline assembler only emits rel32);
  rel32 conditional jumps stay label jumps;
- short unconditional jumps become `jmp short label` (forward and backward alike); a backward
  rel32 `jmp` is _emitted, since the inline assembler would shorten it;
- `xchg r, r` and `test r, r` are _emitted (the inline assembler swaps the ModRM registers);
- addresses of annotated functions and globals (from the module's // FUNCTION, // STUB,
  // GLOBAL and // LIBRARY annotations) become their names: `offset name` for immediates, and
  `[name + reg]` inside brackets (4.1 drops a register written before a symbol).

Anything it can't name is printed as `0x........ /* ??? */`: define or stub it, then rerun.
Jump tables embedded in the code are not handled: an object with them is a MASM object, for
tools/asm2masm.py (CLAUDE.md, "Hand-written Assembly", form 4).

usage: python tools/asm2c.py --target MW2 START END [--body --params p_a,p_b,...] [--dll PATH]
  --body    drop the /Od frame (push ebp; mov ebp, esp; push ebx; push esi; push edi ...
            pop edi; pop esi; pop ebx; leave; ret) and name [ebp + 8 + 4i] after --params.
Needs capstone (installed with reccmp).
"""

import argparse
import glob
import os
import re
import struct
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

JCC = {
    0x70: "jo", 0x71: "jno", 0x72: "jb", 0x73: "jae", 0x74: "je", 0x75: "jne", 0x76: "jbe", 0x77: "ja",
    0x78: "js", 0x79: "jns", 0x7A: "jp", 0x7B: "jnp", 0x7C: "jl", 0x7D: "jge", 0x7E: "jle", 0x7F: "jg",
}
MNEMONICS = {"pushal": "pushad", "popal": "popad"}
STRING_OP = re.compile(r"^(rep[a-z]* )?(movs|stos|lods|cmps|scas)[bwd]$")
PROLOGUE = ["push ebp", "mov ebp, esp", "push ebx", "push esi", "push edi"]
EPILOGUE = ["pop edi", "pop esi", "pop ebx", "leave", "ret"]


class Image:
    def __init__(self, path):
        self.data = open(path, "rb").read()
        pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        count = struct.unpack_from("<H", self.data, pe + 6)[0]
        optional_size = struct.unpack_from("<H", self.data, pe + 20)[0]
        base = struct.unpack_from("<I", self.data, pe + 24 + 28)[0]
        self.sections = []
        for i in range(count):
            offset = pe + 24 + optional_size + 40 * i
            virtual_size, virtual_address, raw_size, raw_pointer = struct.unpack_from("<IIII", self.data, offset + 8)
            self.sections.append((base + virtual_address, max(virtual_size, raw_size), raw_pointer))

    def read(self, address, size):
        for start, length, raw in self.sections:
            if start <= address < start + length:
                offset = raw + address - start
                return self.data[offset : offset + size]
        raise ValueError("0x%08x is outside the image" % address)


def annotated_names(module):
    """Address -> name for the module's annotated functions and globals."""
    names = {}
    marker = re.compile(r"\s*// (FUNCTION|STUB|GLOBAL|LIBRARY): %s 0x([0-9a-f]{8})" % module)
    paths = glob.glob(os.path.join(ROOT, module, "**", "*.c"), recursive=True)
    paths += glob.glob(os.path.join(ROOT, module, "**", "*.cpp"), recursive=True)
    paths += glob.glob(os.path.join(ROOT, module, "**", "*.h"), recursive=True)
    for path in paths:
        lines = open(path, errors="replace").read().split("\n")
        for i, line in enumerate(lines):
            m = marker.match(line)
            if not m:
                continue
            address = int(m.group(2), 16)
            for following in lines[i + 1 : i + 4]:
                if m.group(1) == "LIBRARY" and following.startswith("//"):
                    names[address] = following[2:].strip()
                    break
                if following.startswith("#") or following.startswith("//") or not following.strip():
                    continue
                n = re.search(r"([A-Za-z_]\w*)\s*(\(|\[|=|;)", following.replace("__declspec(naked)", ""))
                if n:
                    names[address] = n.group(1)
                break
    return names


def label(address):
    return "jmp_%08x" % address


def operands(text, names, params):
    def name(m):
        value = int(m.group(0), 16)
        if 0x10000000 <= value < 0x10200000:
            if value in names:
                return "@@" + names[value]
            return "0x%08x /* ??? */" % value
        return m.group(0)

    text = re.sub(r"0x[0-9a-f]+", name, text)
    text = re.sub(r"\[(\w+) \+ @@(\w+)\]", r"[\2 + \1]", text)
    text = re.sub(r"\[(\w+ \+ \w+\*\d) \+ @@(\w+)\]", r"[\2 + \1]", text)
    text = re.sub(r"\[(\w+\*\d) \+ @@(\w+)\]", r"[\2 + \1]", text)
    text = re.sub(r"\[@@(\w+)\]", r"[\1]", text)
    text = re.sub(r"@@(\w+)", r"offset \1", text)
    for i, param in enumerate(params):
        offset = 8 + 4 * i
        for spelling in ("dword ptr [ebp + 0x%x]" % offset, "dword ptr [ebp + %d]" % offset, "[ebp + 0x%x]" % offset, "[ebp + %d]" % offset):
            text = text.replace(spelling, param)
    return text


def transcribe(image, names, start, end, body, params):
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(start, end - start), start))
    while instructions and instructions[-1].mnemonic == "int3":
        end = instructions.pop().address
    if not instructions or instructions[-1].address + instructions[-1].size != end:
        raise SystemExit("the decode doesn't end at 0x%08x: check END (embedded data?)" % end)

    targets = set()
    for ins in instructions:
        b = ins.bytes
        if b[0] in JCC or b[0] in (0xE2, 0xE3, 0xE9, 0xEB) or (b[0] == 0x0F and 0x80 <= b[1] <= 0x8F):
            target = int(ins.op_str, 16)
            if start <= target < end:
                targets.add(target)

    if body:
        text = [(ins.mnemonic + " " + ins.op_str).strip() for ins in instructions]
        if text[:5] != PROLOGUE or text[-5:] != EPILOGUE or text.count("ret") != 1:
            raise SystemExit("--body needs the plain /Od frame and a single exit; transcribe it as naked")
        instructions = instructions[5:-5]

    out = []
    for ins in instructions:
        if ins.address in targets:
            out.append(label(ins.address) + ":")
        b = ins.bytes
        mnemonic = MNEMONICS.get(ins.mnemonic, ins.mnemonic)
        if b[0] in JCC:
            out.append("\t_emit 0x%02x /* %s %s */" % (b[0], JCC[b[0]], label(int(ins.op_str, 16))))
            out.append("\t_emit 0x%02x" % b[1])
        elif b[0] == 0x0F and 0x80 <= b[1] <= 0x8F:
            out.append("\t%s %s" % (JCC[b[1] - 0x10], label(int(ins.op_str, 16))))
        elif b[0] == 0xEB:
            out.append("\tjmp short %s" % label(int(ins.op_str, 16)))
        elif b[0] == 0xE9:
            target = int(ins.op_str, 16)
            if not start <= target < end:
                out.append("\tjmp %s" % names.get(target, "0x%08x /* ??? */" % target))
            elif target > ins.address:
                out.append("\tjmp %s" % label(target))
            else:
                out.append("\t_emit 0xe9 /* jmp %s */" % label(target))
                out += ["\t_emit 0x%02x" % x for x in b[1:]]
        elif b[0] in (0xE2, 0xE3):
            out.append("\t%s %s" % (mnemonic, label(int(ins.op_str, 16))))
        elif b[0] == 0xE8:
            target = int(ins.op_str, 16)
            out.append("\tcall %s" % names.get(target, "0x%08x /* ??? */" % target))
        elif STRING_OP.match(mnemonic):
            out.append("\t" + mnemonic)
        elif mnemonic in ("xchg", "test") and len(b) == 2 and b[1] >= 0xC0:
            out.append("\t_emit 0x%02x /* %s %s */" % (b[0], mnemonic, ins.op_str))
            out.append("\t_emit 0x%02x" % b[1])
        else:
            out.append(("\t%s %s" % (mnemonic, operands(ins.op_str, names, params))).rstrip())
    return "\n".join(out)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", required=True, help="MW2 or MW2SHELL")
    parser.add_argument("--dll", help="The original DLL (default: <target>.DLL in the repository root)")
    parser.add_argument("start", help="The routine's address (hex)")
    parser.add_argument("end", help="The next routine's address (hex)")
    parser.add_argument("--body", action="store_true", help="Leave out the /Od frame")
    parser.add_argument("--params", default="", help="Parameter names for --body, comma-separated")
    args = parser.parse_args()

    image = Image(args.dll or os.path.join(ROOT, args.target + ".DLL"))
    names = annotated_names(args.target)
    params = [p for p in args.params.split(",") if p]
    print(transcribe(image, names, int(args.start, 16), int(args.end, 16), args.body, params))
    return 0


if __name__ == "__main__":
    sys.exit(main())
