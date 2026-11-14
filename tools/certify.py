#!/usr/bin/env python3
"""Certify every annotated function of a target: byte-identical, proven by a witness, or unproven.

A witness record (`tools/witnesses/<TARGET>.txt`, found by `tools/witness.py`) says that a
function compiles to the original's bytes once its locals are renamed and/or N typedefs are
declared ahead of its unit: both change only what the original can't tell us (the locals' names
are lost; its headers declared another number of things). This script trusts none of that: it
applies each record to a scratch copy of the unit, compiles it with the build's own command
(tools/unitbuild.py), and compares the function with the original (tools/funccmp.py). It knows
nothing about the compilers and searches for nothing.

  exact     the function as the source has it compiles to the original's bytes
  witness   it does once its record is applied
  unproven  neither (the record, if any, is a failure)

A record that no longer gives the original's bytes fails, and so does one whose function is
exact without it. Every function has a cost, summed per target: 0 exact, 1 a witness by padding
alone, 2 one that renames locals, 1000 unproven.

usage (from the repository root):
  python tools/certify.py --target MW2 [--build build-wine] [--wine PATH] [--json STATES.json]

In two halves, for CI, where the compilers and the originals live in different jobs:
  python tools/certify.py --target MW2 --compile-only --objects DIR    (needs the compilers)
  python tools/certify.py --target MW2 --objects DIR --json STATES.json (needs the original)

The gate (per function): fails when a function exact or witnessed in BASE is neither in CURRENT,
unless the exceptions file has a line `<TARGET> <function> <reason>`:
  python tools/certify.py --gate BASE.json CURRENT.json [--exceptions FILE]
"""

import argparse
import json
import os
import re
import shutil
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from funccmp import Annotations, Comparison, read_yaml_paths, source_roots, symbol_name  # noqa: E402
from unitbuild import ROOT, TARGETS, Compiler, Image, Object, Request, Target  # noqa: E402

PAD = "typedef int ZqWitnessPad%d;\n"
COST = {"exact": 0, "pad": 1, "rename": 2, "unproven": 1000}


def pads_text(count):
    return "".join(PAD % i for i in range(count))


def read_records(path):
    """[{address, name, unit, pads, renames}] from a witness file."""
    records = []
    if not path.exists():
        return records
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


TOKEN = re.compile(r"""//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|'(?:\\.|[^'\\\n])*'|[A-Za-z_$][\w$]*|\d[\w.]*|->|::|\S""", re.S)


def rename(text, first, last, names):
    """The text with the identifiers in `names` renamed between two lines (1-based, inclusive),
    members (after `.`, `->` or `::`) and preprocessor lines left alone."""
    lines = text.split("\n")
    body = "\n".join(lines[first - 1 : last])
    out, at, previous = [], 0, ""
    for m in TOKEN.finditer(body):
        token = m.group(0)
        line_start = body.rfind("\n", 0, m.start()) + 1
        directive = body[line_start : m.start()].lstrip().startswith("#")
        if token in names and previous not in (".", "->", "::") and not directive:
            out.append(body[at : m.start()] + names[token])
            at = m.end()
        if not token.startswith(("//", "/*")):
            previous = token
    out.append(body[at:])
    return "\n".join(lines[: first - 1] + "".join(out).split("\n") + lines[last:])


def unit_annotations(annotations, unit, obj, tree=ROOT):
    """The unit's annotated functions: {COFF symbol: annotation}. A function annotated in one of
    the unit's own files, or for an assembled unit, by name in a header."""
    files = {str(path.resolve()) for path in unit.files(tree).values()}
    found = {}
    for symbol in obj.functions:
        name = symbol_name(symbol)
        own = [a for a in annotations.functions.get(name, []) if str(Path(a.filename).resolve()) in files]
        if not own and unit.language == "ASM_MASM":
            own = annotations.by_name.get(name, [])
        if len(own) == 1:
            found[symbol] = own[0]
    return found


