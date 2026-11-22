#include "simmain.h"

#include "ai.h"
#include "animation.h"
#include "audio.h"
#include "brightness.h"
#include "callbacks.h"
#include "clock.h"
#include "cmdline.h"
#include "cockpit.h"
#include "collision.h"
#include "commandmenu.h"
#include "commandpointmenu.h"
#include "compat.h"
#include "config.h"
#include "debris.h"
#include "debugprint.h"
#include "decomp.h"
#include "directdraw.h"
#ifdef MW2_3DFX
#include "3dfx/glidedraw.h"
#endif
#include "dispdibmode.h"
#include "displaybackend.h"
#include "dorcs.h"
#include "effectinfo.h"
#include "environment.h"
#include "error.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "gamekeys.h"
#include "gdi.h"
#include "geocache.h"
#include "gpanim.h"
#include "inputmap.h"
#include "keyboard.h"
#include "loadres.h"
#include "mainmenu.h"
#include "menu.h"
#include "missionaudio.h"
#include "mss.h"
#include "mw2log.h"
#include "mw2prj.h"
#include "netlaunchinfo.h"
#include "network.h"
#include "objective.h"
#include "overlay.h"
#include "palette.h"
#include "palettecolor.h"
#include "pausebanner.h"
#include "perf.h"
#include "players.h"
#include "point.h"
#include "polydraw.h"
#ifdef MW2_REGISTRY_SETTINGS
#include "accel/registry.h"
#endif
// Both accelerated editions' sources include it (the 3Dfx edition's code shows it), for the Matrox
// edition's g_reclipCursor.
#ifdef MW2_ACCEL
#include "mouse.h"
#endif
#ifdef MW2_A3D
#include "accel/a3d.h"
#endif
#ifdef MW2_MATROX
#include "matrox/msidraw.h"
#endif
#ifdef MW2_POWERVR
#include "powervr/a3dclient.h"
#endif
#include "random.h"
#include "refreshmode.h"
#include "render.h"
#include "resource.h"
#include "screenscale.h"
#include "setres.h"
#include "settings.h"
#include "shots.h"
#include "speech.h"
#include "startup.h"
#include "staticmem.h"
#include "supanim.h"
#include "targeting.h"
#include "ticks.h"
#include "timedoverlays.h"
#include "types.h"
#include "videodriverchoice.h"
#include "weapons.h"
#include "world.h"

#include <excpt.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#ifdef MW2_MATROX
#include <msi95.h>
#endif
#ifdef MW2_S3
#include <stdio.h>
#endif

#ifdef MW2_VFX16
#include "accel/vfx16.h"
#endif

DECOMP_SIZE_ASSERT(SoundConfig, 0x3c)
DECOMP_SIZE_ASSERT(StarMission, 0x3c0a)
DECOMP_SIZE_ASSERT(MissionObjective, 0x13f)

// GLOBAL: MW2 0x100acb18
// GLOBAL: MW2GBL 0x100a4548
// GLOBAL: MW2MATROX 0x100be4c0
// GLOBAL: MW23DFX 0x1024ac60
// GLOBAL: MW2ATI 0x1017f098
// GLOBAL: MW2S3 0x10065ce0
// GLOBAL: MW2POWERVR 0x101a9e14
MechS32 g_shouldQuit = 0;

// GLOBAL: MW2 0x100acb1c
// GLOBAL: MW2GBL 0x100a454c
// GLOBAL: MW2MATROX 0x100be4c4
// GLOBAL: MW23DFX 0x1024ac64
// GLOBAL: MW2ATI 0x1017f09c
// GLOBAL: MW2S3 0x10065ce4
// GLOBAL: MW2POWERVR 0x101a9e18
MechS32 g_quitStage = 0;

// GLOBAL: MW2 0x100acb20
// GLOBAL: MW2GBL 0x100a4550
// GLOBAL: MW2MATROX 0x100be4c8
// GLOBAL: MW23DFX 0x1024ac68
// GLOBAL: MW2S3 0x10065ce8
// GLOBAL: MW2POWERVR 0x101a9e1c
TimedCallback* g_detachedTasks = NULL;

// GLOBAL: MW2 0x100acb24
// GLOBAL: MW2GBL 0x100a4554
// GLOBAL: MW2MATROX 0x100be4cc
// GLOBAL: MW23DFX 0x1024ac6c
// GLOBAL: MW2ATI 0x1017f0a4
// GLOBAL: MW2S3 0x10065cec
// GLOBAL: MW2POWERVR 0x101a9e20
DifficultyCfg* g_difficulty = NULL;

// GLOBAL: MW2 0x100acb28
// GLOBAL: MW2GBL 0x100a4558
// GLOBAL: MW2MATROX 0x100be4d0
// GLOBAL: MW23DFX 0x1024ac70
// GLOBAL: MW2ATI 0x1017f0a8
// GLOBAL: MW2S3 0x10065cf0
// GLOBAL: MW2POWERVR 0x101a9e24
MechS32 g_localPlayerId = 0;

// GLOBAL: MW2 0x100acb2c
// GLOBAL: MW2MATROX 0x100be4d4
// GLOBAL: MW23DFX 0x1024ac74
// GLOBAL: MW2POWERVR 0x101a9e28
MechS32 g_remoteWaitTime = 0;

// Set when the local player starts on the autopilot (FirstMech).
// GLOBAL: MW2 0x100acb34
// GLOBAL: MW2GBL 0x100a4564
// GLOBAL: MW2MATROX 0x100be4dc
// GLOBAL: MW23DFX 0x1024ac7c
// GLOBAL: MW2ATI 0x1017f0b4
// GLOBAL: MW2S3 0x10065cfc
// GLOBAL: MW2POWERVR 0x101a9e30
MechS32 g_startOnAutopilot = 0;

#ifdef MW2_MATROX
// The Matrox edition's: the window SimMain was given (the shell's), next to the renderer's.
// GLOBAL: MW2MATROX 0x100be508
HWND g_unk0x100be508 = NULL;

#endif
// GLOBAL: MW2 0x100acb60
// GLOBAL: MW2GBL 0x100a4594
// GLOBAL: MW2MATROX 0x100be50c
// GLOBAL: MW23DFX 0x1024aca8
// GLOBAL: MW2ATI 0x1017f0e0
// GLOBAL: MW2S3 0x10065d28
// GLOBAL: MW2POWERVR 0x101a9e58
HWND g_gameWindow = NULL;

