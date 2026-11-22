/* The main in-mission menu (menu 4): abort the mission, the monitor brightness (in the Matrox
   edition, the graphics page of accel/graphicsmenu.c), the audio and combat settings, flee to
   Windows. A data-only object: its data follows network.c's. */
#include "mainmenu.h"

#include "audiomenu.h"
#include "brightnessmenu.h"
#include "dorcs.h"
#ifdef MW2_GRAPHICS_MENU
#include "accel/graphicsmenu.h"
#endif
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
// GLOBAL: MW2GBL 0x100b30b0
// GLOBAL: MW2MATROX 0x100a2448
// GLOBAL: MW23DFX 0x10264488
// GLOBAL: MW2ATI 0x10198780
// GLOBAL: MW2S3 0x10075e20
// GLOBAL: MW2POWERVR 0x101c3af8
MechChar g_mainMenuTitle[] = "MAIN MENU";

// GLOBAL: MW2 0x100a1aa0
// GLOBAL: MW2GBL 0x100b30c0
// GLOBAL: MW2MATROX 0x100a2458
// GLOBAL: MW23DFX 0x10264498
// GLOBAL: MW2ATI 0x10198790
// GLOBAL: MW2S3 0x10075e30
// GLOBAL: MW2POWERVR 0x101c3b08
MechChar g_abortMissionItem[] = "Abort Mission";

#ifndef MW2_GRAPHICS_MENU
// GLOBAL: MW2 0x100a1ab0
// GLOBAL: MW2GBL 0x100b30d0
MechChar g_monitorBrightnessItem[] = "Monitor Brightness";
#endif

// GLOBAL: MW2 0x100a1ac8
// GLOBAL: MW2GBL 0x100b30e8
// GLOBAL: MW2MATROX 0x100a2468
// GLOBAL: MW23DFX 0x102644a8
// GLOBAL: MW2ATI 0x101987a0
// GLOBAL: MW2S3 0x10075e40
// GLOBAL: MW2POWERVR 0x101c3b18
MechChar g_fleeToWindowsItem[] = "Flee to Windows";

// GLOBAL: MW2 0x100a1ad8
// GLOBAL: MW2GBL 0x100b30f8
// GLOBAL: MW2MATROX 0x100a2478
// GLOBAL: MW23DFX 0x102644b8
// GLOBAL: MW2ATI 0x101987b0
// GLOBAL: MW2S3 0x10075e50
// GLOBAL: MW2POWERVR 0x101c3b28
MechChar g_acceptText[] = "Accept (Esc to cancel)";

// GLOBAL: MW2 0x100a1af0
// GLOBAL: MW2GBL 0x100b3110
// GLOBAL: MW2MATROX 0x100a2490
// GLOBAL: MW23DFX 0x102644d0
// GLOBAL: MW2ATI 0x101987c8
// GLOBAL: MW2S3 0x10075e68
// GLOBAL: MW2POWERVR 0x101c3b40
MechChar g_escToExitText[] = "(Esc to exit)";

// GLOBAL: MW2 0x100a1b00
// GLOBAL: MW2GBL 0x100b3120
// GLOBAL: MW2MATROX 0x100a24a0
// GLOBAL: MW23DFX 0x102644e0
// GLOBAL: MW2ATI 0x101987d8
// GLOBAL: MW2S3 0x10075e78
// GLOBAL: MW2POWERVR 0x101c3b50
MechChar g_fleeToWindowsTitle[] = "FLEE TO WINDOWS";

// GLOBAL: MW2 0x100a1b10
// GLOBAL: MW2GBL 0x100b3130
// GLOBAL: MW2MATROX 0x100a24b0
// GLOBAL: MW23DFX 0x102644f0
// GLOBAL: MW2ATI 0x101987e8
// GLOBAL: MW2S3 0x10075e88
// GLOBAL: MW2POWERVR 0x101c3b60
MechChar g_abortMissionTitle[] = "ABORT MISSION";

