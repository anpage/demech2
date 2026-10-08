#ifndef SHOTS_H
#define SHOTS_H

#include "careerrecord.h"
#include "decomp.h"
#include "effect.h"
#include "fixedfloat.h"
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
	MechS32 m_type;           // 0x00 — 7 for a shot that knocks mechs down
	MechS32 m_impact;         // 0x04 — the effect it sets off, with c_impactMech etc.
	MechScalar m_velocity[3]; // 0x08
	MechScalar m_steering[3]; // 0x14
	MechS32 m_age;            // 0x20 — ticks since launch
	MechS32 m_lifetime;       // 0x24 — ticks left
	MechScalar m_swayPhase;   // 0x28 — an angle (SwayShot)
	SceneObject* m_object;    // 0x2c
	MechS32 m_target;         // 0x30
	MechS32 m_targetKind;     // 0x34
	MechU32 m_flags;          // 0x38
	MechS32 m_unk0x3c;        // 0x3c
	MechS32 m_shooter;        // 0x40 — a player index
	MechScalar m_damage;      // 0x44
	MechScalar m_heat;        // 0x48 — added to the heat of the mech it hits
	MechS32 m_tracked;        // 0x4c — the tracked-shot view follows it
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
	extern MechS32 g_lastLocalMissile;
	extern MechS32 g_trackedShot;
	extern struct Player* g_launchEffectPlayer;
	extern MechS32 g_effectCameraEnabled;
	extern MechS32 g_lastHitShooter;
	extern MechS32 g_nukeTimeLeft;
	extern MechScalar g_nukeMaxRadius;
	extern MechScalar g_trackedShotView[7];
	extern MechScalar g_nukeRadius;
	extern Vector3 g_nukePosition;
	extern CareerRecord g_careerRecord;
	extern Shot g_shots[0xaf];
	extern Effect g_effects[0x100];

	void FirstShots(void);
	void ResetEffectSlot(MechS32 p_index);
	void ResetShotSlot(MechS32 p_index);
	void UpdateAllShots(void);
	void UpdateShot(MechS32 p_index);
	void SwayShot(Shot* p_shot, MechScalar* p_x, MechScalar* p_y, MechScalar* p_z);
	void GuideMissileToTarget(Shot* p_shot, MechScalar p_x, MechScalar p_y, MechScalar p_z);
	void DetonateShot(
		MechS32 p_index,
		MechS32 p_explode,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_camX,
		MechScalar p_camY,
		MechScalar p_camZ
	);
	void SpawnEffect(
		MechS32 p_owner,
		MechS32 p_type,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_camX,
		MechScalar p_camY,
		MechScalar p_camZ
	);
	void SpawnRotatedEffect(
		MechS32 p_type,
		MechScalar p_rotX,
		MechScalar p_rotY,
		MechScalar p_rotZ,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z
	);
	void SpawnLaunchEffect(MechS32 p_type, struct Player* p_player);
	void SpawnEffectEx(
		MechS32 p_owner,
		MechS32 p_type,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_camX,
		MechScalar p_camY,
		MechScalar p_camZ,
		MechScalar p_rotX,
		MechScalar p_rotY,
		MechScalar p_rotZ
	);
	void UpdateEffects(void);
	void DamageMechsInRadius(
		MechS32 p_owner,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_radius,
		MechScalar p_rate
	);
	void DamageThingsInRadius(
		MechS32 p_owner,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_radius,
		MechScalar p_rate
	);
	MechScalar* GetTrackedShotView(void);
	MechS32 TrackLastShot(void);
	void KillGameThing(MechU32 p_index);
	void DamageGameThing(
		MechS32 p_owner,
		struct Shape* p_shape,
		MechScalar p_damage,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z
	);
	void ScatterDebris(MechScalar p_x, MechScalar p_y, MechScalar p_z, MechS32 p_count);
	void SaveCareerRecord(void);
	void HeatMechsNearFires(void);
	void StartNuke(struct Player* p_player);
	void UpdateNuke(void);

#ifdef __cplusplus
}
#endif

#endif // SHOTS_H