// GLOBAL: MW2 0x100acb64
// GLOBAL: MW2MATROX 0x100be510
// GLOBAL: MW23DFX 0x1024acac
// GLOBAL: MW2POWERVR 0x101a9e5c
HINSTANCE g_simModule = NULL;

// GLOBAL: MW2 0x100acb68
// GLOBAL: MW2GBL 0x100a459c
// GLOBAL: MW2MATROX 0x100be514
// GLOBAL: MW23DFX 0x1024acb0
// GLOBAL: MW2ATI 0x1017f0e8
// GLOBAL: MW2S3 0x10065d30
// GLOBAL: MW2POWERVR 0x101a9e60
HANDLE g_primaryHeap = NULL;

// GLOBAL: MW2 0x100acb6c
// GLOBAL: MW2GBL 0x100a45a0
// GLOBAL: MW2MATROX 0x100be518
// GLOBAL: MW23DFX 0x1024acb4
// GLOBAL: MW2ATI 0x1017f0ec
// GLOBAL: MW2S3 0x10065d34
// GLOBAL: MW2POWERVR 0x101a9e64
MechS32 g_gameWindowWidth = 0;

// GLOBAL: MW2 0x100acb70
// GLOBAL: MW2GBL 0x100a45a4
// GLOBAL: MW2MATROX 0x100be51c
// GLOBAL: MW23DFX 0x1024acb8
// GLOBAL: MW2ATI 0x1017f0f0
// GLOBAL: MW2S3 0x10065d38
// GLOBAL: MW2POWERVR 0x101a9e68
MechS32 g_gameWindowHeight = 0;

// GLOBAL: MW2 0x100acb74
// GLOBAL: MW2GBL 0x100a45a8
// GLOBAL: MW2MATROX 0x100be520
// GLOBAL: MW23DFX 0x1024acbc
// GLOBAL: MW2ATI 0x1017f0f4
// GLOBAL: MW2S3 0x10065d3c
// GLOBAL: MW2POWERVR 0x101a9e6c
MechS32 g_windowActive = 0;

// GLOBAL: MW2 0x100acb78
// GLOBAL: MW2GBL 0x100a45ac
// GLOBAL: MW2MATROX 0x100be524
// GLOBAL: MW23DFX 0x1024acc0
// GLOBAL: MW2S3 0x10065d40
// GLOBAL: MW2POWERVR 0x101a9e70
MechS32 g_drawModeReady = 0;

// GBL defines these three in refreshmode.c.
#ifndef MW2_GBL
// GLOBAL: MW2 0x100acb7c
// GLOBAL: MW2MATROX 0x100be528
// GLOBAL: MW23DFX 0x1024acc4
undefined4 g_shouldToggleFullscreen = 0;

// GLOBAL: MW2 0x100acb80
// GLOBAL: MW2MATROX 0x100be52c
// GLOBAL: MW23DFX 0x1024acc8
// GLOBAL: MW2S3 0x10065d48
// GLOBAL: MW2POWERVR 0x101a9e78
MechS32 g_desktopWidth = 0;

// GLOBAL: MW2 0x100acb84
// GLOBAL: MW2MATROX 0x100be530
// GLOBAL: MW23DFX 0x1024accc
// GLOBAL: MW2POWERVR 0x101a9e7c
MechS32 g_desktopHeight = 0;
#endif

// GLOBAL: MW2 0x100acb88
// GLOBAL: MW2GBL 0x100a45b0
// GLOBAL: MW2MATROX 0x100be534
// GLOBAL: MW23DFX 0x1024acd0
// GLOBAL: MW2ATI 0x1017f108
// GLOBAL: MW2S3 0x10065d50
// GLOBAL: MW2POWERVR 0x101a9e80
MechS32 g_simPaused = 0;

// GLOBAL: MW2 0x100acb8c
// GLOBAL: MW2GBL 0x100a45b4
// GLOBAL: MW2MATROX 0x100be538
// GLOBAL: MW23DFX 0x1024acd4
// GLOBAL: MW2ATI 0x1017f10c
// GLOBAL: MW2S3 0x10065d54
// GLOBAL: MW2POWERVR 0x101a9e84
MechS32 g_pauseRequested = 0;

// GLOBAL: MW2 0x100acb90
// GLOBAL: MW2GBL 0x100a45b8
// GLOBAL: MW2MATROX 0x100be53c
// GLOBAL: MW23DFX 0x1024acd8
// GLOBAL: MW2ATI 0x1017f110
// GLOBAL: MW2S3 0x10065d58
// GLOBAL: MW2POWERVR 0x101a9e88
undefined4 g_windowedSwitchPending = 0;

// GLOBAL: MW2 0x100acb94
// GLOBAL: MW2GBL 0x100a45bc
// GLOBAL: MW2MATROX 0x100be540
// GLOBAL: MW23DFX 0x1024acdc
// GLOBAL: MW2ATI 0x1017f114
// GLOBAL: MW2S3 0x10065d5c
// GLOBAL: MW2POWERVR 0x101a9e8c
MechS32 g_mouseOutsideClientWindow = 0;

#ifdef MW2_MATROX
// The Matrox edition's: set by SimMain, keeps SimWindowProc from handling the shell's window's
// activation.
// GLOBAL: MW2MATROX 0x100be544
MechS32 g_unk0x100be544 = 0;

#endif
// GLOBAL: MW2 0x100acb98
// GLOBAL: MW2GBL 0x100a45c0
// GLOBAL: MW2MATROX 0x100be548
// GLOBAL: MW23DFX 0x1024ace0
// GLOBAL: MW2ATI 0x1017f118
// GLOBAL: MW2S3 0x10065d60
// GLOBAL: MW2POWERVR 0x101a9e90
MechS32 g_goLaunch = 0;

#ifdef MW2_POWERVR
// The first frame SGL hands over to FUN_10007db0 is only cleared.
// GLOBAL: MW2POWERVR 0x101a9e94
MechS32 g_unk0x101a9e94 = 1;
#endif

// GLOBAL: MW2 0x100e9240
// GLOBAL: MW2GBL 0x101331b4
// GLOBAL: MW2MATROX 0x1012595c
// GLOBAL: MW23DFX 0x1024429c
// GLOBAL: MW2ATI 0x10177f5c
// GLOBAL: MW2S3 0x100abe7c
// GLOBAL: MW2POWERVR 0x101a2c1c
MechU32 g_windowedSwitchTime;

