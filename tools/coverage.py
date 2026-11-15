"""Coverage: an exact or witnessed /Od function covers an optimized one of another target.

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
shares (1.1's fixed-point code never covers it: the editions' float conversion changes the
tokens), and the Matrox edition's optimized units (the depth sort) by 1.1, where the tokens agree.
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


# ---------------------------------------------------------------------------------------------
# Digests: what the comparison needs, without the compilers
#
# The verify jobs that compare the objects with the originals have no compilers, so the job that
# compiles the objects (certify.py --compile-only, which has them) writes each function's closure
# as digests: the definition's hash and the sorted hashes of the items it reaches, and for each
# item hash the names it declares (to say which declarations differ). `tools/audit.py --coverage
# --digests FILE` compares them as it compares the closures.


def short_hash(text):
    return hashlib.sha1(text.encode("latin-1")).hexdigest()[:12]


def item_names(text):
    return " ".join(sorted(declared(text.split()) or {text.split()[0]}))


def optimized_unit(unit):
    return bool(re.search(r"(?:^|\s)[/-]O[x12]", unit.arguments))


def unit_closures(work):
    """{name: (the definition's hash, [(an item's hash, its names)], the closure's fingerprint) or
    None} of the named functions of one preprocessed unit."""
    unit_name, text, names = work
    parsed = Items(text)
    out = {}
    for name in names:
        closure = parsed.closure(name)
        if closure is None:
            out[name] = None
            continue
        own, rest = closure
        out[name] = (short_hash(own), [(short_hash(t), item_names(t)) for t in rest], fingerprint(closure))
    return out


def target_digests(name, build, tree, wine, jobs, scratch, keys=None):
    """{"functions": {address: entry}, "items": {hash: names}} of a target's annotated C functions:
    those of its optimized units (keys None), or those defined at the given (file, line) keys. An
    entry: the function's name, file (relative to the tree), line, unit, whether the unit is
    optimized, and its closure's digest (`own`, `rest`, `tokens`) or why there is none (`error`)."""
    from funccmp import Annotations, source_roots
    from unitbuild import Target

    tree = Path(tree).resolve()
    target = Target(name, build)
    annotations = Annotations(name, source_roots(name, tree), tree)
    units = [u for u in target.units if u.language in ("C", "CXX") and u.path(tree).exists()]
    units_of = {}
    for unit in units:
        for path in unit.files(tree).values():
            units_of.setdefault(str(path.resolve()), []).append(unit)
    wanted = []  # (address, name, file, line, candidate units)
    for fname, found in annotations.functions.items():
        for a in found:
            path = Path(a.filename).resolve()
            try:
                rel = path.relative_to(tree).as_posix()
            except ValueError:
                continue
            candidates = units_of.get(str(path), [])
            if keys is None:
                candidates = [u for u in candidates if optimized_unit(u)]
                if not candidates:
                    continue
            elif (rel, a.line_number) not in keys:
                continue
            wanted.append(("%#010x" % a.offset, fname, rel, a.line_number, candidates))
    needed = {u.name: u for *_, candidates in wanted for u in candidates}
    texts = preprocess(target, list(needed.values()), tree, scratch, wine, jobs)
    # Each unit parsed once, in parallel: {unit: {name: (own, [(hash, names)], tokens) or None}}.
    names_of = {}
    for _, fname, _, _, candidates in wanted:
        for unit in candidates:
            names_of.setdefault(unit.name, set()).add(fname)
    closures = {}
    work = [(u, texts[u], sorted(n)) for u, n in names_of.items() if texts.get(u)]
    if jobs > 1 and len(work) > 1:
        from concurrent.futures import ProcessPoolExecutor

        with ProcessPoolExecutor(max_workers=jobs) as pool:
            for unit_name, found in zip([w[0] for w in work], pool.map(unit_closures, work, chunksize=1)):
                closures[unit_name] = found
    else:
        for w in work:
            closures[w[0]] = unit_closures(w)
    items, functions = {}, {}
    for address, fname, rel, line, candidates in wanted:
        entry = {"name": fname, "file": rel, "line": line}
        for unit in candidates:
            found = closures.get(unit.name, {}).get(fname)
            if found is None:
                continue
            own, rest, tokens = found
            for h, names in rest:
                items.setdefault(h, names)
            entry.update(unit=unit.name, optimized=optimized_unit(unit), own=own, rest=sorted(h for h, _ in rest), tokens=tokens)
            break
        else:
            entry["unit"] = candidates[0].name if candidates else None
            entry["optimized"] = bool(candidates) and optimized_unit(candidates[0])
            entry["error"] = "no unit compiles it" if not candidates else "its definition isn't found in the preprocessed unit"
        functions[address] = entry
    return {"functions": functions, "items": items}


