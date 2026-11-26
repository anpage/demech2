#include "aiweapons.h"

#include "ai.h"
#include "clock.h"
#include "decomp.h"
#include "fixedfloat.h"
#include "lineofsight.h"
#include "mech.h"
#include "players.h"
#include "playersteering.h"
#include "random.h"
#include "simmain.h"
#include "soundfx.h"
#include "types.h"
#include "weapondata.h"
#include "weapondef.h"
#include "weapons.h"
#include "weaponslot.h"

#ifdef MW2_AI_LEAD_TARGET
#include "fixedmul.h"
#include "integrate.h"
#include "targeting.h"

#include <math.h>
#include <stdlib.h>

#pragma function(abs, sqrt) // the early Mercenaries build calls the C runtime's abs and sqrt here
#endif

#ifdef MW2_AI_LEAD_TARGET
// Runs p_player's AI weapons, aiming where its target will be when it leads it (m_leadsTarget).
// Returns whether it fired.
// FUNCTION: MW2PREMERCS 0x10052780
MechS32 RunAIWeapons(Player* p_player, MechScalar p_heading)
{
	MechS32 fired;

	if (p_player->m_leadsTarget) {
		fired = RunAIWeaponsLeading(p_player, p_heading);
	}
	else {
		fired = RunAIWeaponsDirect(p_player, p_heading);
	}

	return fired;
}

// Runs p_player's AI weapons at random intervals (up to m_gunnery x 22 ticks) against where its target
// will be (LeadAITarget): within a degree of that heading and 10 degrees of its pitch (always against
// the local player, else one time in three) it may fire, and a guided volley may lock on. Then turns
// and pitches the torso that way. Returns whether it fired.
// FUNCTION: MW2PREMERCS 0x100527d1
MechS32 RunAIWeaponsLeading(Player* p_player, MechScalar p_heading)
{
	MechS32 guided;
	MechS32 fired;
	MechS32 roll;
	MechS32 decided;
	MechScalar pitch;
	MechScalar delta;
	MechScalar tilt;

	roll = 0;
	guided = FALSE;
	fired = FALSE;
	pitch = p_player->m_targetInfo.m_pitch;
	LeadAITarget(p_player, &p_heading, &pitch);
	delta = p_heading - p_player->m_torsoTwist;
	tilt = (p_player->m_mech->m_torsoPitch.m_value + p_player->m_pitch + pitch) % FIXED_CONST(360);
	if (p_player->m_nextFireTime <= g_currentClock) {
		roll = RandomIntBelow(p_player->m_gunnery);
		p_player->m_nextFireTime = roll * 22 + g_currentClock;
		if (delta < FIXED_CONST(1) && delta > FIXED_CONST(-1) && tilt < FIXED_CONST(10) && tilt > FIXED_CONST(-10)) {
			if ((p_player->m_ai.m_goal & 0xff) == g_localPlayerId || !RandomIntBelow(3)) {
				if (!roll) {
					if (p_player->m_unk0x160) {
						decided = DecideAIGroupFire(p_player, &guided);
					}
					else {
						decided = DecideAIFire(p_player, &guided);
					}

					if (decided) {
						if (!(p_player->m_skillFlag4) || CanSeeTarget(p_player, 1)) {
							if (p_player->m_unk0x160) {
								p_player->m_steering->m_weaponFireGroup = 1;
							}
							else {
								p_player->m_steering->m_weaponFire = 1;
							}

							fired = TRUE;
						}
					}
				}
			}
		}
	}

	if (guided && fired) {
		RollAILock(p_player);
	}

	roll = 0;
	p_player->m_steering->m_torsoPan = AimTorsoPan(p_player, p_heading, roll) * 0x2d00;
	p_player->m_steering->m_torsoTilt = AimTorsoTilt(p_player, pitch, roll) * -0xf00;
	return fired;
}

