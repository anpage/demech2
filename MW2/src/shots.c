#define FIXEDTRIG_FLOAT_SINE /* the accelerated editions' float sine, where they have it (fixedtrig.h) */
#include "shots.h"

#include "ai.h"
#include "animation.h"
#include "approxlen.h"
#include "careerrecord.h"
#include "clock.h"
#include "collision.h"
#include "config.h"
#include "debris.h"
#include "decomp.h"
#include "effect.h"
#include "effectinfo.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "fixeddiv.h"
#include "fixedfloat.h"
#include "fixedmul.h"
#include "fixedmul29.h"
#include "fixedsqrt.h"
#include "fixedtrig.h"
#include "gamething.h"
#include "geocache.h"
#include "integrate.h"
#include "mech.h"
#include "mechdamage.h"
#include "network.h"
#include "object.h"
#include "objective.h"
#include "palette.h"
#include "players.h"
#include "polydraw.h"
#include "random.h"
#include "ray.h"
#include "render.h"
#include "shape.h"
#include "shapelists.h"
#include "simmain.h"
#include "soundfx.h"
#include "team.h"
#include "types.h"
#ifdef MW2_SHOT_TRACERS
#include "premercs/tracers.h"
#endif
#include "view.h"
#include "weapondata.h"
#include "weapons.h"

#include <string.h>

#ifdef MW2_WEAPON_FUNCTIONS
DECOMP_SIZE_ASSERT(Shot, 0x58)
DECOMP_SIZE_ASSERT(Effect, 0x2c)
DECOMP_SIZE_ASSERT(EffectInfo, 0x2c)
#else
DECOMP_SIZE_ASSERT(Shot, 0x50)
DECOMP_SIZE_ASSERT(Effect, 0x28)
DECOMP_SIZE_ASSERT(EffectInfo, 0x1c)
#endif
DECOMP_SIZE_ASSERT(CareerRecord, 0xd6)

// GLOBAL: MW2 0x100ad440
// GLOBAL: MW2GBL 0x100b6900
// GLOBAL: MW2PREMERCS 0x100c6d74
// GLOBAL: MW2MATROX 0x100ae0a4
// GLOBAL: MW23DFX 0x1025a694
// GLOBAL: MW2S3 0x10065078
// GLOBAL: MW2POWERVR 0x101b9ccc
MechS32 g_effectCameraActive = 0;

// GLOBAL: MW2 0x100ad444
// GLOBAL: MW2GBL 0x100b6904
// GLOBAL: MW2PREMERCS 0x100c6d78
// GLOBAL: MW2MATROX 0x100ae0a8
// GLOBAL: MW23DFX 0x1025a698
// GLOBAL: MW2S3 0x1006507c
// GLOBAL: MW2POWERVR 0x101b9cd0
MechS32 g_effectCameraEffect = -1;

// GLOBAL: MW2 0x100ad448
// GLOBAL: MW2GBL 0x100b6908
// GLOBAL: MW2PREMERCS 0x100c6d7c
// GLOBAL: MW2MATROX 0x100ae0ac
// GLOBAL: MW23DFX 0x1025a69c
// GLOBAL: MW2ATI 0x1018e994
// GLOBAL: MW2S3 0x10065080
// GLOBAL: MW2POWERVR 0x101b9cd4
MechS32 g_lastLocalMissile = -1;

// GLOBAL: MW2 0x100ad44c
// GLOBAL: MW2GBL 0x100b690c
// GLOBAL: MW2PREMERCS 0x100c6d80
// GLOBAL: MW2MATROX 0x100ae0b0
// GLOBAL: MW23DFX 0x1025a6a0
// GLOBAL: MW2ATI 0x1018e998
// GLOBAL: MW2S3 0x10065084
// GLOBAL: MW2POWERVR 0x101b9cd8
MechS32 g_trackedShot = -1;

// GLOBAL: MW2 0x100ad450
// GLOBAL: MW2GBL 0x100b6910
// GLOBAL: MW2PREMERCS 0x100c6d84
// GLOBAL: MW2MATROX 0x100ae0b4
// GLOBAL: MW23DFX 0x1025a6a4
// GLOBAL: MW2ATI 0x1018e99c
// GLOBAL: MW2S3 0x10065088
// GLOBAL: MW2POWERVR 0x101b9cdc
Player* g_launchEffectPlayer = NULL;

// GLOBAL: MW2 0x100ad454
// GLOBAL: MW2GBL 0x100b6914
// GLOBAL: MW2PREMERCS 0x100c6d88
// GLOBAL: MW2MATROX 0x100ae0b8
// GLOBAL: MW23DFX 0x1025a6a8
// GLOBAL: MW2S3 0x1006508c
// GLOBAL: MW2POWERVR 0x101b9ce0
MechS32 g_effectCameraEnabled = 1;

// The player whose shot hit something last.
// GLOBAL: MW2 0x100ad458
// GLOBAL: MW2GBL 0x100b6918
// GLOBAL: MW2PREMERCS 0x100c6d8c
// GLOBAL: MW2MATROX 0x100ae0bc
// GLOBAL: MW23DFX 0x1025a6ac
// GLOBAL: MW2ATI 0x1018e9a4
// GLOBAL: MW2S3 0x10065090
// GLOBAL: MW2POWERVR 0x101b9ce4
MechS32 g_lastHitShooter = -1;

// GLOBAL: MW2 0x100ad45c
// GLOBAL: MW2GBL 0x100b691c
// GLOBAL: MW2PREMERCS 0x100c6d90
// GLOBAL: MW2MATROX 0x100ae0c0
// GLOBAL: MW23DFX 0x1025a6b0
// GLOBAL: MW2ATI 0x1018e9a8
// GLOBAL: MW2S3 0x10065094
// GLOBAL: MW2POWERVR 0x101b9ce8
MechS32 g_nukeTimeLeft = 0;

// GLOBAL: MW2 0x100bee50
// GLOBAL: MW2GBL 0x100bb700
// GLOBAL: MW2PREMERCS 0x100cc948
// GLOBAL: MW2MATROX 0x10124724
// GLOBAL: MW23DFX 0x1007dc6c
// GLOBAL: MW2ATI 0x10072c6c
// GLOBAL: MW2S3 0x1007cad8
// GLOBAL: MW2POWERVR 0x10070cb4
MechScalar g_nukeMaxRadius;

// The view that follows a tracked shot: its position, heading and three more angles.
// GLOBAL: MW2 0x100bee58
// GLOBAL: MW2GBL 0x100bb708
// GLOBAL: MW2PREMERCS 0x100cc950
// GLOBAL: MW2MATROX 0x10124708
// GLOBAL: MW23DFX 0x1007dc50
// GLOBAL: MW2ATI 0x10072c50
// GLOBAL: MW2S3 0x1007cae0
// GLOBAL: MW2POWERVR 0x10070c98
MechScalar g_trackedShotView[7];

// GLOBAL: MW2 0x100bee74
// GLOBAL: MW2GBL 0x100bb724
// GLOBAL: MW2PREMERCS 0x100cc96c
// GLOBAL: MW2MATROX 0x10124728
// GLOBAL: MW23DFX 0x1007dc70
// GLOBAL: MW2ATI 0x10072c70
// GLOBAL: MW2S3 0x1007cafc
// GLOBAL: MW2POWERVR 0x10070cb8
MechScalar g_nukeRadius;

// GLOBAL: MW2 0x100bee78
// GLOBAL: MW2GBL 0x100bb728
// GLOBAL: MW2PREMERCS 0x100cc970
// GLOBAL: MW2MATROX 0x101246f8
// GLOBAL: MW23DFX 0x1007dc40
// GLOBAL: MW2ATI 0x10072c40
// GLOBAL: MW2S3 0x1007cb00
// GLOBAL: MW2POWERVR 0x10070c88
Vector3 g_nukePosition;

// The eyepoint's settings while an effect has the camera.
// GLOBAL: MW2 0x100c75f0
// GLOBAL: MW2GBL 0x100bbb70
// GLOBAL: MW2PREMERCS 0x1014e520
// GLOBAL: MW2MATROX 0x1015d5c0
// GLOBAL: MW23DFX 0x10159450
// GLOBAL: MW2S3 0x1014d240
// GLOBAL: MW2POWERVR 0x10083a40
MechScalar g_savedLightX;

// GLOBAL: MW2 0x100c75f4
// GLOBAL: MW2GBL 0x100bbb74
// GLOBAL: MW2PREMERCS 0x1014e524
// GLOBAL: MW2MATROX 0x1015d5c4
// GLOBAL: MW23DFX 0x10159454
// GLOBAL: MW2S3 0x1014d244
// GLOBAL: MW2POWERVR 0x10083a44
MechScalar g_savedLightY;

// GLOBAL: MW2 0x100c75f8
// GLOBAL: MW2GBL 0x100bbb78
// GLOBAL: MW2PREMERCS 0x1014e528
// GLOBAL: MW2MATROX 0x1015d5c8
// GLOBAL: MW23DFX 0x10159458
// GLOBAL: MW2S3 0x1014d248
// GLOBAL: MW2POWERVR 0x10083a48
MechScalar g_savedLightZ;

// GLOBAL: MW2 0x100c75fc
// GLOBAL: MW2GBL 0x100bbb7c
// GLOBAL: MW2PREMERCS 0x1014e52c
// GLOBAL: MW2MATROX 0x1015d5cc
// GLOBAL: MW23DFX 0x1015945c
// GLOBAL: MW2S3 0x1014d24c
// GLOBAL: MW2POWERVR 0x10083a4c
MechScalar g_savedDirectionalLight;

// GLOBAL: MW2 0x100c7600
// GLOBAL: MW2GBL 0x100bbb80
// GLOBAL: MW2PREMERCS 0x1014e530
// GLOBAL: MW2MATROX 0x1015d5d0
// GLOBAL: MW23DFX 0x10159460
// GLOBAL: MW2S3 0x1014d250
// GLOBAL: MW2POWERVR 0x10083a50
MechS32 g_savedAmbientLight;

