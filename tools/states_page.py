#!/usr/bin/env python3
"""Write the page of every target's proof states, for the progress site.

CI leaves each target's audit (TARGETAUDIT.json, from tools/audit.py: every annotated function with
certify's state and, for the unproven ones, the audit's verdict) next to its reccmp report. This
script reads the ones it finds in a directory and writes one self-contained HTML page:

- one table: per target, how many functions are identical to the original, proven by a witness
  record, explained, and not understood yet;
- per target, folded, every function that isn't byte-identical, with its recipe, its reason or its
  first difference.

A target with no audit there falls back to its TARGETSTATES.json (certify's alone: its unproven
functions all count as not understood). The targets come in reccmp-project.yml's order. Everything
read from the reports is data: it is escaped, never markup.

usage: python tools/states_page.py DIRECTORY [-o STATES.HTML] [--commit SHA] [--repository OWNER/NAME]
"""

import argparse
import datetime
import html
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
# The audit's verdicts, best first, and the column each counts in (tools/pr_comment.py's).
COLUMN = {"exact": "identical", "witness": "proven", "classified": "explained", "known": "explained",
          "unexplained": "unexplained"}
COLUMNS = ("identical", "proven", "explained", "unexplained")
HEADINGS = {"identical": "Byte-identical", "proven": "Proven", "explained": "Explained",
            "unexplained": "Not understood"}
WORDS = {"witness": "proven", "classified": "explained: compiler entropy", "known": "explained: known difference",
         "unexplained": "not understood"}
RANK = {"unexplained": 0, "known": 1, "classified": 2, "witness": 3}
EDITIONS = {"MW2SHELL": "1.1", "MW2": "1.1", "NETMECHW": "retail", "MECH2": "retail",
            "MW2MATROX": "Matrox Mystique", "MW23DFX": "3Dfx"}

STYLE = """
:root { color-scheme: light dark; --line: #8884; --dim: #8889; }
body { font: 15px/1.5 system-ui, sans-serif; max-width: 72rem; margin: 2rem auto; padding: 0 1rem; }
h1 { font-size: 1.6rem; } h2 { font-size: 1.2rem; margin-top: 2rem; }
table { border-collapse: collapse; width: 100%; }
th, td { border-bottom: 1px solid var(--line); padding: .3rem .6rem; text-align: left; vertical-align: top; }
td.n, th.n { text-align: right; font-variant-numeric: tabular-nums; }
code, td.what { font: 12.5px/1.4 ui-monospace, monospace; }
td.what { overflow-wrap: anywhere; }
.bar { display: flex; height: .5rem; min-width: 8rem; border-radius: .25rem; overflow: hidden; margin-top: .5rem; }
.identical { background: #2da44e; } .proven { background: #54aeff; }
.explained { background: #d4a72c; } .unexplained { background: #cf222e; }
.key { display: inline-block; width: .7rem; height: .7rem; border-radius: .15rem; margin-right: .3rem; }
.dim, footer { color: var(--dim); } footer { margin: 3rem 0 1rem; font-size: .85rem; }
summary { cursor: pointer; padding: .4rem 0; } details { border-bottom: 1px solid var(--line); }
details table { margin: .5rem 0 1rem; }
"""

LEGEND = """
<p>Each annotated function of a target is compared with the same function in the original binary.</p>
<ul>
<li><span class="key identical"></span><b>Byte-identical</b>: the source compiles to the original's code, every
relocated address pointing where the original's does.</li>
<li><span class="key proven"></span><b>Proven</b>: it does once its locals are renamed and/or a number of symbols
are declared ahead of its unit, as a committed record (<code>tools/witnesses/</code>) says, which CI rechecks.
Neither is recoverable from the original, and both only move stack slots, operand order and (in optimized code)
registers.</li>
<li><span class="key explained"></span><b>Explained</b>: the remaining difference is shown to be the compiler's own
(<code>tools/audit.py</code> rewrites one side into the other), or <code>tools/known_diffs.txt</code> gives the
reason for it.</li>
<li><span class="key unexplained"></span><b>Not understood</b>: it still differs from the original, and nobody has
found why yet.</li>
</ul>
"""


def esc(text):
    return html.escape(str(text), quote=True)


def project_targets():
    """reccmp-project.yml's targets in order: [(id, file name)]."""
    text = (ROOT / "reccmp-project.yml").read_text(encoding="utf-8")
    return re.findall(r"^  (\w+):\s*\n\s+filename:\s*(\S+)", text, re.M)


def load(directory, target):
    """A target's functions, each with a verdict, or None when the directory has no report of it."""
    for name, key in (("%sAUDIT.json", "verdict"), ("%sSTATES.json", "state")):
        path = directory / (name % target)
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, ValueError):
            continue
        functions = data.get("functions")
        if not isinstance(functions, list):
            continue
        for f in functions:
            verdict = f.get(key)
            f["verdict"] = verdict if verdict in COLUMN else "unexplained"
        return functions
    return None


