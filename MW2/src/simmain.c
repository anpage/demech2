#include "simmain.h"

#include "animation.h"
#include "audio.h"
#include "brightness.h"
#include "callbacks.h"
#include "clock.h"
#include "cmdline.h"
#include "cockpit.h"
#include "commandmenu.h"
#include "commandpointmenu.h"
#include "compat.h"
#include "config.h"
#include "debugprint.h"
#include "decomp.h"
#include "directdraw.h"
#include "dispdibmode.h"
#include "displaybackend.h"
#include "effectinfo.h"
#include "environment.h"
#include "error.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "gamekeys.h"
#include "gdi.h"
#include "gpanim.h"
#include "inputmap.h"
#include "keyboard.h"
#include "loadres.h"
#include "mainmenu.h"
#include "menu.h"
#include "mss.h"
#include "mw2log.h"
#include "netlaunchinfo.h"
#include "network.h"
#include "overlay.h"
#include "palette.h"
#include "palettecolor.h"
#include "pausebanner.h"
#include "perf.h"
#include "players.h"
#include "point.h"
#include "random.h"
#include "refreshmode.h"
#include "render.h"
#include "rendertarget.h"
#include "resource.h"
#include "screenscale.h"
#include "shots.h"
#include "speech.h"
#include "startup.h"
#include "staticmem.h"
#include "supanim.h"
#include "timedoverlays.h"
#include "types.h"
#include "unk1005e9b0.h"
#include "unk10073af0.h"
#include "videodriverchoice.h"
#include "weapons.h"

#include <excpt.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(SoundConfig, 0x3c)
DECOMP_SIZE_ASSERT(StarMission, 0x3c0a)
DECOMP_SIZE_ASSERT(MissionObjective, 0x13f)

// The globals SimMain and SimWindowProc use are defined here until the objects that own
// them are decompiled.

// The weapon types (weapons.c) and the effect types: the tables of an object that isn't
// decompiled yet.
// GLOBAL: MW2 0x100a0258
WeaponDef g_weaponDefs[31] = {
	{4, 3, 26, 27, 0, 1, 1, 20, 6, 189, 0, 100, 2, 39321, 0, 7500, 100100, 2534, 11, 905, "LRM20"},
	{4, 3, 26, 27, 0, 1, 1, 15, 8, 189, 0, 100, 2, 39321, 0, 7500, 100100, 2172, 22, 905, "LRM15"},
	{4, 3, 26, 27, 0, 1, 1, 10, 12, 189, 0, 100, 2, 52428, 0, 7500, 100100, 1810, 45, 905, "LRM10"},
	{4, 3, 26, 27, 0, 1, 1, 5, 24, 189, 0, 100, 2, 52428, 0, 7500, 100100, 1448, 45, 905, "LRM5"},
	{3, 3, 26, 27, 0, 1, 0, 6, 15, 258, 0, 70, 3, 87818, 0, 0, 49686, 1448, 60, 543, "SRM6"},
	{3, 3, 26, 27, 0, 1, 0, 4, 25, 258, 0, 70, 3, 98304, 0, 0, 49686, 1267, 45, 543, "SRM4"},
	{3, 3, 26, 27, 0, 1, 0, 2, 50, 258, 0, 100, 3, 131072, 0, 0, 49686, 1086, 45, 543, "SRM2"},
	{4, 3, 26, 27, 0, 1, 1, 6, 15, 189, 1, 70, 3, 87425, 0, 25, 49686, 1086, 45, 543, "SSRM6"},
	{4, 3, 26, 27, 0, 1, 1, 4, 25, 189, 1, 70, 3, 98304, 0, 25, 49686, 1448, 45, 543, "SSRM4"},
	{4, 3, 26, 27, 0, 1, 1, 2, 50, 189, 1, 70, 3, 131072, 0, 25, 49686, 1448, 45, 543, "SSRM2"},
	{5, 5, 23, 24, 1, 1, 0, 1, 200, 226, 1, 300, 1, 0, 0, 0, 17500, 11, 11, 57, "MGun"},
	{7, 11, 23, 24, 0, 1, 0, 1, 8, 218, 0, 280, 18, 32768, 0, 0, 182000, 724, 90, 646, "GAUSS"},
	{5, 11, 23, 24, 1, 1, 0, 1, 45, 1, 0, 240, 3, 32768, 0, 0, 80000, 45, 45, 331, "xAC2"},
	{5, 11, 23, 24, 1, 1, 0, 1, 20, 1, 0, 240, 7, 32768, 0, 0, 70000, 45, 45, 289, "xAC5"},
	{5, 11, 23, 24, 1, 1, 0, 1, 10, 1, 0, 240, 15, 65536, 0, 0, 60000, 45, 11, 247, "xAC10"},
	{5, 11, 23, 24, 1, 1, 0, 1, 5, 1, 0, 240, 30, 196608, 0, 0, 45000, 45, 11, 186, "xAC20"},
	{5, 11, 23, 24, 1, 1, 0, 1, 45, 187, 0, 220, 3, 32768, 0, 0, 70000, 22, 11, 316, "uAC2"},
	{5, 11, 23, 24, 1, 1, 0, 1, 20, 187, 0, 220, 7, 32768, 0, 0, 60000, 22, 11, 271, "uAC5"},
	{5, 11, 23, 24, 1, 1, 0, 1, 10, 187, 0, 220, 15, 98304, 0, 0, 50000, 22, 11, 226, "uAC10"},
	{5, 11, 23, 24, 1, 1, 0, 1, 5, 187, 0, 220, 30, 229376, 0, 0, 40000, 22, 11, 181, "uAC20"},
	{7, -1, -1, -1, 0, 0, 0, 1, 8, 227, 0, 80, 20, 12, 0, 1000, 80000, 90, 181, 181, "FLAMER"},
	{6, 6, 30, 31, 0, 1, 0, 1, -1, 227, 0, 70, 17, 819200, 229376, 0, 120000, 452, 22, 1705, "PPC"},
	{0, 0, 28, 29, 1, 1, 0, 1, -1, 199, 0, 280, 8, 393216, 0, 0, 101920, 108, 60, 362, "LLASER"},
	{1, 1, 28, 29, 1, 1, 0, 1, -1, 228, 0, 280, 5, 163840, 0, 0, 50960, 90, 60, 181, "MLASER"},
	{2, 2, 28, 29, 1, 1, 0, 1, -1, 243, 0, 280, 3, 65536, 0, 0, 25480, 72, 60, 90, "SLASER"},
	{0, 0, 28, 29, 1, 1, 0, 2, -1, 277, 0, 280, 4, 229376, 0, 0, 81536, 45, 60, 289, "LPLAS"},
	{1, 1, 28, 29, 1, 1, 0, 2, -1, 234, 0, 280, 2, 98304, 0, 0, 40768, 36, 11, 144, "MPLAS"},
	{2, 2, 28, 29, 1, 1, 0, 2, -1, 278, 0, 280, 1, 32768, 0, 0, 20384, 18, 11, 72, "SPLAS"},
	{3, 8, 23, 24, 0, 1, 1, 1, -1, 189, 1, 80, 0, 4, 0, 0, 200000, 2534, 45, 1448, "NARC"},
	{5, 5, 23, 24, 1, 1, 0, 1, 24, 1, 1, 600, 1, 0, 0, 0, 10000, 11, 5, 271, "AMS"},
	{22, 22, -1, -1, 1, 1, 0, 1, 1, 189, 0, 0, 100, 0, 0, 0, 200000, 36200, 181, 5430, "NUKE"},
};

