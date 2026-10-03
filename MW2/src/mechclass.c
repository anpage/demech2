#include "mechclass.h"

#include "ai.h"
#include "approxlen.h"
#include "clock.h"
#include "collision.h"
#include "config.h"
#include "decomp.h"
#include "environment.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "fixeddiv.h"
#include "fixeddiv29.h"
#include "fixedmul.h"
#include "fixedmul29.h"
#include "fixedtrig.h"
#include "gamekeys.h"
#include "gpanim.h"
#include "inputmap.h"
#include "integrate.h"
#include "maneuvers.h"
#include "mech.h"
#include "mechcollision.h"
#include "mechdamage.h"
#include "mechreload.h"
#include "mechsection.h"
#include "muldiv.h"
#include "network.h"
#include "object.h"
#include "objective.h"
#include "pausebanner.h"
#include "players.h"
#include "playersteering.h"
#include "polydraw.h"
#include "poolsizes.h"
#include "ramp.h"
#include "random.h"
#include "ray.h"
#include "rendertarget.h"
#include "resource.h"
#include "shape.h"
#include "silvertern.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "staticmem.h"
#include "team.h"
#include "timedoverlays.h"
#include "types.h"
#include "weapons.h"
#include "weaponslot.h"

#include <stdio.h>

DECOMP_SIZE_ASSERT(Mech, 0x10e)

// The "mightymouse" cheat: infinite jump jet fuel.
// GLOBAL: MW2 0x100a2be4
MechS32 g_unk0x100a2be4 = 0;

// Set by a game key: run FUN_1004597b on the local mech next tick.
// GLOBAL: MW2 0x100a2be8
MechS32 g_unk0x100a2be8 = 0;

// Set by the MASC game key: toggle the local mech's MASC next tick.
// GLOBAL: MW2 0x100a2bec
MechS32 g_unk0x100a2bec = 0;

// GLOBAL: MW2 0x100a2bf0
MechS32 g_unk0x100a2bf0 = 0;

// GLOBAL: MW2 0x100a2bf4
MechS32 g_unk0x100a2bf4 = 0;

// GLOBAL: MW2 0x100a2bf8
MechS32 g_unk0x100a2bf8 = 0;

// Set when the local mech lands while g_unk0x100a2c00 is on (FUN_10016edf).
// GLOBAL: MW2 0x100a2bfc
MechS32 g_unk0x100a2bfc = 0;

// GLOBAL: MW2 0x100a2c00
MechS32 g_unk0x100a2c00 = 1;

// GLOBAL: MW2 0x100a2c04
MechS32 g_unk0x100a2c04 = 0;

// A power request for the local mech: 1 powers it up, -1 shuts it down.
// GLOBAL: MW2 0x100a2c08
MechS32 g_unk0x100a2c08 = 0;

// The local player's heading at the last tick, while the torso recenters (FUN_100180cd).
// GLOBAL: MW2 0x100a2c0c
MechS32 g_unk0x100a2c0c = 0;

// GLOBAL: MW2 0x100a2c10
MechS32 g_unk0x100a2c10 = 0;

// The clock at the last MASC malfunction roll (FUN_100180cd).
// GLOBAL: MW2 0x100a2c14
MechS32 g_unk0x100a2c14 = 0;

// GLOBAL: MW2 0x100a2c18
MechS32 g_unk0x100a2c18 = 0;

// GLOBAL: MW2 0x100a2c1c
MechS32 g_unk0x100a2c1c = 0;

// Set once the local mech's collision sound played; cleared when it moves freely (FUN_10016edf).
// GLOBAL: MW2 0x100be00c
MechS32 g_unk0x100be00c;

// Puts p_player's mech back in its starting state: fresh parts for a new mech (and, for the local
// player, its cockpit), the ramps, weapon and motion state, its object back on the ground and the
// player's pose from it. With g_unk0x100acb34 the local player starts on the autopilot.
// FUNCTION: MW2 0x10016ad0
void FUN_10016ad0(struct Player* p_player)
{
	Mech* mech;

	mech = p_player->m_mech;
	if (!mech) {
		return;
	}

	if (p_player->m_index != g_reloadingPlayer) {
		mech->m_unk0x60 = FUN_100506d8();
		mech->m_unk0x64 = FUN_100506d8();
		RememberMechSegments(mech);
		if (mech->m_player->m_index == g_localPlayerId) {
			FUN_1006fca5();
		}
	}

	if (mech->m_player->m_index == g_localPlayerId || g_isNetworkGame) {
		StartRamp(&mech->m_unk0x04, 0, 0, 0.2);
	}
	else {
		StartRamp(&mech->m_unk0x04, 0, 0, 0.6);
	}

	StartRamp(&mech->m_unk0x24, 0, 0, 0.2);
	StartRamp(&mech->m_unk0x34, 0, 0, 0.3);
	StartRamp(&mech->m_unk0x14, 0, 0, 0.2);
	StartRamp(&mech->m_unk0x44, 0x400, 0x400, 0.2);
	mech->m_selectedWeapon = 0;
	mech->m_unk0x98 = 0;
	mech->m_unk0xb8 = 0;
	mech->m_unk0xa4 = 0;
	mech->m_unk0x10c |= 0x2000;
	mech->m_unk0xa0 = 0;
	mech->m_unk0x8c = 0;
	mech->m_unk0x90 = 0;
	mech->m_deltaHeat = 0;
	mech->m_unk0xb4 = 0;
	mech->m_unk0xbc = 0;
	mech->m_unk0xf4 = 0;
	mech->m_unk0xf8 = 0;
	mech->m_unk0xfc = 0;
	mech->m_unk0x100 = 0;
	mech->m_unk0x104 = 0;
	mech->m_unk0x108 = 0;
	mech->m_unk0xf0 = 0;
	mech->m_unk0xb0 = 0x10000;
	FUN_10001667(mech->m_player->m_obj, 0, mech->m_unk0xcc, 0);
	FUN_10001cf8(mech->m_player->m_obj);
	GetObjWorldPos(
		mech->m_player->m_obj,
		&mech->m_player->m_unk0x5c,
		&mech->m_player->m_heading,
		&mech->m_player->m_unk0x64
	);
	GetObjPosition(
		mech->m_player->m_obj,
		&mech->m_player->m_position.m_x,
		&mech->m_player->m_position.m_y,
		&mech->m_player->m_position.m_z
	);
	mech->m_player->m_unk0x68 = mech->m_player->m_unk0x6c = mech->m_player->m_unk0x70 = 0;
	mech->m_player->m_steering->m_unk0x33 = 0;
	mech->m_player->m_steering->m_unk0x34 = 0;
	mech->m_player->m_steering->m_unk0x35 = 0;
	mech->m_player->m_steering->m_unk0x38 = 0;
	mech->m_player->m_steering->m_unk0x3b = 0;
	FUN_1000365a(mech->m_player);
	FUN_100019f6(mech->m_player->m_obj);
	if (g_unk0x100acb34 && mech->m_player->m_index == g_localPlayerId) {
		mech->m_player->m_steering->m_throttle = 0x333;
		mech->m_player->m_steering->m_unk0x42 = 1;
		mech->m_player->m_steering->m_unk0x30 = 1;
		g_unk0x100a2418 = 1;
	}
	else {
		mech->m_player->m_steering->m_unk0x42 = 0;
		mech->m_player->m_steering->m_throttle = 0;
		mech->m_player->m_steering->m_unk0x30 = 0;
	}

	FUN_100516c5(mech->m_player);
	mech->m_unk0x88 = FixedDiv16(mech->m_unk0x88, g_unk0x100ba604);
}

