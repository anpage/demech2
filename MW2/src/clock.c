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
// GLOBAL: MW2GBL 0x100a03d4
// GLOBAL: MW2PREMERCS 0x100b03c0
// GLOBAL: MW2MATROX 0x100a4e50
// GLOBAL: MW23DFX 0x1024ab44
// GLOBAL: MW2ATI 0x1017ef94
// GLOBAL: MW2S3 0x1006409c
// GLOBAL: MW2POWERVR 0x101a9ce4
MechS32 g_currentClock = 0;

// Ticks that run on while the game clock is paused; the network times its messages by them.
// GLOBAL: MW2 0x100ba54c
// GLOBAL: MW2GBL 0x100a03d8
// GLOBAL: MW2PREMERCS 0x100b03c4
// GLOBAL: MW2MATROX 0x100a4e54
// GLOBAL: MW23DFX 0x1024ab48
// GLOBAL: MW2ATI 0x1017ef98
// GLOBAL: MW2S3 0x100640a0
// GLOBAL: MW2POWERVR 0x101a9ce8
MechS32 g_realClock = 0;

// GLOBAL: MW2 0x100ba550
// GLOBAL: MW2GBL 0x100a03dc
// GLOBAL: MW2PREMERCS 0x100b03c8
// GLOBAL: MW2MATROX 0x100a4e58
// GLOBAL: MW23DFX 0x1024ab4c
// GLOBAL: MW2ATI 0x1017ef9c
// GLOBAL: MW2S3 0x100640a4
// GLOBAL: MW2POWERVR 0x101a9cec
MechS32 g_deltaTime = 0;

// 0 the game clock runs on its own ticks, 2 a slave's follows the master's, 3 paused.
// GLOBAL: MW2 0x100ba554
// GLOBAL: MW2GBL 0x100a03e0
// GLOBAL: MW2PREMERCS 0x100b03cc
// GLOBAL: MW2MATROX 0x100a4e5c
// GLOBAL: MW23DFX 0x1024ab50
// GLOBAL: MW2ATI 0x1017efa0
// GLOBAL: MW2S3 0x100640a8
// GLOBAL: MW2POWERVR 0x101a9cf0
MechS32 g_clockMode = 0;

// GLOBAL: MW2 0x100ba558
// GLOBAL: MW2GBL 0x100a03e4
// GLOBAL: MW2PREMERCS 0x100b03d0
// GLOBAL: MW2MATROX 0x100a4e60
// GLOBAL: MW23DFX 0x1024ab54
// GLOBAL: MW2S3 0x100640ac
MechS32 g_clockPaused = 0;

// GLOBAL: MW2 0x100ba55c
// GLOBAL: MW2GBL 0x100a03e8
// GLOBAL: MW2PREMERCS 0x100b03d4
// GLOBAL: MW2MATROX 0x100a4e64
// GLOBAL: MW23DFX 0x1024ab58
// GLOBAL: MW2ATI 0x1017efa8
// GLOBAL: MW2S3 0x100640b0
// GLOBAL: MW2POWERVR 0x101a9cf8
HTIMER g_ticksTimer = -1;

// GLOBAL: MW2 0x100ba560
// GLOBAL: MW2GBL 0x100a03ec
// GLOBAL: MW2PREMERCS 0x100b03d8
// GLOBAL: MW2MATROX 0x100a4e68
// GLOBAL: MW23DFX 0x1024ab5c
// GLOBAL: MW2ATI 0x1017efac
// GLOBAL: MW2S3 0x100640b4
// GLOBAL: MW2POWERVR 0x101a9cfc
MechS32 g_timeCompressionEnabled = 0;

// GLOBAL: MW2 0x100ba564
// GLOBAL: MW2GBL 0x100a03f0
// GLOBAL: MW2PREMERCS 0x100b03dc
// GLOBAL: MW2MATROX 0x100a4e6c
// GLOBAL: MW23DFX 0x1024ab60
// GLOBAL: MW2ATI 0x1017efb0
// GLOBAL: MW2S3 0x100640b8
// GLOBAL: MW2POWERVR 0x101a9d00
MechS32 g_timeExpansionEnabled = 0;

// GLOBAL: MW2 0x100ba568
// GLOBAL: MW2GBL 0x100a03f4
// GLOBAL: MW2PREMERCS 0x100b03e0
// GLOBAL: MW2MATROX 0x100a4e70
// GLOBAL: MW23DFX 0x1024ab64
// GLOBAL: MW2ATI 0x1017efb4
// GLOBAL: MW2S3 0x100640bc
// GLOBAL: MW2POWERVR 0x101a9d04
MechS32 g_framerateLimit = 0;

// GLOBAL: MW2 0x100ba56c
// GLOBAL: MW2GBL 0x100a03f8
// GLOBAL: MW2PREMERCS 0x100b03e4
// GLOBAL: MW2MATROX 0x100a4e74
// GLOBAL: MW23DFX 0x1024ab68
// GLOBAL: MW2ATI 0x1017efb8
// GLOBAL: MW2S3 0x100640c0
// GLOBAL: MW2POWERVR 0x101a9d08
MechS32 g_clockHandle = -1;

