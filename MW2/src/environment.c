/* The mission's time of day: g_timeOfDay counts seconds of a g_secondsPerDay-second day, and
   the four phases that start at g_timeOfDayStarts each fade to their own palette. */
#include "environment.h"

#include "clock.h"
#include "cockpitpanel.h"
#include "config.h"
#include "decomp.h"
#include "fixeddiv.h"
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
TimeOfDayPhase g_timeOfDayPhases[4] = {{4, 2715}, {0, 2715}, {4, 3620}, {8, 2715}};

// GLOBAL: MW2 0x100ba5f8
MechS32 g_timeOfDayPhase = -1;

// GLOBAL: MW2 0x100ba5fc
MechS32 g_unk0x100ba5fc = 0x168;

// GLOBAL: MW2 0x100ba600
MechS32 g_unk0x100ba600 = 0x794;

// GLOBAL: MW2 0x100ba604
MechS32 g_unk0x100ba604 = 0x10000;

// GLOBAL: MW2 0x100ba608
MechS32 g_unk0x100ba608 = 0;

// GLOBAL: MW2 0x100ba60c
MechS32 g_secondsPerDay = 86400;

// GLOBAL: MW2 0x100ba610
MechS32 g_unk0x100ba610 = 365;

// GLOBAL: MW2 0x100ba614
MechS32 g_unk0x100ba614 = 0;

// GLOBAL: MW2 0x100ba618
MechS32 g_timeOfDay = 43200;

// GLOBAL: MW2 0x100ba61c
MechS32 g_unk0x100ba61c = 43200;

// GLOBAL: MW2 0x100bfaa0
MechS32 g_unk0x100bfaa0;

// GLOBAL: MW2 0x100bfaa8
MechS32 g_timeOfDayStarts[4];

// GLOBAL: MW2 0x100bfab8
MechS32 g_unk0x100bfab8;

// GLOBAL: MW2 0x100bfd4c
MechS32 g_unk0x100bfd4c;

// GLOBAL: MW2 0x100bfd50
MechS32 g_unk0x100bfd50;

// FUNCTION: MW2 0x1007d610
void FirstEnvironment(void)
{
	MechS32 hour;

	g_unk0x100bfab8 = 1;
	hour = g_secondsPerDay / 24;
	g_timeOfDayStarts[0] = hour * 5;
	g_timeOfDayStarts[1] = hour * 7;
	g_timeOfDayStarts[2] = hour * 17;
	g_timeOfDayStarts[3] = hour * 19;
	if (g_timeOfDayPhase != -1) {
		g_timeOfDay = g_timeOfDayStarts[g_timeOfDayPhase];
	}

	g_unk0x100ba61c = g_timeOfDay;
	g_timeOfDayPhase = -1;
	g_unk0x100ba604 = FixedDiv16(g_unk0x100ba600, 0x794);
}

// Stack slots: seconds, time and i are permuted.
// FUNCTION: MW2 0x1007d6bb
void FUN_1007d6bb(void)
{
	MechS32 phase;
	MechS32 seconds;
	MechS32 time;
	MechS32 i;

	phase = 3;
	if (g_unk0x100c3280[0]->m_unk0x06 >= 1 && g_unk0x100bfd50 == 1) {
		FUN_1007d88a(0, 0);
		g_unk0x100bfd50 = 0;
	}

	if (g_unk0x100bfaa0 < 2) {
		g_unk0x100bfaa0++;
		g_unk0x100bfd4c = 0;
	}
	else {
		if (g_currentClock < g_unk0x100bfd4c) {
			return;
		}

		g_unk0x100bfd4c = g_currentClock + 0x712;
		seconds = g_currentClock / 181;
		time = (g_unk0x100ba61c + seconds) % g_secondsPerDay;
		if (time < g_timeOfDay) {
			g_unk0x100ba614++;
			g_unk0x100ba614 %= g_unk0x100ba610;
		}

		g_timeOfDay = time;
		for (i = 0; i <= 3; i++) {
			if (g_timeOfDayStarts[i] <= g_timeOfDay) {
				phase = i;
			}
		}

		FUN_1007d7e3(phase);
	}
}

// Operand order: the original compares p_phase != g_timeOfDayPhase with g_timeOfDayPhase in eax.
// FUNCTION: MW2 0x1007d7e3
void FUN_1007d7e3(MechS32 p_phase)
{
	MechS32 duration;

	if (g_unk0x100bfab8 == 1 && p_phase != g_timeOfDayPhase) {
		if (g_unk0x100bfd50 == 1) {
			duration = 181;
			g_unk0x100bfd50 = 0;
		}
		else {
			duration = g_timeOfDayPhases[p_phase].m_duration;
		}

		if (g_unk0x100bfaa0 == 2) {
			g_unk0x100bfaa0++;
			duration = 362;
		}

		FUN_10002a24(g_timeOfDayPhases[p_phase].m_palette, duration);
		g_timeOfDayPhase = p_phase;
	}
}

// FUNCTION: MW2 0x1007d875
MechS32 FUN_1007d875(undefined4 p_unk0x00)
{
	return g_unk0x100bfd50;
}

// Operand order: the original compares g_unk0x100bfd50 != p_state with p_state in eax.
// FUNCTION: MW2 0x1007d88a
void FUN_1007d88a(undefined4 p_unk0x00, MechS32 p_state)
{
	if (g_unk0x100bfd50 != p_state) {
		if (p_state == 1) {
			if (g_unk0x100c3280[0]->m_unk0x06 < 1) {
				g_unk0x100bfd50 = 1;
				g_unk0x100bfab8 = 0;
				FUN_10002a24(12, 181);
				FUN_1007eb23(0xb2, 100, 0x40, 5, 0x50);
				PlayCockpitSound(0x1c, 1);
			}
		}
		else if (p_state == 0) {
			g_unk0x100bfab8 = 1;
			g_timeOfDayPhase = -1;
			g_unk0x100bfd4c = 0;
		}
	}
}

// Stack slots: i and mech are swapped.
// FUNCTION: MW2 0x1007d931
void FUN_1007d931(void)
{
	MechS32 i;
	Mech* mech;

	for (i = 0; i < g_playerCount; i++) {
		mech = g_players[i]->m_mech;
		mech->m_cooling <<= 2;
	}
}

// Stack slots: i and mech are swapped.
// FUNCTION: MW2 0x1007d97c
void FUN_1007d97c(void)
{
	MechS32 i;
	Mech* mech;

	for (i = 0; i < g_playerCount; i++) {
		mech = g_players[i]->m_mech;
		mech->m_cooling >>= 2;
	}
}
