/* The mission's time of day: g_timeOfDay counts seconds of a g_secondsPerDay-second day, and
   the four phases that start at g_timeOfDayStarts each fade to their own palette. */
#include "environment.h"

#include "clock.h"
#include "cockpitpanel.h"
#include "config.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "fixedfloat.h"
#include "mech.h"
#include "palette.h"
#include "players.h"
#include "soundfx.h"
#include "speech.h"
#include "types.h"

typedef struct TimeOfDayPhase {
	MechS32 m_palette;  // 0x00
	MechS32 m_duration; // 0x04
} TimeOfDayPhase;

DECOMP_SIZE_ASSERT(TimeOfDayPhase, 0x08)

// GLOBAL: MW2 0x100ba5d8
// GLOBAL: MW2GBL 0x100adba8
// GLOBAL: MW2PREMERCS 0x100b01f0
// GLOBAL: MW2MATROX 0x100bbb80
// GLOBAL: MW23DFX 0x1024a960
// GLOBAL: MW2ATI 0x1017edb8
// GLOBAL: MW2S3 0x10073228
// GLOBAL: MW2POWERVR 0x101a9b08
TimeOfDayPhase g_timeOfDayPhases[4] = {{4, 2715}, {0, 2715}, {4, 3620}, {8, 2715}};

// GLOBAL: MW2 0x100ba5f8
// GLOBAL: MW2GBL 0x100adbc8
// GLOBAL: MW2PREMERCS 0x100b0210
// GLOBAL: MW2MATROX 0x100bbba0
// GLOBAL: MW23DFX 0x1024a980
// GLOBAL: MW2ATI 0x1017edd8
// GLOBAL: MW2S3 0x10073248
// GLOBAL: MW2POWERVR 0x101a9b28
MechS32 g_timeOfDayPhase = -1;

// The Matrox edition keeps these three as floats, in plain units.

// GLOBAL: MW2 0x100ba5fc
// GLOBAL: MW2GBL 0x100adbcc
// GLOBAL: MW2PREMERCS 0x100b0214
// GLOBAL: MW2MATROX 0x100bbba4
// GLOBAL: MW23DFX 0x1024a984
// GLOBAL: MW2POWERVR 0x101a9b2c
MechScalar g_soundDelayPerUnit = FIXED_LITERAL(0x168, 0.00549f);

// GLOBAL: MW2 0x100ba600
// GLOBAL: MW2GBL 0x100adbd0
// GLOBAL: MW2PREMERCS 0x100b0218
// GLOBAL: MW2MATROX 0x100bbba8
// GLOBAL: MW23DFX 0x1024a988
// GLOBAL: MW2ATI 0x1017ede0
// GLOBAL: MW2S3 0x10073250
// GLOBAL: MW2POWERVR 0x101a9b30
MechScalar g_gravity = FIXED_LITERAL(0x794, 0.0296f);

// GLOBAL: MW2 0x100ba604
// GLOBAL: MW2GBL 0x100adbd4
// GLOBAL: MW2PREMERCS 0x100b021c
// GLOBAL: MW2MATROX 0x100bbbac
// GLOBAL: MW23DFX 0x1024a98c
// GLOBAL: MW2ATI 0x1017ede4
// GLOBAL: MW2S3 0x10073254
// GLOBAL: MW2POWERVR 0x101a9b34
MechScalar g_gravityScale = FIXED_RAW(0x10000);

// GLOBAL: MW2 0x100ba608
// GLOBAL: MW2GBL 0x100adbd8
// GLOBAL: MW2PREMERCS 0x100b0220
// GLOBAL: MW2MATROX 0x100bbbb0
// GLOBAL: MW23DFX 0x1024a990
// GLOBAL: MW2S3 0x10073258
MechS32 g_unk0x100ba608 = 0;

// GLOBAL: MW2 0x100ba60c
// GLOBAL: MW2GBL 0x100adbdc
// GLOBAL: MW2PREMERCS 0x100b0224
// GLOBAL: MW2MATROX 0x100bbbb4
// GLOBAL: MW23DFX 0x1024a994
// GLOBAL: MW2ATI 0x1017edec
// GLOBAL: MW2S3 0x1007325c
// GLOBAL: MW2POWERVR 0x101a9b3c
MechS32 g_secondsPerDay = 86400;

// GLOBAL: MW2 0x100ba610
// GLOBAL: MW2GBL 0x100adbe0
// GLOBAL: MW2PREMERCS 0x100b0228
// GLOBAL: MW2MATROX 0x100bbbb8
// GLOBAL: MW23DFX 0x1024a998
// GLOBAL: MW2S3 0x10073260
// GLOBAL: MW2POWERVR 0x101a9b40
MechS32 g_daysPerYear = 365;

