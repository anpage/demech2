#include "clock.h"

#include "debugprint.h"
#include "decomp.h"
#include "mss.h"
#include "network.h"
#include "ticks.h"
#include "transform.h"
#include "types.h"

#include <math.h>

// The game clock, in ticks of the 181 Hz Miles timer FirstClock registers.

// GLOBAL: MW2 0x100ba548
// GLOBAL: MW2MATROX 0x100a4e50
MechS32 g_currentClock = 0;

// Ticks that run on while the game clock is paused; the network times its messages by them.
// GLOBAL: MW2 0x100ba54c
// GLOBAL: MW2MATROX 0x100a4e54
MechS32 g_realClock = 0;

// GLOBAL: MW2 0x100ba550
// GLOBAL: MW2MATROX 0x100a4e58
MechS32 g_deltaTime = 0;

// 0 the game clock runs on its own ticks, 2 a slave's follows the master's, 3 paused.
// GLOBAL: MW2 0x100ba554
// GLOBAL: MW2MATROX 0x100a4e5c
MechS32 g_clockMode = 0;

// GLOBAL: MW2 0x100ba558
// GLOBAL: MW2MATROX 0x100a4e60
MechS32 g_clockPaused = 0;

// GLOBAL: MW2 0x100ba55c
// GLOBAL: MW2MATROX 0x100a4e64
HTIMER g_ticksTimer = -1;

// GLOBAL: MW2 0x100ba560
// GLOBAL: MW2MATROX 0x100a4e68
MechS32 g_timeCompressionEnabled = 0;

// GLOBAL: MW2 0x100ba564
// GLOBAL: MW2MATROX 0x100a4e6c
MechS32 g_timeExpansionEnabled = 0;

// GLOBAL: MW2 0x100ba568
// GLOBAL: MW2MATROX 0x100a4e70
MechS32 g_framerateLimit = 0;

// GLOBAL: MW2 0x100ba56c
// GLOBAL: MW2MATROX 0x100a4e74
MechS32 g_clockHandle = -1;

// The ticks since a slave's last UpdateNetwork: they advance its game clock when the master's
// clock didn't arrive.
// GLOBAL: MW2 0x100ba570
// GLOBAL: MW2MATROX 0x100a4e78
MechS32 g_syncTicksHandle = -1;

// GLOBAL: MW2 0x100ba574
MechS32 g_unk0x100ba574 = -1;

// GLOBAL: MW2 0x100ba578
// GLOBAL: MW2MATROX 0x100a4e80
MechS32 g_realClockHandle = -1;

// GLOBAL: MW2 0x100ba57c
// GLOBAL: MW2MATROX 0x100a4e84
MechS32 g_previousClock = 0;

// GLOBAL: MW2 0x100ba580
// GLOBAL: MW2MATROX 0x100a4e88
MechS32 g_clockModeBeforePause = 0;

// GLOBAL: MW2 0x100ba584
// GLOBAL: MW2MATROX 0x100a4e8c
BOOL g_ticksTimerInitialized = FALSE;

// GLOBAL: MW2 0x100bfd54
MechS16* g_sqrtTable;

// GLOBAL: MW2 0x100bfd60
// GLOBAL: MW2MATROX 0x1012d7d0
MechScalar g_slopeSines[800];

// GLOBAL: MW2 0x100c09e0
// GLOBAL: MW2MATROX 0x1012cb50
MechScalar g_slopeCosines[800];

// GLOBAL: MW2 0x100c1660
// GLOBAL: MW2MATROX 0x1012e450
MechScalar g_sinTable[TRIG_TABLE_SIZE];

// GLOBAL: MW2 0x100c1a80
MechS16 g_sqrtTableData[0x400];

// GLOBAL: MW2 0x100c2290
// GLOBAL: MW2MATROX 0x1012ec50
MechScalar g_atanTable[TRIG_TABLE_SIZE];

// The Matrox edition's tables, Hypot2D and trigonometry are its own float code, in matrox/floatmath.c.
#ifndef MW2_MATROX
// A quarter wave of sines (2.29 fixed point, 1024 steps to the circle) and the arctangents of
// 0 to 1 in 256 steps (16.16 degrees), each padded with two copies of its last value.
// FUNCTION: MW2 0x1007c930
MechS32 InitSinAtanTables(void)
{
	MechS32 i;

	for (i = 0; i < 0x100; i++) {
		g_sinTable[i] = (MechS32) (sin(i * (3.14159265 / 512)) * 536870912.0);
		g_atanTable[i] = (MechS32) (atan(i / 256.0) * 3754936.210460003);
	}

	g_sinTable[0x100] = g_sinTable[0x101] = 0x20000000;
	g_atanTable[0x100] = g_atanTable[0x101] = 0x2d0000;
	return TRUE;
}

// The cosines and sines of the angles whose tangents are 0 to 50 in steps of 1/16 (2.29).
// Stack-slot permutation: i and cosine.
// FUNCTION: MW2 0x1007c9e3
MechS32 InitSlopeTables(void)
{
	MechS32 i;
	MechDouble cosine;

	for (i = 0; i < 800; i++) {
		g_slopeCosines[i] = (MechS32) ((cosine = 1.0 / sqrt(i / 16.0 * (i / 16.0) + 1.0)) * 536870912.0);
		g_slopeSines[i] = (MechS32) (i / 16.0 * cosine * 536870912.0);
	}

	return TRUE;
}

