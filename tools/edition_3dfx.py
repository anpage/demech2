"""The 3Dfx edition's side of tools/edition.py (--target MW23DFX).

The 3Dfx edition is the Matrox edition's C built by VC++ 2.2 with optimization, so its functions
can't be paired with another binary's byte by byte, as the Matrox edition's are with 1.1's.
They are paired with the rebuild instead, by what optimization leaves alone: the functions,
imports and annotated globals a function calls or addresses, and the strings it uses. A source
function without a MW23DFX annotation is paired with the original's unannotated function whose
references are most like its rebuilt version's, when the choice is clear both ways; functions
with too few references to tell are placed by their neighbours (the unannotated functions between
two paired ones, as many in the rebuild's file order as in the original's address order).

The comparison of a rebuilt function with its original reports two ratios: the instructions as
they are, and with registers renamed away, which tells register allocation (2.2's entropy under
/Ox) from differences in the code. The pairing is cached in DIR/edition-MW23DFX.json.
"""

import bisect
import difflib
import glob
import json
import os
import re
import struct
import sys
from collections import Counter

from port_annotations import (  # noqa: E402
    MARKER,
    Function,
    Image,
    longest_increasing,
    map_lookup,
    read_map,
)

TARGET = "MW23DFX"
BINARY = "MW2_3DFX.DLL"
MODULES = ["MW2SHELL", "MW2", "MW2MATROX", "MW23DFX", "NETMECHW", "MECH2"]  # reccmp-project.yml's order
DIRS = ["MW2", "common"]
CODE_END = 0x10068270  # Texus, then the import thunks and the C runtime
REGISTER = re.compile(r"\b(e?[abcd]x|[abcd][lh]|e?[sd]i|e?[sb]p)\b")
ADDRESS = re.compile(r"0x[0-9a-f]{5,}")
WEIGHTS = {"s": 4, "i": 2, "f": 1, "g": 1}  # strings, imports, functions, globals
MIN_WEIGHT = 3  # below this, a function has too few references to pair by them
MIN_SCORE = 0.5


def disasm():
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs

    d = Cs(CS_ARCH_X86, CS_MODE_32)
    d.skipdata = True
    d.detail = True
    return d


# --- The sources ---


def definitions():
    """Every function definition under MW2 that a marker block annotates: dicts with the name,
    path, the block's line range, and its markers ({module: address})."""
    result = []
    for path in sorted(glob.glob("MW2/src/**/*.c", recursive=True)):
        lines = open(path, encoding="utf-8").read().split("\n")
        i = 0
        while i < len(lines):
            m = MARKER.match(lines[i])
            if not m:
                i += 1
                continue
            j, markers, kinds = i, {}, set()
            while j < len(lines) and MARKER.match(lines[j]):
                k = MARKER.match(lines[j])
                markers[k.group(3)] = int(k.group(4), 16)
                kinds.add(k.group(2))
                j += 1
            k = j
            while k < len(lines) and lines[k].startswith(("#if", "//")):  # a signature per edition
                k += 1
            if k < len(lines) and kinds & {"FUNCTION", "STUB"}:
                n = re.match(r"^[A-Za-z].*?\b(\w+)\(", lines[k])
                if n and not lines[k].rstrip().endswith(";"):
                    result.append({"name": n.group(1), "path": path, "first": i, "last": j - 1, "markers": markers})
            i = j
    return result


def candidates(defs):
    """The definitions to pair: no MW23DFX annotation, and no other definition of the name has one
    (an edition's own version, behind #ifdef). Several definitions of one name are ambiguous."""
    annotated = {d["name"] for d in defs if TARGET in d["markers"]}
    by_name = {}
    for d in defs:
        if d["name"] not in annotated:
            by_name.setdefault(d["name"], []).append(d)
    return by_name


