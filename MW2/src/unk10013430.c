#include "unk10013430.h"

#include "ai.h"
#include "clock.h"
#include "decomp.h"
#include "geocache.h"
#include "object.h"
#include "players.h"
#include "ray.h"
#include "rendertarget.h"
#include "types.h"
#include "unk10034a40.h"
#include "unk1003a530.h"
#include "unk1004b5a0.h"

#include <stdlib.h>

// STUB: MW2 0x10013430
void FUN_10013430(Player* p_player, MechU16 p_target)
{
	STUB(0x10013430);
}

// STUB: MW2 0x100139e9
void FUN_100139e9(Player* p_player)
{
	STUB(0x100139e9);
}

// STUB: MW2 0x1001450e
void FUN_1001450e(Player* p_player)
{
	STUB(0x1001450e);
}

// Places a nav point for p_player where FUN_100147d0 puts it, and makes it the player's target.
// The only diff is a stack-slot permutation of x, y, z and nav.
// FUNCTION: MW2 0x10014723
void FUN_10014723(Player* p_player, MechU32 p_unk0x04, MechS16 p_unk0x08, MechS16 p_unk0x0c)
{
	MechS32 z;
	MechS32 y;
	MechS32 x;
	MechS32 nav;

	FUN_100147d0(p_unk0x04, p_unk0x08, &x, &z, &y, p_unk0x0c);
	nav = FUN_1005ec80(p_player->m_index, x, y, z);
	if (nav != -1) {
		g_navTable[nav].m_flags |= 1;
		g_navTable[nav].m_owner = p_player->m_index | 0x200;
		FUN_10054a30(p_player, 0x100);
	}
}

// STUB: MW2 0x100147d0
void FUN_100147d0(MechU32 p_unk0x00, MechS16 p_unk0x04, MechS32* p_x, MechS32* p_z, MechS32* p_y, MechS16 p_unk0x14)
{
	STUB(0x100147d0);
}

// Whether p_turn (16.16 degrees) is a sharp turn, past 5 degrees either way; a stopped player
// then creeps forward.
// FUNCTION: MW2 0x1001498c
MechS32 FUN_1001498c(Player* p_player, MechS32 p_turn)
{
	MechS32 sharp;

	sharp = 0;
	if (p_turn > 0x50000 || p_turn < -0x50000) {
		sharp = 1;
		if (!p_player->m_steering->m_throttle) {
			p_player->m_steering->m_throttle = 0x66;
		}
	}

	return sharp;
}

// FUNCTION: MW2 0x100149e7
void FUN_100149e7(Player* p_player, MechS16 p_target)
{
	MechS32 range;
	MechS32 heading;

	FUN_1005372c(p_player, p_target);
	if (!FUN_10015709(p_player)) {
		range = 15000;
		p_player->m_steering->m_throttle = FUN_10053811(p_player, range);
		heading = FUN_1005398f(p_player);
		FUN_1001498c(p_player, heading);
	}
	else {
		heading = FUN_1005432f(p_player);
	}

	if (p_player->m_targetInfo.m_distance < 4500) {
		p_player->m_unk0x174 = 10;
	}

	FUN_1004b5a0(p_player, heading);
	FUN_100160eb(p_player);
}

// The only diff is a stack-slot permutation of heading and result.
// FUNCTION: MW2 0x10014df1
MechS32 FUN_10014df1(Player* p_player, MechS16 p_target)
{
	MechS32 heading;
	MechS32 result;

	FUN_1005372c(p_player, p_target);
	heading = FUN_1005398f(p_player);
	FUN_1004b5a0(p_player, heading);
	result = p_player->m_mech->m_unk0xa4;
	p_player->m_steering->m_throttle = 0x400;
	FUN_100160eb(p_player);
	return result;
}

// Turns p_player toward its goal and closes on its target. Whether it is within 3000.
// FUNCTION: MW2 0x10014e5e
MechS32 FUN_10014e5e(Player* p_player)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_player->m_aiGoal);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	FUN_1005372c(p_player, p_player->m_aiTarget);
	if (!FUN_10015709(p_player)) {
		p_player->m_steering->m_throttle = FUN_10053811(p_player, 3000);
		FUN_1005398f(p_player);
	}

	FUN_100160eb(p_player);
	return p_player->m_targetInfo.m_distance <= 3000;
}

