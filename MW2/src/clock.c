#include "clock.h"

#include "decomp.h"
#include "msstimer.h"
#include "resource.h"
#include "simmain.h"
#include "types.h"

// The game clock, in ticks of the 181 Hz Miles timer FirstClock registers.

// GLOBAL: MW2 0x100ba548
MechS32 g_currentClock = 0;

// GLOBAL: MW2 0x100ba54c
MechS32 g_unk0x100ba54c = 0;

// GLOBAL: MW2 0x100ba550
MechS32 g_deltaTime = 0;

// GLOBAL: MW2 0x100ba554
MechS32 g_unk0x100ba554 = 0;

// GLOBAL: MW2 0x100ba558
MechS32 g_unk0x100ba558 = 0;

// GLOBAL: MW2 0x100ba55c
HTIMER g_ticksTimer = -1;

// GLOBAL: MW2 0x100ba560
MechS32 g_timeCompressionEnabled = 0;

// GLOBAL: MW2 0x100ba564
MechS32 g_timeExpansionEnabled = 0;

// GLOBAL: MW2 0x100ba568
MechS32 g_framerateLimit = 0;

// GLOBAL: MW2 0x100ba56c
MechS32 g_clockHandle = -1;

// GLOBAL: MW2 0x100ba570
MechS32 g_unk0x100ba570 = -1;

// GLOBAL: MW2 0x100ba574
MechS32 g_unk0x100ba574 = -1;

// GLOBAL: MW2 0x100ba578
MechS32 g_unk0x100ba578 = -1;

// GLOBAL: MW2 0x100ba57c
MechS32 g_previousClock = 0;

// GLOBAL: MW2 0x100ba580
MechS32 g_unk0x100ba580 = 0;

// GLOBAL: MW2 0x100ba584
BOOL g_ticksTimerInitialized = FALSE;

// STUB: MW2 0x1007c930
MechS32 FUN_1007c930(void)
{
	STUB(0x1007c930);
	return 0;
}

// STUB: MW2 0x1007c9e3
MechS32 FUN_1007c9e3(void)
{
	STUB(0x1007c9e3);
	return 0;
}

// STUB: MW2 0x1007ca8e
MechS32 FUN_1007ca8e(void)
{
	STUB(0x1007ca8e);
	return 0;
}

// STUB: MW2 0x1007cbf1
void FUN_1007cbf1(Matrix* p_matrix)
{
	STUB(0x1007cbf1);
}

// FUNCTION: MW2 0x1007cd50
void FirstClock(void)
{
	if (!g_ticksTimerInitialized) {
		AIL_startup();
		g_ticksTimer = AIL_register_timer((AILTIMERCB) GameTickTimerCallback);
		AIL_set_timer_divisor(g_ticksTimer, 6556);
		AIL_start_timer(g_ticksTimer);
		g_unk0x100ba578 = AllocTicks(0x100);
		g_clockHandle = AllocTicks(0x80);
		g_unk0x100ba570 = AllocTicks(0x80);
		ResetTicks(g_unk0x100ba578);
		ResetTicks(g_unk0x100ba570);
		ResetTicks(g_clockHandle);
		g_ticksTimerInitialized = TRUE;
	}

	g_currentClock = g_previousClock = 0;
	g_unk0x100ba54c = 0;
}

// FUNCTION: MW2 0x1007ce2c
void NextClock(void)
{
	if (!g_ticksTimerInitialized) {
		return;
	}

	if (g_unk0x100ba554 == 0) {
		g_currentClock = GetTicks(g_clockHandle);
		if (g_framerateLimit > 0) {
			while (g_currentClock - g_previousClock < g_framerateLimit) {
				g_currentClock = GetTicks(g_clockHandle);
			}
		}

		if (g_unk0x100ba558) {
			g_unk0x100ba580 = g_unk0x100ba554;
			g_unk0x100ba554 = 3;
			DebugLog("NextClock(1): pause_timer(TRUE)");
			PauseTimer(0x80, TRUE);
		}
	}
	else if (g_unk0x100ba554 == 3) {
		g_currentClock += 12;
		if (!g_unk0x100ba558) {
			g_unk0x100ba554 = g_unk0x100ba580;
			DebugLog("NextClock(2): pause_timer(FALSE)");
			PauseTimer(0x80, FALSE);
			SetTicks(g_clockHandle, g_currentClock);
		}
	}

	g_deltaTime = g_currentClock - g_previousClock;
	if (!g_isNetworkGame) {
		if (g_timeCompressionEnabled) {
			g_deltaTime <<= 3;
			g_currentClock += g_deltaTime;
			SetTicks(g_clockHandle, g_currentClock);
		}
		else if (g_timeExpansionEnabled) {
			g_deltaTime >>= 2;
			g_currentClock += g_deltaTime;
			SetTicks(g_clockHandle, g_currentClock);
		}
	}

	if (g_deltaTime <= 0) {
		g_deltaTime = 0;
		g_currentClock = g_previousClock;
	}

	g_previousClock = g_currentClock;
}

// FUNCTION: MW2 0x1007cff5
void StopTimers(void)
{
	if (g_ticksTimerInitialized) {
		FreeTicks(g_unk0x100ba570);
		FreeTicks(g_clockHandle);
		FreeTicks(g_unk0x100ba578);
		AIL_release_timer_handle(g_ticksTimer);
		g_ticksTimer = -1;
		AIL_shutdown();
		g_ticksTimerInitialized = FALSE;
	}
}

// FUNCTION: MW2 0x1007d05d
MechS32 FUN_1007d05d(void)
{
	return GetTicks(g_clockHandle);
}

// FUNCTION: MW2 0x1007d07b
MechS32 FUN_1007d07b(void)
{
	return GetTicks(g_unk0x100ba570);
}

// FUNCTION: MW2 0x1007d099
void FUN_1007d099(void)
{
	ResetTicks(g_unk0x100ba570);
}

// FUNCTION: MW2 0x1007d0b2
void ResetClocks(void)
{
	ResetTicks(g_unk0x100ba578);
	ResetTicks(g_unk0x100ba570);
	ResetTicks(g_clockHandle);
	g_currentClock = g_previousClock = 0;
}

// FUNCTION: MW2 0x1007d0fb
MechS32 FUN_1007d0fb(void)
{
	return GetTicks(g_unk0x100ba578);
}
