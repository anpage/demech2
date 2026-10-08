#!/usr/bin/env python3
"""Work on another edition of the simulator (MW2MATROX) against MW2's shared sources.

The Matrox edition's functions are 1.1's, changed or not. This tool keeps the pairing of the two
originals' functions (tools/port_annotations.py's matching, the functions placed by their
neighbours, and those named by the calls of matched ones), and compares the rebuilt edition with
its original, tolerating what VC++ 4.x varies on its own: stack slots, the operand order of
comparisons and of commutative operations. Run it from the repository root.

    python tools/edition.py state                  # (re)compute the pairing of the originals
    python tools/edition.py report [--unit F.c]    # unannotated functions, closest first, with diffs
    python tools/edition.py cmp NAME               # one function's diff, rebuild against original
    python tools/edition.py auto                   # annotate the functions that match up to entropy
    python tools/edition.py annotate NAME=0xADDR   # add a MW2MATROX annotation by hand
    python tools/edition.py fix-order              # drop annotations that break a file's address order
    python tools/edition.py crt                    # annotate the Matrox edition's C runtime from the rebuild's
    python tools/edition.py audit REPORT.json      # sort the annotated functions below 100% by what's left
    python tools/edition.py users REGEX            # functions whose body matches, with their state
    python tools/edition.py global 0xADDR          # the annotated symbol at or before an address
    python tools/edition.py scalar HEADER MEMBER...        # MechS32 members become MechScalar
    python tools/edition.py scalar-locals FILE FUNCTION VAR...  # likewise a function's locals
    python tools/edition.py include FILE...        # #include "fixedfloat.h" in sorted position

Options: --build DIR (default build-wine): the Matrox edition's rebuild is DIR/vc40/MW2_MATROX.dll and
its map. The cache is DIR/edition-MW2MATROX.json.
"""

import argparse
import bisect
import difflib
import glob
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from port_annotations import (  # noqa: E402
    Cs,
    CS_ARCH_X86,
    CS_MODE_32,
    Function,
    Image,
    aligned_pairs,
    flipped_equivalent,
    match_functions,
    scan_markers,
)

SOURCE, TARGET = "MW2", "MW2MATROX"
SOURCE_BINARY, TARGET_BINARY = "MW2.DLL", "MW2_MATROX.DLL"
DIRS = ["MW2", "common"]
MIRROR = {"jl": "jg", "jg": "jl", "jle": "jge", "jge": "jle", "jb": "ja", "ja": "jb", "jbe": "jae", "jae": "jbe"}
SLOT = re.compile(r"ebp - (0x[0-9a-f]+|\d+)")
ADDRESS = re.compile(r"0x[0-9a-f]{5,}")


def disasm():
    d = Cs(CS_ARCH_X86, CS_MODE_32)
    d.skipdata = True
    d.detail = True
    return d


def instructions(d, image, start, end):
    return [x for x in d.disasm(image.read(start, end - start), start) if x.id]


def shape(text):
    """An instruction's text without its stack slots and addresses."""
    return ADDRESS.sub("A", SLOT.sub("ebp-S", text))


def ishape(x, image=None):
    """An instruction without its stack slots, and without its addresses when it has any: a
    relocated operand, or a branch's target. Other immediates (a float constant) stay."""
    if image is None or x.group(1) or x.group(2) or x.group(7) or any(x.address + k in image.relocs for k in range(x.size)):
        return x.mnemonic + " " + shape(x.op_str)
    return x.mnemonic + " " + SLOT.sub("ebp-S", x.op_str)


def text(x):
    return x.mnemonic + " " + x.op_str


# --- The sources ---


