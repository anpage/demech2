#!/usr/bin/env python3
"""Prove that a function differs from the original only by compiler entropy.

At /Od the compilers give every named local a stack slot in the order of a hash of its name, and
number the duplicates of a nested scope (`i$99`) with a counter that every symbol declared ahead
of the function advances. Neither is recoverable from the original: its locals' names are lost.
So a function can be right and still differ from the original in its `[ebp-N]` operands.

This tool looks for a witness: a variant of the unit that changes only the locals' names or the
number of symbols declared ahead of it (`--pads N`: up to N typedefs ahead of the unit), and
compiles the function to the original's bytes. A witness proves that the function's tokens are
right. The source keeps its readable names; the witness is a record.

For each unit it compiles the source as the build does (tools/unitbuild.py), compares each
annotated function with the original instruction by instruction, and reports one of:

  exact      the bytes are the original's (relocated fields compared by their targets)
  witness    exact once the locals are renamed and/or with N symbols declared ahead
  slots      the only differences are stack slots, but no renaming tried reproduces them
  other      the first difference isn't a stack slot: it is printed

The comparison is of the whole original function (reccmp reads only as many bytes as the rebuilt
function has): it walks both until the rebuilt function's end, then requires the original's next
bytes to be padding or the next annotated function. A relocated field is checked against the
annotation of the symbol it names, a string or constant against the original's bytes at that
address; a symbol with neither is counted as unverified, not as a difference.

The renaming: where the only differences are `[ebp-N]` operands, the order in which the original
allocated the named locals follows from them (each local must map to one place, without overlap).
The locals are renamed, in a scratch copy, to names the compiler allocates in that order: a probe
compiled twice (its locals declared forward, then reversed) finds a ladder of such names, one per
bucket of the compiler's hash, without knowing the hash.

usage (from the repository root, with reccmp's environment):
  python tools/witness.py --target MW2 [--build build-wine] [--wine PATH] [--unit ai.c ...]
                          [--function NAME ...] [--pads N] [--jobs N] [--write | --check]
                          [--json FILE] [-v]

--write records the witnesses in tools/witnesses/<TARGET>.txt; --check verifies that file: every
record must still give an exact function, and a function that is exact without its record fails
(remove the record).

C units only, for now: a C++ unit's functions are reported as skipped.
"""

import argparse
import json
import os
import re
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

from capstone import CS_ARCH_X86, CS_GRP_CALL, CS_GRP_JUMP, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG, X86_REG_EBP
from reccmp.dir import walk_source_dir
from reccmp.formats import TextFile
from reccmp.parser import DecompCodebase
from reccmp.parser.marker import MarkerType

sys.path.insert(0, str(Path(__file__).resolve().parent))
from unitbuild import (  # noqa: E402
    IMAGE_REL_I386_DIR32,
    IMAGE_REL_I386_REL32,
    IMAGE_SYM_CLASS_EXTERNAL,
    ROOT,
    TARGETS,
    Compiler,
    Image,
    Object,
    Request,
    Target,
    Unit,
)

PAD = "typedef int ZqWitnessPad%d;\n"


def pads_text(count):
    return "".join(PAD % i for i in range(count))


def read_yaml_paths():
    """The originals' paths from reccmp-user.yml (a flat file: no YAML library needed)."""
    paths, target = {}, None
    user = ROOT / "reccmp-user.yml"
    if not user.exists():
        return paths
    for line in user.read_text().split("\n"):
        m = re.match(r"^  (\w+):\s*$", line)
        if m:
            target = m.group(1)
        m = re.match(r"^\s+path:\s*(\S.*)$", line)
        if m and target:
            paths[target] = m.group(1).strip().strip("'\"")
    return paths


def source_roots(target):
    """The target's source roots, from reccmp-project.yml."""
    roots, inside, in_roots = [], False, False
    for line in (ROOT / "reccmp-project.yml").read_text().split("\n"):
        if re.match(r"^  \w+:\s*$", line):
            inside = line.strip() == target + ":"
            in_roots = False
        elif inside and line.strip() == "source-root:":
            in_roots = True
        elif inside and in_roots:
            m = re.match(r"^\s+- (\S+)", line)
            if m:
                roots.append(m.group(1))
            else:
                in_roots = False
    return roots


