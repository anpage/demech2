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