def functions_in_sources():
    """Every function MW2 annotates in MW2/src: name -> (path, marker line index, MW2 address,
    MW2MATROX address or None)."""
    result = {}
    for path in sorted(glob.glob("MW2/src/**/*.c", recursive=True)):
        lines = open(path, encoding="utf-8").read().split("\n")
        for i, line in enumerate(lines):
            m = re.match(r"// FUNCTION: %s (0x[0-9a-f]+)$" % SOURCE, line)
            if not m:
                continue
            target = None
            for k in range(i + 1, min(i + 6, len(lines))):
                t = re.match(r"// FUNCTION: %s (0x[0-9a-f]+)" % TARGET, lines[k])
                if t:
                    target = int(t.group(1), 16)
                    continue
                n = re.match(r"^[A-Za-z].*?\b(\w+)\(", lines[k])
                if n and not lines[k].startswith("//"):
                    result[n.group(1)] = (path, i, int(m.group(1), 16), target)
                    break
    return result


def annotated_targets():
    """The addresses MW2MATROX annotates in MW2/src, the Matrox edition's own files included."""
    result = set()
    for path in glob.glob("MW2/src/**/*.c", recursive=True):
        for m in re.finditer(r"^// FUNCTION: %s (0x[0-9a-f]+)$" % TARGET, open(path, encoding="utf-8").read(), re.M):
            result.add(int(m.group(1), 16))
    return result


def file_order_allows(functions, path, i, address):
    """Whether a MW2MATROX annotation at address keeps the file's annotations in address order."""
    before = [t for p, j, _, t in functions.values() if p == path and j < i and t is not None]
    after = [t for p, j, _, t in functions.values() if p == path and j > i and t is not None]
    return all(t < address for t in before) and all(t > address for t in after)


def add_annotation(name, address, functions=None):
    functions = functions or functions_in_sources()
    path, i, _, target = functions[name]
    if target is not None:
        return False
    marker = "// FUNCTION: %s %#010x" % (TARGET, address)
    if any(marker in open(p, encoding="utf-8").read() for p in glob.glob("MW2/src/**/*.c", recursive=True)):
        return False  # annotated already (the Matrox edition's own version of the function, say)
    if not file_order_allows(functions, path, i, address):
        print("%s at %#x would break %s's address order; left out" % (name, address, os.path.basename(path)))
        return False
    lines = open(path, encoding="utf-8").read().split("\n")
    lines.insert(i + 1, "// FUNCTION: %s %#010x" % (TARGET, address))
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))
    return True


# --- The pairing of the originals ---


def compute_state(path):
    source, target = Image(SOURCE_BINARY), Image(TARGET_BINARY)
    markers = scan_markers(DIRS)
    src = [m for m in markers if m[3] == SOURCE]
    starts = [m[4] for m in src if m[2] in ("FUNCTION", "LIBRARY", "SYNTHETIC", "STUB")]
    code = [m[4] for m in src if m[2] == "FUNCTION"]
    mapping, functions, _, _, _ = match_functions(source, target, starts, code, [])
    _, placed = aligned_pairs(source, target, mapping, functions, [])
    known = dict(mapping)
    known.update({a: b for a, (b, e) in placed.items()})

    # The calls of the paired functions name the Matrox edition's version of more of them.
    d = disasm()
    from collections import Counter, defaultdict

    votes = defaultdict(Counter)
    for a, b in known.items():
        f = functions[a]
        end = placed[a][1] if a in placed else b + len(f.code)
        sc = [x for x in instructions(d, source, a, f.end) if x.mnemonic == "call" and x.op_str.startswith("0x")]
        tc = [x for x in instructions(d, target, b, end) if x.mnemonic == "call" and x.op_str.startswith("0x")]
        if len(sc) == len(tc):
            for x, y in zip(sc, tc):
                votes[int(x.op_str, 16)][int(y.op_str, 16)] += 1
    # A call the Matrox edition makes into its C runtime (the /QIfdiv helpers, say) names no
    # function of the game: leave the CRT's addresses out, from its first LIBRARY annotation on.
    library = [m[4] for m in markers if m[3] == TARGET and m[2] == "LIBRARY"]
    crt = min(library) if library else None
    inferred = {}
    for x, c in votes.items():
        if x in functions and x not in known:
            y, n = c.most_common(1)[0]
            if n > sum(c.values()) - n and (crt is None or y < crt):
                inferred[x] = y
    state = {
        "mapping": {str(a): b for a, b in mapping.items()},
        "placed": {str(a): [b, e] for a, (b, e) in placed.items()},
        "inferred": {str(a): b for a, b in inferred.items()},
    }
    with open(path, "w") as f:
        json.dump(state, f)
    return state


