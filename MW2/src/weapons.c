#include "weapons.h"

#include "careerrecord.h"
#include "clock.h"
#include "collision.h"
#include "config.h"
#include "decomp.h"
#include "effectinfo.h"
#include "environment.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "fixedfloat.h"
#include "fixedmul.h"
#include "fixedsqrt.h"
#include "fixedtrig.h"
#include "inradius.h"
#include "maneuvers.h"
#include "mech.h"
#include "mechclass.h"
#include "mw2prj.h"
#include "network.h"
#include "object.h"
#include "players.h"
#include "polydraw.h"
#include "ramp.h"
#include "random.h"
#include "ray.h"
#include "resource.h"
#include "shape.h"
#include "shots.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "timedoverlays.h"
#include "transform.h"
#include "types.h"
#include "weapondata.h"
#include "weapondef.h"
#include "weaponslot.h"
#ifdef MW2_SHOT_TRACERS
#include "premercs/tracers.h"
#endif
#ifdef MW2_WEAPON_FUNCTIONS
#include "animation.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef MW2_ABS_CALLS
#pragma function(abs) // GBL calls abs in this unit (util/types.h)
#endif

// A mech's weapons: selecting them and their groups, firing volleys, the ammunition and the
// target lock of guided weapons.

// GLOBAL: MW2 0x100a6d34
// GLOBAL: MW2GBL 0x100b0068
// GLOBAL: MW2PREMERCS 0x100bfaec
// GLOBAL: MW2MATROX 0x100a61c8
// GLOBAL: MW23DFX 0x1025a664
// GLOBAL: MW2ATI 0x1018e95c
// GLOBAL: MW2S3 0x10066284
// GLOBAL: MW2POWERVR 0x101b9c6c
Shape* g_aimedShape = NULL;

// The fire button fires only the selected weapon; cleared, it fires them all.
// GLOBAL: MW2 0x100a6d38
// GLOBAL: MW2GBL 0x100b006c
// GLOBAL: MW2PREMERCS 0x100bfaf0
// GLOBAL: MW2MATROX 0x100a61cc
// GLOBAL: MW23DFX 0x1025a668
// GLOBAL: MW2S3 0x10066288
// GLOBAL: MW2POWERVR 0x101b9c70
MechS32 g_singleWeaponFire = 1;

// The weapons each remote player's weapons message fired, and those the local player fired since
// the last one (network.c), one per weapon slot: every loop stops at 10. The 3Dfx edition leaves 4
// bytes after g_localWeaponsFired, as it does after other globals (g_atanTable); the other rooms
// are 16-byte alignment.

// GLOBAL: MW2 0x101099c0
// GLOBAL: MW2GBL 0x100e97e0
// GLOBAL: MW2PREMERCS 0x101569b0
// GLOBAL: MW2MATROX 0x101d4bd0
// GLOBAL: MW23DFX 0x10159470
// GLOBAL: MW2ATI 0x1008d480
// GLOBAL: MW2S3 0x100abcd0
// GLOBAL: MW2POWERVR 0x10083a60
MechS32 g_remoteWeaponsFired[WEAPON_SLOT_COUNT];

// GLOBAL: MW2 0x101099f0
// GLOBAL: MW2GBL 0x100e9820
// GLOBAL: MW2PREMERCS 0x101569f0
// GLOBAL: MW2MATROX 0x101d4c00
// GLOBAL: MW23DFX 0x101594a0
// GLOBAL: MW2ATI 0x1008d4b0
// GLOBAL: MW2S3 0x100abd00
// GLOBAL: MW2POWERVR 0x10083a90
MechS32 g_localWeaponsFired[WEAPON_SLOT_COUNT];

// FUNCTION: MW2 0x10044740
// FUNCTION: MW2GBL 0x100471b0
// FUNCTION: MW2PREMERCS 0x1007ec70
// FUNCTION: MW2MATROX 0x1002b4e0
// FUNCTION: MW23DFX 0x10043760
// FUNCTION: MW2ATI 0x10040fb0
// FUNCTION: MW2S3 0x10028b00
// FUNCTION: MW2POWERVR 0x1003bd10
void ReleaseWeaponTriggers(Mech* p_mech)
{
	WeaponSlot* slot;
	MechS32 i;

	for (i = 0; i < WEAPON_SLOT_COUNT; i++) {
		slot = &p_mech->m_weapons[i];
		if (slot->m_state == c_weaponFiring || slot->m_state == c_weaponHeld) {
			slot->m_state = c_weaponReady;
		}
	}

	for (i = 0; i < WEAPON_SLOT_COUNT; i++) {
		g_localWeaponsFired[i] = 0;
		g_remoteWeaponsFired[i] = 0;
	}
}

