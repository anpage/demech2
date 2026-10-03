#!/usr/bin/env python3
"""Decide whether a change can affect the asmequiv tests (tests/asmequiv).

CI runs the asmequiv suites (the VC++ 4.1 reference, the modern MSVC one, and the
GCC and Clang sanitizer builds of the portable C) only when this prints "true".
A file counts as an input when it is:

- a source of the tests (from the compile commands of tests/asmequiv configured
  as a standalone project, plus every .c in tests/asmequiv, which also holds the
  MSVC-only asmequiv.c), the .asm next to any of them (the MASM units), or a
  file they include, transitively. Includes are followed textually, regardless
  of the preprocessor conditions around them, through the source's directory
  and the compile commands' include directories.
- under one of INPUT_PREFIXES: the tests themselves, VFX's assembly, the CMake
  helpers, the coverage tool, CI's definitions and this script.
- the top-level CMakeLists.txt, unless every changed line only adds or removes
  a source file or a comment. The tests take MW2's codegen flags from it.

Usage: asmequiv_changed.py --compile-commands <build>/compile_commands.json --base <rev>
compares the merge base of <rev> and HEAD with HEAD (git diff <rev>...HEAD).
--list prints the inputs instead.
"""

import argparse
import json
import os
import re
import shlex
import subprocess
import sys

REPO = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

INPUT_PREFIXES = (
    "tests/asmequiv/",
    "3rdparty/vfx/",
    "cmake/",
    "tools/asmblocks.py",
    "tools/asmequiv_changed.py",
    ".github/workflows/build.yml",
    ".github/actions/",
)

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.MULTILINE)
# A changed line of CMakeLists.txt that can't affect the tests: a unit in a SOURCES list, or a comment
HARMLESS_CMAKE_LINE_RE = re.compile(r"^\s*([\w/.${}-]+\.(c|cpp|asm|ASM|\$\{asm_or_c\}))?\s*(#.*)?$")


def relative(path):
    return os.path.relpath(os.path.normpath(path), REPO).replace(os.sep, "/")


def compile_inputs(compile_commands):
    """The sources of the compile commands and the include directories they use."""
    with open(compile_commands) as f:
        commands = json.load(f)
    sources = set()
    include_dirs = set()
    for entry in commands:
        directory = entry.get("directory", REPO)
        sources.add(os.path.normpath(os.path.join(directory, entry["file"])))
        args = entry["arguments"] if "arguments" in entry else shlex.split(entry["command"])
        for i, arg in enumerate(args):
            path = None
            if arg in ("-I", "-isystem", "/I") and i + 1 < len(args):
                path = args[i + 1]
            elif arg.startswith("-isystem") and len(arg) > len("-isystem"):
                path = arg[len("-isystem"):]
            elif arg.startswith(("-I", "/I")) and len(arg) > 2:
                path = arg[2:]
            if path:
                include_dirs.add(os.path.normpath(os.path.join(directory, path)))
    return sources, sorted(include_dirs)


def include_closure(roots, include_dirs):
    seen = set()
    pending = list(roots)
    while pending:
        path = pending.pop()
        if path in seen or not os.path.isfile(path):
            continue
        seen.add(path)
        with open(path, encoding="utf-8", errors="replace") as f:
            text = f.read()
        for name in INCLUDE_RE.findall(text):
            for directory in [os.path.dirname(path)] + include_dirs:
                candidate = os.path.normpath(os.path.join(directory, name))
                if os.path.isfile(candidate):
                    pending.append(candidate)
                    break
    return seen


def inputs(compile_commands):
    sources, include_dirs = compile_inputs(compile_commands)
    tests_dir = os.path.join(REPO, "tests", "asmequiv")
    sources.update(os.path.join(tests_dir, name) for name in os.listdir(tests_dir) if name.endswith(".c"))
    closure = include_closure(sources, include_dirs)
    for source in sources:
        asm = os.path.splitext(source)[0] + ".asm"
        if os.path.isfile(asm):
            closure.add(asm)
    return {relative(path) for path in closure if relative(path).split("/")[0] != ".."}


def git(*args):
    return subprocess.run(["git", "-C", REPO] + list(args), check=True, capture_output=True, text=True).stdout


def cmake_change_matters(base):
    diff = git("diff", "-U0", base + "...HEAD", "--", "CMakeLists.txt")
    for line in diff.splitlines():
        if line.startswith(("+++", "---")) or not line.startswith(("+", "-")):
            continue
        if not HARMLESS_CMAKE_LINE_RE.match(line[1:]):
            return line
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--compile-commands", required=True)
    parser.add_argument("--base", help="the revision to compare HEAD with (merge base)")
    parser.add_argument("--list", action="store_true", help="print the inputs")
    args = parser.parse_args()

    files = inputs(args.compile_commands)
    if args.list:
        print("\n".join(sorted(files)))
        return 0
    if not args.base:
        parser.error("--base or --list is required")

    changed = git("diff", "--name-only", args.base + "...HEAD").splitlines()
    reasons = [path for path in changed if path in files or path.startswith(INPUT_PREFIXES)]
    if "CMakeLists.txt" in changed:
        line = cmake_change_matters(args.base)
        if line:
            reasons.append("CMakeLists.txt (" + line.strip() + ")")
    for reason in reasons:
        print("asmequiv input changed: " + reason, file=sys.stderr)
    if not reasons:
        print("no asmequiv input changed (%d files changed, %d inputs)" % (len(changed), len(files)), file=sys.stderr)
    print("true" if reasons else "false")
    return 0


if __name__ == "__main__":
    sys.exit(main())