// The ticks since a slave's last UpdateNetwork: they advance its game clock when the master's
// clock didn't arrive.
// GLOBAL: MW2 0x100ba570
// GLOBAL: MW2GBL 0x100a03fc
// GLOBAL: MW2PREMERCS 0x100b03e8
// GLOBAL: MW2MATROX 0x100a4e78
// GLOBAL: MW23DFX 0x1024ab6c
// GLOBAL: MW2ATI 0x1017efbc
// GLOBAL: MW2S3 0x100640c4
// GLOBAL: MW2POWERVR 0x101a9d0c
MechS32 g_syncTicksHandle = -1;

// GLOBAL: MW2 0x100ba574
// GLOBAL: MW2GBL 0x100a0400
MechS32 g_unk0x100ba574 = -1;

// GLOBAL: MW2 0x100ba578
// GLOBAL: MW2GBL 0x100a0404
// GLOBAL: MW2PREMERCS 0x100b03f0
// GLOBAL: MW2MATROX 0x100a4e80
// GLOBAL: MW23DFX 0x1024ab74
// GLOBAL: MW2ATI 0x1017efc4
// GLOBAL: MW2S3 0x100640cc
// GLOBAL: MW2POWERVR 0x101a9d14
MechS32 g_realClockHandle = -1;

// GLOBAL: MW2 0x100ba57c
// GLOBAL: MW2GBL 0x100a0408
// GLOBAL: MW2PREMERCS 0x100b03f4
// GLOBAL: MW2MATROX 0x100a4e84
// GLOBAL: MW23DFX 0x1024ab78
// GLOBAL: MW2ATI 0x1017efc8
// GLOBAL: MW2S3 0x100640d0
// GLOBAL: MW2POWERVR 0x101a9d18
MechS32 g_previousClock = 0;

// GLOBAL: MW2 0x100ba580
// GLOBAL: MW2GBL 0x100a040c
// GLOBAL: MW2PREMERCS 0x100b03f8
// GLOBAL: MW2MATROX 0x100a4e88
// GLOBAL: MW23DFX 0x1024ab7c
// GLOBAL: MW2S3 0x100640d4
MechS32 g_clockModeBeforePause = 0;

// GLOBAL: MW2 0x100ba584
// GLOBAL: MW2GBL 0x100a0410
// GLOBAL: MW2PREMERCS 0x100b03fc
// GLOBAL: MW2MATROX 0x100a4e8c
// GLOBAL: MW23DFX 0x1024ab80
// GLOBAL: MW2ATI 0x1017efd0
// GLOBAL: MW2S3 0x100640d8
// GLOBAL: MW2POWERVR 0x101a9d20
BOOL g_ticksTimerInitialized = FALSE;

// GLOBAL: MW2 0x100bfd54
// GLOBAL: MW2GBL 0x1017d2f0
// GLOBAL: MW2PREMERCS 0x10189630
MechS16* g_sqrtTable;

// GLOBAL: MW2 0x100bfd60
// GLOBAL: MW2GBL 0x1017d300
// GLOBAL: MW2PREMERCS 0x10189640
// GLOBAL: MW2MATROX 0x1012d7d0
// GLOBAL: MW23DFX 0x1015d260
// GLOBAL: MW2ATI 0x10091250
// GLOBAL: MW2S3 0x1007f3f0
// GLOBAL: MW2POWERVR 0x10087790
MechScalar g_slopeSines[800];

// GLOBAL: MW2 0x100c09e0
// GLOBAL: MW2GBL 0x1017df80
// GLOBAL: MW2PREMERCS 0x1018a2c0
// GLOBAL: MW2MATROX 0x1012cb50
// GLOBAL: MW23DFX 0x1015c5e0
// GLOBAL: MW2ATI 0x100905d0
// GLOBAL: MW2S3 0x10080070
// GLOBAL: MW2POWERVR 0x10086b10
MechScalar g_slopeCosines[800];

// GLOBAL: MW2 0x100c1660
// GLOBAL: MW2GBL 0x1017ec00
// GLOBAL: MW2PREMERCS 0x1018af40
// GLOBAL: MW2MATROX 0x1012e450
// GLOBAL: MW23DFX 0x1015dee0
// GLOBAL: MW2ATI 0x10091ed0
// GLOBAL: MW2S3 0x10080cf0
// GLOBAL: MW2POWERVR 0x10088410
MechScalar g_sinTable[TRIG_TABLE_SIZE];

// GLOBAL: MW2 0x100c1a80
// GLOBAL: MW2GBL 0x1017f020
// GLOBAL: MW2PREMERCS 0x1018b360
MechS16 g_sqrtTableData[0x400];