// Turns p_player toward p_target. Whether the player is at least 2000 above it.
// FUNCTION: MW2 0x100150c1
MechS32 FUN_100150c1(Player* p_player, MechS16 p_target)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_target);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	if (p_player->m_position.m_y >= p_player->m_targetInfo.m_position.m_y + 2000) {
		return 1;
	}
	else {
		return 0;
	}
}

// Turns p_player toward p_target.
// FUNCTION: MW2 0x1001512e
MechS32 FUN_1001512e(Player* p_player, MechS16 p_target)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_target);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	return 0;
}

// FUNCTION: MW2 0x10015342
void FUN_10015342(Player* p_player, MechS16 p_target)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_target);
	if (!p_player->m_steering->m_unk0x2f) {
		if (!FUN_10015709(p_player)) {
			heading = FUN_1005398f(p_player);
		}
		else {
			heading = FUN_1005432f(p_player);
		}
	}
	else {
		heading = FUN_1005398f(p_player);
	}

	FUN_1004b5a0(p_player, heading);
	p_player->m_steering->m_throttle = 0x400;
	FUN_100160eb(p_player);
}

// Whether p_player, targeting a player it has a clear line to, is more than 800 below it with
// m_unk0xc0 not negative.
// The only diff is a stack-slot permutation of index and below.
// FUNCTION: MW2 0x10015520
MechS32 FUN_10015520(Player* p_player)
{
	MechS16 index;
	MechS32 below;

	below = 0;
	index = p_player->m_targetInfo.m_target & 0xff;
	switch (p_player->m_targetInfo.m_target & 0xf00) {
	default:
		break;
	case 0x200:
		if (!FUN_10015e74(p_player, p_player->m_position.m_y)) {
			if (p_player->m_targetInfo.m_position.m_y - 800 > p_player->m_position.m_y &&
				p_player->m_mech->m_unk0xc0 >= 0) {
				below = 1;
			}
			else {
				below = 0;
			}
		}
		break;
	}

	return below;
}

// STUB: MW2 0x100155e1
void FUN_100155e1(Player* p_player)
{
	STUB(0x100155e1);
}

// FUNCTION: MW2 0x100156f2
void FUN_100156f2(Player* p_player, MechS8 p_value)
{
	p_player->m_steering->m_unk0x1d = p_value;
}

// STUB: MW2 0x10015709
MechS32 FUN_10015709(Player* p_player)
{
	STUB(0x10015709);
	return 0;
}

// Whether p_shape is solid ground to stand on: a flat enough face or a shape of type 0x50.
// FUNCTION: MW2 0x10015b40
MechS32 FUN_10015b40(ScarletOrchid0x4c* p_shape)
{
	if (FUN_10034db8(p_shape) && g_segmentNormalY >= 0xc41b) {
		return 1;
	}

	return (p_shape->m_unk0x02 & 0xf0) == 0x50;
}

// The heading from p_player to p_target (16.16 degrees, 0 to 360), keeping its target.
// FUNCTION: MW2 0x10015dd6
MechS32 FUN_10015dd6(Player* p_player, MechS16 p_target)
{
	MechS32 target;
	MechS32 heading;

	target = p_player->m_targetInfo.m_target;
	FUN_1005372c(p_player, p_target);
	heading = (FUN_1005432f(p_player) + 0x1680000) % 0x1680000;
	FUN_1005372c(p_player, target);
	return heading;
}

// Clamps p_value to +/- p_limit.
// FUNCTION: MW2 0x10015e34
MechS32 FUN_10015e34(MechS32 p_value, MechS32 p_limit)
{
	if (p_value > p_limit) {
		p_value = p_limit;
	}
	else if (p_value < -p_limit) {
		p_value = -p_limit;
	}

	return p_value;
}