def original_names():
    """The original's addresses the sources name for MW23DFX: {address: name}."""
    from port_annotations import marker_blocks

    names = {}
    for path, last, kind, rest, name in marker_blocks(DIRS, [TARGET]):
        m = MARKER.match(open(path, encoding="utf-8").read().split("\n")[last])
        if m and m.group(3) == TARGET:
            names[int(m.group(4), 16)] = name
    csv = "reccmp/mw23dfx-vfx.csv"
    if os.path.exists(csv):
        for line in open(csv, encoding="utf-8"):
            parts = line.strip().split(",")
            if len(parts) >= 2 and parts[0].startswith("0x"):
                names.setdefault(int(parts[0], 16), parts[1])
    return names


# --- The binaries ---


def imports(image):
    """{IAT slot address: imported name} from the import directory."""
    pe = struct.unpack_from("<I", image.data, 0x3C)[0]
    rva, size = struct.unpack_from("<II", image.data, pe + 24 + 96 + 8)
    result = {}
    if not rva:
        return result
    at = image.base + rva
    while True:
        lookup, _, _, name_rva, iat = struct.unpack("<5I", image.read(at, 20))
        if not iat:
            break
        table = lookup or iat
        k = 0
        while True:
            entry = image.u32(image.base + table + 4 * k)
            if not entry:
                break
            if entry & 0x80000000:
                name = "#%d" % (entry & 0xFFFF)
            else:
                raw = image.read(image.base + entry + 2, 64)
                name = raw.split(b"\0")[0].decode("ascii", "replace")
            result[image.base + iat + 4 * k] = name
            k += 1
        at += 20
    return result


def string_at(image, address):
    raw = image.read(address, 160) if image.offset(address) is not None else b""
    if not raw or address < image.text[1] and address >= image.text[0]:
        return None
    s = raw.split(b"\0")[0]
    if len(s) < 3 or len(s) == len(raw) or any(c < 0x20 and c not in (9, 10, 13) or c > 0x7E for c in s):
        return None
    return s.decode("ascii")


class Side:
    """A binary with names for its addresses, and the references of its functions."""

    def __init__(self, image, names):
        self.image, self.names = image, names
        self.imports = imports(image)
        self.thunks = {}
        data = image.text_bytes
        for m in re.finditer(b"\xff\x25", data):
            slot = struct.unpack_from("<I", data, m.start() + 2)[0] if m.start() + 6 <= len(data) else None
            if slot in self.imports:
                self.thunks[image.text[0] + m.start()] = self.imports[slot]
        self.disasm = disasm()

    def refs(self, start, end, allowed=None):
        """The function's references as a Counter of "s:string", "i:import", "f:function",
        "g:global" (named globals only; with allowed, only names in it)."""
        found = Counter()
        image = self.image
        for x in self.disasm.disasm(image.read(start, end - start), start):
            if not x.id:
                continue
            if x.mnemonic in ("call", "jmp") and x.op_str.startswith("0x"):
                t = int(x.op_str, 16)
                if not start <= t < end:
                    if t in self.thunks:
                        found["i:" + self.thunks[t]] += 1
                    elif t in self.names and (allowed is None or self.names[t] in allowed):
                        found["f:" + self.names[t]] += 1
                continue
            for a in range(x.address, x.address + x.size):
                if a not in image.relocs:
                    continue
                v = image.u32(a)
                if v in self.imports:
                    found["i:" + self.imports[v]] += 1
                elif v in self.thunks:
                    found["i:" + self.thunks[v]] += 1
                elif v in self.names and (allowed is None or self.names[v] in allowed):
                    found[("f:" if image.text[0] <= v < image.text[1] else "g:") + self.names[v]] += 1
                else:
                    s = string_at(image, v)
                    if s:
                        found["s:" + s] += 1
        return found


def similarity(a, b):
    """The weighted overlap of two reference multisets, 0 to 1."""
    weight = lambda c: sum(WEIGHTS[k[0]] * n for k, n in c.items())  # noqa: E731
    union = weight(a | b)
    return weight(a & b) / union if union else 0.0