#ifdef MW2_WEAPON_FUNCTIONS
// GBL's: each weapon's frame function runs every frame, its launch function fires the shots (a
// volley loses as many as it fires), and the launch sound plays at the first shot or every shot as
// the weapon's flags say.
// FUNCTION: MW2GBL 0x10047251
void UpdateWeaponFireState(Mech* p_mech)
{
	MechScalar dy;
	MechS32 first;
	MechScalar dx;
	MechScalar dz;
	WeaponSlot* slot;
	MechS32 time;
	WeaponDef* def;
	MechS32 i;
	MechS32 pan;
	MechS32 fired;

	first = FALSE;
	if (p_mech->m_powerState != 2) {
		return;
	}

	p_mech->m_lastSelectedWeapon = p_mech->m_selectedWeapon;
	if (p_mech->m_player->m_steering->m_toggleGroupFire) {
		if (g_singleWeaponFire) {
			g_singleWeaponFire = 0;
			PlayCockpitSound(7, 1);
		}
		else {
			g_singleWeaponFire = 1;
			PlayCockpitSound(6, 1);
		}
	}

	if (p_mech->m_flags & 0x4000) {
		AddNextWeaponToGroup(p_mech);
	}
	else if (p_mech->m_selectedWeapon != -1) {
		slot = &p_mech->m_weapons[p_mech->m_selectedWeapon];
		def = &g_weaponDefs[slot->m_type];
		if (!g_singleWeaponFire && p_mech->m_player->m_index == g_localPlayerId) {
			p_mech->m_player->m_steering->m_weaponFireGroup = p_mech->m_player->m_steering->m_weaponFire;
			p_mech->m_player->m_steering->m_weaponFire = 0;
		}

		if (p_mech->m_player->m_steering->m_weaponFireGroup) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				FireWeaponGroup(p_mech);
			}
		}
		else if (p_mech->m_player->m_steering->m_weaponFireGroup1) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				if (SelectWeaponGroup(p_mech, 0)) {
					FireWeaponGroup(p_mech);
				}
			}
		}
		else if (p_mech->m_player->m_steering->m_weaponFireGroup2) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				if (SelectWeaponGroup(p_mech, 1)) {
					FireWeaponGroup(p_mech);
				}
			}
		}
		else if (p_mech->m_player->m_steering->m_weaponFireGroup3) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				if (SelectWeaponGroup(p_mech, 2)) {
					FireWeaponGroup(p_mech);
				}
			}
		}
		else if (p_mech->m_player->m_steering->m_weaponFire) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				if (slot->m_state == c_weaponReady) {
					slot->m_state = c_weaponFiring;
					slot->m_volley = def->m_volley;
					slot->m_time = 0;
					DodgeShot(slot, p_mech);
					if (def->m_guided && (p_mech->m_flags & 0x80)) {
						slot->m_target = p_mech->m_player->m_targetInfo.m_target & 0xff;
						slot->m_targetKind = p_mech->m_player->m_targetInfo.m_target & 0xf00;
					}
					else {
						slot->m_target = slot->m_targetKind = 0;
					}
				}
				else {
					PlaySoundEffect(0x149, 100, 0x40, 5, 0x32);
				}
			}
		}
		else if (p_mech->m_flags & 1) {
			p_mech->m_flags &= ~1;
			SelectNextWeaponInGroup(p_mech, 0);
		}
	}

	for (i = 0; i < WEAPON_SLOT_COUNT; i++) {
		slot = &p_mech->m_weapons[i];
		if (slot->m_type < 0) {
			continue;
		}

		def = &g_weaponDefs[slot->m_type];
		if (def->m_frame) {
			def->m_frame(p_mech->m_player, def, slot);
		}

		switch (slot->m_state) {
		case c_weaponRecycling:
			if (g_currentClock - slot->m_time >= def->m_recycle) {
				slot->m_state = c_weaponReady;
				slot->m_time = 0;
			}
			break;
		case c_weaponHeld:
			if (g_currentClock - slot->m_time >= def->m_recycle) {
				if (&p_mech->m_weapons[p_mech->m_selectedWeapon] == slot ||
					p_mech->m_player->m_steering->m_weaponFireGroup ||
					p_mech->m_player->m_steering->m_weaponFireGroup1 ||
					p_mech->m_player->m_steering->m_weaponFireGroup2 ||
					p_mech->m_player->m_steering->m_weaponFireGroup3) {
					slot->m_state = c_weaponFiring;
				}
				else {
					slot->m_state = c_weaponReady;
				}

				slot->m_time = 0;
			}
			break;
		case c_weaponFiring:
			if (slot->m_volley > 0) {
				if (slot->m_volley == def->m_volley) {
					first = TRUE;
				}

				time = slot->m_time + g_deltaTime;
				while (def->m_interval <= time && slot->m_volley > 0) {
					p_mech->m_player->m_firingObj = p_mech->m_objects[slot->m_hardpoint];
					fired = 0;
					if (def->m_launch) {
						fired = def->m_launch(p_mech->m_player, def, slot);
					}

					if (fired) {
						SpawnLaunchEffect(def->m_launchEffect, p_mech->m_player);
						SpawnLaunchEffect(def->m_launchEffect2, p_mech->m_player);
						slot->m_volley -= fired;
						time -= def->m_interval;
						slot->m_time = time;
						p_mech->m_deltaHeat += def->m_heat;

						if (((def->m_flags & c_weaponSoundFirst) && first) || (def->m_flags & c_weaponSoundEach)) {
							first = FALSE;
							if (p_mech->m_player->m_index == g_localPlayerId) {
								g_localWeaponsFired[slot->m_index] = 1;
							}

							if (def->m_sound > 0) {
								if (p_mech->m_player->m_index == g_localPlayerId && g_inCockpitView) {
									switch (slot->m_hardpoint) {
									case 2:
									case 5:
										pan = 0x4f;
										break;
									case 4:
									case 6:
										pan = 0x2f;
										break;
									default:
										pan = 0x40;
									}

									if (def->m_flags & c_weaponSoundEach) {
										PlayDelayedSound(0, 0, def->m_sound, 0x32, pan, 0x432);
									}
									else {
										PlayDelayedSound(0, 0, def->m_sound, 0x32, pan, 0x32);
									}
								}
								else {
									dx = g_eyepoint->m_x - p_mech->m_player->m_position.m_x;
									dy = g_eyepoint->m_y - p_mech->m_player->m_position.m_y;
									dz = g_eyepoint->m_z - p_mech->m_player->m_position.m_z;
									if (def->m_flags & c_weaponSoundEach) {
										PlayPositionalSoundEx(dx, dy, dz, def->m_sound, g_inCockpitView, 0x432);
									}
									else {
										PlayPositionalSoundEx(dx, dy, dz, def->m_sound, g_inCockpitView, 0x32);
									}
								}
							}
						}

						if (slot->m_volley <= 0 && def->m_autoRepeat &&
							(p_mech->m_player->m_steering->m_weaponFire ||
							 p_mech->m_player->m_steering->m_weaponFireGroup ||
							 p_mech->m_player->m_steering->m_weaponFireGroup1 ||
							 p_mech->m_player->m_steering->m_weaponFireGroup2 ||
							 p_mech->m_player->m_steering->m_weaponFireGroup3)) {
							slot->m_time = g_currentClock;
							slot->m_state = c_weaponHeld;
							slot->m_volley = def->m_volley;
							break;
						}
					}
					else {
						p_mech->m_flags &= ~1;
						slot->m_volley = 0;
					}
				}

				if (slot->m_state != c_weaponHeld) {
					slot->m_time = time;
				}
			}
			else {
				slot->m_time = g_currentClock;
				slot->m_state = c_weaponRecycling;
				slot->m_volley = 0;
			}
			break;
		}
	}
}
#else
// FUNCTION: MW2 0x100447e1
// FUNCTION: MW2MATROX 0x1002b581
// FUNCTION: MW23DFX 0x100437a0
// FUNCTION: MW2ATI 0x10040ff0
// FUNCTION: MW2S3 0x10028b40
// FUNCTION: MW2POWERVR 0x1003bd50
void UpdateWeaponFireState(Mech* p_mech)
{
	MechScalar dy;
	MechS32 first;
	MechScalar dx;
	MechScalar dz;
	WeaponSlot* slot;
	MechS32 time;
	WeaponDef* def;
	MechS32 i;
	MechS32 pan;
	MechS32 fired;

	first = FALSE;
	if (p_mech->m_powerState != 2) {
		return;
	}

	p_mech->m_lastSelectedWeapon = p_mech->m_selectedWeapon;
	if (p_mech->m_player->m_steering->m_toggleGroupFire) {
		if (g_singleWeaponFire) {
			g_singleWeaponFire = 0;
			PlayCockpitSound(7, 1);
		}
		else {
			g_singleWeaponFire = 1;
			PlayCockpitSound(6, 1);
		}
	}

	if (p_mech->m_flags & 0x4000) {
		AddNextWeaponToGroup(p_mech);
	}
	else if (p_mech->m_selectedWeapon != -1) {
		slot = &p_mech->m_weapons[p_mech->m_selectedWeapon];
		def = &g_weaponDefs[slot->m_type];
		if (!g_singleWeaponFire && p_mech->m_player->m_index == g_localPlayerId) {
			p_mech->m_player->m_steering->m_weaponFireGroup = p_mech->m_player->m_steering->m_weaponFire;
			p_mech->m_player->m_steering->m_weaponFire = 0;
		}

		if (p_mech->m_player->m_steering->m_weaponFireGroup) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				FireWeaponGroup();
			}
		}
		else if (p_mech->m_player->m_steering->m_weaponFireGroup1) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				if (SelectWeaponGroup(p_mech, 0)) {
					FireWeaponGroup();
				}
			}
		}
		else if (p_mech->m_player->m_steering->m_weaponFireGroup2) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				if (SelectWeaponGroup(p_mech, 1)) {
					FireWeaponGroup();
				}
			}
		}
		else if (p_mech->m_player->m_steering->m_weaponFireGroup3) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				if (SelectWeaponGroup(p_mech, 2)) {
					FireWeaponGroup();
				}
			}
		}
		else if (p_mech->m_player->m_steering->m_weaponFire) {
			if (!(p_mech->m_flags & 1)) {
				p_mech->m_flags |= 1;
				if (slot->m_state == c_weaponReady) {
					slot->m_state = c_weaponFiring;
					slot->m_volley = def->m_volley;
					slot->m_time = 0;
					DodgeShot(slot, p_mech);
					if (def->m_guided && (p_mech->m_flags & 0x80)) {
						slot->m_target = p_mech->m_player->m_targetInfo.m_target & 0xff;
						slot->m_targetKind = p_mech->m_player->m_targetInfo.m_target & 0xf00;
					}
					else {
						slot->m_target = slot->m_targetKind = 0;
					}
				}
				else {
					PlaySoundEffect(0x149, 100, 0x40, 5, 0x32);
				}
			}
		}
		else if (p_mech->m_flags & 1) {
			p_mech->m_flags &= ~1;
			SelectNextWeaponInGroup(p_mech, 0);
		}
	}

	for (i = 0; i < WEAPON_SLOT_COUNT; i++) {
		slot = &p_mech->m_weapons[i];
		if (slot->m_type < 0) {
			continue;
		}

		def = &g_weaponDefs[slot->m_type];
		switch (slot->m_state) {
		case c_weaponRecycling:
#ifdef MW2_CHEAT_RECYCLE
			if (g_currentClock - slot->m_time >= def->m_recycle || (g_difficulty->m_unlimitedAmmo & 2)) {
#else
			if (g_currentClock - slot->m_time >= def->m_recycle) {
#endif
				slot->m_state = c_weaponReady;
				slot->m_time = 0;
			}
			break;
		case c_weaponHeld:
#ifdef MW2_CHEAT_RECYCLE_HELD
			if (g_currentClock - slot->m_time >= def->m_recycle || (g_difficulty->m_unlimitedAmmo & 2)) {
#else
			if (g_currentClock - slot->m_time >= def->m_recycle) {
#endif
				if (&p_mech->m_weapons[p_mech->m_selectedWeapon] == slot ||
					p_mech->m_player->m_steering->m_weaponFireGroup ||
					p_mech->m_player->m_steering->m_weaponFireGroup1 ||
					p_mech->m_player->m_steering->m_weaponFireGroup2 ||
					p_mech->m_player->m_steering->m_weaponFireGroup3) {
					slot->m_state = c_weaponFiring;
				}
				else {
					slot->m_state = c_weaponReady;
				}

				slot->m_time = 0;
			}
			break;
		case c_weaponFiring:
			if (slot->m_volley > 0) {
				if (slot->m_volley == def->m_volley) {
					first = TRUE;
				}

				time = slot->m_time + g_deltaTime;
				while (def->m_interval <= time && slot->m_volley > 0) {
					p_mech->m_player->m_firingObj = p_mech->m_objects[slot->m_hardpoint];
					fired = SpawnShot(p_mech->m_player, slot);
					if (fired) {
						SpawnLaunchEffect(def->m_launchEffect, p_mech->m_player);
						SpawnLaunchEffect(def->m_launchEffect2, p_mech->m_player);
						slot->m_volley--;
						time -= def->m_interval;
						slot->m_time = time;
						p_mech->m_deltaHeat += def->m_heat;

						if (first) {
							first = FALSE;
							if (p_mech->m_player->m_index == g_localPlayerId) {
								g_localWeaponsFired[slot->m_index] = 1;
							}

							if (def->m_sound > 0) {
								if (p_mech->m_player->m_index == g_localPlayerId && g_inCockpitView) {
									switch (slot->m_hardpoint) {
									case 2:
									case 5:
										pan = 0x4f;
										break;
									case 4:
									case 6:
										pan = 0x2f;
										break;
									default:
										pan = 0x40;
									}

									PlayWeaponLaunchSound(def->m_shotType, def->m_sound, pan);
								}
								else {
									dx = g_eyepoint->m_x - p_mech->m_player->m_position.m_x;
									dy = g_eyepoint->m_y - p_mech->m_player->m_position.m_y;
									dz = g_eyepoint->m_z - p_mech->m_player->m_position.m_z;
									PlaySoundAt(dx, dy, dz, def->m_sound, g_inCockpitView);
								}
							}
						}

						if (slot->m_volley == 0 && def->m_autoRepeat &&
							(p_mech->m_player->m_steering->m_weaponFire ||
							 p_mech->m_player->m_steering->m_weaponFireGroup ||
							 p_mech->m_player->m_steering->m_weaponFireGroup1 ||
							 p_mech->m_player->m_steering->m_weaponFireGroup2 ||
							 p_mech->m_player->m_steering->m_weaponFireGroup3)) {
							slot->m_time = g_currentClock;
							slot->m_state = c_weaponHeld;
							slot->m_volley = def->m_volley;
							break;
						}
					}
					else {
						p_mech->m_flags &= ~1;
						slot->m_volley = 0;
					}
				}

				if (slot->m_state != c_weaponHeld) {
					slot->m_time = time;
				}
			}
			else {
				slot->m_time = g_currentClock;
				slot->m_state = c_weaponRecycling;
				slot->m_volley = 0;
			}
			break;
		}
	}
}
#endif

#if defined(MW2_SHOT_TRACERS)
// Launches one shot of the weapon p_def in p_slot from the player's current hardpoint: the early
// Mercenaries build's, which fires with no ammunition left under the unlimited-ammunition cheat and
// starts a tracer ahead of each machine-gun shot. Returns 0 when the weapon can't fire again.
// FUNCTION: MW2PREMERCS 0x1007f69c
MechS32 SpawnShot(Player* p_player, WeaponDef* p_def, WeaponSlot* p_slot)
{
	MechS32 result;
	Shot* shot;
	MechS32 found;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;
	MechS32 i;
	MechScalar speed;
	MechS32 spread;
	MechS32 j;
	MechScalar x;
	MechScalar y;
	MechScalar z;

	result = 1;
	if (!p_player || !p_player->m_firingObj) {
		return 0;
	}

	found = FALSE;
	if (!p_slot->m_ammo && !g_difficulty->m_unlimitedAmmo) {
		return 0;
	}

	if (p_player->m_index == g_localPlayerId) {
		g_careerRecord.m_shotsFired++;
	}

	if (g_players[p_player->m_index]->m_team == g_players[g_localPlayerId]->m_team) {
		g_careerRecord.m_teamShotsFired++;
	}

	if (p_slot->m_ammo != -1 && (!g_difficulty->m_unlimitedAmmo || p_player->m_index != g_localPlayerId)) {
		p_slot->m_ammo--;
		if (p_slot->m_bin->m_shots) {
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
			result = 0;
		}
	}

	if (p_def->m_launchesShot) {
		for (i = 0; !found && i < SHOT_COUNT; i++) {
			shot = &g_shots[i];
			if (p_def->m_shotType == shot->m_type && !shot->m_flags && shot->m_object) {
				found = TRUE;
			}
		}

		if (found) {
			if (p_player->m_index != g_localPlayerId) {
				UpdateAimDistance(p_player);
			}

			shot->m_weaponFlags = p_def->m_flags;
			shot->m_age = p_slot->m_time;
			shot->m_lifetime = p_def->m_lifetime - p_slot->m_time;
			shot->m_impact = p_def->m_impact;
			shot->m_damage = p_def->m_damage;
			shot->m_heat = p_def->m_shotHeat;
			shot->m_flags = 1;
			shot->m_unk0x3c = 0;
			shot->m_target = p_slot->m_target;
			shot->m_targetKind = p_slot->m_targetKind;
			shot->m_shooter = p_player->m_index;
			if (p_def->m_flags & c_weaponAnimation) {
				for (j = 0; j < 8; j++) {
					SetAnimFrame(p_def->m_animation + j, 0);
					SetAnimMode(p_def->m_animation + j, 1);
				}
			}

			speed = p_def->m_speed;
			GetMechAimDirection(p_player, &dx, &dy, &dz);
			NormalizeVectorGuarded(&dx, &dy, &dz);
			shot->m_velocity[0] = dx * speed;
			shot->m_velocity[1] = dy * speed;
			shot->m_velocity[2] = dz * speed;
			shot->m_steering[1] = FixedMul16(-g_gravity, p_def->m_gravity << 16);
			shot->m_steering[0] = shot->m_steering[2] = 0;
			spread = shot->m_type == 3 || shot->m_type == 10 || shot->m_type == 0xc || shot->m_type == 9 ||
					 shot->m_type == 4;
			SpawnLaunchFx(p_player, shot->m_object, dx, dy, dz, spread, p_def);
			ShowObjTree(shot->m_object);
			DisableObjTreeCollision(shot->m_object);
			UpdateObj(shot->m_object);
			GetObjPosition(shot->m_object, &x, &y, &z);
			if (shot->m_type == 5) {
				FUN_10089624(
					shot->m_velocity[0] * 2 + x,
					shot->m_velocity[1] * 2 + y,
					shot->m_velocity[2] * 2 + z,
					shot->m_velocity[0],
					shot->m_velocity[1],
					shot->m_velocity[2]
				);
			}

			if (p_player->m_index == g_localPlayerId) {
				if (shot->m_type == 3 || shot->m_type == 4 || shot->m_type == 10 || shot->m_type == 0xc ||
					shot->m_type == 9) {
					g_lastLocalMissile = i - 1;
				}
				else {
					g_lastLocalMissile = -1;
				}
			}
		}
	}

	return result;
}
#elif defined(MW2_WEAPON_FUNCTIONS)
// Launches one shot of the weapon p_def in p_slot from the player's current hardpoint: GBL's, every
// weapon's launch function but the AMS's (WeaponDef::m_launch). Returns 0 when the weapon can't fire
// again: no hardpoint, or out of ammunition.
// FUNCTION: MW2GBL 0x10047bc0
MechS32 SpawnShot(Player* p_player, WeaponDef* p_def, WeaponSlot* p_slot)
{
	MechS32 result;
	Shot* shot;
	MechS32 found;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;
	MechS32 i;
	MechScalar speed;
	MechS32 spread;
	MechS32 j;

	result = 1;
	if (!p_player || !p_player->m_firingObj) {
		return 0;
	}

	found = FALSE;
	if (!p_slot->m_ammo) {
		return 0;
	}

	if (p_player->m_index == g_localPlayerId) {
		g_careerRecord.m_shotsFired++;
	}

	if (g_players[p_player->m_index]->m_team == g_players[g_localPlayerId]->m_team) {
		g_careerRecord.m_teamShotsFired++;
	}

	if (p_slot->m_ammo != -1 && (!g_difficulty->m_unlimitedAmmo || p_player->m_index != g_localPlayerId)) {
		p_slot->m_ammo--;
		if (p_slot->m_bin->m_shots) {
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
			result = 0;
		}
	}

	if (p_def->m_launchesShot) {
		for (i = 0; !found && i < SHOT_COUNT; i++) {
			shot = &g_shots[i];
			if (p_def->m_shotType == shot->m_type && !shot->m_flags && shot->m_object) {
				found = TRUE;
			}
		}

		if (found) {
			if (p_player->m_index != g_localPlayerId) {
				UpdateAimDistance(p_player);
			}

			shot->m_weaponFlags = p_def->m_flags;
			shot->m_age = p_slot->m_time;
			shot->m_lifetime = p_def->m_lifetime - p_slot->m_time;
			shot->m_impact = p_def->m_impact;
			shot->m_damage = p_def->m_damage;
			shot->m_heat = p_def->m_shotHeat;
			shot->m_flags = 1;
			shot->m_unk0x3c = 0;
			shot->m_target = p_slot->m_target;
			shot->m_targetKind = p_slot->m_targetKind;
			shot->m_shooter = p_player->m_index;
			if (p_def->m_flags & c_weaponAnimation) {
				for (j = 0; j < 8; j++) {
					SetAnimFrame(p_def->m_animation + j, 0);
					SetAnimMode(p_def->m_animation + j, 1);
				}
			}

			speed = p_def->m_speed;
			GetMechAimDirection(p_player, &dx, &dy, &dz);
			NormalizeVectorGuarded(&dx, &dy, &dz);
			shot->m_velocity[0] = dx * speed;
			shot->m_velocity[1] = dy * speed;
			shot->m_velocity[2] = dz * speed;
			shot->m_steering[1] = FixedMul16(-g_gravity, p_def->m_gravity << 16);
			shot->m_steering[0] = shot->m_steering[2] = 0;
			spread = shot->m_type == 3 || shot->m_type == 10 || shot->m_type == 0xc || shot->m_type == 9 ||
					 shot->m_type == 4;
			SpawnLaunchFx(p_player, shot->m_object, dx, dy, dz, spread, p_def);
			ShowObjTree(shot->m_object);
			DisableObjTreeCollision(shot->m_object);
			UpdateObj(shot->m_object);

			if (p_player->m_index == g_localPlayerId) {
				if (shot->m_type == 3 || shot->m_type == 4 || shot->m_type == 10 || shot->m_type == 0xc ||
					shot->m_type == 9) {
					g_lastLocalMissile = i - 1;
				}
				else {
					g_lastLocalMissile = -1;
				}
			}
		}
	}

	return result;
}
#else
// Launches one shot of p_slot's weapon from the player's current hardpoint. Returns 0 when the
// weapon can't fire again: no hardpoint, or out of ammunition.
// FUNCTION: MW2 0x1004506c
// FUNCTION: MW2MATROX 0x1002be15
// FUNCTION: MW23DFX 0x10043cc0
// FUNCTION: MW2ATI 0x10041510
// FUNCTION: MW2S3 0x10029070
// FUNCTION: MW2POWERVR 0x1003c270
MechS32 SpawnShot(Player* p_player, WeaponSlot* p_slot)
{
	MechS32 result;
	Shot* shot;
	WeaponDef* def;
	MechS32 found;
	MechScalar dx;
	MechScalar dy;
	MechScalar dz;
	MechS32 i;
	MechScalar speed;
	MechS32 spread;

	def = &g_weaponDefs[p_slot->m_type];
	result = 1;
	if (!p_player || !p_player->m_firingObj) {
		return 0;
	}

	found = FALSE;
	if (!p_slot->m_ammo) {
		return 0;
	}

	if (p_player->m_index == g_localPlayerId) {
		g_careerRecord.m_shotsFired++;
	}

	if (g_players[p_player->m_index]->m_team == g_players[g_localPlayerId]->m_team) {
		g_careerRecord.m_teamShotsFired++;
	}

	if (p_slot->m_ammo != -1 && (!g_difficulty->m_unlimitedAmmo || p_player->m_index != g_localPlayerId) &&
		(!g_isNetworkGame || p_player->m_index == g_localPlayerId)) {
		p_slot->m_ammo--;
		if (p_slot->m_bin->m_shots) {
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
			result = 0;
		}
	}

	if (def->m_launchesShot) {
		for (i = 0; !found && i < SHOT_COUNT; i++) {
			shot = &g_shots[i];
			if (def->m_shotType == shot->m_type && !shot->m_flags && shot->m_object) {
				found = TRUE;
			}
		}

		if (found) {
			if (p_player->m_index != g_localPlayerId) {
				UpdateAimDistance(p_player);
			}

			shot->m_age = p_slot->m_time;
			shot->m_lifetime = def->m_lifetime - p_slot->m_time;
			shot->m_impact = def->m_impact;
			shot->m_damage = def->m_damage;
			shot->m_heat = def->m_shotHeat;
			shot->m_flags = 1;
			shot->m_unk0x3c = 0;
			shot->m_target = p_slot->m_target;
			shot->m_targetKind = p_slot->m_targetKind;
			shot->m_shooter = p_player->m_index;

			speed = def->m_speed;
			GetMechAimDirection(p_player, &dx, &dy, &dz);
			NormalizeVectorGuarded(&dx, &dy, &dz);
			shot->m_velocity[0] = dx * speed;
			shot->m_velocity[1] = dy * speed;
			shot->m_velocity[2] = dz * speed;
#ifdef MW2_FLOAT_SCALARS
			shot->m_steering[1] = -(def->m_gravity * g_gravity);
#else
			shot->m_steering[1] = FixedMul16(-g_gravity, def->m_gravity << 16);
#endif
			shot->m_steering[0] = shot->m_steering[2] = 0;
			spread = shot->m_type == 3 || shot->m_type == 4;
			SpawnLaunchFx(p_player, shot->m_object, dx, dy, dz, spread);
			ShowObjTree(shot->m_object);
			DisableObjTreeCollision(shot->m_object);
			UpdateObj(shot->m_object);

			if (p_player->m_index == g_localPlayerId) {
				if (shot->m_type == 3 || shot->m_type == 4) {
					g_lastLocalMissile = i - 1;
				}
				else {
					g_lastLocalMissile = -1;
				}
			}
		}
	}

	return result;
}
#endif

// Selects the next weapon of the selected one's group; p_wrap moves on to the next group when
// there is none.
// FUNCTION: MW2 0x10045449
// FUNCTION: MW2GBL 0x10048016
// FUNCTION: MW2PREMERCS 0x1007fb6b
// FUNCTION: MW2MATROX 0x1002c1ef
// FUNCTION: MW23DFX 0x10043f80
// FUNCTION: MW2ATI 0x100417d0
// FUNCTION: MW2S3 0x10029330
// FUNCTION: MW2POWERVR 0x1003c530
void SelectNextWeaponInGroup(Mech* p_mech, MechS32 p_wrap)
{
#ifdef MW2_GBL
	MechS32 selected;
	MechS32 index;
	MechS32 group;
	WeaponSlot* slot;
	MechS32 found;
	MechS32 i;
#else
	MechS32 index;
	MechS32 group;
	WeaponSlot* slot;
	MechS32 found;
	MechS32 i;
	MechS32 selected;
#endif
#ifdef MW2_WEAPON_FUNCTIONS
	WeaponDef* def;
#endif

	found = FALSE;
	if (p_mech->m_weaponCount <= 0) {
		return;
	}

	selected = p_mech->m_selectedWeapon;
	group = p_mech->m_weapons[p_mech->m_selectedWeapon].m_group;
	index = selected;
	for (i = 1; i < WEAPON_SLOT_COUNT; i++) {
		index++;
		index %= WEAPON_SLOT_COUNT;
		slot = &p_mech->m_weapons[index];
		if (slot->m_type < 0) {
			continue;
		}

#ifdef MW2_WEAPON_FUNCTIONS
		def = &g_weaponDefs[slot->m_type];
		// The early Mercenaries build tests the index before the flag.
#ifdef MW2_PREMERCS
		if (slot->m_status != -1 && slot->m_group == group && slot->m_state != c_weaponEmpty && index != selected &&
			(def->m_flags & c_weaponSelectable)) {
#else
		if (slot->m_status != -1 && slot->m_group == group && slot->m_state != c_weaponEmpty &&
			(def->m_flags & c_weaponSelectable) && index != selected) {
#endif
#else
		if (slot->m_status != -1 && slot->m_group == group && slot->m_state != c_weaponEmpty && index != selected) {
#endif
			found = TRUE;
			p_mech->m_selectedWeapon = index;
			break;
		}
	}

	if (!found && p_wrap) {
		SelectNextWeaponGroup(p_mech);
	}
}

// Selects the next weapon that still has ammunition.
// FUNCTION: MW2 0x10045567
// FUNCTION: MW2GBL 0x10048159
// FUNCTION: MW2PREMERCS 0x1007fcb2
// FUNCTION: MW2MATROX 0x1002c30d
// FUNCTION: MW23DFX 0x10044030
// FUNCTION: MW2ATI 0x10041880
// FUNCTION: MW2S3 0x100293e0
// FUNCTION: MW2POWERVR 0x1003c5e0
void SelectNextWeapon(Mech* p_mech)
{
	MechS32 done;
	MechS32 tries;
#ifdef MW2_WEAPON_FUNCTIONS
	MechS32 type;
	WeaponDef* def;
#endif

	done = FALSE;
	tries = 0;
	if (p_mech->m_weaponCount <= 0) {
		return;
	}

	while (!done) {
		p_mech->m_selectedWeapon++;
		p_mech->m_selectedWeapon %= WEAPON_SLOT_COUNT;
#ifdef MW2_WEAPON_FUNCTIONS
		type = p_mech->m_weapons[p_mech->m_selectedWeapon].m_type;
		if (type < 0) {
			continue;
		}

		def = &g_weaponDefs[type];
		if (p_mech->m_weapons[p_mech->m_selectedWeapon].m_status == -1) {
			p_mech->m_selectedWeapon = 0;
		}

		if ((p_mech->m_weapons[p_mech->m_selectedWeapon].m_state != c_weaponEmpty &&
			 (def->m_flags & c_weaponSelectable)) ||
			++tries == WEAPON_SLOT_COUNT) {
			done = TRUE;
		}
#else
		if (p_mech->m_weapons[p_mech->m_selectedWeapon].m_type < 0) {
			continue;
		}

		if (g_manualWeaponCycle && p_mech->m_player->m_index == g_localPlayerId) {
			return;
		}

		if (p_mech->m_weapons[p_mech->m_selectedWeapon].m_status == -1) {
			p_mech->m_selectedWeapon = 0;
		}

		if (p_mech->m_weapons[p_mech->m_selectedWeapon].m_state != c_weaponEmpty || ++tries == WEAPON_SLOT_COUNT) {
			done = TRUE;
		}
#endif
	}
}

// Selects the first weapon of the next group that has one.
// FUNCTION: MW2 0x1004567b
// FUNCTION: MW2GBL 0x1004826f
// FUNCTION: MW2PREMERCS 0x1007fdce
// FUNCTION: MW2MATROX 0x1002c421
// FUNCTION: MW23DFX 0x100440c0
// FUNCTION: MW2ATI 0x10041910
// FUNCTION: MW2S3 0x10029470
// FUNCTION: MW2POWERVR 0x1003c670
void SelectNextWeaponGroup(Mech* p_mech)
{
	MechS32 group;
	MechS32 i;
	MechS32 found;
	MechS32 selected;
	MechS32 current;
#ifdef MW2_WEAPON_FUNCTIONS
	MechS32 type;
	WeaponDef* def;
#endif

	i = 0;
	found = FALSE;
	selected = p_mech->m_selectedWeapon;
	if (p_mech->m_weaponCount <= 0) {
		return;
	}

	current = p_mech->m_weapons[p_mech->m_selectedWeapon].m_group;
	group = current + 1;
	group %= WEAPON_SLOT_COUNT;
	while (group != current && !found) {
		for (i = 0; i < WEAPON_SLOT_COUNT && !found; i++) {
#ifdef MW2_WEAPON_FUNCTIONS
			type = p_mech->m_weapons[i].m_type;
			if (type < 0) {
				continue;
			}

			def = &g_weaponDefs[type];
			if (p_mech->m_weapons[i].m_status != -1 && p_mech->m_weapons[i].m_group == group &&
				p_mech->m_weapons[i].m_state != c_weaponEmpty && (def->m_flags & c_weaponSelectable)) {
#else
			if (p_mech->m_weapons[i].m_type < 0) {
				continue;
			}

			if (p_mech->m_weapons[i].m_status != -1 && p_mech->m_weapons[i].m_group == group &&
				p_mech->m_weapons[i].m_state != c_weaponEmpty) {
#endif
				found = TRUE;
				p_mech->m_selectedWeapon = selected = i;
				break;
			}
		}

		group++;
		group %= WEAPON_SLOT_COUNT;
	}
}

// Selects the first weapon of p_group. Returns 1 if the group has one.
// FUNCTION: MW2 0x100457d3
// FUNCTION: MW2GBL 0x100483ef
// FUNCTION: MW2PREMERCS 0x1007ff58
// FUNCTION: MW2MATROX 0x1002c579
// FUNCTION: MW23DFX 0x10044160
// FUNCTION: MW2ATI 0x100419b0
// FUNCTION: MW2S3 0x10029510
// FUNCTION: MW2POWERVR 0x1003c710
MechS32 SelectWeaponGroup(Mech* p_mech, MechS32 p_group)
{
	MechS32 i;
	MechS32 found;
	MechS32 selected;
	MechS32 current;
#ifdef MW2_WEAPON_FUNCTIONS
	MechS32 type;
	WeaponDef* def;
#endif

#ifdef MW2_WEAPON_FUNCTIONS
	type = 0;
#endif
	i = 0;
	found = FALSE;
	if (p_group < 0 || p_group >= 3) {
		return 0;
	}

	if (p_mech->m_weaponCount <= 0) {
		return 0;
	}

	current = p_mech->m_weapons[p_mech->m_selectedWeapon].m_group;
	if (current == p_group) {
		return 1;
	}

	for (i = 0; i < WEAPON_SLOT_COUNT && !found; i++) {
#ifdef MW2_WEAPON_FUNCTIONS
		type = p_mech->m_weapons[i].m_type;
		if (type < 0) {
			continue;
		}

		def = &g_weaponDefs[type];
		if (p_mech->m_weapons[i].m_status != -1 && p_mech->m_weapons[i].m_group == p_group &&
			p_mech->m_weapons[i].m_state != c_weaponEmpty && (def->m_flags & c_weaponSelectable)) {
#else
		if (p_mech->m_weapons[i].m_type < 0) {
			continue;
		}

		if (p_mech->m_weapons[i].m_status != -1 && p_mech->m_weapons[i].m_group == p_group &&
			p_mech->m_weapons[i].m_state != c_weaponEmpty) {
#endif
			found = TRUE;
			p_mech->m_selectedWeapon = selected = i;
			break;
		}
	}

	return found;
}

// Returns 1 if the selected weapon is ready, 0 if not, -1 without one.
// FUNCTION: MW2 0x10045919
// FUNCTION: MW2GBL 0x10048564
// FUNCTION: MW2PREMERCS 0x100800d7
// FUNCTION: MW2MATROX 0x1002c6bf
// FUNCTION: MW23DFX 0x100441f0
// FUNCTION: MW2ATI 0x10041a40
// FUNCTION: MW2S3 0x100295a0
// FUNCTION: MW2POWERVR 0x1003c7a0
MechS32 IsSelectedWeaponReady(Mech* p_mech)
{
	MechS32 result;

	result = -1;
	if (p_mech->m_selectedWeapon != -1) {
		if (p_mech->m_weapons[p_mech->m_selectedWeapon].m_state == c_weaponReady) {
			result = 1;
		}
		else {
			result = 0;
		}
	}

	return result;
}

// Dumps the selected weapon's ammunition.
// FUNCTION: MW2 0x1004597b
// FUNCTION: MW2GBL 0x100485c6
// FUNCTION: MW2PREMERCS 0x1008013b
// FUNCTION: MW2MATROX 0x1002c721
// FUNCTION: MW23DFX 0x10044220
// FUNCTION: MW2ATI 0x10041a70
// FUNCTION: MW2S3 0x100295d0
// FUNCTION: MW2POWERVR 0x1003c7d0
MechS32 JettisonAmmo(Mech* p_mech)
{
	MechChar text[40];
	WeaponSlot* slot;

	if (p_mech->m_selectedWeapon == -1) {
		return 0;
	}

	slot = &p_mech->m_weapons[p_mech->m_selectedWeapon];
	if (g_weaponDefs[slot->m_type].m_volley > 0 && slot->m_ammo > 0) {
		PlaySoundEffect(0xb3, 100, 0x40, 5, 0x50);
		sprintf(text, "Ammo for current weapon jettisonned.");
		ShowInGameMessage(text, 1, 0x16a, 0x32);
		PlaySoundEffect(0xb3, 100, 0x40, 5, 0x32);
		slot->m_ammo = 0;
		slot->m_state = c_weaponEmpty;
		return 1;
	}

	return 0;
}

// Loads the launch sounds of the weapons and the sounds of the effects.
// FUNCTION: MW2 0x10045a5b
// FUNCTION: MW2GBL 0x100486a6
// FUNCTION: MW2PREMERCS 0x1008021d
// FUNCTION: MW2MATROX 0x1002c801
// FUNCTION: MW23DFX 0x100442e0
// FUNCTION: MW2ATI 0x10041b20
// FUNCTION: MW2S3 0x10029680
// FUNCTION: MW2POWERVR 0x1003c880
void LoadWeaponSounds(void)
{
	MechS32 i;
	WeaponDef* def;

#ifdef MW2_WEAPON_FUNCTIONS
	for (i = 0; i < WEAPON_DEF_COUNT; i++) {
#else
	for (i = 0; i < 30; i++) {
#endif
		def = &g_weaponDefs[i];
		if (def->m_sound > 0) {
			PreloadResource(def->m_sound, g_resTagSnds);
		}
	}

	for (i = 0; i < EFFECT_TYPE_COUNT; i++) {
		if (g_effectInfo[i].m_sound > 0) {
			PreloadResource(g_effectInfo[i].m_sound, g_resTagSnds);
		}
	}
}

// FUNCTION: MW2 0x10045b14
// FUNCTION: MW2GBL 0x10048763
// FUNCTION: MW2PREMERCS 0x100802da
// FUNCTION: MW2MATROX 0x1002c8ba
// FUNCTION: MW23DFX 0x10044330
// FUNCTION: MW2ATI 0x10041b70
// FUNCTION: MW2S3 0x100296d0
// FUNCTION: MW2POWERVR 0x1003c8d0
void SetWeaponGroup(Mech* p_mech, MechS32 p_index, MechS32 p_group)
{
	WeaponSlot* slot;

	if (p_group >= 0 && p_group < 3) {
		slot = &p_mech->m_weapons[p_index];
		slot->m_group = p_group;
	}
}

// Moves the local player's selected weapon to p_group.
// FUNCTION: MW2 0x10045b56
// FUNCTION: MW2GBL 0x100487a5
// FUNCTION: MW2PREMERCS 0x1008031e
// FUNCTION: MW2MATROX 0x1002c8fc
// FUNCTION: MW23DFX 0x10044350
// FUNCTION: MW2ATI 0x10041ba0
// FUNCTION: MW2S3 0x100296f0
// FUNCTION: MW2POWERVR 0x1003c8f0
void SetSelectedWeaponGroup(MechS32 p_group)
{
	Mech* mech;
	WeaponSlot* slot;

	mech = g_players[g_localPlayerId]->m_mech;
	slot = &mech->m_weapons[mech->m_selectedWeapon];
	slot->m_group = p_group;
}

// FUNCTION: MW2 0x10045b9c
// FUNCTION: MW2GBL 0x100487eb
// FUNCTION: MW2PREMERCS 0x10080366
// FUNCTION: MW2MATROX 0x1002c942
// FUNCTION: MW23DFX 0x10044380
// FUNCTION: MW2ATI 0x10041bd0
// FUNCTION: MW2S3 0x10029720
// FUNCTION: MW2POWERVR 0x1003c920
void CycleLocalWeaponGroup(void)
{
	Mech* mech;

	mech = g_players[g_localPlayerId]->m_mech;
	SelectNextWeaponGroup(mech);
}

// Fires the weapons g_remoteWeaponsFired lists from p_mech.
// FUNCTION: MW2 0x10045bc8
// FUNCTION: MW2GBL 0x10048817
// FUNCTION: MW2MATROX 0x1002c96e
// FUNCTION: MW23DFX 0x100443a0
// FUNCTION: MW2ATI 0x10041bf0
// FUNCTION: MW2S3 0x10029740
// FUNCTION: MW2POWERVR 0x1003c940
void FireRemoteWeapons(Mech* p_mech)
{
	WeaponSlot* slot;
	WeaponDef* def;
	MechS32 i;

	for (i = 0; i < WEAPON_SLOT_COUNT; i++) {
		if (g_remoteWeaponsFired[i]) {
			g_remoteWeaponsFired[i] = 0;
			slot = &p_mech->m_weapons[i];
			def = &g_weaponDefs[slot->m_type];
			if (slot->m_state != c_weaponEmpty) {
				slot->m_state = c_weaponFiring;
				slot->m_volley = def->m_volley;
				slot->m_time = 0;
				if (def->m_guided && (p_mech->m_flags & 0x80)) {
					slot->m_target = p_mech->m_player->m_targetInfo.m_target & 0xff;
					slot->m_targetKind = p_mech->m_player->m_targetInfo.m_target & 0xf00;
				}
				else {
					slot->m_target = slot->m_targetKind = 0;
				}
			}
		}
	}
}

#ifdef MW2_WEAPON_FUNCTIONS
// Fires every ready weapon of p_mech's selected group.
// FUNCTION: MW2GBL 0x10048927
// FUNCTION: MW2PREMERCS 0x100804b1
void FireWeaponGroup(Mech* p_mech)
{
	MechS32 selected;
	MechS32 first;
	WeaponDef* def;
	WeaponSlot* slot;

	selected = p_mech->m_selectedWeapon;
	SelectNextWeaponInGroup(p_mech, 0);
	first = p_mech->m_selectedWeapon;
	do {
		slot = &p_mech->m_weapons[p_mech->m_selectedWeapon];
		def = &g_weaponDefs[slot->m_type];
		if (slot->m_state == c_weaponReady) {
			slot->m_state = c_weaponFiring;
			slot->m_volley = def->m_volley;
			slot->m_time = 0;
			DodgeShot(slot, p_mech);
			if (def->m_guided && (p_mech->m_flags & 0x80)) {
				slot->m_target = p_mech->m_player->m_targetInfo.m_target & 0xff;
				slot->m_targetKind = p_mech->m_player->m_targetInfo.m_target & 0xf00;
			}
			else {
				slot->m_target = slot->m_targetKind = 0;
			}
		}

		SelectNextWeaponInGroup(p_mech, 0);
	} while (p_mech->m_selectedWeapon != first);

	p_mech->m_selectedWeapon = selected;
}
#else
// Fires every ready weapon of the local player's selected group.
// FUNCTION: MW2 0x10045cd8
// FUNCTION: MW2MATROX 0x1002ca7e
// FUNCTION: MW23DFX 0x10044440
// FUNCTION: MW2ATI 0x10041c90
// FUNCTION: MW2S3 0x100297e0
// FUNCTION: MW2POWERVR 0x1003c9d0
void FireWeaponGroup(void)
{
	Mech* mech;
	MechS32 selected;
	MechS32 first;
	WeaponDef* def;
	WeaponSlot* slot;

	mech = g_players[g_localPlayerId]->m_mech;
	selected = mech->m_selectedWeapon;
	SelectNextWeaponInGroup(mech, 0);
	first = mech->m_selectedWeapon;
	do {
		slot = &mech->m_weapons[mech->m_selectedWeapon];
		def = &g_weaponDefs[slot->m_type];
		if (slot->m_state == c_weaponReady) {
			slot->m_state = c_weaponFiring;
			slot->m_volley = def->m_volley;
			slot->m_time = 0;
			DodgeShot(slot, mech);
			if (def->m_guided && (mech->m_flags & 0x80)) {
				slot->m_target = mech->m_player->m_targetInfo.m_target & 0xff;
				slot->m_targetKind = mech->m_player->m_targetInfo.m_target & 0xf00;
			}
			else {
				slot->m_target = slot->m_targetKind = 0;
			}
		}

		SelectNextWeaponInGroup(mech, 0);
	} while (mech->m_selectedWeapon != first);

	mech->m_selectedWeapon = selected;
}
#endif

// Moves the next weapon into the selected one's group.
// FUNCTION: MW2 0x10045e25
// FUNCTION: MW2GBL 0x10048a62
// FUNCTION: MW2PREMERCS 0x100805ee
// FUNCTION: MW2MATROX 0x1002cbcb
// FUNCTION: MW23DFX 0x10044520
// FUNCTION: MW2ATI 0x10041d70
// FUNCTION: MW2S3 0x100298c0
// FUNCTION: MW2POWERVR 0x1003cab0
void AddNextWeaponToGroup(Mech* p_mech)
{
	MechS32 group;
	MechS32 next;

	p_mech->m_flags ^= 0x4000;
	next = p_mech->m_selectedWeapon;
	group = p_mech->m_weapons[next].m_group;
	next++;
	if (p_mech->m_weapons[next].m_status == -1) {
		next = 0;
	}

	SetWeaponGroup(p_mech, next, group);
}

// Updates the target lock of the selected weapon: with the target between its two ranges and
// within 16 degrees of the aim, it locks on after 0x16a ticks.
// MW2MATROX: in floats (degrees).
// FUNCTION: MW2 0x10045eac
// FUNCTION: MW2GBL 0x10048ae9
// FUNCTION: MW2PREMERCS 0x10080679
// FUNCTION: MW2MATROX 0x1002cc52
// FUNCTION: MW23DFX 0x10044560
// FUNCTION: MW2ATI 0x10041db0
// FUNCTION: MW2S3 0x10029900
// FUNCTION: MW2POWERVR 0x1003caf0
void UpdateWeaponLock(Mech* p_mech)
{
	MechScalar dz;
	MechScalar twist;
	MechScalar pitch;
	MechScalar yaw;
	MechScalar bearing;
	MechS32 inRange;
	MechScalar dx;
	WeaponDef* def;
	MechScalar dy;
	MechScalar heading;

	p_mech->m_flags &= ~0x80;
	def = &g_weaponDefs[p_mech->m_weapons[p_mech->m_selectedWeapon].m_type];
	if (p_mech->m_selectedWeapon == -1 || !p_mech->m_player->m_targetInfo.m_target ||
		(p_mech->m_player->m_targetInfo.m_target & 0x1000) || (p_mech->m_player->m_targetInfo.m_target & 0x100) ||
		!def->m_guided) {
		p_mech->m_flags &= 0x7fff;
		p_mech->m_flags &= 0xffbf;
	}
#ifdef MW2_WEAPON_FUNCTIONS
	else if (
		(p_mech->m_player->m_targetInfo.m_target & 0x200) &&
		(g_players[p_mech->m_player->m_targetInfo.m_target & 0xff]->m_mech->m_unk0x112 & 1)
	) {
		p_mech->m_stateTime = 0;
		p_mech->m_flags |= 0x40;
		p_mech->m_flags |= 0x80;
		p_mech->m_flags |= 0x8000;
	}
#endif
	else {
		dx = p_mech->m_player->m_position.m_x - p_mech->m_player->m_targetInfo.m_position.m_x;
		dy = p_mech->m_player->m_position.m_y - p_mech->m_player->m_targetInfo.m_position.m_y;
		dz = p_mech->m_player->m_position.m_z - p_mech->m_player->m_targetInfo.m_position.m_z;
		if (IsWithinRadius(dx, dy, dz, def->m_shortRange) || !IsWithinRadius(dx, dy, dz, def->m_longRange)) {
			inRange = FALSE;
			yaw = FIXED_LITERAL(0x100001, 17.0f);
			pitch = FIXED_LITERAL(0x100001, 17.0f);
			p_mech->m_flags &= 0x7fff;
		}
		else {
			inRange = TRUE;
			heading = FIXED_MOD360(FIXED_MOD360(p_mech->m_player->m_heading) + FIXED_CONST(360));
			twist = FIXED_MOD360(p_mech->m_player->m_torsoTwist);
			bearing = p_mech->m_player->m_targetInfo.m_heading - heading;
			if (bearing > FIXED_CONST(180)) {
				bearing -= FIXED_CONST(360);
			}
			else if (bearing < -FIXED_CONST(180)) {
				bearing += FIXED_CONST(360);
			}

			yaw = bearing - twist;
			if (yaw > FIXED_CONST(180)) {
				yaw -= FIXED_CONST(360);
			}
			else if (yaw < -FIXED_CONST(180)) {
				yaw += FIXED_CONST(360);
			}

			pitch = FIXED_MOD360(p_mech->m_player->m_targetInfo.m_pitch + p_mech->m_torsoPitch.m_value);
		}

#ifdef MW2_FLOAT_SCALARS
		if (inRange && fabs(yaw) < 16 && fabs(pitch) < 16) {
#else
		if (inRange && abs(yaw) < 0x100000 && abs(pitch) < 0x100000) {
#endif
			p_mech->m_flags |= 0x8000;
			if (!(p_mech->m_flags & 0x40)) {
				p_mech->m_flags |= 0x40;
				p_mech->m_stateTime = 0x16a;
			}
			else if (p_mech->m_stateTime <= 0) {
				p_mech->m_stateTime = 0;
				p_mech->m_flags |= 0x80;
			}
			else {
				p_mech->m_stateTime -= g_deltaTime;
			}
		}
		else {
			p_mech->m_flags &= 0x7fff;
			if (p_mech->m_flags & 0x40) {
				p_mech->m_stateTime += g_deltaTime;
				if (p_mech->m_stateTime >= 0x16a) {
					p_mech->m_flags &= ~0x40;
				}
			}
		}
	}
}

// Casts the player's aim ray and returns the shape it hits, if any. A hit on a mech or a
// building sets the distance the weapons converge at.
// FUNCTION: MW2 0x10046269
// FUNCTION: MW2GBL 0x10048f4f
// FUNCTION: MW2PREMERCS 0x10080ae1
// FUNCTION: MW2MATROX 0x1002d0b1
// FUNCTION: MW23DFX 0x10044810
// FUNCTION: MW2ATI 0x10042060
// FUNCTION: MW2S3 0x10029bb0
// FUNCTION: MW2POWERVR 0x1003cd90
Shape* UpdateAimDistance(Player* p_player)
{
	Ray ray;
	MechScalar length;
	Shape* hit;
	MechU16 flags;
	MechS32 collided;

	hit = NULL;
	UpdateRamp(&p_player->m_aimDistance);
	BuildAimRay(p_player, &ray);
	collided = TestSegmentCollision(&ray, &hit, p_player->m_index);
	if (collided && hit) {
		flags = hit->m_kind;
		if ((flags & 0x100) || (flags & 0x200)) {
#ifdef MW2_FLOAT_SCALARS
			if ((length = GetRayLength(&ray)) > 2000) {
				p_player->m_aimDistance.m_value = length;
			}
			else {
				p_player->m_aimDistance.m_value = 2000;
			}
#else
			length = GetRayLength(&ray);
			p_player->m_aimDistance.m_value = length > 2000 ? length : 2000;
#endif
			if (p_player->m_index == g_localPlayerId) {
				g_aimedShape = hit;
			}
		}
	}

	p_player->m_aimRange.m_target = p_player->m_aimDistance.m_value;
	UpdateRamp(&p_player->m_aimRange);

	return hit;
}

// FUNCTION: MW2 0x1004635c
// FUNCTION: MW2GBL 0x10049042
// FUNCTION: MW2PREMERCS 0x10080bd4
// FUNCTION: MW2MATROX 0x1002d1bc
// FUNCTION: MW23DFX 0x100448d0
// FUNCTION: MW2ATI 0x10042120
// FUNCTION: MW2S3 0x10029c70
// FUNCTION: MW2POWERVR 0x1003ce50
MechScalar GetAimRange(Player* p_player)
{
	return p_player->m_aimRange.m_value;
}

// FUNCTION: MW2 0x10046375
// FUNCTION: MW2GBL 0x1004905b
// FUNCTION: MW2PREMERCS 0x10080bed
// FUNCTION: MW2MATROX 0x1002d1de
// FUNCTION: MW23DFX 0x100448e0
// FUNCTION: MW2ATI 0x10042130
// FUNCTION: MW2S3 0x10029c80
// FUNCTION: MW2POWERVR 0x1003ce60
void GetMechAimDirection(Player* p_player, MechScalar* p_x, MechScalar* p_y, MechScalar* p_z)
{
	Ray ray;
	MechScalar z;
	MechScalar y;
	MechScalar x;

	BuildAimRay(p_player, &ray);
	SetRayLength(&ray, GetAimRange(p_player));
	GetFiringPosition(p_player, &x, &y, &z);
	*p_x = ray.m_x1 - x;
	*p_y = ray.m_y1 - y;
	*p_z = ray.m_z1 - z;
}

// Builds the player's aim ray, 150000 long.
// FUNCTION: MW2 0x100463e5
// FUNCTION: MW2GBL 0x100490cb
// FUNCTION: MW2PREMERCS 0x10080c5d
// FUNCTION: MW2MATROX 0x1002d253
// FUNCTION: MW23DFX 0x10044950
// FUNCTION: MW2ATI 0x100421a0
// FUNCTION: MW2S3 0x10029cf0
// FUNCTION: MW2POWERVR 0x1003ced0
void BuildAimRay(Player* p_player, Ray* p_ray)
{
	MechScalar z;
	MechScalar y;
	MechScalar x;
	MechScalar dz;
	MechScalar dy;
	MechScalar dx;

	GetEyeAimDirection(p_player, &dx, &dy, &dz);
	GetObjPosition(p_player->m_eyeObj, &x, &y, &z);
	if (g_eyeHeightOffset) {
		y += *g_eyeHeightOffset;
	}

	BuildRayFromDirection(p_ray, x, y, z, dx, dy, dz, 150000);
}

// The direction the player aims in.
// FUNCTION: MW2 0x10046466
// FUNCTION: MW2GBL 0x1004914c
// FUNCTION: MW2PREMERCS 0x10080cde
// FUNCTION: MW2MATROX 0x1002d2d7
// FUNCTION: MW23DFX 0x100449d0
// FUNCTION: MW2ATI 0x10042220
// FUNCTION: MW2S3 0x10029d70
// FUNCTION: MW2POWERVR 0x1003cf50
void GetEyeAimDirection(Player* p_player, MechScalar* p_x, MechScalar* p_y, MechScalar* p_z)
{
	Matrix matrix;
#ifdef MW2_FLOAT_SCALARS
	MechScalar z;
	MechScalar y;
	Mech* mech;
	MechScalar x;
#else
	undefined4 z;
	undefined4 y;
	Mech* mech;
	undefined4 x;
#endif

	*p_x = *p_y = 0;
#ifdef MW2_FLOAT_SCALARS
	*p_z = 1.0f;
#else
	*p_z = 0x10000;
#endif
	GetObjWorldAngles(p_player->m_eyeObj, &x, &y, &z);
	mech = p_player->m_mech;
#ifdef MW2_EYE_AIM_ADDS_PITCH
	if (mech->m_unk0x112 & 4) {
		x += mech->m_torsoPitch.m_value;
	}
	else {
		x = mech->m_torsoPitch.m_value;
	}
#else
	x = mech->m_torsoPitch.m_value;
#endif
	BuildMatrix(&matrix, x, y, z, 0, 0, 0);
	RotatePoint(&matrix, p_x, p_y, p_z);
}

// FUNCTION: MW2 0x100464f3
// FUNCTION: MW2GBL 0x100491d9
// FUNCTION: MW2PREMERCS 0x10080d8e
// FUNCTION: MW2MATROX 0x1002d364
// FUNCTION: MW23DFX 0x10044a50
// FUNCTION: MW2ATI 0x100422a0
// FUNCTION: MW2S3 0x10029df0
// FUNCTION: MW2POWERVR 0x1003cfd0
void GetFiringPosition(Player* p_player, MechScalar* p_x, MechScalar* p_y, MechScalar* p_z)
{
	GetObjPosition(p_player->m_firingObj, p_x, p_y, p_z);
}

// Places p_obj at the player's current hardpoint.
// FUNCTION: MW2 0x10046519
// FUNCTION: MW2GBL 0x100491ff
// FUNCTION: MW2PREMERCS 0x10080db4
// FUNCTION: MW2MATROX 0x1002d38a
// FUNCTION: MW23DFX 0x10044a70
// FUNCTION: MW2ATI 0x100422c0
// FUNCTION: MW2S3 0x10029e10
// FUNCTION: MW2POWERVR 0x1003cff0
void PlaceAtFiringObj(Player* p_player, SceneObject* p_obj)
{
	MechScalar z;
	MechScalar y;
	MechScalar x;

	GetFiringPosition(p_player, &x, &y, &z);
	SetObjRotationMatrix(p_obj, GetObjWorldMatrix(p_player->m_firingObj));
	SetObjPosition(p_obj, x, y, z);
}

// Places a launched shot's object at the player's hardpoint, pointing along the launch
// direction, and sets it moving. p_spread moves it up to 100 units off the line.
// FUNCTION: MW2 0x10046573
// FUNCTION: MW2GBL 0x10049259
// FUNCTION: MW2PREMERCS 0x10080e0e
// FUNCTION: MW2MATROX 0x1002d3e4
// FUNCTION: MW23DFX 0x10044ac0
// FUNCTION: MW2ATI 0x10042310
// FUNCTION: MW2S3 0x10029e60
// FUNCTION: MW2POWERVR 0x1003d040
void SpawnLaunchFx(
	Player* p_player,
	SceneObject* p_obj,
	MechScalar p_dx,
	MechScalar p_dy,
	MechScalar p_dz,
	MechS32 p_spread
#ifdef MW2_WEAPON_FUNCTIONS
	,
	WeaponDef* p_def
#endif
)
{
	MechScalar sideX;
	MechScalar x;
	MechScalar y;
	MechScalar z;
	MechScalar pitch;
	MechScalar yaw;
	MechScalar speed;
	MechScalar sideZ;
	MechScalar offset;
	MechScalar upX;
	MechScalar upY;
	MechScalar upZ;
	MechScalar sideY;

	yaw = FixedAtan2(p_dx, p_dz);
#ifdef MW2_FLOAT_SCALARS
	pitch = FixedAsin(p_dy);
#else
	pitch = FixedAsin(p_dy << 13);
#endif
	SetObjRotation(p_obj, -pitch, yaw, 0, 0);
	GetFiringPosition(p_player, &x, &y, &z);
	if (p_spread) {
		offset = RandomIntBelow(200) - 100;
		sideX = -p_dz;
		sideY = 0;
		sideZ = p_dx;
		NormalizeVectorGuarded(&sideX, &sideY, &sideZ);
#ifdef MW2_FLOAT_SCALARS
		x += offset * sideX;
		z += offset * sideZ;
		upX = p_dy * sideZ;
		upY = p_dz * sideX + p_dx * sideZ;
		upZ = -(p_dy * sideX);
		x += offset * upX;
		y += offset * upY;
		z += offset * upZ;
#else
		x += FixedMul16(offset, sideX);
		z += FixedMul16(offset, sideZ);
		upX = FixedMul16(p_dy, sideZ);
		upY = FixedMul16(sideZ, p_dx) + FixedMul16(p_dz, sideX);
		upZ = -FixedMul16(sideX, p_dy);
		x += FixedMul16(offset, upX);
		y += FixedMul16(offset, upY);
		z += FixedMul16(offset, upZ);
#endif
	}

	SetObjPosition(p_obj, x, y, z);
#ifdef MW2_WEAPON_FUNCTIONS
	if (!(p_def->m_flags & c_weaponNoLaunchMove)) {
		speed = p_obj->m_shape->m_radius * 2;
		p_dx = FixedMul16(p_dx, speed);
		p_dy = FixedMul16(p_dy, speed);
		p_dz = FixedMul16(p_dz, speed);
		MoveObj(p_obj, p_dx, p_dy, p_dz);
	}
#else
	speed = p_obj->m_shape->m_radius * 2;
#ifdef MW2_FLOAT_SCALARS
	p_dx *= speed;
	p_dy *= speed;
	p_dz *= speed;
#else
	p_dx = FixedMul16(p_dx, speed);
	p_dy = FixedMul16(p_dy, speed);
	p_dz = FixedMul16(p_dz, speed);
#endif
	MoveObj(p_obj, p_dx, p_dy, p_dz);
#endif
}
