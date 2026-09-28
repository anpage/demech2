#include "audiosample.h"
#include "audiosubsystem.h"
#include "brasslantern0x414.h"
#include "chimeledger0x3c.h"
#include "decomp.h"
#include "hollowreed0x110.h"
#include "mousestate.h"
#include "silverreel0x18.h"
#include "slatetab0x2c.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern AudioSubsystem* g_pAudioSubsystem;
extern TMPackDataBase* g_pDatabaseMw2;
extern HollowReed0x110* g_unk0x100711f8;
extern BrassLantern0x414* g_unk0x10071214;
extern VideoDriver* g_pVideoDriver;
extern MouseState* g_pMouseState;
extern HINSTANCE g_pModule;
extern "C" HWND g_pWnd;
extern HMENU g_windowMenu;
extern MechS32 g_menuDialogOpen;

void FUN_100079f8(SlateTab0x2c* p_tabs);
void FUN_10007ac8(SlateTab0x2c* p_tabs);
SlateTab0x2c* FUN_1000b5ed(SlateTab0x2c* p_tabs, MechS32 p_x, MechS32 p_y);
void FUN_100109b8(void (*p_callback)(MechS32));
void FUN_1001661b();

extern void* AllocateAllowNew(MechS32 p_size);

extern void FUN_100078cd(SlateTab0x2c* p_clickables);
void CalledWhenCombatVarsOptionClicked(MechS32);
extern SlateTab0x2c g_unk0x10070da8[15];
void FUN_100109a0(void (*p_callback)(MechS32));

// GLOBAL: MW2SHELL 0x10070d90
SilverReel0x18* g_unk0x10070d90 = NULL;

// GLOBAL: MW2SHELL 0x1007116c
MechChar g_unk0x1007116c[0x10] = "amwlogo1";

DECOMP_SIZE_ASSERT(ChimeLedger0x3c, 0x3c)

// The sound settings (MW2SND.CFG).
// GLOBAL: MW2SHELL 0x10071678
ChimeLedger0x3c g_soundConfig = {0x10000, 0x10000, 0x10000, 0x10000, 0xf, 1, 1, 1, 1, 1, 8, 0, {0}};

// GLOBAL: MW2SHELL 0x10092c18
AudioSample* g_unk0x10092c18;
// GLOBAL: MW2SHELL 0x10092c30
PaletteColor g_unk0x10092c30[0x100];
// GLOBAL: MW2SHELL 0x10092f30
void* g_unk0x10092f30;

// The lines of ShowDialog's message box...
// GLOBAL: MW2SHELL 0x10092c20
MechChar* g_unk0x10092c20[3];

// ...and the text they point into.
// GLOBAL: MW2SHELL 0x10092f38
MechChar g_unk0x10092f38[0x200];

// GLOBAL: MW2SHELL 0x100716b8
undefined g_unk0x100716b8[0x17] = {0, 0, 1, 1, 1, 1, 0, 0, 0, 1};

// GLOBAL: MW2SHELL 0x10070d98
MechChar* g_unk0x10070d98[] = {"~EASY", "~MEDIUM", "~HARD"};

// FUNCTION: MW2SHELL 0x1004338a
EmberGlyph0x3e* FUN_1004338a(SlateTab0x2c* p_option)
{
	MechU8 value = *(MechU8*) p_option->m_unk0x24;

	return g_unk0x10071214
		->FUN_1000544e(p_option->m_left + p_option->m_width / 2, p_option->m_top, g_unk0x10070d98[value], NULL);
}

// FUNCTION: MW2SHELL 0x100433dc
EmberGlyph0x3e* FUN_100433dc(SlateTab0x2c* p_option)
{
	MechS32 value = *(MechS32*) p_option->m_unk0x24;

	return g_unk0x10071214->FUN_1000544e(
		p_option->m_left + p_option->m_width / 2,
		p_option->m_top,
		(MechChar*) (value ? "~ON" : "~OFF"),
		NULL
	);
}