class Rebuild:
    def __init__(self, build):
        base = os.path.join(build, "vc22", "MW2_3DFX")
        if not os.path.exists(base + ".dll"):
            sys.exit("no rebuilt edition at %s.dll" % base)
        self.image = Image(base + ".dll")
        symbols = read_map(base + ".map")
        self.lookup = map_lookup(symbols)
        code = sorted({a for a, _, sec, _ in symbols if sec == "0001"})
        self.following = dict(zip(code, code[1:] + [self.image.text[1]]))
        names = {}
        for a, name, sec, obj in symbols:
            m = re.match(r"^[_@]?([A-Za-z_]\w*?)(?:@\d+)?$", name)
            names.setdefault(a, m.group(1) if m else name)
        self.side = Side(self.image, names)

    def range(self, name):
        start = self.lookup(name)
        if start is None or start not in self.following:
            return None
        f = Function(self.image, start, self.following[start], self.side.disasm)
        return f.start, f.end


def original_functions(image, names):
    """The original's function starts below CODE_END: after int3 padding at a 16-aligned address,
    call targets, and annotated addresses. {start: end}."""
    lo = image.text[0]
    data = image.text_bytes
    starts = {a for a in names if lo <= a < CODE_END}
    for off in range(16, CODE_END - lo, 16):
        if data[off - 1] == 0xCC and data[off] != 0xCC:
            starts.add(lo + off)
    for m in re.finditer(b"\xe8", data[: CODE_END - lo]):
        if m.start() + 5 <= len(data):
            t = lo + m.start() + 5 + struct.unpack_from("<i", data, m.start() + 1)[0]
            if lo <= t < CODE_END and t % 16 == 0:
                starts.add(t)
    starts = sorted(starts)
    result = {}
    for a, b in zip(starts, starts[1:] + [CODE_END]):
        end = b
        while end > a and image.read(end - 1, 1) == b"\xcc":
            end -= 1
        result[a] = end
    return result


# --- The pairing ---


def compute_state(build, cache):
    original = Image(BINARY)
    names = original_names()
    orig = Side(original, names)
    rebuild = Rebuild(build)
    allowed = set(names.values())
    defs = definitions()
    cands = candidates(defs)

    functions = original_functions(original, names)
    free = {a: e for a, e in functions.items() if a not in names}
    orig_refs = {a: orig.refs(a, e) for a, e in free.items()}

    # Each candidate's rebuilt references, in the names the original has.
    rebuilt = {}
    for name, ds in cands.items():
        if len(ds) != 1:
            continue
        r = rebuild.range(name)
        if r:
            rebuilt[name] = rebuild.side.refs(r[0], r[1], allowed)

    # By references: the best original for each rebuilt function, and the reverse.
    weight = lambda c: sum(WEIGHTS[k[0]] * n for k, n in c.items())  # noqa: E731
    index = {}
    for a, refs in orig_refs.items():
        for k in refs:
            index.setdefault(k, set()).add(a)
    best_for, scores = {}, {}
    for name, refs in rebuilt.items():
        if weight(refs) < MIN_WEIGHT:
            continue
        pool = set().union(*(index.get(k, set()) for k in refs)) if refs else set()
        ranked = sorted(((similarity(refs, orig_refs[a]), a) for a in pool), reverse=True)
        if ranked and ranked[0][0] >= MIN_SCORE and (len(ranked) == 1 or ranked[0][0] > ranked[1][0]):
            best_for[name] = ranked[0][1]
            scores[name] = ranked[0][0]
    claims = Counter(best_for.values())
    paired = {n: a for n, a in best_for.items() if claims[a] == 1}
    method = {n: "references %.2f" % scores[n] for n in paired}

    # By neighbours, in each file: between two functions with known addresses, the unpaired
    # rebuilt functions (file order) against the original's free functions (address order).
    known_in_file = {}
    for d in defs:
        if d["name"] in cands and len(cands[d["name"]]) == 1 and d["name"] in rebuilt:
            known_in_file.setdefault(d["path"], []).append((d["first"], d["name"], None))
        elif TARGET in d["markers"]:
            known_in_file.setdefault(d["path"], []).append((d["first"], d["name"], d["markers"][TARGET]))
    starts = sorted(functions)
    changed = True
    while changed:
        changed = False
        taken = set(paired.values())
        for unit, items in known_in_file.items():
            items.sort()
            seq = [(n, a if a is not None else paired.get(n)) for _, n, a in items]
            anchors = [k for k, (_, a) in enumerate(seq) if a is not None]
            for k0, k1 in zip(anchors, anchors[1:]):
                lo, hi = seq[k0][1], seq[k1][1]
                gap = [n for n, a in seq[k0 + 1 : k1]]
                if not gap or lo >= hi:
                    continue
                i0, i1 = bisect.bisect_right(starts, lo), bisect.bisect_left(starts, hi)
                between = starts[i0:i1]
                if len(between) == len(gap) and not any(a in names or a in taken for a in between):
                    for n, a in zip(gap, between):
                        paired[n], method[n] = a, "neighbours"
                    changed = True
    state = {n: {"address": paired[n], "end": functions[paired[n]], "method": method[n]} for n in paired}
    with open(cache, "w") as f:
        json.dump(state, f, indent=1, sort_keys=True)
    return state


