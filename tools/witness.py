#!/usr/bin/env python3
"""Search for witnesses that a function differs from the original only by compiler entropy.

At /Od the compilers give every named local a stack slot in the order of a hash of its name (and
within one bucket of the hash, mostly in the order of declaration), then the temporaries ($T123) in
the order they're created, outside the hash; and they number the duplicates of a nested
scope (`i$99`) with a counter that every symbol declared ahead of the function advances. Neither is
recoverable from the original: its locals' names are lost. So a function can be right and still
differ from the original in its `[ebp-N]` operands. Optimized code (/Ox: the 3Dfx edition, the
Matrox edition's renderer and depth sort) moves with the symbols declared ahead of it too, in its
registers and its order.

This tool looks for a witness: a variant of the unit that changes only the locals' names or the
number of symbols declared ahead of it (`--pads N`: up to N typedefs ahead of the unit), and
compiles the function to the original's bytes. A witness proves that the function's tokens are
right. The source keeps its readable names; the witness is a record in
`tools/witnesses/<TARGET>.txt`, which `tools/certify.py` checks. This tool is the finder: nothing
rests on it, and it may be as clever as it needs to be.

For each unit it compiles the source as the build does (tools/unitbuild.py), compares each
annotated function with the original (tools/funccmp.py), and reports one of:

  exact      the bytes are the original's (relocated fields compared by their targets)
  witness    exact once the locals are renamed and/or with N symbols declared ahead (one of them,
             with --early, a global of the unit's)
  slots      the only differences are stack slots, but no renaming tried reproduces them
  other      the first difference isn't a stack slot: it is printed

The renaming (unoptimized units): where the only differences are `[ebp-N]` operands, the order in
which the original allocated the named locals follows from them (each local must map to one place,
without overlap; where the original mirrors a comparison or swaps a commutative operation's
operands, the one access paired with the other operand's slot is outvoted by the local's others).
The locals are renamed, in a scratch copy, to names the compiler allocates in
that order: a probe compiled twice (its locals declared forward, then reversed) sorts a pool of
names into the hash's buckets and finds the order of the buckets, and within a bucket whether the
first declared comes first, without knowing the hash. A C++ unit's probe is a member function,
which places `this` (a name that can't change) among the buckets. A nested scope's locals come
after the function's own, scope by scope, each scope's in the hash's order (they are renamed per
scope), or else, where their names are unique, among the function's own. With more locals than
buckets, locals share buckets in their order of declaration: as early as it allows first, then the
other ways it allows, fewest shared buckets first, each compiled until one gives the original.
Optimized units are searched by padding alone.

One global redeclared early (`--early N`, unoptimized units, after the paddings): which side of a
comparison goes in a register, and which operand of a commutative operation loads first, follows the
symbols' positions. Padding shifts every global of a unit at once, so a function whose comparisons
with two globals need different paddings has no witness by padding. For each global such a function
names, the unit is compiled with that global redeclared ahead of it (`extern int g_playerCount;`,
its type spelled in base types and struct tags, since no header is in yet), among up to N typedefs
before it and N after: that moves the one global alone, in this unit only, and changes no header.
The original's headers are lost, and where they declared each global with them. A spelling the
compiler rejects (another edition's type, a static) is dropped. The globals in the operand-order
rewrites tools/audit.py needs for the function are tried first, and a variant is renamed only where
the audit leaves the slots alone to map. Operand order has period 17 in the symbols declared ahead,
so N = 16 covers every position. `--recipe 'pad A early DECL B'` checks one padding found another
way (a record's spelling) before the search.

usage (from the repository root, with reccmp's environment):
  python tools/witness.py --target MW2 [--build build-wine] [--wine PATH] [--unit ai.c ...]
                          [--function NAME ...] [--pads N] [--early N] [--recipe RECIPE] [--jobs N] [--write] [--json FILE] [-v]

--write records the witnesses in tools/witnesses/<TARGET>.txt: the records of the units and
functions searched are replaced, the others kept; a search without --early keeps the records that
redeclare a global unless it finds the function a witness without one.
"""

import argparse
import bisect
import itertools
import json
import os
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from certify import EARLY, TOKEN, block_tokens, pads_text, read_recipe, read_records, recipe, rename, unit_annotations  # noqa: E402
from funccmp import Annotations, Comparison, read_yaml_paths, source_roots, symbol_name  # noqa: E402
from unitbuild import ROOT, TARGETS, Compiler, Image, Object, Request, Target, Unit  # noqa: E402

FIXED = ("this",)  # locals the compiler names itself: they can't be renamed
SHARED_PLANS = 200  # the other bucket sharings tried for a function whose locals outnumber the buckets


def optimized(unit):
    """Whether a unit is compiled with the optimizer (`/Ox`, `/O1`, `/O2`; `/Ob1` alone isn't)."""
    return bool(re.search(r"(?:^|\s)[/-]O[x12]", unit.arguments))


def read_listing(path):
    """{function symbol: [(local name, offset, renameable)]} from a /Fa listing."""
    result, pending = {}, []
    for line in Path(path).read_text(errors="replace").split("\n"):
        m = re.match(r"^__\$\w+\$ = (-?\d+)\s*$", line)
        if m:  # the C++ EH record: a fixed area at the top of the frame, which never moves
            pending.append(("$EHRec", int(m.group(1)), False))
            continue
        m = re.match(r"^_(\S+)\$ = (-?\d+)\s*$", line)
        if m:
            name = m.group(1)
            pending.append((name, int(m.group(2)), "$" not in name and name not in FIXED))
            continue
        m = re.match(r"^_?(\S+) = (-?\d+)\s*$", line)
        if m:  # a nested scope's duplicate (_i$99) or a named temporary ($T123)
            pending.append((m.group(1), int(m.group(2)), False))
            continue
        m = re.match(r"^(\S+)\s+PROC NEAR", line)
        if m:
            result[m.group(1)] = pending
            pending = []
    return result


# ---------------------------------------------------------------------------------------------
# Renaming


