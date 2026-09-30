#include "unk1001ce90.h"

#include "classentry.h"
#include "decomp.h"
#include "object.h"
#include "players.h"
#include "staticmem.h"
#include "types.h"
#include "unk1003a530.h"
#include "unk100563d0.h"
#include "unk1006d680.h"
#include "unk1007f140.h"

DECOMP_SIZE_ASSERT(ClassEntry, 0x44)

// GLOBAL: MW2 0x100a37d4
MechS32 g_classEntryCount = 0;

// GLOBAL: MW2 0x100a37d8
MechS32 g_classTableReady = 0;

// GLOBAL: MW2 0x1012b7e0
ClassEntry g_classTable[0x30c];

// Loads the shapes of p_player's entries for its level m_unk0x18 into one pool block.
// Stack-slot permutation: count, i and buffer.
// Operand order: i < g_classEntryCount loads g_classEntryCount first in the original.
// FUNCTION: MW2 0x1001ce90
MechS32 FUN_1001ce90(Player* p_player)
{
	MechS32 i;
	MechS32 count;
	undefined* buffer;

	count = 0;
	for (i = 0; i < g_classEntryCount; i++) {
		if (g_classTable[i].m_owner == p_player->m_index) {
			count++;
		}
	}

	buffer = StaticPoolAlloc(FUN_100023a8() * count & 0xffff, g_staticPoolTags[2]);
	if (buffer == NULL) {
		return 0;
	}

	for (i = 0; i < g_classEntryCount; i++) {
		if (g_classTable[i].m_owner == p_player->m_index) {
			FUN_1001d3ff(i, p_player->m_unk0x18, buffer);
			buffer += FUN_100023a8();
			FUN_1001d912(i, p_player->m_unk0x18);
		}
	}

	return 1;
}

// Adds p_unk0x00 as level p_level of a class entry: level 0 starts a new entry, a later level
// goes to entry p_index. The unset levels above it take the same value. Returns the entry's
// index, or -1.
// Stack-slot permutation: index and i.
// FUNCTION: MW2 0x1001cf93
MechS32 FUN_1001cf93(
	MechS32 p_unk0x00,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_level,
	MechS32 p_index,
	MechS16 p_unk0x1c
)
{
	MechS32 index;
	ClassEntry* entry;
	MechS32 i;

	index = -1;
	if (!g_classTableReady) {
		ResetClassTable();
	}

	if (g_classEntryCount >= 0x30c) {
		return -1;
	}

	if (p_unk0x10 == -1) {
		p_unk0x10 = -2;
	}

	if (p_level == 0) {
		entry = &g_classTable[g_classEntryCount];
		entry->m_owner = -2;
		entry->m_unk0x04 = -1;
		entry->m_unk0x1c = p_unk0x10;
		entry->m_unk0x2c = p_unk0x04;
		entry->m_unk0x30 = p_unk0x08;
		entry->m_unk0x34 = p_unk0x0c;
		entry->m_unk0x08[p_level] = p_unk0x00;
		entry->m_shape = NULL;
		entry->m_obj = NULL;
		entry->m_unk0x20 = 0;
		entry->m_unk0x42 = 0;
		index = g_classEntryCount++;
	}
	else {
		index = p_index;
		if (index < 0) {
			return -1;
		}

		entry = &g_classTable[index];
	}

	entry->m_unk0x08[p_level] = p_unk0x00;
	entry->m_unk0x38[p_level] = p_unk0x1c;
	if (p_level < 5) {
		for (i = p_level + 1; i < 5; i++) {
			if (entry->m_unk0x08[i] == -1) {
				entry->m_unk0x08[i] = p_unk0x00;
				entry->m_unk0x38[i] = p_unk0x1c;
			}
		}
	}

	return index;
}

// Stack-slot permutation: entry and level.
// FUNCTION: MW2 0x1001d12a
void ResetClassTable(void)
{
	ClassEntry* entry;
	MechS32 i;
	MechS32 level;

	for (i = 0; i < 0x30c; i++) {
		entry = &g_classTable[i];
		entry->m_owner = -1;
		entry->m_unk0x04 = -1;
		entry->m_unk0x1c = -1;
		entry->m_shape = NULL;
		entry->m_obj = NULL;
		entry->m_unk0x20 = 0;
		entry->m_unk0x42 = 0;
		entry->m_unk0x2c = entry->m_unk0x30 = entry->m_unk0x34 = 0;
		for (level = 0; level < 5; level++) {
			entry->m_unk0x08[level] = -1;
			entry->m_unk0x38[level] = 0;
		}
	}

	g_classEntryCount = 0;
	g_classTableReady = 1;
}

// Gives the entries added since the last call (owner -2) to p_player.
// Operand order: i < g_classEntryCount loads g_classEntryCount first in the original.
// FUNCTION: MW2 0x1001d220
void FUN_1001d220(Player* p_player)
{
	MechS32 i;
	ClassEntry* entry;

	for (i = 0; i < g_classEntryCount; i++) {
		entry = &g_classTable[i];
		if (entry->m_owner == -2) {
			entry->m_owner = p_player->m_index;
			if (entry->m_unk0x1c == -1) {
				entry->m_unk0x1c = -2;
			}
		}
	}
}

