#!/usr/bin/env python3
"""Port reccmp annotations from one original binary to another build of the same code.

For every function the source module annotates, look for a function in the target binary
whose bytes are the same, apart from relocated addresses and the displacements of calls and
jumps that leave the function. Functions found more than once (empty functions, getters)
are placed by their neighbours: inside an object, the target keeps the source's function
order. Matches whose calls disagree with the other matches are dropped.

Globals are mapped through the matched code: each relocation the two functions share pairs
a source address with a target address, and an annotated global (or a row of the module's
data-source CSV) takes the target address when every pair that falls inside it agrees on
the displacement.

    python tools/port_annotations.py --from MW2 MW2.DLL --to MW2MATROX MW2_MATROX.DLL \
        --dirs MW2 common [--csv reccmp/mw2-vfx.csv:reccmp/mw2matrox-vfx.csv] [--write]

With --write, the target's annotation goes on the line after the source's (the target
follows the source in reccmp-project.yml's order). Without it, the tool prints statistics.
--library-from MODULE BINARY HEADER adds the C runtime: LIBRARY entries of another module
(the same CRT's release build, say) matched the same way, written to the target's
library_msvc.h, stacked on an existing entry for the same symbol or appended at its end.
"""

import argparse
import csv
import glob
import os
import re
import struct
import sys
from collections import defaultdict

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_EBP

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MARKER = re.compile(r"^(\s*)// (FUNCTION|LIBRARY|GLOBAL|SYNTHETIC|STUB): (\w+) (0x[0-9a-f]+)(.*)$")
PREFIX = 48


class Image:
    def __init__(self, path):
        self.data = open(path, "rb").read()
        pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        count = struct.unpack_from("<H", self.data, pe + 6)[0]
        optional_size = struct.unpack_from("<H", self.data, pe + 20)[0]
        optional = pe + 24
        self.base = struct.unpack_from("<I", self.data, optional + 28)[0]
        self.sections = {}
        for i in range(count):
            offset = optional + optional_size + 40 * i
            name = self.data[offset : offset + 8].rstrip(b"\0").decode()
            vsize, vaddr, rsize, rptr = struct.unpack_from("<IIII", self.data, offset + 8)
            self.sections[name] = (self.base + vaddr, vsize, rsize, rptr)
        reloc_rva, reloc_size = struct.unpack_from("<II", self.data, optional + 96 + 5 * 8)
        self.relocs = set()
        offset, end = self.offset(self.base + reloc_rva), self.offset(self.base + reloc_rva) + reloc_size
        while offset < end:
            page, size = struct.unpack_from("<II", self.data, offset)
            if size == 0:
                break
            for i in range(8, size, 2):
                entry = struct.unpack_from("<H", self.data, offset + i)[0]
                if entry >> 12 == 3:
                    self.relocs.add(self.base + page + (entry & 0xFFF))
            offset += size
        start, vsize, rsize, rptr = self.sections[".text"]
        self.text = (start, start + min(vsize, rsize))
        self.text_bytes = self.data[rptr : rptr + min(vsize, rsize)]

    def offset(self, address):
        for start, vsize, rsize, rptr in self.sections.values():
            if start <= address < start + max(vsize, rsize):
                if address - start >= rsize:
                    return None
                return rptr + address - start
        return None

    def read(self, address, size):
        offset = self.offset(address)
        return self.data[offset : offset + size]

    def u32(self, address):
        return struct.unpack_from("<I", self.data, self.offset(address))[0]