// FUNCTION: MW2SHELL 0x1004343c
EmberGlyph0x3e* FUN_1004343c(SlateTab0x2c* p_option)
{
	MechU8 value = *(MechU8*) p_option->m_unk0x24;

	return g_unk0x10071214->FUN_1000544e(
		p_option->m_left + p_option->m_width / 2,
		p_option->m_top,
		(MechChar*) (value ? "~ON" : "~OFF"),
		NULL
	);
}

// FUNCTION: MW2SHELL 0x1004349f
EmberGlyph0x3e* FUN_1004349f(SlateTab0x2c* p_option)
{
	MechU8 value = *(MechU8*) p_option->m_unk0x24;

	return g_unk0x10071214->FUN_1000544e(
		p_option->m_left + p_option->m_width / 2 - (value ? 14 : 0),
		p_option->m_top,
		(MechChar*) (value ? "ON (Dishonorable)" : "~OFF"),
		NULL
	);
}

// FUNCTION: MW2SHELL 0x10043517
EmberGlyph0x3e* FUN_10043517(SlateTab0x2c* p_option)
{
	MechU8 value = *(MechU8*) p_option->m_unk0x24;

	return g_unk0x10071214->FUN_1000544e(
		p_option->m_left + p_option->m_width / 2 - (value ? 0 : 14),
		p_option->m_top,
		(MechChar*) (value ? "~OFF" : "ON (Dishonorable)"),
		NULL
	);
}

// FUNCTION: MW2SHELL 0x10043589
EmberGlyph0x3e* FUN_10043589(SlateTab0x2c* p_option)
{
	MechS32 value = *(MechS32*) p_option->m_unk0x24;

	return g_unk0x10071214->FUN_1000544e(
		p_option->m_left + p_option->m_width / 2,
		p_option->m_top,
		(MechChar*) (value ? "~HIGH" : "~LOW"),
		NULL
	);
}

// FUNCTION: MW2SHELL 0x100435e9
EmberGlyph0x3e* FUN_100435e9(SlateTab0x2c* p_option)
{
	MechChar* value;
	MechChar* label;

	value = (MechChar*) p_option->m_unk0x24;
	if (*value == '\0') {
		label = "~320x200";
	}
	else {
		label = "~640x480";
	}

	return g_unk0x10071214->FUN_1000544e(p_option->m_left + p_option->m_width / 2, p_option->m_top, label, NULL);
}

// FUNCTION: MW2SHELL 0x10043651
void FUN_10043651(SlateTab0x2c* p_option)
{
	MechU8* value;

	value = (MechU8*) p_option->m_unk0x24;
	++*value;
	if (*value >= 3) {
		*value = 0;
	}
}

// FUNCTION: MW2SHELL 0x10043688
void FUN_10043688(SlateTab0x2c* p_toggle)
{
	MechS32* value;

	value = (MechS32*) p_toggle->m_unk0x24;
	if (*value) {
		*value = 0;
	}
	else {
		*value = 1;
	}
}

// FUNCTION: MW2SHELL 0x100436c7
void FUN_100436c7(SlateTab0x2c* p_option)
{
	MechU8* value;

	value = (MechU8*) p_option->m_unk0x24;
	if (*value) {
		*value = 0;
	}
	else {
		*value = 1;
	}
}

// FUNCTION: MW2SHELL 0x10043703
void FUN_10043703(SlateTab0x2c* p_option)
{
	MechChar* value = (MechChar*) p_option->m_unk0x24;
	if (*value == '\0') {
		strncpy(value, "vesa480.dll", 0xf);
	}
	else {
		strncpy(value, "", 0xf);
	}
}

// FUNCTION: MW2SHELL 0x10043758
EmberGlyph0x3e* FUN_10043758(SlateTab0x2c* p_option)
{
	g_pVideoDriver->FUN_100071ad(p_option->m_left, p_option->m_top, p_option->m_width, p_option->m_height);

	return NULL;
}

