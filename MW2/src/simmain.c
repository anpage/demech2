/* Hand-written assembly: the tick counters (GameTickTimerCallback through PauseTimer) are whole
   assembly routines at the end of this object, transcribed as __declspec(naked) functions. Their
   frames save only the registers they use, and their jumps are short (rel8), which the inline
   assembler never emits, so each short jump is an _emit pair. */
#include "simmain.h"

#include "animation.h"
#include "audio.h"
#include "brightness.h"
#include "callbacks.h"
#include "clock.h"
#include "cockpit.h"
#include "compat.h"
#include "config.h"
#include "debugprint.h"
#include "decomp.h"
#include "directdraw.h"
#include "dispdibmode.h"
#include "displaybackend.h"
#include "environment.h"
#include "error.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "gdi.h"
#include "gpanim.h"
#include "input.h"
#include "keyboard.h"
#include "loadres.h"
#include "menu.h"
#include "mss.h"
#include "mw2log.h"
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
#include "speech.h"
#include "startup.h"
#include "staticmem.h"
#include "timedoverlays.h"
#include "types.h"

#include <excpt.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef struct NetLaunchInfo {
	void* m_directPlay;               // 0x00
	MechS32 m_localPlayerId;          // 0x04
	undefined4 m_unk0x08;             // 0x08
	undefined4 m_unk0x0c;             // 0x0c
	MechS32* m_playerIds;             // 0x10
	undefined m_unk0x14[0x24 - 0x14]; // 0x14
	char* m_missionName;              // 0x24
} NetLaunchInfo;

typedef struct Unk0x1012b7c0 {
	MechU32 m_unk0x00;  // 0x00
	char m_unk0x04[13]; // 0x04
} Unk0x1012b7c0;

DECOMP_SIZE_ASSERT(SoundConfig, 0x3c)
DECOMP_SIZE_ASSERT(StarMission, 0x3c0a)
DECOMP_SIZE_ASSERT(MissionObjective, 0x13f)

// The globals SimMain and SimWindowProc use are defined here until the objects that own
// them are decompiled.

// GLOBAL: MW2 0x100a175c
MechS32 g_isNetworkGame = 0;

// GLOBAL: MW2 0x100a17a0
MechS32 g_unk0x100a17a0 = 0;

// GLOBAL: MW2 0x100a1498
SoundConfig g_soundConfig = {0x10000, 0x10000, 0x10000, 0x10000, 11, 1, 1, 1, 1, 1, 9, "mcga.dll"};

// GLOBAL: MW2 0x100a14d4
SoundConfig* g_mw2SndCfgData = NULL;

// GLOBAL: MW2 0x100a2400
MechS32 g_normalFov = 0x10000;

// GLOBAL: MW2 0x100a2404
MechS32 g_zoomFov = 0x10000;

// GLOBAL: MW2 0x100a2414
MechS32 g_unk0x100a2414 = 0;

// GLOBAL: MW2 0x100a2420
undefined4 g_unk0x100a2420 = 0;

// GLOBAL: MW2 0x100a2424
MechS32 g_unk0x100a2424 = -1;

// GLOBAL: MW2 0x100a242c
struct Player* g_localPlayer = NULL;

// GLOBAL: MW2 0x100a244c
MechS32 g_drawModeIndex = -1;

// GLOBAL: MW2 0x100a2450
MechS32 g_initDrawModeParam2 = 1;

// GLOBAL: MW2 0x100a2460
MechS32 g_unk0x100a2460 = 1;

// GLOBAL: MW2 0x100a2464
MechS32 g_unk0x100a2464 = 0;

// GLOBAL: MW2 0x100a2c04
MechS32 g_unk0x100a2c04 = 0;

// GLOBAL: MW2 0x100a554c
undefined4 g_unk0x100a554c = 0xef;

// GLOBAL: MW2 0x100a59e0
MechS32 g_menuRepeatTimer = -1;

// GLOBAL: MW2 0x100a5a24
undefined4 g_unk0x100a5a24 = 1;

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

// GLOBAL: MW2 0x100a5ee8
Point g_unk0x100a5ee8[6] = {{0x73, 0x10}, {8, 0x4a}, {4, 0x28}, {4, 0x4a}, {0, 0}, {0, 0}};

