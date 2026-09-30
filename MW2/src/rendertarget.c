#include "rendertarget.h"

#include "compat.h"
#include "decomp.h"
#include "fixeddiv29.h"
#include "geocache.h"
#include "object.h"
#include "players.h"
#include "simmain.h"
#include "types.h"
#include "unk100696c0.h"

#include <stdlib.h>

DECOMP_SIZE_ASSERT(PixelBuffer, 0x14)
DECOMP_SIZE_ASSERT(RenderTarget, 0x14)
DECOMP_SIZE_ASSERT(NavPoint, 0x54)

// GLOBAL: MW2 0x100aaba4
MechS32 g_navCount = 0;

// GLOBAL: MW2 0x10177160
NavPoint g_navTable[128];

// Places a nav point for player p_owner at (p_x, p_y, p_z), named "!". Returns its index, or
// -1 if the table is full.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1005ec80
MechS32 FUN_1005ec80(MechU32 p_owner, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 index;
	NavPoint* nav;

	index = -1;
	if (g_navCount < 0x80 && g_navCount != -1) {
		nav = &g_navTable[g_navCount];
		nav->m_position[0] = p_x;
		nav->m_position[1] = p_y;
		nav->m_position[2] = p_z;
		nav->m_unk0x00 = 1;
		nav->m_flags = 0x401;
		nav->m_owner = p_owner | 0x200;
		nav->m_team = g_players[p_owner]->m_team;
		nav->m_radius = 3000;
		nav->m_obj = NULL;
		nav->m_name[0] = '!';
		nav->m_name[1] = '\0';
		index = g_navCount;
		g_navCount++;
	}

	return index;
}

// STUB: MW2 0x1005ed4f
void FUN_1005ed4f(MechU32 p_owner, MechU32 p_nav)
{
	STUB(0x1005ed4f);
}

// STUB: MW2 0x1005ef5e
void FUN_1005ef5e(Player* p_player, MechS32 p_step, MechU32 p_flags)
{
	STUB(0x1005ef5e);
}

// Marks the local player's target (bit 0x1000).
// FUNCTION: MW2 0x1005f284
void FUN_1005f284(void)
{
	Player* player;

	player = g_players[g_localPlayerId];
	player->m_targetInfo.m_target |= 0x1000;
}

// STUB: MW2 0x1005fa22
MechS32 FUN_1005fa22(Player* p_player)
{
	STUB(0x1005fa22);
	return 0;
}

// Returns the player the local player targets, or -1.
// The only diff is a stack-slot permutation of index, player and kind.
// FUNCTION: MW2 0x1005fe63
MechS32 FUN_1005fe63(void)
{
	MechS32 index;
	Player* player;
	MechS32 kind;

	player = g_players[g_localPlayerId];
	kind = player->m_targetInfo.m_target & 0xf00;
	index = player->m_targetInfo.m_target & 0xff;
	if (kind != 0x200) {
		index = -1;
	}

	return index;
}

// Returns the game thing the local player targets, or -1.
// The only diff is a stack-slot permutation of index, player and kind.
// FUNCTION: MW2 0x1005febe
MechS32 FUN_1005febe(void)
{
	MechS32 index;
	Player* player;
	MechS32 kind;

	player = g_players[g_localPlayerId];
	kind = player->m_targetInfo.m_target & 0xf00;
	index = player->m_targetInfo.m_target & 0xff;
	if (kind != 0x400) {
		index = -1;
	}

	return index;
}

// Returns the shape of the local player's target, or NULL.
// FUNCTION: MW2 0x1005ff19
ScarletOrchid0x4c* FUN_1005ff19(void)
{
	AmberWillow0x7c* obj;

	obj = FUN_1005ff56();
	if (obj) {
		return FUN_1000154d(obj);
	}
	else {
		return NULL;
	}
}

// Returns the scene object of the local player's target: a player's or a game thing's.
// The only diff is a stack-slot permutation of index, player, obj, kind and id.
// FUNCTION: MW2 0x1005ff56
AmberWillow0x7c* FUN_1005ff56(void)
{
	MechS32 index;
	Player* player;
	AmberWillow0x7c* obj;
	MechS32 kind;
	MechS32 id;

	obj = NULL;
	player = g_players[g_localPlayerId];
	kind = player->m_targetInfo.m_target & 0xf00;
	index = player->m_targetInfo.m_target & 0xff;
	switch (kind) {
	case 0x200:
		obj = g_players[index]->m_obj;
		break;
	case 0x400:
		id = g_gameThings[index].m_unk0x04;
		obj = FUN_10020bdd(id);
		break;
	default:
		break;
	}

	return obj;
}