def load_state(build, refresh=False):
    path = os.path.join(build, "edition-%s.json" % TARGET)
    if refresh or not os.path.exists(path):
        print("computing the pairing of the originals (a minute) ...", file=sys.stderr)
        state = compute_state(path)
    else:
        state = json.load(open(path))
    return (
        {int(a): b for a, b in state["mapping"].items()},
        {int(a): tuple(v) for a, v in state["placed"].items()},
        {int(a): b for a, b in state["inferred"].items()},
    )


def target_address(state, address):
    mapping, placed, inferred = state
    if address in mapping:
        return mapping[address], "identical"
    if address in placed:
        return placed[address][0], "changed"
    if address in inferred:
        return inferred[address], "changed (by its callers)"
    return None, "not found"


# --- The rebuild ---


class Rebuild:
    def __init__(self, build):
        base = os.path.join(build, "vc40", "MW2_MATROX")
        if not os.path.exists(base + ".dll"):
            sys.exit("no rebuilt edition at %s.dll" % base)
        self.image = Image(base + ".dll")
        self.symbols = {}
        for line in open(base + ".map", errors="replace"):
            m = re.match(r"\s*0001:[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})", line)
            if m:
                self.symbols.setdefault(m.group(1), int(m.group(2), 16))
        starts = sorted(set(self.symbols.values()))
        self.following = dict(zip(starts, starts[1:] + [self.image.text[1]]))
        self.disasm = disasm()

    def function(self, name):
        start = self.symbols.get("_" + name)
        if start is None:
            return None
        return Function(self.image, start, self.following[start], self.disasm)


def compare(rebuild, original, name, address, end=None):
    """The rebuilt function name against the original's at address: (verdict, ratio, ops, R, T).
    The verdict is "match" when the differences are entropy only."""
    f = rebuild.function(name)
    if f is None:
        return None
    if f.matches(original, address) or flipped_equivalent(rebuild.image, original, f, address, address + len(f.code)):
        return "match", 1.0, [], [], []
    d = rebuild.disasm
    R = instructions(d, rebuild.image, f.start, f.start + len(f.code))
    T = instructions(d, original, address, end or address + len(f.code) + 0x40)
    sm = difflib.SequenceMatcher(
        None, [ishape(x, rebuild.image) for x in R], [ishape(x, original) for x in T], autojunk=False
    )
    ops = [o for o in sm.get_opcodes() if o[0] != "equal"]
    if ops and ops[-1][0] == "insert" and ops[-1][1] == len(R):
        ops = ops[:-1]  # past the rebuilt function's end: the original's next function
    entropy = all(entropy_only(R, T, op) for op in ops)
    return ("match" if entropy else "differs"), sm.ratio(), ops, R, T


def entropy_only(R, T, op):
    tag, i1, i2, j1, j2 = op
    if tag != "replace":
        return False
    if i2 - i1 == j2 - j1 == 1:
        return MIRROR.get(R[i1].mnemonic) == T[j1].mnemonic
    if i2 - i1 == j2 - j1 == 2:
        (a, b), (c, e) = R[i1:i2], T[j1:j2]
        if [a.mnemonic, c.mnemonic] != ["mov", "mov"] or b.mnemonic != e.mnemonic:
            return False
        A, B, C, E = [[shape(p.strip()) for p in z.op_str.split(",")] for z in (a, b, c, e)]
        if b.mnemonic == "cmp":
            nxt = R[i2].mnemonic if i2 < len(R) else ""
            return nxt in ("je", "jne") and A[0] == C[0] == B[1] == E[1] and A[1] == E[0] and B[0] == C[1]
        if b.mnemonic in ("add", "imul", "and", "or", "xor") and len(B) == 2:
            return A[0] == C[0] == B[0] == E[0] and A[1] == E[1] and B[1] == C[1]
    return False


