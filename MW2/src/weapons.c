#include "weapons.h"

#include "carcfg.h"
#include "clock.h"
#include "collision.h"
#include "config.h"
#include "decomp.h"
#include "effectinfo.h"
#include "environment.h"
#include "eyepoint.h"
#include "fadepal.h"
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

#include <stdio.h>
#include <stdlib.h>

// A mech's weapons: selecting them and their groups, firing volleys, the ammunition and the
// target lock of guided weapons.

// GLOBAL: MW2 0x100a6d34
Shape* g_unk0x100a6d34 = NULL;

// The fire button fires only the selected weapon; cleared, it fires them all.
// GLOBAL: MW2 0x100a6d38
MechS32 g_unk0x100a6d38 = 1;

// The weapons each remote player's weapons message fired, and those the local player fired since
// the last one (network.c).

// GLOBAL: MW2 0x101099c0
MechS32 g_unk0x101099c0[10];

// GLOBAL: MW2 0x101099f0
MechS32 g_unk0x101099f0[10];

// Index order: the original computes the slot address index first, `&p_mech->m_weapons[i]` loads the base
// first here (it flips with the symbols declared ahead of it).
// FUNCTION: MW2 0x10044740
void FUN_10044740(Mech* p_mech)
{
	WeaponSlot* slot;
	MechS32 i;

	for (i = 0; i < 10; i++) {
		slot = &p_mech->m_weapons[i];
		if (slot->m_state == c_weaponFiring || slot->m_state == c_weaponHeld) {
			slot->m_state = c_weaponReady;
		}
	}

	for (i = 0; i < 10; i++) {
		g_unk0x101099f0[i] = 0;
		g_unk0x101099c0[i] = 0;
	}
}