// Moves p_mech for the tick: eases its ramps, integrates its velocity (towards the speed it is
// driven at on the ground, or under gravity, drag and jump jet thrust in the air), resolves
// collisions with shapes and other mechs, lands it on the terrain (with falling damage), slides
// it down slopes, then poses its objects. Every player type's update (PlayerType::m_updateFn).
// Stack-slot permutation of the locals (its wider [ebp-N] encodings also shift the jumps).
// FUNCTION: MW2 0x10016edf
void FUN_10016edf(Mech* p_mech)
{
	MechS32 height;
	MechS32 accelY;
	MechS32 velZ;
	MechS32 posZ;
	MechS32 dy;
	MechS32 isLocal;
	Player* hitPlayer;
	MechS32 speed;
	MechS32 length;
	MechS32 dragX;
	MechS32 accelZ;
	MechS32 dz;
	struct Shape* hitShape;
	MechS32 dragZ;
	MechS32 targetX;
	MechS32 topSpeed;
	MechS32 drag;
	Mech* mech;
	MechS32 heading;
	MechS32 targetZ;
	MechS32 pitch;
	MechS32 impact;
	MechS32 velX;
	MechS32 posX;
	MechS32 accelX;
	MechS32 velY;
	MechS32 posY;
	MechS32 dx;
	MechS32 a;
	MechS32 b;
	MechS32 c;
	MechS32 topSpeed2;
	MechS32 posX2;
	MechS32 posY2;
	MechS32 posZ2;
	struct Shape* hitShape2;
	MechS32 savedUnk0xa4;
	MechS32 sound;
	MechS32 volume;
	MechS32 sound2;
	MechS32 volume2;
	MechS32 damage;
	MechS32 slope;
	MechS32 objY;
	MechS32 angle;
	MechS32 objZ;
	MechS32 rayLength;
	Ray ray;
	MechS32 objX;

	height = 0;
	hitShape = NULL;
	hitPlayer = NULL;

	if (!p_mech) {
		return;
	}

	mech = p_mech;
	if ((mech->m_unk0xa0 == 4 && mech->m_player->m_unk0x78) || (mech->m_unk0x10c & 0x100)) {
		return;
	}

	mech->m_deltaHeat = 0;
	isLocal = mech->m_player->m_index == g_localPlayerId;

	if (!(mech->m_player->m_flags & 1)) {
		UpdateRamp(&mech->m_unk0x04);
		UpdateRamp(&mech->m_unk0x14);
		UpdateRamp(&mech->m_unk0x34);
		UpdateRamp(&mech->m_unk0x44);
		UpdateRamp(&mech->m_unk0x24);

		velX = mech->m_unk0xf4;
		velY = mech->m_unk0xf8;
		velZ = mech->m_unk0xfc;
		accelX = 0;
		accelY = -g_unk0x100ba600;
		accelZ = 0;
		dragX = 0;
		dragZ = 0;

		if (mech->m_player->m_unk0x78 && (!mech->m_player->m_steering->m_unk0x1d || mech->m_unk0xc0 <= 0)) {
			speed = mech->m_unk0x24.m_value;
			if (speed < 0x20 && speed > -0x20) {
				speed = 0;
			}

			targetX = FixedMul16(speed, mech->m_player->m_unk0xbc);
			targetZ = FixedMul16(speed, mech->m_player->m_unk0xb8);
			if (g_deltaTime < 0x2d) {
				accelX = (targetX - mech->m_unk0xf4) / 0x2d;
				accelZ = (targetZ - mech->m_unk0xfc) / 0x2d;
			}
			else if (g_deltaTime > 0) {
				accelX = (targetX - mech->m_unk0xf4) / g_deltaTime;
				accelZ = (targetZ - mech->m_unk0xfc) / g_deltaTime;
			}
			else {
				accelX = 0;
				accelZ = 0;
			}
		}
		else {
			length = ApproximateVectorLength(velX, 0, velZ);
			topSpeed = MulDiv64(0x1400, mech->m_unk0x88, 2);
			if (length > 0 && topSpeed > 0 && mech->m_unk0xec > 0) {
				topSpeed2 = FixedMul16(topSpeed, topSpeed);
				a = mech->m_unk0xec - FixedDiv16(mech->m_unk0xec, topSpeed) / 3;
				c = topSpeed2 - topSpeed * 3 / 4;
				a = FixedDiv16(a, c);
				b = mech->m_unk0xec - FixedMul16(a, topSpeed2);
				b = FixedDiv16(b, topSpeed);
				drag = -FixedMul16(b + FixedMul16(a, length), length);
				dragX = MulDiv64(drag, velX, length);
				dragZ = MulDiv64(drag, velZ, length);
			}
			else {
				dragX = 0;
				dragZ = 0;
			}
		}

		drag = 0;
		dx = dy = dz = 0;

		if (mech->m_player->m_steering->m_unk0x1d && mech->m_unk0xc0 > 0 && mech->m_unk0xa0 == 2) {
			velX = mech->m_unk0xf4;
			velZ = mech->m_unk0xfc;
			if (velY > 0) {
				drag = FixedMul16((g_unk0x100ba600 - mech->m_unk0xec) * 0xe24, velY) / g_unk0x100a2bdc;
			}

			accelY += mech->m_unk0xec + drag;
			if (mech->m_player->m_steering->m_turn) {
				if (velY <= 0) {
					accelY = 0;
				}
				else {
					accelY = -g_unk0x100ba600;
				}
			}
			else if (mech->m_player->m_steering->m_unk0x1e) {
				if (velY <= 0) {
					accelY = 0;
				}
				else {
					accelY = -g_unk0x100ba600;
				}

				accelX += -FixedMul16(mech->m_player->m_unk0xb8, mech->m_unk0xec) + dragX;
				accelZ += dragZ + FixedMul16(mech->m_player->m_unk0xbc, mech->m_unk0xec);
			}
			else if (mech->m_player->m_steering->m_unk0x1f) {
				if (velY <= 0) {
					accelY = 0;
				}
				else {
					accelY = -g_unk0x100ba600;
				}

				accelX += dragX + FixedMul16(mech->m_player->m_unk0xb8, mech->m_unk0xec);
				accelZ += -FixedMul16(mech->m_player->m_unk0xbc, mech->m_unk0xec) + dragZ;
			}
			else if (mech->m_player->m_steering->m_unk0x20) {
				if (velY <= 0) {
					accelY = 0;
				}
				else {
					accelY = -g_unk0x100ba600;
				}

				accelX += dragX + FixedMul16(mech->m_player->m_unk0xbc, mech->m_unk0xec);
				accelZ += dragZ + FixedMul16(mech->m_player->m_unk0xb8, mech->m_unk0xec);
			}
			else if (mech->m_player->m_steering->m_unk0x21) {
				if (velY <= 0) {
					accelY = 0;
				}
				else {
					accelY = -g_unk0x100ba600;
				}

				accelX += -FixedMul16(mech->m_player->m_unk0xbc, mech->m_unk0xec) + dragX;
				accelZ += -FixedMul16(mech->m_player->m_unk0xb8, mech->m_unk0xec) + dragZ;
			}
		}
		else if (velY == 0 && mech->m_player->m_unk0x78) {
			accelY = 0;
		}

		IntegrateMidpoint(&dx, &velX, accelX, g_deltaTime);
		IntegrateMidpoint(&dy, &velY, accelY, g_deltaTime);
		IntegrateMidpoint(&dz, &velZ, accelZ, g_deltaTime);
		if (velX < 0x1000 && velX > -0x1000) {
			velX = 0;
		}
		if (velZ < 0x1000 && velZ > -0x1000) {
			velZ = 0;
		}

		mech->m_unk0x100 = velX;
		mech->m_unk0x104 = velY;
		mech->m_unk0x108 = velZ;
		posX = mech->m_player->m_position.m_x;
		posY = mech->m_player->m_position.m_y;
		posZ = mech->m_player->m_position.m_z;
		hitPlayer = NULL;
		mech->m_player->m_unk0x7c = -1;

		if (FUN_100758a0(mech, &hitShape, &hitPlayer, dx, dy, dz, &posX, &posY, &posZ)) {
			dx = posX - mech->m_player->m_position.m_x;
			dy = posY - mech->m_player->m_position.m_y;
			dz = posZ - mech->m_player->m_position.m_z;
			velX = mech->m_unk0xf4;
			velY = mech->m_unk0xf8;
			velZ = mech->m_unk0xfc;
			savedUnk0xa4 = mech->m_unk0xa4;

			if (FUN_100758a0(mech, &hitShape2, &hitPlayer, dx, dy, dz, &posX2, &posY2, &posZ2)) {
				if (hitShape2 != hitShape) {
					posX = mech->m_player->m_position.m_x;
					posY = mech->m_player->m_position.m_y;
					posZ = mech->m_player->m_position.m_z;
				}

				velX = mech->m_unk0xf4 = velX * 2;
				velY = mech->m_unk0xf8 = velY * 2;
				velZ = mech->m_unk0xfc = velZ * 2;
			}

			mech->m_unk0xa4 = savedUnk0xa4;
			if (mech->m_unk0xa0 == 4) {
				FUN_10076a23(mech);
				mech->m_player->m_unk0x78 = 1;
				return;
			}

			g_groundNormalX = g_groundNormalY = g_groundNormalZ = 0;
			mech->m_player->m_unk0x74 = GetTerrainHeight(posX, posY, posZ);
			height = posY - mech->m_unk0xcc - mech->m_player->m_unk0x74;

			if (g_isNetworkGame && g_segmentNormalY > 0xddb4 && hitPlayer) {
				FUN_1000faef(hitPlayer->m_index, g_segmentNormalX, g_segmentNormalY, g_segmentNormalZ);
				if (isLocal && g_unk0x100a2420 && !g_unk0x100be00c) {
					g_unk0x100be00c = 1;
					impact = ApproximateVectorLength(mech->m_unk0x100, mech->m_unk0x104, mech->m_unk0x108);
					if (impact > 200000) {
						sound2 = 0xf0;
						volume2 = impact;
						if (volume2 > 1500000) {
							volume2 = 1500000;
						}

						volume2 = MulDiv64(200, volume2, 1500000);
						FUN_1007eb23(sound2, volume2, 0x40, 5, 0x32);
					}
				}
			}
			else if (g_segmentNormalY > 0xb505 && height < 1000 && height > -10000) {
				mech->m_unk0xa4 = 0;
				height = -1;
			}
			else {
				if (isLocal && g_unk0x100a2420 && !g_unk0x100be00c) {
					g_unk0x100be00c = 1;
					impact = ApproximateVectorLength(mech->m_unk0x100, mech->m_unk0x104, mech->m_unk0x108);
					if (impact > 200000) {
						if (hitPlayer) {
							if (g_isNetworkGame) {
								FUN_1000faef(hitPlayer->m_index, g_segmentNormalX, g_segmentNormalY, g_segmentNormalZ);
							}

							sound = 0xf0;
						}
						else if (hitShape && (hitShape->m_unk0x02 & 0x200)) {
							sound = 200;
						}
						else {
							sound = 0xe5;
						}

						volume = impact;
						if (volume > 1500000) {
							volume = 1500000;
						}

						volume = MulDiv64(200, volume, 1500000);
						FUN_1007eb23(sound, volume, 0x40, 5, 0x32);
						PlayPlayerHitFeedback(g_segmentNormalX, g_segmentNormalY, g_segmentNormalZ);
					}
				}

				if (hitPlayer) {
					FUN_100765f8(mech, hitPlayer->m_mech);
				}
				else {
					FUN_100768a8(mech, hitShape);
				}
			}
		}
		else {
			if (isLocal) {
				g_unk0x100be00c = 0;
			}

			g_groundNormalX = g_groundNormalY = g_groundNormalZ = 0;
			mech->m_player->m_unk0x74 = GetTerrainHeight(posX, posY, posZ);
			height = posY - mech->m_unk0xcc - mech->m_player->m_unk0x74;
		}

		mech->m_player->m_unk0x78 = 0;
		if (height > mech->m_unk0xcc) {
			mech->m_player->m_unk0x80 &= ~1;
			mech->m_player->m_unk0x80 |= 4;
			if (isLocal && g_unk0x100a2c00) {
				g_unk0x100a2bfc = 1;
			}
		}
		else {
			mech->m_player->m_unk0x80 &= ~4;
			if (height <= 0) {
				mech->m_player->m_unk0x78 = 1;
				if (height < 0) {
					if (velY < -0x56276) {
						if (mech->m_unk0xa0 == 4) {
							FUN_10076a23(mech);
							return;
						}
						else {
							FUN_1004ce3e(mech, velY);
						}
					}

					if (velY < -0x102762 && g_difficulty->m_collisionDamage) {
						damage = FixedDiv16(velY + 0x102762, -0x204ec4) * 50;
						ApplyDamageToMech(g_localPlayerId, mech, damage, 8);
						ApplyDamageToMech(g_localPlayerId, mech, damage, 7);
					}

					mech->m_unk0xf8 = velY = 0;
					posY = mech->m_player->m_unk0x74 + mech->m_unk0xcc;
				}
			}
		}

		if ((g_groundNormalX || g_groundNormalZ) && mech->m_player->m_unk0x78) {
			slope = FixedMul16(mech->m_player->m_unk0xb8, g_groundNormalZ) +
					FixedMul16(mech->m_player->m_unk0xbc, g_groundNormalX);
			if (slope > g_unk0x100a2be0 || slope < -g_unk0x100a2be0) {
				mech->m_unk0x24.m_value += FixedMul16(FixedMul16(g_unk0x100ba600, slope), g_deltaTime);
			}
		}

		mech->m_player->m_position.m_x = posX;
		mech->m_player->m_position.m_y = posY;
		mech->m_player->m_position.m_z = posZ;
		mech->m_player->m_heading += mech->m_unk0x34.m_value * g_deltaTime / 0xb5;
		mech->m_player->m_heading %= 0x1680000;
		mech->m_unk0xf4 = velX;
		mech->m_unk0xf8 = velY;
		mech->m_unk0xfc = velZ;

		SetObjPosition(
			mech->m_player->m_obj,
			mech->m_player->m_position.m_x,
			mech->m_player->m_position.m_y,
			mech->m_player->m_position.m_z
		);
		SetObjRotation(
			mech->m_player->m_obj,
			mech->m_player->m_unk0x5c,
			mech->m_player->m_heading,
			mech->m_player->m_unk0x64,
			0
		);
		if (isLocal) {
			FUN_10046269(mech->m_player);
		}

		pitch = mech->m_unk0x14.m_value >> 1;
		if (mech->m_unk0x64) {
			FUN_100463e5(mech->m_player, &ray);
			rayLength = FUN_1004635c(mech->m_player);
			SetRayLength(&ray, rayLength);
			GetObjPosition(mech->m_unk0x64, &objX, &objY, &objZ);
			objX = ray.m_x1 - objX;
			objY = ray.m_y1 - objY;
			objZ = ray.m_z1 - objZ;
			angle = FixedDiv29(objY, ApproximateVectorLength(objX, objY, objZ));
			angle = -FUN_1006975b(angle) - pitch;
			SetObjRotation(mech->m_unk0x64, angle, 0, 0, 0);
		}

		if (mech->m_unk0x60) {
			if (mech->m_unk0x10c & 0x2000) {
				mech->m_player->m_unk0x68 = pitch;
			}
			else {
				mech->m_player->m_unk0x68 = 0;
			}

			mech->m_player->m_unk0x6c = mech->m_unk0x04.m_value;
			mech->m_player->m_unk0x70 = 0;
			SetObjRotation(
				mech->m_unk0x60,
				mech->m_player->m_unk0x68,
				mech->m_player->m_unk0x6c,
				mech->m_player->m_unk0x70,
				1
			);
		}

		FUN_10001cf8(mech->m_player->m_obj);
	}

	heading = mech->m_player->m_heading;
	mech->m_player->m_unk0xbc = FUN_100696c0(heading) >> 13;
	mech->m_player->m_unk0xb8 = FUN_1006973a(heading) >> 13;
}