def majority(local_list, slots):
    """The slot pairs with each named local's place decided by majority: where the original mirrors
    a comparison or swaps a commutative operation's operands, one access pairs a local with the
    other operand's slot, and the local's other accesses outvote it (as tools/audit.py's slot map
    does). A witness built on the vote still has to compile to the original's bytes."""
    starts = sorted({offset for name, offset, _ in local_list if offset < 0 and name != "$EHRec"})
    if not starts:
        return slots
    votes = {}
    for ours, theirs in slots:
        if ours >= starts[0]:
            k = max(i for i, s in enumerate(starts) if s <= ours)
            votes.setdefault(k, Counter())[theirs - (ours - starts[k])] += 1
    best = {k: counts.most_common(1)[0][0] for k, counts in votes.items()}
    kept = []
    for ours, theirs in slots:
        if ours >= starts[0]:
            k = max(i for i, s in enumerate(starts) if s <= ours)
            if theirs - (ours - starts[k]) != best[k]:
                continue
        kept.append((ours, theirs))
    return kept


def slot_plan(local_list, slots, unlisted=()):
    """The named locals in the order the original allocates them, or a reason there is none.
    local_list: (name, offset, renameable) from the listing; slots: (ours, the original's);
    unlisted: (name, size) of locals the listing leaves out (2.2 omits the unreferenced ones),
    placed into the frame's free room."""
    # The C++ EH record ends the locals' area: the slots inside it stay where they are.
    top = min([offset for name, offset, _ in local_list if name == "$EHRec"] or [0])
    named = sorted((offset, name, renameable) for name, offset, renameable in local_list if offset < 0 and name != "$EHRec")
    if not named:
        return None, "no named locals"
    starts = [offset for offset, _, _ in named]
    bases = {}
    for ours, theirs in slots:
        if ours < starts[0]:
            if ours != theirs:
                return None, "an unnamed temporary moved (ebp%+d, the original's ebp%+d)" % (ours, theirs)
            continue
        if ours >= top:
            if ours != theirs:
                return None, "the EH record moved"
            continue
        index = max(k for k, start in enumerate(starts) if start <= ours)
        base = theirs - (ours - starts[index])
        if bases.setdefault(index, base) != base:
            return None, "%s is at two places in the original" % named[index][1]
    sizes = [(starts[k + 1] if k + 1 < len(starts) else top) - starts[k] for k in range(len(starts))]
    taken = sorted((base, base + sizes[k], k) for k, base in bases.items())
    for (base, end, k), (start, _, n) in zip(taken, taken[1:]):
        if end > start:
            if start - base >= 4:  # a local the listing doesn't list (never referenced) took the room
                sizes[k] = start - base
                continue
            return None, "%s and %s overlap in the original" % (named[k][1], named[n][1])
    if taken and (taken[0][0] < starts[0] or taken[-1][1] > top):
        return None, "%s is outside the named locals' area in the original" % named[taken[0][2]][1]
    # Locals the code never addresses go into the gaps, the largest first.
    free, at = [], starts[0]
    for start, end, _ in taken:
        if start > at:
            free.append([at, start])
        at = end
    if at < top:
        free.append([at, top])
    for k in sorted((k for k in range(len(named)) if k not in bases), key=lambda k: -sizes[k]):
        gap = next((g for g in free if g[1] - g[0] >= sizes[k]), None)
        if gap is None:
            return None, "no room for %s in the original's frame" % named[k][1]
        bases[k] = gap[1] - sizes[k]
        gap[1] -= sizes[k]
    placed = [(bases[k], named[k][1]) for k in bases]
    for name, size in unlisted:
        gap = next((g for g in reversed(free) if g[1] - g[0] >= size), None)
        if gap is None:
            continue
        placed.append((gap[1] - size, name))
        gap[1] -= size
    return [name for _, name in sorted(placed, reverse=True)], None


class ProbeUnit(Unit):
    """A probe compiled with a unit's compiler and options, from a scratch file of its own."""

    def __init__(self, unit, tag):
        super().__init__(unit.target, unit.compiler, unit.language, unit.arguments, unit.source, unit.root)
        self.source = str(Path(unit.source).parent / ("zqprobe%s%s" % (tag, Path(unit.source).suffix))).replace("\\", "/")
        self.name = Path(self.source).name

    def files(self, tree):
        return {}


def ladder(compiler, target, unit, extras=()):
    """The compiler's allocation order as a list of entries: ("bucket", names, direction) for one
    bucket of the hash (names that all hash there; direction +1 if the first declared is allocated
    first, -1 if last, 0 for a single name) and ("fixed", name) for a name that can't be renamed
    (`this`; `extras`: a nested scope's duplicates, `i$99`, declared in the probe under their own
    names, after its pool), in the order the buckets are allocated.
    Found by compiling a probe twice."""
    alphabet = "abcdefghijklmnopqrstuvwxyz0123456789"
    pool = ["zq" + a + b for a in alphabet[:12] for b in alphabet]
    member = unit.language == "CXX"

    def probe(names):
        names = names + list(extras)  # after the locals, as a nested scope's are
        body = "".join("\tint %s;\n" % n for n in names) + "".join("\t%s = 0;\n" % n for n in names)
        if member:
            return "struct ZqProbeClass { int m_zq; void ZqProbe(); };\nvoid ZqProbeClass::ZqProbe()\n{\n%s\tm_zq = 0;\n}\n" % body
        return "void ZqProbe(void)\n{\n%s}\n" % body

    forward, backward = ProbeUnit(unit, "a"), ProbeUnit(unit, "b")
    requests = [Request(forward, ROOT, "probe", {forward.name: probe(pool)}, listing=True),
                Request(backward, ROOT, "probe", {backward.name: probe(pool[::-1])}, listing=True)]
    compiler.run(target, requests)
    orders = []
    for request in requests:
        if request.error:
            raise SystemExit("the probe doesn't compile: " + request.error)
        local_list = next(iter(read_listing(request.asm).values()))
        orders.append([name for _, name in sorted(((offset, name) for name, offset, _ in local_list), reverse=True)])
    rank = {name: k for k, name in enumerate(orders[1])}
    index = {name: k for k, name in enumerate(pool)}
    groups = [[orders[0][0]]]
    for name in orders[0][1:]:
        if rank[groups[-1][-1]] < rank[name]:  # the same order both ways: another bucket
            groups.append([name])
        else:
            groups[-1].append(name)
    entries = []
    for group in groups:
        fixed = [n for n in group if n not in index]
        if fixed:
            entries.append(("fixed", fixed[0]))
            continue
        direction = 0 if len(group) < 2 else (1 if index[group[1]] > index[group[0]] else -1)
        entries.append(("bucket", sorted(group, key=lambda n: index[n]), direction))
    return entries