def load_state(build, refresh=False):
    path = os.path.join(build, "edition-%s.json" % TARGET)
    if refresh or not os.path.exists(path):
        print("computing the pairing with the rebuild ...", file=sys.stderr)
        return compute_state(build, path)
    return json.load(open(path))


# --- Comparing ---


def instructions(side, start, end):
    return [x for x in side.disasm.disasm(side.image.read(start, end - start), start) if x.id]


def ishape(x, side, registers=True):
    """An instruction without its addresses (relocated operands and branch targets); with
    registers False, without its registers too."""
    text = x.mnemonic + " " + x.op_str
    if x.mnemonic.startswith("j") or x.mnemonic == "call" or any(x.address + k in side.image.relocs for k in range(x.size)):
        text = ADDRESS.sub("A", text)
    if not registers:
        text = REGISTER.sub("R", text)
    return text


def compare(rebuild, original, name, address, end):
    """(exact ratio, ratio without registers, opcodes, R, T), or None without a rebuilt function."""
    r = rebuild.range(name)
    if r is None:
        return None
    R = instructions(rebuild.side, r[0], r[1])
    T = instructions(original, address, end)
    exact = difflib.SequenceMatcher(None, [ishape(x, rebuild.side) for x in R], [ishape(x, original) for x in T], autojunk=False)
    loose = difflib.SequenceMatcher(
        None, [ishape(x, rebuild.side, False) for x in R], [ishape(x, original, False) for x in T], autojunk=False
    )
    ops = [o for o in exact.get_opcodes() if o[0] != "equal"]
    return exact.ratio(), loose.ratio(), ops, R, T


def text(x):
    return x.mnemonic + " " + x.op_str


REGISTERS_NOTE = "// Matches but for the registers, which VC++ 2.2 allocates differently."


BY_NAME_INTRO = "// The 3Dfx edition places these out of %s's order, so they are annotated here, by name."


def add_by_name(path, name, address):
    """Annotate a function by name in its unit's header (MW2/include/<unit>.h), in the header's
    block of the 3Dfx edition's functions that are out of the unit's order, kept in address order.
    The linter's order rule skips markers by name. False when the unit has no header."""
    unit = os.path.splitext(os.path.basename(path))[0]
    header = os.path.join("MW2", "include", os.path.relpath(os.path.dirname(path), "MW2/src"), unit + ".h")
    if not os.path.exists(header):
        return False
    lines = open(header, encoding="utf-8").read().split("\n")
    if any(MARKER.match(l) and MARKER.match(l).group(3) == TARGET and n == "// " + name for l, n in zip(lines, lines[1:])):
        return False  # annotated by name already
    intro = BY_NAME_INTRO % os.path.basename(path)
    if intro not in lines:
        guard = max(i for i, l in enumerate(lines) if l.startswith("#endif"))
        lines[guard:guard] = [intro, ""]
    start = lines.index(intro) + 2
    entries, k = [], start
    while k + 1 < len(lines) and MARKER.match(lines[k]):
        entries.append((int(MARKER.match(lines[k]).group(4), 16), lines[k + 1]))
        k += 3
    entries.append((address, "// " + name))
    block = []
    for a, n in sorted(entries):
        block += ["// FUNCTION: %s %#010x" % (TARGET, a), n, ""]
    lines[start:k] = block
    with open(header, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))
    return True


