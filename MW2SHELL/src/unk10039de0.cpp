#include "audiosample.h"
#include "audiosubsystem.h"
#include "brasslantern0x414.h"
#include "campaignmission.h"
#include "decomp.h"
#include "granitemast0x18.h"
#include "mainmenubutton.h"
#include "menulist0x10d.h"
#include "mousestate.h"
#include "tallowsign0x10.h"
#include "tinwhistle0x3c.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The ready room screen.

extern "C" HWND g_pWnd;
extern HINSTANCE g_pModule;
extern MechU32 g_fQuickTips;
extern AudioSubsystem* g_pAudioSubsystem;
extern VideoDriver* g_pVideoDriver;
extern MouseState* g_pMouseState;
extern BrassLantern0x414* g_unk0x1007120c;
extern TinWhistle0x3c* g_pCurrentPilot;
extern CampaignMission* g_campaignMissions[2];
extern GraniteMast0x18 g_unk0x10061560[];

MechS32 FUN_10002de7(MechS32 p_index, MechChar* p_variant, MechChar* p_name);
void FUN_10003175(MechS32 p_star, MechS32 p_formation, MechS32 p_size, MechS32 p_count, MechS32 p_tonnage);
MechS32 FUN_1000307c(MechS32 p_index);
void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
void FUN_100108fd(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
BOOL CALLBACK FUN_1001067f(HWND p_hDlg, UINT p_msg, WPARAM p_wParam, LPARAM);
void FUN_10016cc0(MechS32 p_index, MechS32 p_mask, MechS32 p_value);
void FUN_10016f45();
MechS32 FUN_100175e2(MechChar* p_name, MechS32 p_left, MechS32 p_top, MechU32 p_flags, MechU32 p_unk0x14);
void SavePilotRoster();
void ShellApplyMissionUiInfo(MechChar* p_scenario, MechS32 p_stars, MechS32 p_video);
MechS32 ShowDialog(const char* p_text, MechS32);
MechS32 FUN_10016b11(MechS32 p_index);
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_x,
	undefined4 p_y,
	MechU32 p_flags,
	MechU32 p_unk0x14
);

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

// GLOBAL: MW2SHELL 0x100701ec
MechChar g_unk0x100701ec[0x0c] = "CLAN HALL";
// GLOBAL: MW2SHELL 0x100701f8
MechChar g_unk0x100701f8[0x0c] = "~MECH LAB";
// GLOBAL: MW2SHELL 0x10070204
MechChar g_unk0x10070204[0x10] = "~STAR CONFIG";
// GLOBAL: MW2SHELL 0x10070214
MechChar g_unk0x10070214[0x14] = "~MISSION BRIEFING";
// GLOBAL: MW2SHELL 0x10070228
MechChar g_unk0x10070228[0x0c] = "<~YELLOW";
// GLOBAL: MW2SHELL 0x10070234
MechChar g_unk0x10070234[0x0c] = "<~ORANGE";
// GLOBAL: MW2SHELL 0x10070240
MechChar g_unk0x10070240[0x08] = "<~TEAL";
// GLOBAL: MW2SHELL 0x10070248
MechChar g_unk0x10070248[0x08] = "<~TAUPE";
// GLOBAL: MW2SHELL 0x10070250
MechChar g_unk0x10070250[0x08] = "<~JENNY";
// GLOBAL: MW2SHELL 0x10070258
MechChar g_unk0x10070258[0x08] = "<~SABLE";
// GLOBAL: MW2SHELL 0x10070260
MechChar g_unk0x10070260[0x08] = "<~GREY";
// GLOBAL: MW2SHELL 0x10070268
MechChar g_unk0x10070268[0x08] = "<~BROWN";
// GLOBAL: MW2SHELL 0x10070270
MechChar g_unk0x10070270[0x08] = "<~AMY";
// GLOBAL: MW2SHELL 0x10070278
MechChar g_unk0x10070278[0x0c] = "<~SILVER";
// GLOBAL: MW2SHELL 0x10070284
MechChar g_unk0x10070284[0x08] = "<~AQUA";
// GLOBAL: MW2SHELL 0x1007028c
MechChar g_unk0x1007028c[0x08] = "<~KIM";
// GLOBAL: MW2SHELL 0x10070294
MechChar g_unk0x10070294[0x08] = "<~CYAN";
// GLOBAL: MW2SHELL 0x1007029c
MechChar g_unk0x1007029c[0x0c] = "<~MAROON";
// GLOBAL: MW2SHELL 0x100702a8
MechChar g_unk0x100702a8[0x08] = "<~GOLD";
// GLOBAL: MW2SHELL 0x100702b0
MechChar g_unk0x100702b0[0x08] = "<~IRENE";
// GLOBAL: MW2SHELL 0x100705b4
MechChar g_unk0x100705b4[0x0c] = "CLAN HALL";
// GLOBAL: MW2SHELL 0x100705c0
MechChar g_unk0x100705c0[0x0c] = "~MECH LAB";
// GLOBAL: MW2SHELL 0x100705cc
MechChar g_unk0x100705cc[0x10] = "~STAR CONFIG";
// GLOBAL: MW2SHELL 0x100705dc
MechChar g_unk0x100705dc[0x14] = "~MISSION BRIEFING";
// GLOBAL: MW2SHELL 0x100705f0
MechChar g_unk0x100705f0[0x08] = "<~PINK";
// GLOBAL: MW2SHELL 0x100705f8
MechChar g_unk0x100705f8[0x08] = "<~GREEN";
// GLOBAL: MW2SHELL 0x10070600
MechChar g_unk0x10070600[0x08] = "<~RED";
// GLOBAL: MW2SHELL 0x10070608
MechChar g_unk0x10070608[0x0c] = "<~FUCHSIA";
// GLOBAL: MW2SHELL 0x10070614
MechChar g_unk0x10070614[0x08] = "<~CINDY";
// GLOBAL: MW2SHELL 0x1007061c
MechChar g_unk0x1007061c[0x08] = "<~RUST";
// GLOBAL: MW2SHELL 0x10070624
MechChar g_unk0x10070624[0x08] = "<~UMBER";
// GLOBAL: MW2SHELL 0x1007062c
MechChar g_unk0x1007062c[0x08] = "<~TAN";
// GLOBAL: MW2SHELL 0x10070634
MechChar g_unk0x10070634[0x08] = "<~HEIDI";
// GLOBAL: MW2SHELL 0x1007063c
MechChar g_unk0x1007063c[0x08] = "<~PLUM";
// GLOBAL: MW2SHELL 0x10070644
MechChar g_unk0x10070644[0x08] = "<~WHITE";
// GLOBAL: MW2SHELL 0x1007064c
MechChar g_unk0x1007064c[0x08] = "<~JILL";
// GLOBAL: MW2SHELL 0x10070654
MechChar g_unk0x10070654[0x08] = "<~PUCE";
// GLOBAL: MW2SHELL 0x1007065c
MechChar g_unk0x1007065c[0x0c] = "<~BLONDE";
// GLOBAL: MW2SHELL 0x10070668
MechChar g_unk0x10070668[0x0c] = "<~BRONZE";
// GLOBAL: MW2SHELL 0x10070674
MechChar g_unk0x10070674[0x08] = "<~MARY";

