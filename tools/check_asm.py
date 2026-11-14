#!/usr/bin/env python3
"""Check that assembly in the C sources is only where the original was assembly.

A function can be made to match by transcribing the original's bytes (`__asm`, `_emit`,
`__declspec(naked)`), which proves nothing about it. The project writes assembly only where the
original was hand-written assembly (CLAUDE.md, "Hand-written Assembly"), and this check holds it
to that: every function and macro of the game's sources that contains `__asm` or `_emit`, or is
declared `naked`, must be listed in `tools/asm_allowlist.txt` with a fingerprint, and the list may
hold nothing else.

A fingerprint is a property of the original's code that a VC++ compiler at /Od doesn't produce,
checked against each /Od original that annotates the function and is available (`--verify`):

  div64       a `div`/`idiv` of a 64-bit dividend: edx not set by `cdq` or `xor edx, edx` just
              before (C divides an `__int64` through `__alldiv`)
  round       an `adc` right after a shift: rounding with the bit shifted out
  eax-result  the body falls into the epilogue with its result just written to eax, where a C
              `return` jumps there (constructors, which return `this` so, excepted)
  no-frame    no /Od frame (`push ebp; mov ebp, esp; [sub esp, N;] push ebx; push esi; push edi`)
  insn:M      an instruction /Od C doesn't emit (`bsr`, `loop`, `int3`, `xlatb`, `in`, `fsin`...)
  copy:PATH   the same assembly as the function of that name in PATH (an inline copy in a header)
  reviewed: REASON
              none of the above holds: a person read it (a register kept across statements, a
              `rep movsd` that the /Oi intrinsics also emit). Listed, never counted as verified.

The fingerprints were validated against every annotated function of the /Od originals that has
no assembly: none of them shows any of these. The original's code is read by following its
control flow (branches and jump tables), not linearly: switch tables are data.

usage (from the repository root):
  python tools/check_asm.py [--verify] [--targets MW2 MW2SHELL ...] [--tree DIR]
"""

import argparse
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from unitbuild import ROOT, Image  # noqa: E402

ROOTS = ("MW2", "MW2SHELL", "NETMECHW", "MECH2", "common", "util")
# The /Od targets, in the order a function is verified (the first that annotates it and whose
# original is here).
OD_TARGETS = ("MW2", "MW2SHELL", "NETMECHW", "MECH2", "MW2MATROX")
INSTRUCTIONS = {"bsr", "bsf", "bswap", "xlatb", "lodsb", "lodsw", "lodsd", "scasb", "scasw", "scasd", "loop", "loope",
                "loopne", "jecxz", "pushal", "popal", "in", "out", "cli", "sti", "fsin", "fcos", "fsincos", "fpatan",
                "fptan", "fyl2x", "fyl2xp1", "f2xm1", "fscale", "int3", "rcl", "rcr"}
KINDS = ("div64", "round", "eax-result", "no-frame")

TOKEN = re.compile(r"""//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|'(?:\\.|[^'\\\n])*'|^[ \t]*#(?:[^\n]*\\\n)*[^\n]*|[A-Za-z_$][\w$]*|\d[\w.]*|::|\S""", re.S | re.M)
ASM = re.compile(r"\b(__asm|_emit)\b")


def definitions(text):
    """[(kind, name, body)]: the functions defined at file scope ("function") and the macros
    ("macro") of a source, with their text."""
    out, stack, paren, candidate, start, previous = [], [], 0, None, None, ""
    for m in TOKEN.finditer(text):
        token = m.group(0)
        if token.startswith(("//", "/*")):
            continue
        if token.lstrip().startswith("#"):
            d = re.match(r"\s*#\s*define\s+(\w+)", token)
            if d and ASM.search(token):
                out.append(("macro", d.group(1), token))
            continue
        depth = sum(1 for s in stack if s != "extern")
        if depth == 0:
            if token == "(":
                if paren == 0:
                    q = re.search(r"((?:[A-Za-z_]\w*\s*::\s*)*~?[A-Za-z_]\w*)\s*$", text[: m.start()])
                    candidate = re.sub(r"\s+", "", q.group(1)) if q else None
                    head = m.start()
                paren += 1
            elif token == ")":
                paren -= 1
            elif token == ";":
                candidate = None
            elif token == "{":
                if previous.startswith('"'):
                    stack.append("extern")
                else:
                    if candidate and paren == 0:
                        start, name = m.start(), candidate
                        line_start = max(text.rfind(";", 0, head), text.rfind("}", 0, head), text.rfind("\n\n", 0, head))
                        naked = "naked" in text[line_start:head]
                    stack.append("{")
                candidate = None
            elif token == "}" and stack:
                stack.pop()
        elif token == "{":
            stack.append("{")
        elif token == "}":
            stack.pop()
            if start is not None and not any(s != "extern" for s in stack):
                body = text[start : m.end()]
                if ASM.search(body) or naked:
                    out.append(("function", name, body))
                start = None
        previous = token
    return out


