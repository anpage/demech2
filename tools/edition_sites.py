#!/usr/bin/env python3
"""List every conditional site the three simulator targets disagree on, and what proves it.

The simulators (MW2, MW2MATROX, MW23DFX) build one tree, which tells the editions apart with the
edition macros, MW2_ACCEL (the accelerated editions), MW2_MATROX and MW2_3DFX, and the features
util/types.h defines for each edition (MW2_FLOAT_SCALARS, MW2_KEY_COLOR...). A site is an
#if/#ifdef/#ifndef group (with its #elif and #else branches) in MW2/ or common/, or an
IF/IFDEF/IFNDEF group in the MASM sources the targets assemble (3rdparty/vfx, MW2/src/accel),
whose condition names an edition macro or a feature, alone or with other macros. A group nested in another
site is a site of its own, with its parent recorded.

Which target compiles which branch comes from the targets' own compile commands, not from a list
kept here: each unit of each target (tools/unitbuild.py, from the build tree's NMake files) is
preprocessed with its own /D options and the compiler's predefined macros (_MSC_VER 1010 for
VC++ 4.1, 1000 for 4.0, 900 for 2.2), following its #include "..." files and #defines, so a
header's site is evaluated as each unit that includes it sees it (`FIXEDTRIG_FLOAT_SINE`, which
a unit defines before its includes), and macros that aren't edition macros (PORTABLE_C,
COMPAT_MODE, _MSC_VER) take the value they have in the comparison build. MASM conditions other
than IFDEF/IFNDEF of a /D symbol (expressions, macro arguments) count as "maybe".

For each site: the file, its lines, each branch's condition and lines, the targets that compile
each branch, the enclosing function (with each target's annotated address) or file scope (or the
struct or initializer it sits in), what each branch defines or uses (includes, macros,
typedefs, functions, globals, annotations; at function scope the globals and functions it uses),
and a content hint for human classification, not a verdict:

  include selection   the branch only includes headers
  annotations only    the branch only carries annotations, or every branch defines the same
                      thing (type and name) and they differ only in initializer or annotation:
                      another target's address, or a .data/.bss duplicate
  float conversion    MechScalar, MechFloat, FIXED_*, float/double, fabs, float literals (or the
                      fixed-point side of a group whose other branch has them); at file scope
                      only MechScalar, FIXED_*, fabs or __ftol, or a group with a fixed-point
                      side (a float type alone, a prototype or a renderer's layout, isn't one)
  declaration         file scope, declarations only
  code                the rest
  empty               nothing at all (an empty #else)

What proves a site: a function-scope site is
proven for target T when T compiles the branch and the enclosing function's state for T
(tools/certify.py) is exact or witness. A function that is unproven, whatever audit.py makes of
it (classified, or listed in tools/known_diffs.txt), proves nothing. A file-scope site isn't
proven by its enclosing function; the tool reports, for the branch T compiles, the share of
proven functions among T's annotated functions that use what the branch declares (by name: in
the same file for a .c, in the files of T's units that include it for a header), or "file scope:
not proven by a function" when none does.

It also lists every file outside the game sources that names an edition macro (tools, workflows,
docker, cmake, vc22, vc40, CMakeLists.txt, tests), output names like MW2_3DFX.DLL left out.

docs/EDITIONS.md's second half is written from the same sites and util/types.h's feature table
(--doc): each feature with its comment, the editions that have it and the files and functions
that test it, the functions with a definition per edition, and the sites that test an edition
macro itself, sorted by what they hold. It carries no line numbers and no proof states, so it
changes only when the editions' source does. --check is what CI fails on: a feature without a
comment above its #define, a feature nothing tests, a tested MW2_ macro the table doesn't define,
a branch no target compiles, and a stale document. Neither needs states.

The proofs need the three simulators' states, which CI only has where it has the retail
originals, without the compilers' trees. So the work splits: --no-states --json writes the sites
with what decides each one's proof (the enclosing function's address, or the functions a
file-scope branch defines and those that use it), and --sites FILE --states ... makes the report
from that file alone.

usage (from the repository root, with reccmp's venv):
  python tools/edition_sites.py [--build build-wine] [--states MW2=f.json --states ...]
                                [--json sites.json] [--markdown sites.md] [--wine PATH]
  python tools/edition_sites.py --doc [FILE]            # rewrite docs/EDITIONS.md's generated part
  python tools/edition_sites.py --check [--no-states --json sites.json]
  python tools/edition_sites.py --sites sites.json --states MW2=f.json ... [--json F] [--markdown F]

The build tree only needs to be configured (and the nested builds' configure steps run). Without
--states for a target, the tool runs tools/certify.py --json for it (minutes: it needs the
built objects and the originals); with them it runs in seconds.
"""

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from unitbuild import EDITION_MACROS, ROOT, Target, edition_features, to_host  # noqa: E402

# 1.1, the reference, then the accelerated editions in the order they were built (their link dates).
SIMULATORS = ("MW2", "MW2ATI", "MW2MATROX", "MW23DFX")
FEATURES = tuple(dict.fromkeys(f for fs in edition_features().values() for f in fs))
EDITION_RE = re.compile(r"\b(%s)\b" % "|".join(EDITION_MACROS + FEATURES))
SCAN_DIRS = ("MW2", "common", "3rdparty/vfx")
C_SUFFIXES = (".c", ".cpp", ".h")
ASM_SUFFIXES = (".asm", ".inc")
PROVEN = ("exact", "witness")
TOOLING = ("tools", ".github/workflows", "docker", "cmake", "vc22", "vc40", "CMakeLists.txt", "tests",
           "reccmp-project.yml", "pyproject.toml")

BLANKED = re.compile(r"//[^\n]*|/\*.*?\*/|\"(?:\\.|[^\"\\\n])*\"|'(?:\\.|[^'\\\n])*'", re.S)
C_DIRECTIVE = re.compile(r"^\s*#\s*(\w+)\s*(.*)$")
ASM_COND = re.compile(r"^\s*(IF|IFE|IFDEF|IFNDEF|IFB|IFNB|IFIDN|IFIDNI|IFDIF|IFDIFI|IF1|IF2|"
                      r"ELSE|ELSEIF|ELSEIFE|ELSEIFDEF|ELSEIFNDEF|ELSEIFB|ELSEIFNB|ELSEIFIDN|ELSEIFIDNI|"
                      r"ELSEIFDIF|ELSEIFDIFI|ENDIF)\b\s*([^;]*)", re.I)
ASM_INCLUDE = re.compile(r"^\s*INCLUDE\s+(\S+)", re.I)
ASM_EQU = re.compile(r"^\s*(\w+)\s*(?:\bEQU\b|\bTEXTEQU\b|=)\s*(.*)$", re.I)
ANNOTATION = re.compile(r"//\s*(FUNCTION|STUB|GLOBAL|LIBRARY|SYNTHETIC|VTABLE|STRING):\s*(\w+)\s+(0x[0-9a-fA-F]+)")
FLOAT_TOKENS = re.compile(r"\bMechScalar\b|\bMechFloat\b|\bMechDouble\b|\bFIXED_\w+|\bfloat\b|\bdouble\b|\bfabs\b|"
                          r"\b__ftol\b|(?<![\w.])\d+\.\d*(?:[eE][-+]?\d+)?[fF]?\b|(?<![\w.])\d+[fF]\b")
FIXED_TOKENS = re.compile(r">>\s*16\b|<<\s*16\b|\b0x10000\b|\b65536\b|\bFixed\w+\s*\(|\bMulDiv64\b")
KEYWORDS = {"if", "while", "for", "switch", "return", "sizeof", "do", "else", "case", "defined", "__asm",
            "_emit", "__declspec", "naked", "int", "char", "short", "long", "unsigned", "signed", "void",
            "float", "double", "struct", "union", "enum", "const", "static", "extern", "volatile"}


def code_only(text):
    """Comments, strings and character literals blanked (newlines kept)."""
    return BLANKED.sub(lambda m: re.sub(r"[^\n]", " ", m.group()), text)


def rel(path):
    try:
        return Path(path).resolve().relative_to(ROOT).as_posix()
    except ValueError:
        return str(path)


# --- The files' conditional structure ---------------------------------------------------------


class Branch:
    def __init__(self, kind, cond, line):
        self.kind, self.cond, self.start, self.end = kind, cond, line, None  # 1-based, directive lines


class Group:
    def __init__(self, path, line, parent):
        self.path, self.start, self.end = path, line, None
        self.branches = []
        self.parent = parent  # (Group, branch index) or None
        self.id = None

    def edition(self):
        return any(EDITION_RE.search(b.cond) for b in self.branches)