// GLOBAL: MW2 0x100a0d00
EffectInfo g_effectInfo[0x20] = {
	{271, -1, 251, -1, 0, 1, 0},  // 0x00
	{271, -1, 251, -1, 0, 1, 0},  // 0x01
	{271, -1, 251, -1, 0, 1, 0},  // 0x02
	{253, 1, 210, -1, 1, 1, 1},   // 0x03
	{253, 1, 210, -1, 1, 1, 1},   // 0x04
	{90, -1, 250, -1, 0, 1, 0},   // 0x05
	{579, 2, 235, -1, 1, 1, 1},   // 0x06
	{398, 1, 188, -1, 0, 1, 1},   // 0x07
	{90, -1, 248, -1, 0, 1, 0},   // 0x08
	{162, -1, 239, 10, 0, 1, 0},  // 0x09
	{0, -1, -1, -1, 0, 0, 0},     // 0x0a
	{1810, -1, -1, -1, 0, 1, 0},  // 0x0b
	{398, 1, 210, -1, 1, 1, 1},   // 0x0c
	{579, 1, 246, -1, 1, 1, 1},   // 0x0d
	{72, -1, 249, -1, 0, 1, 0},   // 0x0e
	{72, -1, 249, -1, 0, 1, 0},   // 0x0f
	{72, -1, 249, -1, 0, 1, 0},   // 0x10
	{108, 1, 255, -1, 1, 1, 1},   // 0x11
	{144, -1, 252, -1, 0, 1, 0},  // 0x12
	{362, 1, 210, -1, 1, 1, 1},   // 0x13
	{362, 1, 210, -1, 1, 1, 1},   // 0x14
	{579, 2, 235, -1, 1, 1, 1},   // 0x15
	{5430, -1, 209, -1, 1, 1, 1}, // 0x16
	{36, -1, -1, -1, 0, 1, 0},    // 0x17
	{36, -1, -1, -1, 0, 1, 0},    // 0x18
	{72, -1, -1, -1, 0, 1, 0},    // 0x19
	{36, -1, -1, -1, 0, 1, 0},    // 0x1a
	{36, -1, -1, -1, 0, 1, 0},    // 0x1b
	{36, -1, -1, -1, 0, 1, 0},    // 0x1c
	{36, -1, -1, -1, 0, 1, 0},    // 0x1d
	{36, -1, -1, -1, 0, 1, 0},    // 0x1e
	{36, -1, -1, -1, 0, 1, 0},    // 0x1f
};

// The distance FUN_10011f9a moves the free camera back from the mech's eye.
// GLOBAL: MW2 0x100a23ec
MechS32 g_unk0x100a23ec = 0;

// The external view's distance limits, height and turn (FUN_100118bc).

// GLOBAL: MW2 0x100a23f0
MechS32 g_unk0x100a23f0 = 0;

// GLOBAL: MW2 0x100a23f4
MechS32 g_unk0x100a23f4 = 0;

// GLOBAL: MW2 0x100a23f8
MechS32 g_unk0x100a23f8 = 0;

// GLOBAL: MW2 0x100a23fc
MechS32 g_unk0x100a23fc = 0xb40000;

// GLOBAL: MW2 0x100a2400
MechS32 g_normalFov = 0x10000;

// GLOBAL: MW2 0x100a2404
MechS32 g_zoomFov = 0x10000;

// The camera's view mode, or -1.
// GLOBAL: MW2 0x100a2408
MechS32 g_unk0x100a2408 = -1;

// The view mode to return to from the external view.
// GLOBAL: MW2 0x100a240c
MechS32 g_unk0x100a240c = -1;

// The zoom FirstEyepoint starts the camera at.
// GLOBAL: MW2 0x100a2410
MechS32 g_unk0x100a2410 = 0;

// GLOBAL: MW2 0x100a2414
MechS32 g_unk0x100a2414 = 0;

// GLOBAL: MW2 0x100a2420
undefined4 g_unk0x100a2420 = 0;

// GLOBAL: MW2 0x100a2424
MechS32 g_unk0x100a2424 = -1;

// GLOBAL: MW2 0x100a2428
MechS32 g_unk0x100a2428 = 0;

// Set by FUN_10011e45.
// GLOBAL: MW2 0x100a2418
MechS32 g_unk0x100a2418 = 1;

// GLOBAL: MW2 0x100a241c
MechS32 g_unk0x100a241c = 0;

// GLOBAL: MW2 0x100a242c
struct Player* g_localPlayer = NULL;

// The player the camera tracks.
// GLOBAL: MW2 0x100a2430
MechS32 g_unk0x100a2430 = 0;

// GLOBAL: MW2 0x100a2434
MechS32* g_unk0x100a2434 = NULL;

