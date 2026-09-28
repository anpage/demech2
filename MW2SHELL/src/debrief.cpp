#include "campaignmission.h"
#include "decomp.h"
#include "tinwhistle0x3c.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <stdio.h>

// SIZE 0x34
// One objective of the mission results.
struct FlintMark0x34 {
	MechS32 m_unk0x00;        // 0x00 — 0 failed, 1 successful
	MechS32 m_unk0x04;        // 0x04 — type: 1 primary, 2 secondary, 4 tertiary, 8 return
	undefined4 m_unk0x08;     // 0x08
	MechS32 m_unk0x0c;        // 0x0c — time in seconds, negative when never reached
	undefined4 m_unk0x10;     // 0x10
	MechChar m_unk0x14[0x20]; // 0x14 — description
};

// SIZE 0x9d4
// The simulator's mission results (MW2MSN.CFG).
struct SlateLedger0x9d4 {
	undefined4 m_unk0x00;        // 0x00
	MechS32 m_unk0x04;           // 0x04 — objectives
	undefined4 m_unk0x08;        // 0x08
	undefined4 m_unk0x0c;        // 0x0c
	MechS32 m_unk0x10;           // 0x10 — outcome, 2 for a completed mission
	FlintMark0x34 m_unk0x14[48]; // 0x14
};

DECOMP_SIZE_ASSERT(FlintMark0x34, 0x34)
DECOMP_SIZE_ASSERT(SlateLedger0x9d4, 0x9d4)

extern TinWhistle0x3c* g_pCurrentPilot;
extern CampaignMission* g_campaignMissions[2];
extern undefined g_unk0x100716b8[0x17];

// Returns TRUE when one of the options that makes a trial easier is set.
// FUNCTION: MW2SHELL 0x10001000
MechU8 FUN_10001000()
{
	if (g_unk0x100716b8[0] == 1) {
		return TRUE;
	}
	if (g_unk0x100716b8[1] == 1) {
		return TRUE;
	}
	if (!g_unk0x100716b8[3]) {
		return TRUE;
	}

	return FALSE;
}

// The qsort order of the debriefing's objectives: by the time they were reached, the ones never
// reached last.
// Not 100%: the stack slots of a, b, first and second are permuted.
// FUNCTION: MW2SHELL 0x10001056
int FUN_10001056(const void* p_a, const void* p_b)
{
	FlintMark0x34** a = (FlintMark0x34**) p_a;
	FlintMark0x34** b = (FlintMark0x34**) p_b;
	FlintMark0x34* first = *a;
	FlintMark0x34* second = *b;

	if (first->m_unk0x0c < 0) {
		return 1;
	}
	if (second->m_unk0x0c < 0) {
		return -1;
	}
	if (second->m_unk0x0c < first->m_unk0x0c) {
		return 1;
	}
	if (second->m_unk0x0c > first->m_unk0x0c) {
		return -1;
	}

	return 0;
}

// Copies p_src to p_dst with every run of spaces and control characters turned into one space.
// FUNCTION: MW2SHELL 0x10001c28
void FUN_10001c28(MechChar* p_dst, MechChar* p_src)
{
	MechS32 space = FALSE;

	while (*p_src) {
		if (*p_src <= ' ') {
			if (!space) {
				*p_dst++ = ' ';
				space = TRUE;
			}
		}
		else {
			space = FALSE;
			*p_dst++ = *p_src;
		}
		p_src++;
	}
	*p_dst = '\0';
}

// Reads the simulator's mission results.
// FUNCTION: MW2SHELL 0x100021a6
void ReadMissionResults(void* p_results)
{
	FILE* file = NULL;

	file = fopen("MW2MSN.CFG", "rb");
	if (file == NULL) {
		return;
	}

	fread(p_results, 0x9d4, 1, file);
	fclose(file);
}

// Returns the rank a trial earns: its successful primary objectives, when a clan pilot has
// completed a trial without the options that make it easier (p_options, g_unk0x100716b8).
// Not 100%: the stack slots of count and i are permuted.
// FUNCTION: MW2SHELL 0x100023bf
MechS32 FUN_100023bf(SlateLedger0x9d4* p_results, undefined* p_options)
{
	MechS32 count;
	MechS32 i;

	if (g_pCurrentPilot->m_unk0x08 != 2 &&
		g_campaignMissions[g_pCurrentPilot->m_unk0x08][g_pCurrentPilot->m_mission].m_unk0x04 == 1 && !FUN_10001000() &&
		p_results->m_unk0x10 == 2 && p_options[4] == 1) {
		count = 0;
		for (i = 0; i < p_results->m_unk0x04; i++) {
			if (p_results->m_unk0x14[i].m_unk0x04 == 1 && p_results->m_unk0x14[i].m_unk0x00 == 1) {
				count++;
			}
		}

		return count;
	}

	return 0;
}

// STUB: MW2SHELL 0x100024a3
void DrawMissionDebrief(TMPackDataBase* p_database, MechS32 p_campaign, char** p_scenario)
{
	STUB(0x100024a3);
}