class Source:
    """A file's lines, its directives in order and its conditional groups."""

    def __init__(self, path):
        self.path = path
        self.text = path.read_text(encoding="utf-8", errors="replace")
        self.lines = self.text.split("\n")
        self.asm = path.suffix.lower() in ASM_SUFFIXES
        self.directives = []  # (index, kind, argument), 0-based
        self.groups = []
        self.parse()

    def parse(self):
        code = (self.text if self.asm else code_only(self.text)).split("\n")
        stack = []
        i = 0
        while i < len(code):
            line = code[i]
            first = i
            if self.asm:
                if ";" in line:
                    line = line[: line.index(";")]
                m = ASM_COND.match(line)
                if m:
                    kind, arg = m.group(1).upper(), m.group(2).strip()
                else:
                    m = ASM_INCLUDE.match(line)
                    if m:
                        self.directives.append((i, "include", m.group(1)))
                    else:
                        m = ASM_EQU.match(line)
                        if m:
                            self.directives.append((i, "equ", (m.group(1), m.group(2).strip())))
                    i += 1
                    continue
            else:
                m = C_DIRECTIVE.match(line)
                if not m:
                    i += 1
                    continue
                joined = line
                while joined.rstrip().endswith("\\") and i + 1 < len(code):
                    i += 1
                    joined = joined.rstrip()[:-1] + " " + code[i]
                m = C_DIRECTIVE.match(joined)
                kind, arg = m.group(1), m.group(2).strip()
                if kind == "include":
                    raw = re.match(r"\s*#\s*include\s*([<\"][^>\"]+[>\"])", self.lines[first])
                    arg = raw.group(1) if raw else arg
                    self.directives.append((first, "include", arg))
                    i += 1
                    continue
                if kind in ("define", "undef"):
                    self.directives.append((first, kind, arg))
                    i += 1
                    continue
            kind = self.normalize(kind)
            if kind == "if":
                group = Group(self.path, first + 1, stack[-1] if stack else None)
                group.branches.append(Branch(self.original_kind(kind, line), arg, first + 1))
                self.groups.append(group)
                stack.append((group, 0))
            elif kind in ("elif", "else") and stack:
                group, _ = stack[-1]
                group.branches[-1].end = first
                group.branches.append(Branch(self.original_kind(kind, line), arg, first + 1))
                stack[-1] = (group, len(group.branches) - 1)
            elif kind == "endif" and stack:
                group, _ = stack.pop()
                group.branches[-1].end = i
                group.end = i + 1
            self.directives.append((first, kind, (self.original_kind(kind, line), arg)))
            i += 1

    def normalize(self, kind):
        k = kind.lower()
        if k in ("if", "ifdef", "ifndef") or (self.asm and k.startswith("if")):
            return "if"
        if k == "elif" or (self.asm and k.startswith("elseif")):
            return "elif"
        if k in ("else", "endif"):
            return k
        return "other"

    def original_kind(self, kind, line):
        if self.asm:
            return ASM_COND.match(line).group(1).upper()
        return C_DIRECTIVE.match(line).group(1)


# --- Preprocessing each unit ------------------------------------------------------------------


def tokenize(expression):
    return re.findall(r"0[xX][0-9a-fA-F]+[uUlL]*|\d+[uUlL]*|[A-Za-z_]\w*|&&|\|\||==|!=|<=|>=|<<|>>|[!<>()+\-*/%&|^~?:]",
                      expression)


MASM_OPERATORS = {"AND": "&", "OR": "|", "XOR": "^", "NOT": "~", "EQ": "==", "NE": "!=", "LT": "<", "GT": ">",
                  "LE": "<=", "GE": ">=", "SHL": "<<", "SHR": ">>", "MOD": "%"}


def evaluate(expression, macros, masm=False):
    """An #if (or MASM IF) expression: True, False, or None when it can't be evaluated."""
    value = evaluate_int(expression, macros, masm)
    return None if value is None else bool(value)


def evaluate_int(expression, macros, masm=False, depth=0):
    if masm:
        expression = re.sub(r"\b(%s)\b" % "|".join(MASM_OPERATORS), lambda m: " %s " % MASM_OPERATORS[m.group(1).upper()],
                            expression, flags=re.I)
        expression = re.sub(r"\b([0-9][0-9a-fA-F]*)[hH]\b", lambda m: str(int(m.group(1), 16)), expression)
    tokens = tokenize(expression)
    out = []
    i = 0
    while i < len(tokens):
        t = tokens[i]
        if t == "defined":
            if i + 1 < len(tokens) and tokens[i + 1] == "(":
                name, i = tokens[i + 2], i + 4
            else:
                name, i = tokens[i + 1], i + 2
            out.append("1" if name in macros else "0")
            continue
        if re.match(r"[A-Za-z_]", t):
            value = macros.get(t)
            if masm and value is None:
                value = next((v for k, v in macros.items() if k.lower() == t.lower()), None)
                if value is None:
                    return None  # a macro argument or a label: unknown
            if value is None or depth > 8 or not value.strip():
                out.append("0")
            else:
                v = evaluate_int(value, macros, masm, depth + 1)
                if v is None:
                    return None
                out.append("(%d)" % v)
        elif re.match(r"\d", t):
            out.append(str(int(re.sub(r"[uUlL]+$", "", t), 0)))
        else:
            out.append({"&&": " and ", "||": " or ", "!": " not "}.get(t, t))
        i += 1
    try:
        return int(eval("".join(out) or "0", {"__builtins__": {}}))  # noqa: S307 (integers and operators only)
    except Exception:
        return None


def and3(a, b):
    if a is False or b is False:
        return False
    if a is None or b is None:
        return None
    return True


def or3(a, b):
    if a is True or b is True:
        return True
    if a is None or b is None:
        return None
    return False


def predefined(unit):
    """The compiler's own macros for a unit, and its /D options."""
    macros = {}
    args = unit.arguments
    if unit.language != "ASM_MASM":
        compiler = unit.compiler.lower()
        version = "900" if "msvc22" in compiler else "1000" if "msvc40" in compiler else "1010"
        macros.update(_MSC_VER=version, _WIN32="1", _M_IX86="500" if re.search(r"[/-]G5\b", args) else "400")
        if unit.language == "CXX":
            macros["__cplusplus"] = "1"
        if re.search(r"[/-]M[TD]d?\b", args):
            macros["_MT"] = "1"
        if re.search(r"[/-]M[TLD]d\b", args):
            macros["_DEBUG"] = "1"
    for m in re.finditer(r"(?:^|\s)[/-]D\s*(\w+)(?:=(\S*))?", args):
        macros[m.group(1)] = m.group(2) if m.group(2) is not None else "1"
    return macros


def include_dirs(unit):
    dirs = []
    root = to_host(unit.root).replace("\\", "/").rstrip("/")
    for m in re.finditer(r"(?:^|\s)[/-]I\s*(\"[^\"]+\"|\S+)", unit.arguments):
        path = to_host(m.group(1).strip('"')).replace("\\", "/")
        if path.startswith(root):
            path = str(ROOT) + path[len(root) :]
        dirs.append(Path(path))
    return dirs


class Reach:
    """What each target compiles: per (file, group id, branch index) the set of units that reach the
    branch active (True) or maybe (None), and per file the lines that are (maybe) active."""

    def __init__(self):
        self.branches = defaultdict(lambda: defaultdict(dict))  # target -> (path, gid, b) -> unit -> value
        self.lines = defaultdict(dict)  # target -> path -> bytearray (1 active)
        self.units = defaultdict(lambda: defaultdict(set))  # target -> path -> units that include it
        self.groups = defaultdict(lambda: defaultdict(set))  # target -> (path, gid) -> units that reach the #if


