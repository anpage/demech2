#include "hallofhonor.h"

#include "campaignmission.h"
#include "decomp.h"
#include "font.h"
#include "keyboardinput.h"
#include "loopingmovie.h"
#include "menudata.h"
#include "mousestate.h"
#include "pilotrecord.h"
#include "refreshmode.h"
#include "shellglobals.h"
#include "shellmain.h"
#include "types.h"
#include "unk1003bf90.h"
#include "video.h"
#include "videodriver.h"

void operator delete(void*);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// FUNCTION: MW2SHELL 0x1003e2f0
MechS32 ComparePilotRecords(const PilotRecord** p_first, const PilotRecord** p_second)
{
	const PilotRecord** firstParam = p_first;
	const PilotRecord** secondParam = p_second;
	const PilotRecord* first = *firstParam;
	const PilotRecord* second = *secondParam;

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

void HallOfHonorCallback(MechS32 p_active);

// GLOBAL: MW2SHELL 0x1006aeac
LoopingMovie* g_unk0x1006aeac = NULL;

// GLOBAL: MW2SHELL 0x10090670
MechChar g_unk0x10090670[0x20];

// Draw the eight highest-ranking active pilots from both clan rosters.
// FUNCTION: MW2SHELL 0x1003e3c9
void DrawHallOfHonor()
{
	PilotRecord* pilots[20];
	MechS32 i;
	MechS32 top;

	g_pVideoDriver->GetPalette(g_unk0x10071378);
	g_keyboardInput->FlushKeys();
	g_pVideoDriver->LoadPalette(2);
	g_pVideoDriver->m_unk0x3a6 = 0;
	g_unk0x1006aeac = new LoopingMovie("amwlogo1", 0x78, 4);

	for (i = 0; i < 20; i++) {
		pilots[i] = &g_pilotRoster[i];
	}
	qsort(pilots, 20, sizeof(pilots[0]), (int (*)(const void*, const void*)) ComparePilotRecords);

	top = 0x96;
	g_pVideoDriver->DrawString(0, top, g_buttonFont->m_unk0x408, "Pilot", NULL);
	g_pVideoDriver->DrawString(0x7d, top, g_buttonFont->m_unk0x408, "Clan", NULL);
	g_pVideoDriver->DrawString(200, top, g_buttonFont->m_unk0x408, "Rank", NULL);
	g_pVideoDriver->DrawString(0x145, top, g_buttonFont->m_unk0x408, "Honor", NULL);
	g_pVideoDriver->DrawString(400, top, g_buttonFont->m_unk0x408, "Kills", NULL);
	g_pVideoDriver->DrawString(0x1cc, top, g_buttonFont->m_unk0x408, "Hit %", NULL);
	g_pVideoDriver->DrawString(0x208, top, g_buttonFont->m_unk0x408, "Last Mission", NULL);
	top += 0x20;

	for (i = 0; i < 8; i++) {
		if (!pilots[i]->m_unk0x00) {
			continue;
		}

		g_pVideoDriver->DrawString(0, top, g_defaultFont->m_unk0x408, pilots[i]->m_callsign, NULL);
		g_pVideoDriver->DrawString(0x7d, top, g_defaultFont->m_unk0x408, g_unk0x10071280[pilots[i]->m_unk0x08], NULL);
		g_pVideoDriver->DrawString(200, top, g_defaultFont->m_unk0x408, g_rankNames[pilots[i]->m_rank], NULL);
		sprintf(g_unk0x10090670, "%d", pilots[i]->m_honor);
		g_pVideoDriver->DrawString(0x145, top, g_defaultFont->m_unk0x408, g_unk0x10090670, NULL);
		sprintf(g_unk0x10090670, "%d", pilots[i]->m_unk0x18);
		g_pVideoDriver->DrawString(400, top, g_defaultFont->m_unk0x408, g_unk0x10090670, NULL);
		if (pilots[i]->m_unk0x20 != 0) {
			sprintf(g_unk0x10090670, "%d%%", (MechS32) pilots[i]->m_unk0x1c * 100 / (MechS32) pilots[i]->m_unk0x20);
		}
		else {
			strcpy(g_unk0x10090670, "-");
		}
		g_pVideoDriver->DrawString(0x1cc, top, g_defaultFont->m_unk0x408, g_unk0x10090670, NULL);
		if (pilots[i]->m_mission == 0) {
			g_pVideoDriver->DrawString(0x208, top, g_defaultFont->m_unk0x408, "----", NULL);
		}
		else {
			g_pVideoDriver->DrawString(
				0x208,
				top,
				g_defaultFont->m_unk0x408,
				g_campaignMissions[pilots[i]->m_unk0x08][pilots[i]->m_mission - 1].m_title,
				NULL
			);
		}
		top += 0x10;
	}

	RegisterMenuFunction(HallOfHonorCallback);
}

// FUNCTION: MW2SHELL 0x1003e86b
void HallOfHonorCallback(MechS32 p_active)
{
	if (p_active && g_unk0x1006aeac != NULL) {
		g_unk0x1006aeac->Update();
	}
	if (!p_active || g_pMouseState->GetRightPressed() == 1 || g_pMouseState->GetLeftPressed() == 1 ||
		g_keyboardInput->PollKey() != 0) {
		UnregisterMenuFunction(HallOfHonorCallback);
		EnableMenuItem(g_windowMenu, 0x9c42, MF_ENABLED);
		g_menuDialogOpen = FALSE;
		if (g_unk0x1006aeac != NULL) {
			delete g_unk0x1006aeac;
		}
		g_unk0x1006aeac = NULL;
		g_pVideoDriver->m_unk0x3a6 = -1;
		g_pVideoDriver->RestoreBackground(0, 0, 0x280, 0x1e0);
		UpdateVideos();
		g_pVideoDriver->SetPalette(g_unk0x10071378, TRUE);
		if (p_active) {
			g_pVideoDriver->DrawShell();
			g_pVideoDriver->RestoreBackground(0, 0, 0x280, 0x1e0);
			UpdateVideos();
		}
	}
}
