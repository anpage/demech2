#ifndef MECHDAMAGE_H
#define MECHDAMAGE_H

#include "decomp.h"
#include "types.h"

struct Mech;

// The functions and globals of mechdamage.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a1590;
	extern MechS32 g_unk0x100a1594;
	extern MechS32 g_unk0x100a1598;
	extern MechS32 g_unk0x100a15a0;

	void FUN_100079d0(struct Mech* p_mech);
	void FUN_10007cb5(struct Mech* p_mech);
	void CalculateHeat(struct Mech* p_mech);
	void FUN_10007d06(MechS32 p_killer, struct Mech* p_mech);
	void FUN_1000832b(MechS32 p_killer, struct Mech* p_mech);
	void FUN_10008938(MechS32 p_attacker, struct Mech* p_mech, MechU32 p_section);
	void FUN_1000899d(MechS32 p_attacker, struct Mech* p_mech, MechU32 p_section);
	void FUN_10008c0f(MechS32 p_attacker, struct Mech* p_mech, MechU32 p_section, MechS32 p_slot, MechS32 p_recursing);
	void ApplyDamageToMech(MechS32 p_attacker, struct Mech* p_mech, MechS32 p_damage, MechS32 p_section);
	void EjectPlayer(struct Mech* p_mech, MechS32 p_eject);
	void FUN_10009dd2(void);

#ifdef __cplusplus
}
#endif

#endif // MECHDAMAGE_H