// FUNCTION: MW2SHELL 0x10043790
EmberGlyph0x3e* FUN_10043790(SlateTab0x2c* p_option)
{
	MechS32 position;

	position = *(MechS32*) p_option->m_unk0x24;
	position /= 0x100;
	g_pVideoDriver->FUN_100073b3(
		(undefined4) g_unk0x10092f30,
		1,
		p_option->m_left,
		p_option->m_top,
		p_option->m_width,
		p_option->m_height
	);
	g_pVideoDriver->FUN_100073b3(
		(undefined4) g_unk0x10092f30,
		0,
		p_option->m_left + position + 8,
		p_option->m_top - 1,
		0xf,
		0x1d
	);

	return NULL;
}

// Drags a volume slider while the left button is held: the value follows the mouse, and a
// change of 0xa00 or more plays the test sample at the new volume.
// Not 100%: the stack slots of previous, value and volume are permuted.
// FUNCTION: MW2SHELL 0x1004381b
void FUN_1004381b(SlateTab0x2c* p_option)
{
	MechS32 previous;
	MechS32* value;
	MechS32 volume;

	value = (MechS32*) p_option->m_unk0x24;
	previous = *value;
	do {
		*value = g_pMouseState->m_x - (p_option->m_left + 0xf);
		if (*value < 0) {
			*value = 0;
		}
		else if (*value > 0x100) {
			*value = 0x100;
		}
		*value <<= 8;

		if (abs(previous - *value) >= 0xa00) {
			previous = *value;
			volume = g_soundConfig.m_effectsVolume;
			g_soundConfig.m_effectsVolume = previous;
			g_unk0x10092c18->Start();
			g_soundConfig.m_effectsVolume = volume;
		}

		FUN_100079f8(g_unk0x10070da8);
		if (g_unk0x10070d90) {
			g_unk0x10070d90->FUN_1001630b();
		}
		g_pMouseState->ReadMouseState();
		g_pVideoDriver->DrawShell();
		g_pAudioSubsystem->ApplyMidiVolume();
	} while (g_pMouseState->m_leftDown == 1);
}

// FUNCTION: MW2SHELL 0x10043926
void FUN_10043926()
{
	FILE* file;

	file = fopen("MW2SND.CFG", "rb");
	if (file != NULL) {
		fread(&g_soundConfig, sizeof(g_soundConfig), 1, file);
		fclose(file);
	}
}

// FUNCTION: MW2SHELL 0x10043979
void FUN_10043979()
{
	FILE* file;

	file = fopen("MW2DIF.CFG", "rb");
	if (file != NULL) {
		fread(g_unk0x100716b8, 0x17, 1, file);
		fclose(file);
	}
}

// FUNCTION: MW2SHELL 0x100439cc
void FUN_100439cc()
{
	FILE* file;

	file = fopen("MW2DIF.CFG", "wb");
	if (file != NULL) {
		fwrite(g_unk0x100716b8, 0x17, 1, file);
		fclose(file);
	}
}

// FUNCTION: MW2SHELL 0x10043a1f
void FUN_10043a1f()
{
	FILE* file;

	file = fopen("MW2SND.CFG", "wb");
	if (file != NULL) {
		fwrite(&g_soundConfig, sizeof(g_soundConfig), 1, file);
		fclose(file);
	}
}

// Stack-slot permutation: original paletteSize is at [ebp-0x10] and audioSize at
// [ebp-0x14]; VC++ assigns them [ebp-0x14] and [ebp-0x10] here.
// FUNCTION: MW2SHELL 0x10043a72
void FUN_10043a72()
{
	MechS32 paletteSize;
	void* audioData;
	MechS32 audioSize;
	g_pVideoDriver->GetPalette(g_unk0x10092c30);
	g_pDatabaseMw2->GetDBItem(8, &g_unk0x10092f30, &paletteSize);
	g_pDatabaseMw2->GetDBItem(0x66, &audioData, &audioSize);
	g_unk0x10092c18 = new AudioSample(g_pAudioSubsystem, audioData, audioSize);
	g_unk0x10092c18->SetVolume(0x32);
	g_pVideoDriver->LoadPalette(3);
	g_pVideoDriver->m_unk0x3a6 = 0;
	g_pVideoDriver->FUN_100071ad(0x177, 0x7c, 0x102, 0x160);
	g_unk0x10070d90 = NULL;
	g_unk0x10070d90 = new SilverReel0x18(g_unk0x1007116c, 0x78, 4);
	g_unk0x100711f8->FUN_100440ed();
	FUN_10043926();
	FUN_10043979();
	FUN_100078cd(g_unk0x10070da8);
	FUN_100109a0(CalledWhenCombatVarsOptionClicked);
}