def read_listing(path):
    """{function symbol: [(local name, offset, renameable)]} from a /Fa listing."""
    result, pending = {}, []
    for line in Path(path).read_text(errors="replace").split("\n"):
        m = re.match(r"^_(\S+)\$ = (-?\d+)\s*$", line)
        if m:
            pending.append((m.group(1), int(m.group(2)), "$" not in m.group(1)))
            continue
        m = re.match(r"^(\S+) = (-?\d+)\s*$", line)
        if m:  # a nested scope's duplicate (_i$99) or a named temporary ($T123)
            pending.append((m.group(1), int(m.group(2)), False))
            continue
        m = re.match(r"^(\S+)\s+PROC NEAR", line)
        if m:
            result[m.group(1)] = pending
            pending = []
    return result


def function_name(text):
    for m in re.finditer(r"(\w+)\s*\(", text):
        if m.group(1) not in ("__declspec", "__attribute__", "naked"):
            return m.group(1)
    return None


class Annotations:
    def __init__(self, target, roots):
        files = [p for root in roots for p in walk_source_dir(ROOT / root)]
        codebase = DecompCodebase(TextFile.from_files(files), target)
        self.functions = defaultdict(list)  # name -> [ParserFunction] (FUNCTION markers in sources)
        self.names = defaultdict(set)  # a symbol's or an entity's name -> addresses
        addresses = set()
        for function in codebase.iter_line_functions():
            name = function_name(function.name) or self.name_from_source(function)
            if name:
                if function.type == MarkerType.FUNCTION:
                    self.functions[name].append(function)
                self.names[name].add(function.offset)
            addresses.add(function.offset)
        for function in codebase.iter_name_functions():
            self.names[function.name].add(function.offset)
            addresses.add(function.offset)
        for variable in codebase.iter_variables():
            self.names[variable.name].add(variable.offset)
        self.addresses = sorted(addresses)

    @staticmethod
    def name_from_source(function):
        """The name of a function whose definition starts with a preprocessor line (reccmp's parser
        then takes the directive for its name): the first identifier before a "(" in the lines
        that follow, directives skipped."""
        lines = Path(function.filename).read_text(encoding="utf-8").split("\n")[function.line_number - 1 : function.end_line]
        return function_name(" ".join(line for line in lines if not line.lstrip().startswith("#")))

    def address_of(self, symbol):
        """The annotated address of a COFF symbol's entity, or None."""
        plain = re.sub(r"^[_@]|@\d+$|\$S\d+$", "", symbol)  # a function's static: _x$S123
        for name in (symbol, plain):
            found = self.names.get(name)
            if found and len(found) == 1:
                return next(iter(found))
        return None


# ---------------------------------------------------------------------------------------------
# The comparison


class Difference(Exception):
    pass


