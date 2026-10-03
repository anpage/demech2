#!/usr/bin/env python3
"""Print the AI scripts (AIT resources) of a MechWarrior 2 resource archive (MW2.PRJ).

The simulator loads AIT resources 1 to 9 (LoadAIScripts, MW2/src/ai.c) and picks one per
objective type, for a star's leader or its followers (g_mechScripts and the other tables).
A script lists, for each AI state, the rules that apply in it; CollectAIRules reads it as:

    u16 stateCount
    stateCount x (u16 state, u16 ruleCount)
    the rules of every state in that order, 12 bytes each (AiRule, MW2/include/airule.h):
        s16 message, u16 target, s16 arg, u8 transition, s16 newState, s16 newTarget, u8 unknown

The archive's layout follows MW2/src/prjfile.c: from offset 0x0c a header with the type count
(0x18) and 0x18-byte type records (0x1a: tag, index offset, index size, ..., 0x14 the size of
each resource's own header); a type's index holds (offset, end) pairs by resource id from 0x16,
and a resource's data starts its header's size past its offset. The resource headers carry the
script's name (0x1e) and a date (0x10: minute and hour, as far as the values tell, then day,
month and a 16-bit year).

The names are the AI's own (ai.c's g_aiMessageNames, g_aiTransitionNames, g_aiStateNames,
g_aiSymbolicTargetNames and g_aiTargetTypeNames).

Usage: dump_ait.py <MW2.PRJ> [--id N]
"""

import argparse
import struct
import sys

MESSAGES = ["NO_MESSAGE", "M_PROX", "M_DIST", "M_REACH", "M_TRUE", "M_FALSE", "M_DESTROY", "M_TGTABLE"]
TRANSITIONS = ["T_NULL", "T_CLEAR_STACK", "T_PUSH", "T_POP", "T_NOTIFY", "T_EVALUATE"]
STATES = {
    0: "idle",
    1: "avoid",
    2: "target",
    3: "attack",
    4: "flee",
    5: "follow",
    6: "recon",
    7: "patrol",
    8: "godirect",
    9: "state9",
    10: "rest",
    11: "shutdown",
    12: "dead",
    -1: "none",
}
SYMBOLIC = {
    0x2200: "agp_user",
    0x2201: "agp_myleader",
    0x2202: "agp_me",
    0x2100: "agp_home",
    0x2101: "agp_rbanchor",
    0x4200: "agp_friendly",
    0x4201: "agp_enemy",
}
TYPES = {0x100: "nv", 0x200: "gp", 0x400: "gt"}
# ResolveRuleTarget's placeholders
PLACEHOLDERS = {ord("@"): "@goal", ord("%"): "%found", ord("*"): "*target"}


def target_name(value):
    value &= 0xFFFF
    if value in PLACEHOLDERS:
        return PLACEHOLDERS[value]
    if value in SYMBOLIC:
        return SYMBOLIC[value]
    if value == 0:
        return "0"
    kind = value & 0xF00
    if kind in TYPES and not value & 0xF000:
        return "%s%d" % (TYPES[kind], value & 0xFF)
    return "0x%04x" % value


def state_name(value):
    return STATES.get(value, "state%d" % value)


def read_ait(data):
    """Yields (resource id, header bytes, data bytes) for every AIT resource."""
    type_count = struct.unpack_from("<H", data, 0x18)[0]
    for i in range(type_count):
        record = 0x1A + i * 0x18
        tag = data[record:record + 4].rstrip(b"\0 ")
        index_offset, index_size = struct.unpack_from("<Ii", data, record + 4)
        header_size = struct.unpack_from("<H", data, record + 0x14)[0]
        if tag != b"AIT" or not index_offset:
            continue
        count = (index_size - 0x16) // 8
        for res_id in range(1, count):
            offset, end = struct.unpack_from("<ii", data, index_offset + 0x16 + res_id * 8)
            if not offset and not end:
                continue
            yield res_id, data[offset:offset + header_size], data[offset + header_size:offset + end]
        return
    sys.exit("no AIT resources in the archive")


def describe(res_id, header, script):
    name = header[0x1E:0x2E].split(b"\0")[0].decode("ascii", "replace")
    minute, hour, day, month, year = struct.unpack_from("<BBBBH", header, 0x10)
    date = "%04d-%02d-%02d %02d:%02d" % (year, month, day, hour, minute)
    print("AIT %d: %s (%d bytes, dated %s)" % (res_id, name, len(script), date))

    state_count = struct.unpack_from("<H", script, 0)[0]
    states = [struct.unpack_from("<hH", script, 2 + i * 4) for i in range(state_count)]
    pos = 2 + state_count * 4
    for state, rule_count in states:
        print("  %s:" % state_name(state))
        for _ in range(rule_count):
            print_rule(script, pos)
            pos += 12

    # Bytes past the last state's rules: no state's list reaches them, so the game never reads
    # them. They look like what an earlier, longer version of the script left in the compiler's
    # buffer: partly whole rules, partly garbage.
    if pos < len(script):
        print("  unreferenced (%d bytes past the last state's rules):" % (len(script) - pos))
        while pos + 12 <= len(script):
            print_rule(script, pos)
            pos += 12
        if pos < len(script):
            print("    (%d bytes left)" % (len(script) - pos))
    print()


def print_rule(script, pos):
    message, target, arg, transition, new_state, new_target, unknown = struct.unpack_from("<hHhBhhB", script, pos)
    message_name = MESSAGES[message] if 0 <= message < len(MESSAGES) else "message%d" % message
    transition_name = TRANSITIONS[transition] if transition < len(TRANSITIONS) else "transition%d" % transition
    # The arg is a distance in hundreds, a symbolic range (-1 to -4: the player's own) or a target
    line = "    %-10s %-14s arg %-6s -> %-13s %-9s %s" % (
        message_name,
        target_name(target),
        target_name(arg) if arg in PLACEHOLDERS else str(arg),
        transition_name,
        state_name(new_state),
        target_name(new_target),
    )
    if unknown:
        line += "  (byte 0x0b: %d)" % unknown
    print(line.rstrip())


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("prj", help="the resource archive (MW2.PRJ)")
    parser.add_argument("--id", type=int, help="only this AIT resource")
    args = parser.parse_args()

    with open(args.prj, "rb") as f:
        data = f.read()
    if data[:4].upper() != b"PROJ":
        sys.exit("%s is not a PROJ file" % args.prj)

    for res_id, header, script in read_ait(data):
        if args.id is None or args.id == res_id:
            describe(res_id, header, script)
    return 0


if __name__ == "__main__":
    sys.exit(main())
