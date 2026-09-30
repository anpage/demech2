#include "world.h"

#include "bwd.h"
#include "bwdstreamkey.h"
#include "decomp.h"
#include "error.h"
#include "gamething.h"
#include "geocache.h"
#include "network.h"
#include "object.h"
#include "players.h"
#include "resource.h"
#include "shots.h"
#include "team.h"
#include "types.h"
#include "unk1003a530.h"
#include "unk1007f140.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Widens p_flags: any of the bits 0x730 sets them all, as does either of the bits 3.
// FUNCTION: MW2 0x1000a9c0
MechU32 FUN_1000a9c0(MechU32 p_flags)
{
	if (p_flags & 0x730) {
		p_flags |= 0x730;
	}

	if (p_flags & 3) {
		p_flags |= 3;
	}

	return p_flags;
}

// Executes a BWD stream: the world's scripts, objects and things.
// STUB: MW2 0x1000a9f5
MechS32 BwdExecuteStream(BwdStream* p_stream)
{
	STUB(0x1000a9f5);
	return 0;
}

// Loads the mission world p_name (a resource number or a file name): resets the teams, the
// thing table and the shape offset, then executes its BWD stream. Returns whether it loaded.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x1000d30f
MechS32 LoadWorld(MechChar* p_name)
{
	BwdStream* stream;
	BwdStreamKey* key;
	MechS32 result;
	MechS32 i;
	BwdStreamKey keyData;
	undefined buffer[0x20];

	result = FALSE;
	ResetTeams();
	for (i = 0; i < 0x96; i++) {
		g_unk0x100ea580[i] = -1;
	}

	g_unk0x100a8620 = 0;
	g_unk0x100a8624 = 0;
	SetShapeOffset(0, 0, 0);
	key = &keyData;
	if (isdigit(*p_name)) {
		key->m_id = atoi(p_name);
	}
	else {
		key->m_id = -1;
	}

	strncpy(key->m_name, p_name, 0xc);
	key->m_name[0xc] = '\0';
	SetMangleBase(0);
	stream = OpenBwdStream(key, (BwdStream*) buffer);
	if (stream && FUN_1001f3e0()) {
		result = BwdExecuteStream(stream);
		UnloadResource(stream);
		FUN_1004fd55();
		FUN_1001f5cb();
		if (!FUN_10020684()) {
			Error(0x4b, NULL);
		}
	}

	if (!result) {
		Error(0xb, "%s", key->m_name);
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// Counts the players and game things of each side and gives each game thing the radius of its
// object's shape. In a network game, every player's team but the local one is on side 1.
// Stack-slot permutation: obj and i. Operand order: i == g_unk0x100a5918 loads
// g_unk0x100a5918 first in the original.
// FUNCTION: MW2 0x1000d4a6
void AfterWorldLoader(void)
{
	AmberWillow0x7c* obj;
	MechS32 i;

	for (i = 0; i < g_playerCount; i++) {
		if (g_isNetworkGame) {
			if (i == g_unk0x100a5918) {
				g_teams[i].m_side = 0;
			}
			else {
				g_teams[i].m_side = 1;
			}
		}

		switch (GetPlayerSide(i)) {
		case 0:
			g_carCfg.m_unk0x38[0]++;
			break;
		case 2:
			g_carCfg.m_unk0x38[1]++;
			break;
		case 1:
			g_carCfg.m_unk0x38[2]++;
			break;
		}
	}

	for (i = 0; i < g_gameThingCount; i++) {
		switch (FUN_1003c30e(i)) {
		case 0:
			g_carCfg.m_unk0x38[3]++;
			break;
		case 2:
			g_carCfg.m_unk0x38[4]++;
			break;
		case 1:
			g_carCfg.m_unk0x38[5]++;
			break;
		}

		obj = FUN_10020bdd(g_gameThings[i].m_unk0x04);
		if (obj && obj->m_unk0x6c) {
			g_gameThings[i].m_unk0x10 = obj->m_unk0x6c->m_unk0x40;
		}
	}
}
