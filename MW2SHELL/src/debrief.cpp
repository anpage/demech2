#include "debrief.h"

#include "archivereader.h"
#include "buttonmenu.h"
#include "campaignmission.h"
#include "collection.h"
#include "customstar.h"
#include "decomp.h"
#include "font.h"
#include "keyboardinput.h"
#include "mainmenubutton.h"
#include "mechbay.h"
#include "mechchassis.h"
#include "mechvariant.h"
#include "menudata.h"
#include "menuscreen.h"
#include "mousestate.h"
#include "options.h"
#include "page.h"
#include "pilotrecord.h"
#include "pilotroster.h"
#include "refreshmode.h"
#include "shellglobals.h"
#include "shellmain.h"
#include "stringutil.h"
#include "textpages.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "video.h"
#include "videodriver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// SIZE 0x34
// One objective of the mission results.
struct MissionObjective {
	MechS32 m_unk0x00;        // 0x00 — 0 failed, 1 successful
	MechS32 m_unk0x04;        // 0x04 — type: 1 primary, 2 secondary, 4 tertiary, 8 return
	undefined4 m_unk0x08;     // 0x08
	MechS32 m_unk0x0c;        // 0x0c — time in seconds, negative when never reached
	undefined4 m_unk0x10;     // 0x10
	MechChar m_unk0x14[0x20]; // 0x14 — description
};

// SIZE 0x9d4
// The simulator's mission results (MW2MSN.CFG).
struct MissionResults {
	undefined4 m_unk0x00;           // 0x00
	MechS32 m_unk0x04;              // 0x04 — objectives
	undefined4 m_unk0x08;           // 0x08
	undefined4 m_unk0x0c;           // 0x0c
	MechS32 m_unk0x10;              // 0x10 — outcome, 2 for a completed mission
	MissionObjective m_unk0x14[48]; // 0x14
};

#pragma pack(1)
// SIZE 0x50
// The simulator's career record (MW2CAR.CFG): the last mission's statistics.
struct CareerRecord {
	undefined m_unk0x00[0x07 - 0x00]; // 0x00
	MechU16 m_unk0x07;                // 0x07 — enemy mechs destroyed
	undefined m_unk0x09[0x13 - 0x09]; // 0x09
	MechU16 m_unk0x13;                // 0x13 — shots fired
	MechU16 m_unk0x15;                // 0x15 — hits
	undefined m_unk0x17[0x1e - 0x17]; // 0x17
	MechU16 m_unk0x1e;                // 0x1e — enemy mechs destroyed by the player
	undefined m_unk0x20[0x34 - 0x20]; // 0x20
	MechU16 m_unk0x34;                // 0x34 — wingmen lost
	undefined m_unk0x36[0x44 - 0x36]; // 0x36
	MechU16 m_unk0x44;                // 0x44 — enemy vehicles destroyed
	undefined m_unk0x46[0x4a - 0x46]; // 0x46
	MechU16 m_unk0x4a;                // 0x4a — enemy vehicles destroyed by the player
	undefined m_unk0x4c[0x50 - 0x4c]; // 0x4c
};
#pragma pack()

DECOMP_SIZE_ASSERT(MissionObjective, 0x34)
DECOMP_SIZE_ASSERT(MissionResults, 0x9d4)
DECOMP_SIZE_ASSERT(CareerRecord, 0x50)

// The debriefing screen.
// GLOBAL: MW2SHELL 0x1005b040
ButtonMenu* g_unk0x1005b040 = NULL;

// GLOBAL: MW2SHELL 0x1005b044
Page* g_unk0x1005b044 = NULL;

// GLOBAL: MW2SHELL 0x1005b048
Collection* g_unk0x1005b048 = NULL;

// The aftermath reader, while it is open.
// GLOBAL: MW2SHELL 0x1005b04c
ArchiveReader* g_unk0x1005b04c = NULL;

// The debriefing's text buffers.
// GLOBAL: MW2SHELL 0x10076860
MechChar g_unk0x10076860[0x80];

// GLOBAL: MW2SHELL 0x100768e0
MechChar g_unk0x100768e0[0x1000];

// The mission's objectives, in the order CompareObjectives sorts them.
// GLOBAL: MW2SHELL 0x100778e0
MissionObjective* g_unk0x100778e0[48];

// GLOBAL: MW2SHELL 0x100779a0
MechChar g_unk0x100779a0[0x400];

// GLOBAL: MW2SHELL 0x10077da0
MechChar g_unk0x10077da0[0x200];