// GLOBAL: MW2 0x100e933c
// GLOBAL: MW2GBL 0x101332ac
// GLOBAL: MW2MATROX 0x10125958
// GLOBAL: MW23DFX 0x10244298
// GLOBAL: MW2ATI 0x10177f58
// GLOBAL: MW2S3 0x100abf6c
// GLOBAL: MW2POWERVR 0x101a2c18
MechU32 g_windowedSwitchDeadline;

// GLOBAL: MW2 0x1012b7c0
// GLOBAL: MW2GBL 0x100e9a80
// GLOBAL: MW2MATROX 0x101d69c0
// GLOBAL: MW23DFX 0x10159ea0
// GLOBAL: MW2ATI 0x1008dea0
// GLOBAL: MW2S3 0x100b1d90
// GLOBAL: MW2POWERVR 0x100843e0
VideoDriverChoice g_videoDriverChoice;

#ifdef MW2_REGISTRY_SETTINGS
// The accelerated editions': reads the renderer's settings from the registry, the sky and the
// ground (drawn unless set to 0) and the Matrox edition's texture filter or the 3Dfx edition's
// resolution (off unless set).
#ifdef MW2_POWERVR
// The PowerVR edition's reads the renderer's fog and smooth shading (on unless set to 0).
// FUNCTION: MW2POWERVR 0x10007d40
void FUN_1008fad0(void)
{
	DWORD value;

	if (ReadRegistryDword("Fog", &value)) {
		A3D_SetQuality(&g_a3dMainView, c_a3dQualityFog, value);
	}
	else {
		A3D_SetQuality(&g_a3dMainView, c_a3dQualityFog, 1);
	}

	if (ReadRegistryDword("Smooth Shading", &value)) {
		A3D_SetQuality(&g_a3dMainView, c_a3dQualitySmoothShading, value);
	}
	else {
		A3D_SetQuality(&g_a3dMainView, c_a3dQualitySmoothShading, 1);
	}
}

// The 2D drawing of the PowerVR edition's frame: SGL calls it (through the renderer's DirectDraw
// mode callback) with the frame's pixels locked, and the game draws the cockpit, the menus and the
// overlays into them, then reads the input.
// FUNCTION: MW2POWERVR 0x10007db0
void FUN_10007db0(void* p_pixels, MechS32 p_pitch)
{
	if (p_pixels != NULL && g_a3dLightsChanged) {
		g_mainPixelBuffer.m_buffer = p_pixels;
		g_mainPixelBuffer.m_xMax = p_pitch - 1;
		if (g_unk0x101a9e94) {
			VFX_pane_wipe(&g_currentPane, VFX_lookaside_read16(0));
			g_unk0x101a9e94 = 0;
			return;
		}

		if (g_cockpitLayoutIndex == 4) {
			FUN_10003310();
		}

		DrawLocalPlayer();
		UpdateMenus();
		DrawTimedOverlays();
		DrawDebugOverlays();
		if (g_simPaused && g_pauseRequested) {
			DrawPausedBanner();
		}

		g_mainPixelBuffer.m_buffer = NULL;
		UpdateInputs();
		UpdateMenuKey();
		HandleGameKeys(0, 0, 0);
	}
}
#else
// FUNCTION: MW2MATROX 0x1008fad0
// FUNCTION: MW23DFX 0x100087e0
// FUNCTION: MW2ATI 0x10008fd0
// FUNCTION: MW2S3 0x10026680
void FUN_1008fad0(void)
{
#ifdef MW2_RES_800X600
	DWORD res800x600;
#endif
#ifdef MW2_TEXTURE_FILTER
	DWORD filter;
#endif

	if (!ReadRegistryDword("Sky", (DWORD*) &g_renderSettings.m_texturedSky)) {
		g_renderSettings.m_texturedSky = 1;
	}

	if (!ReadRegistryDword("Ground", (DWORD*) &g_renderSettings.m_texturedGround)) {
		g_renderSettings.m_texturedGround = 1;
	}

#ifdef MW2_RES_800X600
	if (!ReadRegistryDword("Res800x600", &res800x600)) {
		res800x600 = 0;
	}

	FUN_10066920(res800x600);
#endif
#ifdef MW2_TEXTURE_FILTER
	if (!ReadRegistryDword("Filter", &filter)) {
		filter = 1;
	}

	A3D_SET_TEXTURE_FILTER(filter);
#endif
}
#endif
#endif