// GLOBAL: MW2 0x100c7604
// GLOBAL: MW2GBL 0x100bbb84
// GLOBAL: MW2PREMERCS 0x1014e534
// GLOBAL: MW2MATROX 0x1015d5d4
// GLOBAL: MW23DFX 0x10159464
// GLOBAL: MW2S3 0x1014d254
// GLOBAL: MW2POWERVR 0x10083a54
MechS32 g_savedDistanceFade;

// GLOBAL: MW2 0x100e9250
// GLOBAL: MW2GBL 0x101331c0
// GLOBAL: MW2PREMERCS 0x10212d30
// GLOBAL: MW2MATROX 0x10125880
// GLOBAL: MW23DFX 0x102441c0
// GLOBAL: MW2ATI 0x10177e80
// GLOBAL: MW2S3 0x100abe80
// GLOBAL: MW2POWERVR 0x101a2b40
CareerRecord g_careerRecord;

// GLOBAL: MW2 0x1017bac0
// GLOBAL: MW2GBL 0x100e12f0
// GLOBAL: MW2PREMERCS 0x1014e620
// GLOBAL: MW2MATROX 0x10125970
// GLOBAL: MW23DFX 0x101535a0
// GLOBAL: MW2ATI 0x100875b0
// GLOBAL: MW2S3 0x10084bd0
// GLOBAL: MW2POWERVR 0x1007db90
Shot g_shots[SHOT_COUNT];

// GLOBAL: MW2 0x1017f170
// GLOBAL: MW2GBL 0x100e6aa0
// GLOBAL: MW2PREMERCS 0x10153dd0
// GLOBAL: MW2MATROX 0x10129020
// GLOBAL: MW23DFX 0x10156c50
// GLOBAL: MW2ATI 0x1008ac60
// GLOBAL: MW2S3 0x10088280
// GLOBAL: MW2POWERVR 0x10081240
Effect g_effects[EFFECT_COUNT];

// FUNCTION: MW2 0x1006a230
// FUNCTION: MW2GBL 0x1007be30
// FUNCTION: MW2PREMERCS 0x10085660
// FUNCTION: MW2MATROX 0x1006ad30
// FUNCTION: MW23DFX 0x10044c40
// FUNCTION: MW2ATI 0x10042490
// FUNCTION: MW2S3 0x1001b5b0
// FUNCTION: MW2POWERVR 0x1003d1c0
void FirstShots(void)
{
	MechS32 i;
	Shot* shot;
	Effect* effect;

	for (i = 0; i < SHOT_COUNT; i++) {
		shot = &g_shots[i];
		shot->m_type = -1;
		shot->m_object = NULL;
		ResetShotSlot(i);
	}

	for (i = 0; i < EFFECT_COUNT; i++) {
		effect = &g_effects[i];
		effect->m_object = NULL;
#ifdef MW2_WEAPON_FUNCTIONS
		effect->m_attachedShape = NULL;
#endif
		effect->m_animation = -1;
		effect->m_type = -1;
		effect->m_owner = -1;
		ResetEffectSlot(i);
	}

	g_trackedShotView[0] = g_trackedShotView[1] = g_trackedShotView[2] = 0;
	g_trackedShotView[3] = g_trackedShotView[5] = g_trackedShotView[4] = 0;
	g_trackedShotView[6] = 0;
	memset(&g_careerRecord, 0, sizeof(g_careerRecord));
}

// FUNCTION: MW2 0x1006a349
// FUNCTION: MW2GBL 0x1007bf60
// FUNCTION: MW2PREMERCS 0x10085790
// FUNCTION: MW2MATROX 0x1006ae49
// FUNCTION: MW23DFX 0x10044d60
// FUNCTION: MW2ATI 0x100425b0
// FUNCTION: MW2S3 0x1001b6d0
// FUNCTION: MW2POWERVR 0x1003d2e0
void ResetEffectSlot(MechS32 p_index)
{
	Effect* effect;

	effect = &g_effects[p_index];
	effect->m_position[0] = effect->m_position[1] = effect->m_position[2] = 0;
	effect->m_timeLeft = 0;
	effect->m_active = 0;
	effect->m_hasCamera = 0;
#ifndef MW2_RESET_EFFECT_KEEPS_OWNER
	effect->m_owner = -1;
#endif
}

// FUNCTION: MW2 0x1006a3b1
// FUNCTION: MW2GBL 0x1007bfcd
// FUNCTION: MW2PREMERCS 0x100857f3
// FUNCTION: MW2MATROX 0x1006aeb1
// FUNCTION: MW23DFX 0x10044dd0
// FUNCTION: MW2ATI 0x10042620
// FUNCTION: MW2S3 0x1001b740
// FUNCTION: MW2POWERVR 0x1003d350
void ResetShotSlot(MechS32 p_index)
{
	Shot* shot;

	shot = &g_shots[p_index];
	shot->m_velocity[0] = shot->m_velocity[1] = shot->m_velocity[2] = 0;
	shot->m_steering[0] = shot->m_steering[1] = shot->m_steering[2] = 0;
	shot->m_age = shot->m_lifetime = 0;
	shot->m_swayPhase = 0;
	shot->m_impact = -1;
	shot->m_target = 0;
	shot->m_targetKind = 0;
	shot->m_flags = 0;
	shot->m_unk0x3c = 0;
	shot->m_damage = shot->m_heat = 0;
	shot->m_tracked = 0;
#ifdef MW2_WEAPON_FUNCTIONS
	shot->m_interceptRange = -1;
	shot->m_weaponFlags = 0;
#endif
}

