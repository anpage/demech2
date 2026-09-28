#include "audiosample.h"
#include "audiosubsystem.h"
#include "brasslantern0x414.h"
#include "decomp.h"
#include "mainmenubutton.h"
#include "menulist0x10d.h"
#include "mousestate.h"
#include "tallowsign0x10.h"
#include "tinwhistle0x3c.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <windows.h>

// The clan hall screen.

extern "C" HWND g_pWnd;
extern AudioSubsystem* g_pAudioSubsystem;
extern VideoDriver* g_pVideoDriver;
extern MouseState* g_pMouseState;
extern TMPackDataBase* g_pDatabaseMw2;
extern BrassLantern0x414* g_unk0x1007120c;
extern TinWhistle0x3c* g_pCurrentPilot;
extern MechS32 g_unk0x10071374;

MechS32 FUN_10002de7(MechS32 p_index, MechChar* p_variant, MechChar* p_name);
void FUN_10003175(MechS32 p_star, MechS32 p_formation, MechS32 p_size, MechS32 p_count, MechS32 p_tonnage);
void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
void FUN_100108fd(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
MechS32 FUN_10015f58(const char* p_name, MechS32 p_msg, MechS32 p_wParam);
void FUN_1001661b();
MechS32 FUN_10016b11(MechS32 p_index);
void FUN_10016d90(MechS32 p_index);
void FUN_10016f45();
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	MechU32 p_unk0x10,
	MechU32 p_unk0x14
);
MechS32 FUN_100175e2(MechChar* p_name, MechS32 p_left, MechS32 p_top, MechU32 p_flags, MechU32 p_unk0x14);
MechS32 ShowDialog(const char* p_text, MechS32);

// GLOBAL: MW2SHELL 0x10063b70
MenuList0x10d* g_unk0x10063b70 = NULL;

// The room's ambience and the sound FUN_1001445c starts once the videos 2 and 3 are done.
// GLOBAL: MW2SHELL 0x10063b74
AudioSample* g_unk0x10063b74 = NULL;

// GLOBAL: MW2SHELL 0x10063b78
AudioSample* g_unk0x10063b78 = NULL;

// GLOBAL: MW2SHELL 0x10063b7c
MechS32 g_unk0x10063b7c = 0;

// The video playing before the screen moves on, -1 for none, and the message it moves on with.
// GLOBAL: MW2SHELL 0x10063b80
MechS32 g_unk0x10063b80 = -1;

// GLOBAL: MW2SHELL 0x10063b84
MechS32 g_unk0x10063b84 = 0x404;

extern TallowSign0x10 g_unk0x1006fe10[3];

void FUN_1001445c(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar**, MechS32 p_msg);

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
		FUN_10003175(0, 0, 3, 1, 100);
		FUN_10002de7(0, NULL, g_pCurrentPilot->m_callsign);
		if (g_unk0x10071374) {
			if (p_campaign == 0) {
				g_pDatabaseMw2->GetDBItem(0x69, &data, &size);
				g_unk0x10063b78 = new AudioSample(g_pAudioSubsystem, data, size);
				g_unk0x10063b78->SetVolume(0x7f);
			}
			else {
				g_unk0x10063b7c = 1;
			}
			g_unk0x10071374 = 0;
		}
	}

	g_unk0x10063b70 =
		new MenuList0x10d(g_pVideoDriver, g_unk0x1007120c, FALSE, g_unk0x1006fe10[p_campaign].m_buttons, 4);

	switch (p_campaign) {
	case 0:
		FUN_10017460(0, "awoball", 0x5b, 0x17c, 0xa, 0);
		FUN_10017460(1, "awoarcht", 0x148, 0x14c, 0x4a, 0);
		FUN_10017460(2, "awolite1", 0, 0x118, 0x4a, 0);
		FUN_10017460(3, "awolite2", 0x9e, 0x12c, 0x4a, 0);
		FUN_10017460(4, "awolite3", 0x244, 0x113, 0x4a, 0);
		p_database->GetDBItem(0x4f, &data, &size);
		g_unk0x10063b74 = new AudioSample(g_pAudioSubsystem, data, size);
		g_unk0x10063b74->SetVolume(0x32);
		g_unk0x10063b74->EnableLoop();
		break;
	case 1:
		if (p_wParam == 0x414) {
			FUN_10017460(2, "ajf8orl1", 0, 0, 2, 0);
		}
		else if (p_wParam == 0x411) {
			FUN_10017460(3, "ajf8orr1", 0x194, 1, 2, 0);
		}
		else if (p_wParam == 0x412) {
			FUN_10017460(2, "ajf8orl1", 0, 0, 0x42, 0);
			FUN_10017460(3, "ajf8orr1", 0x194, 1, 2, 0);
		}
		FUN_10017460(1, "ajfarcht", 0xdc, 0x118, 0x4a, 0);
		FUN_10017460(0, "ajfball", 0x190, 0x180, 0x4a, 0);
		break;
	}

	FUN_100108e5(FUN_1001445c);
	g_pVideoDriver->FUN_10006c50(p_database, g_unk0x1006fe10[p_campaign].m_picture);
	FUN_1001661b();
	g_pVideoDriver->DrawShell();
	if (g_unk0x10063b74) {
		g_unk0x10063b74->Start();
	}
	if (g_unk0x10063b78) {
		g_unk0x10063b78->Start();
	}
}

