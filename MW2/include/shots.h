#ifndef SHOTS_H
#define SHOTS_H

#include "decomp.h"
#include "object.h"
#include "types.h"

// Shot::m_targetKind.
enum {
	c_shotTargetPlayer = 0x200,
	c_shotTargetGameThing = 0x400
};

// Shot::m_flags: set when a guided missile comes within 100 units of its target.
enum {
	c_shotProximityFuse = 0x8000
};

// A projectile in flight.
// SIZE 0x50
typedef struct Shot {
	MechS32 m_unk0x00[2];                    // 0x00
	MechS32 m_velocity[3];                   // 0x08
	MechS32 m_steering[3];                   // 0x14
	MechS32 m_age;                           // 0x20 — ticks since launch
	MechS32 m_lifetime;                      // 0x24 — ticks left
	undefined4 m_unk0x28;                    // 0x28
	AmberWillow0x7c* m_object;               // 0x2c
	MechS32 m_target;                        // 0x30
	MechS32 m_targetKind;                    // 0x34
	MechU32 m_flags;                         // 0x38
	MechS32 m_unk0x3c;                       // 0x3c
	undefined4 m_unk0x40[(0x50 - 0x40) / 4]; // 0x40
} Shot;

// The functions and globals of shots.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern Shot g_shots[0xaf];

	void FirstShots(void);
	void UpdateAllShots(void);
	void UpdateShot(MechS32 p_index);
	void GuideMissileToTarget(Shot* p_shot, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void UpdateEffects(void);
	void SaveCarCfg(void);

#ifdef __cplusplus
}
#endif

#endif // SHOTS_H