#ifndef MW2_GRAPHICS_MENU
// GLOBAL: MW2 0x100a1b20
// GLOBAL: MW2GBL 0x100b3140
MechChar g_monitorBrightnessTitle[] = "MONITOR BRIGHTNESS";
#endif

// GLOBAL: MW2 0x100a1b38
// GLOBAL: MW2MATROX 0x100a24c0
MechChar g_areYouSureText[] = "Are you sure?";

// GLOBAL: MW2 0x100a1b48
// GLOBAL: MW2GBL 0x100b3168
// GLOBAL: MW2MATROX 0x100a24d0
// GLOBAL: MW23DFX 0x10264510
// GLOBAL: MW2ATI 0x10198808
// GLOBAL: MW2S3 0x10075ea8
// GLOBAL: MW2POWERVR 0x101c3b80
MechChar g_confirmCowardiceText[] = "Confirm your cowardice";

// GLOBAL: MW2 0x100a1b60
// GLOBAL: MW2GBL 0x100b3180
// GLOBAL: MW2MATROX 0x100a24e8
// GLOBAL: MW23DFX 0x10264528
// GLOBAL: MW2ATI 0x10198820
// GLOBAL: MW2S3 0x10075ec0
// GLOBAL: MW2POWERVR 0x101c3b98
MechChar g_confirmationRequestedText[] = "Confirmation requested";

// GLOBAL: MW2 0x100a1b78
// GLOBAL: MW2MATROX 0x100a2500
// GLOBAL: MW23DFX 0x10264540
// GLOBAL: MW2POWERVR 0x101c3bb0
MechChar g_noText[] = "No";

// GLOBAL: MW2 0x100a1b7c
// GLOBAL: MW2MATROX 0x100a2504
// GLOBAL: MW23DFX 0x10264544
// GLOBAL: MW2POWERVR 0x101c3bb4
MechChar g_yesText[] = "Yes";

// GLOBAL: MW2 0x100a1b80
// GLOBAL: MW2GBL 0x100b31a0
// GLOBAL: MW2MATROX 0x100a2508
// GLOBAL: MW23DFX 0x10264548
// GLOBAL: MW2ATI 0x10198840
// GLOBAL: MW2S3 0x10075ee0
// GLOBAL: MW2POWERVR 0x101c3bb8
MechChar g_offText[] = "Off";

// GLOBAL: MW2 0x100a1b84
// GLOBAL: MW2GBL 0x100b31a4
// GLOBAL: MW2MATROX 0x100a250c
// GLOBAL: MW23DFX 0x1026454c
// GLOBAL: MW2ATI 0x10198844
// GLOBAL: MW2S3 0x10075ee4
// GLOBAL: MW2POWERVR 0x101c3bbc
MechChar g_onText[] = "On";

// GLOBAL: MW2 0x100a1b88
// GLOBAL: MW2GBL 0x100b31a8
// GLOBAL: MW2MATROX 0x100a2510
// GLOBAL: MW23DFX 0x10264550
// GLOBAL: MW2ATI 0x10198848
// GLOBAL: MW2S3 0x10075ee8
// GLOBAL: MW2POWERVR 0x101c3bc0
MechChar g_lowText[] = "Low";

// GLOBAL: MW2 0x100a1b90
// GLOBAL: MW2MATROX 0x100a2518
MechChar g_mediumText[] = "Medium";

// GLOBAL: MW2 0x100a1b98
// GLOBAL: MW2GBL 0x100b31b8
// GLOBAL: MW2MATROX 0x100a2520
// GLOBAL: MW23DFX 0x10264560
// GLOBAL: MW2ATI 0x10198858
// GLOBAL: MW2S3 0x10075ef8
// GLOBAL: MW2POWERVR 0x101c3bd0
MechChar g_highText[] = "High";