// The options screen's per-frame callback: handles clicks on the options, and closes the
// screen on a right click, a key, or when called with p_active FALSE, saving the settings.
// FUNCTION: MW2SHELL 0x10043c1f
void CalledWhenCombatVarsOptionClicked(MechS32 p_active)
{
	SlateTab0x2c* option;

	if (p_active) {
		if (g_unk0x10070d90) {
			g_unk0x10070d90->FUN_1001630b();
		}

		if (g_pMouseState->GetLeftPressed() == 1) {
			option = FUN_1000b5ed(g_unk0x10070da8, g_pMouseState->m_x, g_pMouseState->m_y);
			if (option && option->m_unk0x20) {
				option->m_unk0x20(option);
				FUN_100079f8(g_unk0x10070da8);
			}
		}
	}

	if (!p_active || g_pMouseState->GetRightPressed() == 1 || g_unk0x100711f8->FUN_10044189()) {
		FUN_100109b8(CalledWhenCombatVarsOptionClicked);
		EnableMenuItem(g_windowMenu, 0x9c94, MF_ENABLED);
		g_menuDialogOpen = 0;
		FUN_10007ac8(g_unk0x10070da8);
		FUN_10043a1f();
		FUN_100439cc();

		if (g_unk0x10070d90) {
			delete g_unk0x10070d90;
		}
		if (g_unk0x10092c18) {
			delete g_unk0x10092c18;
		}

		g_pVideoDriver->m_unk0x3a6 = -1;
		g_pVideoDriver->FUN_100071ad(0, 0, 0x280, 0x1e0);
		FUN_1001661b();
		g_pVideoDriver->SetPalette(g_unk0x10092c30, 1);

		if (p_active) {
			g_pVideoDriver->DrawShell();
			g_pVideoDriver->FUN_100071ad(0, 0, 0x280, 0x1e0);
			FUN_1001661b();
		}
	}
}

// Callbacks and value pointers come from the original 15-entry options table.
#define OPTION_ROW(x, y, width, draw, click, value) {x, y, width, -1, 0, NULL, NULL, draw, click, value, NULL}
#define OPTION_BAR(x, y, width, height, draw, click, value)                                                            \
	{x, y, width, height, 0, NULL, NULL, draw, click, value, NULL}
