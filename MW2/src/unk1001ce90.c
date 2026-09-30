#include "unk1001ce90.h"

#include "classentry.h"
#include "decomp.h"
#include "eyepoint.h"
#include "fixedmul.h"
#include "object.h"
#include "players.h"
#include "rendertarget.h"
#include "simmain.h"
#include "staticmem.h"
#include "types.h"
#include "unk1003a530.h"
#include "unk100563d0.h"
#include "unk1006d680.h"
#include "unk1007f140.h"

DECOMP_SIZE_ASSERT(ClassEntry, 0x44)

// A player's model level as FUN_1001da44 chooses it.
// SIZE 0x0c
typedef struct PlayerDetail {
	MechS32 m_distance; // 0x00 — from the eyepoint
	MechS32 m_level;    // 0x04 — -2 until chosen by distance
	MechS32 m_player;   // 0x08
} PlayerDetail;

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

// Chooses each player's model level (Player::m_unk0x1c) by its distance from the eyepoint: the
// nearest player within range gets level 0, the next two level 1, the rest 2 or 3 by distance.
// The local player's own view (FUN_10011440 == 0) takes level 4, and dead players 0 or 1.
// Stack-slot permutation; i == g_localPlayerId compares in the other operand order.
// FUNCTION: MW2 0x1001da44
void FUN_1001da44(void)
{
	MechS32 scale;
	MechS32 third;
	MechS32 second;
	MechS32 range1;
	MechS32 dz;
	MechS32 best;
	MechS32 unk0x18;
	MechS32 level;
	MechS32 range2;
	PlayerDetail* entry;
	MechS32 ex;
	MechS32 range3;
	MechS32 ground;
	MechS32 heading;
	MechS32 ey;
	MechS32 range0;
	MechS32 ez;
	MechS32 count;
	MechS32 length;
	PlayerDetail entries[60];
	MechS32 i;
	MechS32 dx;
	MechS32 nearest;
	Player* player;
	MechS32 dy;

	count = 0;
	nearest = -1;
	second = -1;
	third = -1;
	scale = g_eyepoint->m_unk0xb8;
	range0 = FixedMul16(scale, 0xe10);
	range1 = FixedMul16(scale, 0x2134);
	range2 = FixedMul16(scale, 0x57e4);
	range3 = FixedMul16(scale, 40000);
	best = range0;
	ex = g_eyepoint->m_unk0x00;
	ey = g_eyepoint->m_unk0x04;
	ez = g_eyepoint->m_unk0x08;
	for (i = 0; i < g_playerCount; i++) {
		entry = &entries[i];
		entry->m_player = i;
		player = g_players[i];
		if (i == g_localPlayerId && !FUN_10011440()) {
			entry->m_level = 4;
			entry->m_distance = 0;
		}
		else if (player->m_flags & 2) {
			if (i == g_localPlayerId) {
				entry->m_level = 0;
			}
			else {
				entry->m_level = 1;
			}
		}
		else {
			entry->m_level = -2;
			dx = player->m_position.m_x - ex;
			dy = player->m_position.m_y - ey;
			dz = player->m_position.m_z - ez;
			FUN_10060197(dx, dy, dz, &heading, &length, (MechU32*) &ground, &unk0x18);
			entry->m_distance = length;
			if (ground < best) {
				best = ground;
				third = second;
				second = nearest;
				nearest = i;
			}
		}
	}

	for (i = 0; i < g_playerCount; i++) {
		entry = &entries[i];
		if (entry->m_level == -2) {
			if (entry->m_player == nearest && entry->m_distance < range0) {
				entry->m_level = 0;
			}
			else if (entry->m_player == second || (entry->m_player == third && entry->m_distance < range1)) {
				count++;
				entry->m_level = 1;
			}
			else if ((second == -1 || third == -1) && entry->m_distance < range1 && count < 3) {
				count++;
				entry->m_level = 1;
			}
			else if (entry->m_distance < range2) {
				entry->m_level = 2;
			}
			else {
				entry->m_level = 3;
			}
		}

		player = g_players[entry->m_player];
		level = player->m_unk0x1c;
		if (entry->m_level >= 0 && entry->m_level != level && FUN_1001d292(player->m_index, entry->m_level)) {
			player->m_unk0x1c = entry->m_level;
		}
	}
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
