#ifndef UNK1005D6D0_H
#define UNK1005D6D0_H

struct MechSection;
struct MekHeader;
struct MekWeapon;
struct Mech;

#include "decomp.h"
#include "types.h"

// The functions and globals of unk1005d6d0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechU16 g_unk0x100aa730[30];

	MechS32 FUN_1005d6d0(struct Mech* p_mech, MechChar* p_name, MechS32 p_unk0x08, MechChar* p_unk0x0c);
	MechU16 FUN_1005e534(struct MekHeader* p_header, struct MechSection* p_sections, struct MekWeapon* p_weapons);

#ifdef __cplusplus
}
#endif

#endif // UNK1005D6D0_H
