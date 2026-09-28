#include "rosterscreen.h"

#include "audiosample.h"
#include "audiosubsystem.h"
#include "buttonmenu.h"
#include "campaignmission.h"
#include "decomp.h"
#include "font.h"
#include "keyboardinput.h"
#include "mainmenubutton.h"
#include "mechbay.h"
#include "mechvariant.h"
#include "menudata.h"
#include "menuscreen.h"
#include "missionui.h"
#include "mousestate.h"
#include "options.h"
#include "pilotrecord.h"
#include "pilotroster.h"
#include "refreshmode.h"
#include "screenfield.h"
#include "shellglobals.h"
#include "shellmain.h"
#include "stringutil.h"
#include "textglyph.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "unk1003bf90.h"
#include "videodriver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

// The pilot roster screen of a clan hall: ten pilot slots, the selected pilot's record and the
// mission list.

// GLOBAL: MW2SHELL 0x1007cc98
MechS32 g_unk0x1007cc98;

// GLOBAL: MW2SHELL 0x1007cca0
MechChar g_unk0x1007cca0[0x100];

// The campaign's ten slots of g_pilotRoster.
// GLOBAL: MW2SHELL 0x1007cda8
PilotRecord* g_unk0x1007cda8[10];

// Stack-slot permutation: top and i.
// FUNCTION: MW2SHELL 0x10014b60
void FUN_10014b60()
{
	PilotRecord* pilot;
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
	PilotRecord* pilot;
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
void FUN_10014caa(PilotRecord* p_pilot)
{
	p_pilot->m_unk0x00 = 0;
	strcpy(p_pilot->m_callsign, "");
	if (p_pilot->m_glyph) {
		delete p_pilot->m_glyph;
		p_pilot->m_glyph = NULL;
	}
}

// FUNCTION: MW2SHELL 0x10014d3e
void FUN_10014d3e(PilotRecord* p_pilot)
{
	MechS32 i;

	for (i = 0; i < 10; i++) {
		g_unk0x1007cda8[i]->m_unk0x04 = 0;
	}
	p_pilot->m_unk0x04 = 1;
}

// FUNCTION: MW2SHELL 0x10014d8a
TextGlyph* FUN_10014d8a(ScreenField* p_tab)
{
	MechChar* text = (MechChar*) p_tab->m_unk0x24;

	return g_unk0x10071210->FUN_1000544e(p_tab->m_left, p_tab->m_top, text, NULL);
}

// FUNCTION: MW2SHELL 0x10014dc4
TextGlyph* FUN_10014dc4(ScreenField* p_tab)
{
	MechChar* text = (MechChar*) p_tab->m_unk0x24;

	return g_unk0x10071218->FUN_1000544e(p_tab->m_left, p_tab->m_top, text, NULL);
}

// The mission list: the tab's data is the mission index. Missions the pilot hasn't reached stay blank.
// FUNCTION: MW2SHELL 0x10014dfe
TextGlyph* FUN_10014dfe(ScreenField* p_tab)
{
	if ((MechS32) p_tab->m_unk0x24 >= g_pCurrentPilot->m_mission) {
		return NULL;
	}

	sprintf(g_unk0x1007cca0, "~%s", g_campaignMissions[g_unk0x1007cc98][(MechS32) p_tab->m_unk0x24].m_title);
	return g_unk0x10071210->FUN_1000544e(p_tab->m_left + p_tab->m_width / 2, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// The pilot record callbacks below open with a test of p_tab that does nothing.

// FUNCTION: MW2SHELL 0x10014e83
TextGlyph* FUN_10014e83(ScreenField* p_tab)
{
	if (p_tab) {
	}

	sprintf(g_unk0x1007cca0, "~%s", g_pCurrentPilot->m_callsign);
	return g_unk0x10071214->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// FUNCTION: MW2SHELL 0x10014ed7
TextGlyph* FUN_10014ed7(ScreenField* p_tab)
{
	if (p_tab) {
	}

	sprintf(g_unk0x1007cca0, "~%s", g_rankNames[g_pCurrentPilot->m_rank]);
	return g_unk0x10071214->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// FUNCTION: MW2SHELL 0x10014f32
TextGlyph* FUN_10014f32(ScreenField* p_tab)
{
	if (p_tab) {
	}

	sprintf(g_unk0x1007cca0, "~%d", g_pCurrentPilot->m_honor);
	return g_unk0x10071214->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// FUNCTION: MW2SHELL 0x10014f86
TextGlyph* FUN_10014f86(ScreenField* p_tab)
{
	if (p_tab) {
	}

	sprintf(g_unk0x1007cca0, "~%s", g_campaignMissions[g_unk0x1007cc98][g_pCurrentPilot->m_mission].m_title);
	return g_unk0x10071214->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_unk0x1007cca0, NULL);
}

// The mission list's click callback.
// FUNCTION: MW2SHELL 0x10014fee
void FUN_10014fee(ScreenField* p_tab)
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
ScreenField g_unk0x10063c78[8] = {
	ROSTER_TAB(0x1d4, 0x5c, -1, FUN_10014e83, NULL, NULL),
	ROSTER_TAB(0x1d4, 0xd1, -1, FUN_10014dc4, NULL, "~RANK"),
	ROSTER_TAB(0x1d4, TAB_BELOW(1, 3), -1, FUN_10014ed7, NULL, NULL),
	ROSTER_TAB(0x1d4, TAB_BELOW(2, 2), -1, FUN_10014dc4, NULL, "~HONOR"),
	ROSTER_TAB(0x1d4, TAB_BELOW(1, 3), -1, FUN_10014f32, NULL, NULL),
	ROSTER_TAB(0x1d4, TAB_BELOW(2, 2), -1, FUN_10014dc4, NULL, "~MISSION"),
	ROSTER_TAB(0x1d4, TAB_BELOW(1, 3), -1, FUN_10014f86, NULL, NULL),
	ROSTER_END,
};

// The mission list.
// GLOBAL: MW2SHELL 0x10063dd8
ScreenField g_unk0x10063dd8[] = {
	ROSTER_TAB(0x1d4, 0x5c, -1, FUN_10014e83, NULL, NULL),
	ROSTER_TAB(0x1d4, 0xc8, -1, FUN_10014dc4, NULL, "~Select Mission"),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 3), 100, FUN_10014dfe, FUN_10014fee, 0),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 1),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 2),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 3),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 4),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 5),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 6),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 7),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 8),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 9),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 10),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 11),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 12),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 13),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 14),
	ROSTER_TAB(0x1a2, TAB_BELOW(1, 2), 100, FUN_10014dfe, FUN_10014fee, 15),
	ROSTER_END,
};