// Ages every shot in flight by g_deltaTime and moves it on.
// FUNCTION: MW2 0x1006a486
// FUNCTION: MW2GBL 0x1007c0ba
// FUNCTION: MW2PREMERCS 0x100858e0
// FUNCTION: MW2MATROX 0x1006af86
// FUNCTION: MW23DFX 0x10044eb0
// FUNCTION: MW2ATI 0x10042700
// FUNCTION: MW2S3 0x1001b820
// FUNCTION: MW2POWERVR 0x1003d430
void UpdateAllShots(void)
{
	MechS32 i;
	Shot* shot;

	for (i = 0; i < SHOT_COUNT; i++) {
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

// Moves the shot on by g_deltaTime and tests the segment it covered for a hit: a mech, a game
// thing or the ground. A hit detonates the shot, just short of the point of impact; otherwise
// it detonates when its lifetime runs out, or steers on toward its target.
// The Matrox edition applies the damage after RecordAttack; the 3Dfx edition skips the local
// player's hit feedback while its flag 2 is set.
// FUNCTION: MW2 0x1006a533
// FUNCTION: MW2GBL 0x1007c16b
// FUNCTION: MW2PREMERCS 0x10085991
// FUNCTION: MW2MATROX 0x1006b033
// FUNCTION: MW23DFX 0x10044f60
// FUNCTION: MW2ATI 0x100427b0
// FUNCTION: MW2S3 0x1001b8d0
// FUNCTION: MW2POWERVR 0x1003d4e0
void UpdateShot(MechS32 p_index)
{
	MechS32 hitResult;
	MechScalar velZ;
	MechU16 surface;
	Shape* hit;
	Shot* shot;
	MechS32 victim;
	MechScalar backX;
	MechScalar x;
	MechScalar backY;
	MechScalar x0;
	MechS32 damageFlags;
	MechScalar y;
	MechScalar backZ;
	MechScalar y0;
	MechScalar velY;
	MechScalar bearing;
	MechScalar velX;
	MechScalar hitZ;
	Ray ray;
	MechScalar hitY;
	MechScalar hitX;
	MechS32 dt;
	MechScalar z0;
	MechScalar z;
	MechScalar groundY;

	hit = NULL;
	dt = g_deltaTime;
	damageFlags = 0;
	shot = &g_shots[p_index];
	GetObjPosition(shot->m_object, &x0, &y0, &z0);
	velX = shot->m_velocity[0];
	velY = shot->m_velocity[1];
	velZ = shot->m_velocity[2];
	x = x0;
	y = y0;
	z = z0;
	IntegrateMidpoint(&x, &velX, shot->m_steering[0], dt);
	IntegrateMidpoint(&y, &velY, shot->m_steering[1], dt);
	IntegrateMidpoint(&z, &velZ, shot->m_steering[2], dt);
	BuildRayFromSegment(&ray, x0, y0, z0, x, y, z);
	if (ApproximateVectorLength(x - x0, y - y0, z - z0) > 3000) {
		BuildRayFloat(&ray);
	}
	else {
		BuildRayFixed(&ray);
	}

	hitResult = 0;
	surface = 0;
	hit = NULL;
	if (shot->m_flags & c_shotProximityFuse) {
		hitResult = 1;
		if (shot->m_targetKind == c_shotTargetPlayer) {
			hit = GetObjShape(g_players[shot->m_target]->m_obj);
		}
		else if (shot->m_targetKind == c_shotTargetGameThing) {
			hit = GetStaticObjectShape(g_gameThings[shot->m_target].m_staticObject);
		}

		hitX = backX = x0;
		hitY = backY = y0;
		hitZ = backZ = z0;
	}
	else {
		hitResult = TestSegmentCollision(&ray, &hit, shot->m_shooter);
		if (hitResult) {
			hitX = ray.m_x1;
			hitY = ray.m_y1;
			hitZ = ray.m_z1;
#ifdef MW2_FLOAT_SCALARS
			backX = hitX - ray.m_dirX * 100;
			backY = hitY - ray.m_dirY * 100;
			backZ = hitZ - ray.m_dirZ * 100;
#else
			backX = hitX - FixedMul16(100, ray.m_dirX);
			backY = hitY - FixedMul16(100, ray.m_dirY);
			backZ = hitZ - FixedMul16(100, ray.m_dirZ);
#endif
		}
	}

	if (hitResult && hit) {
		surface = hit->m_kind;
		g_lastHitShooter = shot->m_shooter;
		if (surface & 0x100) {
			victim = hit->m_owner;
			shot->m_impact |= c_impactMech;
			if (shot->m_type == 7) {
				shot->m_impact |= 0x1000;
			}

#ifdef MW2_WEAPON_FUNCTIONS
			if (shot->m_weaponFlags & c_weaponNarc) {
				g_players[victim]->m_mech->m_unk0x112 |= 1;
			}
#endif

			bearing = g_players[victim]->m_heading - FixedAtan2(shot->m_velocity[0], shot->m_velocity[2]);
#ifdef MW2_FLOAT_SCALARS
			bearing = fmod(bearing, 360.0);
#else
			bearing %= 0x1680000;
#endif
			if (bearing > FIXED_CONST(180)) {
				bearing -= FIXED_CONST(360);
			}
			else if (bearing < FIXED_CONST(-180)) {
				bearing += FIXED_CONST(360);
			}

			if (bearing < FIXED_CONST(90) && bearing > FIXED_CONST(-90)) {
				damageFlags |= 0x8000;
			}

			if (g_lastHitShooter == g_localPlayerId) {
				switch (GetPlayerSide(victim)) {
				case 0:
					g_careerRecord.m_friendlyHits++;
					break;
				case 1:
					g_careerRecord.m_hits++;
					break;
				case 2:
					g_careerRecord.m_neutralHits++;
					break;
				}
			}

			if (g_lastHitShooter >= 0 && g_players[g_localPlayerId]->m_team == g_players[g_lastHitShooter]->m_team) {
				switch (GetPlayerSide(victim)) {
				case 0:
					g_careerRecord.m_teamFriendlyHits++;
					break;
				case 1:
					g_careerRecord.m_teamHits++;
					break;
				case 2:
					g_careerRecord.m_teamNeutralHits++;
					break;
				}
			}

			if (victim == g_localPlayerId) {
				g_careerRecord.m_hitsTaken++;
			}

			if (g_players[g_localPlayerId]->m_team == g_players[victim]->m_team) {
				g_careerRecord.m_teamHitsTaken++;
			}

#ifndef MW2_SHOT_DAMAGE_AFTER_RECORD
			if (!g_netRole || victim == g_localPlayerId) {
				g_players[victim]->m_mech->m_deltaHeat += shot->m_heat;
				ApplyDamageToMech(
					shot->m_shooter,
					g_players[victim]->m_mech,
					FIXED_FROM_INT(shot->m_damage),
					hit->m_partId | damageFlags
				);
			}
#endif

#ifdef MW2_HIT_FEEDBACK_FLAG_CHECK
			if (victim == g_localPlayerId && !(g_players[g_localPlayerId]->m_flags & 2)) {
#else
			if (victim == g_localPlayerId) {
#endif
				shot->m_impact |= 0x4000;
				if (hit->m_partId == 1 || hit->m_partId == 3 || hit->m_partId == 2 || hit->m_partId == 4) {
					shot->m_impact |= 0x2000;
				}

				PlayPlayerHitFeedback(velX, velY, velZ);
			}

			RecordAttack(shot->m_shooter, victim);
#ifdef MW2_SHOT_DAMAGE_AFTER_RECORD

			if (!g_netRole || victim == g_localPlayerId) {
				g_players[victim]->m_mech->m_deltaHeat += shot->m_heat;
				ApplyDamageToMech(
					shot->m_shooter,
					g_players[victim]->m_mech,
					FIXED_FROM_INT(shot->m_damage),
					hit->m_partId | damageFlags
				);
			}
#endif
		}
		else if (surface & 0x200) {
			shot->m_impact |= c_impactThing;
			DamageGameThing(shot->m_shooter, hit, shot->m_damage, hitX, hitY, hitZ);
		}
		else if ((surface & 0xf0) == 0x50) {
			DamageChunk(hit->m_owner, FIXED_FROM_INT(shot->m_damage));
		}
		else if (surface & 0x400) {
			shot->m_impact |= c_impactThing;
		}
		else {
			shot->m_impact |= c_impactGround;
		}
	}
	else if (y < 0) {
#ifdef MW2_GROUND_HEIGHT_IN_TEST
		if ((groundY = GetTerrainHeight(x, y, z)) > y) {
#else
		groundY = GetTerrainHeight(x, y, z);
		if (y < groundY) {
#endif
			hitResult = 1;
			shot->m_impact |= c_impactGround;
			ClipRayToGround(&ray, groundY);
			hitX = backX = ray.m_x1;
			hitY = backY = ray.m_y1;
			hitZ = backZ = ray.m_z1;
		}
	}

	if (hitResult) {
#if defined(MW2_SHOT_OWNS_EFFECT)
		DetonateShot(hit, p_index, 1, hitX, hitY, hitZ, backX, backY, backZ);
#elif defined(MW2_WEAPON_FUNCTIONS)
		DetonateShot(victim, hit, p_index, 1, hitX, hitY, hitZ, backX, backY, backZ);
#else
	DetonateShot(p_index, 1, hitX, hitY, hitZ, backX, backY, backZ);
#endif
	}
	else if (shot->m_lifetime <= 0) {
#if defined(MW2_SHOT_OWNS_EFFECT)
		DetonateShot(hit, p_index, 0, x, y, z, x, y, z);
#elif defined(MW2_WEAPON_FUNCTIONS)
		DetonateShot(victim, hit, p_index, 0, x, y, z, x, y, z);
#else
	DetonateShot(p_index, 0, x, y, z, x, y, z);
#endif
	}
	else {
		if (shot->m_targetKind && shot->m_age > 0) {
			GuideMissileToTarget(shot, x, y, z);
		}

		SetObjPosition(shot->m_object, x, y, z);
		UpdateObj(shot->m_object);
		shot->m_velocity[0] = velX;
		shot->m_velocity[1] = velY;
		shot->m_velocity[2] = velZ;
		if (shot->m_tracked) {
			g_trackedShot = p_index;
			g_trackedShotView[0] = x;
			g_trackedShotView[1] = y;
			g_trackedShotView[2] = z;
			g_trackedShotView[3] = FixedAtan2(velX, velZ);
			g_trackedShotView[4] = 0;
			g_trackedShotView[5] = 0;
#ifdef MW2_FLOAT_SCALARS
			/* The mark stays an integer word after the Matrox edition's floats (SaveView). */
			*(MechS32*) &g_trackedShotView[6] = 1;
#else
			g_trackedShotView[6] = 1;
#endif
		}
#ifdef MW2_SHOT_TRACERS

		// The missiles leave a trail.
		switch (shot->m_type) {
		case 3:
		case 4:
		case 9:
		case 10:
		case 0xc:
			FUN_10088c5c(x0, y0, z0, x - x0, y - y0, z - z0);
			break;
		case 5:
		case 6:
		case 7:
		case 8:
		case 0xb:
			break;
		}
#endif
	}
}

// Sways (p_x, p_y, p_z) sideways and up and down around the shot's line of flight, by the
// phase in m_swayPhase, and advances the phase.
// MW2MATROX: in floats, with the phase in degrees (3.9779005 a tick) and no scaling of the sway.
// FUNCTION: MW2 0x1006ad78
// FUNCTION: MW2GBL 0x1007c9ff
// FUNCTION: MW2PREMERCS 0x100862aa
// FUNCTION: MW2MATROX 0x1006b93e
// FUNCTION: MW23DFX 0x10045840
void SwayShot(Shot* p_shot, MechScalar* p_x, MechScalar* p_y, MechScalar* p_z)
{
	MechScalar sideZ;
	MechScalar sideY;
	MechScalar sideX;

	sideX = -p_shot->m_velocity[2];
	sideY = 0;
	sideZ = p_shot->m_velocity[0];
	NormalizeVectorGuarded(&sideX, &sideY, &sideZ);
#ifdef MW2_FLOAT_SCALARS
	*p_x += sideX * FixedSin(p_shot->m_swayPhase);
	*p_z += sideZ * FixedSin(p_shot->m_swayPhase);
	*p_y += sideZ * FixedCos(p_shot->m_swayPhase);
	p_shot->m_swayPhase += g_deltaTime * 3.9779005f;
	p_shot->m_swayPhase = FIXED_MOD360(p_shot->m_swayPhase);
#else
	*p_x += FixedMul29(sideX, FixedSin(p_shot->m_swayPhase)) >> 9;
	*p_z += FixedMul29(sideZ, FixedSin(p_shot->m_swayPhase)) >> 9;
	*p_y += FixedMul29(sideZ, FixedCos(p_shot->m_swayPhase)) >> 10;
	p_shot->m_swayPhase += g_deltaTime * 0x3fa57;
	p_shot->m_swayPhase %= 0x1680000;
#endif
}

// Steers a guided missile at (p_x, p_y, p_z) toward its target, and arms its proximity fuse
// within 100 units of it.
// FUNCTION: MW2 0x1006ae5a
// FUNCTION: MW2GBL 0x1007cae1
// FUNCTION: MW2PREMERCS 0x1008638c
// FUNCTION: MW2MATROX 0x1006ba0e
// FUNCTION: MW23DFX 0x10045910
// FUNCTION: MW2ATI 0x10043090
// FUNCTION: MW2S3 0x1001c1b0
// FUNCTION: MW2POWERVR 0x1003ddc0
void GuideMissileToTarget(Shot* p_shot, MechScalar p_x, MechScalar p_y, MechScalar p_z)
{
	MechScalar targetY;
	MechScalar targetZ;
	Player* player;
	MechScalar dy;
	MechScalar dx;
	MechScalar dz;
	GameThing* thing;
	MechScalar distance;
	MechScalar vx;
	MechScalar vy;
	MechScalar vz;
	MechScalar targetX;
#ifdef MW2_WEAPON_FUNCTIONS
	MechScalar rate;
	MechScalar ax;
	MechScalar ay;
	MechScalar az;
#endif

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

		targetX = player->m_position.m_x;
		targetY = player->m_position.m_y;
		targetZ = player->m_position.m_z;
		break;
	case c_shotTargetGameThing:
		thing = &g_gameThings[p_shot->m_target];
		if (thing->m_flags & 4) {
			p_shot->m_steering[0] = p_shot->m_steering[1] = p_shot->m_steering[2] = 0;
			p_shot->m_targetKind = 0;
			return;
		}

		GetStaticObjectPosition(thing->m_staticObject, &targetX, &targetY, &targetZ);
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

#ifdef MW2_FLOAT_SCALARS
	dx /= distance;
	dy /= distance;
	dz /= distance;
#else
	dx = FixedDiv16(dx, distance);
	dy = FixedDiv16(dy, distance);
	dz = FixedDiv16(dz, distance);
#endif
	vx = p_shot->m_velocity[0];
	vy = p_shot->m_velocity[1];
	vz = p_shot->m_velocity[2];
	NormalizeVectorGuarded(&vx, &vy, &vz);
#ifdef MW2_WEAPON_FUNCTIONS
	if (p_shot->m_weaponFlags & c_weaponSlowTurn) {
		rate = FixedDiv16(g_deltaTime, 0x2d4);
		ax = FixedMul16(dx - vx - p_shot->m_steering[0], rate);
		ay = FixedMul16(dy - vy - p_shot->m_steering[1], rate);
		az = FixedMul16(dz - vz - p_shot->m_steering[2], rate);
		p_shot->m_steering[0] += ax;
		p_shot->m_steering[1] += ay;
		p_shot->m_steering[2] += az;
	}
	else {
		p_shot->m_steering[0] = dx - vx;
		p_shot->m_steering[1] = dy - vy;
		p_shot->m_steering[2] = dz - vz;
	}
#else
	p_shot->m_steering[0] = dx - vx;
	p_shot->m_steering[1] = dy - vy;
	p_shot->m_steering[2] = dz - vz;
#endif
	targetY = FixedAtan2(vx, vz);
#ifdef MW2_FLOAT_SCALARS
	targetX = -FixedAsin(vy);
#else
	targetX = -FixedAsin(vy << 13);
#endif
	SetObjRotation(p_shot->m_object, targetX, targetY, 0, 0);
}

// Removes the shot from the world and frees its slot; p_explode spawns the effect its impact
// calls for at (p_x, p_y, p_z), with the camera looking from (p_camX, p_camY, p_camZ).
// FUNCTION: MW2 0x1006b0b4
// FUNCTION: MW2GBL 0x1007cdd2
// FUNCTION: MW2PREMERCS 0x1008667d
// FUNCTION: MW2MATROX 0x1006bc89
// FUNCTION: MW23DFX 0x10045b90
// FUNCTION: MW2ATI 0x10043310
// FUNCTION: MW2S3 0x1001c3f0
// FUNCTION: MW2POWERVR 0x1003e040
void DetonateShot(
#ifdef MW2_WEAPON_FUNCTIONS
#ifndef MW2_SHOT_OWNS_EFFECT
	MechS32 p_owner,
#endif
	Shape* p_shape,
#endif
	MechS32 p_index,
	MechS32 p_explode,
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
	MechScalar p_camX,
	MechScalar p_camY,
	MechScalar p_camZ
)
{
	Shot* shot;
	MechS32 impact;

	shot = &g_shots[p_index];
	if (shot->m_tracked) {
		g_trackedShot = -1;
	}

	HideObjTree(shot->m_object);
	DisableObjTreeCollision(shot->m_object);
	impact = shot->m_impact;
	ResetShotSlot(p_index);
	if (p_explode) {
#ifdef MW2_WEAPON_FUNCTIONS
#ifdef MW2_SHOT_OWNS_EFFECT
		SpawnEffectEx(shot->m_shooter, p_shape, impact, p_x, p_y, p_z, p_camX, p_camY, p_camZ, 0, 0, 0);
#else
		SpawnEffectEx(p_owner, p_shape, impact, p_x, p_y, p_z, p_camX, p_camY, p_camZ, 0, 0, 0);
#endif
#else
		SpawnEffect(shot->m_shooter, impact, p_x, p_y, p_z, p_camX, p_camY, p_camZ);
#endif
	}
}

// FUNCTION: MW2 0x1006b152
// FUNCTION: MW2GBL 0x1007ce7b
// FUNCTION: MW2PREMERCS 0x10086729
// FUNCTION: MW2MATROX 0x1006bd27
// FUNCTION: MW23DFX 0x10045c30
// FUNCTION: MW2ATI 0x100433b0
// FUNCTION: MW2S3 0x1001c490
// FUNCTION: MW2POWERVR 0x1003e0e0
void SpawnEffect(
	MechS32 p_owner,
	MechS32 p_type,
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
	MechScalar p_camX,
	MechScalar p_camY,
	MechScalar p_camZ
)
{
#ifdef MW2_WEAPON_FUNCTIONS
	SpawnEffectEx(p_owner, NULL, p_type, p_x, p_y, p_z, p_camX, p_camY, p_camZ, 0, 0, 0);
#else
	SpawnEffectEx(p_owner, p_type, p_x, p_y, p_z, p_camX, p_camY, p_camZ, 0, 0, 0);
#endif
}

// FUNCTION: MW2 0x1006b18b
// FUNCTION: MW2GBL 0x1007ceb6
// FUNCTION: MW2PREMERCS 0x10086764
// FUNCTION: MW2MATROX 0x1006bd60
// FUNCTION: MW23DFX 0x10045c70
void SpawnRotatedEffect(
	MechS32 p_type,
	MechScalar p_rotX,
	MechScalar p_rotY,
	MechScalar p_rotZ,
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z
)
{
#ifdef MW2_WEAPON_FUNCTIONS
	SpawnEffectEx(-2, NULL, p_type, p_x, p_y, p_z, p_x, p_y, p_z, p_rotX, p_rotY, p_rotZ);
#else
	SpawnEffectEx(-2, p_type, p_x, p_y, p_z, p_x, p_y, p_z, p_rotX, p_rotY, p_rotZ);
#endif
}

// FUNCTION: MW2 0x1006b1c8
// FUNCTION: MW2GBL 0x1007cef5
// FUNCTION: MW2PREMERCS 0x100867a3
// FUNCTION: MW2MATROX 0x1006bd9d
// FUNCTION: MW23DFX 0x10045cc0
// FUNCTION: MW2ATI 0x100433f0
// FUNCTION: MW2S3 0x1001c4d0
// FUNCTION: MW2POWERVR 0x1003e120
void SpawnLaunchEffect(MechS32 p_type, Player* p_player)
{
	g_launchEffectPlayer = p_player;
#ifdef MW2_WEAPON_FUNCTIONS
	SpawnEffectEx(-2, NULL, p_type, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#else
	SpawnEffectEx(-2, p_type, 0, 0, 0, 0, 0, 0, 0, 0, 0);
#endif
}

#ifdef MW2_WEAPON_FUNCTIONS
// GBL's: p_shape is what the shot hit, which an effect type with c_effectAttaches follows (a mech's
// object, the effect at its origin) or, failing that, plays for 181 ticks only. Effect types 0x22 and
// 0x23 are missiles' like 3, and the launch effects are 0x17 to 0x21.
// FUNCTION: MW2GBL 0x1007cf2a
// FUNCTION: MW2PREMERCS 0x100867d8
void SpawnEffectEx(
	MechS32 p_owner,
	Shape* p_shape,
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
)
{
	MechS32 pieces;
	MechScalar distance;
	MechScalar dy;
	MechS32 flags;
	MechS32 chainType;
	MechS32 chain;
	MechScalar cameraX;
	MechScalar dz;
	MechS32 quiet;
	MechScalar current;
	EffectInfo* info;
	MechS32 slot;
	MechS32 i;
	MechScalar cameraZ;
	MechS32 volume;
	MechS32 count;
	MechS32 closer;
	MechS32 take;
	Effect* effect;
	MechScalar dx;
	SceneObject* object;

	effect = NULL;
	info = NULL;
	quiet = 0;
	chain = 0;
	chainType = -1;
	if (p_type < 0) {
		return;
	}

	flags = p_type & 0xff00;
	p_type &= 0xff;
	if (p_type >= EFFECT_TYPE_COUNT) {
		return;
	}

	if (flags & 0x4000) {
		quiet = 1;
	}

	switch (p_type) {
	case 5:
		if (flags & c_impactMech) {
			p_x = p_camX;
			p_z = p_camZ;
			p_type = 8;
		}
		else if (flags & c_impactGround) {
			p_type = 9;
		}
		else if (flags & 0x2000) {
			p_type = 0x12;
		}
		break;
	case 3:
	case 0x22:
	case 0x23:
		if (flags & c_impactGround) {
			p_type = 0xc;
		}
		else if (flags & 0x2000) {
			p_type = 0x11;
		}
		else if (flags & c_impactThing) {
			p_type = 0x13;
		}
		chain = 1;
		chainType = 0x10b;
		break;
	case 4:
		if (flags & c_impactGround) {
			p_type = 0xc;
		}
		else if (flags & 0x2000) {
			p_type = 0x11;
		}
		else if (flags & c_impactThing) {
			p_type = 0x14;
		}
		chain = 1;
		chainType = 0x10b;
		break;
	case 0:
		if (flags & 0x2000) {
			p_type = 0xe;
		}
		else if (flags & c_impactGround) {
			p_type = 9;
		}
		break;
	case 1:
		if (flags & 0x2000) {
			p_type = 0xf;
		}
		else if (flags & c_impactGround) {
			p_type = 9;
		}
		break;
	case 2:
		if (flags & 0x2000) {
			p_type = 0x10;
		}
		else if (flags & c_impactGround) {
			p_type = 9;
		}
		break;
	case 6:
		if (flags & c_impactGround) {
			p_type = 0x15;
		}
		break;
	case 0xb:
		if (flags & 0x1000) {
			pieces = 0x10;
		}
		else if (flags & c_impactMech) {
			pieces = 4;
		}
		else if (flags & c_impactThing) {
			pieces = 4;
		}
		else {
			pieces = 8;
		}

		ScatterDebris(p_x, p_y, p_z, pieces);
		return;
	default:
		break;
	}

	info = &g_effectInfo[p_type];
	dx = g_eyepoint->m_x - p_x;
	dy = g_eyepoint->m_y - p_y;
	dz = g_eyepoint->m_z - p_z;
	if (info->m_needsObject || info->m_camera) {
		count = 0;
		slot = -1;
		for (i = 0; i < EFFECT_COUNT; i++) {
			effect = &g_effects[i];
			if (effect->m_type == p_type) {
				if (effect->m_active) {
					if ((effect->m_type < 0x17 || effect->m_type > 0x21) && !(info->m_flags & c_effectUncounted) &&
						ApproximateVectorLength(
							effect->m_position[0] - p_x,
							effect->m_position[1] - p_y,
							effect->m_position[2] - p_z
						) < 500) {
						count++;
						if (g_lodQuality != 1 || count > 2) {
							return;
						}
					}
				}
				else if (slot < 0 && (!info->m_needsObject || effect->m_object)) {
					slot = i;
				}
			}
		}

		if (slot < 0) {
			return;
		}

		effect = &g_effects[slot];
		effect->m_active = 1;
		effect->m_position[0] = p_x;
		effect->m_position[1] = p_y;
		effect->m_position[2] = p_z;
		effect->m_timeLeft = info->m_duration + g_deltaTime;
		effect->m_owner = p_owner;
		effect->m_attachedShape = NULL;
		if (effect->m_object) {
			if (info->m_flags & c_effectAttaches) {
				if (p_shape && !(p_shape->m_kind & 0x400) && (p_shape->m_kind & 0x100) &&
					(object = GetShapeObject(p_shape))) {
					p_x = p_y = p_z = 0;
					effect->m_attachedShape = p_shape;
					AttachObj(effect->m_object, object);
				}
				else {
					effect->m_timeLeft = g_deltaTime + 0xb5;
				}
			}

			if (p_type >= 0x17 && p_type <= 0x21 && g_launchEffectPlayer) {
				PlaceAtFiringObj(g_launchEffectPlayer, effect->m_object);
				g_launchEffectPlayer = NULL;
			}
			else {
				SetObjRotation(effect->m_object, p_rotX, p_rotY, p_rotZ, 0);
				SetObjPosition(effect->m_object, p_x, p_y, p_z);
			}

			ShowObjTree(effect->m_object);
			DisableObjTreeCollision(effect->m_object);
			UpdateObj(effect->m_object);
			if (effect->m_animation != -1) {
				SetAnimFrame(effect->m_animation, 0);
				SetAnimMode(effect->m_animation, 2);
			}
		}

		if (info->m_camera && g_effectCameraEnabled) {
			closer = 0;
			take = 1;
			if (g_effectCameraActive) {
				distance = dx * dx + dz * dz;
				cameraX = g_eyepoint->m_lightX - g_eyepoint->m_x;
				cameraZ = g_eyepoint->m_lightZ - g_eyepoint->m_z;
				current = cameraX * cameraX + cameraZ * cameraZ;
				if (current > distance) {
					closer = 1;
				}
				else {
					take = 0;
				}
			}

			if (take) {
				if (!closer) {
					g_effectCameraActive = 1;
					g_lightFollowsObject = 0;
					g_savedLightX = g_eyepoint->m_lightX;
					g_savedLightY = g_eyepoint->m_lightY;
					g_savedLightZ = g_eyepoint->m_lightZ;
					g_savedDirectionalLight = g_eyepoint->m_directionalLight;
					g_savedAmbientLight = g_eyepoint->m_ambientLight;
					g_eyepoint->m_ambientLight -= 10;
					if (g_eyepoint->m_ambientLight > 0xff || g_eyepoint->m_ambientLight < 0) {
						g_eyepoint->m_ambientLight = 0x40;
					}

					g_savedDistanceFade = g_renderSettings.m_distanceFade;
					g_eyepoint->m_directionalLight = 0;
					g_renderSettings.m_distanceFade = 0;
				}

				g_effectCameraEffect = slot;
				effect->m_hasCamera = 1;
				g_eyepoint->m_lightX = p_camX;
				g_eyepoint->m_lightY = p_camY;
				g_eyepoint->m_lightZ = p_camZ;
				if (info->m_flash > -1) {
					StartPaletteFlash(info->m_flash, info->m_duration, 1);
				}
			}
		}
	}

	if (chain) {
		SpawnEffect(p_owner, chainType, p_x, p_y, p_z, p_camX, p_camY, p_camZ);
	}

	if (info->m_sound > 0 && (info->m_soundChance == -1 || RandomIntBelow(100) < info->m_soundChance)) {
		if (!GetViewMode() && !quiet) {
			volume = 1;
		}
		else {
			volume = 0;
		}

		PlaySoundAt(dx, dy, dz, info->m_sound, volume);
	}
}
#else
// Spawns an effect. The low byte of p_type is the effect; the high byte says what a shot hit
// (c_impactMech etc.), which picks a variant of it, and 0x4000 keeps its sound at full volume.
// The effect takes a free slot of its type (none if three of them already burn within 500
// units), and the effect camera may follow it from (p_camX, p_camY, p_camZ).
// FUNCTION: MW2 0x1006b1fb
// FUNCTION: MW2MATROX 0x1006bdd0
// FUNCTION: MW23DFX 0x10045d00
// FUNCTION: MW2ATI 0x10043430
// FUNCTION: MW2S3 0x1001c510
// FUNCTION: MW2POWERVR 0x1003e160
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
)
{
	MechS32 pieces;
	MechScalar distance;
	MechScalar dy;
	MechS32 flags;
	MechS32 chainType;
	MechS32 chain;
	MechScalar cameraX;
	MechScalar dz;
	MechS32 quiet;
	MechScalar current;
	EffectInfo* info;
	MechS32 slot;
	MechS32 i;
	MechScalar cameraZ;
	MechS32 volume;
	MechS32 count;
	MechS32 closer;
	MechS32 take;
	Effect* effect;
	MechScalar dx;
#ifdef MW2_UNROTATED_EFFECTS
	MechScalar scale;
#endif

	effect = NULL;
	info = NULL;
	quiet = 0;
	chain = 0;
	chainType = -1;
	if (p_type < 0) {
		return;
	}

	flags = p_type & 0xff00;
	p_type &= 0xff;
	if (p_type >= EFFECT_TYPE_COUNT) {
		return;
	}

	if (flags & 0x4000) {
		quiet = 1;
	}

	switch (p_type) {
	case 5:
		if (flags & c_impactMech) {
			p_x = p_camX;
			p_z = p_camZ;
			p_type = 8;
		}
		else if (flags & c_impactGround) {
			p_type = 9;
		}
		else if (flags & 0x2000) {
			p_type = 0x12;
		}
		break;
	case 3:
		if (flags & c_impactGround) {
			p_type = 0xc;
		}
		else if (flags & 0x2000) {
			p_type = 0x11;
		}
		else if (flags & c_impactThing) {
			p_type = 0x13;
		}
		chain = 1;
		chainType = 0x10b;
		break;
	case 4:
		if (flags & c_impactGround) {
			p_type = 0xc;
		}
		else if (flags & 0x2000) {
			p_type = 0x11;
		}
		else if (flags & c_impactThing) {
			p_type = 0x14;
		}
		chain = 1;
		chainType = 0x10b;
		break;
	case 0:
		if (flags & 0x2000) {
			p_type = 0xe;
		}
		else if (flags & c_impactGround) {
			p_type = 9;
		}
		break;
	case 1:
		if (flags & 0x2000) {
			p_type = 0xf;
		}
		else if (flags & c_impactGround) {
			p_type = 9;
		}
		break;
	case 2:
		if (flags & 0x2000) {
			p_type = 0x10;
		}
		else if (flags & c_impactGround) {
			p_type = 9;
		}
		break;
	case 6:
		if (flags & c_impactGround) {
			p_type = 0x15;
		}
		break;
	case 0xb:
		if (flags & 0x1000) {
			pieces = 0x10;
		}
		else if (flags & c_impactMech) {
			pieces = 4;
		}
		else if (flags & c_impactThing) {
			pieces = 4;
		}
		else {
			pieces = 8;
		}

		ScatterDebris(p_x, p_y, p_z, pieces);
		return;
	default:
		break;
	}

	info = &g_effectInfo[p_type];
	dx = g_eyepoint->m_x - p_x;
	dy = g_eyepoint->m_y - p_y;
	dz = g_eyepoint->m_z - p_z;
	if (info->m_needsObject || info->m_camera) {
		count = 0;
		slot = -1;
		for (i = 0; i < EFFECT_COUNT; i++) {
			effect = &g_effects[i];
			if (effect->m_type == p_type) {
				if (effect->m_active) {
					if (effect->m_type < 0x17 && ApproximateVectorLength(
													 effect->m_position[0] - p_x,
													 effect->m_position[1] - p_y,
													 effect->m_position[2] - p_z
												 ) < 500) {
						count++;
						if (g_lodQuality != 1 || count > 2) {
							return;
						}
					}
				}
				else if (slot < 0 && (!info->m_needsObject || effect->m_object)) {
					slot = i;
				}
			}
		}

		if (slot < 0) {
			return;
		}

		effect = &g_effects[slot];
		effect->m_active = 1;
		effect->m_position[0] = p_x;
		effect->m_position[1] = p_y;
		effect->m_position[2] = p_z;
		effect->m_timeLeft = info->m_duration + g_deltaTime;
		effect->m_owner = p_owner;
		if (effect->m_object) {
			if (p_type >= 0x17 && g_launchEffectPlayer) {
				PlaceAtFiringObj(g_launchEffectPlayer, effect->m_object);
				g_launchEffectPlayer = NULL;
			}
			else {
#ifdef MW2_UNROTATED_EFFECTS
				if (flags & c_impactThing) {
					scale = effect->m_object->m_shape->m_radius / ApproximateVectorLength(dx, dy, dz);
					p_x += dx * scale;
					p_y += dy * scale;
					p_z += dz * scale;
				}
#else
				SetObjRotation(effect->m_object, p_rotX, p_rotY, p_rotZ, 0);
#endif
				SetObjPosition(effect->m_object, p_x, p_y, p_z);
			}

			ShowObjTree(effect->m_object);
			DisableObjTreeCollision(effect->m_object);
			UpdateObj(effect->m_object);
			if (effect->m_animation != -1) {
				SetAnimFrame(effect->m_animation, 0);
				SetAnimMode(effect->m_animation, 2);
			}
		}

		if (info->m_camera && g_effectCameraEnabled) {
			closer = 0;
			take = 1;
			if (g_effectCameraActive) {
				distance = dx * dx + dz * dz;
				cameraX = g_eyepoint->m_lightX - g_eyepoint->m_x;
#if defined(MW2_ATI)
				// The ATI edition assigns cameraZ inside the sum, which it adds from cameraZ's square.
				current = (cameraZ = g_eyepoint->m_lightZ - g_eyepoint->m_z) * cameraZ + cameraX * cameraX;
				if (current > distance) {
#elif defined(MW2_3DFX)
				cameraZ = g_eyepoint->m_lightZ - g_eyepoint->m_z;
				if ((current = cameraX * cameraX + cameraZ * cameraZ) > distance) {
#else
				cameraZ = g_eyepoint->m_lightZ - g_eyepoint->m_z;
				current = cameraX * cameraX + cameraZ * cameraZ;
				if (current > distance) {
#endif
					closer = 1;
				}
				else {
					take = 0;
				}
			}

			if (take) {
				if (!closer) {
					g_effectCameraActive = 1;
					g_lightFollowsObject = 0;
					g_savedLightX = g_eyepoint->m_lightX;
					g_savedLightY = g_eyepoint->m_lightY;
					g_savedLightZ = g_eyepoint->m_lightZ;
					g_savedDirectionalLight = g_eyepoint->m_directionalLight;
					g_savedAmbientLight = g_eyepoint->m_ambientLight;
					g_eyepoint->m_ambientLight -= 10;
					if (g_eyepoint->m_ambientLight > 0xff || g_eyepoint->m_ambientLight < 0) {
						g_eyepoint->m_ambientLight = 0x40;
					}

					g_savedDistanceFade = g_renderSettings.m_distanceFade;
					g_eyepoint->m_directionalLight = 0;
					g_renderSettings.m_distanceFade = 0;
				}

				g_effectCameraEffect = slot;
				effect->m_hasCamera = 1;
				g_eyepoint->m_lightX = p_camX;
				g_eyepoint->m_lightY = p_camY;
				g_eyepoint->m_lightZ = p_camZ;
				if (info->m_flash > -1) {
					StartPaletteFlash(info->m_flash, info->m_duration, 1);
				}
			}
		}
	}

	if (chain) {
		SpawnEffect(p_owner, chainType, p_x, p_y, p_z, p_camX, p_camY, p_camZ);
	}

	if (info->m_sound > 0 && (info->m_soundChance == -1 || RandomIntBelow(100) < info->m_soundChance)) {
		if (!GetViewMode() && !quiet) {
			volume = 1;
		}
		else {
			volume = 0;
		}

		PlaySoundAt(dx, dy, dz, info->m_sound, volume);
	}
}
#endif

#ifdef MW2_WEAPON_FUNCTIONS
// GBL's: an attached effect follows its object, each effect type has its own damage and heat, and
// the nuke's (0x16) and effect 0x23's radius grows as they burn, damaging mechs whatever the splash
// damage setting.
// FUNCTION: MW2GBL 0x1007d7bd
// FUNCTION: MW2PREMERCS 0x10087070
void UpdateEffects(void)
{
	MechS32 release;
	MechS32 i;
	MechS32 restore;
	Effect* effect;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar radius;
	EffectInfo* info;

	info = NULL;
	UpdateNuke();
	if (g_difficulty->m_heatTracking) {
		HeatMechsNearFires();
	}

	for (i = 0; i < EFFECT_COUNT; i++) {
		effect = &g_effects[i];
		if (effect->m_active) {
			info = &g_effectInfo[effect->m_type];
			if (info->m_flags & c_effectAttaches) {
				GetObjPosition(
					effect->m_object,
					&effect->m_position[0],
					&effect->m_position[1],
					&effect->m_position[2]
				);
			}

			if (g_effectInfo[effect->m_type].m_damages) {
				x = effect->m_position[0];
				y = effect->m_position[1];
				z = effect->m_position[2];
				switch (effect->m_type) {
				case 0x16:
					if (effect->m_timeLeft == info->m_duration) {
						StartPaletteFade(0x12, effect->m_timeLeft, 2);
					}

					radius = (info->m_duration - effect->m_timeLeft) * 0x1c;
					break;
				case 0x23:
					radius = (info->m_duration - effect->m_timeLeft) * 0x18;
					break;
				default:
					radius = effect->m_object->m_shape->m_radius;
					break;
				}

				if (g_difficulty->m_splashDamage || effect->m_type == 0x16 || effect->m_type == 0x23) {
					DamageMechsInRadius(effect->m_owner, x, y, z, radius, info->m_damage, info->m_heat);
				}

				DamageThingsInRadius(effect->m_owner, x, y, z, radius, info->m_damage);
				DamageChunksInRadius(x, y, z, radius, info->m_damage);
			}

			effect->m_timeLeft -= g_deltaTime;
			if (effect->m_timeLeft <= 0) {
				release = 1;
				restore = 0;
				if (effect->m_hasCamera && g_effectCameraActive && i == g_effectCameraEffect) {
					if (!GetPaletteFadeSteps()) {
						restore = 1;
					}
					else {
						release = 0;
					}
				}

				if (release) {
					if (restore) {
						g_lightFollowsObject = 1;
						g_eyepoint->m_lightX = g_savedLightX;
						g_eyepoint->m_lightY = g_savedLightY;
						g_eyepoint->m_lightZ = g_savedLightZ;
						g_eyepoint->m_directionalLight = g_savedDirectionalLight;
						g_eyepoint->m_ambientLight = g_savedAmbientLight;
						g_renderSettings.m_distanceFade = g_savedDistanceFade;
						effect->m_hasCamera = 0;
						g_effectCameraActive = 0;
						g_effectCameraEffect = -1;
					}

					if (effect->m_object) {
						if (GetObjParent(effect->m_object)) {
							DetachObj(effect->m_object);
						}

						HideObjTree(effect->m_object);
					}

					if (effect->m_animation > 0) {
						SetAnimMode(effect->m_animation, 0);
						SetAnimFrame(effect->m_animation, 0);
					}

					ResetEffectSlot(i);
				}
			}
		}
	}
}
#else
// Counts down the effects, lets the damaging ones hurt what is near them, and frees them when
// their time runs out, giving the camera back if one had it.
// GetPaletteFadeSteps (palette.c) has no MW2MATROX annotation yet.
// FUNCTION: MW2 0x1006b99a
// FUNCTION: MW2MATROX 0x1006c584
// FUNCTION: MW23DFX 0x10046490
// FUNCTION: MW2ATI 0x10043bc0
// FUNCTION: MW2S3 0x1001cca0
// FUNCTION: MW2POWERVR 0x1003e930
void UpdateEffects(void)
{
	MechS32 release;
	MechS32 i;
	MechS32 restore;
	Effect* effect;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar radius;

	UpdateNuke();
	if (g_difficulty->m_heatTracking) {
		HeatMechsNearFires();
	}

	for (i = 0; i < EFFECT_COUNT; i++) {
		effect = &g_effects[i];
		if (effect->m_active) {
			if (g_effectInfo[effect->m_type].m_damages) {
				x = effect->m_position[0];
				y = effect->m_position[1];
				z = effect->m_position[2];
				radius = effect->m_object->m_shape->m_radius;
				if (g_difficulty->m_splashDamage) {
					DamageMechsInRadius(effect->m_owner, x, y, z, radius, FIXED_RAW(0x100));
				}

				DamageThingsInRadius(effect->m_owner, x, y, z, radius, FIXED_RAW(0x100));
				DamageChunksInRadius(x, y, z, radius, FIXED_RAW(0x100));
			}

			effect->m_timeLeft -= g_deltaTime;
			if (effect->m_timeLeft <= 0) {
				release = 1;
				restore = 0;
				if (effect->m_hasCamera && g_effectCameraActive && i == g_effectCameraEffect) {
					if (!GetPaletteFadeSteps()) {
						restore = 1;
					}
					else {
						release = 0;
					}
				}

				if (release) {
					if (restore) {
						g_lightFollowsObject = 1;
						g_eyepoint->m_lightX = g_savedLightX;
						g_eyepoint->m_lightY = g_savedLightY;
						g_eyepoint->m_lightZ = g_savedLightZ;
						g_eyepoint->m_directionalLight = g_savedDirectionalLight;
						g_eyepoint->m_ambientLight = g_savedAmbientLight;
						g_renderSettings.m_distanceFade = g_savedDistanceFade;
						effect->m_hasCamera = 0;
						g_effectCameraActive = 0;
						g_effectCameraEffect = -1;
					}

					if (effect->m_object) {
						HideObjTree(effect->m_object);
					}

					if (effect->m_animation > 0) {
						SetAnimMode(effect->m_animation, 0);
						SetAnimFrame(effect->m_animation, 0);
					}

					ResetEffectSlot(i);
				}
			}
		}
	}
}
#endif

// Damages every mech within p_radius (plus its own radius) of (p_x, p_y, p_z) by p_rate per
// tick in each of its sections, twice as much when its power is off.
// FUNCTION: MW2 0x1006bc13
// FUNCTION: MW2GBL 0x1007db62
// FUNCTION: MW2PREMERCS 0x100873ba
// FUNCTION: MW2MATROX 0x1006c803
// FUNCTION: MW23DFX 0x10046700
// FUNCTION: MW2ATI 0x10043e30
// FUNCTION: MW2S3 0x1001cf10
// FUNCTION: MW2POWERVR 0x1003eba0
void DamageMechsInRadius(
	MechS32 p_owner,
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
	MechScalar p_radius,
	MechScalar p_rate
#ifdef MW2_WEAPON_FUNCTIONS
	,
	MechScalar p_heatRate
#endif
)
{
	Player* player;
	MechScalar reach;
	Mech* mech;
	MechS32 i;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;
	MechScalar damage;
#ifdef MW2_WEAPON_FUNCTIONS
	MechScalar heat;
#endif

	i = g_playerCount;
	while (i--) {
		if (g_netRole && i != g_localPlayerId) {
			continue;
		}

		player = g_players[i];
		mech = player->m_mech;
		if (mech->m_flags & 0x100) {
			continue;
		}

		dx = player->m_position.m_x - p_x;
		dy = player->m_position.m_y - p_y;
		dz = player->m_position.m_z - p_z;
		reach = mech->m_radius + p_radius;
		if (ApproximateVectorLength(dx, dy, dz) < reach) {
			damage = p_rate * g_deltaTime;
#ifdef MW2_WEAPON_FUNCTIONS
			heat = p_heatRate * g_deltaTime;
#endif
			if (mech->m_powerState == 4) {
#ifdef MW2_FLOAT_SCALARS
				damage *= 2;
#else
				damage <<= 1;
#endif
			}

#ifdef MW2_RADIUS_HEAD_DAMAGE_HALF
			// Half the damage to the head, where 1.1 gives a quarter.
			ApplyDamageToMech(p_owner, mech, damage / 2, 1);
#else
			ApplyDamageToMech(p_owner, mech, damage >> 2, 1);
#endif
			ApplyDamageToMech(p_owner, mech, damage, 2);
			ApplyDamageToMech(p_owner, mech, damage, 3);
			ApplyDamageToMech(p_owner, mech, damage, 4);
			ApplyDamageToMech(p_owner, mech, damage, 5);
			ApplyDamageToMech(p_owner, mech, damage, 6);
			ApplyDamageToMech(p_owner, mech, damage, 7);
			ApplyDamageToMech(p_owner, mech, damage, 8);
#ifdef MW2_WEAPON_FUNCTIONS
			mech->m_externalHeat += heat;
#endif
		}
	}
}

// Damages every game thing whose shape comes within p_radius of (p_x, p_y, p_z).
// FUNCTION: MW2 0x1006bdb4
// FUNCTION: MW2GBL 0x1007dd1d
// FUNCTION: MW2PREMERCS 0x10087574
// FUNCTION: MW2MATROX 0x1006c9d5
// FUNCTION: MW23DFX 0x100468b0
// FUNCTION: MW2ATI 0x10043fe0
// FUNCTION: MW2S3 0x1001d0b0
// FUNCTION: MW2POWERVR 0x1003ed50
void DamageThingsInRadius(
	MechS32 p_owner,
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
	MechScalar p_radius,
	MechScalar p_rate
)
{
	GameThing* thing;
	MechS32 i;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;
	Shape* shape;
	MechScalar reach;
	MechScalar radius;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar distance;

	i = g_gameThingCount;
	while (i--) {
		thing = &g_gameThings[i];
		if (thing->m_flags & 4) {
			continue;
		}

		shape = GetStaticObjectShape(thing->m_staticObject);
		if (!shape) {
			continue;
		}

		radius = GetShapeBounds(shape, &x, &y, &z);
		dx = x - p_x;
		dy = y - p_y;
		dz = z - p_z;
		reach = radius + p_radius;
#ifdef MW2_MATROX
		if ((distance = ApproximateVectorLength(dx, dy, dz)) < reach) {
#else
		distance = ApproximateVectorLength(dx, dy, dz);
		if (distance < reach) {
#endif
#ifdef MW2_FLOAT_SCALARS
			DamageGameThing(p_owner, shape, p_rate * g_deltaTime, x, y, z);
#else
			DamageGameThing(p_owner, shape, FixedMul16(p_rate, g_deltaTime), x, y, z);
#endif
		}
	}
}

// FUNCTION: MW2 0x1006beb5
// FUNCTION: MW2GBL 0x1007de1e
// FUNCTION: MW2PREMERCS 0x10087675
// FUNCTION: MW2MATROX 0x1006cad9
// FUNCTION: MW23DFX 0x100469b0
// FUNCTION: MW2ATI 0x100440e0
// FUNCTION: MW2S3 0x1001d1b0
// FUNCTION: MW2POWERVR 0x1003ee50
MechScalar* GetTrackedShotView(void)
{
	if (g_trackedShot != -1 && g_shots[g_trackedShot].m_tracked) {
		return g_trackedShotView;
	}
	else {
		g_trackedShot = -1;
		return NULL;
	}
}

// Starts tracking the local player's last shot, if it is still in flight.
// FUNCTION: MW2 0x1006bf05
// FUNCTION: MW2GBL 0x1007de71
// FUNCTION: MW2PREMERCS 0x100876c8
// FUNCTION: MW2MATROX 0x1006cb29
// FUNCTION: MW23DFX 0x10046a00
// FUNCTION: MW2ATI 0x10044130
// FUNCTION: MW2S3 0x1001d200
// FUNCTION: MW2POWERVR 0x1003eea0
MechS32 TrackLastShot(void)
{
	if (g_lastLocalMissile > 0 && g_shots[g_lastLocalMissile].m_flags &&
		g_shots[g_lastLocalMissile].m_shooter == g_localPlayerId) {
		g_trackedShot = g_lastLocalMissile;
		g_shots[g_lastLocalMissile].m_tracked = 1;
		return 1;
	}

	g_trackedShot = -1;
	return 0;
}

// Counts the destruction of game thing p_index for the last shooter's side and team, and
// removes it.
// FUNCTION: MW2 0x1006bf8c
// FUNCTION: MW2GBL 0x1007df01
// FUNCTION: MW2PREMERCS 0x10087758
// FUNCTION: MW2MATROX 0x1006cbb0
// FUNCTION: MW23DFX 0x10046a90
// FUNCTION: MW2ATI 0x100441c0
// FUNCTION: MW2S3 0x1001d290
// FUNCTION: MW2POWERVR 0x1003ef30
void KillGameThing(MechU32 p_index)
{
	GameThing* thing;

	thing = &g_gameThings[p_index];
	if (g_aimedShape && (g_aimedShape->m_kind & 0x200) && g_aimedShape->m_owner == p_index) {
		g_aimedShape = NULL;
	}

	if (thing->m_flags & 4) {
		return;
	}

#ifdef MW2_THING_KILLS_FORGET_SHOOTER
	if (g_lastHitShooter != -1) {
#else
	if (g_lastHitShooter >= 0) {
#endif
		if (g_lastHitShooter == g_localPlayerId) {
			switch (GetThingSide(p_index)) {
			case 0:
				g_careerRecord.m_directFriendlyThingKills++;
				break;
			case 2:
				g_careerRecord.m_directNeutralThingKills++;
				break;
			case 1:
				g_careerRecord.m_directThingKills++;
				break;
			}
		}

#ifdef MW2_THING_KILLS_FORGET_SHOOTER
		switch (GetThingSide(p_index)) {
		case 0:
			g_careerRecord.m_teamFriendlyThingKills++;
			break;
		case 2:
			g_careerRecord.m_teamNeutralThingKills++;
			break;
		case 1:
			g_careerRecord.m_teamThingKills++;
			break;
		}

		g_lastHitShooter = -1;
#else
		if (g_players[g_localPlayerId]->m_team == g_players[g_lastHitShooter]->m_team) {
			switch (GetThingSide(p_index)) {
			case 0:
				g_careerRecord.m_teamFriendlyThingKills++;
				break;
			case 2:
				g_careerRecord.m_teamNeutralThingKills++;
				break;
			case 1:
				g_careerRecord.m_teamThingKills++;
				break;
			}
		}
#endif
	}

	if (thing->m_flags & 0x40) {
		FUN_1001cdd1();
	}

	DestroyThingObject(thing);
}

// Takes p_damage off the hit points of the game thing p_shape belongs to; when they run out,
// blows it up and counts it destroyed.
// FUNCTION: MW2 0x1006c11c
// FUNCTION: MW2GBL 0x1007e076
// FUNCTION: MW2PREMERCS 0x100878e8
// FUNCTION: MW2MATROX 0x1006cd40
// FUNCTION: MW23DFX 0x10046c20
// FUNCTION: MW2ATI 0x10044350
// FUNCTION: MW2S3 0x1001d420
// FUNCTION: MW2POWERVR 0x1003f0c0
void DamageGameThing(
	MechS32 p_owner,
	Shape* p_shape,
	MechScalar p_damage,
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z
)
{
	MechU32 index;
	GameThing* thing;
#ifdef MW2_BUILDINGS_FALL_AT_ONCE
	MechS32 building;
#endif

	if (!p_shape) {
		return;
	}

	index = p_shape->m_owner;
	thing = &g_gameThings[index];
	if (thing->m_flags & 4) {
		return;
	}

#ifdef MW2_BUILDINGS_FALL_AT_ONCE
	building = (GetStaticObjectShape(thing->m_staticObject)->m_kind & 0xf0) == 0xb0;
#endif
	if (!thing->m_hitPoints) {
		return;
	}

#ifdef MW2_BUILDINGS_FALL_AT_ONCE
	if (building) {
		thing->m_hitPoints = 0;
	}
	else {
		thing->m_hitPoints -= (MechS32) p_damage;
	}

	if (thing->m_hitPoints <= 0) {
		thing->m_hitPoints = 0;
		if (building) {
			SpawnEffect(p_owner, 0xd, p_x, p_y, p_z, p_x, p_y, p_z);
		}
		else {
			SpawnEffect(p_owner, 3, p_x, p_y, p_z, p_x, p_y, p_z);
		}

		SpawnEffect(p_owner, 0x20b, p_x, p_y, p_z, p_x, p_y, p_z);
		KillGameThing(index);
	}
#else
#ifdef MW2_SHOTS_FELL_BUILDINGS
	if ((p_shape->m_kind & 0xf0) == 0xb0) {
		thing->m_hitPoints = 0;
	}
	else {
		thing->m_hitPoints -= (MechS32) p_damage;
	}
#else
	thing->m_hitPoints -= (MechS32) p_damage;
#endif
	if (thing->m_hitPoints <= 0) {
		thing->m_hitPoints = 0;
		if ((p_shape->m_kind & 0xf0) == 0xb0) {
			SpawnEffect(p_owner, 0xd, p_x, p_y, p_z, p_x, p_y, p_z);
		}
		else {
			SpawnEffect(p_owner, 3, p_x, p_y, p_z, p_x, p_y, p_z);
			SpawnEffect(p_owner, 0x20b, p_x, p_y, p_z, p_x, p_y, p_z);
		}

		KillGameThing(index);
	}
#endif
}

// Shows up to p_count free effects of type 0xb (all when 0) at (p_x, p_y, p_z) as debris.
// FUNCTION: MW2 0x1006c237
// FUNCTION: MW2GBL 0x1007e1d3
// FUNCTION: MW2PREMERCS 0x10087a27
// FUNCTION: MW2MATROX 0x1006ce62
// FUNCTION: MW23DFX 0x10046d40
// FUNCTION: MW2ATI 0x10044470
// FUNCTION: MW2S3 0x1001d540
// FUNCTION: MW2POWERVR 0x1003f1e0
void ScatterDebris(MechScalar p_x, MechScalar p_y, MechScalar p_z, MechS32 p_count)
{
	MechS32 shown;
	MechS32 piece;
	Effect* effect;
	MechS32 i;
	SceneObject* object;

	effect = NULL;
	object = NULL;
	shown = 0;
	if (!p_count) {
		p_count = EFFECT_COUNT;
	}

	for (i = 0; i < EFFECT_COUNT && shown < p_count; i++) {
		effect = &g_effects[i];
		if (!effect->m_active && effect->m_type == 0xb && effect->m_object) {
			object = effect->m_object;
			piece = AddDebrisPiece(object, 1);
			if (piece != -1) {
				effect->m_active = 1;
				effect->m_timeLeft = g_effectInfo[0xb].m_duration;
				SetObjPosition(object, p_x, p_y, p_z);
				ThrowDebrisPiece(piece);
#ifndef MW2_DEBRIS_NOT_SHOWN
				ShowObjTree(object);
#endif
				UpdateObj(object);
				shown++;
			}
		}
	}
}

// FUNCTION: MW2 0x1006c345
// FUNCTION: MW2GBL 0x1007e2e6
// FUNCTION: MW2PREMERCS 0x10087b2e
// FUNCTION: MW2MATROX 0x1006cf70
// FUNCTION: MW23DFX 0x10046e50
// FUNCTION: MW2ATI 0x10044580
// FUNCTION: MW2S3 0x1001d650
// FUNCTION: MW2POWERVR 0x1003f2f0
void SaveCareerRecord(void)
{
	WriteCareerRecordFile("MW2CAR.CFG", &g_careerRecord);
}

// Heats up the mechs near burning game things (shapes of type 0x10), the more the closer.
// MW2MATROX: in floats, with the heat scaled by FIXED_TO_SCALAR and a quarter where 1.1 shifts
// by 3.
// FUNCTION: MW2 0x1006c362
// FUNCTION: MW2GBL 0x1007e303
// FUNCTION: MW2PREMERCS 0x10087b4b
// FUNCTION: MW2MATROX 0x1006cf8d
// FUNCTION: MW23DFX 0x10046e70
// FUNCTION: MW2ATI 0x100445a0
// FUNCTION: MW2S3 0x1001d670
// FUNCTION: MW2POWERVR 0x1003f310
void HeatMechsNearFires(void)
{
	Shape* root;
	Player* player;
	MechScalar radius;
	MechScalar distance;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	Mech* mech;
	MechScalar dx;
	MechScalar dy;
	Shape* shape;
	MechScalar dz;
	MechScalar heat;
	MechS32 i;
	MechScalar reach;

	root = g_sceneShapes;
	if (!root) {
		return;
	}

	for (shape = root->m_next; shape; shape = shape->m_next) {
		if ((shape->m_kind & 0xf0) != 0x10) {
			continue;
		}

		radius = GetShapeBounds(shape, &x, &y, &z) * 2;
		i = g_playerCount;
		while (i--) {
			if (g_netRole && i != g_localPlayerId) {
				continue;
			}

			player = g_players[i];
			if ((player->m_flags & 2) || (player->m_flags & 4)) {
				continue;
			}

			mech = player->m_mech;
			dx = player->m_position.m_x - x;
			dy = player->m_position.m_y - y;
			dz = player->m_position.m_z - z;
			reach = mech->m_radius + radius;
			distance = ApproximateVectorLength(dx, dy, dz);
			if (distance < reach) {
#ifdef MW2_FLOAT_SCALARS
				heat = FIXED_TO_SCALAR(radius);
				heat = (heat - heat * distance / reach) * 0.25f;
#else
				heat = radius;
				heat = heat - heat * distance / reach >> 3;
#endif
				mech->m_deltaHeat += heat * g_deltaTime;
			}
		}
	}
}

// Sets off a nuke at the player's position: the palette flash, the effect, and the blast that
// grows over the next ticks (UpdateNuke).
// FUNCTION: MW2 0x1006c4e2
// FUNCTION: MW2GBL 0x1007e484
// FUNCTION: MW2PREMERCS 0x10087d08
// FUNCTION: MW2MATROX 0x1006d136
// FUNCTION: MW23DFX 0x10047020
// FUNCTION: MW2ATI 0x10044750
// FUNCTION: MW2S3 0x1001d800
// FUNCTION: MW2POWERVR 0x1003f4c0
void StartNuke(Player* p_player)
{
	MechS32 i;
	MechS32 duration;
	Effect* effect;
	Shape* shape;

	duration = g_effectInfo[0x16].m_duration * 2;
#ifdef MW2_NUKE_RADIUS
	g_nukeMaxRadius = MW2_NUKE_RADIUS;
#else
	g_nukeMaxRadius = 400000;
#endif
	i = EFFECT_COUNT;
	while (i--) {
		effect = &g_effects[i];
		if (effect->m_type == 0x16 && effect->m_object && (shape = effect->m_object->m_shape)) {
#ifdef MW2_NUKE_RADIUS
			g_nukeMaxRadius = MW2_NUKE_RADIUS;
#else
			g_nukeMaxRadius = shape->m_radius * 100;
#endif
			break;
		}
	}

	StartPaletteFade(0x12, duration, 2);
	g_nukePosition = p_player->m_position;
	g_nukeTimeLeft = duration;
	g_nukeRadius = 0;
	SpawnEffect(
		-2,
		0x16,
		g_nukePosition.m_x,
		g_nukePosition.m_y,
		g_nukePosition.m_z,
		g_nukePosition.m_x,
		g_nukePosition.m_y,
		g_nukePosition.m_z
	);
}

// Grows the nuke's blast and damages everything in it.
// FUNCTION: MW2 0x1006c5e7
// FUNCTION: MW2GBL 0x1007e58e
// FUNCTION: MW2PREMERCS 0x10087e08
// FUNCTION: MW2MATROX 0x1006d239
// FUNCTION: MW23DFX 0x10047130
// FUNCTION: MW2ATI 0x10044860
// FUNCTION: MW2S3 0x1001d910
// FUNCTION: MW2POWERVR 0x1003f5d0
void UpdateNuke(void)
{
	if (g_nukeTimeLeft <= 0) {
		return;
	}

	g_nukeRadius += g_deltaTime * 200;
	if (g_nukeRadius > g_nukeMaxRadius) {
		g_nukeRadius = g_nukeMaxRadius;
	}

	DamageMechsInRadius(
		-2,
		g_nukePosition.m_x,
		g_nukePosition.m_y,
		g_nukePosition.m_z,
		g_nukeRadius,
		FIXED_RAW(0x1000)
#ifdef MW2_WEAPON_FUNCTIONS
			,
		0
#endif
	);
	DamageThingsInRadius(
		-2,
		g_nukePosition.m_x,
		g_nukePosition.m_y,
		g_nukePosition.m_z,
		g_nukeRadius,
		FIXED_RAW(0x1000)
	);
	DamageChunksInRadius(g_nukePosition.m_x, g_nukePosition.m_y, g_nukePosition.m_z, g_nukeRadius, FIXED_RAW(0x1000));
	g_nukeTimeLeft -= g_deltaTime;
}
