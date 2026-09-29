#ifndef UNK1004B980_H
#define UNK1004B980_H

#include "decomp.h"
#include "eyepoint.h"
#include "types.h"

// The functions and globals of unk1004b980.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a712c;

	void FUN_1004b980(Eyepoint* p_eyepoint);
	void FUN_1004bc2e(Eyepoint* p_eyepoint);
	void FUN_1004bfe8(Eyepoint* p_eyepoint);
	MechS32 FUN_1004c7a6(undefined4 p_unk0x00);
	void FUN_1004c7cf(undefined4 p_unk0x00, MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // UNK1004B980_H