class Comparison:
    """A rebuilt function against the original at an address."""

    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    disassembler.detail = True

    def __init__(self, function, obj, image, address, annotations):
        self.function, self.obj, self.image, self.address, self.annotations = function, obj, image, address, annotations
        self.slots = []  # (our displacement, the original's)
        self.unverified = []  # relocation targets no annotation names
        self.difference = None
        try:
            self.walk()
        except Difference as e:
            self.difference = str(e)
        self.exact = self.difference is None and all(a == b for a, b in self.slots)
        self.slots_only = self.difference is None and not self.exact

    def decode(self, data, offset):
        return next(self.disassembler.disasm(data[offset : offset + 16], offset, 1), None)

    def walk(self):
        code, relocations = self.function.code, self.function.relocations
        original = self.image.at(self.address, len(code) * 2 + 64)
        starts, branches = {}, []
        i = j = 0
        while i < len(code):
            starts[i] = j
            if i in relocations:
                # A switch's jump table (relocated dwords), then possibly its index table (bytes),
                # inside the function: the code resumes at the first branch target past them.
                while i in relocations:
                    self.target(relocations[i], code[i : i + 4], original[j : j + 4], j + 4, branches, i, True)
                    i, j = i + 4, j + 4
                    starts[i] = j
                later = [ours for ours, _, _, code_target in branches if ours >= i and code_target]
                resume = min(later) if later else len(code)
                while i < resume and i not in relocations:
                    if code[i] != original[j]:
                        raise Difference("+%#x: table byte %#04x, the original's %#04x" % (i, code[i], original[j]))
                    i, j = i + 1, j + 1
                continue
            a, b = self.decode(code, i), self.decode(original, j)
            if a is None or b is None:
                raise Difference("+%#x: undecodable" % i)
            if not self.same(a, b, branches):
                raise Difference("+%#x: %s %s    the original's: %s %s" % (i, a.mnemonic, a.op_str, b.mnemonic, b.op_str))
            i, j = i + a.size, j + b.size
        starts[i] = j
        for ours, theirs, where, _ in branches:
            if starts.get(ours) != theirs:
                raise Difference("+%#x: branches to +%#x, the original's elsewhere (+%#x)" % (where, ours, theirs))
        # The original's function must end here too: what follows is padding or the next function.
        end = self.address + j
        following = self.image.at(end, 16)
        later = [a for a in self.annotations.addresses if a > self.address]
        next_function = min(later) if later else None
        if end != next_function and following[:1] not in (b"\xcc", b"\x90", b"") and following[:3] != b"\x55\x8b\xec" and following[:2] != b"\xff\x25":
            raise Difference("+%#x: the original goes on (%s)" % (j, following[:4].hex()))

    def same(self, a, b, branches):
        if a.mnemonic != b.mnemonic or len(a.operands) != len(b.operands) or a.size != b.size and not self.slot_sized(a, b):
            return False
        relocated = {r - a.address: self.function.relocations[r] for r in range(a.address, a.address + a.size) if r in self.function.relocations}
        ea, eb = a.encoding, b.encoding
        fields_a = {"disp": (ea.disp_offset, ea.disp_size), "imm": (ea.imm_offset, ea.imm_size)}
        fields_b = {"disp": (eb.disp_offset, eb.disp_size), "imm": (eb.imm_offset, eb.imm_size)}
        kinds = {}
        for offset, relocation in relocated.items():
            kind = next((k for k, (o, size) in fields_a.items() if size == 4 and o == offset), None)
            if kind is None or fields_b[kind][1] != 4:
                return False
            kinds[kind] = relocation
        moved = False
        branch = a.group(CS_GRP_JUMP) or a.group(CS_GRP_CALL)
        masked_imm = False
        for x, y in zip(a.operands, b.operands):
            if x.type != y.type or x.size != y.size:
                return False
            if x.type == X86_OP_REG:
                if x.reg != y.reg:
                    return False
            elif x.type == X86_OP_IMM:
                if "imm" in kinds:
                    continue
                if branch:
                    branches.append((x.imm, y.imm, a.address, True))
                    masked_imm = True
                elif x.imm != y.imm:
                    return False
            elif x.type == X86_OP_MEM:
                if (x.mem.segment, x.mem.base, x.mem.index, x.mem.scale) != (y.mem.segment, y.mem.base, y.mem.index, y.mem.scale):
                    return False
                if "disp" in kinds:
                    continue
                if x.mem.base == X86_REG_EBP and x.mem.disp < 0 and y.mem.disp < 0:
                    self.slots.append((x.mem.disp, y.mem.disp))
                    moved = moved or x.mem.disp != y.mem.disp
                elif x.mem.disp != y.mem.disp:
                    return False
        # The encodings: equal outside the relocated fields and a branch's displacement, or, where
        # a slot moved, outside the displacement and the ModRM byte's mod bits (disp8 or disp32).
        ba, bb = bytearray(a.bytes), bytearray(b.bytes)
        for kind in kinds:
            ba[fields_a[kind][0] : fields_a[kind][0] + 4] = bb[fields_b[kind][0] : fields_b[kind][0] + 4] = b"\0\0\0\0"
        if masked_imm:
            if ea.imm_size != eb.imm_size and not (ea.imm_size == 0 or eb.imm_size == 0):
                return False
            for buf, (o, size) in ((ba, fields_a["imm"]), (bb, fields_b["imm"])):
                if size:
                    buf[o : o + size] = b"\0" * size
        if moved:
            ma, mb = ea.modrm_offset, eb.modrm_offset
            if ma != mb or ba[:ma] != bb[:mb] or (ba[ma] ^ bb[mb]) & 0x3F:
                return False
            if bytes(ba[ea.disp_offset + ea.disp_size :]) != bytes(bb[eb.disp_offset + eb.disp_size :]):
                return False
        elif ba != bb:
            return False
        for kind, relocation in kinds.items():
            (oa, _), (ob, _) = fields_a[kind], fields_b[kind]
            self.target(relocation, a.bytes[oa : oa + 4], b.bytes[ob : ob + 4], b.address + b.size, branches, a.address, branch)
        return True

    @staticmethod
    def slot_sized(a, b):
        """Two instructions whose sizes may differ by a moved slot's displacement (disp8/disp32)."""
        return a.encoding.disp_size != b.encoding.disp_size and a.size - a.encoding.disp_size == b.size - b.encoding.disp_size

    def target(self, relocation, ours, theirs, end, branches, where=0, code_target=False):
        """Check a relocated field: the original's must hold the address of the symbol ours names."""
        symbol, kind = relocation
        addend = struct.unpack("<i", bytes(ours))[0]
        value = struct.unpack("<I", bytes(theirs))[0]
        if kind == IMAGE_REL_I386_REL32:
            value = (self.address + end + value) & 0xFFFFFFFF
        elif kind != IMAGE_REL_I386_DIR32:
            raise Difference("+%#x: relocation type %d" % (where, kind))
        function = self.function
        if symbol.section == function.symbol.section and symbol.section > 0:
            offset = symbol.value + addend - function.start
            if 0 <= offset <= len(function.code) and (symbol.storage != IMAGE_SYM_CLASS_EXTERNAL or symbol is function.symbol):
                branches.append((offset, value - self.address, where, code_target))
                return
        expected = self.annotations.address_of(symbol.name)
        if expected is not None:
            if (expected + addend) & 0xFFFFFFFF != value:
                raise Difference("+%#x: refers to %s+%d (%#x), the original's to %#x" % (where, symbol.name, addend, expected + addend, value))
            return
        if symbol.section > 0:
            data = self.obj.data_at(symbol, addend)
            if data is not None and len(data):
                if self.image.at(value, len(data)) != data:
                    raise Difference("+%#x: %s holds other data than the original's %#x" % (where, symbol.name, value))
                return
        self.unverified.append(symbol.name)


