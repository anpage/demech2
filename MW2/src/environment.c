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
// GLOBAL: MW2MATROX 0x100bbb80
// GLOBAL: MW23DFX 0x1024a960
// GLOBAL: MW2ATI 0x1017edb8
TimeOfDayPhase g_timeOfDayPhases[4] = {{4, 2715}, {0, 2715}, {4, 3620}, {8, 2715}};

// GLOBAL: MW2 0x100ba5f8
// GLOBAL: MW2MATROX 0x100bbba0
// GLOBAL: MW23DFX 0x1024a980
// GLOBAL: MW2ATI 0x1017edd8
MechS32 g_timeOfDayPhase = -1;

// The Matrox edition keeps these three as floats, in plain units.

// GLOBAL: MW2 0x100ba5fc
// GLOBAL: MW2MATROX 0x100bbba4
// GLOBAL: MW23DFX 0x1024a984
MechScalar g_soundDelayPerUnit = FIXED_LITERAL(0x168, 0.00549f);

// GLOBAL: MW2 0x100ba600
// GLOBAL: MW2MATROX 0x100bbba8
// GLOBAL: MW23DFX 0x1024a988
// GLOBAL: MW2ATI 0x1017ede0
MechScalar g_gravity = FIXED_LITERAL(0x794, 0.0296f);

// GLOBAL: MW2 0x100ba604
// GLOBAL: MW2MATROX 0x100bbbac
// GLOBAL: MW23DFX 0x1024a98c
// GLOBAL: MW2ATI 0x1017ede4
MechScalar g_gravityScale = FIXED_RAW(0x10000);

// GLOBAL: MW2 0x100ba608
// GLOBAL: MW2MATROX 0x100bbbb0
// GLOBAL: MW23DFX 0x1024a990
MechS32 g_unk0x100ba608 = 0;

// GLOBAL: MW2 0x100ba60c
// GLOBAL: MW2MATROX 0x100bbbb4
// GLOBAL: MW23DFX 0x1024a994
// GLOBAL: MW2ATI 0x1017edec
MechS32 g_secondsPerDay = 86400;

// GLOBAL: MW2 0x100ba610
// GLOBAL: MW2MATROX 0x100bbbb8
// GLOBAL: MW23DFX 0x1024a998
MechS32 g_daysPerYear = 365;

// GLOBAL: MW2 0x100ba614
// GLOBAL: MW2MATROX 0x100bbbbc
// GLOBAL: MW23DFX 0x1024a99c
MechS32 g_dayOfYear = 0;

// GLOBAL: MW2 0x100ba618
// GLOBAL: MW2MATROX 0x100bbbc0
// GLOBAL: MW23DFX 0x1024a9a0
// GLOBAL: MW2ATI 0x1017edf8
MechS32 g_timeOfDay = 43200;

// GLOBAL: MW2 0x100ba61c
// GLOBAL: MW2MATROX 0x100bbbc4
// GLOBAL: MW23DFX 0x1024a9a4
// GLOBAL: MW2ATI 0x1017edfc
MechS32 g_startTimeOfDay = 43200;

// GLOBAL: MW2 0x100ba620
// GLOBAL: MW2MATROX 0x100bbbc8
// GLOBAL: MW23DFX 0x1024a9a8
// GLOBAL: MW2ATI 0x1017ee00
MechS32 g_temperature = 25;

// Set on a planet without a breathable atmosphere: the local player can't eject (the cockpit says
// so), a life support hit kills, and a mech whose pilot ejects counts as lost.
// GLOBAL: MW2 0x100ba624
// GLOBAL: MW2MATROX 0x100bbbcc
// GLOBAL: MW23DFX 0x1024a9ac
// GLOBAL: MW2ATI 0x1017ee04
MechS32 g_hostileAtmosphere = 0;

// GLOBAL: MW2 0x100bfaa0
// GLOBAL: MW2MATROX 0x101252d4
// GLOBAL: MW23DFX 0x1007b064
// GLOBAL: MW2ATI 0x10070064
MechS32 g_timeOfDayFrames;

// GLOBAL: MW2 0x100bfaa8
// GLOBAL: MW2MATROX 0x101252d8
// GLOBAL: MW23DFX 0x1007b068
// GLOBAL: MW2ATI 0x10070068
MechS32 g_timeOfDayStarts[4];

// GLOBAL: MW2 0x100bfab8
// GLOBAL: MW2MATROX 0x101252d0
// GLOBAL: MW23DFX 0x1007b060
// GLOBAL: MW2ATI 0x10070060
MechS32 g_timeOfDayEnabled;

// GLOBAL: MW2 0x100bfd4c
// GLOBAL: MW2MATROX 0x1012c1f0
// GLOBAL: MW23DFX 0x102443a0
MechS32 g_nextTimeOfDayUpdate;

// GLOBAL: MW2 0x100bfd50
// GLOBAL: MW2MATROX 0x1012c1f4
// GLOBAL: MW23DFX 0x102443a4
// GLOBAL: MW2ATI 0x10178064
MechS32 g_infraredOn;

// FUNCTION: MW2 0x1007d610
// FUNCTION: MW2MATROX 0x10085c80
// FUNCTION: MW23DFX 0x10007870
// FUNCTION: MW2ATI 0x100081d0
void FirstEnvironment(void)
{
	MechS32 hour;

	g_timeOfDayEnabled = 1;
	hour = g_secondsPerDay / 24;
	g_timeOfDayStarts[0] = hour * 5;
	g_timeOfDayStarts[1] = hour * 7;
	g_timeOfDayStarts[2] = hour * 17;
	g_timeOfDayStarts[3] = hour * 19;
	if (g_timeOfDayPhase != -1) {
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
// FUNCTION: MW2MATROX 0x10085d3b
// FUNCTION: MW23DFX 0x10007900
// FUNCTION: MW2ATI 0x10008260
void UpdateTimeOfDay(void)
{
	MechS32 phase;
	MechS32 seconds;
	MechS32 time;
	MechS32 i;

	phase = 3;
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
		for (i = 0; i <= 3; i++) {
			if (g_timeOfDayStarts[i] <= g_timeOfDay) {
				phase = i;
			}
		}

		FadeToTimeOfDayPhase(phase);
	}
}

// FUNCTION: MW2 0x1007d7e3
// FUNCTION: MW2MATROX 0x10085e63
// FUNCTION: MW23DFX 0x100079d0
// FUNCTION: MW2ATI 0x10008330
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
// FUNCTION: MW2MATROX 0x10085ef4
// FUNCTION: MW23DFX 0x10007a40
// FUNCTION: MW2ATI 0x100083a0
MechS32 IsInfraredOn(undefined4 p_unk0x00)
{
	return g_infraredOn;
}

// FUNCTION: MW2 0x1007d88a
// FUNCTION: MW2MATROX 0x10085f09
// FUNCTION: MW23DFX 0x10007a50
// FUNCTION: MW2ATI 0x100083b0
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