// GLOBAL: MW2 0x100a2438
MechS32* g_unk0x100a2438 = NULL;

// The drop camera (FUN_10011edc): its vertical speed, acceleration and start clock.
// GLOBAL: MW2 0x100a243c
MechS32 g_unk0x100a243c = 0;

// GLOBAL: MW2 0x100a2440
MechS32 g_unk0x100a2440 = 0x3ca0;

// GLOBAL: MW2 0x100a2444
MechS32 g_unk0x100a2444 = 0;

// Set while no glance key is held (FUN_10011cb0).
// GLOBAL: MW2 0x100a2448
MechS8 g_unk0x100a2448 = 0;

// GLOBAL: MW2 0x100a244c
MechS32 g_drawModeIndex = -1;

// GLOBAL: MW2 0x100a2450
MechS32 g_initDrawModeParam2 = 1;

// GLOBAL: MW2 0x100a2454
MechS32 g_unk0x100a2454 = 0;

// The banner's file name, instead of sbannr (FUN_10012f3c).
// GLOBAL: MW2 0x100a2458
MechChar* g_unk0x100a2458 = NULL;

// GLOBAL: MW2 0x100a245c
void* g_unk0x100a245c = NULL;

// GLOBAL: MW2 0x100a2460
MechS32 g_unk0x100a2460 = 1;

// GLOBAL: MW2 0x100a2464
MechS32 g_unk0x100a2464 = 0;

// The ground's color (FUN_1004320b; the sky's is g_unk0x100a5548).
// GLOBAL: MW2 0x100a554c
MechS32 g_unk0x100a554c = 0xef;

// The horizon map's (LoadMapBitmap, from the world stream's hrzm record).
// GLOBAL: MW2 0x100a5550
MechS32 g_unk0x100a5550 = 0xea;

// GLOBAL: MW2 0x100a5558
MechS32 g_unk0x100a5558 = -1;

// GLOBAL: MW2 0x100a59e0
MechS32 g_menuRepeatTimer = -1;

// The in-mission menus by ID (RegisterMenu): 4 the main menu, 5 the systems status, 1 the lance
// command computer, 7 and 8 command points 2 and 3, 3 the programmers' page.
// GLOBAL: MW2 0x100a59e8
MenuDefinition* g_menuDefinitions[11] = {
	NULL,
	&g_commandMenu,
	NULL,
	&g_dorcsMenu,
	&g_mainMenu,
	&g_systemsMenu,
	NULL,
	&g_commandPoint2Menu,
	&g_commandPoint3Menu,
	NULL,
	NULL,
};

// The satellite view's display option, 0 or 1 (FUN_1003f74e).
// GLOBAL: MW2 0x100a5a18
MechS32 g_unk0x100a5a18 = 0;

// The frame callback the satellite view replaces (FUN_1003ddd7).
// GLOBAL: MW2 0x100a5a1c
void (*g_unk0x100a5a1c)(void) = FUN_10012afe;

// The overlay settings the satellite view keeps while a cockpit view shows (FUN_1003ddd7):
// g_unk0x100a5f1c's, g_unk0x100a5f18's and g_unk0x100a5f20's, and whether they are held.

// GLOBAL: MW2 0x100a5a20
MechS32 g_unk0x100a5a20 = 1;

// GLOBAL: MW2 0x100a5a24
undefined4 g_unk0x100a5a24 = 1;

// GLOBAL: MW2 0x100a5a28
MechS32 g_unk0x100a5a28 = 1;

// GLOBAL: MW2 0x100a5a2c
MechS32 g_unk0x100a5a2c = 0;

// The height the map view's shading starts at (FUN_1003f513).
// GLOBAL: MW2 0x100a5a30
MechS32 g_unk0x100a5a30 = 0;

// GLOBAL: MW2 0x100a5a34
MechS32 g_unk0x100a5a34 = 0x1900;

// The height range the map view's shading spans (FUN_1003f513).
// GLOBAL: MW2 0x100a5a38
MechS32 g_unk0x100a5a38 = 0x1900;

// The gauge functions of the cockpit layouts, by index.
// GLOBAL: MW2 0x100a5a40
CockpitGaugeFn g_cockpitGauges[10] = {
	NULL,
	(CockpitGaugeFn) FUN_100570e9,
	(CockpitGaugeFn) FUN_10057e56,
	NULL,
	(CockpitGaugeFn) FUN_10057fbe,
	(CockpitGaugeFn) FUN_10057a03,
	(CockpitGaugeFn) FUN_1005806a,
	(CockpitGaugeFn) FUN_10057ac4,
	(CockpitGaugeFn) FUN_1005816f,
	NULL
};

// GLOBAL: MW2 0x100a5a68
RenderTarget g_unk0x100a5a68[5] = {
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0}
};

// GLOBAL: MW2 0x100a5ad0
RenderTarget g_unk0x100a5ad0[8] = {
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0},
	{&g_mainPixelBuffer, 0, 0, 0, 0}
};

// GLOBAL: MW2 0x100a5b70
Point g_unk0x100a5b70[4] = {0};

// GLOBAL: MW2 0x100a5b90
Point g_unk0x100a5b90[4] = {0};

// GLOBAL: MW2 0x100a5bb0
Point g_unk0x100a5bb0 = {0, 0};

// GLOBAL: MW2 0x100a5bb8
void* g_unk0x100a5bb8[4] = {g_unk0x100a5b90, g_unk0x100a5b70, g_unk0x100a5ad0, &g_unk0x100a5bb0};

// The heading tape's shape width (FUN_10040f91).
// GLOBAL: MW2 0x100a5ed0
MechS32 g_unk0x100a5ed0 = 0xf0f;

// GLOBAL: MW2 0x100a5ed4
MechS32 g_unk0x100a5ed4 = 0xd79;

// The altimeter's place, in 16.16 fractions of its gauge until FUN_10040f91 scales it.
// GLOBAL: MW2 0x100a5ed8
Point g_unk0x100a5ed8 = {0xb333, 0x8000};