def add_annotation(name, address, note=None):
    """Insert the MW23DFX marker into name's block, after the modules that come before it, with an
    optional comment line above the block. Refuses when the file's MW23DFX annotations would leave
    address order."""
    defs = [d for d in definitions() if d["name"] == name]
    if len(defs) > 1:  # 1.1's version and the accelerated editions' (accel/, 3dfx/)
        defs = [d for d in defs if re.search(r"/(accel|3dfx)/", d["path"])]
    if len(defs) != 1 or TARGET in defs[0]["markers"]:
        return False
    d = defs[0]
    same_file = [(e["first"], e["markers"][TARGET]) for e in definitions() if e["path"] == d["path"] and TARGET in e["markers"]]
    if any(i < d["first"] and a > address or i > d["first"] and a < address for i, a in same_file):
        if add_by_name(d["path"], name, address):
            print("%s at %#x would break %s's address order; annotated by name in its header" % (name, address, os.path.basename(d["path"])))
            return True
        print("%s at %#x would break %s's address order; left out" % (name, address, os.path.basename(d["path"])))
        return False
    lines = open(d["path"], encoding="utf-8").read().split("\n")
    at = d["first"]
    for k in range(d["first"], d["last"] + 1):
        m = MARKER.match(lines[k])
        if m and m.group(3) in MODULES and MODULES.index(m.group(3)) < MODULES.index(TARGET):
            at = k + 1
    lines.insert(at, "// FUNCTION: %s %#010x" % (TARGET, address))
    if note and lines[d["first"] - 1] != note:
        lines.insert(d["first"], note)
    with open(d["path"], "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))
    return True


# --- Commands ---


def cmd_state(args):
    state = load_state(args.build, refresh=True)
    methods = Counter(v["method"].split()[0] for v in state.values())
    print("paired %d: %s" % (len(state), dict(methods)))


def rows(args, state):
    rebuild, original = Rebuild(args.build), Side(Image(BINARY), original_names())
    paths = {d["name"]: d["path"] for d in definitions()}
    for name, v in sorted(state.items()):
        if args.unit and os.path.basename(paths.get(name, "")) != args.unit:
            continue
        result = compare(rebuild, original, name, v["address"], v["end"])
        if result:
            yield name, os.path.basename(paths.get(name, "?")), v, result


def cmd_report(args):
    state = load_state(args.build)
    out = sorted(rows(args, state), key=lambda r: (-r[3][1], -r[3][0]))
    for name, unit, v, (exact, loose, ops, R, T) in out:
        print("== %.3f (%.3f without registers) %s %s %#x [%s]" % (exact, loose, name, unit, v["address"], v["method"]))
        for tag, i1, i2, j1, j2 in ops[: args.ops]:
            print("  RB: " + " | ".join(text(x) for x in R[i1:i2])[:300])
            print("  3D: " + " | ".join(text(x) for x in T[j1:j2])[:300])


def cmd_cmp(args):
    state = load_state(args.build)
    if args.address:
        address = int(args.address, 16)
        functions = original_functions(Image(BINARY), original_names())
        v = {"address": address, "end": functions.get(address, address + 0x400), "method": "given"}
    elif args.name in state:
        v = state[args.name]
    else:
        sys.exit("%s has no paired address in the 3Dfx edition (try --address)" % args.name)
    result = compare(Rebuild(args.build), Side(Image(BINARY), original_names()), args.name, v["address"], v["end"])
    if result is None:
        sys.exit("%s isn't in the rebuild's map" % args.name)
    exact, loose, ops, R, T = result
    print("== %s: original %#x [%s], ratio %.3f, %.3f without registers" % (args.name, v["address"], v["method"], exact, loose))
    for tag, i1, i2, j1, j2 in ops:
        print("  RB: " + " | ".join(text(x) for x in R[i1:i2]))
        print("  3D: " + " | ".join(text(x) for x in T[j1:j2]))


def cmd_auto(args):
    """Annotate the paired functions whose rebuilt code is the original's, addresses aside; with
    --registers, also those that differ only in their registers, with a comment saying so."""
    state = load_state(args.build)
    added = []
    for name, unit, v, (exact, loose, ops, R, T) in rows(args, state):
        if len(R) != len(T):
            continue
        if exact == 1.0 and add_annotation(name, v["address"]):
            added.append(name)
        elif args.registers and loose == 1.0 and add_annotation(name, v["address"], REGISTERS_NOTE):
            added.append(name)
    print("annotated %d: %s" % (len(added), " ".join(added)))


OPERAND_NAME = re.compile(r"([A-Za-z_@?$][\w@?$:.`']*) \((?:FUNCTION|DATA|OFFSET|UNK|IMPORT|POINTER|FLOAT|STRING|VTABLE)\)")


INDEXED = re.compile(r"\[[^\]]*\b(?:e[abcd]x|e[sd]i|e[sb]p)\b[^\]]*\((?:DATA|OFFSET)\)")


def unnamed_operands(report, loose=False):
    """From a reccmp JSON report: the functions whose only differences are operands the original's
    side can't name (<OFFSETn>), each with the names the rebuild's side gives them and the
    original's addresses, {function: {name: address}}, and the names whose call sites disagree."""
    image, names = Image(BINARY), original_names()
    by_name = {}
    for a, n in names.items():
        by_name.setdefault(n, set()).add(a)
    dis = disasm()

    def targets(address):
        x = next(dis.disasm(image.read(address, 16), address))
        found = [int(x.op_str, 16)] if x.mnemonic in ("call", "jmp") and x.op_str.startswith("0x") else []
        return found + [image.u32(a) for a in range(x.address, x.address + x.size) if a in image.relocs]

    def masked(line, pattern):
        return re.sub(r"\t\(.*", "", pattern.sub("X", line)).strip()

    offset = re.compile(r"<OFFSET\d+>")
    result, votes = {}, {}
    for f in json.load(open(report))["data"]:
        if f.get("matching", 1) >= 1 or not f["diff"]:
            continue
        needed = {}
        pairs = [
            (block.get("orig", []), block.get("recomp", []))
            for _, hunk in f["diff"]
            for block in hunk
            if "both" not in block
        ]
        for o, r in pairs:
            # A block counts only when each of its lines is the other side's but for the names:
            # in a block that differs otherwise, a line can face a different load or call.
            ok, block = len(o) == len(r), {}
            for (oa, ot), (_, rt) in zip(o, r) if ok else ():
                ok = masked(ot, offset) == masked(rt, OPERAND_NAME)
                if ok and loose and INDEXED.search(rt):
                    ok = False  # a folded displacement names whatever symbol it lands on
                if ok:
                    found = targets(int(oa, 16))
                    unknown = [t for t in found if t not in names]
                    wanted = [n for n in OPERAND_NAME.findall(rt) if not by_name.get(n, set()) & set(found)]
                    ok = wanted and len(wanted) == len(unknown)
                if not ok:
                    break
                block.update(zip(wanted, unknown))
            if ok:
                needed.update(block)
            elif not loose:  # without --all, any other difference rules the function out
                needed = None
                break
        if needed:
            result[f["name"]] = needed
            for n, a in needed.items():
                votes.setdefault(n, set()).add(a)
    claims = Counter(a for v in votes.values() for a in v)
    conflicts = {n for n, a in votes.items() if len(a) > 1 or n in by_name or any(claims[x] > 1 for x in a)}
    return result, conflicts


def add_global(name, address):
    """Insert the MW23DFX marker after the last marker of a global's definition under MW2/src (a
    definition with markers above it and none for MW23DFX). False when there's no such single one."""
    found = []
    for path in sorted(glob.glob("MW2/src/**/*.c", recursive=True)):
        lines = open(path, encoding="utf-8").read().split("\n")
        for i, line in enumerate(lines):
            m = re.match(r"^[A-Za-z][\w\s*]*?\b(\w+)\s*(?:\[[^\]]*\])*\s*(?:=.*)?[;{]?$", line)
            if m and m.group(1) == name and i and MARKER.match(lines[i - 1]):
                found.append((path, i))
    if len(found) != 1:
        return False
    path, i = found[0]
    lines = open(path, encoding="utf-8").read().split("\n")
    j = i
    while j and MARKER.match(lines[j - 1]):
        j -= 1
    block = [MARKER.match(l) for l in lines[j:i]]
    if any(m.group(2) != "GLOBAL" or m.group(3) == TARGET for m in block):
        return False
    at = j + sum(1 for m in block if m.group(3) in MODULES and MODULES.index(m.group(3)) < MODULES.index(TARGET))
    lines.insert(at, "// GLOBAL: %s %#010x" % (TARGET, address))
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))
    return True


