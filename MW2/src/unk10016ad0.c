#include "unk10016ad0.h"

#include "ai.h"
#include "clock.h"
#include "config.h"
#include "decomp.h"
#include "environment.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "fixeddiv.h"
#include "gpanim.h"
#include "mech.h"
#include "mechsection.h"
#include "network.h"
#include "object.h"
#include "players.h"
#include "playersteering.h"
#include "ramp.h"
#include "resource.h"
#include "silvertern.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "staticmem.h"
#include "timedoverlays.h"
#include "types.h"
#include "unk100563d0.h"
#include "unk1007fbe0.h"
#include "weapons.h"
#include "weaponslot.h"

#include <stdio.h>

DECOMP_SIZE_ASSERT(Mech, 0x10e)

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
MechS32 FUN_100197ca(undefined4 p_unk0x00, Player* p_player)
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
