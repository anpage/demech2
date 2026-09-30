#include "unk10013430.h"

#include "ai.h"
#include "clock.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "geocache.h"
#include "mech.h"
#include "object.h"
#include "players.h"
#include "playersteering.h"
#include "random.h"
#include "ray.h"
#include "rendertarget.h"
#include "silverbrook.h"
#include "types.h"
#include "unk10034a40.h"
#include "unk1003a530.h"
#include "unk1004b5a0.h"
#include "weaponslot.h"

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

// Returns the index of maneuver p_id in p_table, or -1.
// The original loads the index before m_entries (index order).
// FUNCTION: MW2 0x100140e4
MechS16 FUN_100140e4(SilverBrook0x08* p_table, MechS16 p_id)
{
	MechS16 i;

	for (i = 0; i < p_table->m_count; i++) {
		if (p_table->m_entries[i].m_id == p_id) {
			return i;
		}
	}

	return -1;
}

// Starts p_player's maneuver (m_unk0x170): resets its state and sets it up, with the time it
// ends (m_unk0x178).
// FUNCTION: MW2 0x10014149
void FUN_10014149(Player* p_player)
{
	p_player->m_aiGoal = p_player->m_aiTarget;
	p_player->m_unk0x17c = 0;
	p_player->m_unk0x19a = p_player->m_unk0x180 = p_player->m_unk0x184 = 0;
	p_player->m_unk0x196 = 0;
	p_player->m_unk0x178 = g_currentClock + 0x235a;
	switch (p_player->m_unk0x170) {
	case 0:
		p_player->m_unk0x178 = (RandomIntBelow(5) + 8) * 181 + g_currentClock;
		break;
	case 1:
		p_player->m_unk0x178 = g_currentClock + 0xe24;
		p_player->m_unk0x184 = -1;
		break;
	case 3:
		if (RandomIntBelow(2)) {
			p_player->m_unk0x180 = 1;
		}
		else {
			p_player->m_unk0x180 = -1;
		}
		break;
	case 4:
		p_player->m_steering->m_throttle = 0;
		p_player->m_steering->m_turn = 0;
		p_player->m_unk0x196 = 1;
		FUN_100156f2(p_player, 1);
		p_player->m_unk0x178 = g_currentClock + 0x389;
		break;
	case 11:
		p_player->m_steering->m_turn = 0;
		p_player->m_unk0x196 = 1;
		if (!p_player->m_unk0x180 || p_player->m_unk0x180 == 2) {
			p_player->m_steering->m_unk0x1e = 1;
		}
		else {
			p_player->m_steering->m_unk0x20 = 1;
		}

		FUN_100156f2(p_player, 1);
		p_player->m_unk0x178 = g_currentClock + 0x16a;
		break;
	case 5:
		FUN_10016093(p_player);
		p_player->m_unk0x196 = 1;
		break;
	case 8:
		p_player->m_unk0x178 = (RandomIntBelow(11) + 10) * 181 + g_currentClock;
		p_player->m_steering->m_unk0x2f = 1;
		break;
	case 7:
		if (p_player->m_targetInfo.m_distance < 4000) {
			FUN_10014723(p_player, p_player->m_index | 0x200, RandomIntBelow(2) ? 4 : 12, 10000);
		}
		else {
			FUN_10014723(p_player, p_player->m_index | 0x200, RandomIntBelow(2) ? 3 : 15, 10000);
		}
		break;
	case 9:
		p_player->m_unk0x180 = 0;
		p_player->m_steering->m_turn = 0;
		p_player->m_unk0x196 = 1;
		break;
	case 10:
		if (!p_player->m_mech->m_unk0xa4 || p_player->m_unk0x172 != 10 || p_player->m_unk0x7c != -1) {
			p_player->m_steering->m_unk0x2f = 1;
			p_player->m_unk0x178 = g_currentClock + 0x5a8;
		}
		else {
			p_player->m_unk0x178 = g_currentClock + 0x2d4;
		}
		break;
	case 12:
		p_player->m_unk0x178 = g_currentClock + 0x46b4;
		FUN_10054851(p_player);
		break;
	default:
		break;
	}
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

// Turns p_player toward its goal and closes on its target; once stopped and turned more than 5
// degrees away, turns in place (FUN_1001498c) until the target is more than 4500 away.
// FUNCTION: MW2 0x10014aa8
void FUN_10014aa8(Player* p_player)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_player->m_aiGoal);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	if (!p_player->m_unk0x184) {
		FUN_1005372c(p_player, p_player->m_aiTarget);
		if (!FUN_10015709(p_player)) {
			FUN_1005398f(p_player);
			p_player->m_steering->m_throttle = FUN_10053811(p_player, 4000);
		}

		if (!p_player->m_steering->m_throttle && abs(heading) > 0x50000) {
			p_player->m_unk0x184 = 1;
		}

		FUN_100160eb(p_player);
	}

	if (p_player->m_unk0x184 == 1) {
		if (FUN_10015709(p_player)) {
			heading = FUN_1005432f(p_player);
		}
		else {
			heading = FUN_1005398f(p_player);
		}

		if (!FUN_1001498c(p_player, heading)) {
			p_player->m_steering->m_throttle = 0;
		}
		else {
			FUN_100160eb(p_player);
		}

		FUN_1005372c(p_player, p_player->m_aiTarget);
		if (p_player->m_targetInfo.m_distance > 4500) {
			p_player->m_unk0x184 = 0;
		}
	}
}

