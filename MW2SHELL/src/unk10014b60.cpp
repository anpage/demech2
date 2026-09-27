#include "brasslantern0x414.h"
#include "campaignmission.h"
#include "decomp.h"
#include "emberglyph0x3e.h"
#include "slatetab0x2c.h"
#include "tinwhistle0x3c.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

// The pilot roster screen of a clan hall: ten pilot slots, the selected pilot's record and the
// mission list.

extern BrassLantern0x414* g_unk0x10071210;
extern BrassLantern0x414* g_unk0x10071214;
extern BrassLantern0x414* g_unk0x10071218;
extern TinWhistle0x3c* g_pCurrentPilot;
extern MechChar* g_rankNames[9];
extern CampaignMission* g_campaignMissions[2];

// GLOBAL: MW2SHELL 0x1007cc98
MechS32 g_unk0x1007cc98;

// GLOBAL: MW2SHELL 0x1007cca0
MechChar g_unk0x1007cca0[0x100];

// The campaign's ten slots of g_pilotRoster.
// GLOBAL: MW2SHELL 0x1007cda8
TinWhistle0x3c* g_unk0x1007cda8[10];

// Stack-slot permutation: top and i.
// FUNCTION: MW2SHELL 0x10014b60
void FUN_10014b60()
{
	TinWhistle0x3c* pilot;
	MechS32 top;
	MechS32 i;

	top = 0x5c;
	for (i = 0; i < 10; i++, top += 0x23) {
		pilot = g_unk0x1007cda8[i];
		if (pilot->m_unk0x00 == 0) {
			continue;
		}

		if (pilot->m_glyph) {
			delete pilot->m_glyph;
		}
		pilot->m_glyph = g_unk0x10071214->FUN_1000544e(0x2a, top, pilot->m_callsign, NULL);
	}
}

// FUNCTION: MW2SHELL 0x10014c1e
void FUN_10014c1e()
{
	TinWhistle0x3c* pilot;
	MechS32 i;

	for (i = 0; i < 10; i++) {
		pilot = g_unk0x1007cda8[i];
		if (pilot->m_glyph) {
			delete pilot->m_glyph;
			pilot->m_glyph = NULL;
		}
	}
}

// FUNCTION: MW2SHELL 0x10014caa
void FUN_10014caa(TinWhistle0x3c* p_pilot)
{
	p_pilot->m_unk0x00 = 0;
	strcpy(p_pilot->m_callsign, "");
	if (p_pilot->m_glyph) {
		delete p_pilot->m_glyph;
		p_pilot->m_glyph = NULL;
	}
}

// FUNCTION: MW2SHELL 0x10014d3e
void FUN_10014d3e(TinWhistle0x3c* p_pilot)
{
	MechS32 i;

	for (i = 0; i < 10; i++) {
		g_unk0x1007cda8[i]->m_unk0x04 = 0;
	}
	p_pilot->m_unk0x04 = 1;
}

// FUNCTION: MW2SHELL 0x10014d8a
EmberGlyph0x3e* FUN_10014d8a(SlateTab0x2c* p_tab)
{
	MechChar* text = (MechChar*) p_tab->m_unk0x24;

	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, text, NULL);
}

// FUNCTION: MW2SHELL 0x10014dc4
EmberGlyph0x3e* FUN_10014dc4(SlateTab0x2c* p_tab)
{
	MechChar* text = (MechChar*) p_tab->m_unk0x24;

	return g_unk0x10071218->FUN_1000544e(p_tab->m_left, p_tab->m_top, text, NULL);
}