def candidates(state, functions, unit=None):
    """The unannotated functions with an address in the Matrox edition: (name, address, end)."""
    mapping, placed, inferred = state
    annotated = annotated_targets()
    for name, (path, i, address, target) in sorted(functions.items()):
        if target is not None or (unit and os.path.basename(path) != unit):
            continue
        b, _ = target_address(state, address)
        if b is None or b in annotated:
            continue  # annotated already: the Matrox edition's own version, in another file
        end = placed[address][1] if address in placed else None
        yield name, path, b, end


# --- Commands ---


def cmd_state(args):
    load_state(args.build, refresh=True)
    print("done")


def cmd_report(args):
    state = load_state(args.build)
    functions = functions_in_sources()
    rebuild, original = Rebuild(args.build), Image(TARGET_BINARY)
    rows = []
    for name, path, b, end in candidates(state, functions, args.unit):
        result = compare(rebuild, original, name, b, end)
        if result:
            rows.append((result[1], name, os.path.basename(path), b, result))
    rows.sort(key=lambda r: -r[0])
    for ratio, name, unit, b, (verdict, _, ops, R, T) in rows:
        print("== %.3f %s %s %#x%s" % (ratio, name, unit, b, " (matches up to entropy)" if verdict == "match" else ""))
        for tag, i1, i2, j1, j2 in ops[: args.ops]:
            print("  RB: " + " | ".join(text(x) for x in R[i1:i2])[:300])
            print("  MX: " + " | ".join(text(x) for x in T[j1:j2])[:300])


def cmd_cmp(args):
    state = load_state(args.build)
    functions = functions_in_sources()
    path, i, address, target = functions[args.name]
    b = int(args.address, 16) if args.address else (target or target_address(state, address)[0])
    if b is None:
        sys.exit("%s has no address in the Matrox edition (try --address)" % args.name)
    end = state[1][address][1] if address in state[1] else None
    result = compare(Rebuild(args.build), Image(TARGET_BINARY), args.name, b, end)
    verdict, ratio, ops, R, T = result
    print("== %s: original %#x, %s, ratio %.3f" % (args.name, b, verdict, ratio))
    for tag, i1, i2, j1, j2 in ops:
        print("  RB: " + " | ".join(text(x) for x in R[i1:i2]))
        print("  MX: " + " | ".join(text(x) for x in T[j1:j2]))


def cmd_auto(args):
    state = load_state(args.build)
    functions = functions_in_sources()
    rebuild, original = Rebuild(args.build), Image(TARGET_BINARY)
    done = []
    for name, path, b, end in candidates(state, functions, args.unit):
        result = compare(rebuild, original, name, b, end)
        if result and result[0] == "match":
            done.append((name, b))
    added = [n for n, b in done if add_annotation(n, b)]
    print("annotated %d: %s" % (len(added), " ".join(added)))


def cmd_fix_order(args):
    """Drop the MW2MATROX function annotations that break a file's address order, keeping the
    longest run in order."""
    from port_annotations import longest_increasing

    by_path = {}
    for name, (path, i, _, target) in functions_in_sources().items():
        if target is not None:
            by_path.setdefault(path, []).append((i, target, name))
    for path, items in by_path.items():
        items.sort()
        keep = longest_increasing([t for _, t, _ in items])
        drop = [items[k] for k in range(len(items)) if k not in keep]
        if not drop:
            continue
        lines = open(path, encoding="utf-8").read().split("\n")
        for i, t, name in sorted(drop, reverse=True):
            for k in range(i + 1, i + 4):
                if lines[k] == "// FUNCTION: %s %#010x" % (TARGET, t):
                    del lines[k]
                    break
            print("%s: dropped %s (%#x)" % (os.path.basename(path), name, t))
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write("\n".join(lines))