// GLOBAL: MW2 0x100a5f18
undefined4 g_unk0x100a5f18 = 1;

// GLOBAL: MW2 0x100a6be0
Eyepoint g_unk0x100a6be0 = {
	0,
	0,
	0,
	0,
	0,
	0,
	0x10000,
	{1000, 10000, (undefined4) -1000, 0x480001},
	0,
	319,
	0,
	199,
	0x40,
	0x249f0,
	0,
	0,
	0,
	0,
	{0}
};

// GLOBAL: MW2 0x100a6cc0
Eyepoint* g_eyepoint = &g_unk0x100a6be0;

// GLOBAL: MW2 0x100a6cc8
SlateHeron0x68 g_unk0x100a6cc8 =
	{{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0xe0, 0xef, 1, 0, 0, 0, 0, 0x186a0, 0x10000}, 0, 0, NULL, NULL, NULL, {0, 0}};

// GLOBAL: MW2 0x100a6d30
MechS32 g_unk0x100a6d30 = 0x24;

// GLOBAL: MW2 0x100a8678
const char* g_unk0x100a8678 = "CEL";

// GLOBAL: MW2 0x100a8680
const char* g_unk0x100a8680 = "SHP";

// GLOBAL: MW2 0x100a8684
const char* g_unk0x100a8684 = "FONT";

// GLOBAL: MW2 0x100a8694
const char* g_unk0x100a8694 = "PAL";

// GLOBAL: MW2 0x100a8698
const char* g_unk0x100a8698 = "TABL";

// GLOBAL: MW2 0x100a86a0
const char* g_unk0x100a86a0 = "TEXT";

// GLOBAL: MW2 0x100a86bc
const char* g_unk0x100a86bc = g_unk0x100a87c0;

// GLOBAL: MW2 0x100a86c4
const char* g_unk0x100a86c4 = "AIT";

// GLOBAL: MW2 0x100a86cc
const char* g_unk0x100a86cc = "LUMA";

// GLOBAL: MW2 0x100a8740
undefined4 g_unk0x100a8740 = 0xffffffff;

// GLOBAL: MW2 0x100a8744
MechChar* g_unk0x100a8744 = NULL;

// GLOBAL: MW2 0x100a87c0
char g_unk0x100a87c0[] = "BWD";

// GLOBAL: MW2 0x100aa2a4
MechS32 g_unk0x100aa2a4 = 1;

// GLOBAL: MW2 0x100aa2ac
MechS32 g_missionTimerStopped = 0;

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

// GLOBAL: MW2 0x100adf58
RenderTarget g_unk0x100adf58 = {&g_mainPixelBuffer, 13, 10, 80, 60};

// GLOBAL: MW2 0x100b1350
MechS32 g_unk0x100b1350 = 0;

// GLOBAL: MW2 0x100bdff8
RenderTarget g_unk0x100bdff8;

// GLOBAL: MW2 0x100bfd60
MechS32 g_unk0x100bfd60[800];

// GLOBAL: MW2 0x100c09e0
MechS32 g_unk0x100c09e0[800];

// GLOBAL: MW2 0x100e9240
MechU32 g_windowedSwitchTime;

// GLOBAL: MW2 0x100e926d
MechU8 g_unk0x100e926d;

// GLOBAL: MW2 0x100e9322
MechS32 g_unk0x100e9322;

// GLOBAL: MW2 0x100e933c
MechU32 g_windowedSwitchDeadline;

// GLOBAL: MW2 0x100e9350
undefined g_unk0x100e9350[0x100];

// GLOBAL: MW2 0x100e9614
MechS32 g_unk0x100e9614;

// GLOBAL: MW2 0x1012b7c0
Unk0x1012b7c0 g_unk0x1012b7c0;

// GLOBAL: MW2 0x100c3358
MechS32 g_unk0x100c3358;

// GLOBAL: MW2 0x100ea3e4
MechS32 g_unk0x100ea3e4;

// GLOBAL: MW2 0x10138710
MechS32 g_missionTime;

// GLOBAL: MW2 0x10138720
MechS32 g_currentObjective[64]; // by team; length unknown