// GLOBAL: MW2 0x100ba614
// GLOBAL: MW2GBL 0x100adbe4
// GLOBAL: MW2PREMERCS 0x100b022c
// GLOBAL: MW2MATROX 0x100bbbbc
// GLOBAL: MW23DFX 0x1024a99c
// GLOBAL: MW2S3 0x10073264
// GLOBAL: MW2POWERVR 0x101a9b44
MechS32 g_dayOfYear = 0;

// GLOBAL: MW2 0x100ba618
// GLOBAL: MW2GBL 0x100adbe8
// GLOBAL: MW2PREMERCS 0x100b0230
// GLOBAL: MW2MATROX 0x100bbbc0
// GLOBAL: MW23DFX 0x1024a9a0
// GLOBAL: MW2ATI 0x1017edf8
// GLOBAL: MW2S3 0x10073268
// GLOBAL: MW2POWERVR 0x101a9b48
MechS32 g_timeOfDay = 43200;

// GLOBAL: MW2 0x100ba61c
// GLOBAL: MW2GBL 0x100adbec
// GLOBAL: MW2PREMERCS 0x100b0234
// GLOBAL: MW2MATROX 0x100bbbc4
// GLOBAL: MW23DFX 0x1024a9a4
// GLOBAL: MW2ATI 0x1017edfc
// GLOBAL: MW2S3 0x1007326c
// GLOBAL: MW2POWERVR 0x101a9b4c
MechS32 g_startTimeOfDay = 43200;

// GLOBAL: MW2 0x100ba620
// GLOBAL: MW2GBL 0x100adbf0
// GLOBAL: MW2PREMERCS 0x100b0238
// GLOBAL: MW2MATROX 0x100bbbc8
// GLOBAL: MW23DFX 0x1024a9a8
// GLOBAL: MW2ATI 0x1017ee00
// GLOBAL: MW2S3 0x10073270
// GLOBAL: MW2POWERVR 0x101a9b50
MechS32 g_temperature = 25;

// Set on a planet without a breathable atmosphere: the local player can't eject (the cockpit says
// so), a life support hit kills, and a mech whose pilot ejects counts as lost.
// GLOBAL: MW2 0x100ba624
// GLOBAL: MW2GBL 0x100adbf4
// GLOBAL: MW2PREMERCS 0x100b023c
// GLOBAL: MW2MATROX 0x100bbbcc
// GLOBAL: MW23DFX 0x1024a9ac
// GLOBAL: MW2ATI 0x1017ee04
// GLOBAL: MW2S3 0x10073274
// GLOBAL: MW2POWERVR 0x101a9b54
MechS32 g_hostileAtmosphere = 0;

// GLOBAL: MW2 0x100bfaa0
// GLOBAL: MW2GBL 0x100b9d60
// GLOBAL: MW2PREMERCS 0x100c8e18
// GLOBAL: MW2MATROX 0x101252d4
// GLOBAL: MW23DFX 0x1007b064
// GLOBAL: MW2ATI 0x10070064
// GLOBAL: MW2S3 0x1007d808
// GLOBAL: MW2POWERVR 0x1006e064
MechS32 g_timeOfDayFrames;

// GLOBAL: MW2 0x100bfaa8
// GLOBAL: MW2GBL 0x100b9d68
// GLOBAL: MW2PREMERCS 0x100c8e20
// GLOBAL: MW2MATROX 0x101252d8
// GLOBAL: MW23DFX 0x1007b068
// GLOBAL: MW2ATI 0x10070068
// GLOBAL: MW2S3 0x1007d810
// GLOBAL: MW2POWERVR 0x1006e068
MechS32 g_timeOfDayStarts[4];

// GLOBAL: MW2 0x100bfab8
// GLOBAL: MW2GBL 0x100b9d78
// GLOBAL: MW2PREMERCS 0x100c8e30
// GLOBAL: MW2MATROX 0x101252d0
// GLOBAL: MW23DFX 0x1007b060
// GLOBAL: MW2ATI 0x10070060
// GLOBAL: MW2S3 0x1007d820
// GLOBAL: MW2POWERVR 0x1006e060
MechS32 g_timeOfDayEnabled;

// GLOBAL: MW2 0x100bfd4c
// GLOBAL: MW2GBL 0x100e98d4
// GLOBAL: MW2PREMERCS 0x10212f30
// GLOBAL: MW2MATROX 0x1012c1f0
// GLOBAL: MW23DFX 0x102443a0
// GLOBAL: MW2S3 0x1008ac1c
// GLOBAL: MW2POWERVR 0x101a2d20
MechS32 g_nextTimeOfDayUpdate;