def preprocess(target, unit, sources, reach):
    macros = predefined(unit)
    dirs = include_dirs(unit)
    asm = unit.language == "ASM_MASM"

    def load(path):
        path = path.resolve()
        if path not in sources:
            if not path.exists():
                return None
            sources[path] = Source(path)
        return sources[path]

    def resolve(name, here):
        quoted = name.startswith('"')
        name = name.strip('<>"')
        candidates = ([here.parent] if quoted or asm else []) + dirs + ([] if quoted or asm else [here.parent])
        for d in candidates:
            p = d / name
            if p.exists():
                return p
            if asm:  # MASM: case-insensitive file systems
                for q in d.glob("*") if d.exists() else ():
                    if q.name.lower() == name.lower():
                        return q
        return None

    def walk(path, active, depth):
        src = load(path)
        if src is None or depth > 40:
            return
        key = src.path
        reach.units[target][key].add(unit.source)
        lines = reach.lines[target].setdefault(key, bytearray(len(src.lines)))
        stack = []  # (group index in src.groups, branch active, taken, parent active)
        group_iter = iter(range(len(src.groups)))
        cur = 0
        state = active
        for index, kind, arg in src.directives:
            if state is not False and index > cur:
                lines[cur:index] = b"\x01" * (index - cur)
            cur = index + 1
            if kind == "if":
                gid = next(group_iter)
                okind, cond = arg
                if state is not False:
                    reach.groups[target][(src.path, gid)].add(unit.source)
                value = cond_value(okind, cond, src.asm) if state is not False else False
                stack.append([gid, value, value, state, 0])
                state = and3(state, value)
                record(src, gid, 0, state)
            elif kind in ("elif", "else") and stack:
                top = stack[-1]
                gid, _, taken, parent, b = top
                okind, cond = arg
                b += 1
                if kind == "else":
                    value = True
                else:
                    value = cond_value(okind, cond, src.asm) if parent is not False else False
                if taken is True:
                    branch = False
                elif taken is False:
                    branch = value
                else:
                    branch = None if value is not False else False
                top[1:] = [branch, or3(taken, value), parent, b]
                state = and3(parent, branch)
                record(src, gid, b, state)
            elif kind == "endif" and stack:
                state = stack.pop()[3]
            elif state is False:
                continue
            elif kind == "include":
                found = resolve(arg, src.path)
                if found is not None:
                    walk(found, state, depth + 1)
            elif kind == "define":
                m = re.match(r"(\w+)(\([^)]*\))?\s*(.*)", arg)
                if m and state is True:
                    macros[m.group(1)] = "1" if m.group(2) else (m.group(3).strip() or "")
            elif kind == "undef":
                if state is True:
                    macros.pop(arg.split()[0] if arg else "", None)
            elif kind == "equ":
                if state is True:
                    name, value = arg
                    macros[name] = value if re.fullmatch(r"[-+~()\w\s]*", value) else "1"
        if state is not False and cur < len(src.lines):
            lines[cur:] = b"\x01" * (len(src.lines) - cur)

    def cond_value(kind, cond, is_asm):
        if not is_asm:
            if kind in ("ifdef", "ifndef"):
                name = cond.split()[0] if cond else ""
                return (name in macros) != (kind == "ifndef")
            return evaluate(cond, macros)
        k = kind.upper()
        if k in ("IFDEF", "ELSEIFDEF", "IFNDEF", "ELSEIFNDEF"):
            name = cond.split()[0] if cond else ""
            defined = any(n.lower() == name.lower() for n in macros)
            return defined != ("NDEF" in k)
        if k == "ELSE":
            return True
        if k in ("IF", "ELSEIF", "IFE", "ELSEIFE"):
            value = evaluate(cond, macros, masm=True)
            if value is not None and k.endswith("IFE"):
                value = not value
            return value
        return None  # IFB, IFIDN and the rest test macro arguments: maybe

    def record(src, gid, b, value):
        if value is not False:
            reach.branches[target][(src.path, gid, b)][unit.source] = value

    walk(ROOT / unit.source, True, 0)


# --- Functions and states ---------------------------------------------------------------------


def load_annotations(targets):
    """Per target: [(path, first, last, name, address)] of its annotated functions, and the
    addresses of the functions annotated by name in headers (the MASM routines: name -> address)."""
    from funccmp import Annotations, source_roots  # needs reccmp

    functions, globals_ = {}, {}
    for target in targets:
        a = Annotations(target, source_roots(target))
        rows = []
        for name, markers in a.functions.items():
            for f in markers:
                if f.end_line is None:
                    continue
                rows.append((Path(f.filename).resolve(), f.line_number, f.end_line, name, f.offset))
        functions[target] = rows
        globals_[target] = {name: markers[0].offset for name, markers in a.by_name.items() if len(markers) == 1}
    return functions, globals_


def load_states(target, given, build, wine):
    path = given.get(target)
    if path is None:
        tmp = Path(tempfile.mkdtemp()) / ("%s.json" % target)
        print("running certify for %s ..." % target, file=sys.stderr)
        subprocess.run([sys.executable, str(ROOT / "tools/certify.py"), "--target", target, "--build", build, "--wine", wine,
                        "--json", str(tmp)], check=True, cwd=ROOT, stdout=subprocess.DEVNULL)
        path = tmp
    data = json.loads(Path(path).read_text())
    return {int(f["address"], 16): f["state"] for f in data["functions"]}, str(path)


# --- Branch contents --------------------------------------------------------------------------


def statements(code):
    """The top-level statements of a block of C code: (text, has a body)."""
    out, depth, start, body = [], 0, 0, False
    for i, c in enumerate(code):
        if c == "{":
            if depth == 0:
                body = True
            depth += 1
        elif c == "}":
            depth = max(depth - 1, 0)
            if depth == 0:
                rest = code[i + 1 :].lstrip()
                head = code[start:i]
                # a function body ends the statement; a struct or initializer goes on to its ';'
                if re.search(r"\)\s*(?:const\s*)?\{", head) and not re.search(r"=\s*\{", head) and not rest.startswith(";"):
                    out.append((code[start : i + 1], True))
                    start, body = i + 1, False
        elif c == ";" and depth == 0:
            out.append((code[start : i + 1], body))
            start, body = i + 1, False
    if code[start:].strip():
        out.append((code[start:], body))
    return out


def declared_names(code):
    """What file-scope code defines: (functions, prototypes, globals, typedefs, types)."""
    functions, prototypes, globals_, typedefs, types = [], [], [], [], []
    for text, body in statements(code):
        t = " ".join(text.split())
        if not t:
            continue
        if t.startswith("typedef"):
            m = re.search(r"\(\s*(?:__\w+\s+)?\*\s*(\w+)\s*\)\s*\(", t)
            name = m.group(1) if m else (re.findall(r"(\w+)\s*(?:\[[^\]]*\])*\s*;?\s*$", t) or [None])[0]
            if name:
                typedefs.append(name)
            continue
        m = re.match(r"(?:struct|union|enum)\s+(\w+)\s*\{", t)
        if m:
            types.append(m.group(1))
            continue
        m = re.match(r"DECOMP_SIZE_ASSERT\s*\(\s*(\w+)", t)
        if m:
            types.append(m.group(1))  # the size assertion of a type
            continue
        head = re.split(r"=|\{", t, maxsplit=1)[0]
        call = re.search(r"\b(\w+)\s*\(([^()]*(\([^()]*\))*[^()]*)\)\s*(?:;|\{|$)", head + ("{" if body else ""))
        if body and re.search(r"\)\s*\{", t.split("=", 1)[0] if "=" not in t.split("{", 1)[0] else "") and call:
            functions.append(call.group(1))
            continue
        if re.search(r"\(\s*(?:__\w+\s+)?\*\s*\w+\s*\)", head):  # a function pointer variable
            m = re.search(r"\(\s*(?:__\w+\s+)?\*\s*(\w+)\s*\)", head)
            globals_.append(m.group(1))
            continue
        if call and not re.search(r"=", t.split("(", 1)[0]):
            prototypes.append(call.group(1))
            continue
        for part in re.split(r",(?![^(\[{]*[)\]}])", head.rstrip(";")):
            m = re.search(r"\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\]\s*)*$", part.strip())
            if m and m.group(1) not in KEYWORDS:
                globals_.append(m.group(1))
    return functions, prototypes, globals_, typedefs, types


def normalized_definitions(code):
    """Each file-scope statement without initializer and whitespace (the 'same definition' test)."""
    out = []
    for text, body in statements(code):
        t = " ".join(text.split())
        if not t:
            continue
        t = re.split(r"\s*=\s*", t, maxsplit=1)[0].rstrip(";").strip() if not body or "=" in t.split("{", 1)[0] else t
        out.append(t)
    return sorted(out)


def branch_lines(src, group, b):
    branch = group.branches[b]
    return branch.start + 1, branch.end  # 1-based, without the directives


def branch_text(src, group, b, code=False):
    first, last = branch_lines(src, group, b)
    lines = src.lines[first - 1 : last]
    if src.asm:
        return "\n".join(line.split(";", 1)[0] for line in lines) if code else "\n".join(lines)
    if code:
        text = code_only("\n".join(lines))
        return "\n".join(line for line in text.split("\n") if not line.lstrip().startswith("#"))
    return "\n".join(lines)


def contents(src, group, b, scope):
    raw = branch_text(src, group, b)
    code = branch_text(src, group, b, code=True)
    info = {
        "includes": re.findall(r"^\s*#\s*include\s*[<\"]([^>\"]+)", raw, re.M) if not src.asm
        else re.findall(r"^\s*INCLUDE\s+(\S+)", raw, re.M | re.I),
        "macros": re.findall(r"^\s*#\s*define\s+(\w+)", raw, re.M),
        "pragmas": re.findall(r"^\s*#\s*pragma\s+(.*?)\s*$", raw, re.M) if not src.asm else [],
        "annotations": ["%s: %s %s" % m for m in ANNOTATION.findall(raw)],
    }
    if src.asm:
        info["procs"] = re.findall(r"^\s*(\w+)\s+PROC\b", code, re.M | re.I)
        info["data"] = re.findall(r"^\s*(\w+)\s+(?:DB|DW|DD|DQ|BYTE|WORD|DWORD|SBYTE|SWORD|SDWORD)\b", code, re.M | re.I)
        info["uses"] = sorted(set(re.findall(r"\b(?:call|jmp)\s+(\w+)", code, re.I)))
    elif scope == "aggregate":
        info["members"] = sorted(set(re.findall(r"\b(m_\w+)\s*(?:\[[^\]]*\]\s*)*[;:]", code)))
        info["globals_used"] = sorted(set(re.findall(r"\bg_\w+", code)))
    elif scope == "function":
        info["globals_used"] = sorted(set(re.findall(r"\bg_\w+", code)))
        info["calls"] = sorted({m for m in re.findall(r"\b([A-Za-z_]\w*)\s*\(", code) if m not in KEYWORDS and not m.startswith("Mech")})
    else:
        functions, prototypes, globals_, typedefs, types = declared_names(code)
        info.update(functions=functions, prototypes=prototypes, globals=globals_, typedefs=typedefs, types=types)
    defines = "\n".join(re.findall(r"^\s*#\s*define\s+(.*)$", code_only(raw), re.M)) if not src.asm else ""
    info["float_tokens"] = sorted(set(m.group().strip() for m in FLOAT_TOKENS.finditer(code + "\n" + defines)))[:12] \
        if not src.asm else []
    info["fixed_tokens"] = bool(FIXED_TOKENS.search(code)) if not src.asm else False
    info["has_code"] = bool(code.strip())
    info["only_includes"] = bool(info["includes"]) and not code.strip() and not info["macros"] if not src.asm else False
    info["definitions"] = normalized_definitions(code) if scope != "function" and not src.asm else None
    return info