def cmd_names(args):
    """Annotate the source functions and globals whose names the original's side of the annotated
    functions lacks: a function that differs only in unnamed operands matches once they're named."""
    needed, conflicts = unnamed_operands(args.json, args.all)
    wanted = {n: a for v in needed.values() for n, a in v.items() if n not in conflicts}
    defined = {d["name"] for d in definitions()}
    added, other = [], []
    for name, address in sorted(wanted.items()):
        if name not in defined:
            if args.write and name.startswith("g_") and add_global(name, address):
                added.append(name)
            else:
                other.append(name)
        elif args.write and add_annotation(name, address):
            added.append(name)
    ok = set(added) if args.write else {n for n in wanted if n in defined}
    ready = [f for f, v in needed.items() if all(n in ok for n in v)]
    print("%d functions differ %sin unnamed operands; %d names, %d of them disagreeing" % (
        len(needed), "" if args.all else "only ", len(wanted), len(conflicts)))
    if args.write:
        print("annotated %d: %s" % (len(added), " ".join(added)))
    print("%d functions have every name they need%s" % (len(ready), " now" if args.write else " among the source's functions"))
    print("left to annotate by hand (the C runtime, statics, globals): " + " ".join(
        "%s=%#x" % (n, wanted[n]) for n in sorted(other)))
    for n in sorted(conflicts):
        print("disagreeing: %s" % n)