// Runs p_player's AI weapons at random intervals (up to m_gunnery x 22 ticks): within 10 degrees of
// the heading p_heading and of its target's pitch (always against the local player, else one time in
// three) it aims at its goal and may fire, and a guided volley may lock on. Then turns and pitches the
// torso. Returns whether it fired.
// FUNCTION: MW2PREMERCS 0x10052a0d
MechS32 RunAIWeaponsDirect(Player* p_player, MechScalar p_heading)
{
	MechS32 guided;
	MechS32 fired;
	MechS32 roll;
	MechS32 decided;
	MechScalar delta;
	MechScalar tilt;

	guided = FALSE;
	fired = FALSE;
	delta = p_heading - p_player->m_torsoTwist;
	if (p_player->m_nextFireTime <= g_currentClock) {
		roll = RandomIntBelow(p_player->m_gunnery);
		p_player->m_nextFireTime = roll * 22 + g_currentClock;
		tilt = (p_player->m_mech->m_torsoPitch.m_value + p_player->m_pitch + p_player->m_targetInfo.m_pitch) %
			   FIXED_CONST(360);
		if (delta < FIXED_CONST(10) && delta > FIXED_CONST(-10) && tilt < FIXED_CONST(10) && tilt > FIXED_CONST(-10)) {
			if ((p_player->m_ai.m_goal & 0xff) == g_localPlayerId || !RandomIntBelow(3)) {
				SetTarget(p_player, p_player->m_ai.m_goal);
				if (!roll) {
					if (p_player->m_unk0x160) {
						decided = DecideAIGroupFire(p_player, &guided);
					}
					else {
						decided = DecideAIFire(p_player, &guided);
					}

					if (decided) {
						if (!(p_player->m_skillFlag4) || CanSeeTarget(p_player, 1)) {
							if (p_player->m_unk0x160) {
								p_player->m_steering->m_weaponFireGroup = 1;
							}
							else {
								p_player->m_steering->m_weaponFire = 1;
							}

							fired = TRUE;
						}
					}
				}
			}
		}
	}

	if (guided && fired) {
		RollAILock(p_player);
	}

	if (!fired) {
		roll = 0;
	}

	p_player->m_steering->m_torsoPan = AimTorsoPan(p_player, p_heading, roll) * 0x2d00;
	p_player->m_steering->m_torsoTilt = AimTorsoTilt(p_player, p_player->m_targetInfo.m_pitch, roll) * -0xf00;
	return fired;
}

// Decides whether p_player's AI fires its selected weapon: in range of its target, the weapon's heat
// leaving the mech below the pilot's limit, the weapon ready and loaded, and by chance per recycle
// time; otherwise it selects the next weapon. Firing weapon types 0 and 4 also steers. Sets *p_guided
// when the weapon is guided.
// FUNCTION: MW2PREMERCS 0x10052c4f
MechS32 DecideAIFire(Player* p_player, MechS32* p_guided)
{
	MechS32 fire;
	MechS32 type;
	Mech* mech;
	MechS32 i;
	WeaponSlot* slot;
	WeaponDef* def;

	mech = p_player->m_mech;
	*p_guided = FALSE;
	for (i = 0, fire = FALSE; i < mech->m_weaponCount && !fire; i++) {
		fire = FALSE;
		if (mech->m_selectedWeapon != -1) {
			slot = &mech->m_weapons[mech->m_selectedWeapon];
			type = slot->m_type;
			def = &g_weaponDefs[type];
			switch (GetTargetRangeBand(p_player, def)) {
			case 1:
			case 2:
				fire = TRUE;
				break;
			default:
				break;
			}
		}

		if (fire) {
			fire = FALSE;
			if ((mech->m_heat + def->m_heat) >> 16 < p_player->m_heatLimit && IsSelectedWeaponReady(mech) == 1 &&
				slot->m_ammo && !RandomIntBelow(def->m_recycle / 90 + 1)) {
				fire = TRUE;
			}
		}

		if (!fire) {
			SelectNextWeaponInGroup(mech, 1);
		}
	}

	if (fire) {
		if (type == 0) {
			p_player->m_steering->m_torsoTilt += FIXED_RAW(0x1e000);
		}
		else if (type == 4) {
			p_player->m_steering->m_torsoTilt += FIXED_RAW(0x3c000);
		}
		else if (type == 21) {
		}

		if (g_weaponDefs[type].m_guided) {
			*p_guided = TRUE;
		}
	}

	return fire;
}