def hints(src, group, infos, scope):
    """A primary hint and tags per branch."""
    out = []
    group_float = any(i["float_tokens"] for i in infos)
    group_fixed = any(i["fixed_tokens"] for i in infos)
    strong = lambda i: any(t.startswith("FIXED_") or t in ("MechScalar", "fabs", "__ftol") for t in i["float_tokens"])
    layout = (scope != "function" and len(infos) > 1 and all(i["definitions"] for i in infos)
              and all(i["definitions"] == infos[0]["definitions"] for i in infos))
    for info in infos:
        tags = []
        if info["includes"]:
            tags.append("include")
        if info["annotations"]:
            tags.append("annotations")
        if scope == "function":
            float_like = info["float_tokens"] or (group_float and info["fixed_tokens"])
        else:
            # At file scope a float type alone (a prototype's double, a renderer's vertex layout)
            # is no conversion: it takes MechScalar/FIXED_*/fabs, or a fixed-point counterpart.
            float_like = strong(info) or (group_fixed and (info["float_tokens"] or info["fixed_tokens"]))
        if float_like:
            tags.append("float")
        if info["only_includes"]:
            primary = "include selection"
        elif not info["has_code"]:
            if info["macros"]:
                primary = "float conversion" if "float" in tags else "macro definition"
            elif info["pragmas"]:
                primary = "pragma"
            else:
                primary = "annotations only" if info["annotations"] else "empty"
        elif layout:
            primary = "annotations only"
        elif "float" in tags:
            primary = "float conversion"
        elif src.asm:
            primary = "code"
        elif scope != "function" and not info.get("functions"):
            primary = "declaration"
        else:
            primary = "code"
        out.append((primary, tags))
    return out


# --- Scope ------------------------------------------------------------------------------------


def brace_scope(src, line, active):
    """For a site at `line` (1-based) outside any annotated function: ("function", name),
    ("aggregate", name) or ("file", None), from the braces of the lines `active` keeps."""
    code = code_only(src.text).split("\n")
    text = "\n".join(c if (active is None or i >= len(active) or active[i]) and not c.lstrip().startswith("#") else ""
                     for i, c in enumerate(code[: line - 1]))
    depth, opener = 0, None
    for i, c in enumerate(text):
        if c == "{":
            if depth == 0:
                opener = i
            depth += 1
        elif c == "}":
            depth = max(depth - 1, 0)
    if depth == 0:
        return "file", None
    head = text[:opener].rstrip()
    stmt = re.split(r"[;}]", head)[-1]
    if head.endswith(")") and "=" not in stmt:
        from funccmp import function_name

        return "function", function_name(" ".join(stmt.split()))
    m = re.search(r"(\w+)\s*(?:\[[^\]]*\]\s*)*=?\s*$", stmt) or re.search(r"(?:struct|union|enum)\s+(\w+)", stmt)
    return "aggregate", m.group(1) if m else None


def asm_blocks(src):
    """A MASM file's procedures and macros: [(kind, name, first, last)], 1-based lines."""
    out, open_ = [], {}
    for i, line in enumerate(src.lines, 1):
        code = line.split(";", 1)[0]
        if re.match(r"\s*ENDP\b", code, re.I):  # an unnamed ENDP closes the open procedure
            procs = [k for k, v in open_.items() if v[0] == "proc"]
            if procs:
                kind, n, first = open_.pop(procs[-1])
                out.append((kind, n, first, i))
            continue
        m = re.match(r"\s*(\w+)\s+(PROC|MACRO|ENDP)\b", code, re.I)
        if m:
            name, word = m.group(1), m.group(2).upper()
            if word in ("PROC", "MACRO"):
                open_[name.lower()] = (word.lower(), name, i)
            elif name.lower() in open_:
                kind, n, first = open_.pop(name.lower())
                out.append((kind, n, first, i))
            continue
        if re.match(r"\s*ENDM\b", code, re.I):
            macros = [k for k, v in open_.items() if v[0] == "macro"]
            if macros:
                kind, n, first = open_.pop(macros[-1])
                out.append((kind, n, first, i))
    return out


# --- Tooling references -----------------------------------------------------------------------


def tooling_references():
    out = []
    files = []
    for entry in TOOLING:
        p = ROOT / entry
        if p.is_file():
            files.append(p)
        elif p.is_dir():
            files += [q for q in sorted(p.rglob("*")) if q.is_file() and q.suffix.lower() in
                      (".py", ".yml", ".yaml", ".cmake", ".txt", ".sh", ".ps1", ".bat", "", ".c", ".h", ".def", ".in")
                      and "witnesses" not in q.parts and q.name != Path(__file__).name]
    for path in files:
        try:
            lines = path.read_text(encoding="utf-8", errors="replace").split("\n")
        except OSError:
            continue
        for n, line in enumerate(lines, 1):
            names = list(re.finditer(r"(?:(?<![\w])|(?<=[/-]D))MW2_(?:ACCEL|MATROX|3DFX)\b", line))
            if not names:
                continue

            def file_name(m):
                return bool(re.match(r"\.(dll|pdb|map|lib|exp|ilk)\b", line[m.end() :], re.I)
                            or re.search(r"(OUTPUT_NAME\s+|[/\\]|\"vc\d\d\",\s*\")$", line[: m.start()]))

            macros = sorted({m.group() for m in names if not file_name(m)})
            if not macros:
                continue  # MW2_3DFX.DLL, OUTPUT_NAME MW2_MATROX, o\MW2_MATROX\\
            kind = "comment" if re.match(r"\s*(#|//|;)", line) else "code"
            out.append({"file": rel(path), "line": n, "macros": macros, "kind": kind, "text": line.strip()[:160]})
    return out


# --- Main -------------------------------------------------------------------------------------


def scan_files():
    files = []
    for d in SCAN_DIRS:
        for p in sorted((ROOT / d).rglob("*")):
            if p.is_file() and p.suffix.lower() in C_SUFFIXES + ASM_SUFFIXES:
                files.append(p.resolve())
    for p in sorted((ROOT / "MW2/src/accel").rglob("*")):
        if p.suffix.lower() in ASM_SUFFIXES and p.resolve() not in files:
            files.append(p.resolve())
    return files


SITE_HINTS = ("float conversion", "code", "declaration", "macro definition", "include selection", "pragma", "annotations only")
TOOLCHAIN_RE = re.compile(r"\b(PORTABLE_C|PORTABLE_C_LABELS|COMPAT_MODE|REFERENCE_ASM|_MSC_VER)\b")


def condition_of(src, group, b):
    branch = group.branches[b]
    k = branch.kind.lower()
    if k in ("ifdef", "elseifdef"):
        return "defined(%s)" % branch.cond.split()[0]
    if k in ("ifndef", "elseifndef"):
        return "!defined(%s)" % branch.cond.split()[0]
    if k == "else":
        return "else"
    return branch.cond


