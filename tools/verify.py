#!/usr/bin/env python3
"""Certify every target whose original is at hand, and print the counts the README's page shows.

For each target of targets.toml this looks for the original (a file of the target's name in
--originals, any case, or reccmp-user.yml's path), checks that it is the binary the project
targets (the SHA-256 targets.toml carries) and runs tools/certify.py on it: every unit
compiled again with the build's own commands, each witness record applied, every annotated
function compared with the original. It adds nothing to certify; it only saves running it once
per target with the right paths, and it is what the Docker image runs (docker/entrypoint.sh).

A target without an original, or one the build tree doesn't build (no VC++ 2.2 or 4.0), is
skipped and listed as such. The status is 1 when a file isn't the original it is named for, when
a witness record fails, or when no target could be checked; unproven functions don't fail it.

Each target's list goes to OUT/<TARGET>STATES.json (every function) and .md (the ones that aren't
byte-identical, with the recipe that proves each or its first difference).

usage (from the repository root, after building):
  python tools/verify.py [--originals DIR] [--build DIR] [--wine PATH] [--out DIR]
                         [--target T ...] [--functions]
"""

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from funccmp import read_yaml_paths  # noqa: E402
from unitbuild import MANIFEST, ROOT, TARGETS  # noqa: E402

WORDS = {"exact": "byte-identical", "witness": "proven", "unproven": "unproven"}


def project_targets():
    """The targets in targets.toml's order: {target: (file name, sha256)}."""
    return {name: (t["binary"], t["sha256"]) for name, t in MANIFEST.items()}


def find_original(target, filename, directory, user):
    """The target's original: FILENAME in `directory` whatever its case, or reccmp-user.yml's."""
    if directory and Path(directory).is_dir():
        for path in sorted(Path(directory).iterdir()):
            if path.is_file() and path.name.lower() == filename.lower():
                return path
    path = user.get(target)
    if path:
        path = Path(path) if os.path.isabs(path) else ROOT / path
        if path.is_file():
            return path
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("--originals", help="A directory of original binaries, named as targets.toml names them")
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build", help="The build tree")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"), help="Wine, off Windows")
    parser.add_argument("--out", help="Where the lists go (default: <build>/states)")
    parser.add_argument("--target", nargs="+", choices=sorted(TARGETS), help="Only these targets")
    parser.add_argument("--functions", action="store_true", help="Print every function's state, not only the counts")
    args = parser.parse_args()

    build = ROOT / args.build
    out = Path(args.out) if args.out else build / "states"
    out.mkdir(parents=True, exist_ok=True)
    user = read_yaml_paths(ROOT)
    rows, status, checked = [], 0, 0
    for target, (filename, sha256) in project_targets().items():
        if target not in TARGETS or (args.target and target not in args.target):
            continue
        original = find_original(target, filename, args.originals, user)
        if original is None:
            rows.append((target, filename, None, "no original: skipped"))
            continue
        digest = hashlib.sha256(original.read_bytes()).hexdigest()
        if sha256 and digest != sha256:
            rows.append((target, filename, None, "%s isn't the binary this project targets (SHA-256 %s, expected %s)"
                         % (original, digest, sha256)))
            status = 1
            continue
        if not (build / TARGETS[target][0]).is_dir():
            rows.append((target, filename, None, "not in this build tree (its compiler isn't configured): skipped"))
            continue

        states = out / ("%sSTATES.json" % target)
        states.unlink(missing_ok=True)
        command = [sys.executable, str(ROOT / "tools" / "certify.py"), "--target", target, "--build", str(build),
                   "--original", str(original), "--wine", args.wine, "--json", str(states),
                   "--markdown", str(out / ("%sSTATES.md" % target)), "-v"]
        print("%s (%s): certifying..." % (target, filename), flush=True)
        run = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        (out / ("%s-certify.txt" % target)).write_text(run.stdout)
        if not states.exists():
            sys.stdout.write(run.stdout)
            rows.append((target, filename, None, "certify failed (above)"))
            status = 1
            continue
        result = json.loads(states.read_text())
        note = ""
        if run.returncode:
            # A record that no longer gives the original's bytes, or a function exact without it
            sys.stdout.write(run.stdout)
            note = "witness records failed (above)"
            status = 1
        if args.functions:
            for f in result["functions"]:
                print("%s %s %-36s %-22s %s" % (target, f["address"], f["name"], f["unit"], WORDS.get(f["state"], f["state"])))
        rows.append((target, filename, result["summary"], note))
        checked += 1

    print()
    print("%-10s %-15s %9s %14s %7s %9s" % ("Target", "Original", "Functions", "Byte-identical", "Proven", "Unproven"))
    for target, filename, total, note in rows:
        if total is None:
            print("%-10s %-15s %s" % (target, filename, note))
        else:
            print("%-10s %-15s %9d %14d %7d %9d%s" % (target, filename, total["functions"], total["exact"], total["witness"],
                                                      total["unproven"], "  " + note if note else ""))
    print()
    if checked:
        print("Every function's state, and the recipe or the first difference of each one that isn't byte-identical: %s" % out)
    else:
        print("No target was checked: put the originals in --originals, or name them in reccmp-user.yml.")
        status = 1
    return status


if __name__ == "__main__":
    sys.exit(main())
