#include "clanhall.h"

#include "audiosample.h"
#include "audiosubsystem.h"
#include "buttonmenu.h"
#include "decomp.h"
#include "font.h"
#include "mainmenubutton.h"
#include "mechvariant.h"
#include "menudata.h"
#include "menuscreen.h"
#include "mousestate.h"
#include "options.h"
#include "pilotrecord.h"
#include "refreshmode.h"
#include "shellglobals.h"
#include "shellmain.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "video.h"
#include "videodriver.h"

#include <windows.h>

// The clan hall screen.

// GLOBAL: MW2SHELL 0x10063b70
ButtonMenu* g_clanHallMenu = NULL;

// The Wolf hall's ambience.
// GLOBAL: MW2SHELL 0x10063b74
AudioSample* g_clanHallAmbience = NULL;

// The welcome for a newly registered pilot (g_newPilotRegistered), database item 0x69.
// GLOBAL: MW2SHELL 0x10063b78
AudioSample* g_welcomeSound = NULL;

// The Jade Falcon hall holds the welcome back until its door videos (2 and 3) are done.
// GLOBAL: MW2SHELL 0x10063b7c
MechS32 g_welcomePending = 0;

// The video playing before the screen moves on, -1 for none, and the message it moves on with.
// GLOBAL: MW2SHELL 0x10063b80
MechS32 g_clanHallExitVideo = -1;

// GLOBAL: MW2SHELL 0x10063b84
MechS32 g_clanHallExitMessage = 0x404;

void ClanHallCallback(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar**, MechS32 p_msg);

// Opens the clan hall. Coming back from registering a pilot (0x412) sets up the new pilot's
// star first.
// Not 100%: the stack slots of unused, data and size are permuted.
// FUNCTION: MW2SHELL 0x10014040
void DrawClanHall(TMPackDataBase* p_database, MechS32 p_campaign, MechU8, WPARAM p_wParam)
{
	MechS32 unused = -1;
	void* data = NULL;
	MechS32 size;

	if (p_wParam == 0x412) {
		SelectStar(0, 0, 3, 1, 100);
		SetStarMech(0, NULL, g_pCurrentPilot->m_callsign);
		if (g_newPilotRegistered) {
			if (p_campaign == 0) {
				g_pDatabaseMw2->GetDBItem(0x69, &data, &size);
				g_welcomeSound = new AudioSample(g_pAudioSubsystem, data, size);
				g_welcomeSound->SetVolume(0x7f);
			}
			else {
				g_welcomePending = 1;
			}
			g_newPilotRegistered = 0;
		}
	}

	g_clanHallMenu = new ButtonMenu(g_pVideoDriver, g_defaultFont, FALSE, g_clanHallScreens[p_campaign].m_buttons, 4);

	switch (p_campaign) {
	case 0:
		PlayVideo(0, "awoball", 0x5b, 0x17c, 0xa, 0);
		PlayVideo(1, "awoarcht", 0x148, 0x14c, 0x4a, 0);
		PlayVideo(2, "awolite1", 0, 0x118, 0x4a, 0);
		PlayVideo(3, "awolite2", 0x9e, 0x12c, 0x4a, 0);
		PlayVideo(4, "awolite3", 0x244, 0x113, 0x4a, 0);
		p_database->GetDBItem(0x4f, &data, &size);
		g_clanHallAmbience = new AudioSample(g_pAudioSubsystem, data, size);
		g_clanHallAmbience->SetVolume(0x32);
		g_clanHallAmbience->EnableLoop();
		break;
	case 1:
		if (p_wParam == 0x414) {
			PlayVideo(2, "ajf8orl1", 0, 0, 2, 0);
		}
		else if (p_wParam == 0x411) {
			PlayVideo(3, "ajf8orr1", 0x194, 1, 2, 0);
		}
		else if (p_wParam == 0x412) {
			PlayVideo(2, "ajf8orl1", 0, 0, 0x42, 0);
			PlayVideo(3, "ajf8orr1", 0x194, 1, 2, 0);
		}
		PlayVideo(1, "ajfarcht", 0xdc, 0x118, 0x4a, 0);
		PlayVideo(0, "ajfball", 0x190, 0x180, 0x4a, 0);
		break;
	}

	RegisterScreenFunction(ClanHallCallback);
	g_pVideoDriver->LoadBackground(p_database, g_clanHallScreens[p_campaign].m_picture);
	UpdateVideos();
	g_pVideoDriver->DrawShell();
	if (g_clanHallAmbience) {
		g_clanHallAmbience->Start();
	}
	if (g_welcomeSound) {
		g_welcomeSound->Start();
	}
}

