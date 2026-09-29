#include "unk1006ca60.h"

#include "decomp.h"
#include "fixedmul.h"
#include "players.h"
#include "ray.h"
#include "rendertarget.h"
#include "types.h"
#include "unk10034a40.h"
#include "unk1003a530.h"
#include "unk100696c0.h"
#include "weapondef.h"

// Tests whether the player can see its target: nothing but the target itself lies on the line
// to it. With p_ahead, a target behind the player (between 90 and 270 degrees off the view)
// is never seen.
// The heading sum has its operands the other way around.
// FUNCTION: MW2 0x1006ca60
MechS32 FUN_1006ca60(Player* p_player, MechS32 p_ahead)
{
	Ray ray;
	ScarletOrchid0x4c* hit;
	MechS32 bearing;
	MechU32 target;
	MechU16 surface;

	bearing = (p_player->m_heading + p_player->m_unk0x6c - p_player->m_targetInfo.m_heading + 0x1680000) % 0x1680000;
	if (p_ahead && bearing < 0x10e0000 && bearing > 0x5a0000) {
		return FALSE;
	}

	BuildRayFromSegment(
		&ray,
		p_player->m_position.m_x,
		p_player->m_position.m_y,
		p_player->m_position.m_z,
		p_player->m_targetInfo.m_position.m_x,
		p_player->m_targetInfo.m_position.m_y,
		p_player->m_targetInfo.m_position.m_z
	);
	BuildRayFixed(&ray);
	hit = NULL;
	if (TestSegmentCollision(&ray, &hit, p_player->m_index) && hit) {
		surface = hit->m_unk0x02;
		target = p_player->m_targetInfo.m_target;
		if ((surface & 0x100) && (target & 0x200)) {
			return hit->m_unk0x14 == (target & 0xff);
		}
		else if ((surface & 0x200) && (target & 0x400)) {
			return hit->m_unk0x14 == (target & 0xff);
		}
		else {
			return FALSE;
		}
	}

	return TRUE;
}

// Walks the ground from (p_x, p_y, p_z) toward the player in steps of 1000 units, and tests
// that the terrain stays within 1000 units of height 0 all the way.
// Stack-slot permutation of the locals; one height comparison has its operands the other
// way around.
// FUNCTION: MW2 0x1006cbe7
MechS32 FUN_1006cbe7(Player* p_player, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 steps;
	MechS32 level;
	MechS32 pitch;
	MechU32 distance;
	MechS32 unk0x10;
	MechS32 height;
	MechS32 i;
	MechS32 yaw;
	MechS32 stepX;
	MechS32 stepY;
	MechS32 stepZ;

	level = 0;
	FUN_10060197(
		p_player->m_position.m_x - p_x,
		p_player->m_position.m_y - p_y,
		p_player->m_position.m_z - p_z,
		&yaw,
		&unk0x10,
		&distance,
		&pitch
	);
	steps = (MechS32) distance / 1000;
	stepX = FixedMul16(FUN_1006973a(yaw) * 1000, FUN_1006973a(pitch));
	stepZ = FixedMul16(FUN_100696c0(yaw) * 1000, FUN_1006973a(pitch));
	stepY = FUN_100696c0(pitch) * 1000;
	for (i = 0; i < steps; i++) {
		p_x -= stepX;
		p_y -= stepY;
		p_z -= stepZ;
		height = GetTerrainHeight(p_x, p_y, p_z);
		if (height - 1000 > level || height + 1000 < level) {
			return 0;
		}
	}

	return 1;
}

// Returns which of the weapon's range bands the player's target is in: 3 within the short
// range, 2 within the long one, 1 right at it and 0 beyond it.
// FUNCTION: MW2 0x1006cd45
MechS32 FUN_1006cd45(Player* p_player, WeaponDef* p_weapon)
{
	if (p_player->m_targetInfo.m_distance < p_weapon->m_unk0x3c) {
		return 3;
	}

	if (p_player->m_targetInfo.m_distance < p_weapon->m_unk0x40) {
		return 2;
	}

	if (p_player->m_targetInfo.m_distance > p_weapon->m_unk0x40) {
		return 0;
	}

	return 1;
}