class Function:
    """A function of the source binary: its bytes, which of them to ignore, and its calls."""

    def __init__(self, image, start, end, disasm):
        while end > start and image.read(end - 1, 1) == b"\xcc":
            end -= 1
        self.start, self.end = start, end
        self.code = image.read(start, end - start)
        self.mask = bytearray(len(self.code))
        self.relocs = []
        for address in range(start, end):
            if address in image.relocs:
                self.relocs.append(address - start)
                self.mask[address - start : address - start + 4] = b"\1\1\1\1"
        # Branches out of the function: their displacements depend on the layout.
        self.branches = []
        for insn in disasm.disasm(self.code, start):
            if insn.id == 0:  # data
                continue
            if insn.group(1) or insn.group(2) or insn.group(7):  # jump, call, relative branch
                ops = insn.operands
                if len(ops) == 1 and ops[0].type == X86_OP_IMM and insn.size >= 5:
                    target = ops[0].imm
                    if not start <= target < end:
                        at = insn.address + insn.size - 4 - start
                        self.mask[at : at + 4] = b"\1\1\1\1"
                        self.branches.append((at, target))
        self.relocs_set = set(self.relocs)
        self.find_slots(disasm)

    def find_slots(self, disasm):
        """The [ebp-N] displacements: a stack-slot permutation changes only these."""
        self.slots = []
        self.slot_mask = bytearray(len(self.code))
        for insn in disasm.disasm(self.code, self.start):
            if insn.id == 0 or not insn.disp_size:
                continue
            for op in insn.operands:
                if op.type == X86_OP_MEM and op.mem.base == X86_REG_EBP and op.mem.disp < 0:
                    at = insn.address - self.start + insn.disp_offset
                    if not any(self.mask[at : at + insn.disp_size]):
                        self.slots.append((at, insn.disp_size))
                        self.slot_mask[at : at + insn.disp_size] = b"\1" * insn.disp_size

    def pattern(self):
        n = min(PREFIX, len(self.code))
        parts = []
        for i in range(n):
            parts.append(b"." if self.mask[i] or self.slot_mask[i] else re.escape(self.code[i : i + 1]))
        return re.compile(b"(?=" + b"".join(parts) + b")", re.DOTALL)

    def matches(self, image, address):
        """How the target's bytes at address compare with this function's: "same", "slots"
        (the same but for a consistent permutation of its [ebp-N] slots) or None."""
        if not (image.text[0] <= address and address + len(self.code) <= image.text[1]):
            return None
        code = image.read(address, len(self.code))
        if len(code) != len(self.code):
            return None
        for i, (a, b) in enumerate(zip(self.code, code)):
            if a != b and not self.mask[i] and not self.slot_mask[i]:
                return None
        relocs = {r - address for r in range(address, address + len(code)) if r in image.relocs}
        if relocs != self.relocs_set:
            return None
        forward, backward = {}, {}
        for at, size in self.slots:
            a = int.from_bytes(self.code[at : at + size], "little", signed=True)
            b = int.from_bytes(code[at : at + size], "little", signed=True)
            if forward.setdefault(a, b) != b or backward.setdefault(b, a) != a:
                return None
        return "same" if all(a == b for a, b in forward.items()) else "slots"

    def branch_targets(self, image, address):
        result = []
        for at, target in self.branches:
            displacement = struct.unpack_from("<i", image.read(address + at, 4))[0]
            result.append((target, (address + at + 4 + displacement) & 0xFFFFFFFF))
        return result


def scan_markers(dirs):
    """Every annotation marker under dirs: (path, line index, kind, module, address, rest)."""
    markers = []
    for d in dirs:
        for ext in ("c", "cpp", "h"):
            for path in sorted(glob.glob(os.path.join(ROOT, d, "**", "*." + ext), recursive=True)):
                for i, line in enumerate(open(path, encoding="utf-8", errors="replace").read().split("\n")):
                    m = MARKER.match(line)
                    if m:
                        markers.append((path, i, m.group(2), m.group(3), int(m.group(4), 16), m.group(5)))
    return markers