#ifdef MW2_MATROX
// The Matrox edition's SimMain. It logs its command line, runs at 640x400 without asking
// SetGameResolution, reads the sky and ground settings, and hands the game window over to the
// renderer's own (g_msiWindow) for the main loop, hiding the launcher's. Like the 3Dfx edition's,
// it computes the bitmaps' average colours after loading the world, updates the players every
// frame before a network launch, which starts when the go state is 7: each main-loop pass
// sets bit 4 too. It syncs with the renderer each frame and has no refresh mode fallback loop.
// FUNCTION: MW2MATROX 0x1008fb5a
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
	MechChar cmdLine[512];

	quitLatched = 0;
	seed = 0;
	g_unk0x100be544 = 0;
	g_unk0x100be508 = g_gameWindow = p_hWnd;
	g_simModule = p_module;
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

	g_displayBrightness = g_brightnessSetting = g_mw2SndCfgData->m_displayBrightness;
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

	strcpy(cmdLine, p_cmdLine);
	if (!ProcessCmdLineArgs(p_cmdLine, &unk0x28, missionName)) {
		return 0;
	}

	DebugPrint("SimMain(): lpszCmdParam = %s\n\n", cmdLine);
	if (StartupCheckStub()) {
		Error(0x51, NULL);
	}

	// The Matrox edition always runs at 640x400.
	g_gameWindowWidth = 640;
	g_gameWindowHeight = 400;
	if (g_logFileEnabled) {
		OpenMw2Log();
	}

	InitRefreshMode(5, 0, &g_mainPixelBuffer, 640, 480, 0);
	SendMessage(g_unk0x100be508, 0x41f, 0, 0);
	SendMessage(g_unk0x100be508, WM_ACTIVATEAPP, TRUE, 0);
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

	LoadSkyGroundSettings(missionName);
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
		DebugPrint("WinMain(1): pause_timer(TRUE)\n");
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
		DebugPrint("CalcAverageBitmapColors()\n");
		CalcAverageBitmapColors();
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
				palette = LoadCachedResource(g_mw2PrjHandle, hasPalette, g_resTagPal, 0);
				if (palette) {
					g_currentDisplayBackend->m_setPalette(0, 0x100, palette, 1);
				}
			}
		}

		g_drawModeReady = 1;
		g_unk0x100be544 = 1;
		DebugPrint("InitDrawMode(%d,%d)\n", g_gameWindowWidth, g_gameWindowHeight);
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

		FUN_1008fad0();
		while (ShowCursor(FALSE) >= 0) {
		}

		g_mouseOutsideClientWindow = 0;
		if (!g_refreshModeFallback) {
			g_goLaunch |= 2;
		}

		g_careerRecord.m_outcome = 1;
		g_careerRecord.m_winner = -1;
		DebugPrint("Entering Main Loop...\n");
		g_gameWindow = g_msiWindow;
		g_reclipCursor = 1;
		ShowWindow(g_unk0x100be508, SW_HIDE);
		while (g_quitStage < 3) {
			if (g_goLaunch == 7) {
				DebugPrint("GoLaunch == GO_READY\n");
				g_goLaunch |= 0x80000000;
				DebugPrint("WinMain(2): pause_timer(false)\n");
				PauseTimer(0x80, FALSE);
				StartMissionMusic();
				g_statusMessage = 0;
			}
			else if (g_isNetworkGame && !(g_goLaunch & 0x80000000)) {
				if (g_remoteWaitTime == 0) {
					g_remoteWaitTime = g_realClock + 0xb5;
				}
				else if (g_remoteWaitTime < g_realClock) {
					g_statusMessage = 3;
				}
			}

			HandleMessages();
			UpdateNetwork();
			NextClock();
			msiSync();
			UpdateInputs();
			UpdateMenuKey();
			HandleGameKeys(0, 0, 0);
			RunTimedCallbacks(&g_detachedTasks);
			UpdateAllPlayers();

			UpdateEyepoint();
			UpdateAllShots();
			UpdateEffects();
			UpdateLocalPlayer();
			LateUpdateAllPlayers();
			UpdateDebris();
			FUN_1005f820(1, 0);

			if (g_goLaunch & 0x80000000) {
				UpdateGeoCache();
			}

			AdvanceAnimations();
			UpdatePaletteFade();
			ApplyPendingPalette();
			if (g_windowActive && g_currentDisplayBackend->m_id == 0 && !g_renderSettings.m_texturedGround) {
				FUN_1005f820(1, PIXEL_COLOR(g_groundColor));
			}

			if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
				MsiClearFrame();
				g_renderSettings.m_frameDrawCallback();
				DrawLocalPlayer();
				UpdateMenus();
				DrawTimedOverlays();
				DrawDebugOverlays();
				if (g_simPaused && g_pauseRequested) {
					DrawPausedBanner();
				}
				MsiSwapBuffers();
			}

			Blit();
			if (g_goLaunch & 0x80000000) {
				DoAudio();
				AdvanceSpeechQueue();
			}

			UpdateTimeOfDay();
			UpdateObjectives();
			LoopCdMusic();
			UpdatePauseState();
			if (g_shouldQuit && !quitLatched) {
				g_quitStage++;
				quitLatched = 1;
				g_drawModeReady = 0;
			}

			g_goLaunch |= 6;
		}

		FadeToEndPalette(g_careerRecord.m_outcome & 4);
		if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
			VFX_pane_wipe(&g_currentPane, PIXEL_COLOR(0));
		}

		Blit();
		g_unk0x100be544 = 0;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		}

		SendMessage(g_unk0x100be508, 0x41e, 0, 0);
		SaveCareerRecord();
		ShutdownAllPlayers();
		ShutdownNetwork();
		ShutdownAudio();
		FreeMenus();
		StopTimers();
		ShutdownMw2Prj();
		ShutdownRender();
		g_gameWindow = g_unk0x100be508;
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
			SendMessage(g_unk0x100be508, 0x41e, 0, 0);
			MessageBox(NULL, "Attempting to shutdown from an unknown fatal error.", "MECHWARRIOR 2", MB_ICONHAND);
			AIL_shutdown();
		}
	}

	HeapDestroy(g_primaryHeap);
	g_primaryHeap = NULL;
	while (ShowCursor(TRUE) < 1) {
	}

	result = g_fledToWindows ? 0xff : 0;
	return result;
}
#elif defined(MW2_POWERVR)
// The PowerVR edition's SimMain, which the 3Dfx edition's kept six weeks later but for the renderer
// (the window, the sky and ground settings read after the difficulty, the DC released after
// StartSupAnim, the bitmaps' average colours, the players updated every frame). SGL renders the frame and calls back
// the 2D drawing (FUN_10007db0, which also reads the input), so the main loop neither acquires the frame buffer nor
// blits: it only calls the frame draw callback while the window is active, after turning the scene's shapes of kind
// 0x20 (FUN_10028ac0), and it has no refresh mode fallback loop. After the world is loaded it applies the time of day's
// palette and opens the renderer (FUN_10060f00, FUN_10061530) before reading its registry settings (FUN_1008fad0, which
// it calls again after InitRefreshMode), and the mission ends without a last wipe and blit.
// FUNCTION: MW2POWERVR 0x10007e60
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
	int result;
	MechS32 i;
	char missionName[64];
	undefined4 flags[2];
	MechS32 seed;
	MechS32 quitLatched;
	HDC supAnimDc;

	quitLatched = 0;
	seed = 0;
	g_gameWindow = p_hWnd;
	g_simModule = p_module;
	g_desktopWidth = GetSystemMetrics(SM_CXSCREEN);
	g_desktopHeight = GetSystemMetrics(SM_CYSCREEN);
	g_primaryHeap = HeapCreate(HEAP_NO_SERIALIZE, 1000000, 0);
	if (g_primaryHeap == NULL) {
		Error(9, "Insufficient memory available.");
	}

	flags[1] = 1;
	flags[0] = 0;
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

	g_displayBrightness = g_brightnessSetting = g_mw2SndCfgData->m_displayBrightness;
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

	if (!ProcessCmdLineArgs(p_cmdLine, flags, missionName)) {
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

	LoadSkyGroundSettings(missionName);
	DebugPrint("FirstClock()\n");
	__try {
		FirstClock();
		DebugPrint("StartSupAnim()\n");
		supAnimDc = GetDC(g_gameWindow);
		StartSupAnim(GetDeviceCaps(supAnimDc, NUMCOLORS) == -1 || g_windowMode == c_windowModeFullscreen);
		ReleaseDC(g_gameWindow, supAnimDc);
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
		CalcAverageBitmapColors();
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
		ApplyPaletteResource(FUN_10006f30());
		if (!FUN_10060f00()) {
			Error(0x50, NULL);
		}

		FUN_10061530();
		FUN_1008fad0();
		StopSupAnim();
		g_drawModeReady = 1;
		DebugPrint("InitDrawMode(%d,%d)\n", g_gameWindowWidth, g_gameWindowHeight);
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

		DebugPrint("done InitDrawMode\n");
		FUN_1008fad0();
		while (ShowCursor(FALSE) >= 0) {
		}

		g_mouseOutsideClientWindow = 0;
		if (!g_refreshModeFallback) {
			g_goLaunch |= 2;
		}

		g_careerRecord.m_outcome = 1;
		g_careerRecord.m_winner = -1;
		while (g_quitStage < 3) {
			if (g_goLaunch == 7) {
				DebugPrint("GoLaunch == GO_READY\n");
				g_goLaunch |= 0x80000000;
				DebugPrint("WinMain(2): pause_timer(false)");
				PauseTimer(0x80, FALSE);
				StartMissionMusic();
				g_statusMessage = 0;
			}
			else if (g_isNetworkGame && !(g_goLaunch & 0x80000000)) {
				if (g_remoteWaitTime == 0) {
					g_remoteWaitTime = g_realClock + 0xb5;
				}
				else if (g_remoteWaitTime < g_realClock) {
					g_statusMessage = 3;
				}
			}

			HandleMessages();
			UpdateNetwork();
			NextClock();
			RunTimedCallbacks(&g_detachedTasks);
			UpdateAllPlayers();

			UpdateEyepoint();
			UpdateAllShots();
			UpdateEffects();
			UpdateLocalPlayer();
			LateUpdateAllPlayers();
			UpdateDebris();
			if (g_goLaunch & 0x80000000) {
				UpdateGeoCache();
			}

			AdvanceAnimations();
			UpdatePaletteFade();
			ApplyPendingPalette();
			FUN_10028ac0();
			if (g_windowActive) {
				g_renderSettings.m_frameDrawCallback();
			}

			if (g_goLaunch & 0x80000000) {
				DoAudio();
				AdvanceSpeechQueue();
			}

			UpdateTimeOfDay();
			UpdateObjectives();
			LoopCdMusic();
			UpdatePauseState();
			if (g_shouldQuit && !quitLatched) {
				g_quitStage++;
				quitLatched = 1;
				g_drawModeReady = 0;
			}

			g_goLaunch |= 6;
		}

		FadeToEndPalette(g_careerRecord.m_outcome & 4);
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		}

		SendMessage(g_gameWindow, 0x41e, 0, 0);
		SaveCareerRecord();
		ShutdownAllPlayers();
		ShutdownNetwork();
		ShutdownAudio();
		FreeMenus();
		StopTimers();
		ShutdownMw2Prj();
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

	result = g_fledToWindows ? 0xff : 0;
	return result;
}
#elif defined(MW2_3DFX) || defined(MW2_ATI) || defined(MW2_S3)
// The fill of the refresh mode fallback, the edition's display back end's.
#ifdef MW2_3DFX
#define SIM_BACKEND_FILL GlideFill
#else
#define SIM_BACKEND_FILL DdrawFill
#endif
#ifdef MW2_LIT_GROUND_FILL
// The S3 edition fills with the ground's colour lit by its renderer's light, an RGB555 pixel.
#define SIM_RGB555(r, g, b) ((((MechU8) (r) & ~7) << 5 | ((MechU8) (g) & ~7)) << 2 | (MechU8) (b) >> 3)