def declaration_orders(text, first, last, names):
    """The position of each local's declaration in the function's text, as a list of readings, the
    likeliest first. A name declared in more than one #if group (an edition's own body under #else)
    counts where the group that declares most of `names` (the listing's, the target's) declares it,
    and when groups tie (each edition's body declares every local), there is one reading per group;
    a name no line declares counts at its first appearance."""
    lines = text.split("\n")[first - 1 : last]
    start = next((k for k, line in enumerate(lines) if "{" in line), 0)
    stack, current, declarations = [0], 0, []
    for k in range(start, len(lines)):
        line = lines[k]
        if re.match(r"\s*#\s*if", line):
            current += 1
            stack.append(current)
        elif re.match(r"\s*#\s*(else|elif)", line):
            current += 1
            stack[-1] = current
        elif re.match(r"\s*#\s*endif", line) and len(stack) > 1:
            stack.pop()
        m = re.match(r"^\t+(?:const )?\w[\w ]*?[\s*]+(\w+)(?:\[[^\]]*\])*;$", line)
        if m and m.group(1) in names:
            declarations.append((m.group(1), stack[-1], k))
    weight = Counter(group for _, group, _ in declarations)
    heaviest = [g for g in dict.fromkeys(group for _, group, _ in declarations) if weight[g] == max(weight.values())]
    previous, seen, first_use = "", 0, {}
    body = "\n".join(lines[start:])
    for m in TOKEN.finditer(body):
        token = m.group(0)
        if token in names and token not in first_use and previous not in (".", "->", "::"):
            first_use[token] = (start + body.count("\n", 0, m.start()), 1 + seen)
            seen += 1
        if not token.startswith(("//", "/*")):
            previous = token
    readings = []
    for preferred in heaviest or [None]:
        position = {}
        for name, group, k in declarations:
            if name not in position or (position[name][0] != preferred and (group == preferred or weight[group] > weight[position[name][0]])):
                position[name] = (group, k)
        places = {name: (k, 0) for name, (_, k) in position.items()}
        for name, place in first_use.items():
            places.setdefault(name, place)
        reading = {name: k for k, name in enumerate(sorted(places, key=lambda n: places[n]))}
        if reading not in readings:
            readings.append(reading)
    return readings


def declaration_order(text, first, last, names):
    """The likeliest of declaration_orders()."""
    return declaration_orders(text, first, last, names)[0]


NESTED = re.compile(r"^(\w+)\$(\d+)$")  # a nested scope's local in a listing


def scope_groups(body, tail, outer):
    """A nested scope's locals, `tail` in their allocation order, as each scope's (brace blocks
    around it, innermost last; names in that order), or None when they don't come scope by scope.
    The listing numbers a name's copies in their order of declaration, which matches each copy to
    its scope in the text. Blocks are numbered as certify's renaming numbers them."""
    stacks = {}  # line -> the blocks around its first token
    text = "\n".join(body)
    for m, stack in block_tokens(text):
        stacks.setdefault(text.count("\n", 0, m.start()), stack)
    blocks = defaultdict(list)
    for k, line in enumerate(body):
        m = re.match(r"^\t+(?:const )?\w[\w ]*?[\s*]+(\w+)(?:\[[^\]]*\])*(?:\s*=[^=].*)?;$", line)  # with its initializer
        if m and len(stacks.get(k, ())) > 1:
            blocks[stacks[k]].append(m.group(1))
    where = defaultdict(list)  # name -> the blocks declaring it, in order
    for block in sorted(blocks, key=lambda stack: stack[-1]):
        for name in blocks[block]:
            where[name].append(block)
    copies = defaultdict(list)
    for name in tail:
        base, number = NESTED.match(name).groups()
        copies[base].append(int(number))
    block_of = {}
    for base, numbers in copies.items():
        if base in outer or len(numbers) != len(where[base]):
            return None
        for number, block in zip(sorted(numbers), where[base]):
            block_of["%s$%d" % (base, number)] = block
    groups, seen = [], set()
    for name in tail:
        block = block_of[name]
        if groups and groups[-1][0] == block:
            groups[-1][1].append(NESTED.match(name).group(1))
        elif block in seen:
            return None
        else:
            groups.append((block, [NESTED.match(name).group(1)]))
            seen.add(block)
    return groups


def assign_scopes_one_name(scopes, entries, used):
    """{local: new name} giving each scope's locals the buckets of their allocation order (a
    scope's are allocated by the hash like the function's own), or (None, a reason). A name
    several scopes declare gets one name, so it needs one bucket that suits all of them: each
    name takes the earliest bucket after those of the names that precede it in any scope."""
    buckets = [entry[1] for entry in entries if entry[0] == "bucket"]
    before = defaultdict(set)  # name -> the names allocated right before it in some scope
    for _, names in scopes:
        if len(names) < 2:
            continue
        before[names[0]]
        for previous, local in zip(names, names[1:]):
            before[local].add(previous)
    level, visiting = {}, set()

    def place(local):
        if local in level:
            return True
        if local in visiting:
            return False
        visiting.add(local)
        if not all(place(p) for p in before[local]):
            return False
        visiting.discard(local)
        level[local] = 1 + max((level[p] for p in before[local]), default=-1)
        return True

    for local in list(before):
        if not place(local):
            return None, "%s's scopes need it in two places" % local
    mapping, taken = {}, set(used)
    for local in sorted(level, key=lambda n: (level[n], n)):
        if level[local] >= len(buckets):
            return None, "no bucket for %s in its scope" % local
        name = next((n for n in buckets[level[local]] if n not in taken), None)
        if name is None:
            return None, "no name left for %s" % local
        mapping[local] = name
        taken.add(name)
    return mapping, None


