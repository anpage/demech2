#include "shots.h"

#include "approxlen.h"
#include "clock.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "fixedsqrt.h"
#include "gamething.h"
#include "geocache.h"
#include "object.h"
#include "players.h"
#include "types.h"
#include "unk100696c0.h"

// GLOBAL: MW2 0x100ad450
undefined4 g_unk0x100ad450 = 0;

// GLOBAL: MW2 0x1017bac0
Shot g_shots[0xaf];

// STUB: MW2 0x1006a230
void FirstShots(void)
{
	STUB(0x1006a230);
}

// Ages every shot in flight by g_deltaTime and moves it on.
// Stack-slot permutation: i and shot.
// FUNCTION: MW2 0x1006a486
void UpdateAllShots(void)
{
	MechS32 i;
	Shot* shot;

	for (i = 0; i < 0xaf; i++) {
		shot = &g_shots[i];
		if (shot->m_flags && shot->m_object) {
			shot->m_age += g_deltaTime;
			shot->m_lifetime -= g_deltaTime;
			switch (shot->m_unk0x3c) {
			case 0:
				UpdateShot(i);
				break;
			default:
				break;
			}
		}
	}
}

// STUB: MW2 0x1006a533
void UpdateShot(MechS32 p_index)
{
	STUB(0x1006a533);
}

// Steers a guided missile at (p_x, p_y, p_z) toward its target, and arms its proximity fuse
// within 100 units of it.
// Stack-slot permutation: every local but distance.
// FUNCTION: MW2 0x1006ae5a
void GuideMissileToTarget(Shot* p_shot, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 targetY;
	MechS32 targetZ;
	Player* player;
	MechS32 dy;
	MechS32 dx;
	MechS32 dz;
	GameThing* thing;
	MechS32 distance;
	MechS32 vx;
	MechS32 vy;
	MechS32 vz;
	MechS32 targetX;

	if (p_shot->m_target < 0) {
		return;
	}

	switch (p_shot->m_targetKind) {
	case c_shotTargetPlayer:
		player = g_players[p_shot->m_target];
		if ((player->m_flags & 2) || (player->m_flags & 4)) {
			p_shot->m_steering[0] = p_shot->m_steering[1] = p_shot->m_steering[2] = 0;
			p_shot->m_targetKind = 0;
			return;
		}

		targetX = player->m_position[0];
		targetY = player->m_position[1];
		targetZ = player->m_position[2];
		break;
	case c_shotTargetGameThing:
		thing = &g_gameThings[p_shot->m_target];
		if (thing->m_unk0x00 & 4) {
			p_shot->m_steering[0] = p_shot->m_steering[1] = p_shot->m_steering[2] = 0;
			p_shot->m_targetKind = 0;
			return;
		}

		FUN_10020c6f(thing->m_unk0x04, &targetX, &targetY, &targetZ);
		break;
	default:
		return;
	}

	dx = targetX - p_x;
	dy = targetY - p_y;
	dz = targetZ - p_z;
	distance = ApproximateVectorLength(dx, dy, dz);
	if (distance <= 100) {
		p_shot->m_flags |= c_shotProximityFuse;
	}

	dx = FixedDiv16(dx, distance);
	dy = FixedDiv16(dy, distance);
	dz = FixedDiv16(dz, distance);
	vx = p_shot->m_velocity[0];
	vy = p_shot->m_velocity[1];
	vz = p_shot->m_velocity[2];
	NormalizeVectorGuarded(&vx, &vy, &vz);
	p_shot->m_steering[0] = dx - vx;
	p_shot->m_steering[1] = dy - vy;
	p_shot->m_steering[2] = dz - vz;
	targetY = FUN_100698de(vx, vz);
	targetX = -FUN_1006975b(vy << 13);
	SetObjRotation(p_shot->m_object, targetX, targetY, 0, 0);
}

// FUNCTION: MW2 0x1006b152
void FUN_1006b152(
	MechS32 p_unk0x00,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18,
	MechS32 p_unk0x1c
)
{
	FUN_1006b1fb(p_unk0x00, p_unk0x04, p_unk0x08, p_unk0x0c, p_unk0x10, p_unk0x14, p_unk0x18, p_unk0x1c, 0, 0, 0);
}

// FUNCTION: MW2 0x1006b18b
void FUN_1006b18b(
	MechS32 p_unk0x00,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18
)
{
	FUN_1006b1fb(
		-2,
		p_unk0x00,
		p_unk0x10,
		p_unk0x14,
		p_unk0x18,
		p_unk0x10,
		p_unk0x14,
		p_unk0x18,
		p_unk0x04,
		p_unk0x08,
		p_unk0x0c
	);
}

// FUNCTION: MW2 0x1006b1c8
void FUN_1006b1c8(MechS32 p_unk0x00, undefined4 p_unk0x04)
{
	g_unk0x100ad450 = p_unk0x04;
	FUN_1006b1fb(-2, p_unk0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

// STUB: MW2 0x1006b1fb
void FUN_1006b1fb(
	MechS32 p_unk0x00,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18,
	MechS32 p_unk0x1c,
	MechS32 p_unk0x20,
	MechS32 p_unk0x24,
	MechS32 p_unk0x28
)
{
	STUB(0x1006b1fb);
}

// STUB: MW2 0x1006b99a
void UpdateEffects(void)
{
	STUB(0x1006b99a);
}

// STUB: MW2 0x1006c345
void SaveCarCfg(void)
{
	STUB(0x1006c345);
}