// GLOBAL: MW2 0x100a1ba0
// GLOBAL: MW2GBL 0x100b31c0
// GLOBAL: MW2MATROX 0x100a2528
// GLOBAL: MW23DFX 0x10264568
// GLOBAL: MW2ATI 0x10198860
// GLOBAL: MW2S3 0x10075f00
// GLOBAL: MW2POWERVR 0x101c3bd8
MenuChoices g_offOnChoices = {NULL, 2, {g_offText, g_onText}};

// GLOBAL: MW2 0x100a1be8
// GLOBAL: MW2MATROX 0x100a2570
// GLOBAL: MW23DFX 0x102645b0
// GLOBAL: MW2POWERVR 0x101c3c20
MenuChoices g_noYesChoices = {NULL, 2, {g_noText, g_yesText}};

// GLOBAL: MW2 0x100a1c30
// GLOBAL: MW2GBL 0x100b3250
// GLOBAL: MW2MATROX 0x100a25b8
// GLOBAL: MW23DFX 0x102645f8
// GLOBAL: MW2ATI 0x101988f0
// GLOBAL: MW2S3 0x10075f90
// GLOBAL: MW2POWERVR 0x101c3c68
MenuChoices g_lowHighChoices = {NULL, 2, {g_lowText, g_highText}};

// GLOBAL: MW2 0x100a1c78
// GLOBAL: MW2MATROX 0x100a2600
MenuChoices g_offLowMediumHighChoices = {NULL, 4, {g_offText, g_lowText, g_mediumText, g_highText}};

// GLOBAL: MW2 0x100a1cc0
// GLOBAL: MW2GBL 0x100b32e0
// GLOBAL: MW2MATROX 0x100a2648
// GLOBAL: MW23DFX 0x10264688
// GLOBAL: MW2ATI 0x10198980
// GLOBAL: MW2S3 0x10076020
// GLOBAL: MW2POWERVR 0x101c3cf8
MechS32 g_sliderShapes[8] = {
#ifdef MW2_SLIDER_SHAPES
	70,
	0,
	67,
	0,
	73,
	0,
	85,
	0
#else
	67,
	0,
	64,
	0,
	70,
	0,
	82,
	0
#endif
};

// GLOBAL: MW2 0x100a1ce0
// GLOBAL: MW2GBL 0x100b3300
// GLOBAL: MW2MATROX 0x100a2668
// GLOBAL: MW23DFX 0x102646a8
// GLOBAL: MW2ATI 0x101989a0
// GLOBAL: MW2S3 0x10076040
// GLOBAL: MW2POWERVR 0x101c3d18
MenuControl g_confirmControl = {2, 0, NULL, 0, NULL, NULL, NULL, NULL, NULL};

#ifndef MW2_GRAPHICS_MENU
// GLOBAL: MW2 0x100a1d08
// GLOBAL: MW2GBL 0x100b3328
MenuControl g_brightnessControl = {
	2,
	0,
	g_sliderShapes,
	0,
	NULL,
	GetBrightnessFraction,
	PreviewBrightnessFraction,
	SetBrightnessFraction,
	RestoreBrightness
};
#endif

// GLOBAL: MW2 0x100a1d30
// GLOBAL: MW2GBL 0x100b3350
// GLOBAL: MW2MATROX 0x100a2690
// GLOBAL: MW23DFX 0x102646d0
// GLOBAL: MW2ATI 0x101989c8
// GLOBAL: MW2S3 0x10076068
// GLOBAL: MW2POWERVR 0x101c3d40
MenuPage g_abortMissionPage = {
	0,
	g_abortMissionTitle,
	0,
	2,
	0,
	NULL,
	{{3, g_confirmationRequestedText, NULL, NULL, NULL}, {1, g_acceptText, AbortMissionAction, &g_confirmControl, NULL}}
};

#ifndef MW2_GRAPHICS_MENU
// GLOBAL: MW2 0x100a1e88
// GLOBAL: MW2GBL 0x100b34a8
MenuPage g_brightnessPage = {
	0,
	g_monitorBrightnessTitle,
	0,
	2,
	0,
	NULL,
	{{1, g_monitorBrightnessItem, RunMenuSlider, &g_brightnessControl, NULL}, {2, g_acceptText, NULL, NULL, NULL}}
};
#endif