// Runs p_mech's systems for the tick: the cockpit keys of a running mech (eject, view and display
// keys, the autopilot, the throttle), its jump jets and their fuel, the turn rate, the torso's
// recentering, the MASC (which can fail), the heat, and the power state machine: starting up,
// running, shut down, destroyed (exploding until the timer runs out) and ejecting.
// Every player type's late update (PlayerType::m_lateUpdateFn).
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x100180cd
void FUN_100180cd(Mech* p_mech)
{
	MechS32 throttle;
	MechS32 isLocal;
	MechS32 cosine;
	MechS32 jumping;
	MechS32 turn;
	MechS32 speed;
	MechS32 dx;
	Mech* mech;
	MechS32 dy;
	MechS32 dz;
	MechS32 turnRate;
	MechS32 active;
	MechS32 previousDegrees;
	MechS32 degrees;
	MechS32 heading;
	MechS32 delta;
	MechChar text[40];
	MechS32 x;
	MechS32 y;
	MechS32 z;

	jumping = 0;
	speed = 0;
	turnRate = 0;
	turn = 0;
	cosine = 0;
	throttle = 0;
	active = TRUE;
	mech = p_mech;

	if (mech->m_unk0x10c & 0x200) {
		return;
	}

	isLocal = mech->m_player->m_index == g_localPlayerId;
	if (mech->m_player->m_flags & 1) {
		active = FALSE;
	}

	if (mech->m_unk0xa0 == 2) {
		UpdateWeaponFireState(mech);
	}

	if (g_unk0x100a15d0 >= 0 && g_unk0x100a15d4 >= 0 && mech->m_player->m_index == g_localPlayerId) {
		FUN_10008c0f(-1, mech, g_unk0x100a15d4, g_unk0x100a15d0, 0);
		g_unk0x100a15d0 = -1;
	}

	FUN_10051100(mech->m_player);

	if ((!g_isNetworkGame || isLocal) && mech->m_unk0xa0 == 2) {
		if (mech->m_player->m_steering->m_unk0x43) {
			mech->m_unk0x8c = g_currentClock + 0x16a;
			mech->m_unk0xa0 = 7;
			if (isLocal) {
				PlayCockpitSound(4, -1);
			}

			mech->m_player->m_steering->m_unk0x43 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x30) {
			FUN_100602b2(mech->m_player, 1, 0);
			mech->m_player->m_steering->m_unk0x30 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x31) {
			FUN_100602b2(mech->m_player, -1, 0);
			mech->m_player->m_steering->m_unk0x31 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x32) {
			FUN_100602b2(mech->m_player, 0, 0);
			mech->m_player->m_steering->m_unk0x32 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x33) {
			FUN_1005ef5e(mech->m_player, 1, 8);
			mech->m_player->m_steering->m_unk0x33 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x34) {
			FUN_1005ef5e(mech->m_player, -1, 8);
			mech->m_player->m_steering->m_unk0x34 = 0;
		}

		// Clears 0x34, not 0x35.
		if (mech->m_player->m_steering->m_unk0x35) {
			FUN_1005ef5e(mech->m_player, 0, 8);
			mech->m_player->m_steering->m_unk0x34 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x38) {
			FUN_100603ae();
			mech->m_player->m_steering->m_unk0x38 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x37) {
			FUN_1006034a(1);
			mech->m_player->m_steering->m_unk0x37 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x39) {
			mech->m_player->m_steering->m_unk0x39 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x3b) {
			FUN_1005ef5e(mech->m_player, 1, 0x10008);
			mech->m_player->m_steering->m_unk0x3b = 0;
		}

		if (mech->m_player->m_steering->m_unk0x3c) {
			FUN_100602ec(1);
			mech->m_player->m_steering->m_unk0x3c = 0;
		}

		if (mech->m_player->m_steering->m_unk0x3d) {
			FUN_100602ec(-1);
			mech->m_player->m_steering->m_unk0x3d = 0;
		}

		// Clears 0x3d, not 0x3e.
		if (mech->m_player->m_steering->m_unk0x3e) {
			FUN_100602ec(0);
			mech->m_player->m_steering->m_unk0x3d = 0;
		}

		if (mech->m_player->m_steering->m_unk0x3f) {
			FUN_1006031b(1);
			mech->m_player->m_steering->m_unk0x3f = 0;
		}

		if (mech->m_player->m_steering->m_unk0x40) {
			FUN_1006031b(-1);
			mech->m_player->m_steering->m_unk0x40 = 0;
		}

		// Clears 0x40, not 0x41.
		if (mech->m_player->m_steering->m_unk0x41) {
			FUN_1006031b(0);
			mech->m_player->m_steering->m_unk0x40 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x36) {
			FUN_10060010();
			mech->m_player->m_steering->m_unk0x36 = 0;
		}

		if (!(mech->m_player->m_targetInfo.m_target & 0x1000)) {
			FUN_1005fa22(mech->m_player);
		}

		FUN_10045eac(mech);

		if ((mech->m_player->m_steering->m_unk0x10 || g_unk0x100aa2a0) && mech->m_unk0xbc) {
			mech->m_player->m_steering->m_unk0x42 = 1;
		}

		if (mech->m_player->m_steering->m_unk0x42) {
			if (!mech->m_unk0xbc) {
				mech->m_unk0xbc = 1;
				if (mech->m_player->m_index == g_localPlayerId) {
					PlayCockpitSound(0x1d, 1);
				}
			}
			else {
				mech->m_unk0xbc = 0;
				if (mech->m_player->m_index == g_localPlayerId) {
					PlayCockpitSound(0x1d, 2);
				}
			}
		}

		mech->m_player->m_steering->m_unk0x42 = 0;
		if (!mech->m_player->m_unk0x10 && !mech->m_unk0xbc) {
			mech->m_player->m_steering->m_turn = mech->m_player->m_steering->m_unk0x10;
		}

		if (!mech->m_player->m_unk0x10) {
			FUN_100079d0(mech);
		}

		mech->m_unk0x44.m_target = FixedMul16(mech->m_player->m_steering->m_throttle, mech->m_unk0xb0) + 0x400;
		if (mech->m_player->m_unk0x10 != 2 && mech->m_unk0x44.m_target == 0x400 &&
			(mech->m_player->m_steering->m_turn || g_unk0x100aa2a0)) {
			mech->m_unk0x44.m_target = 0x480;
		}
	}

	FUN_10003710(mech->m_player);

	if (mech->m_unk0xc0 == -2 && mech->m_player->m_steering->m_unk0x44) {
		mech->m_unk0xc0 = 0x712;
		mech->m_unk0xc4 = 3;
		mech->m_unk0xec = g_unk0x100ba600 * 3;
	}

	if (mech->m_unk0xc0 >= 0) {
		if (mech->m_player->m_steering->m_unk0x1d && mech->m_unk0xa0 == 2) {
			jumping = 1;
			if (!g_unk0x100a2be4 || !isLocal) {
				mech->m_unk0xc0 -= g_deltaTime;
			}

			if (mech->m_unk0xc0 <= 0) {
				mech->m_unk0xc0 = 0;
			}
		}
		else if (mech->m_unk0xc0 < 0x712) {
			mech->m_unk0xc0 += g_deltaTime / 4;
		}
		else {
			mech->m_unk0xc0 = 0x712;
		}
	}

	if (jumping) {
		turn = 0;
		if (mech->m_unk0xc0 > 0) {
			FUN_1004ccba(mech);
			mech->m_deltaHeat += mech->m_unk0xc4 * g_deltaTime * 0x180;
			mech->m_player->m_unk0x80 &= ~4;
			mech->m_player->m_unk0x80 &= ~1;
			turn = mech->m_player->m_steering->m_turn / 1024 * 90;
		}

		mech->m_unk0x34.m_target = turn * 2;
	}
	else if (!(mech->m_player->m_unk0x80 & 4) && active) {
		speed = mech->m_unk0x24.m_value / 10000;
		if (speed || (isLocal && mech->m_unk0xb0 > 0)) {
			if (speed > 0x50) {
				speed = 0x50;
			}

			turn = mech->m_player->m_steering->m_turn / 1024 * 90;
			cosine = FUN_1006973a(speed << 16);
			turnRate = FUN_10019ad0(turn, cosine);
			mech->m_unk0x34.m_target = turnRate;
		}
		else {
			mech->m_unk0x34.m_target = 0;
		}
	}
	else if (!mech->m_unk0xbc) {
		mech->m_unk0x34.m_target = 0;
	}

	if (isLocal && g_unk0x100aa2a0) {
		heading = mech->m_player->m_heading;
		degrees = heading >> 16;
		if (degrees > 180) {
			heading = -(0x1680000 - heading);
			degrees = heading >> 16;
		}
		else if (degrees < -180) {
			heading += 0x1680000;
			degrees = heading >> 16;
		}

		previousDegrees = g_unk0x100a2c0c >> 16;
		if (previousDegrees > 180) {
			g_unk0x100a2c0c = -(0x1680000 - g_unk0x100a2c0c);
			previousDegrees = g_unk0x100a2c0c >> 16;
		}
		else if (previousDegrees < -180) {
			g_unk0x100a2c0c += 0x1680000;
			previousDegrees = g_unk0x100a2c0c >> 16;
		}

		if (degrees > 90 && previousDegrees < -90) {
			heading = -(0x1680000 - heading);
		}
		else if (previousDegrees > 90 && degrees < -90) {
			heading += 0x1680000;
		}

		delta = heading - g_unk0x100a2c0c;
		if (mech->m_unk0x04.m_value > -0x40000 && mech->m_unk0x04.m_value < 0x40000) {
			mech->m_unk0x34.m_value = mech->m_unk0x34.m_target = 0;
			mech->m_unk0x04.m_value = mech->m_unk0x04.m_target = 0;
			g_localSteering.m_unk0x1c = 1;
			mech->m_player->m_steering->m_unk0x04 = 0;
			g_unk0x100aa2a0 = 0;
		}
		else if (mech->m_unk0x04.m_value > 0) {
			mech->m_unk0x34.m_target = 0x370000;
			if (delta) {
				mech->m_unk0x04.m_value = mech->m_unk0x04.m_target = mech->m_unk0x04.m_value - delta;
			}
		}
		else {
			mech->m_unk0x34.m_target = -0x370000;
			if (delta) {
				mech->m_unk0x04.m_value = mech->m_unk0x04.m_target = mech->m_unk0x04.m_value - delta;
			}
		}

		if (isLocal) {
			g_unk0x100a2c0c = mech->m_player->m_heading;
		}
	}

	if (active) {
		throttle = mech->m_unk0x44.m_value - 0x400;
		if (throttle < 0x20) {
			throttle = 0;
		}

		mech->m_unk0x24.m_target = mech->m_unk0x88 * throttle;
		if (mech->m_unk0x24.m_target && mech->m_player->m_unk0x84 == 2) {
			mech->m_unk0x24.m_target >>= 1;
			mech->m_unk0x24.m_target *= -1;
		}

		if (!jumping && (mech->m_player->m_unk0x80 & 2)) {
			if (g_unk0x100a2bf0 && isLocal && g_currentClock - g_unk0x100a2c14 > 0xb5) {
				g_unk0x100a2c14 = g_currentClock;
				if (RandomIntBelow(0x3c) == 12 && !g_difficulty->m_invulnerable && mech->m_unk0xa0 == 2) {
					sprintf(text, "MASC malfunction.");
					ShowInGameMessage(text, 1, 0x16a, 0x32);
					FUN_1007eb23(0xc9, 100, 0x40, 5, 0x50);
					mech->m_unk0x98 += mech->m_unk0x98 >> 2;
					g_unk0x100a2bf0 = 0;
				}
			}

			if (mech->m_unk0x24.m_target > mech->m_unk0x24.m_value) {
				mech->m_unk0x24.m_duration = 0x5a;
			}
			else {
				mech->m_unk0x24.m_duration = 0x5a;
			}

			if (g_unk0x100a2bf0 && mech->m_player->m_index == g_localPlayerId) {
				mech->m_unk0x24.m_target += mech->m_unk0x24.m_target >> 1;
			}
		}

		if (!g_unk0x100aa2a0 && (isLocal || !g_isNetworkGame)) {
			mech->m_unk0x04.m_target = mech->m_player->m_steering->m_unk0x04;
		}

		if (mech->m_unk0x04.m_target > mech->m_unk0xe0) {
			mech->m_unk0x04.m_target = mech->m_unk0xe0;
			mech->m_player->m_steering->m_unk0x04 = mech->m_unk0xe0;
			mech->m_player->m_steering->m_unk0x1b = 1;
		}
		else if (mech->m_unk0x04.m_target < -mech->m_unk0xe0) {
			mech->m_unk0x04.m_target = -mech->m_unk0xe0;
			mech->m_player->m_steering->m_unk0x04 = -mech->m_unk0xe0;
			mech->m_player->m_steering->m_unk0x1b = 1;
		}

		if (isLocal || !g_isNetworkGame) {
			mech->m_unk0x14.m_target = mech->m_player->m_steering->m_unk0x00;
		}

		mech->m_deltaHeat += MulDiv64(mech->m_unk0x44.m_value - 0x400, mech->m_unk0x9c, 0x2800);
	}

	if (mech->m_unk0xa0 != 2) {
		FUN_1000369e(mech->m_player);
		mech->m_unk0x44.m_target = 0x400;
		mech->m_unk0x24.m_target = 0;
		mech->m_unk0x04.m_target = 0;
		mech->m_unk0x34.m_target = 0;
		mech->m_unk0x14.m_target = 0;
		mech->m_player->m_steering->m_throttle = 0;
		mech->m_player->m_steering->m_unk0x24 = 1;
		mech->m_player->m_steering->m_unk0x1d = 0;
		mech->m_player->m_steering->m_unk0x1e = 0;
		mech->m_player->m_steering->m_unk0x1f = 0;
		mech->m_player->m_steering->m_unk0x20 = 0;
		mech->m_player->m_steering->m_unk0x21 = 0;
		mech->m_player->m_steering->m_unk0x25 = 0;
		mech->m_player->m_steering->m_unk0x26 = 0;
		mech->m_player->m_steering->m_unk0x2d = 0;
		mech->m_player->m_steering->m_unk0x2e = 0;
		mech->m_player->m_steering->m_unk0x2f = 0;
		mech->m_player->m_steering->m_unk0x30 = 0;
		mech->m_player->m_steering->m_unk0x42 = 0;
		if (mech->m_player->m_index == g_localPlayerId) {
			FUN_10044740(mech);
		}
	}

	CalculateHeat(mech);

	switch (mech->m_unk0xa0) {
	case 7:
		if (mech->m_unk0x8c < g_currentClock) {
			EjectPlayer(mech, 0);
		}
		break;
	case 0:
		mech->m_unk0x8c = g_currentClock + RandomIntBelow(0x16a) + 0x43e;
		mech->m_unk0xa0 = 1;
		mech->m_player->m_flags &= ~0x2000;
		dx = mech->m_player->m_position.m_x - g_eyepoint->m_unk0x00;
		dy = mech->m_player->m_position.m_y - g_eyepoint->m_unk0x04;
		dz = mech->m_player->m_position.m_z - g_eyepoint->m_unk0x08;
		if (GetPlayerSide(mech->m_player->m_index) == 1) {
			if (g_players[g_localPlayerId]->m_flags & 0x2000) {
				FUN_1007ebd1(dx, dy, dz, 0x104, g_unk0x100a2420);
			}
		}
		else {
			FUN_1007ebd1(dx, dy, dz, 0xf7, !g_unk0x100a2420);
		}
		break;
	case 1:
		if (mech->m_unk0x8c < g_currentClock) {
			mech->m_unk0xa0 = 2;
			mech->m_player->m_flags |= 0x2000;
			if (!g_unk0x100a2c10) {
				g_unk0x100a2c10 = 1;
			}
		}
		break;
	case 2:
		if (mech->m_player->m_flags & 0x10) {
			mech->m_player->m_flags &= ~0x10;
			mech->m_player->m_flags |= 0x2000;
			if (GetPlayerSide(mech->m_player->m_index) == 1 && (g_players[g_localPlayerId]->m_flags & 0x2000)) {
				PlayCockpitSound(0x1a, -1);
			}
		}
		break;
	case 3:
		mech->m_player->m_flags |= 0x10;
		break;
	case 4:
		mech->m_player->m_flags &= ~0x2000;
		if (mech->m_unk0x8c == 0) {
			FUN_1004cb11(mech);
			mech->m_unk0x8c = g_currentClock + 0x712;
			FUN_1001cdd1();
		}

		if (mech->m_unk0x8c > g_currentClock) {
			FUN_1004cb11(mech);
		}
		else {
			mech->m_unk0x10c |= 0x200;
		}
		break;
	}

	if (mech->m_player->m_index == g_localPlayerId &&
		(mech->m_unk0x04.m_value + 0x20000 < mech->m_unk0x04.m_target ||
		 mech->m_unk0x04.m_value - 0x20000 > mech->m_unk0x04.m_target) &&
		g_unk0x100a2420) {
		x = mech->m_player->m_position.m_x - g_eyepoint->m_unk0x00;
		y = mech->m_player->m_position.m_y - g_eyepoint->m_unk0x04;
		z = mech->m_player->m_position.m_z - g_eyepoint->m_unk0x08;
		FUN_1007ebd1(x, y, z, 0x13c, g_unk0x100a2420);
	}

	if (!isLocal) {
		mech->m_player->m_flags &= ~1;
	}
}