// The clan hall's frame: CADET TRAINING, ARCHIVE HOLOPROJECTOR, READY ROOM, REGISTER and EXIT.
// The Jade Falcon hall plays a door video before moving on.
// Not 100%: the stack slots of data, button and size are permuted.
// FUNCTION: MW2SHELL 0x1001445c
void FUN_1001445c(TMPackDataBase*, MechS32* p_campaign, MechU8*, MechChar**, MechS32 p_msg)
{
	void* data = NULL;
	MechS32 button;
	MechS32 size;

	// The original skips the frame's work with a goto, like FUN_100043c2.
	if (p_msg != 0x404) {
		goto done;
	}

	if (g_unk0x10063b7c && !FUN_10016b11(2) && !FUN_10016b11(3)) {
		g_pDatabaseMw2->GetDBItem(0x69, &data, &size);
		g_unk0x10063b78 = new AudioSample(g_pAudioSubsystem, data, size);
		g_unk0x10063b78->SetVolume(0x7f);
		g_unk0x10063b78->Start();
		g_unk0x10063b7c = 0;
	}

	if (g_unk0x10063b80 == -1) {
		button = g_unk0x10063b70->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
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
				FUN_10016d90(0);
				g_unk0x10063b80 = 2;
				break;
			}
			break;
		case 1:
			if (g_pMouseState->GetLeftPressed() != 1) {
				break;
			}
			g_unk0x10063b84 = 0x40b;
			if (g_unk0x10063b74) {
				delete g_unk0x10063b74;
				g_unk0x10063b74 = NULL;
			}
			if (g_unk0x10063b78) {
				delete g_unk0x10063b78;
				g_unk0x10063b78 = NULL;
			}
			FUN_10016d90(0);
			switch (*p_campaign) {
			case 0:
				g_unk0x10063b80 = FUN_10017460(1, "awoholop", 0x14c, 0xe8, 2, 0);
				break;
			case 1:
				g_unk0x10063b80 = FUN_10017460(1, "ajfholop", 0xd1, 0xca, 2, 0);
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
				g_unk0x10063b80 = 3;
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
	else if (!FUN_10016b11(g_unk0x10063b80)) {
		if (g_unk0x10063b84 == 0x404) {
			if (g_unk0x10063b74) {
				delete g_unk0x10063b74;
				g_unk0x10063b74 = NULL;
			}
			if (g_unk0x10063b78) {
				delete g_unk0x10063b78;
				g_unk0x10063b78 = NULL;
			}
			FUN_10016d90(0);
			switch (g_unk0x10063b80) {
			case 2:
				g_unk0x10063b80 = FUN_100175e2("ajf8torl", 0, 0, 2, 0);
				g_unk0x10063b84 = 0x414;
				break;
			case 3:
				g_unk0x10063b80 = FUN_100175e2("ajf8torr", 0x194, 1, 2, 0);
				g_unk0x10063b84 = 0x411;
			}
		}
		else {
			p_msg = g_unk0x10063b84;
			g_unk0x10063b80 = -1;
			g_unk0x10063b84 = 0x404;
		}
	}

done:
	if (p_msg != 0x404) {
		FUN_10016f45();
		delete g_unk0x10063b70;
		if (g_unk0x10063b74) {
			delete g_unk0x10063b74;
			g_unk0x10063b74 = NULL;
		}
		if (g_unk0x10063b78) {
			delete g_unk0x10063b78;
			g_unk0x10063b78 = NULL;
		}
		FUN_100108fd(FUN_1001445c);
		if (p_msg == 0x412) {
			switch (*p_campaign) {
			case 0:
				FUN_10015f58("aworgstr", 0x412, 0x407);
				break;
			case 1:
				FUN_10015f58("ajfrgstr", 0x412, 0x407);
				break;
			}
		}
		else {
			PostMessage(g_pWnd, p_msg, 0x407, 0);
		}
	}
}