// The compass's place, likewise.
// GLOBAL: MW2 0x100a5ee0
Point g_unk0x100a5ee0 = {0x8000, 0x6666};

// GLOBAL: MW2 0x100a5ee8
Point g_unk0x100a5ee8[6] = {{0x73, 0x10}, {8, 0x4a}, {4, 0x28}, {4, 0x4a}, {0, 0}, {0, 0}};

// GLOBAL: MW2 0x100a5f18
undefined4 g_unk0x100a5f18 = 1;

// GLOBAL: MW2 0x100a5f1c
MechS32 g_unk0x100a5f1c = 1;

// GLOBAL: MW2 0x100a5f20
MechS32 g_unk0x100a5f20 = 1;

// GLOBAL: MW2 0x100a5f24
MechS32 g_unk0x100a5f24 = 1;

// GLOBAL: MW2 0x100a5f2c
MechS32 g_unk0x100a5f2c = 1;

// GLOBAL: MW2 0x100a6be0
Eyepoint g_unk0x100a6be0 = {0, 0,   0, 0,   0,    0,       0x10000, 1000, 10000, -1000, 1,  0x48,
							0, 319, 0, 199, 0x40, 0x249f0, 0,       0,    0,     0,     {0}};

// GLOBAL: MW2 0x100a6cc0
Eyepoint* g_eyepoint = &g_unk0x100a6be0;

// GLOBAL: MW2 0x100a6cc8
SlateHeron0x68 g_unk0x100a6cc8 = {0, 1, 1,       1,       1, 1, 1,    1,    1,    1, {0xe0, 0xef}, 1, 0, 0,
								  0, 0, 0x186a0, 0x10000, 0, 0, NULL, NULL, NULL, 0, NULL};

// GLOBAL: MW2 0x100a6d30
MechS32 g_unk0x100a6d30 = 0x24;

// GLOBAL: MW2 0x100a8674
const char* g_unk0x100a8674 = "SNDS";

// GLOBAL: MW2 0x100a8678
const char* g_unk0x100a8678 = "CEL";

// GLOBAL: MW2 0x100a8680
const char* g_unk0x100a8680 = "SHP";

// GLOBAL: MW2 0x100a8684
const char* g_unk0x100a8684 = "FONT";

// GLOBAL: MW2 0x100a8690
const char* g_unk0x100a8690 = "XMID";

// GLOBAL: MW2 0x100a8694
const char* g_unk0x100a8694 = "PAL";

// GLOBAL: MW2 0x100a8698
const char* g_unk0x100a8698 = "TABL";

// GLOBAL: MW2 0x100a869c
const char* g_unk0x100a869c = "POLY";

// GLOBAL: MW2 0x100a86a0
const char* g_unk0x100a86a0 = "TEXT";

// GLOBAL: MW2 0x100a86a4
const char* g_unk0x100a86a4 = "ANIM";

// GLOBAL: MW2 0x100a86a8
const char* g_unk0x100a86a8 = "MGEO";

// GLOBAL: MW2 0x100a86ac
const char* g_unk0x100a86ac = "HUD";

// GLOBAL: MW2 0x100a86b0
const char* g_unk0x100a86b0 = "CPIT";

// GLOBAL: MW2 0x100a86bc
const char* g_unk0x100a86bc = g_unk0x100a87c0;

// GLOBAL: MW2 0x100a86c4
const char* g_unk0x100a86c4 = "AIT";

// GLOBAL: MW2 0x100a86c8
const char* g_unk0x100a86c8 = "MEK";

// GLOBAL: MW2 0x100a86cc
const char* g_unk0x100a86cc = "LUMA";

// GLOBAL: MW2 0x100a86d0
const char* g_unk0x100a86d0 = "MUS";

// GLOBAL: MW2 0x100a8704
const char* g_unk0x100a8704 = ".wtb";

// GLOBAL: MW2 0x100a870c
const char* g_unk0x100a870c = ".3di";

// GLOBAL: MW2 0x100a8710
const char* g_unk0x100a8710 = ".mgi";

// GLOBAL: MW2 0x100a8714
const char* g_unk0x100a8714 = ".hdi";

// GLOBAL: MW2 0x100a8718
const char* g_unk0x100a8718 = ".cpi";

// GLOBAL: MW2 0x100a872c
const char* g_unk0x100a872c = ".mek";

// GLOBAL: MW2 0x100a8740
undefined4 g_unk0x100a8740 = 0xffffffff;

// GLOBAL: MW2 0x100a8744
MechChar* g_unk0x100a8744 = NULL;

// GLOBAL: MW2 0x100a87c0
char g_unk0x100a87c0[] = "BWD";

// Set by game key 0x11.
// GLOBAL: MW2 0x100aa2a0
MechS32 g_unk0x100aa2a0 = 0;

// GLOBAL: MW2 0x100aa2a4
MechS32 g_unk0x100aa2a4 = 1;

// GLOBAL: MW2 0x100aa2a8
MechS32 g_unk0x100aa2a8 = 0;

// GLOBAL: MW2 0x100aa2ac
MechS32 g_missionTimerStopped = 0;

// The "meepmeep" cheat: enables the time compression key.
// GLOBAL: MW2 0x100aa2b0
MechS32 g_unk0x100aa2b0 = 0;

// The length of the chat message being typed (HandleChatKey).
// GLOBAL: MW2 0x100aa2b8
MechS32 g_unk0x100aa2b8 = 0;

// GLOBAL: MW2 0x100aa2bc
MechS32 g_unk0x100aa2bc = 0;

// GLOBAL: MW2 0x100aa2c0
MechS32 g_unk0x100aa2c0 = 0;

// GLOBAL: MW2 0x100acb18
MechS32 g_shouldQuit = 0;

// GLOBAL: MW2 0x100acb1c
MechS32 g_quitStage = 0;

// GLOBAL: MW2 0x100acb20
TimedCallback* g_unk0x100acb20 = NULL;

// GLOBAL: MW2 0x100acb24
DifficultyCfg* g_difficulty = NULL;

// GLOBAL: MW2 0x100acb28
MechS32 g_localPlayerId = 0;

// GLOBAL: MW2 0x100acb2c
MechS32 g_unk0x100acb2c = 0;

