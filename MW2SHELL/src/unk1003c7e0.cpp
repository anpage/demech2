#include "audiosample.h"
#include "audiosubsystem.h"
#include "decomp.h"
#include "mainmenubutton.h"
#include "menulist0x10d.h"
#include "mousestate.h"
#include "tallowsign0x10.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <stdlib.h>
#include <time.h>
#include <windows.h>

extern "C" HWND g_pWnd;
extern "C" HINSTANCE g_pModule;
extern "C" MechU32 g_fQuickTips;
extern AudioSubsystem* g_pAudioSubsystem;
extern MouseState* g_pMouseState;
extern VideoDriver* g_pVideoDriver;
extern BrassLantern0x414* g_unk0x1007120c;
extern TallowSign0x10 g_unk0x1006ffc0[3];

void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
void FUN_100108fd(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
BOOL CALLBACK FUN_1001067f(HWND p_hDlg, UINT p_msg, WPARAM p_wParam, LPARAM);
MechS32 FUN_10016b11(MechS32 p_index);
void FUN_10016f45();
void ShellApplyMissionUiInfo(MechChar* p_scenario, MechS32 p_stars, MechS32 p_video);
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_left,
	undefined4 p_top,
	MechU32 p_flags,
	MechU32 p_fps
);

extern MechChar** g_unk0x1006fe08[2];

// The trainer's idle video alternates between two takes; a countdown to the next.
// GLOBAL: MW2SHELL 0x1006acc8
MechS32 g_unk0x1006acc8 = 0;

// GLOBAL: MW2SHELL 0x1006accc
MechS32 g_unk0x1006accc = -1;

// Set once the trainer's welcome video is over and the mission buttons are up.
// GLOBAL: MW2SHELL 0x1006acd0
MechS32 g_unk0x1006acd0 = 0;

// The video playing before a training mission starts, -1 for none.
// GLOBAL: MW2SHELL 0x1006acd4
MechS32 g_unk0x1006acd4 = -1;

// GLOBAL: MW2SHELL 0x1006acd8
AudioSample* g_unk0x1006acd8 = NULL;

// Set once the training screen's quick tips have been shown.
// GLOBAL: MW2SHELL 0x1006acdc
MechS32 g_unk0x1006acdc = 0;

// GLOBAL: MW2SHELL 0x10090668
MenuList0x10d* g_unk0x10090668;

// GLOBAL: MW2SHELL 0x1009066c
WPARAM g_unk0x1009066c;

void FUN_1003c966(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32);

// Sets up the campaign's training screen: its first button, its background and the videos of
// the trainer.
// FUNCTION: MW2SHELL 0x1003c7e0
void FUN_1003c7e0(TMPackDataBase* p_database, MechS32 p_campaign, char**, WPARAM p_wParam)
{
	g_unk0x1009066c = p_wParam;
	g_unk0x10090668 = new MenuList0x10d(g_pVideoDriver, g_unk0x1007120c, 0, g_unk0x1006ffc0[p_campaign].m_buttons, 1);
	g_pVideoDriver->FUN_10006c50(p_database, g_unk0x1006ffc0[p_campaign].m_picture);

	switch (p_campaign) {
	case 0:
		FUN_10017460(0, "awotrnwn", 0x1c5, 0, 0x42, 0);
		FUN_10017460(3, "awotrnwa", 0x48, 0xe0, 2, 0);
		break;
	case 1:
		FUN_10017460(0, "ajftrnwn", 0x1a0, 0, 0x42, 0);
		FUN_10017460(3, "ajftrnwa", 0x48, 0xe0, 2, 0);
		break;
	}

	srand(clock());
	FUN_100108e5(FUN_1003c966);
}

// The training screen's frame: once the trainer's welcome is over, the mission buttons and the
// room's sound; EXIT, or a training mission after the trainer's video.
// Not 100%: the stack slots of the locals are permuted.
// FUNCTION: MW2SHELL 0x1003c966
void FUN_1003c966(TMPackDataBase* p_database, MechS32* p_campaign, MechU8*, MechChar** p_scenario, MechS32 p_msg)
{
	void* data = NULL;
	MechS32 i;
	MechS32 button;
	MechS32 size;

	// The original skips the frame's work with a goto, like FUN_100043c2.
	if (p_msg != 0x404) {
		goto done;
	}

	if (g_fQuickTips && !g_unk0x1006acdc && g_unk0x1009066c == 0x407 && g_unk0x1006acd0) {
		DialogBoxParam(g_pModule, MAKEINTRESOURCE(0x68), g_pWnd, (DLGPROC) FUN_1001067f, 0);
		g_unk0x1006acdc = 1;
	}

	if (!g_unk0x1006acd0 && !FUN_10016b11(0)) {
		g_unk0x1006acd0 = 1;
		for (i = 1; i < g_unk0x1006ffc0[*p_campaign].m_count; i++) {
			g_unk0x10090668->FUN_10048b95(g_unk0x1006ffc0[*p_campaign].m_buttons[i], i, FALSE);
		}

		p_database->GetDBItem(0x4c, &data, &size);
		g_unk0x1006acd8 = new AudioSample(g_pAudioSubsystem, data, size);
		g_unk0x1006acd8->SetVolume(0x32);
		g_unk0x1006acd8->EnableLoop();
		g_unk0x1006acd8->Start();
	}

	if (!FUN_10016b11(3)) {
		if (!g_unk0x1006accc) {
			switch (*p_campaign) {
			case 0:
				FUN_10017460(3, g_unk0x1006acc8 ? "awotrnwa" : "awotrnwb", 0x48, 0xe0, 2, 0);
				g_unk0x1006acc8 = 1 - g_unk0x1006acc8;
				break;
			case 1:
				FUN_10017460(3, g_unk0x1006acc8 ? "ajftrnwa" : "ajftrnwb", 0x48, 0xe0, 2, 0);
				g_unk0x1006acc8 = 1 - g_unk0x1006acc8;
				break;
			}
		}

		if (--g_unk0x1006accc < 0) {
			g_unk0x1006accc = 20000;
		}
	}

	if (g_unk0x1006acd4 == -1) {
		button = g_unk0x10090668->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
		switch (button) {
		case 0:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			p_msg = 0x407;
			break;
		default:
			if (button == -1 || g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			*p_scenario = g_unk0x1006fe08[*p_campaign][button - 1];
			ShellApplyMissionUiInfo(*p_scenario, 0, 0);
			delete g_unk0x1006acd8;
			g_unk0x1006acd8 = NULL;
			switch (*p_campaign) {
			case 0:
				g_unk0x1006acd4 = FUN_10017460(0x10, "awotrndr", 0x230, 0xa8, 2, 0);
				break;
			case 1:
				g_unk0x1006acd4 = FUN_10017460(0x10, "ajftrndr", 0x21c, 0xa8, 2, 0);
				break;
			}
			break;
		}
	}
	else if (!FUN_10016b11(g_unk0x1006acd4)) {
		g_unk0x1006acd4 = -1;
		p_msg = 0x410;
	}

done:
	if (p_msg != 0x404) {
		FUN_10016f45();
		delete g_unk0x10090668;
		delete g_unk0x1006acd8;
		g_unk0x1006acd8 = NULL;
		g_unk0x1006acdc = 0;
		g_unk0x1006acc8 = 0;
		g_unk0x1006accc = -1;
		g_unk0x1006acd0 = 0;
		PostMessage(g_pWnd, p_msg, 0x414, 0);
		FUN_100108fd(FUN_1003c966);
	}
}