# ---------------------------------------------------------------------------------------------
# Renaming


def slot_plan(local_list, slots):
    """The named locals in the order the original allocates them, or a reason there is none.
    local_list: (name, offset, renameable) from the listing; slots: (ours, the original's)."""
    named = sorted((offset, name, renameable) for name, offset, renameable in local_list if offset < 0)
    if not named:
        return None, "no named locals"
    starts = [offset for offset, _, _ in named]
    bases = {}
    for ours, theirs in slots:
        if ours < starts[0]:
            if ours != theirs:
                return None, "an unnamed temporary moved (ebp%+d, the original's ebp%+d)" % (ours, theirs)
            continue
        index = max(k for k, start in enumerate(starts) if start <= ours)
        base = theirs - (ours - starts[index])
        if bases.setdefault(index, base) != base:
            return None, "%s is at two places in the original" % named[index][1]
    sizes = [(starts[k + 1] if k + 1 < len(starts) else 0) - starts[k] for k in range(len(starts))]
    taken = sorted((base, base + sizes[k], k) for k, base in bases.items())
    for (_, end, k), (start, _, n) in zip(taken, taken[1:]):
        if end > start:
            return None, "%s and %s overlap in the original" % (named[k][1], named[n][1])
    if taken and (taken[0][0] < starts[0] or taken[-1][1] > 0):
        return None, "%s is outside the named locals' area in the original" % named[taken[0][2]][1]
    # Locals the code never addresses go into the gaps, the largest first.
    free, at = [], starts[0]
    for start, end, _ in taken:
        if start > at:
            free.append([at, start])
        at = end
    if at < 0:
        free.append([at, 0])
    for k in sorted((k for k in range(len(named)) if k not in bases), key=lambda k: -sizes[k]):
        gap = next((g for g in free if g[1] - g[0] >= sizes[k]), None)
        if gap is None:
            return None, "no room for %s in the original's frame" % named[k][1]
        bases[k] = gap[1] - sizes[k]
        gap[1] -= sizes[k]
    order = sorted(bases, key=lambda k: -bases[k])
    if any(not named[k][2] for k in order):
        return None, "a nested scope's duplicate name (%s)" % next(named[k][1] for k in order if not named[k][2])
    return [named[k][1] for k in order], None


TOKEN = re.compile(r"""//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|'(?:\\.|[^'\\\n])*'|[A-Za-z_$][\w$]*|\d[\w.]*|->|\S""", re.S)


