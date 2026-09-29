#ifndef CLOCK_H
#define CLOCK_H

#include "types.h"

#include <windows.h>

// The functions and globals of clock.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_currentClock;
	extern MechS32 g_unk0x100ba54c;
	extern MechS32 g_deltaTime;
	extern BOOL g_ticksTimerInitialized;

	MechS32 FUN_1007c930(void);
	MechS32 FUN_1007c9e3(void);
	MechS32 FUN_1007ca8e(void);
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