def assign_scopes(scopes, entries, used):
    """{local or local@block: new name} giving each scope's locals the buckets of their allocation
    order, or (None, a reason): one name for each local wherever that suits every scope declaring
    it, and otherwise each scope's locals renamed inside their own block (`id@12`, certify's
    rename_tokens), a block's names kept apart from those of the blocks around it and inside it."""
    mapping, reason = assign_scopes_one_name(scopes, entries, used)
    if mapping is not None:
        return mapping, None
    buckets = [entry[1] for entry in entries if entry[0] == "bucket"]
    mapping, taken = {}, {}  # taken: block stack -> the names given there
    for stack, names in scopes:
        if len(names) < 2:
            continue
        if len(names) > len(buckets):
            return None, "%d locals in one scope, and the hash has %d buckets" % (len(names), len(buckets))
        related = set(used).union(*(given for other, given in taken.items() if other[: len(stack)] == stack or stack[: len(other)] == other))
        taken[stack] = set()
        for position, local in enumerate(names):
            name = next((n for n in buckets[position] if n not in related), None)
            if name is None:
                return None, "no name left for %s" % local
            mapping["%s@%d" % (local, stack[-1])] = name
            taken[stack].add(name)
    return mapping, None


def assign(order, entries, declared, fixed, share=False):
    """{local: new name} that the compiler allocates in `order`, or (None, a reason). Each local
    takes the next bucket, a fixed name (`this`, `i$99`) is where the compiler puts it, and with
    `share` (more locals than buckets), a local stays in the current bucket when the order of
    declaration allows it. Sharing is the fallback, not the default: the order within a bucket
    doesn't always follow the declarations as `declaration_order` reads them, and sharing whenever
    it was allowed lost 14 of MW2's witnesses that one local per bucket finds."""
    mapping, placed = {}, set()
    position, last = -1, None  # the current entry, the last local put in it
    for local in order:
        if local in fixed:
            k = next((k for k in range(position + 1, len(entries)) if entries[k] == ("fixed", local)), None)
            if k is None:
                return None, "%s can't come this late" % local
            position, last = k, None
            placed.add(local)
            continue
        entry = entries[position] if position >= 0 else None
        used = sum(1 for v in mapping.values() if entry and entry[0] == "bucket" and v in entry[1])
        if share and entry and entry[0] == "bucket" and last and used < len(entry[1]) and entry[2] * (declared.get(local, -1) - declared.get(last, -1)) > 0:
            mapping[local], last = entry[1][used], local
            continue
        k = position + 1
        while k < len(entries) and entries[k][0] != "bucket":
            if entries[k][1] in order and entries[k][1] not in placed:
                return None, "%s would come before %s" % (entries[k][1], local)
            k += 1
        if k >= len(entries):
            return None, "%d locals, and the hash has %d buckets" % (len(order), sum(1 for e in entries if e[0] == "bucket"))
        position, last = k, local
        mapping[local] = entries[k][1][0]
    return mapping, None


def bucket_plans(order, buckets, limit=20000):
    """Ways to put the locals, in their allocation order, into the buckets in order (a bucket
    holding up to its names' count), fewest locals per bucket first: [[(bucket index, [locals])]]."""
    out = []

    def go(pos, b, acc):
        if len(out) >= limit:
            return
        if pos == len(order):
            out.append(list(acc))
            return
        if b >= len(buckets) or len(order) - pos > sum(len(e[1]) for e in buckets[b:]):
            return
        for take in range(1, min(len(buckets[b][1]), len(order) - pos) + 1):
            acc.append((b, order[pos : pos + take]))
            go(pos + take, b + 1, acc)
            acc.pop()
        go(pos, b + 1, acc)

    go(0, 0, [])
    return out


def shared_plans(order, entries, declared, limit):
    """The renamings that share buckets as the order of declaration allows (within a bucket, the
    compiler allocates its locals in their order of declaration, or the reverse), fewest shared
    buckets first: up to `limit`."""
    buckets = [e for e in entries if e[0] == "bucket"]
    found = []

    def go(pos, b, names, shares):
        if len(found) >= 20000:
            return
        if pos == len(order):
            found.append((shares, len(found), dict(names)))
            return
        if b >= len(buckets) or len(order) - pos > sum(len(e[1]) for e in buckets[b:]):
            return
        entry = buckets[b]
        for take in range(1, min(len(entry[1]), len(order) - pos) + 1):
            group = order[pos : pos + take]
            if take > 1 and entry[2] * (declared.get(group[-1], -1) - declared.get(group[-2], -1)) <= 0:
                break  # the order of declaration doesn't allow it, nor any longer group
            for k, local in enumerate(group):
                names[local] = entry[1][k]
            go(pos + take, b + 1, names, shares + take - 1)
            for local in group:
                del names[local]
        go(pos, b + 1, names, shares)

    go(0, 0, {}, 0)
    return [names for _, _, names in sorted(found, key=lambda f: f[:2])[:limit]]


# ---------------------------------------------------------------------------------------------
# The search


class Entry:
    def __init__(self, unit, name, annotation):
        self.unit, self.name, self.annotation = unit, name, annotation
        self.address = annotation.offset
        self.status = None
        self.detail = ""
        self.renames = {}
        self.pads, self.early = 0, ()
        self.unverified = []

    def note(self, status, detail, pads, early=()):
        """Keep the closest verdict: slots-only (at the lowest padding) over another difference,
        and the unpadded compile's difference over a padded one's. The padding is named as a
        record would (what tools/declorder.py's --pads and --early take)."""
        rank = {"slots": 0, "other": 1}
        if self.status not in rank or rank[status] < rank[self.status] or rank[status] == rank[self.status] and not self.detail:
            ahead = recipe({"pads": pads, "early": early, "renames": {}})
            self.status, self.detail = status, detail + (" (with %s)" % ahead if ahead else "")

    def recipe(self):
        return recipe({"pads": self.pads, "early": self.early, "renames": self.renames})


