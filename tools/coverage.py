"""Rule 5: an exact or witnessed /Od function covers an optimized one of another target.

The optimized editions (the 3Dfx edition, the Matrox edition's renderer and depth sort) compile
some of the same C as an /Od target. Where the two targets compile the same function from the same
tokens, with every declaration it reaches the same, a proof at /Od (byte-identical, or a witness)
says the optimized function's source is the original's too: what's left between the optimized
function and its original is the optimizer's.

"The same" is decided on each target's preprocessed unit (`cl /EP`, the unit's own command and
defines, so macros are expanded and the edition's #ifdefs resolved), split into top-level items
(declarations, and definitions with their bodies):

  - the function's definition, every token of it;
  - and the closure of the declarations it reaches: every item declaring an identifier the
    definition uses, then every item declaring an identifier those use, and so on (a typedef's
    struct, a struct's member types, an enum's constants, a called function's prototype or
    signature; an inline function's body, which the optimizer expands).

Both lists must be equal, token for token. A name is declared by an item where it is a declarator
(followed by ; , = [ or a parameter list, or the name in (*name)), a struct, union or enum tag
with a body or alone, or an enum constant. Over-reaching only makes coverage rarer: an item pulled
in that both targets declare the same costs nothing, and one that differs (the compilers' own
headers do, between VC++ 2.2 and 4.0) leaves the function uncovered.

Which targets cover which (COVERERS): the 3Dfx edition only by the Matrox edition, whose C it
shares (1.1's float-free code never covers it, by the plan's rule 5), and the Matrox edition's
optimized units (the depth sort) by 1.1, where the tokens agree.
"""

import hashlib
import re
import shutil
import subprocess
from pathlib import Path

from unitbuild import WINDOWS, to_tool

COVERERS = {"MW23DFX": ["MW2MATROX"], "MW2MATROX": ["MW2"]}

TOKEN = re.compile(r""""(?:\\.|[^"\\\n])*"|'(?:\\.|[^'\\\n])*'|[A-Za-z_$][\w$]*|\d[\w.]*|->|::|\S""")
IDENTIFIER = re.compile(r"[A-Za-z_$][\w$]*$")
KEYWORDS = set("""auto break case char const continue default do double else enum extern float for goto if int long
register return short signed sizeof static struct switch typedef union unsigned void volatile while
__cdecl __stdcall __fastcall _cdecl _stdcall _fastcall __declspec __inline inline __forceinline __int8 __int16
__int32 __int64 __asm _asm __try __except __finally __leave __based __far __near _far _near far near
__pascal _pascal pascal __export _export __unaligned""".split())
INLINE = {"__inline", "inline", "__forceinline"}
CONVENTIONS = ("__cdecl", "__stdcall", "__fastcall", "_cdecl", "_stdcall", "_fastcall", "__pascal", "_pascal", "pascal")


def preprocess(target, units, tree, scratch, wine="wine", jobs=4):
    """{unit name: the preprocessed text} of a target's units (unitbuild Units), with /EP /P."""
    scratch = Path(scratch)
    shutil.rmtree(scratch, ignore_errors=True)
    batches = [units[i::jobs] for i in range(jobs)]
    processes = []
    for n, batch in enumerate(b for b in batches if b):
        work = scratch / ("batch%d" % n)
        work.mkdir(parents=True)
        lines = ["@echo off"]
        for k, unit in enumerate(batch):
            out = work / ("u%d" % k)
            out.mkdir()
            response = out / "cl.rsp"
            text = "/nologo %s /EP /P %s" % (unit.options(tree), to_tool(unit.path(tree)))
            response.write_text("\n".join(re.findall(r'(?:"[^"]*"|\S)+', text)) + "\n")
            lines.append("cd /d %s" % to_tool(out))
            lines.append("%s @%s" % (unit.compiler, to_tool(response)))
        (work / "run.bat").write_text("\r\n".join(lines) + "\r\n")
        env = target.environment(batch[0])
        command = ["cmd", "/c", str(work / "run.bat")] if WINDOWS else [wine, "cmd", "/c", to_tool(work / "run.bat")]
        log = open(work / "log.txt", "w")
        processes.append((subprocess.Popen(command, stdout=log, stderr=subprocess.STDOUT, env=env, cwd=work), log, work, batch))
    out = {}
    for process, log, work, batch in processes:
        process.wait()
        log.close()
        for k, unit in enumerate(batch):
            found = list((work / ("u%d" % k)).glob("*.i"))
            if found:
                out[unit.name] = found[0].read_text(encoding="latin-1")
    return out