// Handles the local mech's requests for the tick. A running mech (state 2) acts on the MASC and
// other system keys; a destroyed one (4) ends the mission once; an ejecting one (5) runs the
// ejection camera and then ends it. Except while destroyed or ejecting, a power request shuts the
// mech down, or powers up a shut-down mech that isn't overheating.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10019368
void FUN_10019368(Mech* p_mech)
{
	MechS32 dx;
	Mech* mech;
	MechS32 dy;
	MechS32 dz;
	MechChar text[40];

	if (p_mech) {
		mech = p_mech;
	}
	else {
		return;
	}

	switch (mech->m_unk0xa0) {
	case 5:
		if (!g_unk0x100a2c1c) {
			mech->m_unk0x8c = g_currentClock + 0x389;
			g_unk0x100a2c1c++;
		}
		else if (mech->m_unk0x8c > g_currentClock) {
			FUN_10011401(4);
		}
		else {
			g_unk0x100a2c04 = 1;
		}
		break;
	case 4:
		if (!g_unk0x100a2c18) {
			g_unk0x100a2c18 = 1;
			FUN_1004ca0d();
			g_unk0x100a2c04 = 1;
			FUN_10011401(1);
		}
		break;
	case 2:
		if (g_unk0x100a2bf8) {
			if (!mech->m_unk0xb4) {
				FUN_1007eb23(0xfd, 0x32, 0x40, 5, 0x32);
				FUN_10045567(mech);
			}

			g_unk0x100a2bf8 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x26 && !mech->m_unk0xb4) {
			FUN_1007eb23(0xfd, 0x32, 0x40, 5, 0x32);
			g_unk0x100a2bf8 = 1;
			FUN_10045567(mech);
			g_unk0x100a2bf8 = 0;
		}

		if (mech->m_player->m_steering->m_unk0x2b) {
			FUN_10045b9c();
		}

		if (g_unk0x100a2be8) {
			g_unk0x100a2be8 = 0;
			FUN_1004597b(mech);
		}

		if (g_unk0x100a2bec) {
			g_unk0x100a2bec = 0;
			if (mech->m_unk0x10c & 0x10) {
				if (g_unk0x100a2bf0) {
					g_unk0x100a2bf0 = 0;
					FUN_1007eb23(0xca, 100, 0x40, 5, 0x50);
					PlayCockpitSound(0x1f, 0);
				}
				else {
					g_unk0x100a2bf0 = 1;
					FUN_1007eb23(0xcb, 100, 0x40, 5, 0x50);
					PlayCockpitSound(0x1f, 1);
				}
			}
			else {
				sprintf(text, "Not equipped with MASC.");
				ShowInGameMessage(text, 1, 0x16a, 0x32);
			}
		}
	default:
		switch (g_unk0x100a2c08) {
		case 0:
			break;
		case 1:
			if ((!(mech->m_unk0x10c & 4) || (mech->m_unk0x10c & 8)) && mech->m_unk0xa0 == 3) {
				sprintf(text, "Powering up...");
				ShowInGameMessage(text, 1, 0x16a, 0x32);
				mech->m_unk0xa0 = 0;
				mech->m_player->m_flags &= ~0x10;
				g_unk0x100a2c08 = 0;
			}
			break;
		case -1:
			if (mech->m_unk0xa0 != 3) {
				PlayCockpitSound(0xd, -1);
				dx = mech->m_player->m_position.m_x - g_eyepoint->m_unk0x00;
				dy = mech->m_player->m_position.m_y - g_eyepoint->m_unk0x04;
				dz = mech->m_player->m_position.m_z - g_eyepoint->m_unk0x08;
				FUN_1007ebd1(dx, dy, dz, 0xf2, g_unk0x100a2420);
				mech->m_unk0xa0 = 3;
				g_unk0x100a2c08 = 0;
			}
			break;
		}
		break;
	}
}