// GLOBAL: MW2 0x100bfd50
// GLOBAL: MW2GBL 0x100e98d8
// GLOBAL: MW2PREMERCS 0x10212f34
// GLOBAL: MW2MATROX 0x1012c1f4
// GLOBAL: MW23DFX 0x102443a4
// GLOBAL: MW2ATI 0x10178064
// GLOBAL: MW2S3 0x1008ac20
// GLOBAL: MW2POWERVR 0x101a2d24
MechS32 g_infraredOn;

#ifdef MW2_TIME_OF_DAY_LOOKUP
// The phase the time of day p_time falls in: the last whose start it has reached, or the night's
// (3) before the first.
// FUNCTION: MW2POWERVR 0x10006f10
MechS32 FUN_10006f10(MechS32 p_time)
{
	MechS32 phase;
	MechS32 i;

	phase = 3;
	for (i = 0; i <= 3; i++) {
		if (g_timeOfDayStarts[i] <= p_time) {
			phase = i;
		}
	}

	return phase;
}

// The palette of the current time of day's phase.
// FUNCTION: MW2POWERVR 0x10006f30
MechS32 FUN_10006f30(void)
{
	return g_timeOfDayPhases[FUN_10006f10(g_timeOfDay)].m_palette;
}
#endif

// FUNCTION: MW2 0x1007d610
// FUNCTION: MW2GBL 0x1003c780
// FUNCTION: MW2PREMERCS 0x10026210
// FUNCTION: MW2MATROX 0x10085c80
// FUNCTION: MW23DFX 0x10007870
// FUNCTION: MW2ATI 0x100081d0
// FUNCTION: MW2S3 0x10035170
// FUNCTION: MW2POWERVR 0x10006f50
void FirstEnvironment(void)
{
	MechS32 hour;

	g_timeOfDayEnabled = 1;
	hour = g_secondsPerDay / 24;
	g_timeOfDayStarts[0] = hour * 5;
	g_timeOfDayStarts[1] = hour * 7;
	g_timeOfDayStarts[2] = hour * 17;
	g_timeOfDayStarts[3] = hour * 19;
#ifdef MW2_TIME_OF_DAY_PHASES_1_TO_3
	if (g_timeOfDayPhase > 0 && g_timeOfDayPhase < 4) {
#else
	if (g_timeOfDayPhase != -1) {
#endif
		g_timeOfDay = g_timeOfDayStarts[g_timeOfDayPhase];
	}

	g_startTimeOfDay = g_timeOfDay;
	g_timeOfDayPhase = -1;
#ifdef MW2_FLOAT_SCALARS
	g_gravityScale = g_gravity / FIXED_LITERAL(0x794, 0.0296f);
#else
	g_gravityScale = FixedDiv16(g_gravity, 0x794);
#endif
}

// FUNCTION: MW2 0x1007d6bb
// FUNCTION: MW2GBL 0x1003c82b
// FUNCTION: MW2MATROX 0x10085d3b
// FUNCTION: MW23DFX 0x10007900
// FUNCTION: MW2ATI 0x10008260
// FUNCTION: MW2S3 0x10035200
// FUNCTION: MW2POWERVR 0x10006fe0
void UpdateTimeOfDay(void)
{
#ifndef MW2_TIME_OF_DAY_LOOKUP
	MechS32 phase;
#endif
	MechS32 seconds;
	MechS32 time;
#ifndef MW2_TIME_OF_DAY_LOOKUP
	MechS32 i;

	phase = 3;
#endif
	if (g_cockpitPanels[c_panelRadar]->m_damage >= 1 && g_infraredOn == 1) {
		SetInfrared(0, 0);
		g_infraredOn = 0;
	}

	if (g_timeOfDayFrames < 2) {
		g_timeOfDayFrames++;
		g_nextTimeOfDayUpdate = 0;
	}
	else {
		if (g_currentClock < g_nextTimeOfDayUpdate) {
			return;
		}

		g_nextTimeOfDayUpdate = g_currentClock + 0x712;
		seconds = g_currentClock / 181;
		time = (g_startTimeOfDay + seconds) % g_secondsPerDay;
		if (time < g_timeOfDay) {
			g_dayOfYear++;
			g_dayOfYear %= g_daysPerYear;
		}

		g_timeOfDay = time;
#ifdef MW2_TIME_OF_DAY_LOOKUP
		FadeToTimeOfDayPhase(FUN_10006f10(time));
#else
		for (i = 0; i <= 3; i++) {
			if (g_timeOfDayStarts[i] <= g_timeOfDay) {
				phase = i;
			}
		}

		FadeToTimeOfDayPhase(phase);
#endif
	}
}

// FUNCTION: MW2 0x1007d7e3
// FUNCTION: MW2GBL 0x1003c953
// FUNCTION: MW2PREMERCS 0x100263e3
// FUNCTION: MW2MATROX 0x10085e63
// FUNCTION: MW23DFX 0x100079d0
// FUNCTION: MW2ATI 0x10008330
// FUNCTION: MW2S3 0x100352d0
// FUNCTION: MW2POWERVR 0x100070a0
void FadeToTimeOfDayPhase(MechS32 p_phase)
{
	MechS32 duration;

#ifdef MW2_TIME_OF_DAY_FADE_ONCE
	if (g_timeOfDayEnabled == 1 && g_timeOfDayPhase == -1) {
#else
	if (g_timeOfDayEnabled == 1 && p_phase != g_timeOfDayPhase) {
#endif
		if (g_infraredOn == 1) {
			duration = 181;
			g_infraredOn = 0;
		}
		else {
			duration = g_timeOfDayPhases[p_phase].m_duration;
		}

		if (g_timeOfDayFrames == 2) {
			g_timeOfDayFrames++;
			duration = 362;
		}

		FadeToBasePalette(g_timeOfDayPhases[p_phase].m_palette, duration);
		g_timeOfDayPhase = p_phase;
	}
}

// FUNCTION: MW2 0x1007d875
// FUNCTION: MW2GBL 0x1003c9e6
// FUNCTION: MW2PREMERCS 0x10026475
// FUNCTION: MW2MATROX 0x10085ef4
// FUNCTION: MW23DFX 0x10007a40
// FUNCTION: MW2ATI 0x100083a0
// FUNCTION: MW2S3 0x10035340
// FUNCTION: MW2POWERVR 0x10007110
MechS32 IsInfraredOn(undefined4 p_unk0x00)
{
	return g_infraredOn;
}

// FUNCTION: MW2 0x1007d88a
// FUNCTION: MW2GBL 0x1003c9fb
// FUNCTION: MW2PREMERCS 0x1002648a
// FUNCTION: MW2MATROX 0x10085f09
// FUNCTION: MW23DFX 0x10007a50
// FUNCTION: MW2ATI 0x100083b0
// FUNCTION: MW2S3 0x10035350
// FUNCTION: MW2POWERVR 0x10007120
void SetInfrared(undefined4 p_unk0x00, MechS32 p_state)
{
	if (g_infraredOn != p_state) {
		if (p_state == 1) {
			if (g_cockpitPanels[c_panelRadar]->m_damage < 1) {
				g_infraredOn = 1;
				g_timeOfDayEnabled = 0;
				FadeToBasePalette(12, 181);
				PlaySoundEffect(0xb2, 100, 0x40, 5, 0x50);
				PlayCockpitSound(0x1c, 1);
			}
		}
		else if (p_state == 0) {
#ifdef MW2_INFRARED_OFF_FADE
			FadeToBasePalette(0, 181);
#endif
			g_timeOfDayEnabled = 1;
			g_timeOfDayPhase = -1;
			g_nextTimeOfDayUpdate = 0;
		}
	}
}

// FUNCTION: MW2 0x1007d931
// FUNCTION: MW2GBL 0x1003caa1
// FUNCTION: MW2PREMERCS 0x10026530
// FUNCTION: MW2MATROX 0x10085fbe
// FUNCTION: MW23DFX 0x10007af0
void QuadrupleMechCooling(void)
{
	MechS32 i;
	Mech* mech;

	for (i = 0; i < g_playerCount; i++) {
		mech = g_players[i]->m_mech;
#ifdef MW2_FLOAT_SCALARS
		mech->m_cooling *= 4;
#else
		mech->m_cooling <<= 2;
#endif
	}
}

// FUNCTION: MW2 0x1007d97c
// FUNCTION: MW2GBL 0x1003caec
// FUNCTION: MW2PREMERCS 0x1002657b
// FUNCTION: MW2MATROX 0x10086017
// FUNCTION: MW23DFX 0x10007b30
void QuarterMechCooling(void)
{
	MechS32 i;
	Mech* mech;

	for (i = 0; i < g_playerCount; i++) {
		mech = g_players[i]->m_mech;
#ifdef MW2_FLOAT_SCALARS
		mech->m_cooling /= 4;
#else
		mech->m_cooling >>= 2;
#endif
	}
}