def match_functions(source, target, starts, wanted, log):
    """Map source function addresses (wanted) to target addresses. starts: every known
    function start of the source, for the extents."""
    disasm = Cs(CS_ARCH_X86, CS_MODE_32)
    disasm.skipdata = True
    disasm.detail = True
    text_end = source.text[1]
    ordered = sorted(set(starts) | {text_end})
    following = {a: b for a, b in zip(ordered, ordered[1:])}
    functions = {a: Function(source, a, following[a], disasm) for a in sorted(wanted)}

    candidates = {}
    kinds = {}
    for a, f in functions.items():
        if len(f.code) < 4:
            continue
        found = []
        for m in f.pattern().finditer(target.text_bytes):
            address = target.text[0] + m.start()
            kind = f.matches(target, address)
            if kind:
                found.append(address)
                kinds[(a, address)] = kind
        candidates[a] = found

    mapping = {a: c[0] for a, c in candidates.items() if len(c) == 1}
    taken = defaultdict(list)
    for a, t in mapping.items():
        taken[t].append(a)
    for t, sources in taken.items():
        if len(sources) > 1:
            for a in sources:
                del mapping[a]

    def adjacent(end, start):
        """The target's next function after one ending at end can start at start."""
        if start < end or start - end >= 16:
            return False
        return start == end or (start % 16 == 0 and target.read(end, start - end) == b"\xcc" * (start - end))

    order = sorted(functions)
    changed = True
    while changed:
        changed = False
        used = set(mapping.values())
        for i, a in enumerate(order):
            if a in mapping or len(candidates.get(a, [])) < 2:
                continue
            choices = set()
            if i > 0 and order[i - 1] in mapping:
                prev = order[i - 1]
                end = mapping[prev] + len(functions[prev].code)
                choices |= {c for c in candidates[a] if adjacent(end, c)}
            if i + 1 < len(order) and order[i + 1] in mapping:
                nxt = order[i + 1]
                choices |= {c for c in candidates[a] if adjacent(c + len(functions[a].code), mapping[nxt])}
            choices -= used
            if len(choices) == 1:
                mapping[a] = choices.pop()
                changed = True

    # Calls must agree with the other matches.
    inverse = {t: a for a, t in mapping.items()}
    dropped = 0
    for a in list(mapping):
        for src_target, tgt_target in functions[a].branch_targets(target, mapping[a]):
            if src_target in mapping and mapping[src_target] != tgt_target:
                log.append("call mismatch in 0x%08x: 0x%08x -> 0x%08x, not 0x%08x"
                           % (a, src_target, tgt_target, mapping[src_target]))
                del mapping[a]
                dropped += 1
                break
            if src_target not in mapping and tgt_target in inverse:
                log.append("call mismatch in 0x%08x: 0x%08x -> 0x%08x, which matches 0x%08x"
                           % (a, src_target, tgt_target, inverse[tgt_target]))
                del mapping[a]
                dropped += 1
                break
    ambiguous = sum(1 for a in functions if a not in mapping and len(candidates.get(a, [])) > 1)
    permuted = {a for a, t in mapping.items() if kinds[(a, t)] == "slots"}
    return mapping, functions, ambiguous, dropped, permuted


def identical_pairs(source, target, mapping, functions):
    """(source address, target address) of every relocation the matched functions share."""
    pairs = []
    for a, t in mapping.items():
        for r in functions[a].relocs:
            pairs.append((source.u32(a + r), target.u32(t + r)))
    return pairs


def shape(insn):
    return insn.mnemonic + " " + re.sub(r"0x[0-9a-f]+|\b\d+\b", "N", insn.op_str)


def aligned_pairs(source, target, mapping, functions, log):
    """Relocation pairs from the functions that didn't match, placed by their matched
    neighbours: the instructions of each pair are aligned by their shape (mnemonic and
    operands, numbers aside), and aligned instructions with a relocation at the same place
    pair its two addresses. Also returns those placements."""
    disasm = Cs(CS_ARCH_X86, CS_MODE_32)
    disasm.skipdata = True
    order = sorted(functions)
    placed = {}
    i = 0
    while i < len(order):
        if order[i] in mapping:
            i += 1
            continue
        j = i
        while j < len(order) and order[j] not in mapping:
            j += 1
        # order[i:j] didn't match; order[i - 1] and order[j] did.
        if i == 0 or j == len(order):
            i = j
            continue
        prev, nxt = order[i - 1], order[j]
        run = order[i:j]
        if functions[prev].end != run[0] or any(functions[a].end != b for a, b in zip(run, run[1:] + [nxt])):
            i = j  # padding in the source: an object boundary inside the run
            continue
        start, end = mapping[prev] + len(functions[prev].code), mapping[nxt]
        size = sum(len(functions[a].code) for a in run)
        if not 0 < end - start < 2 * size + 64:
            i = j
            continue
        code = target.read(start, end - start)
        if len(run) == 1:
            starts = [start]
        else:
            # The /Od prologue after a ret.
            starts = [start] + [start + k for k in range(1, len(code) - 3)
                                if code[k : k + 3] == b"\x55\x8b\xec" and (code[k - 1] == 0xC3 or code[k - 3] == 0xC2)]
        if len(starts) != len(run):
            i = j
            continue
        for a, b, b_end in zip(run, starts, starts[1:] + [end]):
            placed[a] = (b, b_end)
        i = j

    pairs = []
    for a, (b, b_end) in placed.items():
        f = functions[a]
        src = [x for x in disasm.disasm(f.code, a) if x.id]
        tgt = [x for x in disasm.disasm(target.read(b, b_end - b), b) if x.id]
        import difflib

        matcher = difflib.SequenceMatcher(None, [shape(x) for x in src], [shape(x) for x in tgt], autojunk=False)
        for block in matcher.get_matching_blocks():
            for k in range(block.size):
                x, y = src[block.a + k], tgt[block.b + k]
                for off in range(x.size):
                    if x.address + off in source.relocs and y.address + off in target.relocs:
                        pairs.append((source.u32(x.address + off), target.u32(y.address + off)))
    return pairs, placed