// The clan hall's frame: CADET TRAINING, ARCHIVE HOLOPROJECTOR, READY ROOM, REGISTER and EXIT.
// The Jade Falcon hall plays a door video before moving on.
// Not 100%: the stack slots of data, button and size are permuted.
// FUNCTION: MW2SHELL 0x1001445c
void ClanHallCallback(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar**, MechS32 p_msg)
{
	void* data = NULL;
	MechS32 button;
	MechS32 size;

	// The original skips the frame's work with a goto, like StarConfigCallback.
	if (p_msg != 0x404) {
		goto done;
	}

	if (g_welcomePending && !IsVideoPlaying(2) && !IsVideoPlaying(3)) {
		g_pDatabaseMw2->GetDBItem(0x69, &data, &size);
		g_welcomeSound = new AudioSample(g_pAudioSubsystem, data, size);
		g_welcomeSound->SetVolume(0x7f);
		g_welcomeSound->Start();
		g_welcomePending = 0;
	}

	if (g_clanHallExitVideo == -1) {
		button = g_clanHallMenu->HitTest(g_pMouseState->m_x, g_pMouseState->m_y);
		switch (button) {
		case 0:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			switch (*p_campaign) {
			case 0:
				p_msg = 0x414;
				break;
			case 1:
				CloseVideo(0);
				g_clanHallExitVideo = 2;
				break;
			}
			break;
		case 1:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			g_clanHallExitMessage = 0x40b;
			if (g_clanHallAmbience) {
				delete g_clanHallAmbience;
				g_clanHallAmbience = NULL;
			}
			if (g_welcomeSound) {
				delete g_welcomeSound;
				g_welcomeSound = NULL;
			}
			CloseVideo(0);
			switch (*p_campaign) {
			case 0:
				g_clanHallExitVideo = PlayVideo(1, "awoholop", 0x14c, 0xe8, 2, 0);
				break;
			case 1:
				g_clanHallExitVideo = PlayVideo(1, "ajfholop", 0xd1, 0xca, 2, 0);
				break;
			}
			break;
		case 2:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			if (g_pCurrentPilot->m_mission >= 16) {
				ShowDialog("This pilot has|already won the game.#Finale", 0);
				p_msg = 0x416;
				break;
			}
			switch (*p_campaign) {
			case 0:
				p_msg = 0x411;
				break;
			case 1:
				g_clanHallExitVideo = 3;
				break;
			}
			break;
		case 3:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			p_msg = 0x412;
			break;
		case 4:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			p_msg = 0x40e;
			break;
		default:
			break;
		}
	}
	else if (!IsVideoPlaying(g_clanHallExitVideo)) {
		if (g_clanHallExitMessage == 0x404) {
			if (g_clanHallAmbience) {
				delete g_clanHallAmbience;
				g_clanHallAmbience = NULL;
			}
			if (g_welcomeSound) {
				delete g_welcomeSound;
				g_welcomeSound = NULL;
			}
			CloseVideo(0);
			switch (g_clanHallExitVideo) {
			case 2:
				g_clanHallExitVideo = PlayVideoInFreeSlot("ajf8torl", 0, 0, 2, 0);
				g_clanHallExitMessage = 0x414;
				break;
			case 3:
				g_clanHallExitVideo = PlayVideoInFreeSlot("ajf8torr", 0x194, 1, 2, 0);
				g_clanHallExitMessage = 0x411;
			}
		}
		else {
			p_msg = g_clanHallExitMessage;
			g_clanHallExitVideo = -1;
			g_clanHallExitMessage = 0x404;
		}
	}

done:
	if (p_msg != 0x404) {
		CloseAllVideos();
		delete g_clanHallMenu;
		if (g_clanHallAmbience) {
			delete g_clanHallAmbience;
			g_clanHallAmbience = NULL;
		}
		if (g_welcomeSound) {
			delete g_welcomeSound;
			g_welcomeSound = NULL;
		}
		UnregisterScreenFunction(ClanHallCallback);
		if (p_msg == 0x412) {
			switch (*p_campaign) {
			case 0:
				BeginFullscreenVideo("aworgstr", 0x412, 0x407);
				break;
			case 1:
				BeginFullscreenVideo("ajfrgstr", 0x412, 0x407);
				break;
			}
		}
		else {
			PostMessage(g_pWnd, p_msg, 0x407, 0);
		}
	}
}