// Set when the local player starts on the autopilot (FUN_10016ad0).
// GLOBAL: MW2 0x100acb34
MechS32 g_unk0x100acb34 = 0;

// GLOBAL: MW2 0x100acb60
HWND g_gameWindow = NULL;

// GLOBAL: MW2 0x100acb64
HINSTANCE g_unk0x100acb64 = NULL;

// GLOBAL: MW2 0x100acb68
HANDLE g_primaryHeap = NULL;

// GLOBAL: MW2 0x100acb6c
MechS32 g_gameWindowWidth = 0;

// GLOBAL: MW2 0x100acb70
MechS32 g_gameWindowHeight = 0;

// GLOBAL: MW2 0x100acb74
MechS32 g_windowActive = 0;

// GLOBAL: MW2 0x100acb78
MechS32 g_unk0x100acb78 = 0;

// GLOBAL: MW2 0x100acb7c
undefined4 g_shouldToggleFullscreen = 0;

// GLOBAL: MW2 0x100acb80
MechS32 g_desktopWidth = 0;

// GLOBAL: MW2 0x100acb84
MechS32 g_desktopHeight = 0;

// GLOBAL: MW2 0x100acb88
MechS32 g_simPaused = 0;

// GLOBAL: MW2 0x100acb8c
MechS32 g_pauseRequested = 0;

// GLOBAL: MW2 0x100acb90
undefined4 g_windowedSwitchPending = 0;

// GLOBAL: MW2 0x100acb94
MechS32 g_mouseOutsideClientWindow = 0;

// GLOBAL: MW2 0x100acb98
MechS32 g_goLaunch = 0;

// GLOBAL: MW2 0x100b1350
MechS32 g_unk0x100b1350 = 0;

// The frame draw callback ShowDorcs replaces.
// GLOBAL: MW2 0x100b1354
void (*g_dorcsPreviousDrawCallback)(void) = FUN_10012afe;

// GLOBAL: MW2 0x100bdff8
RenderTarget g_unk0x100bdff8;

// Set once the local mech's collision sound played; cleared when it moves freely (FUN_10016edf).
// GLOBAL: MW2 0x100be00c
MechS32 g_unk0x100be00c;

// GLOBAL: MW2 0x100bfd60
MechS32 g_unk0x100bfd60[800];

// GLOBAL: MW2 0x100c09e0
MechS32 g_unk0x100c09e0[800];

// GLOBAL: MW2 0x100e9240
MechU32 g_windowedSwitchTime;

// The name of the mission's music (the world stream's music record).
// GLOBAL: MW2 0x100e9330
MechChar g_unk0x100e9330[12];

// GLOBAL: MW2 0x100e933c
MechU32 g_windowedSwitchDeadline;

// The mission's "MUS" resource.
// GLOBAL: MW2 0x100e9340
MechS32 g_unk0x100e9340;

// GLOBAL: MW2 0x100e9350
undefined g_unk0x100e9350[0x100];

// GLOBAL: MW2 0x100e9614
MechS32 g_unk0x100e9614;

// GLOBAL: MW2 0x1012b7c0
VideoDriverChoice g_videoDriverChoice;

// GLOBAL: MW2 0x100c3358
MechS32 g_unk0x100c3358;

// GLOBAL: MW2 0x100ea3e4
MechS32 g_unk0x100ea3e4;

// GLOBAL: MW2 0x10138710
MechS32 g_missionTime;

// GLOBAL: MW2 0x10138720
MechS32 g_currentObjective[16]; // by team

// GLOBAL: MW2 0x10138830
StarMission g_objectiveTable[16];

// GLOBAL: MW2 0x10176ed0
RenderTarget g_currentRenderTarget;

// GLOBAL: MW2 0x10176ef0
PixelBuffer g_mainPixelBuffer;

// Whether each objective of the local team has been announced (FUN_1001b0cb).
// GLOBAL: MW2 0x10138760
MechS32 g_unk0x10138760[48];

// GLOBAL: MW2 0x10138820
MechS32 g_objectiveCount; // defined last for the operand order of the DoFirstObjtv loop test

void UpdateDebris(void);
void ZeroChunx(void);
void CollectMissionAudio(void);
MechS32 LoadWorld(char* p_unk0x00);
void AfterWorldLoader(void);
void FirstEyepoint(void);
MechS32 DoFirstObjtv(StarMission* p_mission, MechS32 p_team);
void UpdateObjectives(void);
void EndTheMission1(void);
MechS32 EndTheMission2(void);
void UpdateGeoCache(void);
void FirstStaticCache(void);
void FirstAI(void);
void SetRes(void);
void FirstShots(void);
void UpdateAllShots(void);
void UpdateEffects(void);
void SaveCarCfg(void);