// Stack-slot permutation: dx, dy, dz, first, slot, time, def, i, pan and fired.
// FUNCTION: MW2 0x100447e1
void UpdateWeaponFireState(Mech* p_mech)
{
	MechS32 dy;
	MechS32 first;
	MechS32 dx;
	MechS32 dz;
	WeaponSlot* slot;
	MechS32 time;
	WeaponDef* def;
	MechS32 i;
	MechS32 pan;
	MechS32 fired;

	first = FALSE;
	if (p_mech->m_unk0xa0 != 2) {
		return;
	}

	p_mech->m_unk0xb8 = p_mech->m_selectedWeapon;
	if (p_mech->m_player->m_steering->m_unk0x2c) {
		if (g_unk0x100a6d38) {
			g_unk0x100a6d38 = 0;
			PlayCockpitSound(7, 1);
		}
		else {
			g_unk0x100a6d38 = 1;
			PlayCockpitSound(6, 1);
		}
	}

	if (p_mech->m_unk0x10c & 0x4000) {
		FUN_10045e25(p_mech);
	}
	else if (p_mech->m_selectedWeapon != -1) {
		slot = &p_mech->m_weapons[p_mech->m_selectedWeapon];
		def = &g_weaponDefs[slot->m_type];
		if (!g_unk0x100a6d38 && p_mech->m_player->m_index == g_localPlayerId) {
			p_mech->m_player->m_steering->m_unk0x27 = p_mech->m_player->m_steering->m_unk0x25;
			p_mech->m_player->m_steering->m_unk0x25 = 0;
		}

		if (p_mech->m_player->m_steering->m_unk0x27) {
			if (!(p_mech->m_unk0x10c & 1)) {
				p_mech->m_unk0x10c |= 1;
				FUN_10045cd8();
			}
		}
		else if (p_mech->m_player->m_steering->m_unk0x28) {
			if (!(p_mech->m_unk0x10c & 1)) {
				p_mech->m_unk0x10c |= 1;
				if (FUN_100457d3(p_mech, 0)) {
					FUN_10045cd8();
				}
			}
		}
		else if (p_mech->m_player->m_steering->m_unk0x29) {
			if (!(p_mech->m_unk0x10c & 1)) {
				p_mech->m_unk0x10c |= 1;
				if (FUN_100457d3(p_mech, 1)) {
					FUN_10045cd8();
				}
			}
		}
		else if (p_mech->m_player->m_steering->m_unk0x2a) {
			if (!(p_mech->m_unk0x10c & 1)) {
				p_mech->m_unk0x10c |= 1;
				if (FUN_100457d3(p_mech, 2)) {
					FUN_10045cd8();
				}
			}
		}
		else if (p_mech->m_player->m_steering->m_unk0x25) {
			if (!(p_mech->m_unk0x10c & 1)) {
				p_mech->m_unk0x10c |= 1;
				if (slot->m_state == c_weaponReady) {
					slot->m_state = c_weaponFiring;
					slot->m_volley = def->m_volley;
					slot->m_time = 0;
					FUN_1001632c(slot, p_mech);
					if (def->m_unk0x18 && (p_mech->m_unk0x10c & 0x80)) {
						slot->m_target = p_mech->m_player->m_targetInfo.m_target & 0xff;
						slot->m_targetKind = p_mech->m_player->m_targetInfo.m_target & 0xf00;
					}
					else {
						slot->m_target = slot->m_targetKind = 0;
					}
				}
				else {
					FUN_1007eb23(0x149, 100, 0x40, 5, 0x32);
				}
			}
		}
		else if (p_mech->m_unk0x10c & 1) {
			p_mech->m_unk0x10c &= ~1;
			FUN_10045449(p_mech, 0);
		}
	}

	for (i = 0; i < 10; i++) {
		slot = &p_mech->m_weapons[i];
		if (slot->m_type < 0) {
			continue;
		}

		def = &g_weaponDefs[slot->m_type];
		switch (slot->m_state) {
		case c_weaponRecycling:
			if (g_currentClock - slot->m_time >= def->m_recycle) {
				slot->m_state = c_weaponReady;
				slot->m_time = 0;
			}
			break;
		case c_weaponHeld:
			if (g_currentClock - slot->m_time >= def->m_recycle) {
				if (&p_mech->m_weapons[p_mech->m_selectedWeapon] == slot || p_mech->m_player->m_steering->m_unk0x27 ||
					p_mech->m_player->m_steering->m_unk0x28 || p_mech->m_player->m_steering->m_unk0x29 ||
					p_mech->m_player->m_steering->m_unk0x2a) {
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
					p_mech->m_player->m_unk0x48 = p_mech->m_objects[slot->m_hardpoint];
					fired = SpawnShot(p_mech->m_player, slot);
					if (fired) {
						FUN_1006b1c8(def->m_unk0x08, p_mech->m_player);
						FUN_1006b1c8(def->m_unk0x0c, p_mech->m_player);
						slot->m_volley--;
						time -= def->m_interval;
						slot->m_time = time;
						p_mech->m_deltaHeat += def->m_heat;

						if (first) {
							first = FALSE;
							if (p_mech->m_player->m_index == g_localPlayerId) {
								g_unk0x101099f0[slot->m_index] = 1;
							}

							if (def->m_sound > 0) {
								if (p_mech->m_player->m_index == g_localPlayerId && g_unk0x100a2420) {
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

									FUN_1004c890(def->m_shotType, def->m_sound, pan);
								}
								else {
									dx = g_eyepoint->m_unk0x00 - p_mech->m_player->m_position.m_x;
									dy = g_eyepoint->m_unk0x04 - p_mech->m_player->m_position.m_y;
									dz = g_eyepoint->m_unk0x08 - p_mech->m_player->m_position.m_z;
									FUN_1007ebd1(dx, dy, dz, def->m_sound, g_unk0x100a2420);
								}
							}
						}

						if (slot->m_volley == 0 && def->m_unk0x10 &&
							(p_mech->m_player->m_steering->m_unk0x25 || p_mech->m_player->m_steering->m_unk0x27 ||
							 p_mech->m_player->m_steering->m_unk0x28 || p_mech->m_player->m_steering->m_unk0x29 ||
							 p_mech->m_player->m_steering->m_unk0x2a)) {
							slot->m_time = g_currentClock;
							slot->m_state = c_weaponHeld;
							slot->m_volley = def->m_volley;
							break;
						}
					}
					else {
						p_mech->m_unk0x10c &= ~1;
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

// Launches one shot of p_slot's weapon from the player's current hardpoint. Returns 0 when the
// weapon can't fire again: no hardpoint, or out of ammunition.
// Stack-slot permutation: result, shot, def, found, dx, dy, dz, i and speed. Comparison order: the
// original compares m_binCount with m_binIndex the other way round (either source order compiles alike).
// FUNCTION: MW2 0x1004506c
MechS32 SpawnShot(Player* p_player, WeaponSlot* p_slot)
{
	MechS32 result;
	Shot* shot;
	WeaponDef* def;
	MechS32 found;
	MechS32 dx;
	MechS32 dy;
	MechS32 dz;
	MechS32 i;
	MechS32 speed;

	def = &g_weaponDefs[p_slot->m_type];
	result = 1;
	if (!p_player || !p_player->m_unk0x48) {
		return 0;
	}

	found = FALSE;
	if (!p_slot->m_ammo) {
		return 0;
	}

	if (p_player->m_index == g_localPlayerId) {
		g_carCfg.m_unk0x13++;
	}

	if (g_players[p_player->m_index]->m_team == g_players[g_localPlayerId]->m_team) {
		g_carCfg.m_unk0x2a++;
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

	if (def->m_unk0x14) {
		for (i = 0; !found && i < 0xaf; i++) {
			shot = &g_shots[i];
			if (def->m_shotType == shot->m_type && !shot->m_flags && shot->m_object) {
				found = TRUE;
			}
		}

		if (found) {
			if (p_player->m_index != g_localPlayerId) {
				FUN_10046269(p_player);
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
			shot->m_steering[1] = FixedMul16(-g_unk0x100ba600, def->m_unk0x28 << 16);
			shot->m_steering[0] = shot->m_steering[2] = 0;
			SpawnLaunchFx(p_player, shot->m_object, dx, dy, dz, shot->m_type == 3 || shot->m_type == 4);
			FUN_10001926(shot->m_object);
			FUN_1000199a(shot->m_object);
			FUN_10001cf8(shot->m_object);

			if (p_player->m_index == g_localPlayerId) {
				if (shot->m_type == 3 || shot->m_type == 4) {
					g_unk0x100ad448 = i - 1;
				}
				else {
					g_unk0x100ad448 = -1;
				}
			}
		}
	}

	return result;
}

// Selects the next weapon of the selected one's group; p_wrap moves on to the next group when
// there is none.
// Stack-slot permutation: index, group, slot, found, i and selected.
// FUNCTION: MW2 0x10045449
void FUN_10045449(Mech* p_mech, MechS32 p_wrap)
{
	MechS32 index;
	MechS32 group;
	WeaponSlot* slot;
	MechS32 found;
	MechS32 i;
	MechS32 selected;

	found = FALSE;
	if (p_mech->m_weaponCount <= 0) {
		return;
	}

	selected = p_mech->m_selectedWeapon;
	group = p_mech->m_weapons[p_mech->m_selectedWeapon].m_group;
	index = selected;
	for (i = 1; i < 10; i++) {
		index++;
		index %= 10;
		slot = &p_mech->m_weapons[index];
		if (slot->m_type < 0) {
			continue;
		}

		if (slot->m_unk0x00 != -1 && slot->m_group == group && slot->m_state != c_weaponEmpty && index != selected) {
			found = TRUE;
			p_mech->m_selectedWeapon = index;
			break;
		}
	}

	if (!found && p_wrap) {
		FUN_1004567b(p_mech);
	}
}

// Selects the next weapon that still has ammunition.
// Stack-slot permutation: done and tries.
// FUNCTION: MW2 0x10045567
void FUN_10045567(Mech* p_mech)
{
	MechS32 done;
	MechS32 tries;

	done = FALSE;
	tries = 0;
	if (p_mech->m_weaponCount <= 0) {
		return;
	}

	while (!done) {
		p_mech->m_selectedWeapon++;
		p_mech->m_selectedWeapon %= 10;
		if (p_mech->m_weapons[p_mech->m_selectedWeapon].m_type < 0) {
			continue;
		}

		if (g_unk0x100a2bf8 && p_mech->m_player->m_index == g_localPlayerId) {
			return;
		}

		if (p_mech->m_weapons[p_mech->m_selectedWeapon].m_unk0x00 == -1) {
			p_mech->m_selectedWeapon = 0;
		}

		if (p_mech->m_weapons[p_mech->m_selectedWeapon].m_state != c_weaponEmpty || ++tries == 10) {
			done = TRUE;
		}
	}
}

// Selects the first weapon of the next group that has one.
// Index order: the original loads p_mech->m_weapons before scaling i; stack-slot permutation: group, i,
// found, selected and current.
// FUNCTION: MW2 0x1004567b
void FUN_1004567b(Mech* p_mech)
{
	MechS32 group;
	MechS32 i;
	MechS32 found;
	MechS32 selected;
	MechS32 current;

	i = 0;
	found = FALSE;
	selected = p_mech->m_selectedWeapon;
	if (p_mech->m_weaponCount <= 0) {
		return;
	}

	current = p_mech->m_weapons[p_mech->m_selectedWeapon].m_group;
	group = current + 1;
	group %= 10;
	while (group != current && !found) {
		for (i = 0; i < 10 && !found; i++) {
			if (p_mech->m_weapons[i].m_type < 0) {
				continue;
			}

			if (p_mech->m_weapons[i].m_unk0x00 != -1 && p_mech->m_weapons[i].m_group == group &&
				p_mech->m_weapons[i].m_state != c_weaponEmpty) {
				found = TRUE;
				p_mech->m_selectedWeapon = selected = i;
				break;
			}
		}

		group++;
		group %= 10;
	}
}

// Selects the first weapon of p_group. Returns 1 if the group has one.
// Index order: the original loads p_mech->m_weapons before scaling i; stack-slot permutation: i, found,
// selected and current.
// FUNCTION: MW2 0x100457d3
MechS32 FUN_100457d3(Mech* p_mech, MechS32 p_group)
{
	MechS32 i;
	MechS32 found;
	MechS32 selected;
	MechS32 current;

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

	for (i = 0; i < 10 && !found; i++) {
		if (p_mech->m_weapons[i].m_type < 0) {
			continue;
		}

		if (p_mech->m_weapons[i].m_unk0x00 != -1 && p_mech->m_weapons[i].m_group == p_group &&
			p_mech->m_weapons[i].m_state != c_weaponEmpty) {
			found = TRUE;
			p_mech->m_selectedWeapon = selected = i;
			break;
		}
	}

	return found;
}

// Returns 1 if the selected weapon is ready, 0 if not, -1 without one.
// FUNCTION: MW2 0x10045919
MechS32 FUN_10045919(Mech* p_mech)
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
MechS32 FUN_1004597b(Mech* p_mech)
{
	MechChar text[40];
	WeaponSlot* slot;

	if (p_mech->m_selectedWeapon == -1) {
		return 0;
	}

	slot = &p_mech->m_weapons[p_mech->m_selectedWeapon];
	if (g_weaponDefs[slot->m_type].m_volley > 0 && slot->m_ammo > 0) {
		FUN_1007eb23(0xb3, 100, 0x40, 5, 0x50);
		sprintf(text, "Ammo for current weapon jettisonned.");
		ShowInGameMessage(text, 1, 0x16a, 0x32);
		FUN_1007eb23(0xb3, 100, 0x40, 5, 0x32);
		slot->m_ammo = 0;
		slot->m_state = c_weaponEmpty;
		return 1;
	}

	return 0;
}

// Loads the launch sounds of the weapons and the sounds of the effects.
// Stack-slot permutation: i and def.
// FUNCTION: MW2 0x10045a5b
void FUN_10045a5b(void)
{
	MechS32 i;
	WeaponDef* def;

	for (i = 0; i < 30; i++) {
		def = &g_weaponDefs[i];
		if (def->m_sound > 0) {
			FUN_10050862(def->m_sound, g_resourceTypeTags[c_resTagSnds]);
		}
	}

	for (i = 0; i < 0x20; i++) {
		if (g_effectInfo[i].m_sound > 0) {
			FUN_10050862(g_effectInfo[i].m_sound, g_resourceTypeTags[c_resTagSnds]);
		}
	}
}

// FUNCTION: MW2 0x10045b14
void FUN_10045b14(Mech* p_mech, MechS32 p_index, MechS32 p_group)
{
	WeaponSlot* slot;

	if (p_group >= 0 && p_group < 3) {
		slot = &p_mech->m_weapons[p_index];
		slot->m_group = p_group;
	}
}

// Moves the local player's selected weapon to p_group.
// FUNCTION: MW2 0x10045b56
void FUN_10045b56(MechS32 p_group)
{
	Mech* mech;
	WeaponSlot* slot;

	mech = g_players[g_localPlayerId]->m_mech;
	slot = &mech->m_weapons[mech->m_selectedWeapon];
	slot->m_group = p_group;
}

// FUNCTION: MW2 0x10045b9c
void FUN_10045b9c(void)
{
	Mech* mech;

	mech = g_players[g_localPlayerId]->m_mech;
	FUN_1004567b(mech);
}

// Fires the weapons g_unk0x101099c0 lists from p_mech.
// Index order: `&p_mech->m_weapons[i]` loads the base first in the original; stack-slot permutation:
// slot, def and i.
// FUNCTION: MW2 0x10045bc8
void FUN_10045bc8(Mech* p_mech)
{
	WeaponSlot* slot;
	WeaponDef* def;
	MechS32 i;

	for (i = 0; i < 10; i++) {
		if (g_unk0x101099c0[i]) {
			g_unk0x101099c0[i] = 0;
			slot = &p_mech->m_weapons[i];
			def = &g_weaponDefs[slot->m_type];
			if (slot->m_state != c_weaponEmpty) {
				slot->m_state = c_weaponFiring;
				slot->m_volley = def->m_volley;
				slot->m_time = 0;
				if (def->m_unk0x18 && (p_mech->m_unk0x10c & 0x80)) {
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

// Fires every ready weapon of the local player's selected group.
// Stack-slot permutation: selected, first, def and slot.
// FUNCTION: MW2 0x10045cd8
void FUN_10045cd8(void)
{
	Mech* mech;
	MechS32 selected;
	MechS32 first;
	WeaponDef* def;
	WeaponSlot* slot;

	mech = g_players[g_localPlayerId]->m_mech;
	selected = mech->m_selectedWeapon;
	FUN_10045449(mech, 0);
	first = mech->m_selectedWeapon;
	do {
		slot = &mech->m_weapons[mech->m_selectedWeapon];
		def = &g_weaponDefs[slot->m_type];
		if (slot->m_state == c_weaponReady) {
			slot->m_state = c_weaponFiring;
			slot->m_volley = def->m_volley;
			slot->m_time = 0;
			FUN_1001632c(slot, mech);
			if (def->m_unk0x18 && (mech->m_unk0x10c & 0x80)) {
				slot->m_target = mech->m_player->m_targetInfo.m_target & 0xff;
				slot->m_targetKind = mech->m_player->m_targetInfo.m_target & 0xf00;
			}
			else {
				slot->m_target = slot->m_targetKind = 0;
			}
		}

		FUN_10045449(mech, 0);
	} while (mech->m_selectedWeapon != first);

	mech->m_selectedWeapon = selected;
}

// Moves the next weapon into the selected one's group.
// FUNCTION: MW2 0x10045e25
void FUN_10045e25(Mech* p_mech)
{
	MechS32 group;
	MechS32 next;

	p_mech->m_unk0x10c ^= 0x4000;
	next = p_mech->m_selectedWeapon;
	group = p_mech->m_weapons[next].m_group;
	next++;
	if (p_mech->m_weapons[next].m_unk0x00 == -1) {
		next = 0;
	}

	FUN_10045b14(p_mech, next, group);
}

// Updates the target lock of the selected weapon: with the target between its two ranges and
// within 16 degrees of the aim, it locks on after 0x16a ticks.
// Stack-slot permutation: dx, dy, dz, twist, pitch, yaw, bearing, inRange, def and heading.
// FUNCTION: MW2 0x10045eac
void FUN_10045eac(Mech* p_mech)
{
	MechS32 dz;
	MechS32 twist;
	MechS32 pitch;
	MechS32 yaw;
	MechS32 bearing;
	MechS32 inRange;
	MechS32 dx;
	WeaponDef* def;
	MechS32 dy;
	MechS32 heading;

	p_mech->m_unk0x10c &= ~0x80;
	def = &g_weaponDefs[p_mech->m_weapons[p_mech->m_selectedWeapon].m_type];
	if (p_mech->m_selectedWeapon == -1 || !p_mech->m_player->m_targetInfo.m_target ||
		(p_mech->m_player->m_targetInfo.m_target & 0x1000) || (p_mech->m_player->m_targetInfo.m_target & 0x100) ||
		!def->m_unk0x18) {
		p_mech->m_unk0x10c &= 0x7fff;
		p_mech->m_unk0x10c &= 0xffbf;
	}
	else {
		dx = p_mech->m_player->m_position.m_x - p_mech->m_player->m_targetInfo.m_position.m_x;
		dy = p_mech->m_player->m_position.m_y - p_mech->m_player->m_targetInfo.m_position.m_y;
		dz = p_mech->m_player->m_position.m_z - p_mech->m_player->m_targetInfo.m_position.m_z;
		if (FUN_10004ec0(dx, dy, dz, def->m_unk0x3c) || !FUN_10004ec0(dx, dy, dz, def->m_unk0x40)) {
			inRange = FALSE;
			yaw = 0x100001;
			pitch = 0x100001;
			p_mech->m_unk0x10c &= 0x7fff;
		}
		else {
			inRange = TRUE;
			heading = (p_mech->m_player->m_heading % 0x1680000 + 0x1680000) % 0x1680000;
			twist = p_mech->m_player->m_unk0x6c % 0x1680000;
			bearing = p_mech->m_player->m_targetInfo.m_heading - heading;
			if (bearing > 0xb40000) {
				bearing -= 0x1680000;
			}
			else if (bearing < -0xb40000) {
				bearing += 0x1680000;
			}

			yaw = bearing - twist;
			if (yaw > 0xb40000) {
				yaw -= 0x1680000;
			}
			else if (yaw < -0xb40000) {
				yaw += 0x1680000;
			}

			pitch = (p_mech->m_player->m_targetInfo.m_unk0x18 + p_mech->m_unk0x14.m_value) % 0x1680000;
		}

		if (inRange && abs(yaw) < 0x100000 && abs(pitch) < 0x100000) {
			p_mech->m_unk0x10c |= 0x8000;
			if (!(p_mech->m_unk0x10c & 0x40)) {
				p_mech->m_unk0x10c |= 0x40;
				p_mech->m_unk0x8c = 0x16a;
			}
			else if (p_mech->m_unk0x8c <= 0) {
				p_mech->m_unk0x8c = 0;
				p_mech->m_unk0x10c |= 0x80;
			}
			else {
				p_mech->m_unk0x8c -= g_deltaTime;
			}
		}
		else {
			p_mech->m_unk0x10c &= 0x7fff;
			if (p_mech->m_unk0x10c & 0x40) {
				p_mech->m_unk0x8c += g_deltaTime;
				if (p_mech->m_unk0x8c >= 0x16a) {
					p_mech->m_unk0x10c &= ~0x40;
				}
			}
		}
	}
}

// Casts the player's aim ray and returns the shape it hits, if any. A hit on a mech or a
// building sets the distance the weapons converge at.
// Stack-slot permutation: length, hit, flags and collided.
// FUNCTION: MW2 0x10046269
Shape* FUN_10046269(Player* p_player)
{
	Ray ray;
	MechS32 length;
	Shape* hit;
	MechU16 flags;
	MechS32 collided;

	hit = NULL;
	UpdateRamp(&p_player->m_unk0xa8);
	FUN_100463e5(p_player, &ray);
	collided = TestSegmentCollision(&ray, &hit, p_player->m_index);
	if (collided && hit) {
		flags = hit->m_unk0x02;
		if ((flags & 0x100) || (flags & 0x200)) {
			length = GetRayLength(&ray);
			p_player->m_unk0xa8.m_value = length > 2000 ? length : 2000;
			if (p_player->m_index == g_localPlayerId) {
				g_unk0x100a6d34 = hit;
			}
		}
	}

	p_player->m_aimRange.m_target = p_player->m_unk0xa8.m_value;
	UpdateRamp(&p_player->m_aimRange);

	return hit;
}

// FUNCTION: MW2 0x1004635c
MechS32 FUN_1004635c(Player* p_player)
{
	return p_player->m_aimRange.m_value;
}

// FUNCTION: MW2 0x10046375
void GetMechAimDirection(Player* p_player, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	Ray ray;
	MechS32 z;
	MechS32 y;
	MechS32 x;

	FUN_100463e5(p_player, &ray);
	SetRayLength(&ray, FUN_1004635c(p_player));
	FUN_100464f3(p_player, &x, &y, &z);
	*p_x = ray.m_x1 - x;
	*p_y = ray.m_y1 - y;
	*p_z = ray.m_z1 - z;
}

// Builds the player's aim ray, 150000 long.
// Stack-slot permutation: x, y, z, dx, dy and dz.
// FUNCTION: MW2 0x100463e5
void FUN_100463e5(Player* p_player, Ray* p_ray)
{
	MechS32 z;
	MechS32 y;
	MechS32 x;
	MechS32 dz;
	MechS32 dy;
	MechS32 dx;

	FUN_10046466(p_player, &dx, &dy, &dz);
	GetObjPosition(p_player->m_unk0x44, &x, &y, &z);
	if (g_unk0x100a2434) {
		y += *g_unk0x100a2434;
	}

	BuildRayFromDirection(p_ray, x, y, z, dx, dy, dz, 150000);
}

// The direction the player aims in.
// FUNCTION: MW2 0x10046466
void FUN_10046466(Player* p_player, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	Matrix matrix;
	undefined4 z;
	undefined4 y;
	Mech* mech;
	undefined4 x;

	*p_x = *p_y = 0;
	*p_z = 0x10000;
	GetObjWorldPos(p_player->m_unk0x44, &x, &y, &z);
	mech = p_player->m_mech;
	x = mech->m_unk0x14.m_value;
	FUN_1000e2b9(&matrix, x, y, z, 0, 0, 0);
	FUN_1000d708(&matrix, p_x, p_y, p_z);
}

// FUNCTION: MW2 0x100464f3
void FUN_100464f3(Player* p_player, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	GetObjPosition(p_player->m_unk0x48, p_x, p_y, p_z);
}

// Places p_obj at the player's current hardpoint.
// FUNCTION: MW2 0x10046519
void FUN_10046519(Player* p_player, SceneObject* p_obj)
{
	MechS32 z;
	MechS32 y;
	MechS32 x;

	FUN_100464f3(p_player, &x, &y, &z);
	FUN_10001722(p_obj, FUN_10001e01(p_player->m_unk0x48));
	SetObjPosition(p_obj, x, y, z);
}

// Places a launched shot's object at the player's hardpoint, pointing along the launch
// direction, and sets it moving. p_spread moves it up to 100 units off the line.
// Commutative operand order: the original calls FixedMul16(p_dz, sideX) first for upY. Stack-slot
// permutation of the locals.
// FUNCTION: MW2 0x10046573
void SpawnLaunchFx(Player* p_player, SceneObject* p_obj, MechS32 p_dx, MechS32 p_dy, MechS32 p_dz, MechS32 p_spread)
{
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 pitch;
	MechS32 yaw;
	MechS32 speed;
	MechS32 sideZ;
	MechS32 offset;
	MechS32 upX;
	MechS32 upY;
	MechS32 upZ;
	MechS32 sideX;
	MechS32 sideY;

	yaw = FUN_100698de(p_dx, p_dz);
	pitch = FUN_1006975b(p_dy << 13);
	SetObjRotation(p_obj, -pitch, yaw, 0, 0);
	FUN_100464f3(p_player, &x, &y, &z);
	if (p_spread) {
		offset = RandomIntBelow(200) - 100;
		sideX = -p_dz;
		sideY = 0;
		sideZ = p_dx;
		NormalizeVectorGuarded(&sideX, &sideY, &sideZ);
		x += FixedMul16(offset, sideX);
		z += FixedMul16(offset, sideZ);
		upX = FixedMul16(p_dy, sideZ);
		upY = FixedMul16(p_dz, sideX) + FixedMul16(sideZ, p_dx);
		upZ = -FixedMul16(sideX, p_dy);
		x += FixedMul16(offset, upX);
		y += FixedMul16(offset, upY);
		z += FixedMul16(offset, upZ);
	}

	SetObjPosition(p_obj, x, y, z);
	speed = p_obj->m_unk0x6c->m_unk0x40 * 2;
	p_dx = FixedMul16(p_dx, speed);
	p_dy = FixedMul16(p_dy, speed);
	p_dz = FixedMul16(p_dz, speed);
	FUN_10001667(p_obj, p_dx, p_dy, p_dz);
}