// Whether the segment from p_player at height p_y to its target is clear, or hits the target.
// The only diff is a stack-slot permutation of ray, hit, target and flags.
// FUNCTION: MW2 0x10015e74
MechS32 FUN_10015e74(Player* p_player, MechS32 p_y)
{
	Ray ray;
	ScarletOrchid0x4c* hit;
	MechU32 target;
	MechU16 flags;

	hit = NULL;
	BuildRayFromSegment(
		&ray,
		p_player->m_position.m_x,
		p_y,
		p_player->m_position.m_z,
		p_player->m_targetInfo.m_position.m_x,
		p_player->m_targetInfo.m_position.m_y,
		p_player->m_targetInfo.m_position.m_z
	);
	BuildRayFixed(&ray);
	if (TestSegmentCollision(&ray, &hit, p_player->m_index) && hit) {
		flags = hit->m_unk0x02;
		target = p_player->m_targetInfo.m_target;
		if ((flags & 0x100) && (target & 0x200)) {
			return hit->m_unk0x14 == (target & 0xff);
		}
		else if ((flags & 0x200) && (target & 0x400)) {
			return hit->m_unk0x14 == (target & 0xff);
		}
		else {
			return 0;
		}
	}

	return 1;
}

// STUB: MW2 0x10015fa8
MechS32 FUN_10015fa8(Mech* p_mech)
{
	STUB(0x10015fa8);
	return 0;
}

// FUNCTION: MW2 0x10016057
void FUN_10016057(Player* p_player, MechS16 p_value)
{
	p_player->m_steering->m_unk0x20 = p_value;
	if (!p_value) {
		p_player->m_steering->m_unk0x21 = 1;
	}
	else {
		p_player->m_steering->m_unk0x21 = 0;
	}
}

// How fast p_player closes on its target since the last call, per tick.
// FUNCTION: MW2 0x10016093
MechS32 FUN_10016093(Player* p_player)
{
	MechS32 rate;

	if (!g_deltaTime) {
		return 0;
	}

	rate = (p_player->m_unk0x19a - p_player->m_targetInfo.m_distance) / g_deltaTime;
	p_player->m_unk0x19a = p_player->m_targetInfo.m_distance;
	return rate;
}

// FUNCTION: MW2 0x100160eb
void FUN_100160eb(Player* p_player)
{
	MechS32 turn;
	Mech* mech;

	mech = p_player->m_mech;
	if (!(p_player->m_unk0x19e & 1) || p_player->m_unk0x190 || p_player->m_unk0x170 == 10) {
		return;
	}

	FUN_1005372c(p_player, p_player->m_aiTarget);
	turn = abs(FUN_1005432f(p_player));
	if (turn >= mech->m_unk0xe0 || (turn <= -mech->m_unk0xe0 && p_player->m_steering->m_throttle)) {
		if (FUN_10016222(p_player, 20) && p_player->m_unk0x78 && !p_player->m_steering->m_unk0x1d) {
			FUN_100156f2(p_player, 1);
		}
	}
	else if (!p_player->m_unk0x196 && p_player->m_steering->m_unk0x1d && p_player->m_unk0x78) {
		FUN_100156f2(p_player, 0);
	}
}

// Whether p_player's mech can fire: m_unk0xc0 at least 6, m_unk0xec set and the heat
// (m_unk0x98, 16.16) under p_limit.
// FUNCTION: MW2 0x10016222
MechS32 FUN_10016222(Player* p_player, MechS32 p_limit)
{
	Mech* mech;

	mech = p_player->m_mech;
	return mech->m_unk0xc0 >= 6 && mech->m_unk0xec && mech->m_unk0x98 >> 16 < p_limit;
}

// The shape of the player or game thing an AI target id names, or NULL.
// The only diff is a stack-slot permutation of index, id and obj.
// FUNCTION: MW2 0x1001627f
ScarletOrchid0x4c* FUN_1001627f(MechS16 p_target)
{
	MechS16 index;
	MechS32 id;
	AmberWillow0x7c* obj;

	obj = NULL;
	index = p_target & 0xff;
	switch (p_target & 0xf00) {
	case 0x200:
		obj = g_players[index]->m_obj;
		break;
	case 0x400:
		id = g_gameThings[index].m_unk0x04;
		obj = FUN_10020bdd(id);
		break;
	}

	return obj ? obj->m_unk0x6c : NULL;
}

// STUB: MW2 0x1001632c
void FUN_1001632c(WeaponSlot* p_slot, Mech* p_mech)
{
	STUB(0x1001632c);
}
