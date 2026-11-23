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
// GLOBAL: MW2GBL 0x1009e368
// GLOBAL: MW2MATROX 0x100a8168
MechChar g_gameCtrlItem[] = "Game Ctrl";

// GLOBAL: MW2 0x100aa880
// GLOBAL: MW2GBL 0x1009e378
// GLOBAL: MW2MATROX 0x100a8178
// GLOBAL: MW23DFX 0x10264b88
// GLOBAL: MW2ATI 0x10198e80
// GLOBAL: MW2S3 0x10062c20
// GLOBAL: MW2POWERVR 0x101c41f8
MechChar g_systemsStatusTitle[] = "SYSTEMS STATUS";

// GLOBAL: MW2 0x100aa890
// GLOBAL: MW2GBL 0x1009e388
// GLOBAL: MW2MATROX 0x100a8188
// GLOBAL: MW23DFX 0x10264b98
// GLOBAL: MW2ATI 0x10198e90
// GLOBAL: MW2S3 0x10062c30
// GLOBAL: MW2POWERVR 0x101c4208
MechChar g_lightAmplificationItem[] = "Light Amplification";

// GLOBAL: MW2 0x100aa8a8
// GLOBAL: MW2GBL 0x1009e3a0
// GLOBAL: MW2MATROX 0x100a81a0
// GLOBAL: MW23DFX 0x10264bb0
// GLOBAL: MW2ATI 0x10198ea8
// GLOBAL: MW2S3 0x10062c48
// GLOBAL: MW2POWERVR 0x101c4220
MechChar g_imageEnhancementItem[] = "Image Emhancement";

// GLOBAL: MW2 0x100aa8bc
// GLOBAL: MW2GBL 0x1009e3b4
// GLOBAL: MW2MATROX 0x100a81b4
// GLOBAL: MW23DFX 0x10264bc4
// GLOBAL: MW2ATI 0x10198ebc
// GLOBAL: MW2S3 0x10062c5c
// GLOBAL: MW2POWERVR 0x101c4234
MechChar g_hudItem[] = "HUD";

// GLOBAL: MW2 0x100aa8c0
// GLOBAL: MW2GBL 0x1009e3b8
// GLOBAL: MW2MATROX 0x100a81b8
// GLOBAL: MW2POWERVR 0x101c4238
MechChar g_autoThermOverrideItem[] = "Auto Therm Override";

// GLOBAL: MW2 0x100aa8d8
// GLOBAL: MW2GBL 0x1009e3d0
// GLOBAL: MW2MATROX 0x100a81d0
// GLOBAL: MW23DFX 0x10264be0
// GLOBAL: MW2ATI 0x10198ed8
// GLOBAL: MW2S3 0x10062c78
// GLOBAL: MW2POWERVR 0x101c4250
MechChar g_autoEjectItem[] = "Auto Eject";