// Matches except for the stack slots of seven locals (a consistent permutation; the original
// assigns them in declaration order, which VC++ 4.1 doesn't reproduce from this source). The
// operand order of the DoFirstObjtv loop test and of the network start test follows the unit's
// symbol table: both have flipped back and forth as declarations moved between units.
// FUNCTION: MW2 0x10066a50
int __stdcall SimMain(
	HINSTANCE p_module,
	undefined4 p_unk0x0c,
	LPSTR p_cmdLine,
	NetLaunchInfo* p_netLaunch,
	undefined4 p_isNetGameUnused,
	HWND p_hWnd
)
{
	MSG msg;
	void* palette;
	MechS32 hasPalette;
	int result;
	MechS32 i;
	char missionName[64];
	undefined4 unk0x28;
	MechS32 unk0x24;
	MechS32 seed;
	MechS32 quitLatched;

	quitLatched = 0;
	seed = 0;
	g_gameWindow = p_hWnd;
	g_unk0x100acb64 = p_module;
	g_desktopWidth = GetSystemMetrics(SM_CXSCREEN);
	g_desktopHeight = GetSystemMetrics(SM_CYSCREEN);
	g_primaryHeap = HeapCreate(HEAP_NO_SERIALIZE, 1000000, 0);
	if (g_primaryHeap == NULL) {
		Error(9, "Insufficient memory available.");
	}

	unk0x24 = 1;
	unk0x28 = 0;
	if (getenv("MECHWARRIOR")) {
		strcpy(g_gameDir, getenv("MECHWARRIOR"));
	}

	if (LoadSndCfg("mw2snd.cfg", &g_mw2SndCfgData) == -1) {
		if (g_mw2SndCfgData == NULL) {
			Error(0x11, "%s", "mw2snd.cfg");
		}
		else {
			*g_mw2SndCfgData = g_soundConfig;
		}
	}
	else {
		g_soundConfig = *g_mw2SndCfgData;
	}

	g_displayBrightness = g_unk0x100a946c = g_mw2SndCfgData->m_displayBrightness;
	g_videoDriverChoice.m_flags = 0;
	if (g_mw2SndCfgData->m_videoDriver[0]) {
		g_videoDriverChoice.m_flags |= 1;
		if (_stricmp(g_mw2SndCfgData->m_videoDriver, "scan") == 0) {
			g_videoDriverChoice.m_name[0] = 0;
		}
		else {
			strncpy(g_videoDriverChoice.m_name, g_mw2SndCfgData->m_videoDriver, 12);
			g_videoDriverChoice.m_name[12] = 0;
		}
	}

	if (!ProcessCmdLineArgs(p_cmdLine, &unk0x28, missionName)) {
		return 0;
	}

	if (StartupCheckStub()) {
		Error(0x51, NULL);
	}

	SetGameResolution(g_videoDriverChoice.m_name);
	if (g_logFileEnabled) {
		OpenMw2Log();
	}

	InitRefreshMode(5, 0, &g_mainPixelBuffer, 640, 480, 0);
	SendMessage(g_gameWindow, 0x41f, 0, 0);
	SendMessage(g_gameWindow, WM_ACTIVATEAPP, TRUE, 0);
	if (!InitDisplayGeometry()) {
		Error(0x50, NULL);
	}

	if (p_netLaunch) {
		g_isNetworkGame = 1;
		if (p_netLaunch->m_localPlayerId == 1) {
			g_netRole = 1;
		}
		else {
			g_netRole = 2;
		}

		strncpy(missionName, p_netLaunch->m_missionName, sizeof(missionName));
		if (LoadDifficultyCfg("MW2NET.CFG", &g_difficulty) == -1 || g_difficulty == NULL) {
			Error(0x11, "%s", "MW2NET.CFG");
		}
	}
	else {
		if (LoadDifficultyCfg("mw2dif.cfg", &g_difficulty) == -1 || g_difficulty == NULL) {
			Error(0x11, "%s", "mw2dif.cfg");
		}
	}

	DebugPrint("FirstClock()\n");
	__try {
		FirstClock();
		DebugPrint("StartSupAnim()\n");
		StartSupAnim(GetDeviceCaps(GetDC(g_gameWindow), NUMCOLORS) == -1 || g_windowMode == c_windowModeFullscreen);
		DebugPrint("FirstResource()\n");
		FirstResource();
		DebugPrint("InitStaticMem()\n");
		InitStaticMem(missionName);
		DebugPrint("generate_gammas()\n");
		InitGammaTable();
		DebugPrint("InitRandom()\n");
		InitRandom(seed);
		DebugPrint("FirstAudio()\n");
		FirstAudio();
		DebugPrint("FirstRender()\n");
		FirstRender();
		DebugPrint("FirstNetwork()\n");
		FirstNetwork(p_netLaunch);
		DebugPrint("ResetClocks()\n");
		ResetClocks();
		DebugPrint("WinMain(1): pause_timer(TRUE)");
		PauseTimer(0x80, TRUE);
		DebugPrint("FirstShots()\n");
		FirstShots();
		DebugPrint("ZeroGamethings()\n");
		ZeroGamethings();
		DebugPrint("ZeroChunx()\n");
		ZeroChunx();
		DebugPrint("LoadWorld()\n");
		LoadWorld(missionName);
		DebugPrint("CollectMissionAudio()\n");
		CollectMissionAudio();
		DebugPrint("SetRes()\n");
		SetRes();
		DebugPrint("FirstEnvironment()\n");
		FirstEnvironment();
		DebugPrint("FirstStaticCache()\n");
		FirstStaticCache();
		DebugPrint("CachePreloads()\n");
		CachePreloads();
		DebugPrint("AfterWorldLoader()\n");
		AfterWorldLoader();
		DebugPrint("FirstGPAnim()\n");
		FirstGPAnim();
		DebugPrint("FirstEyepoint()\n");
		FirstEyepoint();
		DebugPrint("FirstInputs()\n");
		FirstInputs();
		DebugPrint("FirstMenu()\n");
		FirstMenu();
		DebugPrint("RegisterMenu()...\n");
		RegisterMenu(4);
		RegisterMenu(5);
		RegisterMenu(1);
		RegisterMenu(7);
		RegisterMenu(8);
		RegisterMenu(3);
		DebugPrint("DoFirstObjtv()...\n");
		for (i = 0; i < g_objectiveCount; i++) {
			DoFirstObjtv(&g_objectiveTable[i], i);
		}

		DebugPrint("FirstClassFunctions()\n");
		FirstClassFunctions();
		DebugPrint("FirstAI()\n");
		FirstAI();
		DebugPrint("FirstExternalCtrl()\n");
		if (!FirstExternalCtrl()) {
			g_shouldQuit = 1;
			g_quitStage = 3;
		}

		DebugPrint("UpdateGeoCache()\n");
		UpdateGeoCache();
		DebugPrint("SecondRender()\n");
		SecondRender();
		DebugPrint("FirstPerfSetting()\n");
		FirstPerfSetting();
		if (g_missionTimerStopped) {
			DebugPrint("SimEntranceDbug()\n");
			SimEntranceDbug(missionName, 0);
		}

		DebugPrint("StopSupAnim()\n");
		StopSupAnim();
		if (GetDeviceCaps(GetDC(g_gameWindow), NUMCOLORS) == -1 || g_windowMode == c_windowModeFullscreen) {
			DebugPrint("StartPalettes()\n");
			StartPalettes(0);
		}
		else {
			hasPalette = g_paletteResourceIds[0x10] != -1;
			if (hasPalette) {
				palette = FUN_1001a19f(g_unk0x100a8740, hasPalette, g_unk0x100a8694, 0);
				if (palette) {
					g_currentDisplayBackend->m_setPalette(0, 0x100, palette, 1);
				}
			}
		}

		g_unk0x100acb78 = 1;
		DebugPrint("InitDrawMode()\n");
		if (!InitRefreshMode(
				g_drawModeIndex,
				g_initDrawModeParam2,
				&g_mainPixelBuffer,
				g_gameWindowWidth,
				g_gameWindowHeight,
				0
			)) {
			Error(0x50, "Error profiling video modes.");
		}

		while (ShowCursor(FALSE) >= 0) {
		}

		g_mouseOutsideClientWindow = 0;
		if (!g_refreshModeFallback) {
			g_goLaunch |= 2;
		}

		g_carCfg.m_unk0x1d = 1;
		g_carCfg.m_unk0xd2 = -1;
		while (g_quitStage < 3) {
			if (g_goLaunch == 3) {
				DebugPrint("GoLaunch == GO_READY\n");
				g_goLaunch |= 0x80000000;
				DebugPrint("WinMain(2): pause_timer(false)");
				PauseTimer(0x80, FALSE);
				StartMissionMusic();
				g_unk0x100aa2c0 = 0;
			}
			else if (g_isNetworkGame && !(g_goLaunch & 0x80000000)) {
				if (g_unk0x100acb2c == 0) {
					g_unk0x100acb2c = g_unk0x100ba54c + 0xb5;
				}
				else if (g_unk0x100acb2c < g_unk0x100ba54c) {
					g_unk0x100aa2c0 = 3;
				}
			}

			HandleMessages();
			UpdateNetwork();
			NextClock();
			UpdateInputs();
			UpdateMenuKey();
			HandleGameKeys(0, 0, 0);
			RunTimedCallbacks(&g_unk0x100acb20);
			if (!g_isNetworkGame || (g_goLaunch & 0x80000000)) {
				UpdateAllPlayers();
			}

			UpdateEyepoint();
			UpdateAllShots();
			UpdateEffects();
			UpdateLocalPlayer();
			LateUpdateAllPlayers();
			UpdateDebris();
			if (g_refreshModeFallback) {
				while (g_refreshModeFallback) {
					if (g_currentDisplayBackend->m_id == 0) {
						DdrawFill(0, 0, g_gameWindowWidth, g_gameWindowHeight, g_unk0x100a554c);
					}

					if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
						g_unk0x100a6cc8.m_frameDrawCallback();
						DrawLocalPlayer();
						UpdateMenus();
						DrawTimedOverlays();
						FUN_10058750();
					}

					Blit();
					ProfileRefreshModes();
				}
			}

			if (g_goLaunch & 0x80000000) {
				UpdateGeoCache();
			}

			AdvanceAnimations();
			UpdatePaletteFade();
			ApplyPendingPalette();
			if (g_windowActive && g_currentDisplayBackend->m_id == 0) {
				DdrawFill(0, 0, g_gameWindowWidth, g_gameWindowHeight, g_unk0x100a554c);
			}

			if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
				g_unk0x100a6cc8.m_frameDrawCallback();
				DrawLocalPlayer();
				UpdateMenus();
				DrawTimedOverlays();
				FUN_10058750();
				if (g_simPaused && g_pauseRequested) {
					DrawPausedBanner();
				}
			}

			Blit();
			if (g_goLaunch & 0x80000000) {
				DoAudio();
				AdvanceSpeechQueue();
			}

			FUN_1007d6bb();
			UpdateObjectives();
			LoopCdMusic();
			UpdatePauseState();
			if (g_shouldQuit && !quitLatched) {
				g_quitStage++;
				quitLatched = 1;
				g_unk0x100acb78 = 0;
			}

			g_goLaunch |= 2;
		}

		FadeToEndPalette(g_carCfg.m_unk0x1d & 4);
		if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
			FillView(&g_currentRenderTarget, 0);
		}

		DebugPrint("Calling Blit()\n");
		Blit();
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		}

		SendMessage(g_gameWindow, 0x41e, 0, 0);
		SaveCarCfg();
		ShutdownAllPlayers();
		ShutdownNetwork();
		ShutdownAudio();
		FreeMenus();
		StopTimers();
		CloseResourceFile();
		ShutdownRender();
		CloseInputDevices();
		if (g_logFileEnabled) {
			CloseMw2Log();
		}

		DebugPrint("Calling EndTheMission()\n");
		EndTheMission1();
		EndTheMission2();
	}
	__finally {
		if (AbnormalTermination() && g_ticksTimerInitialized) {
			SendMessage(g_gameWindow, 0x41e, 0, 0);
			MessageBox(NULL, "Attempting to shutdown from an unknown fatal error.", "MECHWARRIOR 2", MB_ICONHAND);
			AIL_shutdown();
		}
	}

	HeapDestroy(g_primaryHeap);
	g_primaryHeap = NULL;
	while (ShowCursor(TRUE) < 1) {
	}

	result = g_unk0x100b1350 ? 0xff : 0;
	return result;
}

