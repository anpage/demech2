#ifndef UNK100079D0_H
#define UNK100079D0_H

#include "types.h"

struct Mech;

// The functions and globals of unk100079d0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a1598;

	void FUN_1000832b(MechS32 p_killer, struct Mech* p_mech);
	void FUN_1000899d(MechS32 p_attacker, struct Mech* p_mech, MechU32 p_section);
	void ApplyDamageToMech(MechS32 p_attacker, struct Mech* p_mech, MechS32 p_damage, MechU32 p_section);

#ifdef __cplusplus
}
#endif

#endif // UNK100079D0_H