// FUNCTION: MW2 0x1001975a
void FUN_1001975a(Mech* p_mech)
{
	Mech* mech;

	if (p_mech) {
		mech = p_mech;
	}
	else {
		return;
	}

	FUN_1007005a(mech);
}

// Runs FUN_100704c1 for the local player's mech.
// FUNCTION: MW2 0x1001978e
void FUN_1001978e(Mech* p_mech)
{
	Mech* mech;

	mech = p_mech;
	if (!p_mech) {
		return;
	}

	if (mech->m_player->m_index == g_localPlayerId) {
		FUN_100704c1();
	}
}

// Allocates p_player's mech and sets it up.
// Stack-slot permutation of buffer, i, size and mech.
// FUNCTION: MW2 0x100197ca
MechS32 FUN_100197ca(MechS32 p_index, Player* p_player)
{
	void* buffer = NULL;
	MechS32 i;
	MechS32 size;
	Mech* mech = NULL;

	p_player->m_mech = NULL;
	size = FUN_10019a0a();
	buffer = StaticPoolAlloc(size, g_staticPoolTags[1]);
	if (!buffer) {
		return FALSE;
	}

	mech = buffer;
	mech->m_player = p_player;
	for (i = 0; i < 8; i++) {
		mech->m_objects[i] = NULL;
	}

	p_player->m_mech = mech;
	p_player->m_unk0x24 = 0x10e;
	FUN_10019881(mech);
	return TRUE;
}