def digests(targets, build, tree, wine="wine", jobs=4):
    """The digests coverage needs for `targets` (covered targets: their optimized functions; and
    their coverers: the functions defined where those are)."""
    build, out = Path(build), {}
    scratch = build / "certify" / "coverage"
    for name in targets:
        if name in COVERERS:
            out[name] = target_digests(name, build, tree, wine, jobs, scratch / name)
    for name in list(out):
        keys = {(e["file"], e["line"]) for e in out[name]["functions"].values() if e["optimized"]}
        for other in COVERERS[name]:
            found = target_digests(other, build, tree, wine, jobs, scratch / other, keys)
            if other in out:
                out[other]["functions"].update({a: e for a, e in found["functions"].items() if a not in out[other]["functions"]})
                out[other]["items"].update(found["items"])
            else:
                out[other] = found
    return out


def compare(target, data, states):
    """{address: ("covered", {...}) or ("uncovered", why)} of the target's optimized functions in
    `data` (digests), against its coverers' digests and `states` ({target: {address: state}}, or
    None for a coverer whose states aren't known)."""
    mine = data.get(target, {}).get("functions", {})
    out = {}
    for address, entry in mine.items():
        if not entry.get("optimized"):
            continue
        key = (entry["file"], entry["line"])
        why = None
        for other in COVERERS.get(target, []):
            theirs = data.get(other, {}).get("functions", {})
            match = next(((a, e) for a, e in theirs.items() if (e["file"], e["line"]) == key), None)
            if match is None:
                continue  # no function of the coverer is defined there
            if match[1].get("error") == "no unit compiles it":
                why = why or "%s doesn't compile it" % other
                continue
            other_address, other_entry = match
            known = states.get(other)
            state = known.get(other_address) if known is not None else None
            if known is None:
                why = why or "no states of %s" % other
            elif state is None:
                why = why or "%s doesn't compile it" % other
            elif other_entry.get("optimized"):
                why = why or "%s compiles it optimized too" % other
            elif state not in ("exact", "witness"):
                why = why or "%s's /Od function is %s" % (other, state)
            elif "error" in entry or "error" in other_entry:
                why = why or "its definition isn't found in the preprocessed unit"
            elif entry["own"] != other_entry["own"]:
                why = "%s: the definitions differ" % other
            elif entry["rest"] != other_entry["rest"]:
                a, b = set(entry["rest"]), set(other_entry["rest"])
                names = set()
                for h in a - b:
                    names |= set(data[target]["items"].get(h, "?").split())
                for h in b - a:
                    names |= set(data[other]["items"].get(h, "?").split())
                why = "%s: the declarations of %s differ" % (other, ", ".join(sorted(names)[:4])) + (" (and %d more)" % (len(names) - 4) if len(names) > 4 else "")
            else:
                out[address] = ("covered", {"target": other, "address": other_address, "state": state, "tokens": entry["tokens"]})
                break
        else:
            out[address] = ("uncovered", why or "no /Od target compiles it")
    return out


def main():
    import argparse
    import json
    import os

    from unitbuild import ROOT

    parser = argparse.ArgumentParser(description=__doc__ + """
usage (from the repository root): python tools/coverage.py --build build --out certify/coverage.json
  writes the digests of every covered target (COVERERS) and its coverers, which
  `tools/audit.py --coverage --digests FILE` reads where there are no compilers (the verify jobs)""",
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build")
    parser.add_argument("--tree", default=str(ROOT))
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"))
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--target", nargs="*", default=sorted(COVERERS), help="The covered targets (default: all)")
    parser.add_argument("--out", required=True)
    args = parser.parse_args()
    data = digests(args.target, ROOT / args.build, args.tree, args.wine, args.jobs)
    Path(args.out).parent.mkdir(parents=True, exist_ok=True)
    Path(args.out).write_text(json.dumps({"targets": data}, separators=(",", ":")) + "\n")
    for name, d in data.items():
        print("%s: %d functions digested (%d without a closure), %d items"
              % (name, len(d["functions"]), sum("error" in e for e in d["functions"].values()), len(d["items"])))
    return 0


if __name__ == "__main__":
    import sys

    sys.path.insert(0, str(Path(__file__).resolve().parent))
    sys.exit(main())
