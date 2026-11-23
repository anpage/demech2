/* GBL's anti-missile system (the AMS weapon, its TOGGLE_AMS game key): an object of four functions
   GBL links before refreshmode.c. The AMS fires at the enemy missiles that close on its mech, and
   each missile its fire reaches gets a range from the mech at which it detonates. */
#include "gbl/ams.h"

#include "approxlen.h"
#include "careerrecord.h"
#include "config.h"
#include "decomp.h"
#include "maneuvers.h"
#include "mech.h"
#include "object.h"
#include "players.h"
#include "random.h"
#include "shots.h"
#include "simmain.h"
#include "speech.h"
#include "team.h"
#include "types.h"
#include "weapondef.h"
#include "weaponslot.h"

// WeaponSlot::m_state of an AMS the player turned off.
enum AmsSlotState {
	c_weaponOff = 4
};

// The AMS weapon type (g_weaponDefs).
#define AMS_WEAPON_TYPE 0x1d

// How close an enemy missile comes before the AMS fires at it, and the least range at which it
// detonates one.
#define AMS_REACH 50000
#define AMS_MIN_INTERCEPT 2500

// Turns the local mech's AMS off or back on (the TOGGLE_AMS game key), and says so.
// FUNCTION: MW2GBL 0x1002e8c0
void ToggleAms(void)
{
	MechS32 message;
	Mech* mech;
	MechS32 i;
	WeaponSlot* slot;
	MechS32 type;

	message = -1;
	mech = g_players[g_localPlayerId]->m_mech;
	for (i = 0; i < WEAPON_SLOT_COUNT; i++) {
		slot = &mech->m_weapons[i];
		type = slot->m_type;
		if (type < 0) {
			continue;
		}

		if (type == AMS_WEAPON_TYPE) {
			if (slot->m_state == c_weaponOff) {
				slot->m_state = c_weaponReady;
				message = 1;
			}
			else if (slot->m_state == c_weaponReady) {
				slot->m_state = c_weaponOff;
				message = 2;
			}
		}
	}

	if (message != -1) {
		PlayCockpitSound(10, message);
	}
}

// The AMS's frame function (WeaponDef::m_frame): a ready AMS fires a volley when an enemy missile
// within its reach closes on the player.
// FUNCTION: MW2GBL 0x1002e997
MechS32 AmsFrame(Player* p_player, WeaponDef* p_def, WeaponSlot* p_slot)
{
	MechS32 side;
	MechS32 i;
	Shot* shot;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;
	MechScalar playerX;
	MechScalar playerY;
	MechScalar playerZ;

	if (p_slot->m_state != c_weaponReady) {
		return 1;
	}

	side = GetPlayerSide(p_player->m_index);
	for (i = 0; i < SHOT_COUNT; i++) {
		shot = &g_shots[i];
		if (shot->m_flags && shot->m_object &&
			(shot->m_type == 3 || shot->m_type == 4 || shot->m_type == 10 || shot->m_type == 0xc ||
			 shot->m_type == 9) &&
			GetPlayerSide(shot->m_shooter) != side) {
			GetObjPosition(shot->m_object, &x, &y, &z);
			playerX = p_player->m_position.m_x;
			playerY = p_player->m_position.m_y;
			playerZ = p_player->m_position.m_z;
			dx = playerX - x;
			dy = playerY - y;
			dz = playerZ - z;
			if (ApproximateVectorLength(dx, dy, dz) <= AMS_REACH &&
				shot->m_velocity[2] * dz + shot->m_velocity[1] * dy + shot->m_velocity[0] * dx > 0) {
				p_slot->m_state = c_weaponFiring;
				p_slot->m_volley = p_def->m_volley;
				p_slot->m_time = 0;
				p_slot->m_target = p_slot->m_targetKind = 0;
				DodgeShot(p_slot, p_player->m_mech);
			}
		}
	}

	return 1;
}

