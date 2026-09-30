#include "rendertarget.h"

#include "compat.h"
#include "decomp.h"
#include "fixeddiv29.h"
#include "gamething.h"
#include "geocache.h"
#include "mech.h"
#include "object.h"
#include "players.h"
#include "playersteering.h"
#include "simmain.h"
#include "soundfx.h"
#include "team.h"
#include "types.h"
#include "unk1003a530.h"
#include "unk100696c0.h"
#include "weapons.h"

#include <stdlib.h>

DECOMP_SIZE_ASSERT(PixelBuffer, 0x14)
DECOMP_SIZE_ASSERT(RenderTarget, 0x14)
DECOMP_SIZE_ASSERT(NavPoint, 0x54)

// GLOBAL: MW2 0x100aaba4
MechS32 g_navCount = 0;

// What claiming the local player's target did (FUN_1005fa22): 1 claimed it, 2 too far, 3 already
// claimed by the team, 0 nothing.
// GLOBAL: MW2 0x100aaba8
MechS32 g_unk0x100aaba8 = 0;

// Set when the local player's target has changed (FUN_10060010).
// GLOBAL: MW2 0x100aabac
MechS32 g_unk0x100aabac = 0;

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

// Removes nav p_nav (an AI target id, 0x100 | index) if player p_owner placed it: the later navs
// move down, and so do the players' targets, goals and navs pointing past it (not the local
// player's own target). Returns the new nav count, or -1.
// Stack-slot permutation of the locals. Operand order: the second loop test (i < g_playerCount)
// compares with i in eax in the original.
// FUNCTION: MW2 0x1005ed4f
MechS32 FUN_1005ed4f(MechU32 p_owner, MechU32 p_nav)
{
	MechS32 index;
	MechS32 i;
	Player* player;

	if (!(p_nav & 0x100)) {
		return -1;
	}

	index = p_nav & 0xff;
	if (index >= g_navCount) {
		return -1;
	}

	if (!(g_navTable[index].m_flags & 1) || g_navTable[index].m_owner != (p_owner | 0x200)) {
		return -1;
	}

	for (i = index; i < g_navCount - 1; i++) {
		g_navTable[i] = g_navTable[i + 1];
	}

	for (i = 0; i < g_playerCount; i++) {
		player = g_players[i];
		if (!player) {
			continue;
		}

		if (player->m_aiTarget & 0x100 && (player->m_aiTarget & 0xff) > index) {
			player->m_aiTarget--;
		}

		if (player->m_aiGoal & 0x100 && (player->m_aiGoal & 0xff) > index) {
			player->m_aiGoal--;
		}

		if (player->m_targetInfo.m_target & 0x100 && (player->m_targetInfo.m_target & 0xff) > index &&
			player->m_index != g_localPlayerId) {
			player->m_targetInfo.m_target--;
		}

		if (player->m_nav & 0x100 && (player->m_nav & 0xff) > index) {
			player->m_nav--;
		}
	}

	g_navCount--;
	return g_navCount;
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

// Makes nav point p_nav player p_player's target: its position, heading and distance. Returns 1,
// or a negative code: -1 and -2 for an index out of range, -9 for flags 0xe, -3 for a free nav,
// -4 when flag 0x10000 asks for a nav with flag 0x40, and -6 for a nav of another owner or team
// (flag 0x100 accepts only a nav its owner placed).
// Stack-slot permutation of the locals; p_nav >= g_navCount compares in the other operand order.
// FUNCTION: MW2 0x1005f2ae
MechS32 FUN_1005f2ae(MechU32 p_player, MechS32 p_nav, MechU32 p_flags)
{
	Player* player;
	NavPoint* nav;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 dx;
	MechS32 dy;
	MechS32 dz;
	MechS32 heading;
	MechS32 unk0x04;
	MechU32 distance;
	MechS32 unk0x18;

	if (p_nav < 0) {
		return -1;
	}

	if (p_nav >= g_navCount) {
		return -2;
	}

	if (p_flags & 0xe) {
		return -9;
	}

	player = g_players[p_player];
	nav = &g_navTable[p_nav];
	if (!nav->m_unk0x00) {
		return -3;
	}

	if ((p_flags & 0x10000) && !(nav->m_flags & 0x40)) {
		return -4;
	}

	if (nav->m_flags & 1) {
		if (nav->m_owner != (p_player | 0x200)) {
			return -6;
		}
	}
	else if (p_flags & 0x100) {
		return -6;
	}

	if (player->m_team != nav->m_team) {
		return -6;
	}

	if (nav->m_obj) {
		GetObjPosition(nav->m_obj, &nav->m_position[0], &nav->m_position[1], &nav->m_position[2]);
	}

	x = nav->m_position[0];
	y = nav->m_position[1];
	z = nav->m_position[2];
	dx = x - player->m_position.m_x;
	dy = y - player->m_position.m_y;
	dz = z - player->m_position.m_z;
	FUN_10060197(dx, dy, dz, &heading, &unk0x04, &distance, &unk0x18);
	player->m_targetInfo.m_position.m_x = x;
	player->m_targetInfo.m_position.m_y = y;
	player->m_targetInfo.m_position.m_z = z;
	player->m_targetInfo.m_heading = heading;
	player->m_targetInfo.m_unk0x04 = unk0x04;
	player->m_targetInfo.m_distance = distance;
	player->m_targetInfo.m_unk0x18 = unk0x18;
	return 1;
}

// Makes player p_index player p_player's target, like FUN_1005f2ae for a nav point. Returns 1, or
// a negative code: -1 and -2 for an index out of range, -9 for flags 0x15 or the wrong side
// (flags 0x20000 and 0x40000), -3 for a player that can't be targeted or is p_player, -4 when
// flag 0x10000 asks for a player with flag 0x40, -5 and -7 for targets the local player may not
// pick, and -8 for a player without a shape.
// Stack-slot permutation of the locals; p_player == g_localPlayerId compares in the other operand
// order.
// FUNCTION: MW2 0x1005f4ac
MechS32 FUN_1005f4ac(MechS32 p_player, MechS32 p_index, MechU32 p_flags)
{
	MechS32 breakpoint;
	Player* player;
	Player* target;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 dx;
	MechS32 dy;
	MechS32 dz;
	MechS32 heading;
	MechS32 unk0x04;
	MechU32 distance;
	MechS32 unk0x18;

	if (p_index == 1 && p_player == g_localPlayerId) {
		breakpoint = 0;
	}

	if (p_index < 0) {
		return -1;
	}

	if (p_index >= g_playerCount) {
		return -2;
	}

	if (p_flags & 0x15) {
		return -9;
	}

	player = g_players[p_player];
	target = g_players[p_index];
	if (!g_unk0x100aabac && (target->m_flags & 0x10) && p_player == g_localPlayerId) {
		return -3;
	}

	if (target->m_flags & 6) {
		return -3;
	}

	if (target->m_index == p_player) {
		return -3;
	}

	if ((p_flags & 0x10000) && !(target->m_flags & 0x40)) {
		return -4;
	}

	if ((p_flags & 0x20000) && GetPlayerSide(p_index)) {
		return -9;
	}

	if ((p_flags & 0x40000) && GetPlayerSide(p_index) != 1) {
		return -9;
	}

	if (!g_unk0x100aabac && !(target->m_flags & 0x1400) && p_player == g_localPlayerId) {
		return -5;
	}

	if ((target->m_flags & 0x800) && p_player == g_localPlayerId) {
		return -5;
	}

	if (!target->m_obj) {
		return -8;
	}

	if (!FUN_1000154d(target->m_obj)) {
		return -8;
	}

	x = target->m_position.m_x;
	y = target->m_position.m_y;
	z = target->m_position.m_z;
	dx = x - player->m_position.m_x;
	dy = y - player->m_position.m_y;
	dz = z - player->m_position.m_z;
	FUN_10060197(dx, dy, dz, &heading, &unk0x04, &distance, &unk0x18);
	if (!(target->m_flags & 0x1000) && unk0x04 > 0x2ab98 && p_player == g_localPlayerId) {
		return -7;
	}

	player->m_targetInfo.m_position.m_x = x;
	player->m_targetInfo.m_position.m_y = y;
	player->m_targetInfo.m_position.m_z = z;
	player->m_targetInfo.m_heading = heading;
	player->m_targetInfo.m_unk0x04 = unk0x04;
	player->m_targetInfo.m_distance = distance;
	player->m_targetInfo.m_unk0x18 = unk0x18;
	return 1;
}

// Makes game thing p_index player p_player's target, like FUN_1005f4ac for a player. Returns 1,
// or the same negative codes; its test of flag 0x10000 can never succeed.
// Stack-slot permutation of the locals; p_player == g_localPlayerId compares in the other operand
// order.
// FUNCTION: MW2 0x1005f798
MechS32 FUN_1005f798(MechS32 p_player, MechS32 p_index, MechU32 p_flags)
{
	Player* player;
	GameThing* thing;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 dx;
	MechS32 dy;
	MechS32 dz;
	MechS32 heading;
	MechS32 unk0x04;
	MechU32 distance;
	MechS32 unk0x18;

	if (p_index < 0) {
		return -1;
	}

	if (p_index >= g_gameThingCount) {
		return -2;
	}

	if (p_flags & 0x23) {
		return -9;
	}

	player = g_players[p_player];
	thing = &g_gameThings[p_index];
	if (thing->m_unk0x00 & 4) {
		return -3;
	}

	if (p_flags & 0x10000 & !(thing->m_unk0x00 & 0x40)) {
		return -4;
	}

	if ((p_flags & 0x20000) && FUN_1003c30e(p_index)) {
		return -9;
	}

	if ((p_flags & 0x40000) && FUN_1003c30e(p_index) != 1) {
		return -9;
	}

	if (!g_unk0x100aabac && !(thing->m_unk0x00 & 0x1400) && p_player == g_localPlayerId) {
		return -5;
	}

	if ((thing->m_unk0x00 & 0x800) && p_player == g_localPlayerId) {
		return -5;
	}

	if (!FUN_10020bdd(thing->m_unk0x04)) {
		return -8;
	}

	if (!FUN_10020c26(thing->m_unk0x04)) {
		return -8;
	}

	FUN_10020c6f(thing->m_unk0x04, &x, &y, &z);
	dx = x - player->m_position.m_x;
	dy = y - player->m_position.m_y;
	dz = z - player->m_position.m_z;
	FUN_10060197(dx, dy, dz, &heading, &unk0x04, &distance, &unk0x18);
	if (!(thing->m_unk0x00 & 0x1000) && unk0x04 > 0x2ab98 && p_player == g_localPlayerId) {
		return -7;
	}

	player->m_targetInfo.m_position.m_x = x;
	player->m_targetInfo.m_position.m_y = y;
	player->m_targetInfo.m_position.m_z = z;
	player->m_targetInfo.m_heading = heading;
	player->m_targetInfo.m_unk0x04 = unk0x04;
	player->m_targetInfo.m_distance = distance;
	player->m_targetInfo.m_unk0x18 = unk0x18;
	return 1;
}

// Revalidates p_player's target (m_targetInfo.m_target: a nav, player or game thing index) and
// updates the target info. Reaching a nav target's radius marks the nav reached by the team; with
// the claim key (steering m_unk0x3a) pressed, a player or game thing within 20000 of its splash
// radius is claimed for the team. Returns 0, flagging the target lost (0x1000), when it no longer
// qualifies.
// The only diff is a stack-slot permutation of lost, claim, index and kind.
// FUNCTION: MW2 0x1005fa22
MechS32 FUN_1005fa22(Player* p_player)
{
	MechS32 isLocal;
	MechS32 claim;
	MechS32 lost;
	MechS32 kind;
	MechS32 index;

	lost = TRUE;
	claim = 0;
	isLocal = FALSE;
	if (p_player->m_index == g_localPlayerId) {
		isLocal = TRUE;
	}

	index = p_player->m_targetInfo.m_target & 0xff;
	kind = p_player->m_targetInfo.m_target & 0xf00;
	switch (kind) {
	case 0x200:
		if (FUN_1005f4ac(p_player->m_index, index, 0) >= 0) {
			lost = FALSE;
		}
		break;
	case 0x400:
		if (FUN_1005f798(p_player->m_index, index, 0) >= 0) {
			lost = FALSE;
		}
		break;
	case 0x100:
		if (FUN_1005f2ae(p_player->m_index, index, 0) >= 0) {
			lost = FALSE;
			if (g_navTable[index].m_radius > p_player->m_targetInfo.m_distance && p_player->m_mech->m_unk0xbc != 1 &&
				p_player->m_index == g_localPlayerId) {
				if (!(g_navTable[index].m_flags & 0x20)) {
					g_navTable[index].m_flags |= 0x20;
					g_navTable[index].m_unk0x26 |= 1 << p_player->m_team;
					FUN_1007eb23(0xe7, 100, 0x40, 5, 0x50);
				}

				FUN_100602b2(p_player, 1, 0);
			}
		}
		break;
	default:
		break;
	}

	if (lost) {
		p_player->m_targetInfo.m_target |= 0x1000;
		return 0;
	}

	if (isLocal) {
		g_unk0x100aaba8 = 0;
	}

	claim = p_player->m_steering->m_unk0x3a;
	if (claim) {
		p_player->m_steering->m_unk0x3a = 0;
		switch (kind) {
		case 0x200:
			if (!(g_players[index]->m_unk0x16 & (1 << p_player->m_team))) {
				if (g_players[index]->m_mech->m_radius + 20000 > p_player->m_targetInfo.m_distance) {
					if (isLocal) {
						g_unk0x100aaba8 = 1;
					}

					g_players[index]->m_flags |= 0x20;
					g_players[index]->m_unk0x16 |= 1 << p_player->m_team;
				}
				else if (isLocal) {
					g_unk0x100aaba8 = 2;
				}
			}
			else if (isLocal) {
				g_unk0x100aaba8 = 3;
			}
			break;
		case 0x400:
			if (!(g_gameThings[index].m_unk0x02 & (1 << p_player->m_team))) {
				if (g_gameThings[index].m_unk0x10 + 20000 > p_player->m_targetInfo.m_distance) {
					if (isLocal) {
						g_unk0x100aaba8 = 1;
					}

					g_gameThings[index].m_unk0x00 |= 0x20;
					g_gameThings[index].m_unk0x02 |= 1 << p_player->m_team;
				}
				else if (isLocal) {
					g_unk0x100aaba8 = 2;
				}
			}
			else if (isLocal) {
				g_unk0x100aaba8 = 3;
			}
			break;
		default:
			break;
		}
	}

	return 1;
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

// Targets the shape the local player points at (g_unk0x100a6d34): a player's mech (0x100) or a
// game thing (0x200), when FUN_1005f4ac or FUN_1005f798 allows it. When FUN_1005fa22 rejects the
// new target, the old one comes back, with the autopilot.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10060010
void FUN_10060010(void)
{
	MechS32 autopilot;
	Player* player;
	MechS32 target;
	MechS32 previous;
	ScarletOrchid0x4c* shape;

	target = -1;
	autopilot = FALSE;
	player = g_players[g_localPlayerId];
	previous = player->m_targetInfo.m_target;
	if (player->m_mech->m_unk0xbc == 1) {
		autopilot = TRUE;
	}

	shape = g_unk0x100a6d34;
	if (shape) {
		if (shape->m_unk0x02 & 0x100) {
			if (FUN_1005f4ac(g_localPlayerId, shape->m_unk0x14, 0) >= 0) {
				target = shape->m_unk0x14 | 0x200;
			}
		}
		else if (shape->m_unk0x02 & 0x200) {
			if (FUN_1005f798(g_localPlayerId, shape->m_unk0x14, 0) >= 0) {
				target = shape->m_unk0x14 | 0x400;
			}
		}

		if (target != -1 && player->m_targetInfo.m_target != target) {
			player->m_targetInfo.m_target = target;
			if (player->m_mech->m_unk0xbc) {
				player->m_steering->m_unk0x42 = 1;
			}

			if (!FUN_1005fa22(player)) {
				player->m_targetInfo.m_target = previous;
				if (autopilot) {
					player->m_steering->m_unk0x42 = 0;
					player->m_mech->m_unk0xbc = 1;
				}
			}
		}
	}

	g_unk0x100aabac = 1;
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