// Lays out p_mech's allocation (its ten weapons, eight sections and 25 ammunition bins after it)
// and empties the weapons and the bins.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10019881
void FUN_10019881(struct Mech* p_mech)
{
	SilverTern0x14* bin;
	WeaponSlot* slot;
	SilverTern0x14* bins = NULL;
	MechS32 i;
	WeaponSlot* weapons = NULL;
	MechSection* sections = NULL;

	weapons = (WeaponSlot*) (p_mech + 1);
	sections = (MechSection*) (weapons + 10);
	bins = (SilverTern0x14*) (sections + 8);
	p_mech->m_weapons = weapons;
	p_mech->m_sections = sections;
	p_mech->m_unk0x5c = bins;
	slot = p_mech->m_weapons;
	for (i = 0; i < 10; i++) {
		slot->m_unk0x00 = -1;
		slot->m_type = -1;
		slot->m_state = c_weaponEmpty;
		slot->m_time = 0;
		slot->m_unk0x14 = 0;
		slot->m_ammo = -1;
		slot->m_group = 0;
		slot->m_target = -1;
		slot->m_targetKind = 0;
		slot->m_volley = 0;
		slot->m_hardpoint = -1;
		slot->m_unk0x2c = 0;
		slot->m_binCount = 0;
		slot->m_index = 0;
		slot++;
	}

	bin = p_mech->m_unk0x5c;
	for (i = 0; i < 25; i++) {
		bin->m_unk0x00 = -1;
		bin->m_unk0x02 = 0;
		bin->m_weapon = -1;
		bin->m_id = 0;
		bin->m_unk0x08 = 0;
		bin->m_unk0x0a = 0;
		bin->m_unk0x0c = 0;
		bin->m_unk0x10 = 0;
		bin++;
	}
}

// Returns the size of a mech's allocation: the mech, its ten weapons and eight sections, and 500
// bytes more.
// FUNCTION: MW2 0x10019a0a
MechS32 FUN_10019a0a(void)
{
	MechS32 size;

	size = sizeof(Mech);
	size += 10 * sizeof(WeaponSlot);
	size += 8 * sizeof(MechSection);
	size += 500;
	return size;
}

// FUNCTION: MW2 0x10019a3c
MechS32 FUN_10019a3c(Player* p_player)
{
	Mech* mech;

	mech = p_player->m_mech;
	return mech->m_unk0xb8;
}

// Selects weapon p_weapon of p_player's mech.
// FUNCTION: MW2 0x10019a61
void FUN_10019a61(Player* p_player, MechS32 p_weapon)
{
	Mech* mech;

	mech = p_player->m_mech;
	mech->m_selectedWeapon = p_weapon;
}

// FUNCTION: MW2 0x10019a84
MechS32 FUN_10019a84(Player* p_player)
{
	Mech* mech;

	mech = p_player->m_mech;
	return mech->m_unk0xcc;
}