def cmd_annotate(args):
    for item in args.items:
        name, address = item.split("=")
        print(name, "annotated" if add_annotation(name, int(address, 16)) else "not annotated")


def cmd_fix_order(args):
    """Drop the MW23DFX function annotations that break a file's address order, keeping the
    longest run in order."""
    by_path = {}
    for d in definitions():
        if TARGET in d["markers"]:
            by_path.setdefault(d["path"], []).append((d["first"], d["last"], d["markers"][TARGET], d["name"]))
    for path, items in by_path.items():
        items.sort()
        keep = longest_increasing([a for _, _, a, _ in items])
        drop = [items[k] for k in range(len(items)) if k not in keep]
        if not drop:
            continue
        lines = open(path, encoding="utf-8").read().split("\n")
        for first, last, a, name in sorted(drop, reverse=True):
            for k in range(first, last + 1):
                if lines[k] == "// FUNCTION: %s %#010x" % (TARGET, a):
                    del lines[k]
                    break
            print("%s: dropped %s (%#x)" % (os.path.basename(path), name, a))
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join(lines))


def cmd_unpaired(args):
    """The original's unannotated functions that nothing paired, with their references: what is
    left to find in the sources (or to write)."""
    state = load_state(args.build)
    names = original_names()
    orig = Side(Image(BINARY), names)
    taken = {v["address"] for v in state.values()}
    for a, e in sorted(original_functions(orig.image, names).items()):
        if a in names or a in taken:
            continue
        refs = orig.refs(a, e)
        before = max((n for n in names if n < a), default=None)
        where = "after %s" % names[before] if before else ""
        print("%#x %5d bytes  %-34s %s" % (a, e - a, where, " ".join(sorted(refs))[:200]))
