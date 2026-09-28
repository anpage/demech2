#include "unk10039de0.h"

#include "audiosample.h"
#include "audiosubsystem.h"
#include "brasslantern0x414.h"
#include "campaignmission.h"
#include "decomp.h"
#include "granitemast0x18.h"
#include "mainmenubutton.h"
#include "mechbay.h"
#include "mechvariant.h"
#include "menulist0x10d.h"
#include "missionui.h"
#include "mousestate.h"
#include "options.h"
#include "pilotroster.h"
#include "shellmain.h"
#include "tallowsign0x10.h"
#include "tinwhistle0x3c.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "unk10010a30.h"
#include "unk1003bf90.h"
#include "unk1006e150.h"
#include "unk100711f8.h"
#include "video.h"
#include "videodriver.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The ready room screen.

// The video playing before the screen moves on, -1 for none, and the message it moves on with.
// GLOBAL: MW2SHELL 0x1006a588
MechS32 g_unk0x1006a588 = -1;

// GLOBAL: MW2SHELL 0x1006a58c
MechS32 g_unk0x1006a58c = 0x404;

// GLOBAL: MW2SHELL 0x1006a590
AudioSample* g_unk0x1006a590 = NULL;

// Set once the ready room's quick tips have been shown.
// GLOBAL: MW2SHELL 0x1006a594
MechS32 g_unk0x1006a594 = 0;

// GLOBAL: MW2SHELL 0x100904a0
MenuList0x10d* g_unk0x100904a0;

// GLOBAL: MW2SHELL 0x100904a4
WPARAM g_unk0x100904a4;

// Play the second faction grid animation only when video slot zero is idle.
// FUNCTION: MW2SHELL 0x10039de0
void FUN_10039de0(MechS32 p_campaign)
{
	if (!FUN_10016b11(0)) {
		switch (p_campaign) {
		case 0:
			FUN_10017460(0, "awogrid2", 0x12f, 0x149, 0x48, 0);
			break;
		case 1:
			FUN_10017460(0, "ajfgrid2", 0x115, 0x155, 0x48, 0);
		default:
			break;
		}
	}
}

void FUN_1003a151(TMPackDataBase* p_database, MechS32* p_campaign, MechU8*, MechChar** p_scenario, MechS32 p_msg);

// Opens the ready room. Coming from the pilot roster (0x407) or a mission (0x409) sets up the
// pilot's next mission first; only the pilot FREEBIRTHTOAD gets the mission buttons.
// FUNCTION: MW2SHELL 0x10039e72
void FUN_10039e72(TMPackDataBase* p_database, MechS32 p_campaign, char** p_scenario, WPARAM p_wParam)
{
	if (p_wParam == 0x407 || p_wParam == 0x409) {
		FUN_10003175(0, 0, 3, 1, 100);
		*p_scenario = g_campaignMissions[p_campaign][g_pCurrentPilot->m_mission].m_unk0x00;
		ShellApplyMissionUiInfo(*p_scenario, 1, 0);
	}

	g_unk0x100904a4 = p_wParam;
	FUN_10003175(1, 0, 0, 0, 100);
	FUN_10003175(0, -1, -1, -1, -1);
	FUN_10002de7(0, NULL, NULL);
	g_pVideoDriver->FUN_10006c50(p_database, g_unk0x1006fed0[p_campaign].m_picture);

	if (!strcmp(g_pCurrentPilot->m_callsign, "FREEBIRTHTOAD")) {
		g_unk0x100904a0 = new MenuList0x10d(
			g_pVideoDriver,
			g_unk0x1007120c,
			FALSE,
			g_unk0x1006fed0[p_campaign].m_buttons,
			g_unk0x1006fed0[p_campaign].m_count
		);
	}
	else {
		g_unk0x100904a0 =
			new MenuList0x10d(g_pVideoDriver, g_unk0x1007120c, FALSE, g_unk0x1006fed0[p_campaign].m_buttons, 4);
	}

	switch (p_campaign) {
	case 0:
		FUN_10017460(0, "awogrid1", 0x12f, 0x149, 0x44, 0);
		g_pMouseState->MoveCursorTo(0x1b3, 0x168);
		break;
	case 1:
		FUN_10017460(0, "ajfgrid1", 0x115, 0x155, 0x44, 0);
		if (p_wParam == 0x407) {
			FUN_10017460(0x10, "ajfv8trd", 1, 0x6c, 2, 0);
		}
		break;
	}

	FUN_100108e5(FUN_1003a151);
}

