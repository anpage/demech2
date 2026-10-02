#!/usr/bin/env python3
"""List the basic blocks of tests/asmequiv's reference routines, for its coverage check.

Reads the reference DLL (asmref.dll, built by the decomp build: the matched assembly) and writes,
for each exported routine, the offset of every basic block from the routine's start and the
block's first byte:

    FUN_100074e0 0:55 1a:33 2c:8a ...

`asmequiv REF CANDIDATE -coverage FILE` puts an int3 on each block, and fails unless its cases
reach every block of the routines it runs. Blocks come from a recursive descent from each
routine's entry (conditional jumps start two blocks, calls return), so the dead jmp /Od emits
after a return isn't one. An incremental link's export is a jmp to the routine: it is followed,
as the driver does.

usage: python tools/asmblocks.py REF.dll -o BLOCKS.txt
Needs capstone (installed with reccmp).
"""

import argparse
import os
import struct
import sys

from capstone import CS_ARCH_X86, CS_MODE_32, Cs

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from asm2c import JCC, Image  # noqa: E402

IMAGE_SCN_MEM_EXECUTE = 0x20000000


class Dll(Image):
    def __init__(self, path):
        super().__init__(path)
        data = self.data
        pe = struct.unpack_from("<I", data, 0x3C)[0]
        count = struct.unpack_from("<H", data, pe + 6)[0]
        optional_size = struct.unpack_from("<H", data, pe + 20)[0]
        self.base = struct.unpack_from("<I", data, pe + 24 + 28)[0]
        self.code = []
        for i in range(count):
            offset = pe + 24 + optional_size + 40 * i
            virtual_size, virtual_address = struct.unpack_from("<II", data, offset + 8)
            characteristics = struct.unpack_from("<I", data, offset + 36)[0]
            if characteristics & IMAGE_SCN_MEM_EXECUTE:
                self.code.append((self.base + virtual_address, virtual_size))
        export_rva = struct.unpack_from("<I", data, pe + 24 + 96)[0]
        self.exports = {}
        if export_rva:
            directory = self.read(self.base + export_rva, 40)
            ordinal_base, functions, names, function_table, name_table, ordinal_table = struct.unpack_from(
                "<IIIIII", directory, 16
            )
            for i in range(names):
                name_rva = struct.unpack("<I", self.read(self.base + name_table + 4 * i, 4))[0]
                ordinal = struct.unpack("<H", self.read(self.base + ordinal_table + 2 * i, 2))[0]
                rva = struct.unpack("<I", self.read(self.base + function_table + 4 * ordinal, 4))[0]
                name = self.read(self.base + name_rva, 256).split(b"\0")[0].decode()
                self.exports[name] = self.base + rva

    def is_code(self, address):
        return any(start <= address < start + size for start, size in self.code)


def is_conditional(ins):
    b = ins.bytes
    return b[0] in JCC or b[0] in (0xE0, 0xE1, 0xE2, 0xE3) or (b[0] == 0x0F and 0x80 <= b[1] <= 0x8F)


def blocks(dll, start):
    """The routine's block starts, from a recursive descent."""
    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    leaders = {start}
    seen = set()
    work = [start]
    while work:
        address = work.pop()
        while address not in seen:
            seen.add(address)
            ins = next(disassembler.disasm(dll.read(address, 16), address), None)
            if ins is None:
                raise SystemExit("can't decode 0x%08x" % address)
            following = address + ins.size
            if ins.mnemonic == "ret":
                break
            if ins.mnemonic == "jmp" or is_conditional(ins):
                if not ins.op_str.startswith("0x"):
                    raise SystemExit("indirect jump at 0x%08x: %s %s" % (address, ins.mnemonic, ins.op_str))
                target = int(ins.op_str, 16)
                leaders.add(target)
                work.append(target)
                if ins.mnemonic == "jmp":
                    break
                leaders.add(following)
                work.append(following)
                break
            address = following
    return sorted(leaders)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("dll")
    parser.add_argument("-o", "--output", required=True)
    args = parser.parse_args()

    dll = Dll(args.dll)
    lines = [
        "# The basic blocks of %s's routines (tools/asmblocks.py): offset:first byte, in hex."
        % os.path.basename(args.dll)
    ]
    for name, address in sorted(dll.exports.items(), key=lambda item: item[1]):
        if not dll.is_code(address):
            continue
        code = dll.read(address, 5)
        if code[0] == 0xE9:
            address += 5 + struct.unpack_from("<i", code, 1)[0]
        starts = blocks(dll, address)
        lines.append(
            name + " " + " ".join("%x:%02x" % (start - address, dll.read(start, 1)[0]) for start in starts)
        )
    with open(args.output, "w", newline="\n") as file:
        file.write("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