MIRROR = {"jl": "jg", "jg": "jl", "jle": "jge", "jge": "jle", "jb": "ja", "ja": "jb", "jbe": "jae", "jae": "jbe"}


def swapped_comparison(window_s, window_t, base_s, base_t, source, target, key):
    """True for mov r, X; cmp Y, r; jcc against mov r, Y; cmp X, r; the mirrored jcc."""
    if len(window_s) != 3 or len(window_t) != 3:
        return False
    (ms, cs, js), (mt, ct, jt) = window_s, window_t
    if [ms.mnemonic, cs.mnemonic] != ["mov", "cmp"] or [mt.mnemonic, ct.mnemonic] != ["mov", "cmp"]:
        return False
    if MIRROR.get(js.mnemonic) != jt.mnemonic:
        return False

    def parts(z, base, image):
        return [p.strip() for p in key(z, base, image).split(" ", 1)[1].split(",")]

    reg_s, x_s = parts(ms, base_s, source)
    y_s, r2_s = parts(cs, base_s, source)
    reg_t, x_t = parts(mt, base_t, target)
    y_t, r2_t = parts(ct, base_t, target)
    return reg_s == r2_s == reg_t == r2_t and x_s == y_t and y_s == x_t


def flipped_equivalent(source, target, f, address, end):
    """True when the target's function at address is the source function f but for the
    operand order of some comparisons (the symbol-table entropy of VC++ 4.x: the operands of a
    cmp swap places and its jump mirrors) and a permutation of its stack slots, with the same
    size."""
    from collections import Counter

    if end - address != len(f.code):
        return False
    disasm = Cs(CS_ARCH_X86, CS_MODE_32)
    disasm.skipdata = True
    disasm.detail = True
    src = [x for x in disasm.disasm(f.code, f.start) if x.id]
    tgt = [x for x in disasm.disasm(target.read(address, end - address), address) if x.id]
    if len(src) != len(tgt):
        return False

    slot = re.compile(r"ebp - (0x[0-9a-f]+|\d+)")

    def masked(x, image):
        # The instruction with its addresses masked: relocated operands and branch targets.
        text = x.mnemonic + " " + x.op_str
        if any(x.address + k in image.relocs for k in range(x.size)) or x.group(1) or x.group(2) or x.group(7):
            text = re.sub(r"0x[0-9a-f]{5,}", "A", text)
        return text

    # The target's stack slots in the source's terms, by majority over the instructions that
    # only differ in their slots (a permutation, as Function.matches allows).
    votes = defaultdict(Counter)
    for x, y in zip(src, tgt):
        a, b = masked(x, source), masked(y, target)
        if slot.sub("S", a) == slot.sub("S", b):
            for u, v in zip(slot.findall(a), slot.findall(b)):
                votes[v][u] += 1
    slots = {v: c.most_common(1)[0][0] for v, c in votes.items()}
    if len(set(slots.values())) != len(slots):
        return False

    def key(x, base, image):
        text = masked(x, image)
        if image is target:
            text = slot.sub(lambda m: "ebp - " + slots.get(m.group(1), "?"), text)
        return text

    window_s, window_t, flipped = [], [], False
    for x, y in zip(src + [None], tgt + [None]):
        if x is not None and key(x, f.start, source) == key(y, address, target):
            if window_s:
                return False
            continue
        if x is not None:
            window_s.append(x)
            window_t.append(y)
            # A window closes at its jump.
            if not (x.group(1) or x.group(7)):
                continue
        if not window_s:
            continue
        if not swapped_comparison(window_s, window_t, f.start, address, source, target, key):
            return False
        flipped = True
        window_s, window_t = [], []
    return flipped