// GLOBAL: MW2 0x10138830
StarMission g_objectiveTable[16];

// GLOBAL: MW2 0x10176ed0
RenderTarget g_currentRenderTarget;

// GLOBAL: MW2 0x10176ef0
PixelBuffer g_mainPixelBuffer;

// The two tick counters GameTickTimerCallback advances, and the start values of each counter's
// handles (0: free).

// GLOBAL: MW2 0x100ad008
MechU32 g_ticksPaused = 0;

// GLOBAL: MW2 0x100ad00c
MechS32 g_ticks1Bases[64] = {0};

// GLOBAL: MW2 0x100ad10c
MechS32 g_ticks2Bases[64] = {0};

// GLOBAL: MW2 0x100ad20c
MechS32 g_ticks1 = 0;

// GLOBAL: MW2 0x100ad210
MechS32 g_ticks2 = 0;

// GLOBAL: MW2 0x10138820
MechS32 g_objectiveCount; // defined last for the operand order of the DoFirstObjtv loop test

void StartSupAnim(MechS32 p_unk0x00);
void StopSupAnim(void);
void UpdateDebris(void);
void ZeroChunx(void);
void CollectMissionAudio(void);
void LoadWorld(char* p_unk0x00);
void AfterWorldLoader(void);
void FirstNetwork(NetLaunchInfo* p_unk0x00);
MechS32 FirstExternalCtrl(void);
void UpdateNetwork(void);
void ShutdownNetwork(void);
void FirstEyepoint(void);
void UpdateEyepoint(void);
void DoFirstObjtv(StarMission* p_unk0x00, MechS32 p_unk0x04);
void UpdateObjectives(void);
void EndTheMission1(void);
void EndTheMission2(void);
MechS32 ProcessCmdLineArgs(LPSTR p_unk0x00, undefined4* p_unk0x04, char* p_unk0x08);
void UpdateGeoCache(void);
void FirstStaticCache(void);
void FirstAI(void);
void HandleGameKeys(MechS32 p_unk0x00, MechS32 p_unk0x04, MechS32 p_unk0x08);
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
	g_unk0x1012b7c0.m_unk0x00 = 0;
	if (g_mw2SndCfgData->m_videoDriver[0]) {
		g_unk0x1012b7c0.m_unk0x00 |= 1;
		if (_stricmp(g_mw2SndCfgData->m_videoDriver, "scan") == 0) {
			g_unk0x1012b7c0.m_unk0x04[0] = 0;
		}
		else {
			strncpy(g_unk0x1012b7c0.m_unk0x04, g_mw2SndCfgData->m_videoDriver, 12);
			g_unk0x1012b7c0.m_unk0x04[12] = 0;
		}
	}

	if (!ProcessCmdLineArgs(p_cmdLine, &unk0x28, missionName)) {
		return 0;
	}

	if (StartupCheckStub()) {
		Error(0x51, NULL);
	}

	SetGameResolution(g_unk0x1012b7c0.m_unk0x04);
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
			g_unk0x100a17a0 = 1;
		}
		else {
			g_unk0x100a17a0 = 2;
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

		g_unk0x100e926d = 1;
		g_unk0x100e9322 = -1;
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

		FadeToEndPalette(g_unk0x100e926d & 4);
		if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
			FillRenderTargetRect(&g_currentRenderTarget, 0);
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
		if (g_simPaused && g_keyCode) {
			FUN_10009f13();
			g_keyCode = 0;
			g_pauseRequested = FALSE;
		}
		else if (g_unk0x100a17a0 || GetMenuSlotState(4)) {
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
			FUN_10007064();
			EnableGameplayInput();
			g_simPaused = FALSE;
		}
	}
	else if (!g_simPaused && !g_unk0x100a17a0) {
		if (g_pauseRequested) {
			FUN_10009ef1();
			g_keyCode = 0;
		}

		if (!GetMenuSlotState(4) && g_windowMode != c_windowModeFullscreen) {
			while (ShowCursor(TRUE) < 0) {
			}

			g_mouseOutsideClientWindow = TRUE;
		}

		DebugPrint("WinMain(4): pause_timer(TRUE)");
		PauseTimer(0x80, TRUE);
		FUN_10007040();
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

#pragma warning(disable : 4102) /* the labels mark the targets of the _emit short jumps */
#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// The Miles timer that FirstClock registers (181 Hz) calls this. Each counter only runs while
// its bit in g_ticksPaused is clear (PauseTimer).
#ifdef COMPAT_MODE
void GameTickTimerCallback(void)
{
	STUB(0x10067ed8);
}
#else
// FUNCTION: MW2 0x10067ed8
__declspec(naked) void GameTickTimerCallback(void)
{
	__asm {
		push ds
		pushad
		test g_ticksPaused, 0x200
		_emit 0x75 /* jne jmp_10067eec */
		_emit 0x06
		inc g_ticks1
jmp_10067eec:
		test g_ticksPaused, 0x100
		_emit 0x75 /* jne jmp_10067efe */
		_emit 0x06
		inc g_ticks2
jmp_10067efe:
		popad
		pop ds
		ret
	}
}
#endif

// Takes a free slot (0) of the counter selected by bit 0x80 and starts it at the counter's
// current value. Returns the handle (the slot, with 0x80 for the first counter).
#ifdef COMPAT_MODE
MechS16 AllocTicks(MechU32 p_flags)
{
	STUB(0x10067f01);
	return 0;
}
#else
// FUNCTION: MW2 0x10067f01
__declspec(naked) MechS16 AllocTicks(MechU32 p_flags)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push ecx
		push edx
		mov eax, dword ptr [ebp+8]
		test ax, 0x80
		_emit 0x75 /* jne jmp_10067f38 */
		_emit 0x28
		lea ebx, g_ticks2Bases
		xor ecx, ecx
jmp_10067f18:
		cmp dword ptr [ebx], 0
		_emit 0x74 /* je jmp_10067f28 */
		_emit 0x0b
		cmp dword ptr [ebx], -1
		_emit 0x74 /* je jmp_10067f64 */
		_emit 0x42
		inc ecx
		add ebx, 4
		_emit 0xeb /* jmp jmp_10067f18 */
		_emit 0xf0
jmp_10067f28:
		mov eax, g_ticks2
		or eax, eax
		_emit 0x75 /* jne jmp_10067f32 */
		_emit 0x01
		inc eax
jmp_10067f32:
		mov dword ptr [ebx], eax
		mov eax, ecx
		_emit 0xeb /* jmp jmp_10067f68 */
		_emit 0x30
jmp_10067f38:
		lea ebx, g_ticks1Bases
		xor ecx, ecx
jmp_10067f40:
		cmp dword ptr [ebx], 0
		_emit 0x74 /* je jmp_10067f50 */
		_emit 0x0b
		cmp dword ptr [ebx], -1
		_emit 0x74 /* je jmp_10067f64 */
		_emit 0x1a
		inc ecx
		add ebx, 4
		_emit 0xeb /* jmp jmp_10067f40 */
		_emit 0xf0
jmp_10067f50:
		mov eax, g_ticks1
		or eax, eax
		_emit 0x75 /* jne jmp_10067f5a */
		_emit 0x01
		inc eax
jmp_10067f5a:
		mov dword ptr [ebx], eax
		mov eax, ecx
		or ax, 0x80
		_emit 0xeb /* jmp jmp_10067f68 */
		_emit 0x04
jmp_10067f64:
		mov ax, 0xffff
jmp_10067f68:
		pop edx
		pop ecx
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}
#endif