def scan(tree):
    """{(path, name): body} of every function and macro with assembly in the game's sources."""
    found = {}
    for root in ROOTS:
        for path in sorted((tree / root).rglob("*")):
            if path.suffix not in (".c", ".cpp", ".h") or not path.is_file():
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            if not re.search(r"__asm|_emit|naked", text):
                continue
            rel = path.relative_to(tree).as_posix()
            for kind, name, body in definitions(text):
                key = (rel, name if kind == "function" else "macro:" + name)
                found[key] = body
    return found


def read_allowlist(tree):
    entries = {}
    for number, line in enumerate((tree / "tools" / "asm_allowlist.txt").read_text().split("\n"), 1):
        if not line.strip() or line.startswith("#"):
            continue
        path, name, rest = (line.split(None, 2) + ["", ""])[:3]
        fingerprints, reason = [], None
        m = re.search(r"\breviewed:\s*(.*)$", rest)
        if m:
            reason, rest = m.group(1).strip(), rest[: m.start()]
        fingerprints = rest.split()
        entries[(path, name)] = {"line": number, "fingerprints": fingerprints, "reason": reason}
    return entries


def asm_text(body):
    """The assembly of a body, whitespace normalized: for comparing a copy."""
    blocks = re.findall(r"__asm\s*\{(.*?)\}|__asm\s+([^\n]*)", body, re.S)
    return re.sub(r"\s+", " ", " ".join(a or b for a, b in blocks)).strip()


# ---------------------------------------------------------------------------------------------
# The original's code


def reachable(image, start, end):
    """The instructions reachable from `start` within [start, end), in address order, following
    fallthrough, direct branches and jump tables (`jmp [reg*4 + table]`)."""
    from capstone import CS_ARCH_X86, CS_GRP_JUMP, CS_GRP_RET, CS_MODE_32, Cs
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM

    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    disassembler.detail = True
    seen, todo, found = set(), [start], {}
    while todo:
        address = todo.pop()
        while start <= address < end and address not in seen:
            seen.add(address)
            x = next(disassembler.disasm(image.at(address, 16), address, 1), None)
            if x is None:
                break
            found[address] = x
            if x.group(CS_GRP_RET):
                break
            if x.group(CS_GRP_JUMP):
                op = x.operands[0]
                if op.type == X86_OP_IMM:
                    todo.append(op.imm)
                elif op.type == X86_OP_MEM and op.mem.scale == 4 and start <= op.mem.disp < end:
                    entry = op.mem.disp
                    while start <= entry < end:
                        value = struct.unpack("<I", image.at(entry, 4))[0]
                        if not start <= value < end:
                            break
                        todo.append(value)
                        entry += 4
                if x.mnemonic == "jmp":
                    break
            address += x.size
    return [found[k] for k in sorted(found)]


def fingerprints(image, start, end, constructor):
    """The fingerprints the original's function at [start, end) shows."""
    from capstone.x86 import X86_OP_IMM, X86_OP_REG, X86_REG_AL, X86_REG_AX, X86_REG_EAX

    code = image.at(start, end - start).rstrip(b"\xcc")
    ins = reachable(image, start, start + len(code))
    found = set()
    k = 3
    if code[k : k + 1] == b"\x83":
        k += 3
    elif code[k : k + 1] == b"\x81":
        k += 6
    if code[:3] != b"\x55\x8b\xec" or code[k : k + 3] != b"\x53\x56\x57":
        found.add("no-frame")
    for i, x in enumerate(ins):
        if x.mnemonic in INSTRUCTIONS:
            found.add("insn:" + x.mnemonic)
        previous = ins[i - 1] if i else None
        if x.mnemonic in ("idiv", "div") and previous and not (previous.mnemonic == "cdq" or previous.mnemonic in ("xor", "sub") and previous.op_str == "edx, edx"):
            found.add("div64")
        if x.mnemonic == "adc" and previous and previous.mnemonic in ("shrd", "shr", "sar", "shl", "shld"):
            found.add("round")
        epilogue = [y.mnemonic for y in ins[i : i + 5]] == ["pop", "pop", "pop", "leave", "ret"] and x.op_str == "edi"
        if epilogue and previous and not constructor and previous.address + previous.size == x.address:
            jumped = any(y.mnemonic.startswith("j") and y.operands and y.operands[0].type == X86_OP_IMM and y.operands[0].imm == x.address for y in ins)
            target = previous.operands[0] if previous.operands else None
            if not jumped and target is not None and target.type == X86_OP_REG and target.reg in (X86_REG_EAX, X86_REG_AX, X86_REG_AL) and previous.mnemonic not in ("push", "cmp", "test"):
                found.add("eax-result")
    return found