def build_sites(sources, files, reach, functions, by_name, targets):
    sites = []
    by_group = {}
    fn_index = defaultdict(list)
    for t in targets:
        for path, first, last, name, address in functions[t]:
            fn_index[path].append((first, last, name, t, address))
    for path in files:
        src = sources.get(path) or Source(path)
        sources[path] = src
        for gid, group in enumerate(src.groups):
            if not group.edition():
                continue
            group.id = len(sites)
            by_group[(path, gid)] = group.id
            parent = None
            p = group.parent
            while p is not None:
                pg, pb = p
                if pg.edition():
                    parent = {"site": pg.id, "branch": pb}
                    break
                p = pg.parent
            context = []
            p = group.parent
            while p is not None:
                pg, pb = p
                if not pg.edition():
                    context.append(condition_of(src, pg, pb))
                p = pg.parent
            # scope
            enclosing = [(f, l, n, t, a) for f, l, n, t, a in fn_index[path] if f <= group.start <= l]
            if enclosing:
                names = Counter(n for _, _, n, _, _ in enclosing)
                fname = names.most_common(1)[0][0]
                scope = "function"
                addresses = {t: "0x%08x" % a for _, _, n, t, a in enclosing if n == fname}
                ranges = {t: [f, l] for f, l, n, t, _ in enclosing if n == fname}
            else:
                view = None
                for t in targets:
                    if any(reach.branches[t].get((path, gid, b)) for b in range(len(group.branches))):
                        view = reach.lines[t].get(path)
                        break
                addresses, ranges = {}, {}
                if src.asm:
                    scope, fname = "file", None
                    inside = [b for b in asm_blocks(src) if b[2] < group.start < b[3]]
                    if inside:
                        kind, fname, first, last = min(inside, key=lambda b: b[3] - b[2])
                        scope = "function" if kind == "proc" else "macro"
                        if kind == "proc":
                            addresses = {t: "0x%08x" % by_name[t][fname] for t in targets if fname in by_name[t]}
                            ranges = {t: [first, last] for t in addresses}
                else:
                    scope, fname = brace_scope(src, group.start, view)
            container = None
            if scope in ("aggregate", "macro"):
                container, fname = fname, None  # the struct, initialized global or MASM macro around it
            branches = []
            compiled = {}
            infos = []
            for b, branch in enumerate(group.branches):
                info = contents(src, group, b, scope if scope in ("function", "aggregate") else "file")
                infos.append(info)
                per = {}
                for t in targets:
                    units = reach.branches[t].get((path, gid, b), {})
                    if any(v is True for v in units.values()):
                        per[t] = "yes"
                    elif units:
                        per[t] = "maybe"
                    else:
                        per[t] = "no"
                compiled[b] = per
                first, last = branch_lines(src, group, b)
                branches.append({"index": b, "directive": branch.kind, "condition": condition_of(src, group, b),
                                 "lines": [branch.start, branch.end], "body": [first, last],
                                 "targets": per, "units": {t: len(reach.branches[t].get((path, gid, b), {})) for t in targets},
                                 "contents": {k: v for k, v in info.items() if k not in ("has_code", "only_includes", "definitions")}})
            for br, (primary, tags) in zip(branches, hints(src, group, infos, scope)):
                br["hint"], br["tags"] = primary, tags
                br["edition"] = sorted(set(EDITION_RE.findall(br["condition"])))
                if all(v == "no" for v in br["targets"].values()):
                    # No simulator's comparison build compiles it: the portable C (COMPAT_MODE
                    # builds), or text no build reaches (a test its enclosing branch decided)
                    br["unreached"] = "portable C" if TOOLCHAIN_RE.search(br["condition"]) else "dead"
            hint_set = {br["hint"] for br in branches} - {"empty"}
            site_hint = next((h for h in SITE_HINTS if h in hint_set), "empty")
            macros = sorted({m for br in group.branches for m in EDITION_RE.findall(br.cond)})
            site = {
                "id": group.id, "file": rel(path), "lines": [group.start, group.end],
                "condition": condition_of(src, group, 0), "macros": macros,
                "kind": "asm" if src.asm else ("header" if path.suffix.lower() == ".h" else "source"),
                "hint": site_hint, "parent": parent, "context": context, "scope": scope, "function": fname,
                "container": container,
                "annotations": addresses, "function_lines": ranges, "branches": branches,
                "built_by": {t: len(reach.units[t].get(path, ())) for t in targets},
                "reached_by": {t: len(reach.groups[t].get((path, gid), ())) for t in targets},
            }
            if any(site["built_by"].values()) and not any(site["reached_by"].values()):
                # No comparison build reaches the group: the portable C's, if a toolchain test
                # encloses it, or text no build compiles (a test its enclosing branch decided).
                enclosing = context + ([condition_of(src, group.parent[0], group.parent[1])] if group.parent else [])
                site["unreached"] = "portable C" if any(TOOLCHAIN_RE.search(c) for c in enclosing) else "dead"
                for br in branches:
                    br["unreached"] = site["unreached"]
            site["dead_branches"] = [br["index"] for br in branches if br.get("unreached") == "dead"]
            sites.append(site)
    return sites


def unit_files(path, target, reach, suffixes):
    """The files of the target's units that reach `path` (the units' sources and the fragments
    they include), restricted to `suffixes`."""
    units = reach.units[target].get(path, set())
    files = {(ROOT / u).resolve() for u in units}
    files |= {f for f, us in reach.units[target].items() if us & units}
    return {f for f in files if f.suffix.lower() in suffixes}


def file_scope_plan(site, compiled, target, reach, functions, by_name, sources, body_tokens):
    """For a file-scope site: the target's annotated functions the branches it compiles define, and
    those that use what the branches declare, by name: anywhere in the target for a .c (its
    definitions reach the other units through headers), in the units that include it for a header
    or a MASM include. Each as [name, address]: their states decide the proof (apply_states)."""
    path = (ROOT / site["file"]).resolve()
    names, inside = set(), []
    for b in compiled:
        br = site["branches"][b]
        c = br["contents"]
        for key in ("functions", "prototypes", "globals", "typedefs", "types", "macros", "procs", "data"):
            names.update(c.get(key) or ())
        inside.append(br["body"])
    if site["scope"] in ("macro", "aggregate") and site["container"]:
        names.add(site["container"])  # a MASM macro, or the struct or initialized global the site sits in
    names = {n for n in names if len(n) > 2 and n not in KEYWORDS}
    defined, users = [], []
    if site["kind"] == "asm":
        rows = []
        for f in unit_files(path, target, reach, ASM_SUFFIXES):
            src = sources.get(f) or Source(f)
            for kind, name, first, last in asm_blocks(src):
                if kind == "proc" and name in by_name[target]:
                    rows.append((f, first, last, name, by_name[target][name]))
    elif path.suffix.lower() == ".h":
        files = unit_files(path, target, reach, (".c", ".cpp"))
        rows = [r for r in functions[target] if r[0] in files]
    else:
        rows = functions[target]  # a unit's globals and functions reach the others through headers
    for f, first, last, name, address in rows:
        if f == path and any(lo <= first and last <= hi for lo, hi in inside):
            defined.append([name, address])
        elif names and body_tokens(target, f, first, last) & names:
            users.append([name, address])
    out = {}
    if defined:
        out["defined"] = defined
    if users:
        out["users"] = users
    if out:
        out["names"] = sorted(names)[:20]
    return out


def plan_proofs(sites, targets, reach, functions, by_name, sources):
    """Each site's proof plan per target: what decides it, before any state is known. Final for a
    target that doesn't build or reach the group; otherwise the enclosing function's address, or
    for a file-scope site the functions its compiled branch defines and those that use it."""
    token_cache = {}

    def body_tokens(target, path, first, last):
        key = (target, path, first, last)
        if key not in token_cache:
            src = sources.get(path) or Source(path)
            sources[path] = src
            active = reach.lines[target].get(path)
            body = src.lines[first - 1 : last]
            code = [line.split(";", 1)[0] for line in body] if src.asm else code_only("\n".join(body)).split("\n")
            kept = [c for i, c in enumerate(code) if active is None or first - 1 + i >= len(active) or active[first - 1 + i]]
            token_cache[key] = set(re.findall(r"[A-Za-z_]\w*", "\n".join(kept)))
        return token_cache[key]

    for site in sites:
        plan = {}
        for t in targets:
            compiled = [b for b, br in enumerate(site["branches"]) if br["targets"][t] != "no"]
            if not site["built_by"][t]:
                plan[t] = {"state": "not built"}
            elif site.get("unreached"):
                plan[t] = {"state": site["unreached"]}  # no comparison build reaches the group
            elif not site["reached_by"][t]:
                plan[t] = {"state": "not reached"}  # an enclosing conditional leaves it out
            elif site["scope"] == "function":
                # A target that reaches the group but none of its branches compiles the implicit
                # empty #else: the enclosing function proves that absence like any branch.
                address = site["annotations"].get(t)
                plan[t] = {"state": "function" if address else "no function", "branches": compiled}
                if address:
                    plan[t]["address"] = address
            else:
                found = file_scope_plan(site, compiled, t, reach, functions, by_name, sources, body_tokens)
                plan[t] = {"state": "file scope", "branches": compiled, **found}
        site["plan"] = plan


def apply_states(sites, targets, states):
    """Each site's proof per target, from its plan and the functions' certify states."""
    for site in sites:
        proof = {}
        for t in targets:
            plan = site["plan"][t]
            if plan["state"] == "function":
                st = states[t].get(int(plan["address"], 16), "no state")
                proof[t] = {"state": "proven" if st in PROVEN else "not proven", "function_state": st, "branches": plan["branches"]}
            elif plan["state"] == "file scope":
                p = {"state": "file scope", "branches": plan["branches"]}
                for key in ("defined", "users"):
                    if key in plan:
                        found = [(n, states[t].get(a)) for n, a in plan[key]]
                        p[key] = {"functions": len(found), "proven": sum(s in PROVEN for _, s in found),
                                  "unproven": sorted(n for n, s in found if s not in PROVEN)[:20]}
                if "names" in plan:
                    p["names"] = plan["names"]
                else:
                    p["note"] = "file scope: not proven by a function"
                proof[t] = p
            else:
                proof[t] = dict(plan)
        site["proof"] = proof


