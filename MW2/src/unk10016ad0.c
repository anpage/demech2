#include "unk10016ad0.h"

#include "config.h"
#include "decomp.h"
#include "mech.h"
#include "mechsection.h"
#include "players.h"
#include "silvertern.h"
#include "simmain.h"
#include "staticmem.h"
#include "types.h"
#include "unk100563d0.h"
#include "weaponslot.h"

DECOMP_SIZE_ASSERT(Mech, 0x10e)

// STUB: MW2 0x10016ad0
void FUN_10016ad0(struct Player* p_player)
{
	STUB(0x10016ad0);
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