def rename(text, first, last, names):
    """The text with the identifiers in `names` renamed between two lines (1-based, inclusive),
    member names and preprocessor lines left alone."""
    lines = text.split("\n")
    body = "\n".join(lines[first - 1 : last])
    out, at, previous = [], 0, ""
    for m in TOKEN.finditer(body):
        token = m.group(0)
        line_start = body.rfind("\n", 0, m.start()) + 1
        directive = body[line_start : m.start()].lstrip().startswith("#")
        if token in names and previous not in (".", "->") and not directive:
            out.append(body[at : m.start()] + names[token])
            at = m.end()
        if not token.startswith(("//", "/*")):
            previous = token
    out.append(body[at:])
    return "\n".join(lines[: first - 1] + "".join(out).split("\n") + lines[last:])


class ProbeUnit(Unit):
    """A probe compiled with a unit's compiler and options, from a scratch file of its own."""

    def __init__(self, unit, tag):
        super().__init__(unit.target, unit.compiler, unit.language, unit.arguments, unit.source, unit.root)
        self.source = str(Path(unit.source).parent / ("zqprobe%s%s" % (tag, Path(unit.source).suffix))).replace("\\", "/")
        self.name = Path(self.source).name

    def files(self, tree):
        return {}


def ladder(compiler, target, unit):
    """Names whose slots a function gets in this order, whatever the order of declaration: one
    name from each bucket of the compiler's hash, found by compiling a probe twice."""
    alphabet = "abcdefghijklmnopqrstuvwxyz0123456789"
    pool = ["zq" + a + b for a in alphabet[:12] for b in alphabet]

    def probe(names):
        return "void ZqProbe(void)\n{\n%s%s}\n" % ("".join("\tint %s;\n" % n for n in names), "".join("\t%s = 0;\n" % n for n in names))

    forward, backward = ProbeUnit(unit, "a"), ProbeUnit(unit, "b")
    requests = [Request(forward, ROOT, "probe", {forward.name: probe(pool)}, listing=True),
                Request(backward, ROOT, "probe", {backward.name: probe(pool[::-1])}, listing=True)]
    compiler.run(target, requests)
    orders = []
    for request in requests:
        if request.error:
            raise SystemExit("the probe doesn't compile: " + request.error)
        local_list = next(iter(read_listing(request.asm).values()))
        orders.append([name for _, name in sorted(((offset, name) for name, offset, _ in local_list), reverse=True)])
    rank = {name: k for k, name in enumerate(orders[1])}
    names = [orders[0][0]]
    for name in orders[0][1:]:
        if rank[names[-1]] < rank[name]:  # the same order both ways: another bucket
            names.append(name)
    return names


# ---------------------------------------------------------------------------------------------
# The search


class Entry:
    def __init__(self, unit, name, annotation):
        self.unit, self.name, self.annotation = unit, name, annotation
        self.address = annotation.offset
        self.status = None
        self.detail = ""
        self.renames = {}
        self.pads = 0
        self.unverified = []

    def note(self, status, detail, pads):
        """Keep the closest verdict: slots-only (at the lowest padding) over another difference,
        and the unpadded compile's difference over a padded one's."""
        rank = {"slots": 0, "other": 1}
        if self.status not in rank or rank[status] < rank[self.status] or rank[status] == rank[self.status] and not self.detail:
            self.status, self.detail = status, detail + (" (with %d pads)" % pads if pads else "")

    def recipe(self):
        parts = []
        if self.pads:
            parts.append("pad %d" % self.pads)
        if self.renames:
            parts.append("rename " + " ".join("%s=%s" % item for item in sorted(self.renames.items())))
        return " ".join(parts)