// GLOBAL: MW2 0x100a1fe0
// GLOBAL: MW2GBL 0x100b3600
// GLOBAL: MW2MATROX 0x100a27e8
// GLOBAL: MW23DFX 0x10264828
// GLOBAL: MW2ATI 0x10198b20
// GLOBAL: MW2S3 0x100761c0
// GLOBAL: MW2POWERVR 0x101c3e98
MenuPage g_fleePage = {
	0,
	g_fleeToWindowsTitle,
	0,
	2,
	0,
	NULL,
	{{3, g_confirmCowardiceText, NULL, NULL, NULL}, {1, g_acceptText, FleeToWindowsAction, &g_confirmControl, NULL}}
};

// GLOBAL: MW2 0x100a2138
// GLOBAL: MW2GBL 0x100b3758
// GLOBAL: MW2MATROX 0x100a2940
// GLOBAL: MW23DFX 0x10264980
// GLOBAL: MW2ATI 0x10198c78
// GLOBAL: MW2S3 0x10076318
// GLOBAL: MW2POWERVR 0x101c3ff0
MenuPage g_mainMenuPage = {
	0,
	g_mainMenuTitle,
	0,
	6,
	0,
	NULL,
	{{0, g_abortMissionItem, NULL, NULL, &g_abortMissionPage},
#ifdef MW2_GRAPHICS_MENU
	 {0, g_graphicsItem, NULL, NULL, &g_graphicsPage},
#else
	 {0, g_monitorBrightnessItem, NULL, NULL, &g_brightnessPage},
#endif
	 {0, g_audioItem, NULL, NULL, &g_audioPage},
	 {0, g_combatVariablesItem, NULL, NULL, &g_combatVariablesPage},
	 {0, g_fleeToWindowsItem, NULL, NULL, &g_fleePage},
	 {2, g_acceptText, NULL, NULL, NULL}}
};

// GLOBAL: MW2 0x100a2290
// GLOBAL: MW2GBL 0x100b38b0
// GLOBAL: MW2MATROX 0x100a2a98
// GLOBAL: MW23DFX 0x10264ad8
// GLOBAL: MW2ATI 0x10198dd0
// GLOBAL: MW2S3 0x10076470
// GLOBAL: MW2POWERVR 0x101c4148
PANE g_mainMenuTarget = {NULL, 0x4000, 0x3333, 0x10000, 0xcccd};

// GLOBAL: MW2 0x100a22a8
// GLOBAL: MW2GBL 0x100b38c8
// GLOBAL: MW2MATROX 0x100a2ab0
// GLOBAL: MW23DFX 0x10264af0
// GLOBAL: MW2ATI 0x10198de8
// GLOBAL: MW2S3 0x10076488
// GLOBAL: MW2POWERVR 0x101c4160
PANE g_mainMenuBackgroundTarget = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100a22c0
// GLOBAL: MW2GBL 0x100b38e0
// GLOBAL: MW2MATROX 0x100a2ac8
// GLOBAL: MW23DFX 0x10264b08
// GLOBAL: MW2ATI 0x10198e00
// GLOBAL: MW2S3 0x100764a0
// GLOBAL: MW2POWERVR 0x101c4178
MenuDefinition g_mainMenu = {
	&g_mainMenuTarget,
	17,
	g_mainMenuPageStack,
	0,
#ifdef MW2_MENU_BACKGROUND_0X118
	0x118,
#else
	0x112,
#endif
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
// GLOBAL: MW2GBL 0x100bf1a0
// GLOBAL: MW2MATROX 0x10218410
// GLOBAL: MW23DFX 0x101534f0
// GLOBAL: MW2ATI 0x10087510
// GLOBAL: MW2S3 0x100846d0
// GLOBAL: MW2POWERVR 0x1007daf0
MenuPage* g_mainMenuPageStack[8];
