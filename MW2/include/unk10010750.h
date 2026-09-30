#ifndef UNK10010750_H
#define UNK10010750_H

#include "decomp.h"
#include "types.h"

// The functions and globals of unk10010750.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10010750(MechU32 p_flags, MechS32 p_count, MechU32* p_points, MechS32 p_unk0x0c);
	MechS32 FUN_100107de(
		undefined4 p_unk0x00,
		MechS32 p_unk0x04,
		undefined4* p_unk0x08,
		MechS32 p_unk0x0c,
		MechS32 p_unk0x10
	);
	MechS32 FUN_10010a7f(undefined4 p_unk0x00, MechS32 p_unk0x04, undefined4* p_unk0x08);

#ifdef __cplusplus
}
#endif

#endif // UNK10010750_H
