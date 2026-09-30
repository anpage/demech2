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
	extern MechS32 g_unk0x100ea820;
	extern MechS32 g_unk0x100ea824;
	extern MechS32 g_unk0x100ea828;
	extern MechS32 g_unk0x100ea82c;
	extern MechS32 g_unk0x100ea834;
	extern MechS32 g_unk0x100ea858;
	extern MechS32 g_unk0x100ea860;
	extern MechS32 g_unk0x100ea8d0;

	void FUN_1004b980(Eyepoint* p_eyepoint);
	void FUN_1004bc2e(Eyepoint* p_eyepoint);
	void FUN_1004bf61(Eyepoint* p_eyepoint, MechS32 p_value);
	void FUN_1004bf8a(Eyepoint* p_eyepoint, MechS32 p_value);
	void FUN_1004bfe8(Eyepoint* p_eyepoint);
	void FUN_1004c05c(Eyepoint* p_eyepoint);
	void FUN_1004c093(Eyepoint* p_eyepoint, Matrix* p_matrix);
	void FUN_1004c0d8(Eyepoint* p_eyepoint, Matrix* p_matrix);
	MechS32 FUN_1004c11d(MechS32* p_x, MechS32* p_y, MechS32* p_z);
	MechS32 FUN_1004c779(MechU16* p_flags);
	MechS32 FUN_1004c7a6(undefined4 p_unk0x00);
	void FUN_1004c7cf(undefined4 p_unk0x00, MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // UNK1004B980_H