def verify(entries, scanned, targets, tree):
    """Check each entry's machine fingerprints against the first /Od original that annotates it.
    Returns (failures, verified count, entries no available original covers)."""
    from funccmp import Annotations, read_yaml_paths, source_roots

    paths = read_yaml_paths(tree)
    loaded = {}
    failures, verified, unchecked = [], 0, []
    for (path, name), entry in sorted(entries.items()):
        machine = [f for f in entry["fingerprints"] if not f.startswith("copy:")]
        for f in entry["fingerprints"]:
            if f.startswith("copy:"):
                other = scanned.get((f[5:], name))
                if other is None:
                    failures.append("%s %s: copy:%s: no such function with assembly" % (path, name, f[5:]))
                elif asm_text(other) != asm_text(scanned.get((path, name), "")):
                    failures.append("%s %s: its assembly isn't %s's" % (path, name, f[5:]))
        if not machine:
            continue
        done = False
        for target in targets:
            if target not in paths or not Path(paths[target]).exists():
                continue
            if not any(path.startswith(r + "/") for r in source_roots(target, tree)):
                continue
            if target not in loaded:
                loaded[target] = (Annotations(target, source_roots(target, tree), tree), Image(paths[target]))
            annotations, image = loaded[target]
            found = [a for a in annotations.functions.get(name, []) if Path(a.filename).resolve() == (tree / path).resolve()]
            if not found:
                continue
            start = found[0].offset
            later = [a for a in annotations.addresses if a > start]
            end = min(later) if later else start + 0x1000
            have = fingerprints(image, start, end, re.search(r"(\w+)::~?\1$", name) is not None)
            missing = [f for f in machine if f not in have]
            if missing:
                failures.append("%s %s: %s %#x doesn't show %s (it shows: %s)" % (path, name, target, start, " ".join(missing), " ".join(sorted(have)) or "nothing"))
            else:
                verified += 1
            done = True
            break
        if not done:
            unchecked.append("%s %s" % (path, name))
    return failures, verified, unchecked


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--verify", action="store_true", help="Check the fingerprints against the originals in reccmp-user.yml")
    parser.add_argument("--targets", nargs="*", default=list(OD_TARGETS), help="The originals to verify against, in order")
    parser.add_argument("--tree", default=str(ROOT), help="The source tree to check (default: this one)")
    parser.add_argument("-v", "--verbose", action="store_true")
    args = parser.parse_args()

    tree = Path(args.tree).resolve()
    scanned = scan(tree)
    entries = read_allowlist(tree)
    failures = []
    for key in sorted(set(scanned) - set(entries)):
        failures.append("%s %s: assembly not in tools/asm_allowlist.txt" % key)
    for key in sorted(set(entries) - set(scanned)):
        failures.append("%s %s: listed, but has no assembly (line %d): remove it" % (key[0], key[1], entries[key]["line"]))
    for key, entry in sorted(entries.items()):
        bad = [f for f in entry["fingerprints"] if not (f in KINDS or f.startswith("copy:") or f.startswith("insn:") and f[5:] in INSTRUCTIONS)]
        if bad:
            failures.append("%s %s: unknown fingerprint %s" % (key[0], key[1], " ".join(bad)))
        if not entry["fingerprints"] and not entry["reason"]:
            failures.append("%s %s: no fingerprint and no `reviewed:` reason" % key)
    reviewed = sum(1 for e in entries.values() if e["reason"] and not e["fingerprints"])
    message = "%d with assembly, %d listed (%d reviewed by hand only)" % (len(scanned), len(entries), reviewed)
    if args.verify:
        more, verified, unchecked = verify(entries, scanned, args.targets, tree)
        failures += more
        message += "; %d verified against an original" % verified
        if unchecked:
            message += ", %d not checkable here (no original that annotates them)" % len(unchecked)
            if args.verbose:
                for line in unchecked:
                    print("  not checked: " + line)
    for failure in failures:
        print(failure)
    print(message + ("; %d failures" % len(failures) if failures else ""))
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
