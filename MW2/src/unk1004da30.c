#include "unk1004da30.h"

#include "decomp.h"
#include "missiontable.h"
#include "objective.h"
#include "resource.h"
#include "simmain.h"
#include "starmission.h"
#include "types.h"

#include <string.h>

DECOMP_SIZE_ASSERT(MissionEntry, 0x97)

// STUB: MW2 0x1004da30
void FUN_1004da30(void* p_table)
{
	STUB(0x1004da30);
}

// Returns the objectives that wait (state 0, 1 or 7) on the event list p_name, marking them as
// waiting (7): bit j for objective j, bit 16 + i for mission table i.
// Stack-slot permutation: every local.
// FUNCTION: MW2 0x1004e35b
MechU32 FindEventList(MechChar* p_name)
{
	MechU32 unk0x04;
	MechS32 j;
	MechU8 state;
	MechS32 i;
	MechU32 lists;

	lists = 0;
	if (!*p_name) {
		return 0;
	}

	for (i = 0; i < g_objectiveCount; i++) {
		if (g_missionTables[i]) {
			for (j = 0; j < g_objectiveTable[i].m_objectiveCount; j++) {
				state = g_objectiveTable[i].m_objectives[j].m_state;
				if (state == 0 || state == 1 || state == 7) {
					unk0x04 = (MechU8) g_objectiveTable[i].m_objectives[j].m_targets[0];
					if (!_strcmpi(p_name, g_missionTables[i]->m_entries[j].m_name)) {
						lists |= 1 << j;
						lists |= (1 << i) << 16;
						g_objectiveTable[i].m_objectives[j].m_state = 7;
					}
				}
			}
		}
	}

	return lists;
}

// Adds the target p_target to the waiting objectives on the event list p_name whose type is in
// p_types.
// The objective's address scales i and j in the opposite order (index order), and stack-slot
// permutation: count, i, j and state.
// FUNCTION: MW2 0x1004e4e6
void PostEventToList(MechChar* p_name, MechS32 p_types, MechU32 p_target)
{
	MechS32 count;
	MechS32 j;
	MechU8 state;
	MechS32 i;

	for (i = 0; i < g_objectiveCount; i++) {
		if (g_missionTables[i]) {
			for (j = 0; j < 48; j++) {
				state = g_objectiveTable[i].m_objectives[j].m_state;
				if (state == 7 && !_strcmpi(p_name, g_missionTables[i]->m_entries[j].m_name) &&
					(p_types & g_objectiveTable[i].m_objectives[j].m_type) &&
					g_objectiveTable[i].m_objectives[j].m_targetCount < 40) {
					count = g_objectiveTable[i].m_objectives[j].m_targetCount;
					g_objectiveTable[i].m_objectives[j].m_targetCount++;
					g_objectiveTable[i].m_objectives[j].m_targets[count] = p_target;
					FUN_1001cde1(p_target);
				}
			}
		}
	}
}

// Walks the waiting objectives of the lists p_lists (FindEventList's bits) and does nothing
// with them.
// Stack-slot permutation: i and j.
// FUNCTION: MW2 0x1004e69c
void FlushEventLists(MechU32 p_lists)
{
	MechS32 j;
	MechS32 i;

	for (i = 0; i < g_objectiveCount; i++) {
		if (g_missionTables[i]) {
			for (j = 0; j < 48; j++) {
				if ((p_lists & (1 << j)) && (p_lists & ((1 << i) << 16)) &&
					g_objectiveTable[i].m_objectives[j].m_state == 7) {
				}
			}
		}
	}
}