#undef TAB_BELOW
#undef ROSTER_TAB
#undef ROSTER_END

// GLOBAL: MW2SHELL 0x1006411c
AudioSample* g_unk0x1006411c = NULL;

// Set while the mission list shows instead of the pilot's record.
// GLOBAL: MW2SHELL 0x10064120
MechS32 g_unk0x10064120 = 0;

// Set once each campaign's quick tips have been shown.
// GLOBAL: MW2SHELL 0x10064124
MechS32 g_unk0x10064124 = 0;

// GLOBAL: MW2SHELL 0x10064128
MechS32 g_unk0x10064128 = 0;

// GLOBAL: MW2SHELL 0x1007cda0
ButtonMenu* g_unk0x1007cda0;

void FUN_1001534c(TMPackDataBase*, MechS32* p_campaign, MechU8* p_pilotChosen, MechChar** p_scenario, MechS32 p_msg);
BOOL CALLBACK FUN_10015a6c(HWND p_hDlg, UINT p_msg, WPARAM p_wParam, LPARAM);

// Opens the pilot roster of the campaign's clan hall: the ten pilot slots, the active pilot's
// record, and the menu.
// Not 100%: the stack slots of data, slot, i and size are permuted.
// FUNCTION: MW2SHELL 0x10015008
void FUN_10015008(TMPackDataBase* p_database, MechS32 p_campaign, MechU8* p_pilotChosen, char**)
{
	void* data = NULL;
	MechS32 slot;
	MechS32 i;
	MechS32 size;

	g_unk0x1007cc98 = p_campaign;
	srand(clock());
	g_pVideoDriver->FUN_10006c50(p_database, g_unk0x1006fe40[p_campaign].m_picture);
	LoadPilotRoster();
	g_pCurrentPilot = NULL;

	switch (p_campaign) {
	case 0:
		slot = 0;
		break;
	case 1:
		slot = 10;
		break;
	default:
		slot = 0;
		break;
	}

	for (i = 0; i < 10; i++, slot++) {
		g_unk0x1007cda8[i] = &g_pilotRoster[slot];
		if (g_pCurrentPilot) {
			g_pilotRoster[slot].m_unk0x04 = 0;
		}
		else if (g_pilotRoster[slot].m_unk0x04) {
			if (g_pilotRoster[slot].m_unk0x00 == 1) {
				g_pCurrentPilot = &g_pilotRoster[slot];
			}
			else {
				g_pilotRoster[slot].m_unk0x04 = 0;
			}
		}
	}

	g_unk0x1007cda0 = new ButtonMenu(g_pVideoDriver, g_unk0x1007120c, FALSE, g_unk0x1006fe40[p_campaign].m_buttons, 15);
	FUN_10014b60();
	if (!g_pCurrentPilot) {
		g_unk0x1007cda0->FUN_10048d65(11);
		g_unk0x1007cda0->FUN_10048d65(12);
		g_unk0x1007cda0->FUN_10048d65(13);
		g_unk0x1007cda0->FUN_10048d65(14);
	}
	else {
		FUN_100078cd(g_unk0x10063c78);
		g_unk0x1007cda0->FUN_10048d65(14);
		if (!g_pCurrentPilot->m_mission) {
			g_unk0x1007cda0->FUN_10048d65(13);
		}
	}

	g_pDatabaseMw2->GetDBItem(0x51, &data, &size);
	g_unk0x1006411c = new AudioSample(g_pAudioSubsystem, data, size);
	g_unk0x1006411c->SetVolume(0x1e);
	g_unk0x1006411c->Start();
	*p_pilotChosen = 0;
	FUN_100108e5(FUN_1001534c);
	g_pVideoDriver->DrawShell();
	g_pVideoDriver->ExpandRectBySize(0, 0, 640, 480);
}

