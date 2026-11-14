"""Compare a function of a compiled object with the original's bytes at its annotated address.

Shared by `certify.py` (which checks the witness records) and `witness.py` (which searches for
them). The comparison is of the whole original function: reccmp reads only as many bytes as the
rebuilt function has, so a function shorter than the original's can score 100% there.

A function compares equal when every instruction is the original's, a relocated field holds the
address of the entity the field names (by its annotation, or for a string or constant of the
unit, the same bytes at the original's address), and the original's function ends where the
rebuilt one does. The differences that a witness may remove are reported apart: `[ebp-N]`
operands (stack slots) whose bytes differ only in the displacement.
"""

import re
import struct
from collections import defaultdict
from pathlib import Path

from capstone import CS_ARCH_X86, CS_GRP_CALL, CS_GRP_JUMP, CS_MODE_32, Cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG, X86_REG_EBP
from reccmp.dir import walk_source_dir
from reccmp.formats import TextFile
from reccmp.parser import DecompCodebase
from reccmp.parser.marker import MarkerType

from unitbuild import IMAGE_REL_I386_DIR32, IMAGE_REL_I386_REL32, IMAGE_SYM_CLASS_EXTERNAL, ROOT


def read_yaml_paths(root=ROOT):
    """The originals' paths from reccmp-user.yml (a flat file: no YAML library needed)."""
    paths, target = {}, None
    user = Path(root) / "reccmp-user.yml"
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


def source_roots(target, root=ROOT):
    """The target's source roots, from reccmp-project.yml."""
    roots, inside, in_roots = [], False, False
    for line in (Path(root) / "reccmp-project.yml").read_text().split("\n"):
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


QUALIFIED = re.compile(r"((?:[A-Za-z_]\w*::)*(?:~?[A-Za-z_]\w*|operator\s*[^\s(]+))\s*\(")


def function_name(text):
    """The (qualified, for C++) name of the function a definition's text declares."""
    for m in QUALIFIED.finditer(text):
        if m.group(1) not in ("__declspec", "__attribute__", "naked"):
            return re.sub(r"operator\s+", "operator", m.group(1))
    return None


OPERATORS = {"0": None, "1": None, "2": "operator new", "3": "operator delete", "4": "operator=", "8": "operator==",
             "9": "operator!=", "A": "operator[]", "_G": "`scalar deleting destructor'", "_E": "`vector deleting destructor'"}


def undecorate(symbol):
    """The qualified name of a Visual C++ decorated symbol (`?Run@ArchiveReader@@QAEHXZ` is
    `ArchiveReader::Run`), or None. Names only: no types, no back-references."""
    if not symbol.startswith("?"):
        return None
    text, special = symbol[1:], None
    if text.startswith("?"):
        special = text[1:3] if text[1] == "_" else text[1]
        text = text[1 + len(special) :]
        if special not in OPERATORS:
            return None
    end = text.find("@@")
    if end < 0:
        return None
    parts = text[:end].split("@")
    if any(not re.match(r"^[A-Za-z_]\w*$", p) for p in parts):
        return None
    if special is None:
        name, scopes = parts[0], parts[1:]
    elif special == "0":
        name, scopes = parts[0], parts
    elif special == "1":
        name, scopes = "~" + parts[0], parts
    else:
        name, scopes = OPERATORS[special], parts
    return "::".join(list(reversed(scopes)) + [name])


def symbol_name(symbol):
    """The source name of a COFF symbol: undecorated C++, or C without its `_`, `@` and `@N`."""
    return undecorate(symbol) or re.sub(r"^[_@]|@\d+$", "", symbol)


class Annotations:
    """The target's annotations, from the source tree (reccmp's parser)."""

    def __init__(self, target, roots, root=ROOT):
        files = [p for r in roots for p in walk_source_dir(Path(root) / r)]
        codebase = DecompCodebase(TextFile.from_files(files), target)
        self.functions = defaultdict(list)  # name -> [ParserFunction] (FUNCTION markers in sources)
        self.names = defaultdict(set)  # a symbol's or an entity's name -> addresses
        addresses = set()
        for function in codebase.iter_line_functions():
            name = self.name_of(function)
            if name:
                if function.type == MarkerType.FUNCTION:
                    self.functions[name].append(function)
                self.names[name].add(function.offset)
            addresses.add(function.offset)
        self.by_name = defaultdict(list)  # name -> [ParserFunction] (FUNCTION markers by name, in headers)
        for function in codebase.iter_name_functions():
            if function.type == MarkerType.FUNCTION:
                self.by_name[function.name].append(function)
            self.names[function.name].add(function.offset)
            addresses.add(function.offset)
        for variable in codebase.iter_variables():
            self.names[variable.name].add(variable.offset)
        self.addresses = sorted(addresses)

    @staticmethod
    def name_of(function):
        """A line function's name. reccmp's parser takes a preprocessor line for the name of a
        definition that starts with one: then the first name before a "(" in the lines that
        follow, directives skipped."""
        name = function_name(function.name)
        if name:
            return name
        lines = Path(function.filename).read_text(encoding="utf-8").split("\n")[function.line_number - 1 : function.end_line]
        return function_name(" ".join(line for line in lines if not line.lstrip().startswith("#")))

    def address_of(self, symbol):
        """The annotated address of a COFF symbol's entity, or None."""
        plain = re.sub(r"\$S\d+$", "", symbol_name(symbol))  # a function's static: _x$S123
        for name in (symbol, plain):
            found = self.names.get(name)
            if found and len(found) == 1:
                return next(iter(found))
        return None


class Difference(Exception):
    pass


class Comparison:
    """A rebuilt function against the original at an address.

    `exact`: the bytes are the original's. `slots_only`: they are but for `[ebp-N]` operands;
    `slots` lists them as (ours, the original's). `difference`: the first other difference.
    `unverified`: relocation targets no annotation names (and that hold no data to compare)."""

    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    disassembler.detail = True

    def __init__(self, function, obj, image, address, annotations):
        self.function, self.obj, self.image, self.address, self.annotations = function, obj, image, address, annotations
        self.slots = []
        self.unverified = []
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
        if end == next_function or following[:1] in (b"\xcc", b"\x90", b"") or following[:3] == b"\x55\x8b\xec" or following[:2] == b"\xff\x25":
            return
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
            expected = (expected + addend) & 0xFFFFFFFF
            if expected != value and self.thunk(value) != expected:
                raise Difference("+%#x: refers to %s+%d (%#x), the original's to %#x" % (where, symbol.name, addend, expected, value))
            return
        if symbol.section > 0:
            data = self.obj.data_at(symbol, addend)
            if data is not None and len(data):
                if self.image.at(value, len(data)) != data:
                    raise Difference("+%#x: %s holds other data than the original's %#x" % (where, symbol.name, value))
                return
        self.unverified.append(symbol.name)

    def thunk(self, address):
        """Where a `jmp rel32` at `address` goes (an incremental link's thunk), or None."""
        data = self.image.at(address, 5)
        if data[:1] != b"\xe9":
            return None
        return (address + 5 + struct.unpack("<i", data[1:5])[0]) & 0xFFFFFFFF