# --- The feature table and the document -------------------------------------------------------

DOC = "docs/EDITIONS.md"
DOC_BEGIN = "<!-- generated by tools/edition_sites.py --doc: begin -->"
DOC_END = "<!-- generated: end -->"
EDITION_NAMES = {"MW2": "1.1", "MW2MATROX": "Matrox", "MW23DFX": "3Dfx", "MW2ATI": "ATI"}
KIND_PREFIXES = {"Artefact:": "artefact", "Representation:": "representation"}


def feature_table(root=ROOT):
    """util/types.h's table, in its order: {feature: {"kind", "line", "blocks": {identity macro:
    {"value", "doc"}}}}. A feature's documentation is the comment right above its #define, inside
    the #ifdef of the identity macro that has it; a comment that starts "Artefact:" or
    "Representation:" says the feature is no difference of the game's behaviour."""
    table, stack, doc, open_comment = {}, [], [], False
    for n, line in enumerate((Path(root) / "util" / "types.h").read_text(encoding="utf-8").splitlines(), 1):
        text = line.strip()
        if open_comment or text.startswith("/*"):
            if not open_comment:
                doc = []
            open_comment = "*/" not in text
            doc.append(re.sub(r"^/\*|\*/$", "", text).strip())
            continue
        m = re.match(r"#\s*(\w+)\s*(\w*)\s*(.*)", text)
        if m and m.group(1) == "define" and stack and stack[-1]:
            words = " ".join(w for w in doc if w)
            kind = "feature"
            for prefix, k in KIND_PREFIXES.items():
                if words.startswith(prefix):
                    kind, words = k, words[len(prefix) :].strip()
            words = words[:1].upper() + words[1:]
            row = table.setdefault(m.group(2), {"kind": kind, "line": n, "blocks": {}})
            row["blocks"][stack[-1]] = {"value": m.group(3).strip(), "doc": words}
        elif m and m.group(1) in ("if", "ifdef", "ifndef"):
            stack.append(m.group(2) if m.group(1) == "ifdef" and m.group(2) in EDITION_MACROS else None)
        elif m and m.group(1) == "endif" and stack:
            stack.pop()
        doc = []
    return table


def tested_macros(sources, files):
    """Every MW2_ macro a conditional of the scanned sources tests: {macro: "file:line"} (the first)."""
    out = {}
    for path in files:
        src = sources.get(path)
        if src is None:
            continue
        for group in src.groups:
            for branch in group.branches:
                for name in re.findall(r"\bMW2_\w+", branch.cond.split(";")[0] if src.asm else code_only(branch.cond)):
                    out.setdefault(name, "%s:%d" % (rel(path), branch.start))
    return out


def has(row, identity):
    """The block of the table that gives a target (by its identity macros) the feature, or None."""
    return next((b for m, b in row["blocks"].items() if m in identity), None)


def site_label(x):
    if x["scope"] == "function" and x["function"]:
        return "`%s`" % x["function"]
    if x["container"]:
        return "`%s`" % x["container"]
    defined = sorted({f for br in x["branches"] for f in br["contents"].get("functions") or ()}, key=str.lower)
    if defined:  # a file-scope group that holds whole functions
        return "the definition%s of %s" % ("" if len(defined) == 1 else "s", ", ".join("`%s`" % f for f in defined))
    return "file scope"


def where(sites):
    """Sites as text without line numbers: each file with its functions, a count where one has several."""
    by_file = defaultdict(Counter)
    for x in sites:
        by_file[x["file"]][site_label(x)] += 1
    parts = []
    for f in sorted(by_file):
        labels = sorted(by_file[f].items(), key=lambda kv: (not kv[0].startswith("`"), kv[0].lower()))
        parts.append("`%s`: %s" % (f, ", ".join(k + (" (%d)" % v if v > 1 else "") for k, v in labels)))
    return "; ".join(parts)


IDENTITY_KINDS = (
    ("code", "In a function: the edition's back end, what the shared code does for it, or the same computation written another way"),
    ("definitions", "An edition's own definition of a function, or functions only some editions' objects have"),
    ("fragment", "A fragment included at the position of the object that holds it"),
    ("layout", "The same definition at another address or in another section"),
    ("data", "Data and declarations that differ"),
    ("include", "Includes"),
    ("pragma", "Pragmas that reproduce a unit's flags"),
)


def identity_kind(x):
    """What a site that tests an edition's identity is, as far as its content shows."""
    branches = x["branches"]
    includes = [i for br in branches for i in br["contents"].get("includes") or ()]
    if x["hint"] == "include selection":
        return "fragment" if any(i.lower().endswith(".c") for i in includes) else "include"
    if x["hint"] == "pragma":
        return "pragma"
    if x["hint"] == "annotations only":
        return "layout"
    if x["scope"] == "function":
        return "code"
    if any(br["contents"].get("functions") for br in branches):
        return "definitions"
    return "data"


def document(sites, table, identity, targets):
    """The generated part of docs/EDITIONS.md: no line numbers and no proof states, so that it
    changes only when a feature, its text or the place of a site does."""
    names = [EDITION_NAMES[t] for t in targets]
    using = defaultdict(list)
    for x in sites:
        for m in x["macros"]:
            using[m].append(x)
    out = [DOC_BEGIN, "", "## Features", "",
           "From the table in `util/types.h` (%d features) and the %d conditional sites of the simulators' sources "
           "(`MW2/`, `common/`, `3rdparty/vfx`). A feature's cell is its value where it has one." % (len(table), len(sites)), ""]
    blocks = []
    for name, row in table.items():
        key = tuple(row["blocks"])
        if key not in blocks:
            blocks.append(key)
    for key in blocks:
        have = [EDITION_NAMES[t] for t in targets if any(m in identity[t] for m in key)]
        rows = [(n, r) for n, r in table.items() if tuple(r["blocks"]) == key]
        listed = have[0] if len(have) == 1 else "%s and %s" % (", ".join(have[:-1]), have[-1])
        title = "The %s edition%s" % (listed, "s" if len(have) > 1 else "")
        if len(key) > 1 and any(len({b["value"] for b in r["blocks"].values()}) > 1 for _, r in rows):
            title = "A value per edition: the %s editions" % listed
        out += ["### %s (%s)" % (title, ", ".join("`%s`" % m for m in key)), ""]
        if len(have) > 1 and len(key) == 1:
            out += ["Every edition that has one of these has them all, so no binary separates one from another: which of them "
                    "a site belongs to is a reading of what its code does.", ""]
        out += ["| Feature | %s | Sites | What it does |" % " | ".join(names), "|---|%s--:|---|" % (":-:|" * len(names))]
        for name, row in rows:
            cells = []
            for t in targets:
                b = has(row, identity[t])
                cells.append("" if b is None else (b["value"] or "yes"))
            docs = list(dict.fromkeys(b["doc"] for b in row["blocks"].values()))
            text = docs[0] if len(docs) == 1 else "; ".join(
                "%s (%s)" % (b["doc"], ", ".join(EDITION_NAMES[t] for t in targets if m in identity[t])) for m, b in row["blocks"].items())
            if row["kind"] != "feature":
                text = "*%s.* %s" % (row["kind"].capitalize(), text)
            out.append("| `%s` | %s | %d | %s |" % (name, " | ".join(cells), len(using[name]), text.replace("|", "\\|")))
        out.append("")
    out += ["## Where each feature is tested", ""]
    for name in table:
        out.append("- `%s`: %s" % (name, where(using[name]) or "nowhere"))
    out.append("")
    split = []
    for x in sites:
        if x["scope"] != "file":
            continue
        count = Counter(f for br in x["branches"] for f in set(br["contents"].get("functions") or ()))
        for f in sorted(n for n, c in count.items() if c > 1):
            split.append((f, x))
    out += ["## Functions with a definition per edition", "",
            "Where an edition's version of a function differs in its control flow or in more than a few statements, each "
            "version is a definition of its own; the comment above each says what that edition's does.", "",
            "| Function | File | Selected by | Definitions |", "|---|---|---|---|"]
    for f, x in sorted(split, key=lambda fx: fx[0].lower()):
        defs = []
        for br in x["branches"]:
            if f in (br["contents"].get("functions") or ()):
                have = [EDITION_NAMES[t] for t in targets if br["targets"][t] != "no"]
                defs.append(" and ".join([", ".join(have[:-1]), have[-1]] if len(have) > 2 else have) or "no target")
        out.append("| `%s` | `%s` | `%s` | %s |" % (f, x["file"], x["condition"].replace("|", "\\|"), "; ".join(defs)))
    out.append("")
    out += ["## Sites that test an edition", "",
            "What tests `MW2_ACCEL`, `MW2_MATROX` or `MW2_3DFX` itself rather than a feature, sorted by what the site holds "
            "(the tool's reading of its content).", ""]
    for macro in EDITION_MACROS:
        mine = using.get(macro, [])
        out += ["### `%s` (%d sites)" % (macro, len(mine)), ""]
        for kind, title in IDENTITY_KINDS:
            group = [x for x in mine if identity_kind(x) == kind]
            if group:
                out.append("- %s (%d): %s" % (title, len(group), where(group)))
        out.append("")
    out.append(DOC_END)
    return "\n".join(out)


