/* A mech class: its reset, update and allocation functions. */
#include "unk10059fc0.h"

#include "decomp.h"
#include "mech.h"
#include "players.h"
#include "staticmem.h"
#include "types.h"
#include "unk10016ad0.h"
#include "unk100563d0.h"
#include "weaponslot.h"

// STUB: MW2 0x10059fc0
void FUN_10059fc0(Player* p_player)
{
	STUB(0x10059fc0);
}

// STUB: MW2 0x1005a203
void FUN_1005a203(Mech* p_mech)
{
	STUB(0x1005a203);
}

// STUB: MW2 0x1005a2ea
void FUN_1005a2ea(Mech* p_mech)
{
	STUB(0x1005a2ea);
}

// FUNCTION: MW2 0x1005a61d
void FUN_1005a61d(Mech* p_mech)
{
	if (!p_mech) {
		return;
	}
}

// Allocates p_player's mech, its weapons and sections, and sets them up.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1005a637
MechS32 FUN_1005a637(undefined4 p_unk0x00, Player* p_player)
{
	void* buffer = NULL;
	MechSection* sections = NULL;
	WeaponSlot* weapons = NULL;
	void* extra = NULL;
	Mech* mech = NULL;
	WeaponSlot* slot;
	MechS32 i;
	MechS32 size;

	p_player->m_mech = NULL;
	size = FUN_10019a0a();
	buffer = StaticPoolAlloc(size, g_unk0x100a942c);
	if (!buffer) {
		return FALSE;
	}

	mech = buffer;
	weapons = (WeaponSlot*) (mech + 1);
	sections = (MechSection*) (weapons + 10);
	extra = sections + 8;
	mech->m_weapons = weapons;
	mech->m_sections = sections;
	mech->m_unk0x5c = extra;
	mech->m_player = p_player;
	for (i = 0; i < 8; i++) {
		mech->m_objects[i] = NULL;
	}

	p_player->m_mech = mech;
	p_player->m_unk0x24 = 0x10e;
	slot = mech->m_weapons;
	for (i = 0; i < 10; i++) {
		slot->m_unk0x00 = -1;
		slot->m_type = -1;
		slot->m_state = c_weaponEmpty;
		slot->m_time = 0;
		slot->m_unk0x14 = 0;
		slot->m_ammo = -1;
		slot++;
	}

	return TRUE;
}