// GLOBAL: MW2 0x100c2290
// GLOBAL: MW2GBL 0x1017f830
// GLOBAL: MW2PREMERCS 0x1018bb70
// GLOBAL: MW2MATROX 0x1012ec50
// GLOBAL: MW23DFX 0x1015e6f0
// GLOBAL: MW2ATI 0x100926e0
// GLOBAL: MW2S3 0x10081500
// GLOBAL: MW2POWERVR 0x10088c20
MechScalar g_atanTable[TRIG_TABLE_SIZE];

// The tables, Hypot2D and BuildMatrixFromDirection: GBL has them in an object of their own (and
// the early Mercenaries build uses GBL's units, until its own are placed).
#if !defined(MW2_GBL) && !defined(MW2_PREMERCS)
#include "clockmath.c"
#endif

// FUNCTION: MW2 0x1007cd50
// FUNCTION: MW2GBL 0x1001d4f0
// FUNCTION: MW2PREMERCS 0x10026aa0
// FUNCTION: MW2MATROX 0x1001e8e0
// FUNCTION: MW23DFX 0x10007f20
// FUNCTION: MW2ATI 0x10008740
// FUNCTION: MW2S3 0x10014870
// FUNCTION: MW2POWERVR 0x100074b0
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
// FUNCTION: MW2GBL 0x1001d5cc
// FUNCTION: MW2PREMERCS 0x10026b7c
// FUNCTION: MW2MATROX 0x1001e9bc
// FUNCTION: MW23DFX 0x10007ff0
// FUNCTION: MW2ATI 0x10008810
// FUNCTION: MW2S3 0x10014940
// FUNCTION: MW2POWERVR 0x10007580
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
// FUNCTION: MW2GBL 0x1001d795
// FUNCTION: MW2PREMERCS 0x10026d45
// FUNCTION: MW2MATROX 0x1001eb85
// FUNCTION: MW23DFX 0x10008190
// FUNCTION: MW2ATI 0x100089d0
// FUNCTION: MW2S3 0x10014ae0
// FUNCTION: MW2POWERVR 0x10007740
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
// FUNCTION: MW2GBL 0x1001d7fd
// FUNCTION: MW2PREMERCS 0x10026dad
// FUNCTION: MW2MATROX 0x1001ebed
// FUNCTION: MW23DFX 0x100081f0
// FUNCTION: MW2ATI 0x10008a30
// FUNCTION: MW2S3 0x10014b40
// FUNCTION: MW2POWERVR 0x100077a0
MechS32 GetGameClock(void)
{
	return GetTicks(g_clockHandle);
}

// FUNCTION: MW2 0x1007d07b
// FUNCTION: MW2GBL 0x1001d81b
// FUNCTION: MW2PREMERCS 0x10026dcb
// FUNCTION: MW2MATROX 0x1001ec0b
// FUNCTION: MW23DFX 0x10008200
// FUNCTION: MW2ATI 0x10008a40
// FUNCTION: MW2S3 0x10014b50
// FUNCTION: MW2POWERVR 0x100077b0
MechS32 GetTicksSinceSync(void)
{
	return GetTicks(g_syncTicksHandle);
}

// FUNCTION: MW2 0x1007d099
// FUNCTION: MW2GBL 0x1001d839
// FUNCTION: MW2PREMERCS 0x10026de9
// FUNCTION: MW2MATROX 0x1001ec29
// FUNCTION: MW23DFX 0x10008210
// FUNCTION: MW2ATI 0x10008a50
// FUNCTION: MW2S3 0x10014b60
// FUNCTION: MW2POWERVR 0x100077c0
void ResetSyncTicks(void)
{
	ResetTicks(g_syncTicksHandle);
}

// FUNCTION: MW2 0x1007d0b2
// FUNCTION: MW2GBL 0x1001d852
// FUNCTION: MW2PREMERCS 0x10026e02
// FUNCTION: MW2MATROX 0x1001ec42
// FUNCTION: MW23DFX 0x10008220
// FUNCTION: MW2ATI 0x10008a60
// FUNCTION: MW2S3 0x10014b70
// FUNCTION: MW2POWERVR 0x100077d0
void ResetClocks(void)
{
	ResetTicks(g_realClockHandle);
	ResetTicks(g_syncTicksHandle);
	ResetTicks(g_clockHandle);
	g_currentClock = g_previousClock = 0;
}

// FUNCTION: MW2 0x1007d0fb
// FUNCTION: MW2GBL 0x1001d89b
// FUNCTION: MW2PREMERCS 0x10026e4b
// FUNCTION: MW2MATROX 0x1001ec8b
// FUNCTION: MW23DFX 0x10008260
// FUNCTION: MW2ATI 0x10008aa0
// FUNCTION: MW2S3 0x10014bb0
// FUNCTION: MW2POWERVR 0x10007810
MechS32 GetRealClock(void)
{
	return GetTicks(g_realClockHandle);
}
