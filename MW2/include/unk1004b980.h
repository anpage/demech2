#ifndef UNK1004B980_H
#define UNK1004B980_H

#include "decomp.h"
#include "eyepoint.h"
#include "types.h"

struct ScarletOrchid0x4c;

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
	extern MechS32 g_unk0x100ea830;
	extern MechS32 g_unk0x100ea834;
	extern MechS32 g_unk0x100ea840;
	extern MechS32 g_unk0x100ea84c;
	extern MechS32 g_unk0x100ea850;
	extern MechS32 g_unk0x100ea858;
	extern MechS32 g_unk0x100ea860;
	extern MechS32 g_unk0x100ea864;
	extern MechS32 g_unk0x100ea868;
	extern MechS32 g_unk0x100ea86c;
	extern MechS32 g_unk0x100ea870;
	extern MechS32 g_unk0x100ea874;
	extern MechS32 g_unk0x100ea878;
	extern MechS32 g_unk0x100ea87c;
	extern MechS32 g_unk0x100ea880;
	extern MechS32 g_unk0x100ea884;
	extern MechS32 g_unk0x100ea890;
	extern MechS32 g_unk0x100ea894;
	extern MechS32 g_unk0x100ea898;
	extern MechS32 g_unk0x100ea89c;
	extern MechS32 g_unk0x100ea8a0;
	extern MechS32 g_unk0x100ea8a4;
	extern MechS32 g_unk0x100ea8a8;
	extern MechS32 g_unk0x100ea8ac;
	extern MechS32 g_unk0x100ea8b0;
	extern MechS32 g_unk0x100ea8b4;
	extern MechS32 g_unk0x100ea8b8;
	extern MechS32 g_unk0x100ea8bc;
	extern MechS32 g_unk0x100ea8c0;
	extern MechS32 g_unk0x100ea8c4;
	extern MechS32 g_unk0x100ea8c8;
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
	MechS32 FUN_1004c2ef(struct ScarletOrchid0x4c* p_shape);
	MechS32 FUN_1004c565(struct ScarletOrchid0x4c* p_shape);
	MechS32 FUN_1004c779(MechU16* p_flags);
	MechS32 FUN_1004c7a6(undefined4 p_unk0x00);
	void FUN_1004c7cf(undefined4 p_unk0x00, MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // UNK1004B980_H