// Operand order: the timer test's time > g_windowedSwitchDeadline loads g_windowedSwitchDeadline
// first in the original. The test's comparisons follow the unit's symbol table: the other two flipped when
// the shell's Miles declarations joined mss.h and back when the unit's declarations moved into
// headers.
// FUNCTION: MW2 0x10067757
LRESULT CALLBACK SimWindowProc(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	WINDOWPOS* windowPos;

	if (p_msg >= WM_KEYFIRST && p_msg <= WM_KEYLAST) {
		HandleKeyboardMessages(p_msg, p_wParam, p_lParam);
		return 0;
	}

	switch (p_msg) {
	case WM_ACTIVATEAPP:
		if (g_shouldQuit) {
			break;
		}

		g_windowActive = p_wParam;
		if (g_windowActive == TRUE) {
			KeyboardClearKeyStates();
			if (g_desktopWidth <= 640 && g_desktopHeight <= 480) {
				if (g_shouldToggleFullscreen == TRUE) {
					ToggleFullScreen();
					g_shouldToggleFullscreen = FALSE;
					ShowWindow(g_gameWindow, SW_RESTORE);
				}
				else if (g_currentDisplayBackend && g_currentDisplayBackend->m_id == 1) {
					ShowWindow(g_gameWindow, SW_SHOWNOACTIVATE);
				}
			}
			else {
				if (g_windowMode == c_windowModeFullscreen) {
					ShowWindow(g_gameWindow, SW_RESTORE);
				}
			}
		}
		else {
			if (g_desktopWidth <= 640 && g_desktopHeight <= 480) {
				if (g_currentDisplayBackend && g_currentDisplayBackend->m_id == 0 &&
					g_shouldToggleFullscreen == FALSE) {
					ShowWindow(g_gameWindow, SW_MINIMIZE);
					g_shouldToggleFullscreen = TRUE;
					ToggleFullScreen();
				}
				else if (g_currentDisplayBackend && g_currentDisplayBackend->m_id == 1) {
					ShowWindow(g_gameWindow, SW_HIDE);
				}
			}
			else {
				if (g_windowMode == c_windowModeFullscreen) {
					ShowWindow(g_gameWindow, SW_MINIMIZE);
				}
			}
		}

		if (g_currentDisplayBackend) {
			g_currentDisplayBackend->m_setPalette(0, 0x100, g_paletteColors, 1);
		}
		return 0;
	case WM_PAINT:
		if (g_unk0x100acb78 && g_windowMode == c_windowModeWindowed) {
			g_currentRefreshMode->m_flip();
			ValidateRect(p_hWnd, NULL);
			return 0;
		}
		else {
			break;
		}
	case WM_NCMOUSEMOVE:
		if (g_mouseOutsideClientWindow == FALSE && g_windowMode == c_windowModeWindowed) {
			while (ShowCursor(TRUE) < 0) {
			}
			g_mouseOutsideClientWindow = TRUE;
		}
		break;
	case WM_MOUSEMOVE:
		if (g_mouseOutsideClientWindow) {
			if (!g_simPaused || GetMenuSlotState(4) == 1) {
				while (ShowCursor(FALSE) >= 0) {
				}
				g_mouseOutsideClientWindow = FALSE;
			}
		}
		return 0;
	case WM_WINDOWPOSCHANGING:
		windowPos = (WINDOWPOS*) p_lParam;
		if (g_windowedSwitchPending) {
			LONG time;

			time = GetMessageTime();
			if (time > g_windowedSwitchDeadline &&
				(g_windowedSwitchDeadline > g_windowedSwitchTime || time < g_windowedSwitchTime)) {
				g_windowedSwitchPending = FALSE;
			}
			else {
				windowPos->x = g_windowedRect.left;
				windowPos->y = g_windowedRect.top;
				windowPos->cx = g_windowedRect.right;
				windowPos->cy = g_windowedRect.bottom;
			}
			return 0;
		}
		else {
			break;
		}
	case WM_QUERYNEWPALETTE:
		if (g_currentDisplayBackend) {
			g_currentDisplayBackend->m_setPalette(0, 0x100, g_paletteColors, 1);
			return 1;
		}
		else {
			return 0;
		}
	case WM_CLOSE:
		g_shouldQuit = TRUE;
		g_quitStage += 2;
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	}

	return DefWindowProc(p_hWnd, p_msg, p_wParam, p_lParam);
}