// GLOBAL: MW2 0x100aa8e8
// GLOBAL: MW2GBL 0x1009e3e0
// GLOBAL: MW2MATROX 0x100a81e0
// GLOBAL: MW23DFX 0x10264bf0
// GLOBAL: MW2ATI 0x10198ee8
// GLOBAL: MW2S3 0x10062c88
// GLOBAL: MW2POWERVR 0x101c4260
MenuControl g_infraredControl =
	{3, 0, &g_offOnChoices, GAME_KEY_ACTION(166), NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// GLOBAL: MW2 0x100aa910
// GLOBAL: MW2GBL 0x1009e408
// GLOBAL: MW2MATROX 0x100a8208
// GLOBAL: MW23DFX 0x10264c18
// GLOBAL: MW2ATI 0x10198f10
// GLOBAL: MW2S3 0x10062cb0
// GLOBAL: MW2POWERVR 0x101c4288
MenuControl g_enhancedVisionControl =
	{3, 0, &g_offOnChoices, GAME_KEY_ACTION(167), NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// GLOBAL: MW2 0x100aa938
// GLOBAL: MW2GBL 0x1009e430
// GLOBAL: MW2MATROX 0x100a8230
// GLOBAL: MW23DFX 0x10264c40
// GLOBAL: MW2ATI 0x10198f38
// GLOBAL: MW2S3 0x10062cd8
// GLOBAL: MW2POWERVR 0x101c42b0
MenuControl g_hudControl = {3, 0, &g_offOnChoices, 19, NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// GLOBAL: MW2 0x100aa960
// GLOBAL: MW2GBL 0x1009e458
// GLOBAL: MW2MATROX 0x100a8258
// GLOBAL: MW2POWERVR 0x101c42d8
MenuControl g_overrideShutdownControl =
	{3, 0, &g_offOnChoices, 64, NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// GLOBAL: MW2 0x100aa988
// GLOBAL: MW2GBL 0x1009e480
// GLOBAL: MW2MATROX 0x100a8280
// GLOBAL: MW23DFX 0x10264c90
// GLOBAL: MW2ATI 0x10198f88
// GLOBAL: MW2S3 0x10062d28
// GLOBAL: MW2POWERVR 0x101c4300
MenuControl g_autoEjectControl = {3, 0, &g_offOnChoices, 60, NULL, GetSystemSetting, NULL, SetSystemSetting, NULL};

// The PowerVR edition's page doesn't list image enhancement, though it keeps the item's text.
#ifdef MW2_POWERVR
// GLOBAL: MW2POWERVR 0x101c4328
MenuPage g_systemsStatusPage = {
	0,
	g_systemsStatusTitle,
	0,
	4,
	0,
	NULL,
	{{4, g_lightAmplificationItem, RunMenuChoice, &g_infraredControl, NULL},
	 {4, g_hudItem, RunMenuChoice, &g_hudControl, NULL},
	 {4, g_autoEjectItem, RunMenuChoice, &g_autoEjectControl, NULL},
	 {6, g_escToExitText, NULL, NULL, NULL}}
};
#else
// GLOBAL: MW2 0x100aa9b0
// GLOBAL: MW2GBL 0x1009e4a8
// GLOBAL: MW2MATROX 0x100a82a8
// GLOBAL: MW23DFX 0x10264cb8
// GLOBAL: MW2ATI 0x10198fb0
// GLOBAL: MW2S3 0x10062d50
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
#endif

// GLOBAL: MW2 0x100aab08
// GLOBAL: MW2GBL 0x1009e600
// GLOBAL: MW2MATROX 0x100a8400
// GLOBAL: MW23DFX 0x10264e10
// GLOBAL: MW2ATI 0x10199108
// GLOBAL: MW2S3 0x10062ea8
// GLOBAL: MW2POWERVR 0x101c4480
PANE g_systemsMenuTarget = {NULL, 0x8ccd, 0x547b, 0x10000, 0x9eb8};

// GLOBAL: MW2 0x100aab20
// GLOBAL: MW2GBL 0x1009e618
// GLOBAL: MW2MATROX 0x100a8418
// GLOBAL: MW23DFX 0x10264e28
// GLOBAL: MW2ATI 0x10199120
// GLOBAL: MW2S3 0x10062ec0
// GLOBAL: MW2POWERVR 0x101c4498
PANE g_systemsMenuBackgroundTarget = {NULL, 0x8ccd, 0x547b, 0x10000, 0x9eb8};

// GLOBAL: MW2 0x100aab38
// GLOBAL: MW2GBL 0x1009e630
// GLOBAL: MW2MATROX 0x100a8430
// GLOBAL: MW23DFX 0x10264e40
// GLOBAL: MW2ATI 0x10199138
// GLOBAL: MW2S3 0x10062ed8
// GLOBAL: MW2POWERVR 0x101c44b0
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
// GLOBAL: MW2GBL 0x1015d8f0
// GLOBAL: MW23DFX 0x101534d0
// GLOBAL: MW2ATI 0x100874f0
// GLOBAL: MW2S3 0x1014d9b0
// GLOBAL: MW2POWERVR 0x1007dad0
MenuPage* g_systemsMenuPageStack[8];

// The systems status menu's settings, by the game key that toggles each (GAMEKEY.MAP): 0x13
// TOGGLE_HUD, 0x3c TOGGLE_AUTOEJECT, 0x40 OVERRIDE_SHUTDOWN, 0xa6 INFRARED (light amplification)
// and 0xa7 ENHANCED_VISION (image enhancement: outlined polygons); GBL's INFRARED and ENHANCED_VISION are
// one higher (GAME_KEY_ACTION).

// Returns the setting p_id, or 0.
// FUNCTION: MW2 0x1005e9b0
// FUNCTION: MW2GBL 0x10016a60
// FUNCTION: MW2MATROX 0x10044400
// FUNCTION: MW23DFX 0x10065030
// FUNCTION: MW2ATI 0x100626a0
// FUNCTION: MW2S3 0x10013bd0
// FUNCTION: MW2POWERVR 0x1005d410
MechS32 GetSystemSetting(MechS32 p_id)
{
	MechS32 value;

	switch (p_id) {
	case GAME_KEY_ACTION(0xa6):
		value = IsInfraredOn(0);
		break;
	case GAME_KEY_ACTION(0xa7):
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
// FUNCTION: MW2GBL 0x10016bc1
// FUNCTION: MW2MATROX 0x10044560
// FUNCTION: MW23DFX 0x10065130
// FUNCTION: MW2ATI 0x100627a0
// FUNCTION: MW2S3 0x10013cd0
// FUNCTION: MW2POWERVR 0x1005d510
void SetSystemSetting(MechS32 p_id, MechS32 p_value)
{
	switch (p_id) {
	case GAME_KEY_ACTION(0xa6):
		SetInfrared(0, p_value);
		break;
	case GAME_KEY_ACTION(0xa7):
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
