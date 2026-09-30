#ifndef CLOCK_H
#define CLOCK_H

#include "transform.h"
#include "types.h"

#include <windows.h>

// The functions and globals of clock.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_currentClock;
	extern MechS32 g_unk0x100ba54c;
	extern MechS32 g_unk0x100ba554;
	extern MechS32 g_deltaTime;
	extern BOOL g_ticksTimerInitialized;
	extern MechS16* g_sqrtTable;
	extern MechS32 g_sinTable[0x102];
	extern MechS16 g_sqrtTableData[0x400];
	extern MechS32 g_atanTable[0x102];

	MechS32 FUN_1007c930(void);
	MechS32 FUN_1007c9e3(void);
	MechS32 FUN_1007ca8e(void);
	MechS32 FUN_1007caf7(MechS32 p_x, MechS32 p_y);
	void FUN_1007cb3d(Matrix* p_matrix, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void FUN_1007cbf1(Matrix* p_matrix);
	void FUN_1007ccc2(MechS32 p_length, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FirstClock(void);
	void NextClock(void);
	void StopTimers(void);
	MechS32 FUN_1007d05d(void);
	MechS32 FUN_1007d07b(void);
	void FUN_1007d099(void);
	void ResetClocks(void);
	MechS32 FUN_1007d0fb(void);

#ifdef __cplusplus
}
#endif

#endif // CLOCK_H