def cmd_crt(args):
    """Annotate the Matrox edition's C runtime from the rebuild's: the rebuild links VC++ 4.0's LIBCMT,
    as the Matrox edition did. Functions match by their bytes (relocations masked) and take the map's
    public symbol; the CRT data they address takes its C name through the relocations they share."""
    base = os.path.join(args.build, "vc40", "MW2_MATROX")
    rebuilt, original = Image(base + ".dll"), Image(TARGET_BINARY)
    publics = []
    for line in open(base + ".map", errors="replace"):
        m = re.match(r"\s*(000[1-9]):[0-9a-f]{8}\s+(\S+)\s+([0-9a-f]{8})\s+(f\s+)?(\S+)\s*$", line)
        if m:
            publics.append((int(m.group(3), 16), m.group(2), m.group(1), m.group(5)))
    code = sorted({a for a, _, sec, _ in publics if sec == "0001"})
    following = dict(zip(code, code[1:] + [rebuilt.text[1]]))
    by_address = {}
    for a, name, sec, obj in publics:
        by_address.setdefault(a, (name, sec, obj))
    d = disasm()
    functions, pairs = {}, []
    for a, name, sec, obj in publics:
        if sec != "0001" or not obj.startswith("LIBCMT:") or a in functions.values():
            continue
        f = Function(rebuilt, a, following[a], d)
        if len(f.code) < 8:
            continue
        found = [original.text[0] + m.start() for m in f.pattern().finditer(original.text_bytes)]
        found = [b for b in found if f.matches(original, b) == "same"]
        if len(found) == 1:
            functions[name] = found[0]
            pairs += [(rebuilt.u32(a + r), original.u32(found[0] + r)) for r in f.relocs]
    data = {}
    for s_, t in pairs:
        entry = by_address.get(s_)
        if entry and entry[1] != "0001" and entry[2].startswith("LIBCMT:"):
            data.setdefault(entry[0], set()).add(t)
    data = {name[1:]: next(iter(v)) for name, v in data.items() if len(v) == 1}
    print("CRT functions matched: %d, data: %d" % (len(functions), len(data)))

    path = "MW2/library_msvc.h"
    lines = open(path, encoding="utf-8").read().split("\n")
    have = set()
    for i, line in enumerate(lines):
        m = re.match(r"// (LIBRARY|GLOBAL): %s (0x[0-9a-f]+)" % TARGET, line)
        if m:
            have.add(int(m.group(2), 16))
    # Stack on 1.1's entry for the same name, else append.
    entries = [("LIBRARY", n, a, " SYMBOL") for n, a in functions.items()] + [("GLOBAL", n, a, "") for n, a in data.items()]
    appended = []
    for kind, name, address, rest in sorted(entries, key=lambda e: e[2]):
        if address in have:
            continue
        marker = "// %s: %s %#010x%s" % (kind, TARGET, address, rest)
        for i in range(len(lines) - 1):
            if re.match(r"// %s: %s 0x[0-9a-f]+%s$" % (kind, SOURCE, rest), lines[i]):
                k = i + 1
                while k < len(lines) and re.match(r"// (LIBRARY|GLOBAL): ", lines[k]):
                    k += 1
                if k < len(lines) and lines[k] == "// " + name and not any(
                    re.match(r"// %s: %s " % (kind, TARGET), lines[j]) for j in range(i, k)
                ):
                    lines.insert(k, marker)
                    break
        else:
            appended += [marker, "// " + name, ""]
        have.add(address)
    if appended:
        end = max(i for i, l in enumerate(lines) if l.startswith("#endif"))
        lines[end:end] = ["// %s's C runtime, matched against the rebuild's (tools/edition.py crt)." % TARGET, ""] + appended
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))


