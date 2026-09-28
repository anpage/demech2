#include "briefing.h"

#include "archivereader.h"
#include "buttonmenu.h"
#include "collection.h"
#include "decomp.h"
#include "font.h"
#include "keyboardinput.h"
#include "mainmenubutton.h"
#include "menudata.h"
#include "menuscreen.h"
#include "mousestate.h"
#include "page.h"
#include "pilotrecord.h"
#include "refreshmode.h"
#include "shellglobals.h"
#include "shellmain.h"
#include "stringutil.h"
#include "textpages.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <string.h>
#include <windows.h>

// The mission briefing screen.

// GLOBAL: MW2SHELL 0x10071cd8
MechChar g_unk0x10071cd8[0x04] = "";

// GLOBAL: MW2SHELL 0x10071cdc
ButtonMenu* g_unk0x10071cdc = NULL;

// GLOBAL: MW2SHELL 0x10071ce0
Page* g_unk0x10071ce0 = NULL;

// GLOBAL: MW2SHELL 0x10071ce4
Collection* g_unk0x10071ce4 = NULL;

// The situation reader, while it is open.
// GLOBAL: MW2SHELL 0x10071ce8
ArchiveReader* g_unk0x10071ce8 = NULL;

void FUN_10046653(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar**, MechS32 p_msg);

// Opens the briefing screen: lays the scenario's briefing text (its first four letters, padded
// with '_', plus "BRF1") out on pages under the menu. A campaign's last mission briefs from
// KTWOBRF1/KTJFBRF1 once the pilot's rank is high enough.
// Not 100%: the stack slots of left, top, width, height and i are permuted.
// FUNCTION: MW2SHELL 0x10046200
void FUN_10046200(TMPackDataBase* p_database, char* p_scenario, MechS32 p_campaign)
{
	MechS32 left;
	MechS32 top;
	MechS32 width;
	MechS32 height;
	MechS32 i;
	MechChar name[0x80];

	CreateCollection(&g_unk0x10071ce4, 10, NULL, 4, NULL);
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

	if (g_pCurrentPilot->m_mission == 15 && g_pCurrentPilot->m_rank >= 6) {
		switch (p_campaign) {
		case 0:
			strcpy(name, "KTWOBRF1");
			break;
		case 1:
			strcpy(name, "KTJFBRF1");
			break;
		default:
			break;
		}
	}
	else {
		for (i = 0; i < 4 && p_scenario[i] && p_scenario[i] != '.'; i++) {
			name[i] = p_scenario[i];
		}
		while (i < 4) {
			name[i] = '_';
			i++;
		}
		name[i] = '\0';
		strcat(name, "BRF1");
	}

	FUN_100309b6(name);
	FUN_1002e1b1(g_unk0x10071ce4, left, top, width, height, name, g_unk0x10071228, g_unk0x10071cd8);

	g_unk0x10071ce0 = (Page*) CollectionGet(g_unk0x10071ce4, 0);
	if (!g_unk0x10071ce0) {
		PostMessage(g_pWnd, 0x410, 0x406, 0);
		return;
	}

	g_pVideoDriver->FUN_10006c50(p_database, g_unk0x1006ff60[p_campaign].m_picture);
	FUN_1003c3ba(g_unk0x10071ce4, g_unk0x10071ce0, FALSE);
	g_unk0x100711f8->FUN_100440ed();

	if (strcmp(g_pCurrentPilot->m_callsign, "FERRARI")) {
		g_unk0x1006ff60[p_campaign].m_count = 3;
	}
	g_unk0x10071cdc = new ButtonMenu(
		g_pVideoDriver,
		g_unk0x10071228,
		FALSE,
		g_unk0x1006ff60[p_campaign].m_buttons,
		g_unk0x1006ff60[p_campaign].m_count
	);

	if (!g_unk0x10071ce4->m_count) {
		g_unk0x10071cdc->FUN_10048d65(1);
	}
	g_unk0x10071ce0->FUN_1004596f();
	FUN_100108e5(FUN_10046653);
}

// The briefing screen's frame: ABORT, SITUATION (the text in a reader), LAUNCH and SKIP.
// FUNCTION: MW2SHELL 0x10046653
void FUN_10046653(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar**, MechS32 p_msg)
{
	MechS32 result;
	MechS32 button;

	// The original skips the frame's work with a goto, like FUN_100043c2.
	if (p_msg != 0x404) {
		goto done;
	}

	if (!g_unk0x10071ce8) {
		g_unk0x10071ce0->FUN_10045a2b();
		button = g_unk0x10071cdc->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
		switch (button) {
		case 2:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			p_msg = 0x410;
			break;
		case 0:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			p_msg = 0x411;
			break;
		case 3:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			p_msg = 0x409;
			break;
		case 1:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			g_unk0x10071ce0->FUN_10045ab0();
			delete g_unk0x10071cdc;
			g_unk0x10071ce8 = new ArchiveReader(
				"",
				g_unk0x10071224,
				-1,
				FALSE,
				NULL,
				g_unk0x10071ce4,
				g_unk0x1006ff90[*p_campaign].m_buttons,
				g_unk0x1006ff90[*p_campaign].m_count
			);
			break;
		default:
			break;
		}
	}
	else {
		result = g_unk0x10071ce8->Run();
		if (result == 0x402) {
			p_msg = 0x402;
		}
		if (result == 0x40e) {
			p_msg = 0x40e;
		}
		if (result != 0x40b) {
			delete g_unk0x10071ce8;
			g_unk0x10071ce8 = NULL;
			g_unk0x10071cdc = new ButtonMenu(
				g_pVideoDriver,
				g_unk0x1007120c,
				FALSE,
				g_unk0x1006ff60[*p_campaign].m_buttons,
				g_unk0x1006ff60[*p_campaign].m_count
			);
			g_unk0x10071ce0->FUN_1004596f();
		}
	}

done:
	if (p_msg != 0x404) {
		if (g_unk0x10071ce8) {
			delete g_unk0x10071ce8;
		}
		g_unk0x10071ce8 = NULL;
		delete g_unk0x10071ce0;
		delete g_unk0x10071cdc;
		PostMessage(g_pWnd, p_msg, 0x406, 0);
		FUN_100108fd(FUN_10046653);
	}
}
