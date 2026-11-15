#!/usr/bin/env python3
"""Write the pull request's comment: where each target stands, and what the PR changed.

The Build workflow (the public targets) and the retail workflow (the others) each leave a payload
directory: for every target it checked, its audit (TARGETAUDIT.json: every function's state, from
tools/audit.py), main's audit from the latest release (TARGETAUDIT-main.json), and the tools'
own output (TARGET-diff.txt from reccmp, TARGET-certify.txt from certify and the audit), with
targets.txt listing the targets in order ("TARGET FILE"). This script reads one or more payloads
and writes one comment in Markdown:

- a verdict: whether any function got worse than on main, and how many got better;
- one table: per target, how many functions are identical to the original, proven, explained,
  and not understood yet, with the change from main;
- the functions whose state changed, worst first;
- the lines of the tools' output that need attention (a witness record that fails, a function no
  longer proven, a known_diffs.txt line to add or remove);
- what the columns mean, and every tool's full output, folded.

The payloads come from a pull request, so everything they hold is data: names are shown as code,
and text from the tools inside fenced blocks.

usage: python tools/pr_comment.py PAYLOAD... [--exceptions PR_BODY_FILE] [--note TEXT ...]
       [--tools-changed] [-o COMMENT.md]
"""

import argparse
import json
import re
from pathlib import Path

# The audit's verdicts, best first, and the column each counts in.
RANK = {"exact": 0, "witness": 1, "classified": 2, "known": 2, "unexplained": 3}
COLUMN = {"exact": "identical", "witness": "proven", "classified": "explained", "known": "explained",
          "unexplained": "unexplained"}
WORDS = {"exact": "identical", "witness": "proven", "classified": "explained (compiler noise)",
         "known": "explained (known difference)", "unexplained": "not understood", None: "gone"}
COLUMNS = ("identical", "proven", "explained", "unexplained")
HEADINGS = {"identical": "Identical", "proven": "Proven", "explained": "Explained",
            "unexplained": "Not understood"}
# The lines of certify's and the audit's output that summarize: everything else needs attention.
SUMMARY = re.compile(r"^\s*$|^(\w+): (\d+ functions|\d+ exact|coverage of )|^\(main has no states yet")
MAX_CHANGES = 60

LEGEND = """\
<details><summary>What the columns mean</summary>

Each function of a target is compared with the same function in the original binary.

- **Identical**: the rebuilt function is byte for byte the original's.
- **Proven**: it becomes byte-identical once its local variables are renamed or a few
  declarations are added ahead of it (a recipe recorded in `tools/witnesses/`, which CI
  rechecks). The compiler's choices differ; the code doesn't.
- **Explained**: the remaining difference is shown to be the compiler's own (register choice,
  the order of commutative operands, scheduling: `tools/audit.py` rewrites one side into the
  other), or `tools/known_diffs.txt` gives the reason for it.
- **Not understood**: it still differs from the original, and nobody has found why yet.

A pull request fails when a function that is identical or proven on main is neither any more,
unless its description has a line `Gate exception: TARGET FUNCTION REASON`.
</details>
"""


def code(text):
    """Text from a payload shown as inline code: no backticks or line breaks can escape it."""
    return "`%s`" % re.sub(r"[`\r\n]", "", str(text))[:80]


def fence(text):
    """Text from a payload inside a fenced block that it can't close."""
    return "```\n%s\n```" % re.sub(r"`{3,}", "'''", text.strip())


def load(path):
    try:
        return json.loads(path.read_text())
    except (OSError, ValueError):
        return None


def counts(audit):
    out = dict.fromkeys(COLUMNS, 0)
    for f in audit["functions"]:
        out[COLUMN.get(f["verdict"], "unexplained")] += 1
    return out


def exceptions(path):
    """The functions the PR's description excuses: {(target, function)}."""
    if not path:
        return set()
    text = Path(path).read_text(errors="replace")
    return {(m.group(1), m.group(2)) for m in re.finditer(r"^\s*Gate exception:\s*(\S+)\s+(\S+)", text, re.M)}