// Closes on p_target. Whether it is within 8000, or more when closing fast (FUN_10016093, per
// 500000).
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10014c3d
MechS32 FUN_10014c3d(Player* p_player, MechS16 p_target)
{
	MechDouble scale;
	MechS32 heading;
	MechS32 result;

	result = FALSE;
	FUN_1005372c(p_player, p_target);
	if (!FUN_10015709(p_player)) {
		heading = FUN_1005398f(p_player);
		p_player->m_steering->m_throttle = FUN_10053811(p_player, 5500);
	}
	else {
		heading = FUN_1005432f(p_player);
	}

	FUN_1004b5a0(p_player, heading);
	scale = FixedDiv16(FUN_10016093(p_player) << 16, 500000) / 65536.0;
	if (scale < 1.0) {
		scale = 1.0;
	}

	if (p_player->m_targetInfo.m_distance <= scale * 8000.0) {
		result = TRUE;
	}

	FUN_100160eb(p_player);
	return result;
}

// Closes on p_target's shape until it is its target's again. Whether it is within 2000 (then it
// sets steering flag 0x43).
// FUNCTION: MW2 0x10014d4e
MechS32 FUN_10014d4e(Player* p_player, MechS16 p_target)
{
	MechS32 result;

	result = FALSE;
	FUN_1005372c(p_player, p_target);
	if (!FUN_10015709(p_player) || FUN_1001627f(p_target) == p_player->m_unk0x188) {
		FUN_1005398f(p_player);
		p_player->m_steering->m_throttle = FUN_10053811(p_player, 0);
	}

	if (p_player->m_targetInfo.m_unk0x04 <= 2000) {
		p_player->m_steering->m_unk0x43 = 1;
		result = TRUE;
	}

	return result;
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

// Turns p_player toward p_target and runs an attack in three steps (m_unk0x180): steer at it
// while it can fire and has a line to it, then for a second, then FUN_100155e1. Whether that ended.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10014f23
MechS32 FUN_10014f23(Player* p_player, MechS16 p_target)
{
	MechS32 heading;
	MechS32 result;

	result = FALSE;
	FUN_1005372c(p_player, p_target);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	switch (p_player->m_unk0x180) {
	case 0:
		if (FUN_10016222(p_player, 40) && FUN_10015520(p_player)) {
			FUN_100156f2(p_player, 1);
		}
		else {
			FUN_100156f2(p_player, 0);
			p_player->m_unk0x180 = 1;
			if (FUN_10016222(p_player, 20) && !RandomIntBelow(4)) {
				p_player->m_unk0x174 = 5;
			}

			p_player->m_unk0x17c = g_currentClock + 181;
		}
		break;
	case 1:
		if (p_player->m_unk0x17c < g_currentClock) {
			p_player->m_unk0x180 = 2;
			FUN_100156f2(p_player, 0);
		}
		else {
			FUN_100156f2(p_player, 1);
		}

		FUN_1005398f(p_player);
		break;
	case 2:
		if (FUN_100155e1(p_player)) {
			result = TRUE;
		}
		break;
	}

	return result;
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

// Turns p_player toward p_target and circles it (steering flags 0x20/0x21) while more than 1000
// away, counting the passes it makes closer in (m_unk0x180). Whether it has weapons ready.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10015172
MechS32 FUN_10015172(Player* p_player, MechS16 p_target)
{
	MechS32 rate;
	MechS32 result;
	MechS32 turn;
	MechS16 side;
	Mech* mech;
	Mech* targetMech;

	FUN_1005372c(p_player, p_target);
	turn = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, turn);
	if (p_player->m_unk0x78 || p_player->m_mech->m_unk0xa4) {
		result = TRUE;
	}
	else {
		result = FALSE;
	}

	if (p_player->m_unk0x180 == 2) {
		return result;
	}

	turn = abs(turn);
	targetMech = g_players[p_target & 0xff]->m_mech;
	mech = p_player->m_mech;
	if (turn > mech->m_unk0xe0 && turn < mech->m_unk0xe0 * 3) {
		side = 1;
	}
	else {
		side = 0;
	}

	if (p_player->m_targetInfo.m_distance > 1000) {
		rate = FUN_10016093(p_player);
		if (rate > (p_player->m_targetInfo.m_distance <= 6000 ? 9 : 36)) {
			FUN_10016057(p_player, side);
		}
		else {
			FUN_10016057(p_player, !side);
		}

		FUN_100156f2(p_player, 1);
		if (!FUN_10016222(p_player, 40)) {
			result = TRUE;
		}
	}
	else {
		if (p_player->m_steering->m_unk0x1d) {
			p_player->m_unk0x180++;
		}

		FUN_100156f2(p_player, 0);
		p_player->m_steering->m_unk0x20 = 0;
		p_player->m_steering->m_unk0x21 = 0;
	}

	return result;
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

// Turns p_player toward its goal and closes on its target, giving up the goal (FUN_10054778,
// FUN_10054851) now and then while far from the target, and handing the target to FUN_10054a30
// within 3000.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100153e6
MechS32 FUN_100153e6(Player* p_player)
{
	MechS32 heading;
	MechS32 result;

	result = 0;
	FUN_1005372c(p_player, p_player->m_aiGoal);
	if (p_player->m_unk0x17c < g_currentClock) {
		if (p_player->m_targetInfo.m_distance > 15000.0) {
			FUN_10054778(p_player);
			FUN_10054851(p_player);
		}

		p_player->m_unk0x17c = g_currentClock + 0x5a8;
	}

	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	FUN_1005372c(p_player, p_player->m_aiTarget);
	if (!FUN_10015709(p_player)) {
		p_player->m_steering->m_throttle = FUN_10053811(p_player, 3000);
		FUN_1005398f(p_player);
	}

	FUN_100160eb(p_player);
	if (p_player->m_targetInfo.m_distance <= 3000) {
		FUN_10054a30(p_player, p_player->m_aiTarget);
	}

	return result;
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
MechS32 FUN_100155e1(Player* p_player)
{
	STUB(0x100155e1);
	return 0;
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

// Which side of p_player the point (p_x, p_y, p_z) is, seen from p_shape: 1 or -1.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10015d2a
MechS16 FUN_10015d2a(Player* p_player, ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 dz;
	MechS32 unused;
	MechU32 distance;
	MechS32 heading;
	MechS32 pitch;
	MechS32 dx;
	MechS32 dy;

	dx = p_shape->m_unk0x34 - p_x;
	dy = p_shape->m_unk0x38 - p_y;
	dz = p_shape->m_unk0x3c - p_z;
	FUN_10060197(dx, dy, dz, &heading, &pitch, &distance, &unused);
	heading -= p_player->m_heading;
	if (heading > 0xb40000) {
		heading -= 0x1680000;
	}
	else if (heading < -0xb40000) {
		heading += 0x1680000;
	}

	return heading >= 0 ? -1 : 1;
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

// Whether p_mech has weapons but none left that can fire: every one is out of ammunition.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10015fa8
MechS32 FUN_10015fa8(Mech* p_mech)
{
	MechS16 i;
	WeaponSlot* slot;
	MechS32 found;

	found = FALSE;
	for (i = 0; i < p_mech->m_weaponCount && !found; i++) {
		slot = &p_mech->m_weapons[i];
		if (slot->m_state != c_weaponEmpty && slot->m_ammo) {
			found = TRUE;
			break;
		}
	}

	return !found && p_mech->m_weaponCount;
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

// Picks p_player's place around its goal player (eight places, 45 degrees apart): the one it is
// nearest, moved off the front and back, or else the first free one on its side. Returns twice
// the place.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100166b1
MechS32 FUN_100166b1(Player* p_player)
{
	MechS32 angle;
	MechS32 i;
	MechS32 place;
	MechS32 found;
	Player* leader;

	leader = g_players[p_player->m_aiGoal & 0xff];
	angle = FixedDiv16(FUN_10015dd6(leader, p_player->m_index | 0x200), 0x2d0000);
	place = (angle + 0x8000) >> 16;
	if (place >= 8) {
		place = 0;
	}

	if (place == 0 || place == 1 || place == 7) {
		if (place == 0) {
			if (RandomIntBelow(2)) {
				place = 2;
			}
			else {
				place = 6;
			}

			if (p_player->m_unk0x1a2[place]) {
				if (place == 2) {
					place = 6;
				}
				else {
					place = 2;
				}
			}
		}

		if (place == 1) {
			place = 3;
		}

		if (place == 7) {
			place = 5;
		}
	}
	else {
		found = FALSE;
		if (place > 4) {
			for (i = 6; i > 3 && !found; i--) {
				if (!leader->m_unk0x1a2[i]) {
					found = TRUE;
				}
			}

			i++;
		}
		else {
			for (i = 2; i < 5 && !found; i++) {
				if (!leader->m_unk0x1a2[i]) {
					found = TRUE;
				}
			}

			i--;
		}

		place = i;
	}

	return place * 2;
}

// FUNCTION: MW2 0x10016880
MechS32 FUN_10016880(Mech* p_mech)
{
	return p_mech->m_unk0xa4 && p_mech->m_player->m_steering->m_unk0x2f != 1 && !p_mech->m_player->m_unk0x196 &&
		   p_mech->m_player->m_unk0x170 != 10;
}
