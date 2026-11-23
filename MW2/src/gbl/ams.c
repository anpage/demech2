/* GBL's anti-missile system (the AMS weapon, its TOGGLE_AMS game key): an object of four functions
   GBL links before refreshmode.c. */
#include "gbl/ams.h"

#include "decomp.h"
#include "types.h"
#include "weapondef.h"
#include "weaponslot.h"

// The AMS's frame function (WeaponDef::m_frame).
// STUB: MW2GBL 0x1002e997
void FUN_1002e997(struct Player* p_player, WeaponDef* p_def, WeaponSlot* p_slot)
{
	STUB(0x1002e997);
}

// The AMS's launch function (WeaponDef::m_launch).
// STUB: MW2GBL 0x1002eb5f
MechS32 FUN_1002eb5f(struct Player* p_player, WeaponDef* p_def, WeaponSlot* p_slot)
{
	STUB(0x1002eb5f);
	return 0;
}