#ifdef COMPAT_MODE
MechS32 GetTicks(MechU32 p_handle)
{
	STUB(0x10067f6f);
	return 0;
}
#else
// FUNCTION: MW2 0x10067f6f
__declspec(naked) MechS32 GetTicks(MechU32 p_handle)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push ecx
		xor ecx, ecx
		mov ecx, dword ptr [ebp+8]
		test cx, 0x80
		_emit 0x75 /* jne jmp_10067f8d */
		_emit 0x0d
		lea ebx, g_ticks2Bases
		mov eax, g_ticks2
		_emit 0xeb /* jmp jmp_10067f9d */
		_emit 0x10
jmp_10067f8d:
		xor cx, 0x80
		lea ebx, g_ticks1Bases
		mov eax, g_ticks1
jmp_10067f9d:
		shl ecx, 2
		add ebx, ecx
		sub eax, dword ptr [ebx]
		pop ecx
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void ResetTicks(MechU32 p_handle)
{
	STUB(0x10067faa);
}
#else
// FUNCTION: MW2 0x10067faa
__declspec(naked) void ResetTicks(MechU32 p_handle)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push ecx
		xor ecx, ecx
		mov ecx, dword ptr [ebp+8]
		test cx, 0x80
		_emit 0x75 /* jne jmp_10067fc8 */
		_emit 0x0d
		lea ebx, g_ticks2Bases
		mov eax, g_ticks2
		_emit 0xeb /* jmp jmp_10067fd8 */
		_emit 0x10