// polydraw.c's, which declares them nowhere else.
extern MechFloat g_groundBlue;
extern MechFloat g_groundGreen;
extern MechFloat g_groundRed;
#endif

// The ATI and 3Dfx editions' SimMain (the ATI edition's is six months older). Both have the window
// 1.1 has and ask SetGameResolution (the 3Dfx edition 800x600 with its registry setting); like the
// Matrox edition's, they compute the bitmaps' average colours after loading the world and update
// the players every frame before a network launch, which starts when the go state is 7; and they
// have the refresh mode fallback loop 1.1 has, filling through the edition's display back end
// (DirectDraw, Glide). The ATI edition reads the sky and ground and the render settings right after
// the command line, tests the palette through DCs it doesn't release, as 1.1 does, and shuts its
// renderer down (A3D_shutdown) after the last blit; the S3 edition releases the first DC; the 3Dfx
// edition reads the render settings before FirstClock (the Matrox edition before the main loop) and
// releases both.
// FUNCTION: MW23DFX 0x10008850
// FUNCTION: MW2ATI 0x10009040
// FUNCTION: MW2S3 0x100266f0
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
	undefined4 flags[2];
	MechS32 seed;
	MechS32 quitLatched;
#ifdef MW2_SUPANIM_DC_RELEASED
	HDC supAnimDc;
#endif
#ifdef MW2_PALETTE_DC_RELEASED
	HDC paletteDc;
#endif

	quitLatched = 0;
	seed = 0;
	g_gameWindow = p_hWnd;
	g_simModule = p_module;
	g_desktopWidth = GetSystemMetrics(SM_CXSCREEN);
	g_desktopHeight = GetSystemMetrics(SM_CYSCREEN);
	g_primaryHeap = HeapCreate(HEAP_NO_SERIALIZE, 1000000, 0);
	if (g_primaryHeap == NULL) {
		Error(9, "Insufficient memory available.");
	}

	flags[1] = 1;
	flags[0] = 0;
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

	g_displayBrightness = g_brightnessSetting = g_mw2SndCfgData->m_displayBrightness;
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

	if (!ProcessCmdLineArgs(p_cmdLine, flags, missionName)) {
		return 0;
	}

#ifdef MW2_ATI
	LoadSkyGroundSettings(missionName);
	FUN_1008fad0();
