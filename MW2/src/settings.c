#include "settings.h"

#include "decomp.h"
#include "environment.h"
#include "gamekeys.h"
#include "hud.h"
#include "mainmenu.h"
#include "mechdamage.h"
#include "menu.h"
#include "menucontrol.h"
#include "menucontrols.h"
#include "menupage.h"
#include "polydraw.h"
#include "rendersettings.h"
#include "speech.h"
#include "targeting.h"
#include "types.h"

// GLOBAL: MW2 0x100aa870
// GLOBAL: MW2MATROX 0x100a8168
MechChar g_gameCtrlItem[] = "Game Ctrl";

// GLOBAL: MW2 0x100aa880
// GLOBAL: MW2MATROX 0x100a8178
MechChar g_systemsStatusTitle[] = "SYSTEMS STATUS";

// GLOBAL: MW2 0x100aa890
// GLOBAL: MW2MATROX 0x100a8188
MechChar g_lightAmplificationItem[] = "Light Amplification";

// GLOBAL: MW2 0x100aa8a8
// GLOBAL: MW2MATROX 0x100a81a0
MechChar g_imageEnhancementItem[] = "Image Emhancement";

// GLOBAL: MW2 0x100aa8bc
// GLOBAL: MW2MATROX 0x100a81b4
MechChar g_hudItem[] = "HUD";

// GLOBAL: MW2 0x100aa8c0
// GLOBAL: MW2MATROX 0x100a81b8
MechChar g_autoThermOverrideItem[] = "Auto Therm Override";

// GLOBAL: MW2 0x100aa8d8
// GLOBAL: MW2MATROX 0x100a81d0
MechChar g_autoEjectItem[] = "Auto Eject";

// GLOBAL: MW2 0x100aa8e8
// GLOBAL: MW2MATROX 0x100a81e0
MenuControl g_infraredControl = {3, 0, &g_offOnChoices, 166, NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// GLOBAL: MW2 0x100aa910
// GLOBAL: MW2MATROX 0x100a8208
MenuControl g_enhancedVisionControl =
	{3, 0, &g_offOnChoices, 167, NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// GLOBAL: MW2 0x100aa938
// GLOBAL: MW2MATROX 0x100a8230
MenuControl g_hudControl = {3, 0, &g_offOnChoices, 19, NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// GLOBAL: MW2 0x100aa960
// GLOBAL: MW2MATROX 0x100a8258
MenuControl g_overrideShutdownControl =
	{3, 0, &g_offOnChoices, 64, NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// GLOBAL: MW2 0x100aa988
// GLOBAL: MW2MATROX 0x100a8280
MenuControl g_autoEjectControl = {3, 0, &g_offOnChoices, 60, NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// GLOBAL: MW2 0x100aa9b0
// GLOBAL: MW2MATROX 0x100a82a8
MenuPage g_systemsStatusPage = {
	0,
	g_systemsStatusTitle,
	0,
	5,
	0,
	NULL,
	{{4, g_lightAmplificationItem, RunMenuChoice, &g_infraredControl, NULL},
	 {4, g_imageEnhancementItem, RunMenuChoice, &g_enhancedVisionControl, NULL},
	 {4, g_hudItem, RunMenuChoice, &g_hudControl, NULL},
	 {4, g_autoEjectItem, RunMenuChoice, &g_autoEjectControl, NULL},
	 {6, g_escToExitText, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100aab08
// GLOBAL: MW2MATROX 0x100a8400
PANE g_systemsMenuTarget = {NULL, 0x8ccd, 0x547b, 0x10000, 0x9eb8};

// GLOBAL: MW2 0x100aab20
// GLOBAL: MW2MATROX 0x100a8418
PANE g_systemsMenuBackgroundTarget = {NULL, 0x8ccd, 0x547b, 0x10000, 0x9eb8};

// GLOBAL: MW2 0x100aab38
// GLOBAL: MW2MATROX 0x100a8430
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

// The systems status menu's settings, by the game key that toggles each (GAMEKEY.MAP): 0x13
// TOGGLE_HUD, 0x3c TOGGLE_AUTOEJECT, 0x40 OVERRIDE_SHUTDOWN, 0xa6 INFRARED (light amplification)
// and 0xa7 ENHANCED_VISION (image enhancement: outlined polygons).

// Returns the setting p_id, or 0.
// FUNCTION: MW2 0x1005e9b0
// FUNCTION: MW2MATROX 0x10044400
MechS32 GetSystemSetting(MechS32 p_id)
{
	MechS32 value;

	switch (p_id) {
	case 0xa6:
		value = IsInfraredOn(0);
		break;
	case 0xa7:
		if (g_renderSettings.m_wireframe == 1) {
			value = 1;
		}
		else {
			value = 0;
		}
		break;
	case 0x13:
		value = g_showHud;
		break;
	case 0x40:
		value = g_overrideShutdown;
		break;
	case 0x3c:
		value = g_autoEject;
		break;
	default:
		value = 0;
		break;
	}

	return value;
}

// Changes the setting p_id (see GetSystemSetting) to p_value.
// FUNCTION: MW2 0x1005eb10
// FUNCTION: MW2MATROX 0x10044560
void SetSystemSetting(MechS32 p_id, MechS32 p_value)
{
	switch (p_id) {
	case 0xa6:
		SetInfrared(0, p_value);
		break;
	case 0xa7:
		if (p_value) {
			g_renderSettings.m_wireframe = 1;
			PlayCockpitSound(0x1b, 1);
		}
		else {
			g_renderSettings.m_wireframe = 0;
		}
		break;
	case 0x13:
		g_showHud = p_value;
		break;
	case 0x40:
		g_overrideShutdown = p_value;
		break;
	case 0x3c:
		g_autoEject = p_value;
		break;
	default:
		break;
	}
}