def what(function):
    """What the reports say about a function that isn't byte-identical."""
    verdict = function["verdict"]
    if verdict == "witness":
        return function.get("recipe", "")
    if verdict == "classified":
        return ", ".join("%s %s" % (kind, count) for kind, count in sorted(function.get("kinds", {}).items()))
    return function.get("reason") or function.get("difference") or function.get("detail", "")


def page(directory, commit, repository):
    targets = []
    for target, binary in project_targets():
        functions = load(directory, target)
        if functions is not None:
            counts = dict.fromkeys(COLUMNS, 0)
            for f in functions:
                counts[COLUMN[f["verdict"]]] += 1
            targets.append((target, binary, functions, counts))
    if not targets:
        raise SystemExit("no TARGETAUDIT.json or TARGETSTATES.json in %s" % directory)

    out = ["<!DOCTYPE html>", '<html lang="en"><head><meta charset="utf-8">',
           '<meta name="viewport" content="width=device-width, initial-scale=1">',
           "<title>demech2: proof states</title>", "<style>%s</style></head><body>" % STYLE,
           "<h1>MechWarrior 2 decompilation: proof states</h1>", LEGEND,
           "<table><thead><tr><th>Binary</th><th class=n>Functions</th>"
           + "".join("<th class=n>%s</th>" % HEADINGS[c] for c in COLUMNS) + "<th>Reports</th></tr></thead><tbody>"]
    for target, binary, functions, counts in targets:
        total = max(len(functions), 1)
        bar = "".join('<span class="%s" style="width:%.3f%%" title="%s: %d"></span>' % (c, 100.0 * counts[c] / total, HEADINGS[c], counts[c])
                      for c in COLUMNS if counts[c])
        reports = ['<a href="#%s">functions</a>' % esc(target)]
        if (directory / (target + "PROGRESS.HTML")).exists():
            reports.append('<a href="%sPROGRESS.HTML">reccmp</a>' % esc(target))
        edition = ' <span class="dim">(%s)</span>' % esc(EDITIONS[target]) if target in EDITIONS else ""
        out.append("<tr><td><code>%s</code>%s<div class=bar>%s</div></td><td class=n>%d</td>%s<td>%s</td></tr>"
                   % (esc(binary), edition, bar, len(functions),
                      "".join("<td class=n>%d</td>" % counts[c] for c in COLUMNS), " · ".join(reports)))
    out.append("</tbody></table>")

    out.append("<h2>Functions that aren't byte-identical</h2>")
    for target, binary, functions, counts in targets:
        rest = sorted((f for f in functions if f["verdict"] != "exact"),
                      key=lambda f: (RANK[f["verdict"]], str(f.get("address", ""))))
        summary = ", ".join("%d %s" % (counts[c], HEADINGS[c].lower()) for c in COLUMNS[1:] if counts[c]) or "none"
        out.append('<details id="%s"><summary><code>%s</code>: %s</summary>' % (esc(target), esc(binary), summary))
        if rest:
            out.append("<table><thead><tr><th>Address</th><th>Function</th><th>Unit</th><th>State</th>"
                       "<th>Recipe, reason or first difference</th></tr></thead><tbody>")
            for f in rest:
                out.append("<tr><td><code>%s</code></td><td><code>%s</code></td><td>%s</td><td>%s</td><td class=what>%s</td></tr>"
                           % (esc(f.get("address", "")), esc(f.get("name", "")), esc(f.get("unit", "")),
                              WORDS[f["verdict"]], esc(what(f))))
            out.append("</tbody></table>")
        out.append("</details>")

    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    source = ""
    if commit and re.fullmatch(r"[0-9a-f]{7,40}", commit):
        link = esc(commit[:7])
        if repository and re.fullmatch(r"[\w.-]+/[\w.-]+", repository):
            link = '<a href="https://github.com/%s/commit/%s">%s</a>' % (repository, commit, link)
        source = " from %s" % link
    out.append("<footer>Written by <code>tools/states_page.py</code>%s, %s.</footer>" % (source, stamp))
    out.append("<script>if (location.hash) { var d = document.getElementById(location.hash.slice(1)); if (d) d.open = true; }"
               "addEventListener('hashchange', function () { var d = document.getElementById(location.hash.slice(1)); if (d) d.open = true; });</script>")
    out.append("</body></html>")
    return "\n".join(out) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("directory", type=Path)
    parser.add_argument("-o", "--output", type=Path)
    parser.add_argument("--commit")
    parser.add_argument("--repository")
    args = parser.parse_args()
    text = page(args.directory, args.commit, args.repository)
    output = args.output or args.directory / "STATES.HTML"
    output.write_text(text, encoding="utf-8")
    print("wrote %s" % output)
    return 0


if __name__ == "__main__":
    sys.exit(main())