// The AMS's launch function (WeaponDef::m_launch): fires one to five rounds of the volley, then
// gives the missiles it reaches their ranges. Returns the rounds fired, 0 when the AMS can't fire
// again.
// FUNCTION: MW2GBL 0x1002eb5f
MechS32 AmsLaunch(Player* p_player, WeaponDef* p_def, WeaponSlot* p_slot)
{
	MechS32 fired;

	fired = 2;
	if (!p_player || !p_player->m_firingObj || !p_slot->m_ammo) {
		return 0;
	}

	if (p_player->m_index == g_localPlayerId) {
		g_careerRecord.m_shotsFired++;
	}

	if (g_players[p_player->m_index]->m_team == g_players[g_localPlayerId]->m_team) {
		g_careerRecord.m_teamShotsFired++;
	}

	if (p_slot->m_ammo != -1 && (!g_difficulty->m_unlimitedAmmo || p_player->m_index != g_localPlayerId)) {
		fired = RandomIntBelow(5) + 1;
		if (p_slot->m_volley - fired < 0) {
			fired = p_slot->m_volley;
		}

		p_slot->m_ammo -= fired;
		if (p_slot->m_ammo < 0) {
			p_slot->m_ammo = 0;
		}

		if (p_slot->m_bin && p_slot->m_bin->m_shots) {
			p_slot->m_bin->m_shots--;
		}
		else {
			p_slot->m_binIndex++;
			if (p_slot->m_binCount == p_slot->m_binIndex) {
				p_slot->m_binIndex--;
			}
			else {
				p_slot->m_bin++;
			}
		}

		if (p_slot->m_ammo == 0) {
			p_slot->m_state = c_weaponEmpty;
			fired = 0;
			PlayCockpitSound(10, 0);
		}
	}

	AmsIntercept(p_player, p_def, p_slot);
	return fired;
}

// Gives each enemy missile within the AMS's reach that closes on the player, once, the range from
// the player at which it detonates: three times in four a random one beyond AMS_MIN_INTERCEPT,
// otherwise none; and detonates the missiles within theirs. p_def and p_slot go unused.
// FUNCTION: MW2GBL 0x1002ecfd
MechS32 AmsIntercept(Player* p_player, WeaponDef* p_def, WeaponSlot* p_slot)
{
	MechS32 side;
	MechS32 i;
	Shot* shot;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;
	MechScalar distance;
	MechS32 beyond;
	MechScalar playerX;
	MechScalar playerY;
	MechScalar playerZ;

	side = GetPlayerSide(p_player->m_index);
	for (i = 0; i < SHOT_COUNT; i++) {
		shot = &g_shots[i];
		if (shot->m_flags && shot->m_object &&
			(shot->m_type == 3 || shot->m_type == 4 || shot->m_type == 10 || shot->m_type == 0xc ||
			 shot->m_type == 9) &&
			GetPlayerSide(shot->m_shooter) != side) {
			GetObjPosition(shot->m_object, &x, &y, &z);
			playerX = p_player->m_position.m_x;
			playerY = p_player->m_position.m_y;
			playerZ = p_player->m_position.m_z;
			dx = playerX - x;
			dy = playerY - y;
			dz = playerZ - z;
			distance = ApproximateVectorLength(dx, dy, dz);
			if (distance <= AMS_REACH) {
				if (shot->m_interceptRange < 0) {
					beyond = distance - AMS_MIN_INTERCEPT;
					if (beyond > 0) {
						if (shot->m_velocity[2] * dz + shot->m_velocity[0] * dx + shot->m_velocity[1] * dy > 0) {
							if (RandomIntBelow(100) <= 75) {
								if (beyond < 100) {
									shot->m_interceptRange = AMS_MIN_INTERCEPT;
								}
								else {
									shot->m_interceptRange = RandomIntBelow(beyond / 100) * 100 + AMS_MIN_INTERCEPT;
								}
							}
							else {
								shot->m_interceptRange = 0;
							}
						}
					}
					else {
						shot->m_interceptRange = 0;
					}
				}

				if (shot->m_interceptRange > 0 && distance <= shot->m_interceptRange) {
					shot->m_flags |= c_shotProximityFuse;
				}
			}
		}
	}

	return 1;
}