class Certify:
    def __init__(self, target, build, wine="wine", jobs=4, tree=ROOT):
        self.target_name = target
        self.tree = Path(tree)
        self.annotations = Annotations(target, source_roots(target, self.tree), self.tree)
        self.target = Target(target, build) if build else None
        self.compiler = Compiler(Path(build or ".") / "certify" / target, wine, jobs) if build else None
        self.records = read_records(self.tree / "tools" / "witnesses" / (target + ".txt"))
        self.failures = []

    def compile(self, out, listings=True):
        """Compile every unit (with its /Fa listing, which tools/audit.py reads), and each unit once
        per padding its records use with their renames applied; copy the objects to `out` with a
        manifest."""
        units = [u for u in self.target.units if u.path(self.tree).exists()]
        by_name = {u.name: u for u in units}
        requests = [Request(u, self.tree, "plain", listing=listings and u.language != "ASM_MASM") for u in units]
        groups = defaultdict(list)
        for record in self.records:
            groups[(record["unit"], record["pads"])].append(record)
        variants = []
        for (unit_name, pads), group in sorted(groups.items()):
            unit = by_name.get(unit_name)
            if unit is None:
                self.failures += ["%s: no such unit (%s)" % (r["name"], unit_name) for r in group]
                continue
            texts = {}
            for record in group:
                found = [a for a in self.annotations.functions.get(record["name"], []) if a.offset == record["address"]]
                if not found:
                    self.failures.append("%s: no annotation at %#x" % (record["name"], record["address"]))
                    continue
                if record["renames"]:
                    path = Path(found[0].filename)
                    text = texts.get(path.name) or path.read_text(encoding="utf-8")
                    texts[path.name] = rename(text, found[0].line_number, found[0].end_line, record["renames"])
            request = Request(unit, self.tree, "pad%d" % pads, texts, prefix=pads_text(pads))
            requests.append(request)
            variants.append((unit_name, pads, request))
        self.compiler.run(self.target, requests)
        out = Path(out)
        shutil.rmtree(out, ignore_errors=True)
        out.mkdir(parents=True)
        manifest = {"target": self.target_name, "units": {}, "variants": []}
        for k, request in enumerate(requests):
            if request.error:
                self.failures.append("%s: %s" % (request.unit.name, request.error))
                continue
            name = "%d.obj" % k
            shutil.copy(request.obj, out / name)
            if request.tag == "plain":
                entry = {"obj": name, "source": request.unit.source, "language": request.unit.language,
                         "optimized": bool(re.search(r"(?:^|\s)[/-]O[x12]", request.unit.arguments))}
                if request.asm and Path(request.asm).exists():
                    entry["asm"] = "%d.asm" % k
                    shutil.copy(request.asm, out / entry["asm"])
                manifest["units"][request.unit.name] = entry
        for unit_name, pads, request in variants:
            if not request.error:
                manifest["variants"].append({"unit": unit_name, "pads": pads, "obj": "%d.obj" % requests.index(request)})
        (out / "manifest.json").write_text(json.dumps(manifest, indent=1) + "\n")
        self.compiler.discard(requests)

    def compare(self, objects, image):
        """Every annotated function's state, from the objects `compile` wrote."""
        objects = Path(objects)
        manifest = json.loads((objects / "manifest.json").read_text())
        units = {u.name: u for u in (self.target.units if self.target else [])}
        variants = {(v["unit"], v["pads"]): Object(objects / v["obj"]) for v in manifest["variants"]}
        records = {(r["unit"], r["address"]): r for r in self.records}
        used = set()
        states = []
        for unit_name, entry in sorted(manifest["units"].items()):
            unit = units.get(unit_name) or _Unit(entry)
            obj = Object(objects / entry["obj"])
            for symbol, annotation in unit_annotations(self.annotations, unit, obj, self.tree).items():
                comparison = Comparison(obj.functions[symbol], obj, image, annotation.offset, self.annotations)
                record = records.get((unit_name, annotation.offset))
                state = {"address": "%#010x" % annotation.offset, "name": symbol_name(symbol), "unit": unit_name}
                if comparison.exact:
                    state["state"], kind = "exact", "exact"
                    if record:
                        self.failures.append("%s (%s): exact without its witness: remove the record" % (record["name"], unit_name))
                elif record:
                    used.add((unit_name, annotation.offset))
                    variant = variants.get((unit_name, record["pads"]))
                    witnessed = None
                    if variant is not None and symbol in variant.functions:
                        witnessed = Comparison(variant.functions[symbol], variant, image, annotation.offset, self.annotations)
                    if witnessed is not None and witnessed.exact:
                        state["state"], kind = "witness", "rename" if record["renames"] else "pad"
                        state["recipe"] = recipe(record)
                    else:
                        state["state"], kind = "unproven", "unproven"
                        state["detail"] = comparison.difference or "stack slots"
                        self.failures.append("%s (%s): the witness no longer gives the original's bytes" % (record["name"], unit_name))
                else:
                    state["state"], kind = "unproven", "unproven"
                    state["detail"] = comparison.difference or "stack slots"
                state["cost"] = COST[kind]
                states.append(state)
        for key, record in records.items():
            if key not in used and not any(s["unit"] == key[0] and int(s["address"], 16) == key[1] for s in states):
                self.failures.append("%s (%s): no annotated function at %#x" % (record["name"], key[0], key[1]))
        return sorted(states, key=lambda s: s["address"])