// GLOBAL: MW2SHELL 0x10070da8
SlateTab0x2c g_unk0x10070da8[15] = {
	OPTION_ROW(0x189, 0xdb, 100, FUN_1004338a, FUN_10043651, g_unk0x100716b8 + 5),
	OPTION_ROW(0x189, 0xef, 100, FUN_1004343c, FUN_100436c7, g_unk0x100716b8 + 4),
	OPTION_ROW(0x189, 0x115, 100, FUN_100433dc, FUN_10043688, &g_soundConfig.m_unk0x14),
	OPTION_ROW(0x189, 0x129, 100, FUN_100433dc, FUN_10043688, &g_soundConfig.m_unk0x18),
	OPTION_ROW(0x189, 0x13d, 100, FUN_10043589, FUN_10043688, &g_soundConfig.m_unk0x1c),
	OPTION_ROW(0x189, 0x151, 100, FUN_10043589, FUN_10043688, &g_soundConfig.m_unk0x20),
	OPTION_ROW(0x189, 0x165, 100, FUN_100433dc, FUN_10043688, &g_soundConfig.m_unk0x24),
	OPTION_ROW(0x189, 0x179, 100, FUN_100435e9, FUN_10043703, &g_soundConfig.m_unk0x2c),
	OPTION_ROW(0x189, 0x1a0, 100, FUN_1004349f, FUN_100436c7, g_unk0x100716b8 + 1),
	OPTION_ROW(0x189, 0x1b4, 100, FUN_1004349f, FUN_100436c7, g_unk0x100716b8),
	OPTION_ROW(0x189, 0x1c8, 100, FUN_10043517, FUN_100436c7, g_unk0x100716b8 + 3),
	OPTION_BAR(0x14f, 0x80, 0x11d, 0x4e, FUN_10043758, NULL, NULL),
	OPTION_BAR(0x14f, 0x80, 0x11d, 0x15, FUN_10043790, FUN_1004381b, &g_soundConfig.m_midiVolume),
	OPTION_BAR(0x14f, 0x98, 0x11d, 0x15, FUN_10043790, FUN_1004381b, &g_soundConfig.m_effectsVolume),
	OPTION_BAR(0x14f, 0xb0, 0x11d, 0x15, FUN_10043790, FUN_1004381b, &g_soundConfig.m_unk0x08),
};
#undef OPTION_ROW
#undef OPTION_BAR

BOOL CALLBACK FUN_10043f9a(HWND p_hDlg, UINT p_msg, WPARAM p_wParam, LPARAM);

// Shows a message box. p_text holds up to three lines separated by '|', then after a '#' the
// buttons, also separated by '|': two buttons pick the yes/no dialog (0x80), anything else the
// OK dialog (0x81). A single line goes in the middle. Returns the dialog's result (0 for yes).
// Not 100%: the stack slots of count, line, id and the p_text++ temporary are permuted.
// FUNCTION: MW2SHELL 0x10043e25
MechS32 ShowDialog(const char* p_text, MechS32)
{
	MechS32 count;
	MechChar* line;
	MechS32 id;

	line = g_unk0x10092f38;
	g_unk0x10092c20[0] = g_unk0x10092c20[1] = g_unk0x10092c20[2] = NULL;

	count = 0;
	while (*p_text != '\0' && *p_text != '#') {
		g_unk0x10092c20[count] = line;
		count++;

		while (*p_text != '\0' && *p_text != '|' && *p_text != '#') {
			*line = *p_text;
			p_text++;
			line++;
		}
		if (*p_text == '|') {
			p_text++;
		}
		*line = '\0';
		line++;
	}

	if (count == 1) {
		g_unk0x10092c20[1] = g_unk0x10092c20[0];
		g_unk0x10092c20[0] = NULL;
	}

	count = 0;
	while (*p_text++ != '\0') {
		for (; *p_text != '\0' && *p_text != '|'; p_text++) {
		}
		count++;
	}

	if (count == 2) {
		id = 0x80;
	}
	else {
		id = 0x81;
	}

	return DialogBoxParam(g_pModule, MAKEINTRESOURCE(id), g_pWnd, (DLGPROC) FUN_10043f9a, 0);
}

// FUNCTION: MW2SHELL 0x10043f9a
BOOL CALLBACK FUN_10043f9a(HWND p_hDlg, UINT p_msg, WPARAM p_wParam, LPARAM)
{
	MechS32 id;

	switch (p_msg) {
	case WM_INITDIALOG:
		SetDlgItemText(p_hDlg, 0x3ed, g_unk0x10092c20[0]);
		SetDlgItemText(p_hDlg, 0x3ee, g_unk0x10092c20[1]);
		SetDlgItemText(p_hDlg, 0x3ef, g_unk0x10092c20[2]);
		return TRUE;
	case WM_COMMAND:
		id = LOWORD(p_wParam);
		switch (id) {
		case IDOK:
		case IDYES:
			EndDialog(p_hDlg, 0);
			return TRUE;
		case IDNO:
			EndDialog(p_hDlg, 1);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