#endif

// The square roots of 0 to 1023 (6.10 fixed point).
// FUNCTION: MW2 0x1007ca8e
MechS32 InitSqrtTable(void)
{
	MechS32 i;

	g_sqrtTable = g_sqrtTableData;
	for (i = 0; i < 0x400; i++) {
		g_sqrtTable[i] = (MechS16) (sqrt(i) * 1024.0);
	}

	return TRUE;
}

#ifndef MW2_MATROX
// FUNCTION: MW2 0x1007caf7
MechScalar Hypot2D(MechScalar p_x, MechScalar p_y)
{
	MechS32 length;
	MechDouble x;
	MechDouble y;

	x = p_x;
	length = (MechS32) sqrt((y = p_y) * y + x * x);
	return length;
}
#endif

// Sets p_matrix to the rotation that points along (p_x, p_y, p_z).
// Operand order: the original loads the squares of p_x, p_z and p_y in that order; this
// build loads p_z, p_y and p_x, and swaps the stack slots of pitch and yaw.
// FUNCTION: MW2 0x1007cb3d
void BuildMatrixFromDirection(Matrix* p_matrix, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechDouble pitch;
	MechDouble yaw;

	yaw = atan2(p_x, p_z);
	pitch = -asin(p_y / sqrt((MechDouble) p_x * p_x + (MechDouble) p_y * p_y + (MechDouble) p_z * p_z));
	BuildMatrix(p_matrix, (MechS32) (pitch * 3754939.378), (MechS32) (yaw * 3754939.378), 0, 0, 0, 0);
}

// The rotation's normalization: 1.1 has it here, the Matrox edition at the end of transform.c's object.
#ifndef MW2_MATROX
#include "normalizerotation.c"
#endif

// FUNCTION: MW2 0x1007cd50
// FUNCTION: MW2MATROX 0x1001e8e0
void FirstClock(void)
{
	if (!g_ticksTimerInitialized) {
		AIL_startup();
		g_ticksTimer = AIL_register_timer((AILTIMERCB) GameTickTimerCallback);
		AIL_set_timer_divisor(g_ticksTimer, 6556);
		AIL_start_timer(g_ticksTimer);
		g_realClockHandle = AllocTicks(0x100);
		g_clockHandle = AllocTicks(0x80);
		g_syncTicksHandle = AllocTicks(0x80);
		ResetTicks(g_realClockHandle);
		ResetTicks(g_syncTicksHandle);
		ResetTicks(g_clockHandle);
		g_ticksTimerInitialized = TRUE;
	}

	g_currentClock = g_previousClock = 0;
	g_realClock = 0;
}

// FUNCTION: MW2 0x1007ce2c
// FUNCTION: MW2MATROX 0x1001e9bc
void NextClock(void)
{
	if (!g_ticksTimerInitialized) {
		return;
	}

	if (g_clockMode == 0) {
		g_currentClock = GetTicks(g_clockHandle);
		if (g_framerateLimit > 0) {
			while (g_currentClock - g_previousClock < g_framerateLimit) {
				g_currentClock = GetTicks(g_clockHandle);
			}
		}

		if (g_clockPaused) {
			g_clockModeBeforePause = g_clockMode;
			g_clockMode = 3;
			DebugPrint("NextClock(1): pause_timer(TRUE)");
			PauseTimer(0x80, TRUE);
		}
	}
	else if (g_clockMode == 3) {
		g_currentClock += 12;
		if (!g_clockPaused) {
			g_clockMode = g_clockModeBeforePause;
			DebugPrint("NextClock(2): pause_timer(FALSE)");
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
// FUNCTION: MW2MATROX 0x1001eb85
void StopTimers(void)
{
	if (g_ticksTimerInitialized) {
		FreeTicks(g_syncTicksHandle);
		FreeTicks(g_clockHandle);
		FreeTicks(g_realClockHandle);
		AIL_release_timer_handle(g_ticksTimer);
		g_ticksTimer = -1;
		AIL_shutdown();
		g_ticksTimerInitialized = FALSE;
	}
}

// FUNCTION: MW2 0x1007d05d
// FUNCTION: MW2MATROX 0x1001ebed
MechS32 GetGameClock(void)
{
	return GetTicks(g_clockHandle);
}

// FUNCTION: MW2 0x1007d07b
// FUNCTION: MW2MATROX 0x1001ec0b
MechS32 GetTicksSinceSync(void)
{
	return GetTicks(g_syncTicksHandle);
}

// FUNCTION: MW2 0x1007d099
// FUNCTION: MW2MATROX 0x1001ec29
void ResetSyncTicks(void)
{
	ResetTicks(g_syncTicksHandle);
}

// FUNCTION: MW2 0x1007d0b2
// FUNCTION: MW2MATROX 0x1001ec42
void ResetClocks(void)
{
	ResetTicks(g_realClockHandle);
	ResetTicks(g_syncTicksHandle);
	ResetTicks(g_clockHandle);
	g_currentClock = g_previousClock = 0;
}

// FUNCTION: MW2 0x1007d0fb
// FUNCTION: MW2MATROX 0x1001ec8b
MechS32 GetRealClock(void)
{
	return GetTicks(g_realClockHandle);
}
