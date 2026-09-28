#include "archivereader.h"
#include "brasslantern0x414.h"
#include "collection.h"
#include "decomp.h"
#include "hollowreed0x110.h"
#include "mainmenubutton.h"
#include "menulist0x10d.h"
#include "mousestate.h"
#include "page.h"
#include "tallowsign0x10.h"
#include "tinwhistle0x3c.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <string.h>
#include <windows.h>

// The mission briefing screen.

extern "C" HWND g_pWnd;
extern TinWhistle0x3c* g_pCurrentPilot;
extern VideoDriver* g_pVideoDriver;
extern MouseState* g_pMouseState;
extern HollowReed0x110* g_unk0x100711f8;
extern BrassLantern0x414* g_unk0x1007120c;
extern BrassLantern0x414* g_unk0x10071224;
extern BrassLantern0x414* g_unk0x10071228;

void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
void FUN_100108fd(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
extern "C" MechChar* FUN_100309b6(MechChar* p_string);
void FUN_1002e1b1(
	Collection* p_pages,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height,
	MechChar* p_name,
	BrassLantern0x414* p_font,
	MechChar* p_quote
);

// GLOBAL: MW2SHELL 0x10071cd8
MechChar g_unk0x10071cd8[0x04] = "";

// GLOBAL: MW2SHELL 0x10071cdc
MenuList0x10d* g_unk0x10071cdc = NULL;

// GLOBAL: MW2SHELL 0x10071ce0
Page* g_unk0x10071ce0 = NULL;

// GLOBAL: MW2SHELL 0x10071ce4
Collection* g_unk0x10071ce4 = NULL;

// The situation reader, while it is open.
// GLOBAL: MW2SHELL 0x10071ce8
ArchiveReader* g_unk0x10071ce8 = NULL;

// GLOBAL: MW2SHELL 0x100702b8
MechChar g_unk0x100702b8[0x08] = "<~ABORT";
// GLOBAL: MW2SHELL 0x100702c0
MechChar g_unk0x100702c0[0x0c] = "<~SITUATION";
// GLOBAL: MW2SHELL 0x100702cc
MechChar g_unk0x100702cc[0x0c] = "<~LAUNCH";
// GLOBAL: MW2SHELL 0x100702d8
MechChar g_unk0x100702d8[0x08] = "<~SKIP";
// GLOBAL: MW2SHELL 0x100702e0
MechChar g_unk0x100702e0[0x08] = "<~EXIT";
// GLOBAL: MW2SHELL 0x100702e8
MechChar g_unk0x100702e8[0x0c] = "<~PREV PAGE";
// GLOBAL: MW2SHELL 0x100702f4
MechChar g_unk0x100702f4[0x0c] = "<~NEXT PAGE";
// GLOBAL: MW2SHELL 0x10070300
MechChar g_unk0x10070300[0x04] = "";

// GLOBAL: MW2SHELL 0x1007067c
MechChar g_unk0x1007067c[0x08] = "<~ABORT";
// GLOBAL: MW2SHELL 0x10070684
MechChar g_unk0x10070684[0x0c] = "<~SITUATION";
// GLOBAL: MW2SHELL 0x10070690
MechChar g_unk0x10070690[0x0c] = "<~LAUNCH";
// GLOBAL: MW2SHELL 0x1007069c
MechChar g_unk0x1007069c[0x08] = "<~SKIP";
// GLOBAL: MW2SHELL 0x100706a4
MechChar g_unk0x100706a4[0x08] = "<~EXIT";
// GLOBAL: MW2SHELL 0x100706ac
MechChar g_unk0x100706ac[0x0c] = "<~PREV PAGE";
// GLOBAL: MW2SHELL 0x100706b8
MechChar g_unk0x100706b8[0x0c] = "<~NEXT PAGE";
// GLOBAL: MW2SHELL 0x100706c4
MechChar g_unk0x100706c4[0x04] = "";

// GLOBAL: MW2SHELL 0x1006e808
MainMenuButton g_unk0x1006e808[4] = {
	{430, 450, 529, 474, 480, 455, g_unk0x100702b8},
	{110, 450, 209, 474, 160, 455, g_unk0x100702c0},
	{270, 450, 369, 474, 320, 455, g_unk0x100702cc},
	{540, 450, 639, 474, 590, 455, g_unk0x100702d8},
};

// GLOBAL: MW2SHELL 0x1006e878
MainMenuButton g_unk0x1006e878[4] = {
	{110, 450, 209, 474, 160, 455, g_unk0x100702e0},
	{270, 450, 369, 474, 320, 455, g_unk0x100702e8},
	{430, 450, 529, 474, 480, 455, g_unk0x100702f4},
	{510, 550, 609, 574, 560, 555, g_unk0x10070300},
};

// GLOBAL: MW2SHELL 0x1006f188
MainMenuButton g_unk0x1006f188[4] = {
	{430, 450, 529, 474, 480, 455, g_unk0x1007067c},
	{110, 450, 209, 474, 160, 455, g_unk0x10070684},
	{270, 450, 369, 474, 320, 455, g_unk0x10070690},
	{540, 450, 639, 474, 590, 455, g_unk0x1007069c},
};

// GLOBAL: MW2SHELL 0x1006f1f8
MainMenuButton g_unk0x1006f1f8[4] = {
	{110, 450, 209, 474, 160, 455, g_unk0x100706a4},
	{270, 450, 369, 474, 320, 455, g_unk0x100706ac},
	{430, 450, 529, 474, 480, 455, g_unk0x100706b8},
	{510, 550, 609, 574, 560, 555, g_unk0x100706c4},
};

// The briefing screen of each campaign. SKIP (the fourth button) is dropped for every pilot
// but FERRARI.
// GLOBAL: MW2SHELL 0x1006ff60
TallowSign0x10 g_unk0x1006ff60[3] = {
	{g_unk0x1006e808, 4, 16, -1},
	{g_unk0x1006f188, 4, 23, -1},
	{g_unk0x1006e808, 4, 10, -1},
};

// The situation reader of each campaign.
// GLOBAL: MW2SHELL 0x1006ff90
TallowSign0x10 g_unk0x1006ff90[3] = {
	{g_unk0x1006e878, 4, 16, -1},
	{g_unk0x1006f1f8, 4, 23, -1},
	{g_unk0x1006e878, 4, 10, -1},
};

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
	g_unk0x10071cdc = new MenuList0x10d(
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
			g_unk0x10071cdc = new MenuList0x10d(
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