// The pilot roster's frame: EXIT, the ten pilot slots (an empty one asks for a callsign),
// ACCEPT, DELETE MECHWARRIOR, LAUNCH OLD MISSION and PILOT INFO.
// Not 100%: the stack slots are permuted, and the mission test loads the tab before the pilot
// (reversing the comparison's operands doesn't flip it).
// FUNCTION: MW2SHELL 0x1001534c
void FUN_1001534c(TMPackDataBase*, MechS32* p_campaign, MechU8* p_pilotChosen, MechChar** p_scenario, MechS32 p_msg)
{
	PilotRecord* pilot;
	MechS32 button;
	ScreenField* tab;

	// The original skips the frame's work with a goto, like FUN_100043c2.
	if (p_msg != 0x404) {
		goto done;
	}

	button = g_unk0x1007cda0->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
	if (g_unk0x1006a9e0 && ((g_unk0x1007cc98 == 1 && !g_unk0x10064124) || (g_unk0x1007cc98 == 0 && !g_unk0x10064128))) {
		switch (g_unk0x1007cc98) {
		case 1:
			DialogBoxParam(g_pModule, MAKEINTRESOURCE(0x65), g_pWnd, (DLGPROC) FUN_10015a6c, 0);
			g_unk0x10064124 = 1;
			break;
		case 0:
			DialogBoxParam(g_pModule, MAKEINTRESOURCE(0x66), g_pWnd, (DLGPROC) FUN_10015a6c, 0);
			g_unk0x10064128 = 1;
			break;
		}
	}

	if (g_pMouseState->GetLeftPressed() == 1) {
		if (g_unk0x10064120) {
			tab = FUN_1000b5ed(g_unk0x10063dd8, g_pMouseState->m_x, g_pMouseState->m_y);
			if (tab && tab->m_unk0x20 && g_pCurrentPilot->m_mission > (MechS32) tab->m_unk0x24) {
				*p_scenario = g_campaignMissions[*p_campaign][(MechS32) tab->m_unk0x24].m_unk0x00;
				FUN_10003175(0, 0, 3, 1, 100);
				ShellApplyMissionUiInfo(*p_scenario, 1, 0);
				FUN_10003175(1, 0, 0, 0, 100);
				FUN_10003175(0, -1, -1, -1, -1);
				FUN_10002de7(0, NULL, g_pCurrentPilot->m_callsign);
				p_msg = 0x410;
			}
		}

	again:
		switch (button) {
		case 0:
			g_pCurrentPilot = NULL;
			p_msg = 0x40e;
			break;
		case 12:
			if (g_pCurrentPilot && ShowDialog("Terminate MechWarrior?#Yes|No", 1) == 1) {
				break;
			}
			FUN_10014caa(g_pCurrentPilot);
			if (g_unk0x10071374) {
				g_unk0x10071374 = 0;
			}
			FUN_10007ac8(g_unk0x10063c78);
			FUN_10007ac8(g_unk0x10063dd8);
			g_unk0x10064120 = 0;
			g_pCurrentPilot = NULL;
			g_unk0x1007cda0->FUN_10048d65(11);
			g_unk0x1007cda0->FUN_10048d65(12);
			g_unk0x1007cda0->FUN_10048d65(13);
			g_unk0x1007cda0->FUN_10048d65(14);
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			pilot = g_unk0x1007cda8[button - 1];
			if (pilot->m_unk0x00) {
				g_pCurrentPilot = pilot;
				FUN_10014d3e(pilot);
				if (g_pMouseState->GetDoubleClicked()) {
					*p_pilotChosen = 1;
					p_msg = 0x407;
					break;
				}
				g_unk0x1007cda0->FUN_10048cc1(11);
				g_unk0x1007cda0->FUN_10048cc1(12);
				if (g_pCurrentPilot->m_mission) {
					g_unk0x1007cda0->FUN_10048cc1(13);
				}
				else {
					g_unk0x1007cda0->FUN_10048d65(13);
				}
				g_unk0x1007cda0->FUN_10048d65(14);
				FUN_10007ac8(g_unk0x10063dd8);
				g_unk0x10064120 = 0;
				FUN_10007ac8(g_unk0x10063c78);
				FUN_100078cd(g_unk0x10063c78);
			}
			else {
				g_unk0x1007cda0->FUN_10048d65(11);
				g_unk0x1007cda0->FUN_10048d65(12);
				g_unk0x1007cda0->FUN_10048d65(13);
				g_unk0x1007cda0->FUN_10048d65(14);
				FUN_10007ac8(g_unk0x10063dd8);
				g_unk0x10064120 = 0;
				FUN_10007ac8(g_unk0x10063c78);
				g_pCurrentPilot = NULL;
				pilot->m_callsign[0] = '\0';
				EditTextField(g_unk0x10071214, 0x2a, (button - 1) * 35 + 0x5c, pilot->m_callsign, NULL, 14, 300);
				FUN_100309b6(pilot->m_callsign);
				if (pilot->m_callsign[0]) {
					g_pCurrentPilot = pilot;
					pilot->m_unk0x00 = 1;
					pilot->m_mission = 0;
					pilot->m_rank = 0;
					pilot->m_honor = (MechS32) (rand() / 32767.0 * 1000.0 + 1000.0);
					pilot->m_unk0x18 = 0;
					pilot->m_unk0x1c = 0;
					pilot->m_unk0x20 = 0;
					pilot->m_unk0x24 = 0;
					FUN_10014d3e(pilot);
					FUN_10014b60();
					g_unk0x1007cda0->FUN_10048cc1(11);
					g_unk0x1007cda0->FUN_10048cc1(12);
					FUN_100078cd(g_unk0x10063c78);
					g_unk0x10071374 = 1;
				}

				if (g_pMouseState->m_leftDown == 1) {
					button = g_unk0x1007cda0->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
					goto again;
				}
			}
			break;
		case 11:
			if (g_pCurrentPilot) {
				*p_pilotChosen = 1;
				p_msg = 0x407;
			}
			break;
		case 13:
			g_unk0x1007cda0->FUN_10048d65(13);
			g_unk0x1007cda0->FUN_10048cc1(14);
			FUN_10007ac8(g_unk0x10063c78);
			FUN_100078cd(g_unk0x10063dd8);
			g_unk0x10064120 = 1;
			break;
		case 14:
			if (g_pCurrentPilot->m_mission) {
				g_unk0x1007cda0->FUN_10048cc1(13);
			}
			g_unk0x1007cda0->FUN_10048d65(14);
			FUN_10007ac8(g_unk0x10063dd8);
			g_unk0x10064120 = 0;
			FUN_100078cd(g_unk0x10063c78);
			break;
		}
	}

done:
	if (p_msg != 0x404) {
		FUN_10007ac8(g_unk0x10063dd8);
		FUN_10007ac8(g_unk0x10063c78);
		SavePilotRoster();
		delete g_unk0x1007cda0;
		delete g_unk0x1006411c;
		FUN_10014c1e();
		PostMessage(g_pWnd, p_msg, 0x412, 0);
		FUN_100108fd(FUN_1001534c);
	}
}