def match_globals(source, target, exact_pairs, near_pairs, globals_, log):
    """Map the source's global addresses through relocation pairs. Each global takes the
    displacement that a majority of the pairs addressing its first byte agree on, or without
    those, of the pairs that fall inside it. Pairs of identical functions count three times."""
    import bisect
    from collections import Counter

    text = source.text
    ordered = sorted(set(globals_))
    votes = defaultdict(Counter)
    exact = defaultdict(Counter)
    for pairs, weight in ((exact_pairs, 3), (near_pairs, 1)):
        for s, t in pairs:
            if text[0] <= s < text[1]:
                continue
            i = bisect.bisect_right(ordered, s) - 1
            if i < 0:
                continue
            g = ordered[i]
            votes[g][t - s] += weight
            if s == g:
                exact[g][t - s] += weight
    result = {}
    for g in ordered:
        # References to the global's first byte decide; past it, the range may run into
        # data the source doesn't annotate.
        ballot = exact[g] or votes[g]
        if not ballot:
            continue
        (delta, best), *rest = ballot.most_common()
        others = sum(n for _, n in rest)
        if best > others:
            result[g] = g + delta
            if rest:
                log.append("global 0x%08x: took displacement %d (%d votes) over %s"
                           % (g, delta, best, ", ".join("%d (%d)" % x for x in rest)))
        else:
            log.append("global 0x%08x: no majority among %s" % (g, dict(ballot)))
    seen = defaultdict(list)
    for g, t in result.items():
        seen[t].append(g)
    for t, gs in seen.items():
        if len(gs) > 1:
            log.append("globals %s all map to 0x%08x; dropped" % (", ".join("0x%08x" % g for g in gs), t))
            for g in gs:
                del result[g]
    return result


def longest_increasing(values):
    """Indices of a longest strictly increasing subsequence of values."""
    import bisect

    tails, tail_index, previous = [], [], [-1] * len(values)
    for k, v in enumerate(values):
        j = bisect.bisect_left(tails, v)
        if j == len(tails):
            tails.append(v)
            tail_index.append(k)
        else:
            tails[j] = v
            tail_index[j] = k
        previous[k] = tail_index[j - 1] if j else -1
    result, k = set(), tail_index[-1] if tail_index else -1
    while k >= 0:
        result.add(k)
        k = previous[k]
    return result


def insert_after(edits, path, index, line):
    edits[path].append((index, line))