// Decides whether p_player's AI fires a group: every loaded weapon in its target's range band 2 joins
// group 1 while it is ready and the group's heat leaves the mech below the pilot's limit; one that
// isn't ready or would overheat it fires none (the loaded ammunition is counted, and never read).
// Sets *p_guided when a weapon of the group is guided.
// FUNCTION: MW2PREMERCS 0x10052e48
MechS32 DecideAIGroupFire(Player* p_player, MechS32* p_guided)
{
	MechS32 fire;
	MechS32 guided;
	Mech* mech;
	MechS16 i;
	MechS16 type;
	WeaponSlot* slot;
	WeaponDef* def;
	MechScalar heat;
	MechS32 ammo;
	MechScalar tilt;

	mech = p_player->m_mech;
	fire = FALSE;
	guided = FALSE;
	heat = 0;
	ammo = 0;
	tilt = 0;
	for (i = 0; i < mech->m_weaponCount; i++) {
		slot = &mech->m_weapons[i];
		type = slot->m_type;
		def = &g_weaponDefs[type];
		slot->m_group = 0;
		if (GetTargetRangeBand(p_player, def) == 2 && slot->m_ammo) {
			if (slot->m_state == c_weaponReady && (mech->m_heat + def->m_heat + heat) >> 16 < p_player->m_heatLimit) {
				slot->m_group = 1;
				ammo += slot->m_ammo;
				heat += def->m_heat;
				fire = TRUE;
				if (guided || g_weaponDefs[type].m_guided) {
					guided = TRUE;
				}
				else {
					guided = FALSE;
				}

				if (type == 0) {
					tilt = FIXED_RAW(0x1e000);
				}

				if (type == 4) {
					tilt = FIXED_RAW(0x3c000);
				}
			}
			else {
				fire = FALSE;
				break;
			}
		}
	}

	if (fire) {
		p_player->m_steering->m_torsoTilt += tilt;
		SelectWeaponGroup(mech, 1);
	}
	else {
		SelectNextWeaponInGroup(mech, 1);
	}

	*p_guided = guided;
	return fire;
}

// Locks p_player's guided volley on, by its lock chance, and warns the local player when it is the
// target.
// FUNCTION: MW2PREMERCS 0x1005301c
void RollAILock(Player* p_player)
{
	Mech* mech;

	mech = p_player->m_mech;
	if (RandomIntBelow(100) < p_player->m_lockChance) {
		mech->m_flags |= 0x80;
		if (p_player->m_ai.m_goal == (g_localPlayerId | 0x200)) {
			PlaySoundEffect(0x6f, 100, 0x40, 5, 0x32);
		}
	}
}