class Search:
    def __init__(self, args, target):
        self.args, self.target_name = args, target
        original = args.original or read_yaml_paths().get(target)
        if not original or not Path(original).exists():
            raise SystemExit("no original for %s: pass --original" % target)
        self.image = Image(original)
        self.annotations = Annotations(target, source_roots(target))
        self.target = Target(target, ROOT / args.build)
        self.units = [u for u in self.target.units if u.path(ROOT).exists() and u.language != "ASM_MASM"]
        scratch = ROOT / args.build / "witness" / target
        self.compiler = Compiler(scratch, args.wine, args.jobs)
        self.ladders = {}
        self.entries = []
        self.skipped = Counter()

    def entries_of(self, unit, obj):
        """The unit's annotated functions: {COFF symbol: annotation in the unit's own files}."""
        files = {str(path.resolve()) for path in unit.files(ROOT).values()}
        found = {}
        for symbol in obj.functions:
            plain = re.sub(r"^[_@]|@\d+$", "", symbol)
            own = [c for c in self.annotations.functions.get(plain, []) if str(c.filename) in files]
            if len(own) == 1:
                found[symbol] = own[0]
        return found

    def compare(self, request, wanted=None):
        """{symbol: (Comparison, the function's locals, annotation)} of one compile, or an error."""
        if request.error:
            return request.error
        obj, local_lists = Object(request.obj), read_listing(request.asm)
        out = {}
        for symbol, annotation in self.entries_of(request.unit, obj).items():
            if wanted is None or symbol in wanted:
                comparison = Comparison(obj.functions[symbol], obj, self.image, annotation.offset, self.annotations)
                out[symbol] = (comparison, local_lists.get(symbol, []), annotation)
        return out

    def plan(self, unit, compared, pending):
        """The renamed files of the unit for the functions that differ only in slots."""
        if unit.language not in self.ladders:
            self.ladders[unit.language] = ladder(self.compiler, self.target, unit)
            if self.args.verbose:
                print("ladder (%s, %d names): %s" % (unit.language, len(self.ladders[unit.language]), " ".join(self.ladders[unit.language])))
        names = self.ladders[unit.language]
        texts, plans = {}, {}
        pads = self.pads
        for symbol, entry in pending.items():
            comparison, local_list, annotation = compared[symbol]
            if comparison.difference:
                entry.note("other", comparison.difference, pads)
                continue
            order, reason = slot_plan(local_list, comparison.slots)
            if order is None:
                entry.note("slots", reason, pads)
                continue
            if len(order) > len(names):
                entry.note("slots", "%d locals, and the compiler's hash has %d buckets" % (len(order), len(names)), pads)
                continue
            path = Path(annotation.filename)
            text = texts.get(path.name) or path.read_text(encoding="utf-8")
            plans[symbol] = dict(zip(order, names))
            texts[path.name] = rename(text, annotation.line_number, annotation.end_line, plans[symbol])
        return texts, plans

    def run(self):
        units = [u for u in self.units if not self.args.unit or u.name in self.args.unit]
        for unit in units:
            if unit.language != "C":
                self.skipped["C++ unit"] += 1
        units = [u for u in units if u.language == "C"]
        base = {r.unit.name: r for r in self.compiler.run(self.target, [Request(u, ROOT, "base", listing=True) for u in units])}
        pending = {}  # unit name -> {symbol: Entry}
        exact = defaultdict(set)
        for unit in units:
            compared = self.compare(base[unit.name])
            if isinstance(compared, str):
                print("%s: %s" % (unit.name, compared))
                continue
            for symbol, (comparison, _, annotation) in compared.items():
                name = function_name(annotation.name) or Annotations.name_from_source(annotation)
                if self.args.function and name not in self.args.function:
                    continue
                entry = Entry(unit, name, annotation)
                entry.unverified = comparison.unverified
                self.entries.append(entry)
                if comparison.exact:
                    entry.status = "exact"
                    exact[unit.name].add(symbol)
                else:
                    entry.status = None
                    entry.note("other" if comparison.difference else "slots", comparison.difference or "", 0)
                    pending.setdefault(unit.name, {})[symbol] = entry
        by_name = {u.name: u for u in units}
        for pads in range(0, self.args.pads + 1):
            self.pads = pads
            todo = [by_name[n] for n, entries in pending.items() if entries]
            if not todo:
                break
            if pads:
                padded = {r.unit.name: r for r in self.compiler.run(self.target, [Request(u, ROOT, "pad", prefix=pads_text(pads), listing=True) for u in todo])}
            requests, plans = [], {}
            for unit in todo:
                compared = self.compare(padded[unit.name] if pads else base[unit.name], pending[unit.name])
                if isinstance(compared, str):
                    continue
                for symbol, entry in list(pending[unit.name].items()):
                    if pads and symbol in compared and compared[symbol][0].exact:
                        entry.status, entry.pads, entry.unverified, entry.detail = "witness", pads, compared[symbol][0].unverified, ""
                        del pending[unit.name][symbol]
                texts, plans[unit.name] = self.plan(unit, compared, {s: e for s, e in pending[unit.name].items() if s in compared})
                if texts:
                    requests.append(Request(unit, ROOT, "rename", texts, prefix=pads_text(pads), listing=True))
            self.compiler.run(self.target, requests)
            for request in requests:
                unit = request.unit
                compared = self.compare(request)
                if isinstance(compared, str):
                    for symbol in plans[unit.name]:
                        pending[unit.name][symbol].note("slots", "renamed: " + compared, pads)
                    continue
                for symbol in exact[unit.name]:
                    if not pads and symbol in compared and not compared[symbol][0].exact:
                        print("warning: %s: renaming other functions' locals changed %s" % (unit.name, symbol))
                for symbol, names in plans[unit.name].items():
                    entry = pending[unit.name][symbol]
                    comparison = compared[symbol][0] if symbol in compared else None
                    if comparison is not None and comparison.exact:
                        entry.status, entry.renames, entry.pads, entry.unverified = "witness", names, pads, comparison.unverified
                        entry.detail = ""
                        del pending[unit.name][symbol]
                    else:
                        entry.note("slots", "renamed, still differs" + (": " + comparison.difference if comparison is not None and comparison.difference else ""), pads)
            self.compiler.discard(requests)
            if pads:
                self.compiler.discard(padded.values())
        self.compiler.discard(base.values())

    def check(self, records):
        """Verify the recorded witnesses. Returns the list of failures."""
        failures = []
        by_name = {u.name: u for u in self.units}
        groups = defaultdict(list)
        for record in records:
            groups[(record["unit"], record["pads"])].append(record)
        base = {r.unit.name: r for r in self.compiler.run(self.target, [Request(by_name[u], ROOT, "base", listing=True) for u in {u for u, _ in groups} if u in by_name])}
        requests, keys = [], []
        for (unit_name, pads), group in groups.items():
            unit = by_name.get(unit_name)
            if unit is None:
                failures += ["%s: no such unit (%s)" % (r["name"], unit_name) for r in group]
                continue
            texts = {}
            for record in group:
                candidates = [a for a in self.annotations.functions.get(record["name"], []) if a.offset == record["address"]]
                if not candidates:
                    failures.append("%s: no annotation at %#x" % (record["name"], record["address"]))
                    continue
                annotation = candidates[0]
                record["annotation"] = annotation
                if record["renames"]:
                    path = Path(annotation.filename)
                    text = texts.get(path.name) or path.read_text(encoding="utf-8")
                    texts[path.name] = rename(text, annotation.line_number, annotation.end_line, record["renames"])
            requests.append(Request(unit, ROOT, "pad%d" % pads, texts, prefix=pads_text(pads), listing=True))
            keys.append((unit_name, pads))
        self.compiler.run(self.target, requests)
        for (unit_name, pads), request in zip(keys, requests):
            plain = self.compare(base[unit_name])
            compared = self.compare(request)
            for record in groups[(unit_name, pads)]:
                if "annotation" not in record:
                    continue
                if isinstance(compared, str) or isinstance(plain, str):
                    failures.append("%s: %s" % (record["name"], compared if isinstance(compared, str) else plain))
                    continue
                at = {a.offset: s for s, (_, _, a) in compared.items()}
                symbol = at.get(record["address"])
                if symbol is None or not compared[symbol][0].exact:
                    failures.append("%s (%s): the witness no longer gives the original's bytes" % (record["name"], unit_name))
                elif symbol in plain and plain[symbol][0].exact:
                    failures.append("%s (%s): exact without its witness: remove the record" % (record["name"], unit_name))
        return failures