#endif
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

#ifdef MW2_SKY_SETTINGS_BEFORE_CLOCK
	LoadSkyGroundSettings(missionName);
#endif
#ifdef MW2_RENDER_SETTINGS_BEFORE_CLOCK
	FUN_1008fad0();
#endif
	DebugPrint("FirstClock()\n");
	__try {
		FirstClock();
		DebugPrint("StartSupAnim()\n");
#ifdef MW2_SUPANIM_DC_RELEASED
		supAnimDc = GetDC(g_gameWindow);
		StartSupAnim(GetDeviceCaps(supAnimDc, NUMCOLORS) == -1 || g_windowMode == c_windowModeFullscreen);
		ReleaseDC(g_gameWindow, supAnimDc);
#else
		StartSupAnim(GetDeviceCaps(GetDC(g_gameWindow), NUMCOLORS) == -1 || g_windowMode == c_windowModeFullscreen);
#endif
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
		CalcAverageBitmapColors();
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
#ifdef MW2_PALETTE_DC_RELEASED
		paletteDc = GetDC(g_gameWindow);
		if (GetDeviceCaps(paletteDc, NUMCOLORS) == -1 || g_windowMode == c_windowModeFullscreen) {
#else
		if (GetDeviceCaps(GetDC(g_gameWindow), NUMCOLORS) == -1 || g_windowMode == c_windowModeFullscreen) {
#endif
			DebugPrint("StartPalettes()\n");
			StartPalettes(0);
		}
		else {
			hasPalette = g_paletteResourceIds[0x10] != -1;
			if (hasPalette) {
				palette = LoadCachedResource(g_mw2PrjHandle, hasPalette, g_resTagPal, 0);
				if (palette) {
					g_currentDisplayBackend->m_setPalette(0, 0x100, palette, 1);
				}
			}
		}

#ifdef MW2_PALETTE_DC_RELEASED
		ReleaseDC(g_gameWindow, paletteDc);
#endif
		g_drawModeReady = 1;
		DebugPrint("InitDrawMode(%d,%d)\n", g_gameWindowWidth, g_gameWindowHeight);
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

#ifdef MW2_RENDER_SETTINGS_AFTER_MODES
		FUN_1008fad0();
#endif
		while (ShowCursor(FALSE) >= 0) {
		}

		g_mouseOutsideClientWindow = 0;
		if (!g_refreshModeFallback) {
			g_goLaunch |= 2;
		}

		g_careerRecord.m_outcome = 1;
		g_careerRecord.m_winner = -1;
		while (g_quitStage < 3) {
			if (g_goLaunch == 7) {
				DebugPrint("GoLaunch == GO_READY\n");
				g_goLaunch |= 0x80000000;
				DebugPrint("WinMain(2): pause_timer(false)");
				PauseTimer(0x80, FALSE);
				StartMissionMusic();
				g_statusMessage = 0;
			}
			else if (g_isNetworkGame && !(g_goLaunch & 0x80000000)) {
				if (g_remoteWaitTime == 0) {
					g_remoteWaitTime = g_realClock + 0xb5;
				}
				else if (g_remoteWaitTime < g_realClock) {
					g_statusMessage = 3;
				}
			}

			HandleMessages();
			UpdateNetwork();
			NextClock();
			UpdateInputs();
			UpdateMenuKey();
			HandleGameKeys(0, 0, 0);
			RunTimedCallbacks(&g_detachedTasks);
			UpdateAllPlayers();

			UpdateEyepoint();
			UpdateAllShots();
			UpdateEffects();
			UpdateLocalPlayer();
			LateUpdateAllPlayers();
			UpdateDebris();
			if (g_refreshModeFallback) {
				while (g_refreshModeFallback) {
					if (g_currentDisplayBackend->m_id == 0 && !g_renderSettings.m_texturedGround) {
						SIM_BACKEND_FILL(0, 0, g_gameWindowWidth, g_gameWindowHeight, PIXEL_COLOR(g_groundColor));
					}

					if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
						g_renderSettings.m_frameDrawCallback();
						DrawLocalPlayer();
						UpdateMenus();
						DrawTimedOverlays();
						DrawDebugOverlays();
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
#ifdef MW2_LIT_GROUND_FILL
			if ((g_goLaunch & 4) && g_windowActive && g_currentDisplayBackend->m_id == 0) {
				SIM_BACKEND_FILL(
					0,
					0,
					g_gameWindowWidth,
					g_gameWindowHeight,
					SIM_RGB555(
						g_unk0x10184700[2] * g_groundRed / 255,
						g_unk0x10184700[1] * g_groundGreen / 255,
						g_unk0x10184700[0] * g_groundBlue / 255
					)
				);
			}
#else
			if (g_windowActive && g_currentDisplayBackend->m_id == 0 && !g_renderSettings.m_texturedGround) {
				SIM_BACKEND_FILL(0, 0, g_gameWindowWidth, g_gameWindowHeight, PIXEL_COLOR(g_groundColor));
			}
#endif

			if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
				g_renderSettings.m_frameDrawCallback();
				DrawLocalPlayer();
				UpdateMenus();
				DrawTimedOverlays();
				DrawDebugOverlays();
				if (g_simPaused && g_pauseRequested) {
					DrawPausedBanner();
				}
			}

			Blit();
			if (g_goLaunch & 0x80000000) {
				DoAudio();
				AdvanceSpeechQueue();
			}

			UpdateTimeOfDay();
			UpdateObjectives();
			LoopCdMusic();
			UpdatePauseState();
			if (g_shouldQuit && !quitLatched) {
				g_quitStage++;
				quitLatched = 1;
				g_drawModeReady = 0;
			}

			g_goLaunch |= 6;
		}

		FadeToEndPalette(g_careerRecord.m_outcome & 4);
		if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
			VFX_pane_wipe(&g_currentPane, PIXEL_COLOR(0));
		}

		DebugPrint("Calling Blit()\n");
		Blit();
#if defined(MW2_3DFX)
		FUN_10066910();
#elif !defined(MW2_NO_SIMMAIN_A3D_SHUTDOWN)
		A3D_shutdown();
#endif
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		}

		SendMessage(g_gameWindow, 0x41e, 0, 0);
		SaveCareerRecord();
		ShutdownAllPlayers();
		ShutdownNetwork();
		ShutdownAudio();
		FreeMenus();
		StopTimers();
		ShutdownMw2Prj();
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

	result = g_fledToWindows ? 0xff : 0;
	return result;
}
#else
// 1.1's SimMain. In a network game it doesn't update the players until the launch, which
// starts when the go state is 3: the network's bit (1) and the bit each main-loop pass sets (2).
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
	undefined4 flags[2];
	MechS32 seed;
	MechS32 quitLatched;

	quitLatched = 0;
	seed = 0;
	g_gameWindow = p_hWnd;
	g_simModule = p_module;
	g_desktopWidth = GetSystemMetrics(SM_CXSCREEN);
	g_desktopHeight = GetSystemMetrics(SM_CYSCREEN);
	g_primaryHeap = HeapCreate(HEAP_NO_SERIALIZE, 1000000, 0);
	if (g_primaryHeap == NULL) {
		Error(9, "Insufficient memory available.");
	}

	flags[1] = 1;
	flags[0] = 0;
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

	g_displayBrightness = g_brightnessSetting = g_mw2SndCfgData->m_displayBrightness;
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

	if (!ProcessCmdLineArgs(p_cmdLine, flags, missionName)) {
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
				palette = LoadCachedResource(g_mw2PrjHandle, hasPalette, g_resTagPal, 0);
				if (palette) {
					g_currentDisplayBackend->m_setPalette(0, 0x100, palette, 1);
				}
			}
		}

		g_drawModeReady = 1;
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

		g_careerRecord.m_outcome = 1;
		g_careerRecord.m_winner = -1;
		while (g_quitStage < 3) {
			if (g_goLaunch == 3) {
				DebugPrint("GoLaunch == GO_READY\n");
				g_goLaunch |= 0x80000000;
				DebugPrint("WinMain(2): pause_timer(false)");
				PauseTimer(0x80, FALSE);
				StartMissionMusic();
				g_statusMessage = 0;
			}
			else if (g_isNetworkGame && !(g_goLaunch & 0x80000000)) {
				if (g_remoteWaitTime == 0) {
					g_remoteWaitTime = g_realClock + 0xb5;
				}
				else if (g_remoteWaitTime < g_realClock) {
					g_statusMessage = 3;
				}
			}

			HandleMessages();
			UpdateNetwork();
			NextClock();
			UpdateInputs();
			UpdateMenuKey();
			HandleGameKeys(0, 0, 0);
			RunTimedCallbacks(&g_detachedTasks);
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
						DdrawFill(0, 0, g_gameWindowWidth, g_gameWindowHeight, g_groundColor);
					}

					if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
						g_renderSettings.m_frameDrawCallback();
						DrawLocalPlayer();
						UpdateMenus();
						DrawTimedOverlays();
						DrawDebugOverlays();
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
				DdrawFill(0, 0, g_gameWindowWidth, g_gameWindowHeight, g_groundColor);
			}

			if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
				g_renderSettings.m_frameDrawCallback();
				DrawLocalPlayer();
				UpdateMenus();
				DrawTimedOverlays();
				DrawDebugOverlays();
				if (g_simPaused && g_pauseRequested) {
					DrawPausedBanner();
				}
			}

			Blit();
			if (g_goLaunch & 0x80000000) {
				DoAudio();
				AdvanceSpeechQueue();
			}

			UpdateTimeOfDay();
			UpdateObjectives();
			LoopCdMusic();
			UpdatePauseState();
			if (g_shouldQuit && !quitLatched) {
				g_quitStage++;
				quitLatched = 1;
				g_drawModeReady = 0;
			}

			g_goLaunch |= 2;
		}

		FadeToEndPalette(g_careerRecord.m_outcome & 4);
		if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
			VFX_pane_wipe(&g_currentPane, PIXEL_COLOR(0));
		}

		DebugPrint("Calling Blit()\n");
		Blit();
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		}

		SendMessage(g_gameWindow, 0x41e, 0, 0);
		SaveCareerRecord();
		ShutdownAllPlayers();
		ShutdownNetwork();
		ShutdownAudio();
		FreeMenus();
		StopTimers();
		ShutdownMw2Prj();
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

	result = g_fledToWindows ? 0xff : 0;
	return result;
}
#endif