class Items:
    """A preprocessed unit as top-level items: [(tokens, declared names, is a definition)], and
    for each definition where its body starts."""

    def __init__(self, text):
        # Directives are dropped, but #pragma pack changes layouts: each item that starts under a
        # pack other than the default carries it as its first token.
        lines, pack, stack = [], None, []
        for line in text.split("\n"):
            m = re.match(r"\s*#\s*pragma\s+pack\s*\(\s*(push)?\s*,?\s*(pop)?\s*,?\s*(\d+)?\s*\)", line)
            if m:
                if m.group(1):
                    stack.append(pack)
                if m.group(2):
                    pack = stack.pop() if stack else None
                if m.group(3) or not (m.group(1) or m.group(2)):
                    pack = m.group(3) if m.group(3) != "8" else None  # 8: the compilers' default (no /Zp)
                lines.append(" %s " % ("__pack_" + pack if pack else "__pack_default"))
            elif not line.lstrip().startswith("#"):
                lines.append(line)
        tokens = TOKEN.findall("\n".join(lines))
        marks, pack = [], None
        for t in tokens:
            if t.startswith("__pack_"):
                pack = None if t == "__pack_default" else t
            else:
                marks.append((t, pack))
        tokens = [t for t, _ in marks]
        packs = [p for _, p in marks]
        self.items = []  # (tokens, body start or None)
        start, depth, k, n = 0, 0, 0, len(tokens)
        while k < n:
            t = tokens[k]
            if t in "([":
                depth += 1
            elif t in ")]":
                depth -= 1
            elif t == "{":
                if depth == 0 and k > start and tokens[k - 1] == ")" and "=" not in tokens[start:k]:
                    # A function's body: the item ends where it does.
                    level, j = 0, k
                    while j < n:
                        level += tokens[j] == "{"
                        level -= tokens[j] == "}"
                        if level == 0:
                            break
                        j += 1
                    prefix = [packs[start]] if packs[start] else []
                    self.items.append((prefix + tokens[start : j + 1], k - start + len(prefix)))
                    start = k = j + 1
                    continue
                depth += 1
            elif t == "}":
                depth -= 1
            elif t == ";" and depth == 0:
                self.items.append((([packs[start]] if packs[start] else []) + tokens[start : k + 1], None))
                start = k + 1
            k += 1
        self.declares = {}  # name -> [item index]
        for index, (item, body) in enumerate(self.items):
            for name in declared(item[:body] if body is not None else item):
                self.declares.setdefault(name, []).append(index)

    def definition(self, name):
        """The index of the definition of a function, or None."""
        found = [i for i in self.declares.get(name, []) if self.items[i][1] is not None]
        return found[0] if len(found) == 1 else None

    def closure(self, name):
        """(the function's tokens, the sorted texts of the items it reaches), or None."""
        index = self.definition(name)
        if index is None:
            return None
        seen, queue = {index}, [index]
        while queue:
            item, body = self.items[queue.pop()]
            used = item if body is None or item is self.items[index][0] or INLINE & set(item[:body]) else item[:body]
            for token in used:
                if IDENTIFIER.match(token) and token not in KEYWORDS:
                    for other in self.declares.get(token, []):
                        if other not in seen:
                            seen.add(other)
                            queue.append(other)
        own = " ".join(self.items[index][0])
        rest = sorted(" ".join(self.text(i)) for i in seen if i != index)
        return own, rest

    def text(self, i):
        """An item as the closure compares it: a definition other than an inline one by its
        signature (its body doesn't change its callers' code)."""
        item, body = self.items[i]
        if body is None or INLINE & set(item[:body]):
            return item
        return item[:body]


def declared(tokens):
    """The names an item declares (see the module's docstring)."""
    out = set()
    depth, brace, skip = 0, 0, False
    enum_brace = None
    n = len(tokens)
    for k, t in enumerate(tokens):
        following = tokens[k + 1] if k + 1 < n else ""
        previous = tokens[k - 1] if k else ""
        if t in "([":
            depth += 1
        elif t in ")]":
            depth -= 1
        elif t == "{":
            brace += 1
            if previous == "enum" or k >= 2 and tokens[k - 2] == "enum":
                enum_brace = brace
        elif t == "}":
            if enum_brace == brace:
                enum_brace = None
            brace -= 1
        if not IDENTIFIER.match(t) or t in KEYWORDS:
            if t == "=" and depth == 0 and brace == 0:
                skip = True
            elif t in (",", ";") and depth == 0 and brace == 0:
                skip = False
            continue
        if enum_brace is not None and brace == enum_brace and previous in ("{", ","):
            out.add(t)  # an enum constant
            continue
        if brace or skip:
            continue
        if previous in ("struct", "union", "enum") and following in ("{", ";"):
            out.add(t)
        elif depth == 0 and following in (";", ",", "=", "[", ":"):
            out.add(t)
        elif depth == 0 and following == "(" and (tokens[k + 2] if k + 2 < n else "") not in CONVENTIONS + ("*",):
            out.add(t)  # a function's name, not the return type of a (*name) or (__stdcall name)
        elif depth == 1 and previous in CONVENTIONS and following == ")":
            out.add(t)  # (__stdcall name): a function type
        elif previous == "*" and following == ")" and k >= 2 and (tokens[k - 2] == "(" or tokens[k - 2] in KEYWORDS and k >= 3 and tokens[k - 3] == "("):
            out.add(t)  # (*name) or (__cdecl *name): a pointer to a function
    return out


def fingerprint(closure):
    own, rest = closure
    return hashlib.sha1(("\n".join([own] + rest)).encode("latin-1")).hexdigest()[:12]


def difference(mine, other):
    """Why two closures differ, in a few words."""
    if mine[0] != other[0]:
        return "the definitions differ"
    a, b = set(mine[1]), set(other[1])
    names = set()
    for text in sorted(a ^ b):
        names |= declared(text.split()) or {text.split()[0]}
    return "the declarations of %s differ" % ", ".join(sorted(names)[:4]) + (" (and %d more)" % (len(names) - 4) if len(names) > 4 else "")