def cmd_audit(args):
    """Sort the annotated functions below 100% by what is left, from a reccmp JSON report
    (reccmp-reccmp --target MW2MATROX --json FILE): entropy (the same instructions, reordered or in
    other stack slots), references the original's side can't name yet, or other differences."""
    import ast
    from collections import Counter

    stubs = set()
    for path in glob.glob("MW2/src/**/*.c", recursive=True):
        stubs |= {int(m, 16) for m in re.findall(r"// STUB: %s (0x[0-9a-f]+)" % TARGET, open(path, encoding="utf-8").read())}

    def norm(line):
        line = re.sub(r"\s*\t\(.*\)$", "", line)
        line = SLOT.sub("ebp-S", line)
        return re.sub(r"^(j\w+|call) -?0x[0-9a-f]+$", r"\1 J", line)

    kinds, rows = Counter(), []
    for e in json.load(open(args.json))["data"]:
        address = int(e["address"], 16)
        if address >= args.code_end or address in stubs or float(e["matching"]) >= 1.0 or not e.get("diff"):
            continue
        diff = ast.literal_eval(e["diff"]) if isinstance(e["diff"], str) else e["diff"]
        old, new = Counter(), Counter()
        for _, blocks in diff:
            for b in blocks:
                old.update(norm(x[1]) for x in b.get("orig", []))
                new.update(norm(x[1]) for x in b.get("recomp", []))
        only_old, only_new = old - new, new - old
        if not only_old and not only_new:
            kind = "entropy"
        elif all("<OFFSET" in x for x in only_old):
            kind = "unnamed"
        else:
            kind = "other"
        kinds[kind] += 1
        rows.append((kind, float(e["matching"]), e["name"], list(only_old.elements())[:3], list(only_new.elements())[:3]))
    print(dict(kinds))
    for kind, score, name, o, n in sorted(rows):
        if args.kind in (None, kind):
            print("%-8s %.2f %-28s MX: %s" % (kind, score, name, " | ".join(o)))
            print("%-8s %4s %-28s RB: %s" % ("", "", "", " | ".join(n)))


def cmd_annotate(args):
    for item in args.items:
        name, address = item.split("=")
        # Each insertion moves the lines after it: read the sources again for every name.
        print(name, "annotated" if add_annotation(name, int(address, 16)) else "not annotated")


def cmd_users(args):
    state = load_state(args.build)
    for name, (path, i, address, target) in sorted(functions_in_sources().items()):
        lines = open(path, encoding="utf-8").read().split("\n")
        end = i + 1
        while end < len(lines) and not lines[end].startswith("// FUNCTION: " + SOURCE + " "):
            end += 1
        if re.search(args.regex, "\n".join(lines[i:end])):
            if target is not None:
                status = "annotated %#x" % target
            else:
                b, kind = target_address(state, address)
                status = kind + (" %#x" % b if b else "")
            print("%-30s %-18s %#x  %s" % (name, os.path.basename(path), address, status))


def cmd_global(args):
    entries = []
    for path in glob.glob("MW2/src/**/*.c", recursive=True) + glob.glob("MW2/include/*.h") + glob.glob("common/include/*.h"):
        lines = open(path, encoding="utf-8").read().split("\n")
        for i, line in enumerate(lines):
            m = re.match(r"// (GLOBAL|FUNCTION|LIBRARY): %s (0x[0-9a-f]+)" % TARGET, line)
            if not m:
                continue
            name = "?"
            for k in range(i + 1, min(i + 5, len(lines))):
                if re.match(r"// (GLOBAL|FUNCTION|LIBRARY):", lines[k]):
                    continue
                if lines[k].startswith("// "):
                    name = lines[k][3:].strip()
                    break
                n = re.search(r"([A-Za-z_]\w*)\s*(\(|\[|=|;)", lines[k])
                if n:
                    name = n.group(1)
                break
            entries.append((int(m.group(2), 16), name, m.group(1)))
    entries.sort()
    keys = [e[0] for e in entries]
    for a in args.addresses:
        k = bisect.bisect_right(keys, int(a, 16)) - 1
        print(a, "%s+%#x (%s)" % (entries[k][1], int(a, 16) - entries[k][0], entries[k][2]) if k >= 0 else "?")


