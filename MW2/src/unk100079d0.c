#include "unk100079d0.h"

#include "decomp.h"
#include "mech.h"
#include "types.h"

// GLOBAL: MW2 0x100a1598
MechS32 g_unk0x100a1598 = 4;

// Destroys p_mech, on behalf of player p_killer (-2: its player left the game).
// STUB: MW2 0x1000832b
void FUN_1000832b(MechS32 p_killer, Mech* p_mech)
{
	STUB(0x1000832b);
}

// Destroys section p_section of p_mech, on behalf of player p_attacker.
// STUB: MW2 0x1000899d
void FUN_1000899d(MechS32 p_attacker, Mech* p_mech, MechU32 p_section)
{
	STUB(0x1000899d);
}

// Deals p_damage (16.16) to section p_section of the mech, on behalf of player p_attacker.
// STUB: MW2 0x1000991b
void ApplyDamageToMech(MechS32 p_attacker, Mech* p_mech, MechS32 p_damage, MechU32 p_section)
{
	STUB(0x1000991b);
}