def split_document(path):
    """The hand-written text before and after the generated part, and that part."""
    text = Path(path).read_text(encoding="utf-8") if Path(path).exists() else ""
    if DOC_BEGIN not in text or DOC_END not in text:
        return None
    a, b = text.index(DOC_BEGIN), text.index(DOC_END) + len(DOC_END)
    return text[:a], text[a:b], text[b:]


def write_document(path, generated):
    parts = split_document(path)
    if parts is None:
        sys.exit("%s has no generated part (%s ... %s)" % (path, DOC_BEGIN, DOC_END))
    Path(path).write_text(parts[0] + generated + parts[2], encoding="utf-8", newline="\n")


def check(sites, table, base, targets, path, generated):
    """What CI fails on: the table, the sources and the document must agree."""
    problems = []
    used = {m for x in sites for m in x["macros"]}
    for name, row in table.items():
        for macro, b in row["blocks"].items():
            if not b["doc"]:
                problems.append("util/types.h:%d: %s has no comment above its #define (under %s)" % (row["line"], name, macro))
        if name not in used:
            problems.append("util/types.h:%d: no conditional tests %s" % (row["line"], name))
    for name, place in sorted(base.get("tested", {}).items()):
        if name not in table and name not in EDITION_MACROS and not name.endswith("_H"):
            problems.append("%s: %s is tested, but util/types.h's table doesn't define it" % (place, name))
    for t in targets:
        if t != targets[0] and not base["identity"][t]:
            problems.append("%s's units define no edition macro" % t)
    for x in sites:
        for br in x["branches"]:
            if br.get("unreached") == "dead":
                problems.append("%s:%d: no target compiles this branch (%s)" % (x["file"], br["lines"][0], br["condition"]))
    parts = split_document(path)
    if parts is None:
        problems.append("%s is missing or has no generated part" % rel(path))
    elif parts[1].replace("\r\n", "\n") != generated:
        problems.append("%s is out of date: run python tools/edition_sites.py --doc" % rel(path))
    return problems


SITE_CLASSES = ("proven", "partly", "unverified", "no function")


def site_class(p):
    """A site's proof for one target in one word: proven (its function, or every function that
    defines or uses what a file-scope branch declares), partly (some of those), unverified (the
    function isn't proven, or none of them is), no function (nothing here can prove it), or
    None where the target doesn't compile the site."""
    st = p["state"]
    if st in ("proven",):
        return "proven"
    if st in ("not proven", "no function"):
        return "unverified"
    if st == "file scope":
        kind = file_scope_kind(p)
        return {"all proven": "proven", "some proven": "partly", "none proven": "unverified"}.get(kind, "no function")
    return None


def feature_proofs(sites, table, targets):
    """Per feature and edition macro, per target: how many of its sites are in each class."""
    out = {}
    for x in sites:
        for m in x["macros"]:
            row = out.setdefault(m, {"sites": 0, **{t: dict.fromkeys(SITE_CLASSES, 0) for t in targets}})
            row["sites"] += 1
            for t in targets:
                c = site_class(x["proof"][t])
                if c:
                    row[t][c] += 1
    order = list(table) + list(EDITION_MACROS)
    return {m: out[m] for m in order if m in out}


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build", help="The build tree")
    parser.add_argument("--states", action="append", default=[], metavar="TARGET=FILE", help="certify.py --json output")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"), help="Wine, for certify (off Windows)")
    parser.add_argument("--json", help="Write every site to this file")
    parser.add_argument("--markdown", help="Write the report (every site, with its lines and proofs) to this file")
    parser.add_argument("--no-states", action="store_true",
                        help="Stop before the proofs: the sites and what decides each one's proof (with --json, for --sites)")
    parser.add_argument("--sites", metavar="FILE",
                        help="Read the sites from a --no-states --json file instead of the build tree, and apply --states")
    parser.add_argument("--doc", nargs="?", const=DOC, metavar="FILE",
                        help="Rewrite the generated part of docs/EDITIONS.md (or FILE)")
    parser.add_argument("--check", action="store_true",
                        help="Fail unless the feature table, the sources and docs/EDITIONS.md agree (no states needed)")
    args = parser.parse_args()

    given = {}
    for s in args.states:
        t, _, f = s.partition("=")
        given[t] = f
    targets = SIMULATORS
    build = str((ROOT / args.build) if not Path(args.build).is_absolute() else args.build)

    if args.sites:
        data = json.loads(Path(args.sites).read_text(encoding="utf-8"))
        sites, tooling, base = data["sites"], data["tooling"], data["summary"]
    else:
        sources = {}
        reach = Reach()
        base = {"units": {}, "identity": {}}
        for t in targets:
            units = Target(t, build).units
            base["units"][t] = len(units)
            base["identity"][t] = sorted({m for unit in units for m in predefined(unit) if m in EDITION_MACROS},
                                         key=EDITION_MACROS.index)
            for unit in units:
                preprocess(t, unit, sources, reach)
        functions, by_name = load_annotations(targets)
        files = scan_files()
        sites = build_sites(sources, files, reach, functions, by_name, targets)
        plan_proofs(sites, targets, reach, functions, by_name, sources)
        tooling = tooling_references()
        base["tested"] = tested_macros(sources, files)
        base["table"] = feature_table()

    status = 0
    table = base["table"]  # with --sites, the table of the tree the sites came from
    if (args.doc or args.check) and not args.sites:
        text = document(sites, table, base["identity"], targets)
        path = Path(args.doc or DOC)
        path = path if path.is_absolute() else ROOT / path
        if args.doc:
            write_document(path, text)
        if args.check:
            problems = check(sites, table, base, targets, path, text)
            for line in problems:
                print("edition_sites: %s" % line)
            print("edition_sites --check: %s" % ("%d problem%s" % (len(problems), "" if len(problems) == 1 else "s") if problems else "ok"))
            status = 1 if problems else 0

    if args.no_states or ((args.doc or args.check) and not (given or args.markdown)):
        if args.json:
            Path(args.json).write_text(json.dumps({"summary": base, "sites": sites, "tooling": tooling}, indent=1) + "\n", encoding="utf-8")
        return status

    states, state_files = {}, {}
    for t in targets:
        states[t], state_files[t] = load_states(t, given, build, args.wine)
    apply_states(sites, targets, states)

    summary = summarize(sites, targets, tooling)
    summary.update(base)
    summary["states"] = state_files
    summary["features"] = feature_proofs(sites, table, targets)
    summary["unverified"] = {t: sum(site_class(x["proof"][t]) == "unverified" for x in sites) for t in targets}

    if args.json:
        Path(args.json).write_text(json.dumps({"summary": summary, "sites": sites, "tooling": tooling}, indent=1) + "\n", encoding="utf-8")
    if args.markdown:
        Path(args.markdown).write_text(markdown(sites, tooling, summary, targets), encoding="utf-8", newline="\n")
    print_summary(summary, targets)
    return status


def summarize(sites, targets, tooling):
    s = {"sites": len(sites), "directives": sum(1 for x in sites for br in x["branches"] if br["edition"])}
    s["directives_by_macro"] = dict(Counter(m for x in sites for br in x["branches"] for m in br["edition"]))
    s["by_macro"] = dict(Counter(m for x in sites for m in x["macros"]))
    s["by_macro_set"] = dict(Counter("+".join(x["macros"]) for x in sites))
    s["by_kind"] = dict(Counter(x["kind"] for x in sites))
    s["by_scope"] = dict(Counter(x["scope"] for x in sites))
    s["nested"] = sum(1 for x in sites if x["parent"])
    s["by_hint"] = dict(Counter(br["hint"] for x in sites for br in x["branches"]))
    s["sites_by_hint"] = dict(Counter(x["hint"] for x in sites))
    s["files"] = len({x["file"] for x in sites})
    s["proof"] = {t: dict(Counter(x["proof"][t]["state"] for x in sites)) for t in targets}
    s["proven_by_absence"] = {t: sum(1 for x in sites if x["proof"][t]["state"] == "proven" and not x["proof"][t]["branches"])
                              for t in targets}
    s["not_proven_by_function_state"] = {t: dict(Counter(x["proof"][t]["function_state"] for x in sites
                                                         if x["proof"][t]["state"] == "not proven")) for t in targets}
    s["file_scope"] = {t: dict(Counter(file_scope_kind(x["proof"][t]) for x in sites if x["proof"][t]["state"] == "file scope"))
                       for t in targets}
    s["unproven_sites_any"] = sum(1 for x in sites if any(x["proof"][t]["state"] == "not proven" for t in targets))
    s["sites_with_dead_branches"] = [x["id"] for x in sites if x["dead_branches"]]
    s["unreached_branches"] = dict(Counter(br["unreached"] for x in sites for br in x["branches"] if br.get("unreached")))
    s["tooling_references"] = len(tooling)
    s["tooling_files"] = len({r["file"] for r in tooling})
    return s