def read_records(path):
    records = []
    for line in path.read_text().split("\n"):
        if not line.strip() or line.startswith("#"):
            continue
        address, name, unit, *recipe = line.split()
        record = {"address": int(address, 16), "name": name, "unit": unit, "pads": 0, "renames": {}}
        mode = None
        for word in recipe:
            if word in ("pad", "rename"):
                mode = word
            elif mode == "pad":
                record["pads"] = int(word)
            else:
                old, new = word.split("=")
                record["renames"][old] = new
        records.append(record)
    return records


def claims(entry):
    """What the comments above the function's annotations say its diff is: 'slots' for a stack-slot
    permutation, 'other' for another explanation, '' for none."""
    lines = Path(entry.annotation.filename).read_text(encoding="utf-8").split("\n")
    at = entry.annotation.line_number - 2
    text = []
    while at >= 0 and lines[at].lstrip().startswith("//"):
        text.append(lines[at])
        at -= 1
    comment = " ".join(line for line in text if not re.match(r"\s*// (FUNCTION|STUB|LIBRARY|SYNTHETIC|GLOBAL):", line))
    if re.search(r"slot", comment, re.I):
        return "slots"
    if re.search(r"operand order|register|matches but|only diff|differs|scheduling|entropy", comment, re.I):
        return "other"
    return ""


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", required=True, choices=sorted(TARGETS))
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build", help="The build tree, relative to the repository")
    parser.add_argument("--original", help="The original binary (default: reccmp-user.yml's)")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"), help="Wine, off Windows")
    parser.add_argument("--unit", nargs="*", help="Only these units (ai.c)")
    parser.add_argument("--function", nargs="*", help="Only these functions")
    parser.add_argument("--pads", type=int, default=0, help="Also try up to N symbols declared ahead of the unit")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--write", action="store_true", help="Record the witnesses in tools/witnesses/<TARGET>.txt")
    parser.add_argument("--check", action="store_true", help="Verify tools/witnesses/<TARGET>.txt")
    parser.add_argument("--json", help="Write every function's result to this file")
    parser.add_argument("-v", "--verbose", action="store_true", help="List every function that isn't exact")
    args = parser.parse_args()

    search = Search(args, args.target)
    record_file = ROOT / "tools" / "witnesses" / (args.target + ".txt")
    if args.check:
        records = read_records(record_file) if record_file.exists() else []
        failures = search.check(records)
        for failure in failures:
            print(failure)
        print("%d witnesses, %d failures (%d compiles)" % (len(records), len(failures), search.compiler.count))
        return 1 if failures else 0

    search.run()
    entries = sorted(search.entries, key=lambda e: e.address)
    counts = Counter(e.status for e in entries)
    if args.verbose:
        for entry in entries:
            if entry.status != "exact":
                print("%#010x %-8s %-32s %s" % (entry.address, entry.status, entry.name, entry.recipe() or entry.detail))
    print("%s: %d functions in %d units (%d compiles)" % (args.target, len(entries), len({e.unit.name for e in entries}), search.compiler.count))
    for status in ("exact", "witness", "slots", "other"):
        print("  %-8s %d" % (status, counts[status]))
    for reason, count in search.skipped.items():
        print("  skipped: %d %s" % (count, reason))
    said = Counter((claims(e), e.status) for e in entries if e.status != "exact")
    for claim in ("slots", "other", ""):
        row = {s: said[(claim, s)] for s in ("witness", "slots", "other")}
        if sum(row.values()):
            print("  comment says %-6s %s" % ((claim or "nothing") + ":", ", ".join("%s %d" % item for item in row.items())))
    unverified = Counter(name for e in entries if e.status in ("exact", "witness") for name in e.unverified)
    if unverified:
        print("  relocation targets no annotation names (unverified): %d symbols, %d uses" % (len(unverified), sum(unverified.values())))
        if args.verbose:
            print("    " + ", ".join("%s (%d)" % item for item in unverified.most_common(20)))
    if args.json:
        Path(args.json).write_text(json.dumps([
            {"address": "%#010x" % e.address, "name": e.name, "unit": e.unit.name, "status": e.status, "detail": e.detail,
             "pads": e.pads, "renames": e.renames, "unverified": e.unverified, "claim": claims(e) if e.status != "exact" else ""}
            for e in entries], indent=1) + "\n")
    if args.write:
        record_file.parent.mkdir(exist_ok=True)
        lines = ["# Witnesses that a function differs from the original only by compiler entropy (tools/witness.py).",
                 "# address, function, unit, then the recipe: `pad N` symbols declared ahead of the unit, and/or",
                 "# `rename old=new ...` of the function's locals. Checked by `tools/witness.py --check`.", ""]
        lines += ["%#010x %s %s %s" % (e.address, e.name, e.unit.name, e.recipe()) for e in entries if e.status == "witness"]
        record_file.write_text("\n".join(lines) + "\n")
        print("wrote %s" % record_file.relative_to(ROOT))
    return 0


if __name__ == "__main__":
    sys.exit(main())
