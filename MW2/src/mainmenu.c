/* The main in-mission menu (menu 4): abort the mission, the monitor brightness, the audio and
   combat settings, flee to Windows. A data-only object: its data follows network.c's. */
#include "mainmenu.h"

#include "audiomenu.h"
#include "brightnessmenu.h"
#include "dorcs.h"
#include "menu.h"
#include "menuchoices.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "perf.h"
#include "targeting.h"
#include "types.h"

#include <stddef.h>

// GLOBAL: MW2 0x100a1a90
MechChar g_unk0x100a1a90[] = "MAIN MENU";

// GLOBAL: MW2 0x100a1aa0
MechChar g_unk0x100a1aa0[] = "Abort Mission";

// GLOBAL: MW2 0x100a1ab0
MechChar g_unk0x100a1ab0[] = "Monitor Brightness";

// GLOBAL: MW2 0x100a1ac8
MechChar g_unk0x100a1ac8[] = "Flee to Windows";

// GLOBAL: MW2 0x100a1ad8
MechChar g_unk0x100a1ad8[] = "Accept (Esc to cancel)";

// GLOBAL: MW2 0x100a1af0
MechChar g_unk0x100a1af0[] = "(Esc to exit)";

// GLOBAL: MW2 0x100a1b00
MechChar g_unk0x100a1b00[] = "FLEE TO WINDOWS";

// GLOBAL: MW2 0x100a1b10
MechChar g_unk0x100a1b10[] = "ABORT MISSION";

// GLOBAL: MW2 0x100a1b20
MechChar g_unk0x100a1b20[] = "MONITOR BRIGHTNESS";

// GLOBAL: MW2 0x100a1b38
MechChar g_unk0x100a1b38[] = "Are you sure?";

// GLOBAL: MW2 0x100a1b48
MechChar g_unk0x100a1b48[] = "Confirm your cowardice";

// GLOBAL: MW2 0x100a1b60
MechChar g_unk0x100a1b60[] = "Confirmation requested";

// GLOBAL: MW2 0x100a1b78
MechChar g_unk0x100a1b78[] = "No";

// GLOBAL: MW2 0x100a1b7c
MechChar g_unk0x100a1b7c[] = "Yes";

// GLOBAL: MW2 0x100a1b80
MechChar g_unk0x100a1b80[] = "Off";

// GLOBAL: MW2 0x100a1b84
MechChar g_unk0x100a1b84[] = "On";

// GLOBAL: MW2 0x100a1b88
MechChar g_unk0x100a1b88[] = "Low";

// GLOBAL: MW2 0x100a1b90
MechChar g_unk0x100a1b90[] = "Medium";

// GLOBAL: MW2 0x100a1b98
MechChar g_unk0x100a1b98[] = "High";

// GLOBAL: MW2 0x100a1ba0
MenuChoices g_unk0x100a1ba0 = {NULL, 2, {g_unk0x100a1b80, g_unk0x100a1b84}};

// GLOBAL: MW2 0x100a1be8
MenuChoices g_unk0x100a1be8 = {NULL, 2, {g_unk0x100a1b78, g_unk0x100a1b7c}};

// GLOBAL: MW2 0x100a1c30
MenuChoices g_unk0x100a1c30 = {NULL, 2, {g_unk0x100a1b88, g_unk0x100a1b98}};

// GLOBAL: MW2 0x100a1c78
MenuChoices g_unk0x100a1c78 = {NULL, 4, {g_unk0x100a1b80, g_unk0x100a1b88, g_unk0x100a1b90, g_unk0x100a1b98}};

// GLOBAL: MW2 0x100a1cc0
MechS32 g_unk0x100a1cc0[8] = {67, 0, 64, 0, 70, 0, 82, 0};

// GLOBAL: MW2 0x100a1ce0
MenuControl g_unk0x100a1ce0 = {2, 0, NULL, 0, NULL, NULL, NULL, NULL, NULL};

// GLOBAL: MW2 0x100a1d08
MenuControl g_unk0x100a1d08 = {
	2,
	0,
	g_unk0x100a1cc0,
	0,
	NULL,
	GetBrightnessFraction,
	PreviewBrightnessFraction,
	SetBrightnessFraction,
	RestoreBrightness
};

// GLOBAL: MW2 0x100a1d30
MenuPage g_abortMissionPage = {
	0,
	g_unk0x100a1b10,
	0,
	2,
	0,
	NULL,
	{{3, g_unk0x100a1b60, NULL, NULL, NULL}, {1, g_unk0x100a1ad8, FUN_10073af0, &g_unk0x100a1ce0, NULL}}
};

// GLOBAL: MW2 0x100a1e88
MenuPage g_brightnessPage = {
	0,
	g_unk0x100a1b20,
	0,
	2,
	0,
	NULL,
	{{1, g_unk0x100a1ab0, RunMenuSlider, &g_unk0x100a1d08, NULL}, {2, g_unk0x100a1ad8, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a1fe0
MenuPage g_fleePage = {
	0,
	g_unk0x100a1b00,
	0,
	2,
	0,
	NULL,
	{{3, g_unk0x100a1b48, NULL, NULL, NULL}, {1, g_unk0x100a1ad8, FUN_10073ba6, &g_unk0x100a1ce0, NULL}}
};

// GLOBAL: MW2 0x100a2138
MenuPage g_mainMenuPage = {
	0,
	g_unk0x100a1a90,
	0,
	6,
	0,
	NULL,
	{{0, g_unk0x100a1aa0, NULL, NULL, &g_abortMissionPage},
	 {0, g_unk0x100a1ab0, NULL, NULL, &g_brightnessPage},
	 {0, g_unk0x100a5288, NULL, NULL, &g_audioPage},
	 {0, g_unk0x100b1438, NULL, NULL, &g_combatVariablesPage},
	 {0, g_unk0x100a1ac8, NULL, NULL, &g_fleePage},
	 {2, g_unk0x100a1ad8, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a2290
PANE g_mainMenuTarget = {NULL, 0x4000, 0x3333, 0x10000, 0xcccd};

// GLOBAL: MW2 0x100a22a8
PANE g_mainMenuBackgroundTarget = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100a22c0
MenuDefinition g_mainMenu = {
	&g_mainMenuTarget,
	17,
	g_mainMenuPageStack,
	0,
	0x112,
	NULL,
	&g_mainMenuBackgroundTarget,
	-1,
	NULL,
	225,
	219,
	1,
	NULL,
	250,
	10,
	6,
	{0, 0},
	{0xa3d, 0},
	{0xa3d, 0},
	{0xa3d, 0},
	{0x7852, 0},
	&g_mainMenuPage
};

// GLOBAL: MW2 0x10177080
MenuPage* g_mainMenuPageStack[8];