// The mission list: the tab's data is the mission index. Missions the pilot hasn't reached stay blank.
// The original loads the mission index ahead of the campaign's table; reordering the declarations
// doesn't flip it.
// FUNCTION: MW2SHELL 0x10014dfe
EmberGlyph0x3e* FUN_10014dfe(SlateTab0x2c* p_tab)
{
	if ((MechS32) p_tab->m_unk0x24 >= g_pCurrentPilot->m_mission) {
		return NULL;
	}

	sprintf(g_unk0x1007cca0, "~%s", g_campaignMissions[g_unk0x1007cc98][(MechS32) p_tab->m_unk0x24].m_title);
	return g_unk0x10071210->FUN_1000544e(p_tab->m_left + p_tab->m_width / 2, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// The pilot record callbacks below open with a test of p_tab that does nothing.

// FUNCTION: MW2SHELL 0x10014e83
EmberGlyph0x3e* FUN_10014e83(SlateTab0x2c* p_tab)
{
	if (p_tab) {
	}

	sprintf(g_unk0x1007cca0, "~%s", g_pCurrentPilot->m_callsign);
	return g_unk0x10071214->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// FUNCTION: MW2SHELL 0x10014ed7
EmberGlyph0x3e* FUN_10014ed7(SlateTab0x2c* p_tab)
{
	if (p_tab) {
	}

	sprintf(g_unk0x1007cca0, "~%s", g_rankNames[g_pCurrentPilot->m_rank]);
	return g_unk0x10071214->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// FUNCTION: MW2SHELL 0x10014f32
EmberGlyph0x3e* FUN_10014f32(SlateTab0x2c* p_tab)
{
	if (p_tab) {
	}

	sprintf(g_unk0x1007cca0, "~%d", g_pCurrentPilot->m_honor);
	return g_unk0x10071214->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// FUNCTION: MW2SHELL 0x10014f86
EmberGlyph0x3e* FUN_10014f86(SlateTab0x2c* p_tab)
{
	if (p_tab) {
	}

	sprintf(g_unk0x1007cca0, "~%s", g_campaignMissions[g_unk0x1007cc98][g_pCurrentPilot->m_mission].m_title);
	return g_unk0x10071214->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// The mission list's click callback.
// FUNCTION: MW2SHELL 0x10014fee
void FUN_10014fee(SlateTab0x2c* p_tab)
{
	if (p_tab) {
	}
}

// Negative m_top: rows (times the previous tab's height) and an offset below the previous tab.
#define TAB_BELOW(rows, offset) ((MechS32) (0x80000000 | ((rows) << 4) | (offset)))
#define ROSTER_TAB(x, y, width, draw, click, data) {x, y, width, -1, 0, NULL, NULL, draw, click, (void*) (data), NULL}
#define ROSTER_END {-1, 0, 0, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL}

// The selected pilot's record.
// GLOBAL: MW2SHELL 0x10063c78
SlateTab0x2c g_unk0x10063c78[8] = {
	ROSTER_TAB(0x1d4, 0x5c, -1, FUN_10014e83, NULL, NULL),
	ROSTER_TAB(0x1d4, 0xd1, -1, FUN_10014dc4, NULL, "~RANK"),
	ROSTER_TAB(0x1d4, TAB_BELOW(1, 3), -1, FUN_10014ed7, NULL, NULL),
	ROSTER_TAB(0x1d4, TAB_BELOW(2, 2), -1, FUN_10014dc4, NULL, "~HONOR"),
	ROSTER_TAB(0x1d4, TAB_BELOW(1, 3), -1, FUN_10014f32, NULL, NULL),
	ROSTER_TAB(0x1d4, TAB_BELOW(2, 2), -1, FUN_10014dc4, NULL, "~MISSION"),
	ROSTER_TAB(0x1d4, TAB_BELOW(1, 3), -1, FUN_10014f86, NULL, NULL),
	ROSTER_END,
};

#undef TAB_BELOW
#undef ROSTER_TAB
#undef ROSTER_END

// STUB: MW2SHELL 0x10015008
void FUN_10015008(TMPackDataBase* p_database, MechS32 p_campaign, MechU8* p_pilotChosen, char** p_scenario)
{
	STUB(0x10015008);
}