def apply_edits(edits):
    for path, items in edits.items():
        lines = open(path, encoding="utf-8").read().split("\n")
        for index, line in sorted(items, key=lambda x: -x[0]):
            lines.insert(index + 1, line)
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join(lines))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--from", dest="source", nargs=2, metavar=("MODULE", "BINARY"), required=True)
    parser.add_argument("--to", dest="target", nargs=2, metavar=("MODULE", "BINARY"), required=True)
    parser.add_argument("--dirs", nargs="+", required=True, help="source directories to scan and edit")
    parser.add_argument("--csv", action="append", default=[], metavar="SOURCE:TARGET",
                        help="a data-source CSV of the source module, and the target's to write")
    parser.add_argument("--library-from", nargs=3, metavar=("MODULE", "BINARY", "HEADER"),
                        help="match another module's LIBRARY entries too")
    parser.add_argument("--library-to", metavar="HEADER", help="the target's library_msvc.h")
    parser.add_argument("--write", action="store_true")
    parser.add_argument("--log", help="write the details here")
    parser.add_argument("--unmatched", help="list the source functions without a match here")
    args = parser.parse_args()

    src_module, tgt_module = args.source[0], args.target[0]
    source, target = Image(args.source[1]), Image(args.target[1])
    markers = scan_markers(args.dirs)
    log = []

    existing = {(path, i) for path, i, kind, module, address, rest in markers if module == tgt_module}
    src = [m for m in markers if m[3] == src_module]
    starts = [m[4] for m in src if m[2] in ("FUNCTION", "LIBRARY", "SYNTHETIC", "STUB")]
    # A LIBRARY entry named by its CodeView name only exists in the source's CRT build.
    code = [m for m in src if m[2] in ("FUNCTION", "SYNTHETIC") or (m[2] == "LIBRARY" and m[5].strip() == "SYMBOL")]
    mapping, functions, ambiguous, dropped, permuted = match_functions(
        source, target, starts, [m[4] for m in code], log)

    game = [m for m in code if m[2] == "FUNCTION"]
    game_found = sum(1 for m in game if m[4] in mapping)
    print("%s functions: %d of %d matched in %s, %d of them with permuted stack slots"
          " (%d ambiguous, %d dropped for their calls)"
          % (src_module, game_found, len(game), tgt_module, sum(1 for m in game if m[4] in permuted),
             ambiguous, dropped))
    lib = [m for m in code if m[2] == "LIBRARY"]
    print("%s library functions: %d of %d" % (src_module, sum(1 for m in lib if m[4] in mapping), len(lib)))

    csv_rows = {}
    for spec in args.csv:
        src_csv, tgt_csv = spec.split(":")
        with open(src_csv, newline="") as f:
            lines = [line for line in f if not line.startswith("#")]
        csv_rows[(src_csv, tgt_csv)] = list(csv.DictReader(lines))
    globals_ = [m[4] for m in src if m[2] == "GLOBAL"]
    for rows in csv_rows.values():
        globals_ += [int(r["address"], 16) for r in rows]
    exact_pairs = identical_pairs(source, target, mapping, functions)
    near_pairs, placed = aligned_pairs(source, target, mapping, functions, log)
    print("%s functions placed by their neighbours: %d" % (src_module, sum(1 for m in game if m[4] in placed)))
    global_map = match_globals(source, target, exact_pairs, near_pairs, globals_, log)
    # The functions placed by their neighbours that only differ in the operand order of
    # comparisons are the same source: annotate them too.
    flipped = set()
    taken = set(mapping.values())
    for a, (b, end) in placed.items():
        if b not in taken and flipped_equivalent(source, target, functions[a], b, end):
            mapping[a] = b
            flipped.add(a)
    print("%s functions placed by their neighbours that only differ in comparison order: %d"
          % (src_module, sum(1 for m in game if m[4] in flipped)))
    annotated_globals = [m for m in src if m[2] == "GLOBAL"]
    print("%s globals: %d of %d mapped" % (src_module, sum(1 for m in annotated_globals if m[4] in global_map),
                                          len(annotated_globals)))

    # A unit's functions must be in address order for every module: where the target moved
    # some of a file's functions, keep the longest run in order and leave the others out.
    by_file = defaultdict(list)
    for path, i, kind, module, address, rest in code:
        if kind == "FUNCTION" and address in mapping:
            by_file[path].append((i, address))
    out_of_order = set()
    for path, items in by_file.items():
        items.sort()
        keep = longest_increasing([mapping[a] for _, a in items])
        for k, (i, a) in enumerate(items):
            if k not in keep:
                out_of_order.add(a)
                log.append("0x%08x (%s:%d) at 0x%08x is out of order in the target; left out"
                           % (a, os.path.relpath(path, ROOT), i + 1, mapping[a]))
    if out_of_order:
        print("%s functions out of their file's order in %s, left out: %d" % (src_module, tgt_module, len(out_of_order)))
    for a in out_of_order:
        del mapping[a]

    edits = defaultdict(list)
    for path, i, kind, module, address, rest in code + annotated_globals:
        new = mapping.get(address) if kind != "GLOBAL" else global_map.get(address)
        if new is None:
            continue
        # Skip markers already followed by one for the target.
        if (path, i + 1) in existing:
            continue
        indent = re.match(r"\s*", open(path, encoding="utf-8").read().split("\n")[i]).group(0)
        insert_after(edits, path, i, "%s// %s: %s 0x%08x%s" % (indent, kind, tgt_module, new, rest))

    library_entries = []
    if args.library_from:
        lib_module, lib_binary, lib_header = args.library_from
        lib_image = Image(lib_binary)
        lib_markers = scan_markers([os.path.dirname(os.path.relpath(lib_header, ROOT))])
        lib_src = [m for m in lib_markers if m[3] == lib_module]
        lib_starts = [m[4] for m in lib_src if m[2] in ("FUNCTION", "LIBRARY", "SYNTHETIC", "STUB")]
        lib_code = [m for m in lib_src if m[2] == "LIBRARY" and m[1] is not None]
        lib_map, lib_functions, lib_ambiguous, lib_dropped, lib_permuted = match_functions(
            lib_image, target, lib_starts, [m[4] for m in lib_code], log)
        taken = set(mapping.values())
        for path, i, kind, module, address, rest in lib_code:
            # Short ones (a lone ret) turn up anywhere.
            if address in lib_map and lib_map[address] not in taken and len(lib_functions[address].code) >= 8:
                lines = open(path, encoding="utf-8").read().split("\n")
                symbol = lines[i + 1][2:].strip() if rest.strip() == "SYMBOL" and lines[i + 1].startswith("//") else None
                library_entries.append((lib_map[address], rest, symbol))
                taken.add(lib_map[address])
        print("%s library functions: %d of %d" % (lib_module, len(library_entries), len(lib_code)))

    if args.log:
        with open(args.log, "w") as f:
            f.write("\n".join(log) + "\n")
    if args.unmatched:
        with open(args.unmatched, "w") as f:
            for m in game:
                if m[4] not in mapping:
                    where = " -> 0x%08x" % placed[m[4]][0] if m[4] in placed else ""
                    f.write("0x%08x%s %s:%d\n" % (m[4], where, os.path.relpath(m[0], ROOT), m[1] + 1))

    if not args.write:
        return

    if library_entries and args.library_to:
        args.library_to = os.path.abspath(args.library_to)
        lines = open(args.library_to, encoding="utf-8").read().split("\n")
        symbols = {}
        for i, line in enumerate(lines):
            m = MARKER.match(line)
            if m and m.group(3) == src_module and m.group(5).strip() == "SYMBOL" and i + 1 < len(lines):
                symbols[lines[i + 1][2:].strip()] = i
        appended = []
        already = {(m[4]) for m in markers if m[3] == tgt_module and m[2] == "LIBRARY"}
        for address, rest, symbol in sorted(library_entries):
            if address in already or symbol is None:
                continue
            if symbol in symbols:
                insert_after(edits, args.library_to, symbols[symbol], "// LIBRARY: %s 0x%08x SYMBOL" % (tgt_module, address))
            else:
                appended.append("// LIBRARY: %s 0x%08x SYMBOL\n// %s\n" % (tgt_module, address, symbol))
        if appended:
            edits[args.library_to]  # created
            apply_edits(edits)
            edits = defaultdict(list)
            text = open(args.library_to, encoding="utf-8").read()
            end = text.rstrip().rfind("#endif")
            text = text[:end] + "// %s's C runtime functions that only %s's matched (tools/port_annotations.py).\n\n" \
                % (tgt_module, args.library_from[0]) \
                + "\n".join(appended) + "\n" + text[end:]
            with open(args.library_to, "w", encoding="utf-8", newline="\n") as f:
                f.write(text)
    apply_edits(edits)

    for (src_csv, tgt_csv), rows in csv_rows.items():
        with open(src_csv, newline="") as f:
            header = [line for line in f if line.startswith("#")]
        fields = list(rows[0].keys()) if rows else ["address", "type", "name"]
        with open(tgt_csv, "w", newline="") as f:
            f.write("# Ported from %s by tools/port_annotations.py.\n" % os.path.basename(src_csv))
            writer = csv.DictWriter(f, fieldnames=fields, lineterminator="\n")
            writer.writeheader()
            for row in rows:
                address = int(row["address"], 16)
                if address in global_map:
                    row = dict(row, address="0x%08x" % global_map[address])
                    writer.writerow(row)


if __name__ == "__main__":
    main()