class _Unit:
    """A unit known only from a manifest (comparing without a build tree)."""

    def __init__(self, entry):
        self.source, self.language = entry["source"], entry["language"]
        self.name = self.source.split("/")[-1]

    def files(self, tree):
        found, queue = {}, [Path(tree) / self.source]
        while queue:
            path = queue.pop()
            if path.name in found or not path.exists():
                continue
            found[path.name] = path
            for name in re.findall(r'^\s*#\s*include\s+"([^"]+\.c(?:pp)?)"', path.read_text(encoding="utf-8"), re.M):
                queue.append(path.parent / name)
        return found


def recipe(record):
    parts = []
    if record["pads"]:
        parts.append("pad %d" % record["pads"])
    if record["renames"]:
        parts.append("rename " + " ".join("%s=%s" % item for item in sorted(record["renames"].items())))
    return " ".join(parts)


def summary(target, states):
    counts = Counter(s["state"] for s in states)
    return {"target": target, "functions": len(states), "exact": counts["exact"], "witness": counts["witness"],
            "unproven": counts["unproven"], "cost": sum(s["cost"] for s in states)}


def gate(base_path, current_path, exceptions_path):
    """The functions proven in BASE that aren't in CURRENT, less the listed exceptions."""
    base, current = json.loads(Path(base_path).read_text()), json.loads(Path(current_path).read_text())
    target = current["summary"]["target"]
    allowed = set()
    if exceptions_path and Path(exceptions_path).exists():
        for line in Path(exceptions_path).read_text().split("\n"):
            m = re.match(r"^\s*(?:[-*]\s*)?(?:Gate exception:\s*)?(\w+)\s+(\S+)\s+\S", line)
            if m and m.group(1) == target:
                allowed.add(m.group(2))
    now = {s["address"]: s for s in current["functions"]}
    lost, excused, gained = [], [], []
    for old in base["functions"]:
        new = now.get(old["address"])
        if old["state"] in ("exact", "witness") and (new is None or new["state"] == "unproven"):
            (excused if old["name"] in allowed else lost).append((old, new))
    was = {s["address"]: s for s in base["functions"]}
    for new in current["functions"]:
        old = was.get(new["address"])
        if new["state"] != "unproven" and (old is None or old["state"] == "unproven"):
            gained.append(new)
    b, c = base["summary"], current["summary"]
    print("%s: %d exact, %d witnessed, %d unproven, cost %d (main: %d exact, %d witnessed, %d unproven, cost %d)"
          % (target, c["exact"], c["witness"], c["unproven"], c["cost"], b["exact"], b["witness"], b["unproven"], b["cost"]))
    for new in gained:
        print("  proven now: %s %s (%s)" % (new["address"], new["name"], new["state"]))
    for old, new in excused:
        print("  no longer proven, listed as an exception: %s %s" % (old["address"], old["name"]))
    for old, new in lost:
        print("  NO LONGER PROVEN: %s %s (was %s)%s" % (old["address"], old["name"], old["state"], ": " + new["detail"] if new else ": gone"))
    return 1 if lost else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", choices=sorted(TARGETS))
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build", help="The build tree")
    parser.add_argument("--original", help="The original binary (default: reccmp-user.yml's)")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"), help="Wine, off Windows")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--objects", help="Where the compiled objects go (--compile-only) or come from")
    parser.add_argument("--compile-only", action="store_true")
    parser.add_argument("--tree", default=str(ROOT), help="The source tree the annotations and records come from (default: this one)")
    parser.add_argument("--json", help="Write every function's state to this file")
    parser.add_argument("--markdown", help="Write the summary and the unproven functions to this file")
    parser.add_argument("--gate", nargs=2, metavar=("BASE", "CURRENT"), help="Compare two --json files per function")
    parser.add_argument("--exceptions", help="For --gate: lines `<TARGET> <function> <reason>`")
    parser.add_argument("-v", "--verbose", action="store_true", help="List every unproven function")
    args = parser.parse_args()

    if args.gate:
        return gate(args.gate[0], args.gate[1], args.exceptions)
    if not args.target:
        parser.error("--target is required")

    compiling = args.compile_only or not args.objects
    certify = Certify(args.target, ROOT / args.build if compiling else None, args.wine, args.jobs, Path(args.tree).resolve())
    objects = args.objects or (ROOT / args.build / "certify" / (args.target + "-objects"))
    if compiling:
        certify.compile(objects)
        if args.compile_only:
            for failure in certify.failures:
                print(failure)
            print("%s: compiled %d objects into %s" % (args.target, certify.compiler.count, objects))
            return 1 if certify.failures else 0

    original = args.original or read_yaml_paths(args.tree).get(args.target)
    if not original or not Path(original).exists():
        raise SystemExit("no original for %s: pass --original" % args.target)
    states = certify.compare(objects, Image(original))
    total = summary(args.target, states)
    if args.verbose:
        for state in states:
            if state["state"] == "unproven":
                print("%s %-32s %-20s %s" % (state["address"], state["name"], state["unit"], state["detail"]))
    for failure in certify.failures:
        print(failure)
    print("%s: %d functions: %d exact, %d witnessed, %d unproven; cost %d; %d failures"
          % (args.target, total["functions"], total["exact"], total["witness"], total["unproven"], total["cost"], len(certify.failures)))
    if args.json:
        Path(args.json).write_text(json.dumps({"summary": total, "functions": states}, indent=1) + "\n")
    if args.markdown:
        Path(args.markdown).write_text(markdown(total, states, certify.failures))
    return 1 if certify.failures else 0


def markdown(total, states, failures):
    """The summary and every function that isn't exact, as a Markdown table."""
    lines = ["# %s: %d functions" % (total["target"], total["functions"]), "",
             "| State | Functions |", "|---|---|",
             "| byte-identical | %d |" % total["exact"],
             "| proven by a witness (`tools/witnesses/%s.txt`) | %d |" % (total["target"], total["witness"]),
             "| unproven | %d |" % total["unproven"], "",
             "Cost: %d (0 byte-identical, 1 a witness by padding, 2 one that renames locals, 1000 unproven)." % total["cost"], ""]
    if failures:
        lines += ["## Record failures", ""] + ["- " + f for f in failures] + [""]
    lines += ["## Not byte-identical", "", "| Address | Function | Unit | State | Recipe or first difference |", "|---|---|---|---|---|"]
    for s in states:
        if s["state"] != "exact":
            what = s.get("recipe") or s.get("detail", "")
            lines.append("| %s | `%s` | %s | %s | %s |" % (s["address"], s["name"], s["unit"], s["state"], what.replace("|", "\\|")))
    return "\n".join(lines) + "\n"


if __name__ == "__main__":
    sys.exit(main())