// GLOBAL: MW2SHELL 0x1006e5d8
MainMenuButton g_unk0x1006e5d8[20] = {
	{0, 0, 130, 479, 56, 223, g_unk0x100701ec},      {300, 320, 559, 419, 450, 364, g_unk0x100701f8},
	{510, 420, 559, 469, 542, 455, g_unk0x10070204}, {140, 90, 399, 313, 265, 226, g_unk0x10070214},
	{400, 25, 519, 49, 460, 30, g_unk0x10070228},    {400, 55, 519, 79, 460, 60, g_unk0x10070234},
	{400, 85, 519, 109, 460, 90, g_unk0x10070240},   {400, 115, 519, 139, 460, 120, g_unk0x10070248},
	{400, 145, 519, 169, 460, 150, g_unk0x10070250}, {400, 175, 519, 199, 460, 180, g_unk0x10070258},
	{400, 205, 519, 229, 460, 210, g_unk0x10070260}, {400, 235, 519, 259, 460, 240, g_unk0x10070268},
	{520, 25, 639, 49, 580, 30, g_unk0x10070270},    {520, 55, 639, 79, 580, 60, g_unk0x10070278},
	{520, 85, 639, 109, 580, 90, g_unk0x10070284},   {520, 115, 639, 139, 580, 120, g_unk0x1007028c},
	{520, 145, 639, 169, 580, 150, g_unk0x10070294}, {520, 175, 639, 199, 580, 180, g_unk0x1007029c},
	{520, 205, 639, 229, 580, 210, g_unk0x100702a8}, {520, 235, 639, 259, 580, 240, g_unk0x100702b0},
};

// GLOBAL: MW2SHELL 0x1006ef58
MainMenuButton g_unk0x1006ef58[20] = {
	{0, 0, 130, 479, 56, 223, g_unk0x100705b4},      {280, 340, 534, 439, 415, 370, g_unk0x100705c0},
	{432, 440, 482, 479, 464, 460, g_unk0x100705cc}, {140, 90, 399, 313, 277, 226, g_unk0x100705dc},
	{400, 25, 519, 49, 460, 30, g_unk0x100705f0},    {400, 55, 519, 79, 460, 60, g_unk0x100705f8},
	{400, 85, 519, 109, 460, 90, g_unk0x10070600},   {400, 115, 519, 139, 460, 120, g_unk0x10070608},
	{400, 145, 519, 169, 460, 150, g_unk0x10070614}, {400, 175, 519, 199, 460, 180, g_unk0x1007061c},
	{400, 205, 519, 229, 460, 210, g_unk0x10070624}, {400, 235, 519, 259, 460, 240, g_unk0x1007062c},
	{520, 25, 639, 49, 580, 30, g_unk0x10070634},    {520, 55, 639, 79, 580, 60, g_unk0x1007063c},
	{520, 85, 639, 109, 580, 90, g_unk0x10070644},   {520, 115, 639, 139, 580, 120, g_unk0x1007064c},
	{520, 145, 639, 169, 580, 150, g_unk0x10070654}, {520, 175, 639, 199, 580, 180, g_unk0x1007065c},
	{520, 205, 639, 229, 580, 210, g_unk0x10070668}, {520, 235, 639, 259, 580, 240, g_unk0x10070674},
};

// GLOBAL: MW2SHELL 0x1006fed0
TallowSign0x10 g_unk0x1006fed0[3] = {
	{g_unk0x1006e5d8, 20, 14, 0x25},
	{g_unk0x1006ef58, 20, 21, 0x28},
	{NULL, 0, 0, 0},
};

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