def file_scope_kind(p):
    """A file-scope proof in one word: every function involved proven, some, none, or no function."""
    total = sum(p[k]["functions"] for k in ("defined", "users") if k in p)
    proven = sum(p[k]["proven"] for k in ("defined", "users") if k in p)
    if not total:
        return "no function"
    return "all proven" if proven == total else "none proven" if not proven else "some proven"


def print_summary(s, targets):
    print("%d sites (%d edition directives) in %d files, %d nested" % (s["sites"], s["directives"], s["files"], s["nested"]))
    print("sites by macro:", ", ".join("%s %d" % kv for kv in sorted(s["by_macro"].items())),
          "| directives by macro:", ", ".join("%s %d" % kv for kv in sorted(s["directives_by_macro"].items())))
    print("by kind:", ", ".join("%s %d" % kv for kv in sorted(s["by_kind"].items())),
          "| by scope:", ", ".join("%s %d" % kv for kv in sorted(s["by_scope"].items())))
    print("sites by hint:", ", ".join("%s %d" % kv for kv in sorted(s["sites_by_hint"].items(), key=lambda kv: -kv[1])))
    print("branches by hint:", ", ".join("%s %d" % kv for kv in sorted(s["by_hint"].items(), key=lambda kv: -kv[1])))
    for t in targets:
        print("%-9s" % t, ", ".join("%s %d" % kv for kv in sorted(s["proof"][t].items(), key=lambda kv: -kv[1])))
    print("proven by the absence of every branch (the implicit #else):",
          ", ".join("%s %d" % (t, s["proven_by_absence"][t]) for t in targets))
    print("file scope:", "; ".join("%s %s" % (t, ", ".join("%s %d" % kv for kv in sorted(s["file_scope"][t].items())))
                                   for t in targets))
    print("branches no simulator compiles:", ", ".join("%s %d" % kv for kv in sorted(s["unreached_branches"].items())),
          "| sites with a dead branch:", ", ".join("#%d" % i for i in s["sites_with_dead_branches"]) or "none")
    print("tooling references: %d lines in %d files" % (s["tooling_references"], s["tooling_files"]))


SHORT = {"MW2": "1.1", "MW2MATROX": "Mtx", "MW23DFX": "3Dfx", "MW2ATI": "ATI"}


def proof_cell(p):
    st = p["state"]
    if st == "proven":
        return "proven (%s)" % p["function_state"]
    if st == "not proven":
        return "**not proven** (%s)" % p["function_state"]
    if st == "file scope":
        parts = ["%s %d/%d" % (k, p[k]["proven"], p[k]["functions"]) for k in ("defined", "users") if k in p]
        return "file: " + ", ".join(parts) + " proven" if parts else "file: no function"
    return st


def markdown(sites, tooling, s, targets):
    out = ["# Edition sites", "", "Generated by `tools/edition_sites.py`. One row per conditional group naming an edition macro;",
           "branches list the targets that compile them (1.1 = MW2, ATI = MW2ATI, Mtx = MW2MATROX, 3Dfx = MW23DFX; `?`: a MASM condition",
           "the tool can't evaluate encloses it). Proof per target: `proven`/`not proven` by the enclosing function's certify",
           "state (a target that compiles none of the branches is proven by the function too: the implicit #else); `file:` for",
           "file scope, the proven share of the functions the compiled branch defines and of those that use what it declares;",
           "`not reached` when an enclosing conditional leaves the group out for that target, `dead` when no build reaches it,",
           "`portable C` when only the portable (COMPAT_MODE) builds do, `not built` when the target doesn't compile the file.",
           ""]
    out.append("%d sites in %d files (%d nested)." % (s["sites"], s["files"], s["nested"]))
    out.append("")
    out += ["## Features", "", "Each feature's sites, and each edition macro's, by what proves them per target: proven (the enclosing",
            "function is byte-identical or witnessed; at file scope, every function that defines or uses what the branch",
            "declares) / partly (some of those functions) / unverified (the function isn't proven, or none is) / no function",
            "(a file-scope site no annotated function reaches: includes, data).", "",
            "| feature | sites | %s |" % " | ".join(SHORT[t] for t in targets), "|---|--:|" + "---|" * len(targets)]
    for name, row in s["features"].items():
        out.append("| `%s` | %d | %s |" % (name, row["sites"], " | ".join(" / ".join(str(row[t][c]) for c in SITE_CLASSES) for t in targets)))
    out.append("")
    by_file = defaultdict(list)
    for x in sites:
        by_file[x["file"]].append(x)
    for f in sorted(by_file):
        out += ["## %s" % f, "", "| # | lines | condition | function | branches | hints | %s |" % " | ".join(SHORT[t] for t in targets),
                "|---|---|---|---|---|---|" + "---|" * len(targets)]
        for x in by_file[f]:
            fn = x["function"] or x["container"] or ""
            if x["scope"] == "function" and x["annotations"]:
                fn = "%s (%s)" % (fn, ", ".join("%s %s" % (SHORT[t], a) for t, a in x["annotations"].items()))
            elif x["scope"] != "function":
                fn = "%s%s" % (x["scope"], " " + fn if fn else "")
            brs = "; ".join("%s %d-%d: %s" % (br["condition"] if br["directive"].lower() != "else" else "else", br["body"][0], br["body"][1],
                                              ",".join(SHORT[t] + ("?" if br["targets"][t] == "maybe" else "") for t in targets
                                                       if br["targets"][t] != "no") or "none (%s)" % br.get("unreached"))
                            for br in x["branches"])
            hint = "; ".join(br["hint"] for br in x["branches"])
            ident = str(x["id"]) + (" (in #%d)" % x["parent"]["site"] if x["parent"] else "")
            cells = [ident, "%d-%d" % tuple(x["lines"]), "`%s`" % x["condition"].replace("|", "\\|"), fn.replace("|", "\\|"),
                     brs.replace("|", "\\|"), hint] + [proof_cell(x["proof"][t]) for t in targets]
            out.append("| " + " | ".join(cells) + " |")
        out.append("")
    out += ["## Tooling references", "", "%d lines in %d files name an edition macro." % (s["tooling_references"], s["tooling_files"]), "",
            "| file | line | macros | text |", "|---|---|---|---|"]
    for r in tooling:
        out.append("| %s | %d | %s | `%s` |" % (r["file"], r["line"], ", ".join(r["macros"]), r["text"].replace("|", "\\|").replace("`", "'")))
    out += ["", "## Summary", "", "- sites: %d (%d edition directives), %d files, %d nested" % (s["sites"], s["directives"], s["files"], s["nested"]),
            "- by macro: " + ", ".join("%s %d" % kv for kv in sorted(s["by_macro"].items())),
            "- by kind: " + ", ".join("%s %d" % kv for kv in sorted(s["by_kind"].items())),
            "- by scope: " + ", ".join("%s %d" % kv for kv in sorted(s["by_scope"].items())),
            "- branches by hint: " + ", ".join("%s %d" % kv for kv in sorted(s["by_hint"].items(), key=lambda kv: -kv[1])),
            "- sites by macro set: " + ", ".join("%s %d" % kv for kv in sorted(s["by_macro_set"].items(), key=lambda kv: -kv[1])),
            "- sites by hint: " + ", ".join("%s %d" % kv for kv in sorted(s["sites_by_hint"].items(), key=lambda kv: -kv[1])),
            "- branches no simulator compiles: " + ", ".join("%s %d" % kv for kv in sorted(s["unreached_branches"].items())),
            "- sites with a dead branch (text no build compiles): " + (", ".join("#%d" % i for i in s["sites_with_dead_branches"]) or "none"),
            "- proven by the absence of every branch (the implicit #else): "
            + ", ".join("%s %d" % (t, s["proven_by_absence"][t]) for t in targets),
            "- file-scope sites: "
            + "; ".join("%s %s" % (t, ", ".join("%s %d" % kv for kv in sorted(s["file_scope"][t].items()))) for t in targets),
            "", "Proof per target:", "",
            "| target | %s |" % " | ".join(sorted({k for t in targets for k in s["proof"][t]})), "|---|" + "---|" * len({k for t in targets for k in s["proof"][t]})]
    keys = sorted({k for t in targets for k in s["proof"][t]})
    for t in targets:
        out.append("| %s | %s |" % (t, " | ".join(str(s["proof"][t].get(k, 0)) for k in keys)))
    out.append("")
    return "\n".join(out)


if __name__ == "__main__":
    sys.exit(main())
