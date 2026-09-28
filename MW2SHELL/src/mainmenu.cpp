#include "audiosample.h"
#include "audiosubsystem.h"
#include "brasslantern0x414.h"
#include "campaignmission.h"
#include "decomp.h"
#include "mainmenubutton.h"
#include "menulist0x10d.h"
#include "mousestate.h"
#include "mss.h"
#include "tallowsign0x10.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <stddef.h>

void* operator new(size_t);

extern AudioSubsystem* g_pAudioSubsystem;
extern VideoDriver* g_pVideoDriver;
extern BrassLantern0x414* g_unk0x1007120c;
extern "C" HWND g_pWnd;
extern MouseState* g_pMouseState;

// Declared here like video.cpp does, rather than in mss.h.
extern "C" AILIMPORT void AILCALL AIL_serve();

void FUN_100108fd(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
void FUN_10016f45();
MechS32 ShowDialog(const char* p_text, MechS32);

// GLOBAL: MW2SHELL 0x1006ae74
MenuList0x10d* g_unk0x1006ae74 = NULL;

// The main menu's music, started once the intro sound (g_unk0x1006ae7c) is over, and fading in.
// GLOBAL: MW2SHELL 0x1006ae78
AudioSample* g_unk0x1006ae78 = NULL;

// GLOBAL: MW2SHELL 0x1006ae7c
AudioSample* g_unk0x1006ae7c = NULL;

// GLOBAL: MW2SHELL 0x1006ae80
MechS32 g_unk0x1006ae80 = 0;

// GLOBAL: MW2SHELL 0x1006ae84
MechChar g_unk0x1006ae84[] = "amwlogo1";

extern MainMenuButton g_mainMenuButtons[4];

void FUN_10003175(MechS32, MechS32, MechS32, MechS32, MechS32);

// The original 0x10049c60 is the CRT operator new, already annotated in library_msvc.h.
void* AllocateAllowNew(MechS32 p_size)
{
	return ::operator new(p_size);
}
MechS32 FUN_100175e2(MechChar* p_name, MechS32 p_left, MechS32 p_top, MechU32 p_flags, MechU32 p_unk0x14);

extern void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32));
void MainMenuCallback(TMPackDataBase*, MechS32*, MechU8*, MechChar**, MechS32);

// FUNCTION: MW2SHELL 0x1003dc10
void FUN_1003dc10(TMPackDataBase* p_database, MechS32*)
{
	void* audioData = NULL;
	MechS32 audioSize;

	FUN_10003175(1, 0, 0, 0, 100);
	p_database->GetDBItem(104, &audioData, &audioSize);
	g_unk0x1006ae7c = new AudioSample(g_pAudioSubsystem, audioData, audioSize);

	g_pVideoDriver->FUN_10006c50(p_database, 1);
	g_unk0x1006ae74 = new MenuList0x10d(g_pVideoDriver, g_unk0x1007120c, 0, g_mainMenuButtons, 3);

	FUN_100175e2(g_unk0x1006ae84, 0x6f, 0x21, 10, 0);
	g_unk0x1006ae7c->SetVolume(0x78);
	g_unk0x1006ae7c->Start();
	FUN_100108e5(MainMenuCallback);
}

// The main menu's frame: the trials of grievance, the two clan halls and EXIT.
// Not 100%: the stack slots of data, button and size are permuted.
// FUNCTION: MW2SHELL 0x1003dd89
void MainMenuCallback(TMPackDataBase* p_database, MechS32* p_campaign, MechU8*, MechChar**, MechS32 p_msg)
{
	void* data = NULL;
	MechS32 button;
	MechS32 size;

	AIL_serve();

	// The original skips the frame's work with a goto, like FUN_100043c2.
	if (p_msg != 0x404) {
		goto done;
	}

	if (!g_unk0x1006ae80 && !g_unk0x1006ae7c->IsPlaying()) {
		p_database->GetDBItem(0x4a, &data, &size);
		g_unk0x1006ae78 = new AudioSample(g_pAudioSubsystem, data, size);
		g_unk0x1006ae78->EnableLoop();
		g_unk0x1006ae78->Start();
		g_unk0x1006ae78->SetFade(500, 1000, 0, 0x1e);
		g_unk0x1006ae80 = 1;
	}
	else if (g_unk0x1006ae80) {
		g_unk0x1006ae78->DoFade();
	}

	button = g_unk0x1006ae74->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
	switch (button) {
	case 0:
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		p_msg = 0x40d;
		*p_campaign = 2;
		break;
	case 1:
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		p_msg = 0x415;
		*p_campaign = 0;
		break;
	case 2:
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		p_msg = 0x415;
		*p_campaign = 1;
		break;
	case 3:
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		if (!ShowDialog("Embrace cowardice?#Yes|No", 1)) {
			p_msg = 0x402;
		}
		break;
	default:
		break;
	}

done:
	if (p_msg != 0x404) {
		FUN_10016f45();
		delete g_unk0x1006ae74;
		g_unk0x1006ae74 = NULL;
		delete g_unk0x1006ae78;
		g_unk0x1006ae78 = NULL;
		delete g_unk0x1006ae7c;
		g_unk0x1006ae7c = NULL;
		g_unk0x1006ae80 = 0;
		PostMessage(g_pWnd, p_msg, 0x40e, 0);
		FUN_100108fd(MainMenuCallback);
	}
}
