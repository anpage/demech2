#include "unk1003e2f0.h"

#include "brasslantern0x414.h"
#include "campaignmission.h"
#include "decomp.h"
#include "hollowreed0x110.h"
#include "mousestate.h"
#include "shellmain.h"
#include "silverreel0x18.h"
#include "tinwhistle0x3c.h"
#include "types.h"
#include "unk10010a30.h"
#include "unk1003bf90.h"
#include "unk1006e150.h"
#include "unk100711f8.h"
#include "video.h"
#include "videodriver.h"

void operator delete(void*);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// FUNCTION: MW2SHELL 0x1003e2f0
MechS32 FUN_1003e2f0(const TinWhistle0x3c** p_first, const TinWhistle0x3c** p_second)
{
	const TinWhistle0x3c** firstParam = p_first;
	const TinWhistle0x3c** secondParam = p_second;
	const TinWhistle0x3c* first = *firstParam;
	const TinWhistle0x3c* second = *secondParam;

	if (second->m_rank < first->m_rank) {
		return -1;
	}
	if (second->m_rank > first->m_rank) {
		return 1;
	}
	if (second->m_honor < first->m_honor) {
		return -1;
	}
	if (second->m_honor > first->m_honor) {
		return 1;
	}
	if (second->m_mission < first->m_mission) {
		return -1;
	}
	if (second->m_mission > first->m_mission) {
		return 1;
	}

	return 0;
}

void FUN_1003e86b(MechS32 p_active);

// GLOBAL: MW2SHELL 0x1006aeac
SilverReel0x18* g_unk0x1006aeac = NULL;

// GLOBAL: MW2SHELL 0x10090670
MechChar g_unk0x10090670[0x20];

// Draw the eight highest-ranking active pilots from both clan rosters.
// FUNCTION: MW2SHELL 0x1003e3c9
void FUN_1003e3c9()
{
	TinWhistle0x3c* pilots[20];
	MechS32 i;
	MechS32 top;

	g_pVideoDriver->GetPalette(g_unk0x10071378);
	g_unk0x100711f8->FUN_100440ed();
	g_pVideoDriver->LoadPalette(2);
	g_pVideoDriver->m_unk0x3a6 = 0;
	g_unk0x1006aeac = new SilverReel0x18("amwlogo1", 0x78, 4);

	for (i = 0; i < 20; i++) {
		pilots[i] = &g_pilotRoster[i];
	}
	qsort(pilots, 20, sizeof(pilots[0]), (int (*)(const void*, const void*)) FUN_1003e2f0);

	top = 0x96;
	g_pVideoDriver->FUN_100074d2(0, top, g_unk0x10071218->m_unk0x408, "Pilot", NULL);
	g_pVideoDriver->FUN_100074d2(0x7d, top, g_unk0x10071218->m_unk0x408, "Clan", NULL);
	g_pVideoDriver->FUN_100074d2(200, top, g_unk0x10071218->m_unk0x408, "Rank", NULL);
	g_pVideoDriver->FUN_100074d2(0x145, top, g_unk0x10071218->m_unk0x408, "Honor", NULL);
	g_pVideoDriver->FUN_100074d2(400, top, g_unk0x10071218->m_unk0x408, "Kills", NULL);
	g_pVideoDriver->FUN_100074d2(0x1cc, top, g_unk0x10071218->m_unk0x408, "Hit %", NULL);
	g_pVideoDriver->FUN_100074d2(0x208, top, g_unk0x10071218->m_unk0x408, "Last Mission", NULL);
	top += 0x20;

	for (i = 0; i < 8; i++) {
		if (!pilots[i]->m_unk0x00) {
			continue;
		}

		g_pVideoDriver->FUN_100074d2(0, top, g_unk0x1007120c->m_unk0x408, pilots[i]->m_callsign, NULL);
		g_pVideoDriver
			->FUN_100074d2(0x7d, top, g_unk0x1007120c->m_unk0x408, g_unk0x10071280[pilots[i]->m_unk0x08], NULL);
		g_pVideoDriver->FUN_100074d2(200, top, g_unk0x1007120c->m_unk0x408, g_rankNames[pilots[i]->m_rank], NULL);
		sprintf(g_unk0x10090670, "%d", pilots[i]->m_honor);
		g_pVideoDriver->FUN_100074d2(0x145, top, g_unk0x1007120c->m_unk0x408, g_unk0x10090670, NULL);
		sprintf(g_unk0x10090670, "%d", pilots[i]->m_unk0x18);
		g_pVideoDriver->FUN_100074d2(400, top, g_unk0x1007120c->m_unk0x408, g_unk0x10090670, NULL);
		if (pilots[i]->m_unk0x20 != 0) {
			sprintf(g_unk0x10090670, "%d%%", (MechS32) pilots[i]->m_unk0x1c * 100 / (MechS32) pilots[i]->m_unk0x20);
		}
		else {
			strcpy(g_unk0x10090670, "-");
		}
		g_pVideoDriver->FUN_100074d2(0x1cc, top, g_unk0x1007120c->m_unk0x408, g_unk0x10090670, NULL);
		if (pilots[i]->m_mission == 0) {
			g_pVideoDriver->FUN_100074d2(0x208, top, g_unk0x1007120c->m_unk0x408, "----", NULL);
		}
		else {
			g_pVideoDriver->FUN_100074d2(
				0x208,
				top,
				g_unk0x1007120c->m_unk0x408,
				g_campaignMissions[pilots[i]->m_unk0x08][pilots[i]->m_mission - 1].m_title,
				NULL
			);
		}
		top += 0x10;
	}

	FUN_100109a0(FUN_1003e86b);
}

// FUNCTION: MW2SHELL 0x1003e86b
void FUN_1003e86b(MechS32 p_active)
{
	if (p_active && g_unk0x1006aeac != NULL) {
		g_unk0x1006aeac->FUN_1001630b();
	}
	if (!p_active || g_pMouseState->GetRightPressed() == 1 || g_pMouseState->GetLeftPressed() == 1 ||
		g_unk0x100711f8->FUN_10044189() != 0) {
		FUN_100109b8(FUN_1003e86b);
		EnableMenuItem(g_windowMenu, 0x9c42, MF_ENABLED);
		g_menuDialogOpen = FALSE;
		if (g_unk0x1006aeac != NULL) {
			delete g_unk0x1006aeac;
		}
		g_unk0x1006aeac = NULL;
		g_pVideoDriver->m_unk0x3a6 = -1;
		g_pVideoDriver->FUN_100071ad(0, 0, 0x280, 0x1e0);
		FUN_1001661b();
		g_pVideoDriver->SetPalette(g_unk0x10071378, TRUE);
		if (p_active) {
			g_pVideoDriver->DrawShell();
			g_pVideoDriver->FUN_100071ad(0, 0, 0x280, 0x1e0);
			FUN_1001661b();
		}
	}
}