jmp_10067fc8:
		xor cx, 0x80
		lea ebx, g_ticks1Bases
		mov eax, g_ticks1
jmp_10067fd8:
		shl ecx, 2
		add ebx, ecx
		mov dword ptr [ebx], eax
		pop ebx
		pop ecx
		mov esp, ebp
		pop ebp
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void SetTicks(MechU32 p_handle, MechS32 p_ticks)
{
	STUB(0x10067fe5);
}
#else
// FUNCTION: MW2 0x10067fe5
__declspec(naked) void SetTicks(MechU32 p_handle, MechS32 p_ticks)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push ecx
		xor ecx, ecx
		mov ecx, dword ptr [ebp+8]
		test cx, 0x80
		_emit 0x75 /* jne jmp_10068003 */
		_emit 0x0d
		lea ebx, g_ticks2Bases
		mov eax, g_ticks2
		_emit 0xeb /* jmp jmp_10068013 */
		_emit 0x10
jmp_10068003:
		xor cx, 0x80
		mov eax, g_ticks1
		lea ebx, g_ticks1Bases
jmp_10068013:
		shl ecx, 2
		add ebx, ecx
		sub eax, dword ptr [ebp+0xc]
		mov dword ptr [ebx], eax
		pop ecx
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void FreeTicks(MechU32 p_handle)
{
	STUB(0x10068023);
}
#else
// FUNCTION: MW2 0x10068023
__declspec(naked) void FreeTicks(MechU32 p_handle)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push ecx
		xor ecx, ecx
		mov ecx, dword ptr [ebp+8]
		test cx, 0x80
		_emit 0x75 /* jne jmp_1006803c */
		_emit 0x08
		lea ebx, g_ticks2Bases
		_emit 0xeb /* jmp jmp_10068047 */
		_emit 0x0b
jmp_1006803c:
		xor cx, 0x80
		lea ebx, g_ticks1Bases
jmp_10068047:
		shl ecx, 2
		add ebx, ecx
		mov dword ptr [ebx], 0
		pop ecx
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}
#endif

// Stops (p_paused) or restarts the counters selected by p_flags: 0x80 the first, 0x100 the
// second.
#ifdef COMPAT_MODE
void PauseTimer(MechS32 p_flags, MechS32 p_paused)
{
	STUB(0x10068058);
}
#else
// FUNCTION: MW2 0x10068058
__declspec(naked) void PauseTimer(MechS32 p_flags, MechS32 p_paused)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		xor eax, eax
		test word ptr [ebp+8], 0x80
		_emit 0x74 /* je jmp_1006806b */
		_emit 0x05
		or eax, 0x200
jmp_1006806b:
		test word ptr [ebp+8], 0x100
		_emit 0x74 /* je jmp_10068078 */
		_emit 0x05
		or eax, 0x100
jmp_10068078:
		mov bx, word ptr [ebp+0xc]
		or bx, bx
		_emit 0x74 /* je jmp_10068083 */
		_emit 0x02
		_emit 0xeb /* jmp jmp_1006808d */
		_emit 0x0a
jmp_10068083:
		not eax
		and g_ticksPaused, eax
		_emit 0xeb /* jmp jmp_10068093 */
		_emit 0x06
jmp_1006808d:
		or g_ticksPaused, eax
jmp_10068093:
		pop ebx
		mov esp, ebp
		pop ebp
		ret
	}
}
#endif
