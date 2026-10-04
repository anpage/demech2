#ifndef SHOTS_H
#define SHOTS_H

#include "carcfg.h"
#include "decomp.h"
#include "effect.h"
#include "object.h"
#include "types.h"
#include "vector3.h"

// Shot::m_targetKind.
enum {
	c_shotTargetPlayer = 0x200,
	c_shotTargetGameThing = 0x400
};

// Shot::m_flags: set when a guided missile comes within 100 units of its target.
enum {
	c_shotProximityFuse = 0x8000
};

// Shot::m_impact, above the effect in the low byte: what the shot hit.
enum {
	c_impactMech = 0x100,
	c_impactThing = 0x200,
	c_impactGround = 0x400
};

// A projectile in flight.
// SIZE 0x50
typedef struct Shot {
	MechS32 m_type;        // 0x00 — 7 for a shot that knocks mechs down
	MechS32 m_impact;      // 0x04 — the effect it sets off, with c_impactMech etc.
	MechS32 m_velocity[3]; // 0x08
	MechS32 m_steering[3]; // 0x14
	MechS32 m_age;         // 0x20 — ticks since launch
	MechS32 m_lifetime;    // 0x24 — ticks left
	MechS32 m_unk0x28;     // 0x28 — a phase angle (FUN_1006ad78)
	SceneObject* m_object; // 0x2c
	MechS32 m_target;      // 0x30
	MechS32 m_targetKind;  // 0x34
	MechU32 m_flags;       // 0x38
	MechS32 m_unk0x3c;     // 0x3c
	MechS32 m_shooter;     // 0x40 — a player index
	MechS32 m_damage;      // 0x44
	MechS32 m_heat;        // 0x48 — added to the heat of the mech it hits
	MechS32 m_tracked;     // 0x4c — the tracked-shot view follows it
} Shot;

struct Player;
struct Shape;

// The functions and globals of shots.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_effectCameraActive;
	extern MechS32 g_effectCameraEffect;
	extern MechS32 g_unk0x100ad448;
	extern MechS32 g_trackedShot;
	extern struct Player* g_unk0x100ad450;
	extern MechS32 g_unk0x100ad454;
	extern MechS32 g_lastHitShooter;
	extern MechS32 g_nukeTimeLeft;
	extern MechS32 g_nukeMaxRadius;
	extern MechS32 g_trackedShotView[7];
	extern MechS32 g_nukeRadius;
	extern Vector3 g_nukePosition;
	extern CarCfg g_carCfg;
	extern Shot g_shots[0xaf];
	extern Effect g_effects[0x100];

	void FirstShots(void);
	void ResetEffectSlot(MechS32 p_index);
	void ResetShotSlot(MechS32 p_index);
	void UpdateAllShots(void);
	void UpdateShot(MechS32 p_index);
	void FUN_1006ad78(Shot* p_shot, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void GuideMissileToTarget(Shot* p_shot, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void DetonateShot(
		MechS32 p_index,
		MechS32 p_explode,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z,
		MechS32 p_camX,
		MechS32 p_camY,
		MechS32 p_camZ
	);
	void FUN_1006b152(
		MechS32 p_owner,
		MechS32 p_type,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z,
		MechS32 p_camX,
		MechS32 p_camY,
		MechS32 p_camZ
	);
	void FUN_1006b18b(
		MechS32 p_type,
		MechS32 p_rotX,
		MechS32 p_rotY,
		MechS32 p_rotZ,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z
	);
	void FUN_1006b1c8(MechS32 p_type, struct Player* p_player);
	void FUN_1006b1fb(
		MechS32 p_owner,
		MechS32 p_type,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z,
		MechS32 p_camX,
		MechS32 p_camY,
		MechS32 p_camZ,
		MechS32 p_rotX,
		MechS32 p_rotY,
		MechS32 p_rotZ
	);
	void UpdateEffects(void);
	void FUN_1006bc13(MechS32 p_owner, MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32 p_radius, MechS32 p_rate);
	void FUN_1006bdb4(MechS32 p_owner, MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32 p_radius, MechS32 p_rate);
	MechS32* GetTrackedShotView(void);
	MechS32 TrackLastShot(void);
	void FUN_1006bf8c(MechU32 p_index);
	void FUN_1006c11c(MechS32 p_owner, struct Shape* p_shape, MechS32 p_damage, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void FUN_1006c237(MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32 p_count);
	void SaveCarCfg(void);
	void FUN_1006c362(void);
	void FUN_1006c4e2(struct Player* p_player);
	void FUN_1006c5e7(void);

#ifdef __cplusplus
}
#endif

#endif // SHOTS_H
