/* Reloading a player's mech: RememberLoadMech and RememberMechSegments save how it was
   loaded and its scene objects, and FUN_1007fbe0 restores both. */
#include "mechreload.h"

#include "classtable.h"
#include "config.h"
#include "debris.h"
#include "debugprint.h"
#include "decomp.h"
#include "gpanim.h"
#include "mech.h"
#include "mechclass.h"
#include "mekfile.h"
#include "network.h"
#include "object.h"
#include "objective.h"
#include "players.h"
#include "rememberedmech.h"
#include "simmain.h"
#include "types.h"

#include <string.h>
#include <windows.h>

// The player whose mech FUN_1007fbe0 is reloading, or -1.
// GLOBAL: MW2 0x100ba690
MechS32 g_reloadingPlayer = -1;

// GLOBAL: MW2 0x100ba694
MechS32 g_unk0x100ba694 = 0;

// GLOBAL: MW2 0x100ba698
RememberedMech g_rememberedMechs[60] = {0};

// GLOBAL: MW2 0x100babc0
MechSegment* g_mechSegments[60] = {NULL};

// Reloads player p_player's mech as it was remembered. Without p_force, only a player with
// flag 2 set.
// FUNCTION: MW2 0x1007fbe0
MechS32 FUN_1007fbe0(MechS32 p_player, MechS32 p_force)
{
	Mech* mech;
	MechS16 flags;

	mech = NULL;
	if (p_player >= 0 && p_player < 60) {
		mech = g_players[p_player]->m_mech;
	}

	if (!mech) {
		return 0;
	}

	flags = g_players[p_player]->m_flags;
	if (!p_force && (!(flags & 2) || !(flags & 2))) {
		return 0;
	}

	mech->m_unk0x24.m_target = mech->m_unk0x24.m_value = 0;
	mech->m_unk0x04.m_target = mech->m_unk0x04.m_value = 0;
	mech->m_unk0x34.m_target = mech->m_unk0x34.m_value = 0;
	mech->m_unk0x14.m_target = mech->m_unk0x14.m_value = 0;
	mech->m_player->m_steering->m_throttle = 0;
	mech->m_player->m_steering->m_unk0x24 = 1;
	mech->m_player->m_steering->m_unk0x1d = 0;
	mech->m_player->m_steering->m_unk0x1e = 0;
	mech->m_player->m_steering->m_unk0x1f = 0;
	mech->m_player->m_steering->m_unk0x20 = 0;
	mech->m_player->m_steering->m_unk0x21 = 0;
	mech->m_player->m_steering->m_unk0x25 = 0;
	mech->m_player->m_steering->m_unk0x26 = 0;
	mech->m_player->m_steering->m_unk0x2d = 0;
	mech->m_player->m_steering->m_unk0x2e = 0;
	mech->m_player->m_steering->m_unk0x2f = 0;
	mech->m_player->m_steering->m_unk0x30 = 0;
	mech->m_player->m_steering->m_unk0x42 = 0;

	g_reloadingPlayer = p_player;
	FUN_10019881(mech);
	LoadMechConfig(
		mech,
		g_rememberedMechs[p_player].m_name,
		g_rememberedMechs[p_player].m_unk0x00,
		g_rememberedMechs[p_player].m_unk0x0d
	);
	mech->m_player->m_obj = RestoreMechSegments(g_mechSegments[p_player]);
	FUN_10001926(mech->m_player->m_obj);
	UpdateObj(mech->m_player->m_obj);
	mech->m_player->m_unk0x1c = -1;
	mech->m_player->m_targetInfo.m_target = 0x1000;

	flags &= ~6;
	flags &= ~0x4001;
	flags |= 0x10;
	g_players[p_player]->m_flags = flags;

	if (g_isNetworkGame) {
		FUN_1001cc5c(p_player);
	}

	// gpanim.c's Mech is the player
	FUN_10003a10(mech->m_player);
	FUN_10016ad0(mech->m_player);

	if (g_localPlayerId == p_player) {
		FUN_1006ff7b();
		g_unk0x100a2c04 = 0;
		g_unk0x100a2c18 = 0;
		g_unk0x100a2c10 = 0;
	}

	g_reloadingPlayer = -1;
	return 1;
}

// FUNCTION: MW2 0x1007fecf
void RememberLoadMech(Mech* p_mech, MechChar* p_name, MechS32 p_unk0x08, MechChar* p_unk0x0c)
{
	MechS32 id;

	if (!p_mech || !p_mech->m_player) {
		return;
	}

	id = p_mech->m_player->m_index;
	if (id > 60) {
		DebugPrint("RememberLoadMech(): gp_id > DEFAULT_GAMEPIECES\n");
		return;
	}

	g_rememberedMechs[id].m_unk0x00 = p_unk0x08;
	strcpy(g_rememberedMechs[id].m_name, p_name);
	strcpy(g_rememberedMechs[id].m_unk0x0d, p_unk0x0c);
}

// FUNCTION: MW2 0x1007ff9c
void RememberMechSegments(Mech* p_mech)
{
	SceneObject* obj;
	MechS32 id;

	obj = NULL;
	if (!p_mech || !p_mech->m_player) {
		return;
	}

	id = p_mech->m_player->m_index;
	if (id > 60) {
		DebugPrint("RememberMechSegments(): gp_id > DEFAULT_GAMEPIECES\n");
		return;
	}

	obj = p_mech->m_player->m_obj;
	g_mechSegments[id] = SaveMechSegments(obj);
}

// FUNCTION: MW2 0x10080014
SceneObject* RestoreMechSegments(MechSegment* p_segment)
{
	SceneObject* obj;

	if (!p_segment) {
		return NULL;
	}

	obj = p_segment->m_obj;
	if (obj) {
		FUN_10004dcb(obj, FUN_1001ddf2);
		FUN_1001de84(obj);
		SetObjPosition(obj, p_segment->m_position[0], p_segment->m_position[1], p_segment->m_position[2]);
		SetObjRotation(obj, p_segment->m_rotation[0], p_segment->m_rotation[1], p_segment->m_rotation[2], 0);
		obj->m_firstChild = RestoreMechSegments(p_segment->m_firstChild);
		obj->m_nextSibling = RestoreMechSegments(p_segment->m_nextSibling);
		obj->m_parent = p_segment->m_parent;
	}

	return obj;
}

// FUNCTION: MW2 0x100800e3
MechSegment* SaveMechSegments(SceneObject* p_obj)
{
	MechSegment* segment;

	segment = NULL;
	if (!p_obj) {
		return NULL;
	}

	segment = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(MechSegment));
	if (!segment) {
		return NULL;
	}

	segment->m_obj = p_obj;
	segment->m_parent = p_obj->m_parent;
	segment->m_firstChild = SaveMechSegments(p_obj->m_firstChild);
	segment->m_nextSibling = SaveMechSegments(p_obj->m_nextSibling);
	FUN_1000160e(p_obj, &segment->m_position[0], &segment->m_position[1], &segment->m_position[2]);
	FUN_100015bc(p_obj, &segment->m_rotation[0], &segment->m_rotation[1], &segment->m_rotation[2]);

	return segment;
}