#ifdef MW2_MATROX
// The Matrox edition's window procedure, which the MSI95 back end's windows call too. Once the
// renderer has its window, that one takes the messages and the others only their activation: the
// launcher's window restores or minimizes and resets the palette, the renderer's records the
// activation, clears the keys and takes the game window back.
// FUNCTION: MW2MATROX 0x100908a4
LRESULT CALLBACK SimWindowProc(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	WINDOWPOS* windowPos;

	// The renderer's window takes the messages, and the other windows only their activation.
	if (g_msiWindow && p_hWnd != g_msiWindow && p_msg != WM_ACTIVATEAPP) {
		return DefWindowProc(p_hWnd, p_msg, p_wParam, p_lParam);
	}

	if (p_msg >= WM_KEYFIRST && p_msg <= WM_KEYLAST) {
		HandleKeyboardMessages(p_msg, p_wParam, p_lParam);
		return 0;
	}

	switch (p_msg) {
	case WM_ACTIVATEAPP:
		if (g_shouldQuit) {
			break;
		}

		if (p_hWnd == g_unk0x100be508 && !g_unk0x100be544) {
			g_windowActive = p_wParam;
			if (g_windowActive) {
				ShowWindow(p_hWnd, SW_RESTORE);
			}
			else {
				ShowWindow(p_hWnd, SW_MINIMIZE);
			}

			if (g_currentDisplayBackend) {
				g_currentDisplayBackend->m_setPalette(0, 0x100, g_paletteColors, 1);
			}
			return 0;
		}

		if (p_hWnd == g_msiWindow) {
			if ((g_windowActive && p_wParam) || (!g_windowActive && !p_wParam)) {
				return 0;
			}

			g_windowActive = p_wParam;
			if (p_wParam) {
				KeyboardClearKeyStates();
				FUN_1005d090();
				if (g_msiWindow) {
					g_gameWindow = g_msiWindow;
				}
			}
			else {
			}

			return 0;
		}

		return 0;
	case WM_PAINT:
		if (g_drawModeReady && g_windowMode == c_windowModeWindowed) {
			g_currentRefreshMode->m_flip();
			ValidateRect(p_hWnd, NULL);
			return 0;
		}
		break;
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
		break;
	case WM_QUERYNEWPALETTE:
		if (g_currentDisplayBackend) {
			g_currentDisplayBackend->m_setPalette(0, 0x100, g_paletteColors, 1);
			return 1;
		}
		return 0;
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
#elif defined(MW2_3DFX) || defined(MW2_ATI) || defined(MW2_S3) || defined(MW2_POWERVR)
// The ATI and 3Dfx editions' window procedure. On WM_ACTIVATEAPP it has none of 1.1's switching
// between full screen and the desktop: it only restores or minimizes a full-screen window and
// resets the palette.
// FUNCTION: MW23DFX 0x10009400
// FUNCTION: MW2ATI 0x10009bd0
// FUNCTION: MW2S3 0x100272f0
// FUNCTION: MW2POWERVR 0x10008830
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
			ShowWindow(g_gameWindow, SW_RESTORE);
		}
		else {
			ShowWindow(g_gameWindow, SW_MINIMIZE);
		}

		if (g_currentDisplayBackend) {
			g_currentDisplayBackend->m_setPalette(0, 0x100, g_paletteColors, 1);
		}
		return 0;
	case WM_PAINT:
		if (g_drawModeReady && g_windowMode == c_windowModeWindowed) {
			g_currentRefreshMode->m_flip();
			ValidateRect(p_hWnd, NULL);
			return 0;
		}
		break;
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
		break;
	case WM_QUERYNEWPALETTE:
		if (g_currentDisplayBackend) {
			g_currentDisplayBackend->m_setPalette(0, 0x100, g_paletteColors, 1);
			return 1;
		}
		return 0;
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
#else
// 1.1's window procedure. On WM_ACTIVATEAPP, on a desktop of 640x480 or less, it leaves full
// screen when the game is deactivated and returns to it when activated again.
// FUNCTION: MW2 0x10067757
// FUNCTION: MW2GBL 0x1002e136
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
		if (g_drawModeReady && g_windowMode == c_windowModeWindowed) {
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
#endif

// The Matrox edition returns when the mouse message loop's PeekMessage finds nothing.
// FUNCTION: MW2 0x10067bbc
// FUNCTION: MW2GBL 0x1002e59b
// FUNCTION: MW2MATROX 0x10090ca7
// FUNCTION: MW23DFX 0x10009680
// FUNCTION: MW2ATI 0x10009e50
// FUNCTION: MW2S3 0x10027570
// FUNCTION: MW2POWERVR 0x10008ab0
void HandleMessages(void)
{
	MSG msg;

	if (!g_windowActive) {
		WaitMessage();
	}

	if (!g_shouldQuit && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		while (!g_mouseOutsideClientWindow && msg.message >= WM_MOUSEFIRST && msg.message <= WM_MBUTTONDBLCLK) {
#if defined(MW2_MOUSE_PEEK_RETURNS)
			if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				return;
			}
#elif defined(MW2_MOUSE_PEEK_BREAKS)
			if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				break;
			}
#else
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
#endif
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
// FUNCTION: MW2GBL 0x1002e658
// FUNCTION: MW2MATROX 0x10090d71
// FUNCTION: MW23DFX 0x10009740
// FUNCTION: MW2ATI 0x10009f10
// FUNCTION: MW2S3 0x10027630
// FUNCTION: MW2POWERVR 0x10008b70
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
#ifdef MW2_DEBUG_LOG_TEXT
			DebugPrint("WinMain(3): pause_timer(false)\n");
#else
			DebugPrint("WinMain(3): pause_timer(false)");
#endif
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

#ifdef MW2_DEBUG_LOG_TEXT
		DebugPrint("WinMain(4): pause_timer(TRUE)\n");
#else
		DebugPrint("WinMain(4): pause_timer(TRUE)");
#endif
		PauseTimer(0x80, TRUE);
		PauseAudio();
		DisableGameplayInput();
		g_simPaused = TRUE;
	}
}

#ifdef MW2_RES_800X600
// The 3Dfx edition's: 800x600 with the registry's "Res800x600" setting, 640x480 otherwise.
// FUNCTION: MW23DFX 0x10009890
void SetGameResolution(char* p_driverName)
{
	DWORD res800x600;

	if (!ReadRegistryDword("Res800x600", &res800x600)) {
		res800x600 = 0;
	}

	FUN_10066920(res800x600);

	if (g_a3dRes800x600) {
		g_gameWindowWidth = 800;
		g_gameWindowHeight = 600;
	}
	else {
		g_gameWindowWidth = 640;
		g_gameWindowHeight = 480;
	}
}
#elif defined(MW2_S3)
// The S3 edition's: the driver name is the size ("640x480"), kept if DirectDraw has it in 16 bits,
// 320x200 otherwise.
// FUNCTION: MW2S3 0x10027780
void SetGameResolution(char* p_driverName)
{
	sscanf(p_driverName, "%dx%d", &g_gameWindowWidth, &g_gameWindowHeight);
	if (!FUN_100059b0(g_gameWindowWidth, g_gameWindowHeight, 16)) {
		g_gameWindowWidth = 320;
		g_gameWindowHeight = 200;
	}
}
#elif defined(MW2_POWERVR)
// The PowerVR edition's game window is always 640x480, whatever the driver.
// FUNCTION: MW2POWERVR 0x10008cc0
void SetGameResolution(char* p_driverName)
{
	g_gameWindowWidth = 640;
	g_gameWindowHeight = 480;
}
#else
// FUNCTION: MW2 0x10067e23
// FUNCTION: MW2GBL 0x1002e802
// FUNCTION: MW2MATROX 0x10090f1b
// FUNCTION: MW2ATI 0x1000a060
void SetGameResolution(char* p_driverName)
{
	if (_strcmpi(p_driverName, "MCGA.DLL") == 0) {
		g_gameWindowWidth = 320;
		g_gameWindowHeight = 200;
	}
	else if (_strcmpi(p_driverName, "VESA480.DLL") == 0) {
#ifdef MW2_RES_512X384
		g_gameWindowWidth = 512;
		g_gameWindowHeight = 384;
		if (!FUN_100059b0(512, 384, 16)) {
			g_gameWindowWidth = 640;
			g_gameWindowHeight = 480;
		}
#else
		g_gameWindowWidth = 640;
		g_gameWindowHeight = 480;
#endif
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
#endif