// Turns the vector (p_dx, p_dy, p_dz) into its heading (*p_unk0x0c), its length along the
// ground (*p_distance), its full length (*p_unk0x10) and its pitch (*p_unk0x18).
// The abs(p_dx)/abs(p_dz) comparison evaluates its operands in the opposite order (one attempt at
// swapping them didn't flip it), and stack-slot permutation: every local.
// FUNCTION: MW2 0x10060197
void FUN_10060197(
	MechS32 p_dx,
	MechS32 p_dy,
	MechS32 p_dz,
	MechS32* p_unk0x0c,
	MechS32* p_unk0x10,
	MechU32* p_distance,
	MechS32* p_unk0x18
)
{
	MechS32 pitch;
	MechS32 cosine;
	MechS32 ground;
	MechS32 heading;
	MechS32 sine;
	MechS32 length;
	MechS32 pitchCosine;

	heading = FUN_100698de(p_dx, p_dz);
	if (abs(p_dx) > abs(p_dz)) {
		sine = FUN_100696c0(heading);
		if (sine) {
			ground = FixedDiv29(p_dx, sine);
		}
		else {
			ground = 0;
		}
	}
	else {
		cosine = FUN_1006973a(heading);
		if (cosine) {
			ground = FixedDiv29(p_dz, cosine);
		}
		else {
			ground = 0;
		}
	}

	pitch = FUN_100698de(p_dy, ground);
	pitchCosine = FUN_1006973a(pitch);
	if (pitchCosine) {
		length = FixedDiv29(ground, pitchCosine);
	}
	else {
		length = 0;
	}

	*p_unk0x0c = heading;
	*p_distance = ground;
	*p_unk0x10 = length;
	*p_unk0x18 = pitch;
}

// Steps p_player's selected target by p_step, with bit 0x100 of the flags set when p_unk0x08.
// FUNCTION: MW2 0x100602b2
void FUN_100602b2(Player* p_player, MechS32 p_step, MechS32 p_unk0x08)
{
	MechU32 flags;

	flags = 1;
	if (p_unk0x08) {
		flags |= 0x100;
	}

	FUN_1005ef5e(p_player, p_step, flags);
}

// FUNCTION: MW2 0x100602ec
void FUN_100602ec(MechS32 p_step)
{
	Player* player;

	player = g_players[g_localPlayerId];
	FUN_1005ef5e(player, p_step, 4);
}

// FUNCTION: MW2 0x1006031b
void FUN_1006031b(MechS32 p_step)
{
	Player* player;

	player = g_players[g_localPlayerId];
	FUN_1005ef5e(player, p_step, 2);
}

// FUNCTION: MW2 0x1006034a
void FUN_1006034a(MechS32 p_step)
{
	Player* player;

	player = g_players[g_localPlayerId];
	FUN_1005ef5e(player, p_step, 0x20008);
}

// FUNCTION: MW2 0x1006037c
void FUN_1006037c(MechS32 p_step)
{
	Player* player;

	player = g_players[g_localPlayerId];
	FUN_1005ef5e(player, p_step, 0x40008);
}

// Selects the local player's nearest target within 0x2ab98, cycling through them all; keeps the
// current one if there is none or FUN_1005fa22 refuses it, and then clears the autopilot's
// steering flag (PlayerSteering::m_unk0x42).
// The distance/bestDistance comparison loads its operands in the opposite order (one attempt at
// swapping them didn't flip it), and stack-slot permutation: every local.
// FUNCTION: MW2 0x100603ae
void FUN_100603ae(void)
{
	MechS32 autopilot;
	MechS32 target;
	Player* player;
	MechS32 best;
	MechS32 saved;
	MechS32 distance;
	MechS32 bestDistance;

	target = 0;
	best = -1;
	bestDistance = 0x7fffffff;
	autopilot = FALSE;
	player = g_players[g_localPlayerId];
	saved = player->m_targetInfo.m_target;
	if (player->m_mech->m_unk0xbc == 1) {
		autopilot = TRUE;
	}

	FUN_1006037c(0);
	while (best != target) {
		target = player->m_targetInfo.m_target;
		if (target & 0x1000) {
			break;
		}

		distance = player->m_targetInfo.m_unk0x04;
		if (distance < bestDistance) {
			best = target;
			bestDistance = distance;
			target = 0;
		}

		FUN_1006037c(1);
	}

	if (best == -1 || bestDistance > 0x2ab98) {
		player->m_targetInfo.m_target = saved;
		if (autopilot) {
			player->m_steering->m_unk0x42 = 0;
		}
	}
	else {
		player->m_targetInfo.m_target = best;
		if (!FUN_1005fa22(player)) {
			player->m_targetInfo.m_target = saved;
			if (autopilot) {
				player->m_steering->m_unk0x42 = 0;
			}
		}
	}
}