// Loads the level-p_level shapes of p_owner's entries, or, for a player with flag 2, keeps the
// shape already loaded. Returns 0 when a shape doesn't load.
// Stack-slot permutation: i and result.
// FUNCTION: MW2 0x1001d292
MechS32 FUN_1001d292(MechS32 p_owner, MechS32 p_level)
{
	MechS32 i;
	ClassEntry* entry;
	MechS32 result;

	result = 1;
	if (p_level == -1) {
		return 1;
	}

	g_unk0x100bfd40 = 1;
	g_unk0x100bfd44 = 0x100;
	g_unk0x100bfd48 = p_owner;
	for (i = 0; i < g_classEntryCount; i++) {
		entry = &g_classTable[i];
		if (entry->m_owner == p_owner) {
			if ((g_players[entry->m_owner]->m_flags & 2) && !entry->m_unk0x20) {
				FUN_1006da2d(entry->m_shape);
			}
			else if (!FUN_1001d3ff(i, p_level, NULL)) {
				result = 0;
			}

			if (!entry->m_unk0x20) {
				FUN_1006d989(entry->m_shape);
			}
		}
	}

	g_unk0x100bfd40 = 0;
	return result;
}

// Releases the level-p_level shapes of p_owner's entries.
// FUNCTION: MW2 0x1001d3a4
void FUN_1001d3a4(MechS32 p_owner, MechS32 p_level)
{
	MechS32 i;

	for (i = 0; i < g_classEntryCount; i++) {
		if (g_classTable[i].m_owner == p_owner) {
			FUN_1001d912(i, p_level);
		}
	}
}

// STUB: MW2 0x1001d3ff
MechS32 FUN_1001d3ff(MechS32 p_index, MechS32 p_level, void* p_buffer)
{
	STUB(0x1001d3ff);
	return 0;
}

// Releases the shapes of the first player whose m_unk0x1c names a level, and clears it.
// Operand order: i < g_playerCount loads g_playerCount first in the original.
// FUNCTION: MW2 0x1001d88b
void FUN_1001d88b(void)
{
	MechS32 i;
	MechS32 done;

	done = FALSE;
	for (i = 0; i < g_playerCount && !done; i++) {
		if (g_players[i]->m_unk0x1c != -1) {
			done = TRUE;
			FUN_1001d3a4(i, g_players[i]->m_unk0x1c);
			g_players[i]->m_unk0x1c = -1;
		}
	}
}

// Releases entry p_index's shape if it is the one loaded for level p_level.
// FUNCTION: MW2 0x1001d912
void FUN_1001d912(MechS32 p_index, MechS32 p_level)
{
	ClassEntry* entry;

	entry = &g_classTable[p_index];
	if (entry->m_unk0x04 >= 0 && entry->m_unk0x04 == p_level && entry->m_shape) {
		FUN_1003b78b(entry->m_shape);
		entry->m_shape = NULL;
		entry->m_unk0x04 = -1;
	}
}

// FUNCTION: MW2 0x1001d980
struct AmberWillow0x7c* FUN_1001d980(MechS32 p_index)
{
	struct AmberWillow0x7c* obj;

	obj = NULL;
	if (p_index < g_classEntryCount && p_index >= 0) {
		obj = g_classTable[p_index].m_obj;
	}

	return obj;
}

// Operand order: p_index < g_classEntryCount loads p_index first in the original.
// FUNCTION: MW2 0x1001d9ca
ScarletOrchid0x4c* FUN_1001d9ca(MechS32 p_index)
{
	ScarletOrchid0x4c* shape;

	shape = NULL;
	if (p_index < g_classEntryCount && p_index >= 0) {
		shape = g_classTable[p_index].m_shape;
	}

	return shape;
}

// FUNCTION: MW2 0x1001da14
void FUN_1001da14(MechS32 p_index, MechU16 p_value)
{
	ClassEntry* entry;

	entry = &g_classTable[p_index];
	entry->m_unk0x20 = p_value;
}

// STUB: MW2 0x1001da44
void FUN_1001da44(void)
{
	STUB(0x1001da44);
}

// Releases the shape of the entry whose object is p_obj.
// Stack-slot permutation: i and entry.
// FUNCTION: MW2 0x1001ddf2
void FUN_1001ddf2(struct AmberWillow0x7c* p_obj)
{
	MechS32 i;
	ClassEntry* entry;

	for (i = 0; i < g_classEntryCount; i++) {
		entry = &g_classTable[i];
		if (entry->m_obj == p_obj) {
			if (entry->m_shape) {
				FUN_1003b78b(entry->m_shape);
			}

			entry->m_shape = NULL;
			entry->m_unk0x04 = -1;
			entry->m_unk0x42 = 1;
			break;
		}
	}
}

// Forgets the shape of the entry whose object is p_obj without releasing it.
// Operand order: i < g_classEntryCount loads g_classEntryCount first in the original.
// FUNCTION: MW2 0x1001de84
void FUN_1001de84(struct AmberWillow0x7c* p_obj)
{
	ClassEntry* entry;
	MechS32 i;

	for (i = 0; i < g_classEntryCount; i++) {
		entry = &g_classTable[i];
		if (entry->m_obj == p_obj) {
			entry->m_shape = NULL;
			entry->m_unk0x04 = -1;
			entry->m_unk0x42 = 0;
			break;
		}
	}
}