// Points *p_heading and *p_pitch (relative to p_player's heading) at where its goal's mech will be when
// a shot of the selected weapon reaches it: the shot's flight time from the closing speed along the
// target's velocity, if the shot lives that long and it is more than a tick.
// FUNCTION: MW2PREMERCS 0x10053098
void LeadAITarget(Player* p_player, MechScalar* p_heading, MechScalar* p_pitch)
{
	MechDouble vz;
	Player* target;
	MechS32 goal;
	Mech* mech;
	Mech* targetMech;
	WeaponDef* def;
	MechS32 shotSpeed;
	MechScalar dx;
	MechScalar dy;
	MechS32 type;
	MechScalar along;
	MechScalar dz;
	MechDouble vx;
	WeaponSlot* slot;
	MechDouble vy;
	MechS32 closing;
	MechS32 ticks;
	MechS32 velocityX;
	MechS32 velocityY;
	MechS32 velocityZ;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar heading;
	MechScalar length;
	BearingDistance distance;

	goal = p_player->m_ai.m_goal & 0xff;
	target = g_players[goal];
	mech = p_player->m_mech;
	targetMech = target->m_mech;
	slot = &mech->m_weapons[mech->m_selectedWeapon];
	type = slot->m_type;
	def = &g_weaponDefs[type];
	shotSpeed = def->m_speed;
	dx = target->m_position.m_x - p_player->m_position.m_x;
	dy = target->m_position.m_y - p_player->m_position.m_y;
	dz = target->m_position.m_z - p_player->m_position.m_z;
	along = FixedMul16(dx, targetMech->m_velocityX) + FixedMul16(dz, targetMech->m_velocityZ) +
			FixedMul16(dy, targetMech->m_velocityY);
	vx = targetMech->m_velocityX;
	vy = targetMech->m_velocityY;
	vz = targetMech->m_velocityZ;
	if (vx == 0.0 && vy == 0.0 && vz == 0.0) {
		return;
	}

	closing = (MechS32) sqrt(vy * vy + vx * vx + vz * vz) >> 16;
	if (along >= 0) {
	}
	else {
		closing = -closing;
	}

	if (abs(closing) == shotSpeed) {
		ticks = 2;
	}
	else {
		ticks = p_player->m_targetInfo.m_range / (shotSpeed - closing);
	}

	if (!ticks || def->m_lifetime < ticks || ticks <= g_deltaTime) {
		return;
	}

	velocityX = (MechS32) vx;
	velocityY = (MechS32) vy;
	velocityZ = (MechS32) vz;
	x = target->m_position.m_x;
	y = target->m_position.m_y;
	z = target->m_position.m_z;
	IntegrateMidpoint(&x, &velocityX, 0, ticks);
	IntegrateMidpoint(&y, &velocityY, 0, ticks);
	IntegrateMidpoint(&z, &velocityZ, 0, ticks);
	x -= p_player->m_position.m_x;
	y -= p_player->m_position.m_y;
	z -= p_player->m_position.m_z;
	GetBearingAndRange(x, y, z, &heading, &length, &distance, p_pitch);
	*p_heading = heading - p_player->m_heading;
	if (*p_heading > FIXED_CONST(180)) {
		*p_heading -= FIXED_CONST(360);
	}
	else if (*p_heading < FIXED_CONST(-180)) {
		*p_heading += FIXED_CONST(360);
	}
}
#else
// Runs p_player's AI weapons at random intervals (up to m_gunnery x 22 ticks): within 10 degrees
// of the heading p_heading (always against the local player, else one time in three) it aims at
// its goal and may fire (DecideAIFire). Then turns and pitches the torso. Returns whether it
// fired.
// FUNCTION: MW2 0x1004b5a0
// FUNCTION: MW2GBL 0x10071dd0
// FUNCTION: MW2MATROX 0x1007a850
// FUNCTION: MW23DFX 0x100231a0
// FUNCTION: MW2ATI 0x10023460
// FUNCTION: MW2S3 0x1003c180
// FUNCTION: MW2POWERVR 0x100226a0
MechS32 RunAIWeapons(Player* p_player, MechScalar p_heading)
{
	MechS32 fired;
	MechS32 roll;
	MechScalar delta;

	fired = FALSE;
	if (p_player->m_nextFireTime <= g_currentClock) {
		roll = RandomIntBelow(p_player->m_gunnery);
		p_player->m_nextFireTime = roll * 22 + g_currentClock;
		delta = p_heading - p_player->m_torsoTwist;
		if (delta < FIXED_CONST(10) && delta > FIXED_CONST(-10)) {
			if ((p_player->m_ai.m_goal & 0xff) == g_localPlayerId || !RandomIntBelow(3)) {
				SetTarget(p_player, p_player->m_ai.m_goal);
				if (!roll) {
					if (DecideAIFire(p_player)) {
						if (!(p_player->m_skillFlag4) || CanSeeTarget(p_player, 1)) {
#ifdef MW2_PILOT_TABLE
							if (p_player->m_unk0x160) {
								p_player->m_steering->m_weaponFireGroup = 1;
							}
							else {
								p_player->m_steering->m_weaponFire = 1;
							}
#else
							p_player->m_steering->m_weaponFire = 1;
#endif
							fired = TRUE;
						}
					}
				}
			}
		}
	}

	if (!fired) {
		roll = 0;
	}

	p_player->m_steering->m_torsoPan = AimTorsoPan(p_player, p_heading, roll) * 0x2d00;
#if defined(MW2_TILT_NEGATIVE_FACTOR)
	p_player->m_steering->m_torsoTilt = AimTorsoTilt(p_player, roll) * -0xf00;
#else
	p_player->m_steering->m_torsoTilt = -(AimTorsoTilt(p_player, roll) * 0xf00);
#endif
	return fired;
}

