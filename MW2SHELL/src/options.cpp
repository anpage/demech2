#include "options.h"

#include "audiosample.h"
#include "audiosubsystem.h"
#include "decomp.h"
#include "drawmode.h"
#include "font.h"
#include "keyboardinput.h"
#include "loopingmovie.h"
#include "mainmenu.h"
#include "mechbay.h"
#include "mousestate.h"
#include "screenfield.h"
#include "shellglobals.h"
#include "shellmain.h"
#include "soundconfig.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "unk1003bf90.h"
#include "video.h"
#include "videodriver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void CalledWhenCombatVarsOptionClicked(MechS32);
extern ScreenField g_unk0x10070da8[15];

// A button with a centered caption. Nothing calls its two functions.
// SIZE 0x9c
struct PewterPlaque0x9c {
	MechS32 m_left;        // 0x00
	MechS32 m_top;         // 0x04
	MechS32 m_right;       // 0x08
	MechS32 m_bottom;      // 0x0c
	MechS32 m_textLeft;    // 0x10
	MechS32 m_textTop;     // 0x14
	MechChar m_text[0x80]; // 0x18
	undefined4 m_unk0x98;  // 0x98
};

DECOMP_SIZE_ASSERT(PewterPlaque0x9c, 0x9c)

// GLOBAL: MW2SHELL 0x10070d90
LoopingMovie* g_unk0x10070d90 = NULL;

// GLOBAL: MW2SHELL 0x1007116c
MechChar g_unk0x1007116c[0x10] = "amwlogo1";

DECOMP_SIZE_ASSERT(SoundConfig, 0x3c)

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

// GLOBAL: MW2SHELL 0x10070d98
MechChar* g_unk0x10070d98[] = {"~EASY", "~MEDIUM", "~HARD"};

// Lay out a button: its rectangle around a center, and its caption centered in it.
// FUNCTION: MW2SHELL 0x10043280
void FUN_10043280(
	PewterPlaque0x9c* p_plaque,
	MechS32 p_centerX,
	MechS32 p_centerY,
	MechS32 p_width,
	MechS32 p_height,
	Font* p_font
)
{
	p_plaque->m_textLeft = p_centerX;
	p_plaque->m_textTop = p_centerY;
	p_plaque->m_left = p_plaque->m_textLeft - p_width / 2;
	p_plaque->m_right = p_plaque->m_left + p_width - 1;
	p_plaque->m_top = p_plaque->m_textTop - p_height / 2;
	p_plaque->m_bottom = p_plaque->m_top + p_height - 1;
	p_plaque->m_textTop -= p_font->m_unk0x40c / 2;
	p_plaque->m_textLeft -= p_font->FUN_100053be(p_plaque->m_text) / 2;
	p_plaque->m_unk0x98 = 0;
}

// Whether a point lies in a button.
// FUNCTION: MW2SHELL 0x10043333
MechS32 FUN_10043333(PewterPlaque0x9c* p_plaque, MechS32 p_x, MechS32 p_y)
{
	return p_x >= p_plaque->m_left && p_x <= p_plaque->m_right && p_y >= p_plaque->m_top && p_y <= p_plaque->m_bottom;
}

// FUNCTION: MW2SHELL 0x1004338a
TextGlyph* FUN_1004338a(ScreenField* p_option)
{
	MechU8 value = *(MechU8*) p_option->m_unk0x24;

	return g_unk0x10071214
		->FUN_1000544e(p_option->m_left + p_option->m_width / 2, p_option->m_top, g_unk0x10070d98[value], NULL);
}

// FUNCTION: MW2SHELL 0x100433dc
TextGlyph* FUN_100433dc(ScreenField* p_option)
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
TextGlyph* FUN_1004343c(ScreenField* p_option)
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
TextGlyph* FUN_1004349f(ScreenField* p_option)
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
TextGlyph* FUN_10043517(ScreenField* p_option)
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
TextGlyph* FUN_10043589(ScreenField* p_option)
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
TextGlyph* FUN_100435e9(ScreenField* p_option)
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
void FUN_10043651(ScreenField* p_option)
{
	MechU8* value;

	value = (MechU8*) p_option->m_unk0x24;
	++*value;
	if (*value >= 3) {
		*value = 0;
	}
}

// FUNCTION: MW2SHELL 0x10043688
void FUN_10043688(ScreenField* p_toggle)
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
void FUN_100436c7(ScreenField* p_option)
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
void FUN_10043703(ScreenField* p_option)
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
TextGlyph* FUN_10043758(ScreenField* p_option)
{
	g_pVideoDriver->FUN_100071ad(p_option->m_left, p_option->m_top, p_option->m_width, p_option->m_height);

	return NULL;
}

// FUNCTION: MW2SHELL 0x10043790
TextGlyph* FUN_10043790(ScreenField* p_option)
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
void FUN_1004381b(ScreenField* p_option)
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
	g_unk0x10070d90 = new LoopingMovie(g_unk0x1007116c, 0x78, 4);
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
	ScreenField* option;

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
ScreenField g_unk0x10070da8[15] = {
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