class Search:
    def __init__(self, args, target):
        self.args, self.target_name = args, target
        original = args.original or read_yaml_paths().get(target)
        if not original or not Path(original).exists():
            raise SystemExit("no original for %s: pass --original" % target)
        self.image = Image(original)
        self.annotations = Annotations(target, source_roots(target))
        self.target = Target(target, ROOT / args.build)
        self.units = [u for u in self.target.units if u.path(ROOT).exists() and u.language != "ASM_MASM"]
        scratch = ROOT / args.build / "witness" / target
        self.compiler = Compiler(scratch, args.wine, args.jobs)
        self.ladders = {}
        self.entries = []
        self.pads, self.ahead = 0, ()

    def compare(self, request, wanted=None):
        """{symbol: (Comparison, the function's locals, annotation)} of one compile, or an error."""
        if request.error:
            return request.error
        obj, local_lists = Object(request.obj), read_listing(request.asm)
        out = {}
        for symbol, annotation in unit_annotations(self.annotations, request.unit, obj).items():
            if wanted is None or symbol in wanted:
                comparison = Comparison(obj.functions[symbol], obj, self.image, annotation.offset, self.annotations)
                out[symbol] = (comparison, local_lists.get(symbol, []), annotation)
        return out

    def plan(self, unit, compared, pending):
        """The renamed files of the unit for the functions that differ only in slots."""
        texts, plans, orders = {}, {}, {}
        pads = self.pads
        for symbol, entry in pending.items():
            comparison, local_list, annotation = compared[symbol]
            if comparison.difference:
                entry.note("other", comparison.difference, pads)
                continue
            if optimized(unit):
                entry.note("slots", "an optimized unit: padding only", pads, self.ahead)
                continue
            # Scalars the body declares that the listing leaves out (never referenced).
            listed = {name for name, _, _ in local_list}
            body = Path(annotation.filename).read_text(encoding="utf-8").split("\n")[annotation.line_number - 1 : annotation.end_line]
            unlisted = []
            for line in body:
                m = re.match(r"^\t(?:const )?\w+\s*\**\s*(\w+);$", line)
                if m and m.group(1) not in listed:
                    unlisted.append((m.group(1), 4))
            order, reason = slot_plan(local_list, comparison.slots, unlisted)
            if order is None and reason.endswith("at two places in the original"):
                order, voted = slot_plan(local_list, majority(local_list, comparison.slots), unlisted)
                if order is None:
                    reason = voted
            if order is None:
                entry.note("slots", reason, pads, self.ahead)
                continue
            # The compiler's temporaries ($T123) come after the named locals, in the order it
            # creates them, outside the hash: they must end the order.
            while order and (order[-1].startswith("$T") or order[-1] == "this"):
                order = order[:-1]
            fixed = {name for name, _, renameable in local_list if not renameable} & set(order)
            # A nested scope's locals (`poly$1015`) come after the function's own, scope by scope:
            # the function's own are planned as usual, each scope's on its own.
            scopes = None
            tail = len(order)
            while tail and NESTED.match(order[tail - 1]):
                tail -= 1
            if tail < len(order) and not any(NESTED.match(name) for name in order[:tail]):
                scopes = scope_groups(body, order[tail:], set(order[:tail]))
                if scopes is not None:
                    fixed -= set(order[tail:])
                    order = order[:tail]
            if scopes is None:
                # Otherwise a nested local whose name no other local has is renamed like the
                # function's own (the hash places it by its name).
                bases = Counter(name.split("$")[0] for name, _, _ in local_list if not name.startswith("$"))
                unique = {name: NESTED.match(name).group(1) for name in fixed if NESTED.match(name) and bases[NESTED.match(name).group(1)] == 1}
                order = [unique.get(name, name) for name in order]
                fixed -= set(unique)
            if any(name.startswith("$T") for name in fixed):
                entry.note("slots", "a temporary among the named locals", pads, self.ahead)
                continue
            orders[symbol] = (order, fixed, scopes)
        if not orders:
            return texts, plans
        extras = sorted({n for _, fixed, _ in orders.values() for n in fixed} - set(FIXED))
        key = (unit.language, unit.compiler, tuple(extras))
        if key not in self.ladders:
            self.ladders[key] = ladder(self.compiler, self.target, unit, extras)
        entries = self.ladders[key]
        for symbol, (order, fixed, scopes) in orders.items():
            entry, annotation = pending[symbol], compared[symbol][2]
            path = Path(annotation.filename)
            text = texts.get(path.name) or path.read_text(encoding="utf-8")
            readings = declaration_orders(path.read_text(encoding="utf-8"), annotation.line_number, annotation.end_line, set(order))
            declared = readings[0]
            names, reason = assign(order, entries, declared, fixed)
            entry.alternatives = []
            if names is None:
                # Sharing follows the declarations: each reading of them (the #if group the target
                # compiles isn't known here) gives its own.
                greedy = [n for n in (assign(order, entries, reading, fixed, share=True)[0] for reading in readings) if n is not None]
                names, reason = assign(order, entries, declared, fixed, share=True)
                if names is None and greedy:
                    names = greedy[0]
                others = [n for n in greedy if n != names]
                if not fixed and not scopes:
                    # The greedy sharing may not be the compiler's: the others, fewest shares first.
                    others += [n for reading in readings for n in shared_plans(order, entries, reading, SHARED_PLANS) if n != names and n not in others]
                    if names is None and others:
                        names, others = others[0], others[1:]
                    others = others[:SHARED_PLANS]
                entry.alternatives = [(path, annotation, n) for n in others]
            if names is not None and scopes:
                inner, reason = assign_scopes(scopes, entries, set(names.values()))
                names = None if inner is None else dict(names, **inner)
            if names is None:
                entry.note("slots", reason, pads, self.ahead)
                continue
            plans[symbol] = names
            texts[path.name] = rename(text, annotation.line_number, annotation.end_line, names)
        return texts, plans

    def run(self):
        units = [u for u in self.units if not self.args.unit or u.name in self.args.unit]
        base = {r.unit.name: r for r in self.compiler.run(self.target, [Request(u, ROOT, "base", listing=True) for u in units])}
        pending = {}  # unit name -> {symbol: Entry}
        exact = defaultdict(set)
        for unit in units:
            compared = self.compare(base[unit.name])
            if isinstance(compared, str):
                print("%s: %s" % (unit.name, compared))
                continue
            for symbol, (comparison, _, annotation) in compared.items():
                name = symbol_name(symbol)
                if self.args.function and name not in self.args.function:
                    continue
                entry = Entry(unit, name, annotation)
                entry.unverified = comparison.unverified
                self.entries.append(entry)
                if comparison.exact:
                    entry.status = "exact"
                    exact[unit.name].add(symbol)
                else:
                    entry.note("other" if comparison.difference else "slots", comparison.difference or "", 0)
                    pending.setdefault(unit.name, {})[symbol] = entry
        by_name = {u.name: u for u in units}
        for pads in range(0, self.args.pads + 1):
            todo = [by_name[n] for n, entries in pending.items() if entries]
            if not todo:
                break
            self.stage([(u, pads, ()) for u in todo], base, pending, exact)
        if self.args.recipe:
            # Recipes given by hand (found another way): checked like the search's own.
            jobs = []
            for text in self.args.recipe:
                record = read_recipe(text)
                jobs += [(u, record["pads"], record["early"]) for n, u in by_name.items() if pending.get(n)]
            self.stage(jobs, base, pending, exact, slots_only=True)
        if self.args.early:
            self.early_stages(by_name, base, pending, exact)
        self.compiler.discard(base.values())

    def stage(self, jobs, base, pending, exact, slots_only=False):
        """One round of compiles: each job (unit, pads, early) is the unit with that declared
        ahead of it. A pending function exact there is witnessed; one that differs only in slots is
        renamed, compiled again, and witnessed if that gives the original. Returns the jobs that
        didn't compile."""
        ahead = [job for job in jobs if job[1] or job[2]]
        padded = self.compiler.run(self.target, [Request(u, ROOT, "pad", prefix=pads_text(pads, early), listing=True) for u, pads, early in ahead])
        compiled = dict(zip(ahead, padded))
        failed, requests = [], []
        for job in jobs:
            unit, pads, early = job
            self.pads, self.ahead = pads, early
            if not pending.get(unit.name):
                continue
            compared = self.compare(compiled[job] if job in compiled else base[unit.name], pending[unit.name])
            if isinstance(compared, str):
                failed.append(job)
                continue
            for symbol, entry in list(pending[unit.name].items()):
                if job in compiled and symbol in compared and compared[symbol][0].exact:
                    entry.status, entry.pads, entry.early = "witness", pads, early
                    entry.unverified, entry.detail = compared[symbol][0].unverified, ""
                    del pending[unit.name][symbol]
            left = {s: e for s, e in pending[unit.name].items() if s in compared}
            if slots_only and left:
                # Renaming can't settle an operand order: only the functions the audit leaves with
                # their slots to map are worth the compiles (and the bucket sharings after them).
                steps = self.rewrites(compiled[job] if job in compiled else base[unit.name], {s: compared[s] for s in left})
                left = {s: e for s, e in left.items() if steps.get(s) == []}
            texts, plans = self.plan(unit, compared, left)
            if texts:
                request = Request(unit, ROOT, "rename", texts, prefix=pads_text(pads, early), listing=True)
                request.job, request.plans = job, plans
                requests.append(request)
        self.compiler.run(self.target, requests)
        alternatives = []
        for request in requests:
            unit, pads, early = request.job
            self.ahead = early
            compared = self.compare(request)
            if isinstance(compared, str):
                for symbol in request.plans:
                    if symbol in pending[unit.name]:
                        pending[unit.name][symbol].note("slots", "renamed: " + compared, pads, self.ahead)
                continue
            for symbol in exact[unit.name]:
                if not pads and not early and symbol in compared and not compared[symbol][0].exact:
                    print("warning: %s: renaming other functions' locals changed %s" % (unit.name, symbol))
            for symbol, names in request.plans.items():
                entry = pending[unit.name].get(symbol)
                if entry is None:  # witnessed by an earlier job of this round
                    continue
                comparison = compared[symbol][0] if symbol in compared else None
                if comparison is not None and comparison.exact:
                    entry.status, entry.renames, entry.unverified = "witness", names, comparison.unverified
                    entry.pads, entry.early, entry.detail = pads, early, ""
                    del pending[unit.name][symbol]
                else:
                    entry.note("slots", "renamed, still differs" + (": " + comparison.difference if comparison is not None and comparison.difference else ""), pads, self.ahead)
                    if getattr(entry, "alternatives", None):
                        for path, annotation, names in entry.alternatives:
                            text = rename(path.read_text(encoding="utf-8"), annotation.line_number, annotation.end_line, names)
                            alternative = Request(unit, ROOT, "share", {path.name: text}, prefix=pads_text(pads, early), listing=True)
                            alternative.entry, alternative.symbol, alternative.names, alternative.job = entry, symbol, names, request.job
                            alternatives.append(alternative)
        # The other bucket sharings of the functions whose renaming didn't give the original.
        self.compiler.run(self.target, alternatives)
        for alternative in alternatives:
            entry = alternative.entry
            if entry.status == "witness":
                continue
            compared = self.compare(alternative)
            if isinstance(compared, str) or alternative.symbol not in compared or not compared[alternative.symbol][0].exact:
                continue
            comparison = compared[alternative.symbol][0]
            _, pads, early = alternative.job
            entry.status, entry.renames, entry.unverified, entry.detail = "witness", alternative.names, comparison.unverified, ""
            entry.pads, entry.early = pads, early
            pending[alternative.unit.name].pop(alternative.symbol, None)
        self.compiler.discard(alternatives)
        self.compiler.discard(requests)
        self.compiler.discard(padded)
        return failed

    def named_globals(self, entry):
        """The globals a function's body names."""
        lines = Path(entry.annotation.filename).read_text(encoding="utf-8").split("\n")
        body = "\n".join(lines[entry.annotation.line_number - 1 : entry.annotation.end_line])
        return {m.group(0) for m in TOKEN.finditer(body) if m.group(0).startswith("g_")}

    def rewrites(self, request, compared):
        """{symbol: the rewrites other than the slots' that tools/audit.py needs to turn the function
        of one compile into the original's (their instructions before)}, for the functions it explains;
        an empty list is a function that differs in its slots alone."""
        from audit import Lifted, classify, exit_reads  # audit imports this module

        obj = Object(request.obj)
        addresses = sorted(self.annotations.addresses)
        out = {}
        for symbol, (comparison, local_list, annotation) in compared.items():
            later = [a for a in addresses if a > annotation.offset]
            lifted = Lifted(obj.functions[symbol], obj, self.image, annotation.offset, self.annotations, later[0] if later else None)
            lifted.exit_reads = exit_reads(annotation)
            steps = []
            try:
                _, difference = classify(lifted, local_list, False, steps)
            except Exception:  # the finder may fail
                continue
            if not difference:
                out[symbol] = [before for kind, _, before, _ in steps if kind != "slots"]
        return out

    def flipped_globals(self, unit, request, entries):
        """{symbol: the globals in the operand-order rewrites tools/audit.py needs for the function, in
        one compile}: the ones whose position matters. Empty where the audit explains nothing."""
        compared = self.compare(request, entries)
        if isinstance(compared, str):
            return {}
        named = {}
        for name, places in self.annotations.names.items():
            for address in places:
                named.setdefault(address, name)
        starts = sorted(named)
        out = {}
        for symbol, steps in self.rewrites(request, compared).items():
            found = set()
            for before in steps:
                for text in before:
                    for address in re.findall(r"addr:(0x[0-9a-f]+)", text):
                        at = bisect.bisect_right(starts, int(address, 16)) - 1
                        if at >= 0 and named[starts[at]].startswith("g_"):  # the global the address is in
                            found.add(named[starts[at]])
            out[symbol] = found
        return out

    def early_stages(self, by_name, base, pending, exact):
        """For the functions padding left: a global the function names redeclared ahead of the unit,
        with up to --early typedefs before it and after it. Padding shifts every global of a unit at
        once, this one alone, so it reaches the functions whose comparisons with different globals
        need different paddings. The globals in the operand-order rewrites tools/audit.py finds come
        first. Unoptimized units only."""
        table = Globals(self.target_name)
        limit = self.args.early
        queues = {}  # unit name -> [(the globals moved, job's early)], the nearest first
        names = {}
        for unit_name, entries in pending.items():
            if not entries or optimized(by_name[unit_name]):
                continue
            unit = by_name[unit_name]
            flipped = self.flipped_globals(unit, base[unit_name], entries)
            names[unit_name] = {symbol: self.named_globals(entry) for symbol, entry in entries.items()}
            first = set().union(set(), *(flipped.get(s, set()) & names[unit_name][s] for s in entries))
            every = set().union(*names[unit_name].values())
            spellings = [(g, sp) for g in sorted(first) + sorted(every - first) for sp in table.spellings(g)]
            # Each spelling alone first: the ones the compiler rejects (another edition's type, a
            # static) drop out.
            failed = self.stage([(unit, 0, ((sp, 0),)) for _, sp in spellings], base, pending, exact, slots_only=True)
            bad = {job[2][0][0] for job in failed}
            spellings = [(g, sp) for g, sp in spellings if sp not in bad]
            near = sorted((p for p in itertools.product(range(limit + 1), repeat=2) if any(p)), key=lambda p: (sum(p), p))
            singles = lambda wanted: [(g, a, ((sp, b),)) for a, b in near for g, sp in spellings if g in wanted]
            queue = singles(first) + singles(every - first)
            queues[unit_name] = queue
        size = max(1, self.compiler.jobs * 4)
        while True:
            for unit_name, queue in queues.items():
                # A candidate serves the pending functions that name its global.
                left = set().union(set(), *(names[unit_name][s] for s in pending[unit_name]))
                queue[:] = [c for c in queue if c[0] in left]
            live = [n for n, queue in queues.items() if queue]
            if not live:
                break
            share = max(1, size // len(live))
            jobs = []
            for unit_name in live:
                take, queues[unit_name][:] = queues[unit_name][:share], queues[unit_name][share:]
                jobs += [(by_name[unit_name], a, early) for _, a, early in take]
            self.stage(jobs, base, pending, exact, slots_only=True)


BASE_TYPES = {"char", "short", "int", "long", "float", "double", "signed", "unsigned"}
SIMPLE_TYPEDEF = re.compile(r"^\s*(?:typedef\s+([\w ]+?)\s+(\w+)\s*;|#\s*define\s+(\w+)\s+([A-Za-z_][\w ]*?))\s*(?://.*|/\*.*)?$")
GLOBAL_DECLARATION = re.compile(r"^\s*(?:extern\s+)?(?:struct\s+)?([A-Za-z_]\w*(?:\s+[A-Za-z_]\w*)*?)\s*(\**)\s*(g_\w+)\s*((?:\[[^\]]*\])*)\s*(?:=|;)")


class Globals:
    """How to redeclare a target's globals ahead of a unit, before any header: each one's type in
    base types and struct tags, from the declarations in the target's sources. A type whose spelling
    depends on the edition has several; the compiler rejects the wrong ones."""

    def __init__(self, target):
        self.aliases, self.declared = defaultdict(set), defaultdict(set)
        files = [p for p in sorted((ROOT / "util").glob("*.h"))]
        for root in source_roots(target):
            files += [p for p in sorted((ROOT / root).rglob("*")) if p.suffix in (".h", ".c", ".cpp")]
        for path in files:
            for line in path.read_text(encoding="utf-8", errors="replace").split("\n"):
                m = SIMPLE_TYPEDEF.match(line)
                if m:
                    self.aliases[m.group(2) or m.group(3)].add((m.group(1) or m.group(4)).strip())
                m = GLOBAL_DECLARATION.match(line)
                if m and m.group(1).split()[0] not in ("return", "typedef", "static", "const", "goto"):
                    self.declared[m.group(3)].add((m.group(1), m.group(2), m.group(4)))

    def base(self, text, depth=0):
        """The spellings of a type in base types, or as a struct tag when nothing defines the name."""
        words = text.split()
        if all(w in BASE_TYPES for w in words):
            return {" ".join(words)}
        if len(words) != 1 or depth > 4:
            return set()
        if words[0] in self.aliases:
            return {b for alias in self.aliases[words[0]] for b in self.base(alias, depth + 1)}
        return {"struct " + words[0]} if not depth else set()

    def spellings(self, name):
        """The record forms (`int~g_x`, `struct~Player*~g_players[]`) a global may be redeclared with."""
        out = set()
        for type_text, stars, bounds in self.declared.get(name, ()):
            dims = re.findall(r"\[([^\]]*)\]", bounds)
            if any(not re.fullmatch(r"\d*", d.strip()) for d in dims[1:]):
                continue
            if dims and not re.fullmatch(r"\d*", dims[0].strip()):
                dims[0] = ""  # a bound only the headers can spell: an incomplete array is compatible
            for base in self.base(type_text):
                out.add((base + stars + " " + name + "".join("[%s]" % d.strip() for d in dims)).replace(" ", "~"))
        return sorted(o for o in out if EARLY.match(o))


def claims(entry):
    """What the comments above the function's annotations say its diff is: 'slots' for a stack-slot
    permutation, 'other' for another explanation, '' for none."""
    lines = Path(entry.annotation.filename).read_text(encoding="utf-8").split("\n")
    at = entry.annotation.line_number - 2
    text = []
    while at >= 0 and lines[at].lstrip().startswith("//"):
        text.append(lines[at])
        at -= 1
    comment = " ".join(line for line in text if not re.match(r"\s*// (FUNCTION|STUB|LIBRARY|SYNTHETIC|GLOBAL):", line))
    if re.search(r"slot", comment, re.I):
        return "slots"
    if re.search(r"operand order|register|matches but|only diff|differs|scheduling|entropy", comment, re.I):
        return "other"
    return ""


def write_records(path, target, entries, searched, early=True):
    """Replace the records of the searched functions, keep the others, and (a search without --early)
    the records that redeclare a global, which it couldn't have found again: certify checks them."""
    kept = [r for r in read_records(path) if (r["unit"], r["address"]) not in searched or not early and r["early"]]
    lines = {r["address"]: "%#010x %s %s %s" % (r["address"], r["name"], r["unit"], recipe(r)) for r in kept}
    for e in entries:
        if e.status == "exact":
            lines.pop(e.address, None)
        elif e.status == "witness":
            lines[e.address] = "%#010x %s %s %s" % (e.address, e.name, e.unit.name, e.recipe())
    header = ["# %s: witnesses that a function differs from the original only by compiler entropy." % target,
              "# address, function, unit, then the recipe: `pad N` symbols declared ahead of the unit, `early DECL M`",
              "# one global redeclared after them (spaces as ~) and M more symbols, and/or `rename old=new ...` of",
              "# the function's locals (`old@K`: inside the function's K-th brace block). Found by",
              "# tools/witness.py, checked by tools/certify.py.", ""]
    path.parent.mkdir(exist_ok=True)
    path.write_text("\n".join(header + [lines[a] for a in sorted(lines)]) + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--target", required=True, choices=sorted(TARGETS))
    parser.add_argument("--build", default="build-wine" if (ROOT / "build-wine").exists() else "build", help="The build tree, relative to the repository")
    parser.add_argument("--original", help="The original binary (default: reccmp-user.yml's)")
    parser.add_argument("--wine", default=os.environ.get("WINE", "wine"), help="Wine, off Windows")
    parser.add_argument("--unit", nargs="*", help="Only these units (ai.c)")
    parser.add_argument("--function", nargs="*", help="Only these functions")
    parser.add_argument("--pads", type=int, default=0, help="Also try up to N symbols declared ahead of the unit")
    parser.add_argument("--early", type=int, default=0, help="Also try each global a function names redeclared ahead of the unit, among up to N typedefs before and after it")
    parser.add_argument("--recipe", action="append", default=[], help="A padding to try first, as a record spells it: 'pad 4 early int~g_playerCount 4'")
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4)
    parser.add_argument("--write", action="store_true", help="Record the witnesses in tools/witnesses/<TARGET>.txt")
    parser.add_argument("--json", help="Write every function's result to this file")
    parser.add_argument("-v", "--verbose", action="store_true", help="List every function that isn't exact")
    args = parser.parse_args()

    search = Search(args, args.target)
    search.run()
    entries = sorted(search.entries, key=lambda e: e.address)
    counts = Counter(e.status for e in entries)
    if args.verbose:
        for entry in entries:
            if entry.status != "exact":
                print("%#010x %-8s %-32s %s" % (entry.address, entry.status, entry.name, entry.recipe() or entry.detail))
    print("%s: %d functions in %d units (%d compiles)" % (args.target, len(entries), len({e.unit.name for e in entries}), search.compiler.count))
    for status in ("exact", "witness", "slots", "other"):
        print("  %-8s %d" % (status, counts[status]))
    said = Counter((claims(e), e.status) for e in entries if e.status != "exact")
    for claim in ("slots", "other", ""):
        row = {s: said[(claim, s)] for s in ("witness", "slots", "other")}
        if sum(row.values()):
            print("  comment says %-6s %s" % ((claim or "nothing") + ":", ", ".join("%s %d" % item for item in row.items())))
    unverified = Counter(name for e in entries if e.status in ("exact", "witness") for name in e.unverified)
    if unverified:
        print("  relocation targets no annotation names (unverified): %d symbols, %d uses" % (len(unverified), sum(unverified.values())))
        if args.verbose:
            print("    " + ", ".join("%s (%d)" % item for item in unverified.most_common(20)))
    if args.json:
        Path(args.json).write_text(json.dumps([
            {"address": "%#010x" % e.address, "name": e.name, "unit": e.unit.name, "status": e.status, "detail": e.detail,
             "pads": e.pads, "early": [list(x) for x in e.early], "renames": e.renames, "unverified": e.unverified, "claim": claims(e) if e.status != "exact" else ""}
            for e in entries], indent=1) + "\n")
    if args.write:
        record_file = ROOT / "tools" / "witnesses" / (args.target + ".txt")
        write_records(record_file, args.target, entries, {(e.unit.name, e.address) for e in entries}, bool(args.early))
        print("wrote %s" % record_file.relative_to(ROOT))
    return 0


if __name__ == "__main__":
    sys.exit(main())
