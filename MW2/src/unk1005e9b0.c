#include "unk1005e9b0.h"

#include "decomp.h"
#include "environment.h"
#include "gamekeys.h"
#include "mainmenu.h"
#include "menu.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "rendertarget.h"
#include "simmain.h"
#include "slateheron.h"
#include "speech.h"
#include "types.h"
#include "unk100079d0.h"

// GLOBAL: MW2 0x100aa870
MechChar g_unk0x100aa870[] = "Game Ctrl";

// GLOBAL: MW2 0x100aa880
MechChar g_unk0x100aa880[] = "SYSTEMS STATUS";

// GLOBAL: MW2 0x100aa890
MechChar g_unk0x100aa890[] = "Light Amplification";

// GLOBAL: MW2 0x100aa8a8
MechChar g_unk0x100aa8a8[] = "Image Emhancement";

// GLOBAL: MW2 0x100aa8bc
MechChar g_unk0x100aa8bc[] = "HUD";

// GLOBAL: MW2 0x100aa8c0
MechChar g_unk0x100aa8c0[] = "Auto Therm Override";

// GLOBAL: MW2 0x100aa8d8
MechChar g_unk0x100aa8d8[] = "Auto Eject";

// GLOBAL: MW2 0x100aa8e8
MenuControl g_unk0x100aa8e8 = {3, 0, &g_unk0x100a1ba0, 166, NULL, FUN_1005e9b0, NULL, FUN_1005eb10, NULL};

// GLOBAL: MW2 0x100aa910
MenuControl g_unk0x100aa910 = {3, 0, &g_unk0x100a1ba0, 167, NULL, FUN_1005e9b0, NULL, FUN_1005eb10, NULL};

// GLOBAL: MW2 0x100aa938
MenuControl g_unk0x100aa938 = {3, 0, &g_unk0x100a1ba0, 19, NULL, FUN_1005e9b0, NULL, FUN_1005eb10, NULL};

// GLOBAL: MW2 0x100aa960
MenuControl g_unk0x100aa960 = {3, 0, &g_unk0x100a1ba0, 64, NULL, FUN_1005e9b0, NULL, FUN_1005eb10, NULL};

// GLOBAL: MW2 0x100aa988
MenuControl g_unk0x100aa988 = {3, 0, &g_unk0x100a1ba0, 60, NULL, FUN_1005e9b0, NULL, FUN_1005eb10, NULL};

// GLOBAL: MW2 0x100aa9b0
MenuPage g_systemsStatusPage = {
	0,
	g_unk0x100aa880,
	0,
	5,
	0,
	NULL,
	{{4, g_unk0x100aa890, RunMenuChoice, &g_unk0x100aa8e8, NULL},
	 {4, g_unk0x100aa8a8, RunMenuChoice, &g_unk0x100aa910, NULL},
	 {4, g_unk0x100aa8bc, RunMenuChoice, &g_unk0x100aa938, NULL},
	 {4, g_unk0x100aa8d8, RunMenuChoice, &g_unk0x100aa988, NULL},
	 {6, g_unk0x100a1af0, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100aab08
RenderTarget g_systemsMenuTarget = {NULL, 0x8ccd, 0x547b, 0x10000, 0x9eb8};

// GLOBAL: MW2 0x100aab20
RenderTarget g_systemsMenuBackgroundTarget = {NULL, 0x8ccd, 0x547b, 0x10000, 0x9eb8};

// GLOBAL: MW2 0x100aab38
MenuDefinition g_systemsMenu = {
	&g_systemsMenuTarget,
	2,
	g_systemsMenuPageStack,
	0,
	-1,
	NULL,
	&g_systemsMenuBackgroundTarget,
	-1,
	NULL,
	225,
	219,
	1,
	NULL,
	14,
	14,
	6,
	{0, 0},
	{0, 0},
	{0x51f, 0},
	{0x51f, 0},
	{0xcccd, 0},
	&g_systemsStatusPage
};

// GLOBAL: MW2 0x100e95f0
MenuPage* g_systemsMenuPageStack[8];

// Returns the setting p_id (0x13 the cockpit overlays, 0x3c and 0x40 two game-key toggles, 0xa6
// FUN_1007d875's state, 0xa7 outlined polygons), or 0.
// FUNCTION: MW2 0x1005e9b0
MechS32 FUN_1005e9b0(MechS32 p_id)
{
	MechS32 value;

	switch (p_id) {
	case 0xa6:
		value = FUN_1007d875(0);
		break;
	case 0xa7:
		if (g_unk0x100a6cc8.m_unk0x34 == 1) {
			value = 1;
		}
		else {
			value = 0;
		}
		break;
	case 0x13:
		value = g_unk0x100a5f18;
		break;
	case 0x40:
		value = g_unk0x100aa298;
		break;
	case 0x3c:
		value = g_unk0x100a1590;
		break;
	default:
		value = 0;
		break;
	}

	return value;
}

// Changes the setting p_id (see FUN_1005e9b0) to p_value.
// FUNCTION: MW2 0x1005eb10
void FUN_1005eb10(MechS32 p_id, MechS32 p_value)
{
	switch (p_id) {
	case 0xa6:
		FUN_1007d88a(0, p_value);
		break;
	case 0xa7:
		if (p_value) {
			g_unk0x100a6cc8.m_unk0x34 = 1;
			PlayCockpitSound(0x1b, 1);
		}
		else {
			g_unk0x100a6cc8.m_unk0x34 = 0;
		}
		break;
	case 0x13:
		g_unk0x100a5f18 = p_value;
		break;
	case 0x40:
		g_unk0x100aa298 = p_value;
		break;
	case 0x3c:
		g_unk0x100a1590 = p_value;
		break;
	default:
		break;
	}
}