#ifdef MW2_PILOT_TABLE
// GBL's: with the pilot table's flag set the AI fires the group, deciding by the heat and ammunition
// of the ready weapons (its loop reads the selected weapon's slot each time, not each slot's).
// FUNCTION: MW2GBL 0x10071f73
MechS32 DecideAIFire(Player* p_player)
{
	MechS32 fire;
	MechS32 type;
	Mech* mech;
	MechS32 i;
	WeaponSlot* slot;
	WeaponDef* def;
	MechScalar heat;
	MechS32 ammo;
	MechS16 j;

	mech = p_player->m_mech;
	for (i = 0, fire = FALSE; i < mech->m_weaponCount && !fire; i++) {
		fire = FALSE;
		if (mech->m_selectedWeapon != -1) {
			slot = &mech->m_weapons[mech->m_selectedWeapon];
			type = slot->m_type;
			def = &g_weaponDefs[type];
			switch (GetTargetRangeBand(p_player, def)) {
			case 1:
			case 2:
				fire = TRUE;
				break;
			default:
				break;
			}
		}

		if (fire) {
			fire = FALSE;
			if (p_player->m_unk0x160) {
				heat = 0;
				ammo = 0;
				for (j = 0; j < mech->m_weaponCount; j++) {
					slot = &mech->m_weapons[mech->m_selectedWeapon];
					def = &g_weaponDefs[slot->m_type];
					if (slot->m_state == c_weaponReady && slot->m_ammo) {
						ammo += slot->m_ammo;
						heat += def->m_heat;
					}
				}

				if (ammo && (mech->m_heat + heat) >> 16 < 65.0 && !RandomIntBelow(def->m_recycle / 90 + 1)) {
					fire = TRUE;
				}
			}
			else {
				if ((def->m_heat + mech->m_heat) >> 16 < 65.0 && IsSelectedWeaponReady(mech) == 1 && slot->m_ammo &&
					!RandomIntBelow(def->m_recycle / 90 + 1)) {
					fire = TRUE;
				}
			}
		}

		if (!fire) {
			SelectNextWeaponInGroup(mech, 1);
		}
	}

	if (fire) {
		if (type == 0) {
			p_player->m_steering->m_torsoTilt += FIXED_RAW(0x1e000);
		}
		else if (type == 4) {
			p_player->m_steering->m_torsoTilt += FIXED_RAW(0x3c000);
		}
		else if (type == 21) {
		}

		if (g_weaponDefs[type].m_guided) {
			if (p_player->m_gunnery <= 4 && !RandomIntBelow(p_player->m_gunnery + 1)) {
				mech->m_flags |= 0x80;
			}

			if (p_player->m_ai.m_goal == (g_localPlayerId | 0x200)) {
				PlaySoundEffect(0x6f, 100, 0x40, 5, 0x32);
			}
		}
	}

	return fire;
}
#else
// Decides whether p_player's AI fires its selected weapon: in range of its target, cool enough, the
// weapon ready and loaded, and by chance per recycle time; otherwise it selects the next weapon.
// Firing weapon types 0 and 4 also steers; a guided weapon's volley may lock on.
// FUNCTION: MW2 0x1004b724
// FUNCTION: MW2MATROX 0x1007a9d9
// FUNCTION: MW23DFX 0x100232b0
// FUNCTION: MW2ATI 0x10023570
// FUNCTION: MW2S3 0x1003c290
// FUNCTION: MW2POWERVR 0x100227b0
MechS32 DecideAIFire(Player* p_player)
{
	MechS32 fire;
	MechS32 type;
	Mech* mech;
	MechS32 i;
	WeaponSlot* slot;
	WeaponDef* def;

	mech = p_player->m_mech;
	for (i = 0, fire = FALSE; i < mech->m_weaponCount && !fire; i++) {
		fire = TRUE;
		slot = &mech->m_weapons[mech->m_selectedWeapon];
		type = slot->m_type;
		def = &g_weaponDefs[type];
		switch (GetTargetRangeBand(p_player, def)) {
		case 1:
		case 2:
			break;
		default:
			fire = FALSE;
			break;
		}

		if (fire) {
			fire = FALSE;
#ifdef MW2_FLOAT_SCALARS
			if (def->m_heat + mech->m_heat < 65.0f && IsSelectedWeaponReady(mech) == 1 && slot->m_ammo &&
				!RandomIntBelow(def->m_recycle / 90 + 1)) {
#else
			if ((def->m_heat + mech->m_heat) >> 16 < 65.0 && IsSelectedWeaponReady(mech) == 1 && slot->m_ammo &&
				!RandomIntBelow(def->m_recycle / 90 + 1)) {
#endif
				fire = TRUE;
			}
		}

		if (!fire) {
			SelectNextWeaponInGroup(mech, 1);
		}
	}

	if (fire) {
		if (type == 0) {
			p_player->m_steering->m_torsoTilt += FIXED_RAW(0x1e000);
		}
		else if (type == 4) {
			p_player->m_steering->m_torsoTilt += FIXED_RAW(0x3c000);
		}
#ifndef MW2_ACCEL
		else if (type == 21) {
		}
#endif

		if (g_weaponDefs[type].m_guided) {
			if (p_player->m_gunnery <= 4 && !RandomIntBelow(p_player->m_gunnery + 1)) {
				mech->m_flags |= 0x80;
			}

			if (p_player->m_ai.m_goal == (g_localPlayerId | 0x200)) {
				PlaySoundEffect(0x6f, 100, 0x40, 5, 0x32);
			}
		}
	}

	return fire;
}
#endif
#endif

#ifdef MW2_PILOT_TABLE
// Puts every weapon of p_player's mech into group p_group, for the pilot table's entries that fire
// the group; p_unused is overwritten and never read. Returns 0 for a negative group or a mech
// without one or two weapons, else 1.
// FUNCTION: MW2GBL 0x100722dd
MechS32 GroupAIWeapons(Player* p_player, MechS32 p_group, MechS32 p_unused)
{
	Mech* mech;
	MechS32 i;

	p_unused = -1;
	mech = p_player->m_mech;
	if (p_group < 0) {
		return 0;
	}

	if (mech->m_weaponCount <= 0 || mech->m_weaponCount >= 3) {
		return 0;
	}

	for (i = 0; i < mech->m_weaponCount; i++) {
		SetWeaponGroup(mech, i, p_group);
	}

	return 1;
}
#endif