// FUNCTION: MW2 0x10067bbc
void HandleMessages(void)
{
	MSG msg;

	if (!g_windowActive) {
		WaitMessage();
	}

	if (!g_shouldQuit && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		while (!g_mouseOutsideClientWindow && msg.message >= WM_MOUSEFIRST && msg.message <= WM_MBUTTONDBLCLK) {
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		}

		if (msg.hwnd != NULL && msg.message == WM_QUIT) {
			g_shouldQuit = TRUE;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

// FUNCTION: MW2 0x10067c79
void UpdatePauseState(void)
{
	if (g_pauseRequested) {
		if (g_simPaused && g_localSteering.m_keyCode) {
			PlayResumeSound();
			g_localSteering.m_keyCode = 0;
			g_pauseRequested = FALSE;
		}
		else if (g_netRole || GetMenuSlotState(4)) {
			g_pauseRequested = FALSE;
		}
	}

	if (!GetMenuSlotState(4) && g_windowActive && !g_pauseRequested) {
		if (g_simPaused) {
			while (ShowCursor(FALSE) >= 0) {
			}

			g_mouseOutsideClientWindow = FALSE;
			DebugPrint("WinMain(3): pause_timer(false)");
			PauseTimer(0x80, FALSE);
			ResumeAudio();
			EnableGameplayInput();
			g_simPaused = FALSE;
		}
	}
	else if (!g_simPaused && !g_netRole) {
		if (g_pauseRequested) {
			PlayPauseSound();
			g_localSteering.m_keyCode = 0;
		}

		if (!GetMenuSlotState(4) && g_windowMode != c_windowModeFullscreen) {
			while (ShowCursor(TRUE) < 0) {
			}

			g_mouseOutsideClientWindow = TRUE;
		}

		DebugPrint("WinMain(4): pause_timer(TRUE)");
		PauseTimer(0x80, TRUE);
		PauseAudio();
		DisableGameplayInput();
		g_simPaused = TRUE;
	}
}

// FUNCTION: MW2 0x10067e23
void SetGameResolution(char* p_driverName)
{
	if (_strcmpi(p_driverName, "MCGA.DLL") == 0) {
		g_gameWindowWidth = 320;
		g_gameWindowHeight = 200;
	}
	else if (_strcmpi(p_driverName, "VESA480.DLL") == 0) {
		g_gameWindowWidth = 640;
		g_gameWindowHeight = 480;
	}
	else if (_strcmpi(p_driverName, "VESA768.DLL") == 0) {
		g_gameWindowWidth = 1024;
		g_gameWindowHeight = 768;
	}
	else {
		g_gameWindowWidth = 320;
		g_gameWindowHeight = 200;
	}
}