// The pilot as the mission found them, restored by a replay.
// GLOBAL: MW2SHELL 0x10077fa0
PilotRecord g_unk0x10077fa0;

// GLOBAL: MW2SHELL 0x10077fe0
undefined g_unk0x10077fe0[0x100];

// GLOBAL: MW2SHELL 0x100780e0
MissionResults g_unk0x100780e0;

// GLOBAL: MW2SHELL 0x10078ab8
MechChar g_unk0x10078ab8[0x80];

// GLOBAL: MW2SHELL 0x10078b38
MechChar g_unk0x10078b38[0x200];

// GLOBAL: MW2SHELL 0x10078d38
MechChar g_unk0x10078d38[0x80];

// GLOBAL: MW2SHELL 0x10078db8
MechChar g_unk0x10078db8[0x200];

// GLOBAL: MW2SHELL 0x10078fb8
MechChar g_unk0x10078fb8[0x200];

// GLOBAL: MW2SHELL 0x100791b8
MechChar g_unk0x100791b8[0x200];

// GLOBAL: MW2SHELL 0x100793b8
MechChar g_unk0x100793b8[0x80];

void MissionDebriefCallback(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar** p_scenario, MechS32 p_msg);

// Returns TRUE when one of the options that makes a trial easier is set.
// FUNCTION: MW2SHELL 0x10001000
MechU8 HasEasyOptions()
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
int CompareObjectives(const void* p_a, const void* p_b)
{
	MissionObjective** a = (MissionObjective**) p_a;
	MissionObjective** b = (MissionObjective**) p_b;
	MissionObjective* first = *a;
	MissionObjective* second = *b;

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

// Appends the honor breakdown of the mission to p_text and returns the honor it earns.
// Not 100%: the stack slots of the locals are permuted.
// FUNCTION: MW2SHELL 0x100010ed
MechS32 AppendHonorBreakdown(undefined* p_options, CareerRecord* p_career, MissionResults* p_results, MechChar* p_text)
{
	MechS32 honor = 0;
	MissionObjective* objective = NULL;
	MechS32 points = 5000;
	MechS32 i;
	MechS32 width;
	MechS32 secondary;
	MechS32 tertiary;
	CustomStar* star;
	MechS32 tons;
	MechDouble hit;
	MechS32 hitBonus;
	MechDouble multiplier;
	MechS32 bonus;

	for (i = 0; i < p_results->m_unk0x04; i++) {
		objective = &p_results->m_unk0x14[i];
		if (objective->m_unk0x04 == 1 && objective->m_unk0x00 == 0) {
			points = 0;
		}
	}

	sprintf(g_unk0x10078b38, "%d", points);
	width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
	sprintf(g_unk0x100791b8, "\\nMission Completion:\\g%03d\\b%03d%s\\n", 350, width, g_unk0x10078b38);
	strcat(p_text, g_unk0x100791b8);
	honor += points;

	secondary = 0;
	tertiary = 0;
	for (i = 0; i < p_results->m_unk0x04; i++) {
		objective = &p_results->m_unk0x14[i];
		switch (objective->m_unk0x04) {
		case 2:
			if (objective->m_unk0x00 == 1) {
				secondary++;
			}
			break;
		case 4:
			if (objective->m_unk0x00 == 1) {
				tertiary++;
			}
			break;
		default:
			break;
		}
	}

	if (secondary > 0) {
		points = secondary * 1500;
		honor += points;
		sprintf(g_unk0x10078b38, "%d", points);
		width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
		sprintf(
			g_unk0x100791b8,
			"Secondary Objective Completed:\\t\\t%d\\t(x%d)\\g%03d\\b%03d%s\\n",
			1500,
			secondary,
			350,
			width,
			g_unk0x10078b38
		);
		strcat(p_text, g_unk0x100791b8);
	}

	if (tertiary > 0) {
		points = tertiary * 500;
		honor += points;
		sprintf(g_unk0x10078b38, "%d", points);
		width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
		sprintf(
			g_unk0x100791b8,
			"Tertiary Objective Completed:\\t\\t%d\\t(x%d)\\g%03d\\b%03d%s\\n",
			500,
			tertiary,
			350,
			width,
			g_unk0x10078b38
		);
		strcat(p_text, g_unk0x100791b8);
	}

	if (p_career->m_unk0x34 > 0 && p_results->m_unk0x10 == 2) {
		points = p_career->m_unk0x34 * -4000;
		honor += points;
		sprintf(g_unk0x10078b38, "%d", points);
		width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
		sprintf(
			g_unk0x100791b8,
			"Wingman Deaths:\\t\\t\\t\\t%d\\t(x%d)\\g%03d\\b%03d%s\\n",
			-4000,
			p_career->m_unk0x34,
			350,
			width,
			g_unk0x10078b38
		);
		strcat(p_text, g_unk0x100791b8);
	}

	sprintf(g_unk0x100791b8, "\\t\\t\\t\\t\\t\\tdirect\\ttotal\\n");
	strcat(p_text, g_unk0x100791b8);

	points = p_career->m_unk0x1e * 250;
	g_pCurrentPilot->m_unk0x18 += p_career->m_unk0x1e;
	sprintf(g_unk0x10078b38, "%d", points);
	width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
	honor += points;
	sprintf(
		g_unk0x100791b8,
		"Enemy Mechs Destroyed:\\t\\t\\t%d\\t%d\\g%03d\\b%03d%s\\n",
		p_career->m_unk0x07,
		p_career->m_unk0x1e,
		350,
		width,
		g_unk0x10078b38
	);
	strcat(p_text, g_unk0x100791b8);

	points = p_career->m_unk0x4a * 125;
	g_pCurrentPilot->m_unk0x18 += p_career->m_unk0x44;
	sprintf(g_unk0x10078b38, "%d", points);
	width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
	honor += points;
	sprintf(
		g_unk0x100791b8,
		"Enemy Vehicles Destoyed:\\t\\t\\t%d\\t%d\\g%03d\\b%03d%s\\n",
		p_career->m_unk0x44,
		p_career->m_unk0x4a,
		350,
		width,
		g_unk0x10078b38
	);
	strcat(p_text, g_unk0x100791b8);

	star = GetStar(0);
	tons = star->m_size * star->m_tonnage;
	for (i = 0; i < star->m_count; i++) {
		tons -= g_mechChassis[star->m_mechs[i].m_chassis].m_tonnage;
	}

	if (tons > 0) {
		points = tons * 25;
		sprintf(g_unk0x10078b38, "%d", points);
		width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
		honor += points;
		sprintf(
			g_unk0x100791b8,
			"Star Underweight Bonus:\\t\\t\\t%d\\t(x%d tons)\\g%03d\\b%03d%s\\n",
			25,
			tons,
			350,
			width,
			g_unk0x10078b38
		);
		strcat(p_text, g_unk0x100791b8);
	}

	if (p_career->m_unk0x13 == 0) {
		hit = 0.0;
	}
	else {
		hit = (MechDouble) (MechU32) p_career->m_unk0x15 / (MechDouble) (MechU32) p_career->m_unk0x13;
	}

	if (hit >= 0.7) {
		hitBonus = 750;
	}
	else {
		hitBonus = 0;
	}

	if (hit <= 1.0) {
		sprintf(g_unk0x10078b38, "%d", hitBonus);
		width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
		sprintf(
			g_unk0x100791b8,
			"Hit Percentage:\\t\\t\\t\\t%3.1f\\g%03d\\b%03d%s\\n",
			hit * 100.0,
			350,
			width,
			g_unk0x10078b38
		);
		strcat(p_text, g_unk0x100791b8);
		honor += hitBonus;
	}

	g_pCurrentPilot->m_unk0x1c += p_career->m_unk0x15;
	g_pCurrentPilot->m_unk0x20 += p_career->m_unk0x13;

	switch (p_options[5]) {
	case 0:
		multiplier = 0.8;
		strcpy(g_unk0x10078db8, "EASY");
		break;
	case 1:
		multiplier = 1.0;
		strcpy(g_unk0x10078db8, "MEDIUM");
		break;
	case 2:
		multiplier = 1.3;
		strcpy(g_unk0x10078db8, "HARD");
		break;
	}

	if (!p_options[4]) {
		strcpy(g_unk0x100791b8, "\\n\\cThe Keshik deems it Dishonorable to Alter Heat Tracking\\n");
		strcat(p_text, g_unk0x100791b8);
		honor = 0;
	}
	else if (p_results->m_unk0x10 == 2) {
		sprintf(g_unk0x10078b38, "%d", honor);
		width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
		sprintf(g_unk0x100791b8, "\\nMission Honor:\\g%03d\\b%03d%s\\n", 350, width, g_unk0x10078b38);
		strcat(p_text, g_unk0x100791b8);

		bonus = (MechS32) (honor * multiplier) - honor;
		honor += bonus;
		sprintf(g_unk0x10078b38, "%d", honor);
		width = g_bodyFont->GetTextWidth(g_unk0x10078b38);
		sprintf(
			g_unk0x100791b8,
			"Difficulty Multiplier:\\t(%s = %1.1f)\\g%03d\\b%03d%s\\n",
			g_unk0x10078db8,
			multiplier,
			350,
			width,
			g_unk0x10078b38
		);
		strcat(p_text, g_unk0x100791b8);
	}

	if (HasEasyOptions() == 1) {
		strcpy(g_unk0x100791b8, "\\n\\cNo Career Advancement with Altered Reality Enabled\\n");
		strcat(p_text, g_unk0x100791b8);
		honor = 0;
	}

	if (p_results->m_unk0x10 == 3) {
		strcpy(g_unk0x100791b8, "\\n\\cMission Failed:  NO HONOR ACQUIRED\\n");
		strcat(p_text, g_unk0x100791b8);
		honor = 0;
	}

	return honor;
}

// Copies p_src to p_dst with every run of spaces and control characters turned into one space.
// FUNCTION: MW2SHELL 0x10001c28
void CollapseWhitespace(MechChar* p_dst, MechChar* p_src)
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

// Builds the debriefing's text in p_text: the objectives, sorted by time, and the honor
// breakdown, whose honor a completed mission adds to the pilot's.
// Not 100%: the stack slots of i, seconds, minutes, honor and width are permuted.
// FUNCTION: MW2SHELL 0x10001ca0
void BuildDebriefText(CareerRecord* p_career, MissionResults* p_results, MechChar* p_text, undefined* p_options)
{
	MechS32 i;
	MechS32 time;
	MechS32 seconds;
	MechS32 minutes;
	MechS32 honor;
	MechS32 width;

	strcpy(p_text, "");

	g_unk0x10077fe0[0] = 0xff;
	for (i = 1; i < 0x100; i++) {
		g_unk0x10077fe0[i] = i;
	}

	for (i = 0; i < p_results->m_unk0x04; i++) {
		g_unk0x100778e0[i] = &p_results->m_unk0x14[i];
	}
	qsort(g_unk0x100778e0, p_results->m_unk0x04, sizeof(MissionObjective*), CompareObjectives);

	strcat(p_text, "Time\\g050Type\\g170Objective\\g370Status\\n\\n");
	for (i = 0; i < p_results->m_unk0x04; i++) {
		switch (g_unk0x100778e0[i]->m_unk0x04) {
		case 0:
			strcpy(g_unk0x10078d38, "Default Objective");
			break;
		case 1:
			strcpy(g_unk0x10078d38, "Primary Objective");
			break;
		case 2:
			strcpy(g_unk0x10078d38, "Secondary Objective");
			break;
		case 4:
			strcpy(g_unk0x10078d38, "Tertiary Objective");
			break;
		case 8:
			strcpy(g_unk0x10078d38, "Return Objective");
			break;
		default:
			strcpy(g_unk0x10078d38, "Unknown");
			break;
		}

		switch (g_unk0x100778e0[i]->m_unk0x00) {
		case 1:
			strcpy(g_unk0x10076860, "Successful");
			break;
		case 0:
			strcpy(g_unk0x10076860, "Failed");
			break;
		default:
			strcpy(g_unk0x10076860, "Unknown");
			break;
		}

		CollapseWhitespace(g_unk0x10078ab8, g_unk0x100778e0[i]->m_unk0x14);

		if (g_unk0x100778e0[i]->m_unk0x0c < 0) {
			strcpy(g_unk0x100793b8, "DNF");
		}
		else {
			time = g_unk0x100778e0[i]->m_unk0x0c;
			seconds = time % 60;
			time /= 60;
			minutes = time % 60;
			time /= 60;
			time %= 24;
			sprintf(g_unk0x100793b8, "%02d:%02d", minutes, seconds);
		}

		sprintf(
			g_unk0x100779a0,
			"%s\\g050%s\\g170%s\\g370%s\\n",
			g_unk0x100793b8,
			g_unk0x10078d38,
			g_unk0x10078ab8,
			g_unk0x10076860
		);
		strcat(p_text, g_unk0x100779a0);
	}

	honor = AppendHonorBreakdown(p_options, p_career, p_results, p_text);
	if (p_results->m_unk0x10 == 2) {
		g_pCurrentPilot->m_honor += honor;
	}

	sprintf(g_unk0x10077da0, "%d", g_pCurrentPilot->m_honor);
	width = g_bodyFont->GetTextWidth(g_unk0x10077da0);
	sprintf(g_unk0x10078fb8, "\\nCareer Honor:\\g%03d\\b%03d%s\\n", 350, width, g_unk0x10077da0);
	strcat(p_text, g_unk0x10078fb8);
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

// Lays out the debriefing's text on pages, under the scenario's debriefing project (its first four
// letters and DBFS after a completed mission, DBFF otherwise; a pilot of the highest rank gets
// the clan's own).
// FUNCTION: MW2SHELL 0x10002207
void LayoutDebriefPages(
	MissionResults* p_results,
	MechChar* p_name,
	ButtonMenu*,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height,
	Collection* p_pages,
	MechChar* p_scenario,
	MechChar* p_text,
	MechS32 p_campaign
)
{
	MechS32 i;

	for (i = 0; i < 4 && p_scenario[i] && p_scenario[i] != '.'; i++) {
		p_name[i] = p_scenario[i];
	}
	while (i < 4) {
		p_name[i] = '_';
		i++;
	}
	p_name[i] = '\0';

	switch (p_results->m_unk0x10) {
	case 2:
		if (g_pCurrentPilot->m_rank >= 8) {
			switch (p_campaign) {
			case 0:
				strcpy(p_name, "KTWO");
				break;
			case 1:
				strcpy(p_name, "KTJF");
				break;
			}
		}
		strcat(p_name, "DBFS");
		break;
	default:
		strcat(p_name, "DBFF");
		break;
	}

	UppercaseString(p_name);
	LoadTextPages(p_pages, p_left, p_top, p_width, p_height, p_name, g_bodyFont, p_text);
}

// Returns the rank a trial earns: its successful primary objectives, when a clan pilot has
// completed a trial without the options that make it easier (p_options, g_unk0x100716b8).
// Not 100%: the stack slots of count and i are permuted.
// FUNCTION: MW2SHELL 0x100023bf
MechS32 GetTrialRank(MissionResults* p_results, undefined* p_options)
{
	MechS32 count;
	MechS32 i;

	if (g_pCurrentPilot->m_unk0x08 != 2 &&
		g_campaignMissions[g_pCurrentPilot->m_unk0x08][g_pCurrentPilot->m_mission].m_unk0x04 == 1 &&
		!HasEasyOptions() && p_results->m_unk0x10 == 2 && p_options[4] == 1) {
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

// Opens the debriefing screen: reads the mission's results and statistics, scores them for the
// pilot and lays the text out on pages under the menu.
// Not 100%: the stack slots of file, left, top, width, height, career and name are permuted.
// FUNCTION: MW2SHELL 0x100024a3
void DrawMissionDebrief(TMPackDataBase* p_database, MechS32 p_campaign, char** p_scenario)
{
	FILE* file = NULL;
	MechS32 left;
	MechS32 top;
	MechS32 width;
	MechS32 height;
	CareerRecord career;
	MechChar name[0x10];

	g_pVideoDriver->LoadBackground(p_database, g_unk0x1006ff00[p_campaign].m_picture);
	switch (p_campaign) {
	case 0:
		left = 0x58;
		top = 0x1e;
		width = 0x1c6;
		height = 430 - top;
		break;
	case 1:
		left = 0x62;
		top = 0x33;
		width = 0x193;
		height = 423 - top;
		break;
	default:
		left = 0x58;
		top = 0x1e;
		width = 0x1c6;
		height = 438 - top;
		break;
	}

	LoadPilotRoster();
	file = fopen("MW2CAR.CFG", "rb");
	if (file) {
		fread(&career, 0x50, 1, file);
		fclose(file);
	}
	ReadMissionResults(&g_unk0x100780e0);

	CreateCollection(&g_unk0x1005b048, 10, NULL, 4, NULL);
	g_keyboardInput->FlushKeys();
	g_unk0x1005b040 = new ButtonMenu(
		g_pVideoDriver,
		g_defaultFont,
		FALSE,
		g_unk0x1006ff00[p_campaign].m_buttons,
		g_unk0x1006ff00[p_campaign].m_count
	);

	if (p_campaign != 2) {
		g_unk0x10077fa0 = *g_pCurrentPilot;
	}
	BuildDebriefText(&career, &g_unk0x100780e0, g_unk0x100768e0, g_unk0x100716b8);

	if (p_campaign != 2) {
		g_pCurrentPilot->m_rank += GetTrialRank(&g_unk0x100780e0, g_unk0x100716b8);
		if (g_pCurrentPilot->m_rank >= 8) {
			g_pCurrentPilot->m_rank = 8;
		}
		if (g_unk0x100780e0.m_unk0x10 == 2 && !HasEasyOptions()) {
			g_pCurrentPilot->m_mission++;
		}
		SavePilotRoster();
		LayoutDebriefPages(
			&g_unk0x100780e0,
			name,
			g_unk0x1005b040,
			left,
			top,
			width,
			height,
			g_unk0x1005b048,
			*p_scenario,
			g_unk0x100768e0,
			p_campaign
		);
	}

	g_unk0x1005b044 = (Page*) CollectionGet(g_unk0x1005b048, 0);
	if (!g_unk0x1005b044) {
		g_unk0x1005b044 = new Page(g_bodyFont, g_pVideoDriver, NULL, 0, 0, 100, 100);
	}
	else {
		CollectionRemove(g_unk0x1005b048, g_unk0x1005b044, FALSE);
	}

	if (!g_unk0x1005b048->m_count || g_unk0x100780e0.m_unk0x10 != 2) {
		g_unk0x1005b040->DisableButton(1);
	}
	g_unk0x1005b044->Restart();
	RegisterScreenFunction(MissionDebriefCallback);
}

// The debriefing screen's frame: EXIT (on to the next mission once one is completed),
// AFTERMATH (the text in a reader) and REPLAY (restores the pilot).
// FUNCTION: MW2SHELL 0x1000287f
void MissionDebriefCallback(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar** p_scenario, MechS32 p_msg)
{
	MechS32 result;
	MechS32 button;

	// The original skips the frame's work with a goto, like StarConfigCallback.
	if (p_msg != 0x404) {
		goto done;
	}

	if (!g_unk0x1005b04c) {
		g_unk0x1005b044->TypeStep();
		button = g_unk0x1005b040->HitTest(g_pMouseState->m_x, g_pMouseState->m_y);
		switch (button) {
		case 0:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			p_msg = 0x411;
			if (*p_campaign != 2 && g_unk0x100780e0.m_unk0x10 == 2) {
				if (g_pCurrentPilot->m_mission >= 16) {
					p_msg = 0x416;
				}
				else {
					*p_scenario = g_campaignMissions[*p_campaign][g_pCurrentPilot->m_mission].m_unk0x00;
				}
			}
			break;
		case 1:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			g_unk0x1005b044->Hide();
			delete g_unk0x1005b040;
			g_unk0x1005b04c = new ArchiveReader(
				"",
				g_archiveFont,
				-1,
				FALSE,
				NULL,
				g_unk0x1005b048,
				g_unk0x1006ff30[*p_campaign].m_buttons,
				g_unk0x1006ff30[*p_campaign].m_count
			);
			break;
		case 2:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			if (g_unk0x100780e0.m_unk0x10 == 2) {
				if (!ShowDialog("Are you Sure?#Yes|No", 1)) {
					*g_pCurrentPilot = g_unk0x10077fa0;
					SavePilotRoster();
					p_msg = 0x406;
				}
				else {
					UpdateVideos();
				}
			}
			else {
				p_msg = 0x406;
			}
			break;
		default:
			break;
		}
	}
	else {
		result = g_unk0x1005b04c->Run();
		if (result == 0x402) {
			p_msg = 0x402;
		}
		if (result == 0x40e) {
			p_msg = 0x40e;
		}
		if (result != 0x40b) {
			delete g_unk0x1005b04c;
			g_unk0x1005b04c = NULL;
			g_unk0x1005b040 = new ButtonMenu(
				g_pVideoDriver,
				g_defaultFont,
				FALSE,
				g_unk0x1006ff00[*p_campaign].m_buttons,
				g_unk0x1006ff00[*p_campaign].m_count
			);
			g_unk0x1005b044->Restart();
		}
	}

done:
	if (p_msg != 0x404) {
		delete g_unk0x1005b044;
		delete g_unk0x1005b040;
		if (g_unk0x1005b04c) {
			delete g_unk0x1005b04c;
		}
		g_unk0x1005b04c = NULL;
		g_pVideoDriver->ClearGlyphs(TRUE);
		PostMessage(g_pWnd, p_msg, 0x409, 0);
		UnregisterScreenFunction(MissionDebriefCallback);
	}
}
