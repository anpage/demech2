#include "cadettraining.h"

#include "audiosample.h"
#include "audiosubsystem.h"
#include "buttonmenu.h"
#include "decomp.h"
#include "mainmenubutton.h"
#include "menudata.h"
#include "menuscreen.h"
#include "missionui.h"
#include "mousestate.h"
#include "refreshmode.h"
#include "shellglobals.h"
#include "shellmain.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "unk1003bf90.h"
#include "video.h"
#include "videodriver.h"

#include <stdlib.h>
#include <time.h>
#include <windows.h>

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
ButtonMenu* g_unk0x10090668;

// GLOBAL: MW2SHELL 0x1009066c
WPARAM g_unk0x1009066c;

void CadetTrainingCallback(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32);

// Sets up the campaign's training screen: its first button, its background and the videos of
// the trainer.
// FUNCTION: MW2SHELL 0x1003c7e0
void DrawCadetTraining(TMPackDataBase* p_database, MechS32 p_campaign, char**, WPARAM p_wParam)
{
	g_unk0x1009066c = p_wParam;
	g_unk0x10090668 = new ButtonMenu(g_pVideoDriver, g_defaultFont, 0, g_unk0x1006ffc0[p_campaign].m_buttons, 1);
	g_pVideoDriver->LoadBackground(p_database, g_unk0x1006ffc0[p_campaign].m_picture);

	switch (p_campaign) {
	case 0:
		PlayVideo(0, "awotrnwn", 0x1c5, 0, 0x42, 0);
		PlayVideo(3, "awotrnwa", 0x48, 0xe0, 2, 0);
		break;
	case 1:
		PlayVideo(0, "ajftrnwn", 0x1a0, 0, 0x42, 0);
		PlayVideo(3, "ajftrnwa", 0x48, 0xe0, 2, 0);
		break;
	}

	srand(clock());
	RegisterScreenFunction(CadetTrainingCallback);
}

// The training screen's frame: once the trainer's welcome is over, the mission buttons and the
// room's sound; EXIT, or a training mission after the trainer's video.
// Not 100%: the stack slots of the locals are permuted.
// FUNCTION: MW2SHELL 0x1003c966
void CadetTrainingCallback(
	TMPackDataBase* p_database,
	MechS32* p_campaign,
	MechU8*,
	MechChar** p_scenario,
	MechS32 p_msg
)
{
	void* data = NULL;
	MechS32 i;
	MechS32 button;
	MechS32 size;

	// The original skips the frame's work with a goto, like StarConfigCallback.
	if (p_msg != 0x404) {
		goto done;
	}

	if (g_fQuickTips && !g_unk0x1006acdc && g_unk0x1009066c == 0x407 && g_unk0x1006acd0) {
		DialogBoxParam(g_pModule, MAKEINTRESOURCE(0x68), g_pWnd, (DLGPROC) OkDialogProc, 0);
		g_unk0x1006acdc = 1;
	}

	if (!g_unk0x1006acd0 && !IsVideoPlaying(0)) {
		g_unk0x1006acd0 = 1;
		for (i = 1; i < g_unk0x1006ffc0[*p_campaign].m_count; i++) {
			g_unk0x10090668->AddButton(g_unk0x1006ffc0[*p_campaign].m_buttons[i], i, FALSE);
		}

		p_database->GetDBItem(0x4c, &data, &size);
		g_unk0x1006acd8 = new AudioSample(g_pAudioSubsystem, data, size);
		g_unk0x1006acd8->SetVolume(0x32);
		g_unk0x1006acd8->EnableLoop();
		g_unk0x1006acd8->Start();
	}

	if (!IsVideoPlaying(3)) {
		if (!g_unk0x1006accc) {
			switch (*p_campaign) {
			case 0:
				PlayVideo(3, g_unk0x1006acc8 ? "awotrnwa" : "awotrnwb", 0x48, 0xe0, 2, 0);
				g_unk0x1006acc8 = 1 - g_unk0x1006acc8;
				break;
			case 1:
				PlayVideo(3, g_unk0x1006acc8 ? "ajftrnwa" : "ajftrnwb", 0x48, 0xe0, 2, 0);
				g_unk0x1006acc8 = 1 - g_unk0x1006acc8;
				break;
			}
		}

		if (--g_unk0x1006accc < 0) {
			g_unk0x1006accc = 20000;
		}
	}

	if (g_unk0x1006acd4 == -1) {
		button = g_unk0x10090668->HitTest(g_pMouseState->m_x, g_pMouseState->m_y);
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
				g_unk0x1006acd4 = PlayVideo(0x10, "awotrndr", 0x230, 0xa8, 2, 0);
				break;
			case 1:
				g_unk0x1006acd4 = PlayVideo(0x10, "ajftrndr", 0x21c, 0xa8, 2, 0);
				break;
			}
			break;
		}
	}
	else if (!IsVideoPlaying(g_unk0x1006acd4)) {
		g_unk0x1006acd4 = -1;
		p_msg = 0x410;
	}

done:
	if (p_msg != 0x404) {
		CloseAllVideos();
		delete g_unk0x10090668;
		delete g_unk0x1006acd8;
		g_unk0x1006acd8 = NULL;
		g_unk0x1006acdc = 0;
		g_unk0x1006acc8 = 0;
		g_unk0x1006accc = -1;
		g_unk0x1006acd0 = 0;
		PostMessage(g_pWnd, p_msg, 0x414, 0);
		UnregisterScreenFunction(CadetTrainingCallback);
	}
}