// The quick tips dialog of the pilot roster: whether the quick tips show, and a checkbox for
// g_unk0x1006a9e0.
// FUNCTION: MW2SHELL 0x10015a6c
BOOL CALLBACK FUN_10015a6c(HWND p_hDlg, UINT p_msg, WPARAM p_wParam, LPARAM)
{
	UINT command;

	switch (p_msg) {
	case WM_INITDIALOG:
		if (g_fQuickTips) {
			CheckDlgButton(p_hDlg, 0x3e8, 1);
		}
		else {
			CheckDlgButton(p_hDlg, 0x3e9, 1);
		}
		SetFocus(GetDlgItem(p_hDlg, 1));
		return FALSE;
	case WM_COMMAND:
		command = LOWORD(p_wParam);
		switch (command) {
		case 0x3e8:
			if (IsDlgButtonChecked(p_hDlg, 0x3e8) == 1) {
				CheckDlgButton(p_hDlg, 0x3e9, 0);
			}
			else {
				CheckDlgButton(p_hDlg, 0x3e9, 1);
			}
			break;
		case 0x3e9:
			if (IsDlgButtonChecked(p_hDlg, 0x3e9) == 1) {
				CheckDlgButton(p_hDlg, 0x3e8, 0);
			}
			else {
				CheckDlgButton(p_hDlg, 0x3e8, 1);
			}
			break;
		case 0x3ea:
			if (IsDlgButtonChecked(p_hDlg, 0x3ea) == 1) {
				g_unk0x1006a9e0 = 0;
			}
			else {
				g_unk0x1006a9e0 = 1;
			}
			break;
		case 1:
			EndDialog(p_hDlg, 0);
			break;
		}

		if (IsDlgButtonChecked(p_hDlg, 0x3e8) == 1) {
			g_fQuickTips = 1;
			CheckMenuItem(g_windowMenu, 0x9c72, MF_CHECKED);
		}
		else {
			g_fQuickTips = 0;
			CheckMenuItem(g_windowMenu, 0x9c72, MF_UNCHECKED);
		}
		return TRUE;
	}

	return FALSE;
}