def include_fixedfloat(path):
    s = open(path, encoding="utf-8").read()
    if '#include "fixedfloat.h"' in s:
        return
    incs = re.findall(r'^#include "([\w]+\.h)"$', s, re.M)
    if path.endswith(".c"):
        incs = incs[1:]  # the unit's own header comes first
    before = [h for h in incs if h < "fixedfloat.h"]
    if before:
        s = s.replace('#include "%s"\n' % before[-1], '#include "%s"\n#include "fixedfloat.h"\n' % before[-1], 1)
    elif incs:
        s = s.replace('#include "%s"\n' % incs[0], '#include "fixedfloat.h"\n#include "%s"\n' % incs[0], 1)
    else:
        print("no include to put fixedfloat.h next to in", path)
        return
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(s)


def cmd_include(args):
    for path in args.files:
        include_fixedfloat(path)


def cmd_scalar(args):
    s = open(args.header, encoding="utf-8").read()
    names, done = set(args.members), set()

    def rep(m):
        if m.group(2) in names:
            done.add(m.group(2))
            return m.group(1) + "MechScalar " + m.group(2) + m.group(3)
        return m.group(0)

    s = re.sub(r"^(\s*)MechS32 (m_\w+)((?:\[[^\]]*\])?;)", rep, s, flags=re.M)
    with open(args.header, "w", encoding="utf-8", newline="\n") as f:
        f.write(s)
    if done:
        include_fixedfloat(args.header)
    if names - done:
        print("not found:", " ".join(sorted(names - done)))


def cmd_scalar_locals(args):
    lines = open(args.file, encoding="utf-8").read().split("\n")
    start = next(i for i, l in enumerate(lines) if re.match(r"^[A-Za-z].*\b%s\(" % args.function, l) and not l.rstrip().endswith(";"))
    end = next(i for i in range(start, len(lines)) if lines[i] == "}")
    names, done = set(args.vars), set()
    for i in range(start, end):
        m = re.match(r"^(\s*)MechS32 (\w+);(.*)$", lines[i])
        if m and m.group(2) in names:
            lines[i] = "%sMechScalar %s;%s" % m.groups()
            done.add(m.group(2))
    with open(args.file, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))
    if done:
        include_fixedfloat(args.file)
    if names - done:
        print("not found:", " ".join(sorted(names - done)))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build", default="build-wine")
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("state").set_defaults(fn=cmd_state)
    p = sub.add_parser("report")
    p.add_argument("--unit")
    p.add_argument("--ops", type=int, default=12)
    p.set_defaults(fn=cmd_report)
    p = sub.add_parser("cmp")
    p.add_argument("name")
    p.add_argument("--address")
    p.set_defaults(fn=cmd_cmp)
    p = sub.add_parser("auto")
    p.add_argument("--unit")
    p.set_defaults(fn=cmd_auto)
    sub.add_parser("fix-order").set_defaults(fn=cmd_fix_order)
    sub.add_parser("crt").set_defaults(fn=cmd_crt)
    p = sub.add_parser("audit")
    p.add_argument("json")
    p.add_argument("--kind", choices=["entropy", "unnamed", "other"])
    p.add_argument("--code-end", type=lambda x: int(x, 16), default=0x10092D30)
    p.set_defaults(fn=cmd_audit)
    p = sub.add_parser("annotate")
    p.add_argument("items", nargs="+")
    p.set_defaults(fn=cmd_annotate)
    p = sub.add_parser("users")
    p.add_argument("regex")
    p.set_defaults(fn=cmd_users)
    p = sub.add_parser("global")
    p.add_argument("addresses", nargs="+")
    p.set_defaults(fn=cmd_global)
    p = sub.add_parser("include")
    p.add_argument("files", nargs="+")
    p.set_defaults(fn=cmd_include)
    p = sub.add_parser("scalar")
    p.add_argument("header")
    p.add_argument("members", nargs="+")
    p.set_defaults(fn=cmd_scalar)
    p = sub.add_parser("scalar-locals")
    p.add_argument("file")
    p.add_argument("function")
    p.add_argument("vars", nargs="+")
    p.set_defaults(fn=cmd_scalar_locals)
    args = parser.parse_args()
    args.fn(args)


if __name__ == "__main__":
    main()