class Target:
    def __init__(self, payload, name, binary):
        self.name, self.binary = name, binary
        self.audit = load(payload / ("%sAUDIT.json" % name))
        self.main = load(payload / ("%sAUDIT-main.json" % name))
        self.certify = (payload / ("%s-certify.txt" % name))
        self.diff = (payload / ("%s-diff.txt" % name))

    def text(self, path):
        try:
            return re.sub(r"\x1b\[[0-9;]*[A-Za-z]", "", path.read_text(errors="replace"))
        except OSError:
            return ""

    def changes(self):
        """(rank change, address, name, main's verdict, the PR's verdict) for each function whose state changed."""
        if not self.audit or not self.main:
            return []
        old = {f["address"]: f for f in self.main["functions"]}
        new = {f["address"]: f for f in self.audit["functions"]}
        out = []
        for address in sorted(set(old) | set(new)):
            a, b = old.get(address), new.get(address)
            va, vb = a and a["verdict"], b and b["verdict"]
            if va == vb:
                continue
            ra = RANK.get(va, 4) if a else None
            rb = RANK.get(vb, 4) if b else 4
            if a is None:
                continue  # a function new on this branch is in the table, not a change
            out.append((rb - ra, address, (b or a)["name"], va, vb))
        return out

    def attention(self):
        return [line.rstrip() for line in self.text(self.certify).splitlines() if not SUMMARY.match(line)]


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("payloads", nargs="+", type=Path)
    parser.add_argument("--exceptions", help="The PR's description, for its Gate exception lines")
    parser.add_argument("--note", action="append", default=[], help="A line to show under the verdict")
    parser.add_argument("--tools-changed", action="store_true",
                        help="The PR changes the proof tools, which the retail check runs from main")
    parser.add_argument("-o", "--output", type=Path)
    args = parser.parse_args()

    targets = []
    for payload in args.payloads:
        listing = payload / "targets.txt"
        if not listing.exists():
            continue
        for line in listing.read_text().split("\n"):
            if line.strip():
                name, binary = (line.split() + [""])[:2]
                targets.append(Target(payload, name, binary))
    excused = exceptions(args.exceptions)

    rows, worse, better, changes, attention = [], 0, 0, [], []
    for t in targets:
        if not t.audit:
            rows.append("| %s | | | | | | no results: see the run |" % t.name)
            continue
        now = counts(t.audit)
        total = len(t.audit["functions"])
        if t.main:
            then = counts(t.main)
            delta = ", ".join("%+d %s" % (now[c] - then[c], HEADINGS[c].lower()) for c in COLUMNS if now[c] != then[c])
            delta = delta or "unchanged"
        else:
            delta = "main has no results yet"
        cells = [t.name, str(total)] + [str(now[c]) for c in COLUMNS] + [delta]
        rows.append("| %s |" % " | ".join(cells))
        for c in t.changes():
            changes.append((t,) + c)
            worse += c[0] > 0
            better += c[0] < 0
        lines = t.attention()
        if lines:
            attention.append((t, lines))

    gate = [c for c in changes if c[1] > 0 and RANK.get(c[4], 0) <= 1 and RANK.get(c[5], 4) > 1]
    failing = [c for c in gate if (c[0].name, c[3]) not in excused]

    out = ["## Decompilation status", ""]
    plural = lambda n, word: "%d %s%s" % (n, word, "" if n == 1 else "s")  # noqa: E731
    if not targets:
        out.append("No results: see the run.")
    else:
        if failing:
            out.append("**The gate fails:** %s identical or proven on main %s neither now (below). A line "
                       "`Gate exception: TARGET FUNCTION REASON` in the description accepts one." % (
                           plural(len(failing), "function"), "is" if len(failing) == 1 else "are"))
            out.append("")
        summary = ("%s got worse" % plural(worse, "function")) if worse else "No function got worse"
        if better:
            summary += ", %d got better" % better
        out.append(("**%s.**" % summary) if not failing else summary + ".")
    if attention:
        out.append("")
        problems = sum(len(lines) for _, lines in attention)
        out.append("The tools report %s needing attention (below)." % ("a problem" if problems == 1 else "problems"))
    if args.tools_changed and changes:
        out.append("")
        out.append("This pull request changes the proof tools, and the retail targets are checked with main's: "
                   "a change there may only be the tools'. It settles once the tools are merged.")
    for note in args.note:
        out.append("")
        out.append(note)
    out.append("")
    if rows:
        out.append("| Target | Functions | Identical | Proven | Explained | Not understood | Change from main |")
        out.append("|---|--:|--:|--:|--:|--:|---|")
        out += rows
        out.append("")

    if changes:
        out.append("### Functions whose state changed")
        out.append("")
        changes.sort(key=lambda c: (-c[1], c[0].name, c[2]))
        for t, step, address, name, old, new in changes[:MAX_CHANGES]:
            mark = "worse" if step > 0 else "better" if step < 0 else "changed"
            note = ""
            if (t, step, address, name, old, new) in gate:
                note = " (listed as a gate exception)" if (t.name, name) in excused else " (fails the gate)"
            states = "%s → %s" % (WORDS.get(old, old), WORDS.get(new, new))
            out.append("- %s: %s %s (%s), %s%s" % (mark, t.name, code(name), address, states, note))
        if len(changes) > MAX_CHANGES:
            out.append("- and %d more (see the run's artifacts)" % (len(changes) - MAX_CHANGES))
        out.append("")

    if attention:
        out.append("### Needs attention")
        out.append("")
        for t, lines in attention:
            out.append("%s:" % t.name)
            out.append(fence("\n".join(lines)))
        out.append("")

    out.append(LEGEND)
    out.append("<details><summary>The tools' full output (reccmp's scores, certify, the audit)</summary>")
    out.append("")
    out.append("reccmp's accuracy is a similarity score over every function; the table above counts "
               "proofs, which is what the checks rely on.")
    out.append("")
    for t in targets:
        out.append("#### %s (%s)" % (t.name, t.binary))
        out.append(fence(t.text(t.diff) or "(no reccmp report)"))
        out.append(fence(t.text(t.certify) or "(no certify output)"))
    out.append("</details>")

    text = "\n".join(out) + "\n"
    if args.output:
        args.output.write_text(text)
    else:
        print(text, end="")


if __name__ == "__main__":
    main()