// The ready room's frame: CLAN HALL, MECH LAB, STAR CONFIG, MISSION BRIEFING, and for
// FREEBIRTHTOAD the missions. The mech lab and briefing play a video before moving on.
// Not 100%: the stack slots of the locals are permuted.
// FUNCTION: MW2SHELL 0x1003a151
void FUN_1003a151(TMPackDataBase* p_database, MechS32* p_campaign, MechU8*, MechChar** p_scenario, MechS32 p_msg)
{
	void* data = NULL;
	MechChar name[16];
	MechS32 type;
	MechS32 button;
	MechS32 size;

	// The original skips the frame's work with a goto, like FUN_100043c2.
	if (p_msg != 0x404) {
		goto done;
	}

	if (g_fQuickTips && !g_unk0x1006a594 && g_unk0x100904a4 == 0x407 && !FUN_10016b11(0x10)) {
		DialogBoxParam(g_pModule, MAKEINTRESOURCE(0x70), g_pWnd, (DLGPROC) FUN_1001067f, 0);
		g_unk0x1006a594 = 1;
	}

	if (g_unk0x1006a588 == -1) {
		FUN_10039de0(*p_campaign);
		button = g_unk0x100904a0->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
		switch (button) {
		case -1:
			break;
		case 1:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			if (g_campaignMissions[g_pCurrentPilot->m_unk0x08][g_pCurrentPilot->m_mission].m_unk0x04 == 1) {
				ShowDialog("Trial Protocol: X0769-Q|Keshik to determine appropriate|'Mech for trial.#Ok", 0);
				break;
			}
			switch (*p_campaign) {
			case 0:
				FUN_10016cc0(0, 0x40000000, 0x40000000);
				type = FUN_1000307c(-1);
				if (type < 0) {
					type = 7;
				}
				sprintf(name, "awo%stbl", g_unk0x10061560[type].m_unk0x00);
				g_unk0x1006a588 = FUN_10017460(0, name, 0x131, 0xb9, 6, 0);
				break;
			case 1:
				FUN_10016cc0(0, 0x40000000, 0x40000000);
				type = FUN_1000307c(-1);
				if (type < 0) {
					type = 7;
				}
				sprintf(name, "ajf%stbl", g_unk0x10061560[type].m_unk0x00);
				g_unk0x1006a588 = FUN_10017460(0, name, 0x114, 0xa4, 6, 0);
				break;
			}
			p_database->GetDBItem(0x64, &data, &size);
			g_unk0x1006a590 = new AudioSample(g_pAudioSubsystem, data, size);
			g_unk0x1006a590->SetVolume(0x32);
			g_unk0x1006a590->Start();
			g_unk0x1006a58c = 0x40f;
			break;
		case 2:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			if (g_campaignMissions[g_pCurrentPilot->m_unk0x08][g_pCurrentPilot->m_mission].m_unk0x04 == 1) {
				ShowDialog("Your 'Mech has been|selected for you.|Prepare for Trial!#Ok", 0);
				break;
			}
			p_msg = 0x413;
			break;
		case 0:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			switch (*p_campaign) {
			case 0:
				p_msg = 0x407;
				break;
			case 1:
				g_unk0x1006a588 = 0x10;
				break;
			}
			break;
		case 3:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
		briefing:
			switch (*p_campaign) {
			case 0:
				g_unk0x1006a588 = FUN_100175e2("awobrief", 0x69, 0x64, 6, 0);
				break;
			case 1:
				g_unk0x1006a588 = FUN_100175e2("ajfbrief", 0x6b, 0x69, 6, 0);
				break;
			}
			g_unk0x1006a58c = 0x406;
			break;
		default:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			g_pCurrentPilot->m_mission = button - 4;
			SavePilotRoster();
			*p_scenario = g_campaignMissions[*p_campaign][button - 4].m_unk0x00;
			FUN_10003175(0, 0, 3, 1, 100);
			ShellApplyMissionUiInfo(*p_scenario, 1, 0);
			goto briefing;
		}
	}
	else if (!FUN_10016b11(g_unk0x1006a588)) {
		if (g_unk0x1006a58c == 0x404) {
			g_unk0x1006a588 = FUN_10017460(0x10, "ajfv8tru", 1, 0x6c, 2, 0);
			g_unk0x1006a58c = 0x407;
		}
		else {
			p_msg = g_unk0x1006a58c;
			g_unk0x1006a588 = -1;
			g_unk0x1006a58c = 0x404;
		}
	}

done:
	if (p_msg != 0x404) {
		FUN_10016f45();
		delete g_unk0x100904a0;
		if (g_unk0x1006a590) {
			delete g_unk0x1006a590;
		}
		g_unk0x1006a590 = NULL;
		g_unk0x1006a594 = 0;
		PostMessage(g_pWnd, p_msg, 0x411, 0);
		FUN_100108fd(FUN_1003a151);
	}
}
