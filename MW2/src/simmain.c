#include "decomp.h"
#include "mss.h"
#include "types.h"

#include <excpt.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef enum {
	c_windowModeUnknown = 0,
	c_windowModeFullscreen = 1,
	c_windowModeWindowed = 2
} WindowMode;

typedef struct {
	MechS32 m_index;                                                                                         // 0x00
	WindowMode m_windowMode;                                                                                 // 0x04
	MechU32 m_gwlStyle;                                                                                      // 0x08
	void* m_drawModeBegin;                                                                                   // 0x0c
	void* m_drawModeEnd;                                                                                     // 0x10
	void (*m_setPalette)(undefined4 p_unk0x00, undefined4 p_unk0x04, void* p_unk0x08, undefined4 p_unk0x0c); // 0x14
	void* m_setPaletteWithBrightness;                                                                        // 0x18
	void* m_paletteFade;                                                                                     // 0x1c
	MechS32 (*m_lockBuffer)(void);                                                                           // 0x20
	undefined4 m_unk0x24;                                                                                    // 0x24
} DrawModeExtension;

typedef struct {
	MechU32 m_index;          // 0x00
	MechS32 m_extensionIndex; // 0x04
	MechS32 m_initialized;    // 0x08
	MechU32 m_profileTime;    // 0x0c
	void* m_begin;            // 0x10
	void* m_end;              // 0x14
	void (*m_blitFlip)(void); // 0x18
} DrawMode;

// SIZE 0x3c
typedef struct Unk0x100a14d4 {
	undefined4 m_unk0x00;        // 0x00
	undefined4 m_unk0x04;        // 0x04
	undefined4 m_unk0x08;        // 0x08
	undefined4 m_unk0x0c;        // 0x0c
	undefined4 m_unk0x10;        // 0x10
	undefined4 m_unk0x14;        // 0x14
	undefined4 m_unk0x18;        // 0x18
	undefined4 m_unk0x1c;        // 0x1c
	undefined4 m_unk0x20;        // 0x20
	undefined4 m_unk0x24;        // 0x24
	MechS32 m_displayBrightness; // 0x28
	char m_unk0x2c[0x3c - 0x2c]; // 0x2c
} Unk0x100a14d4;

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

// SIZE 0x3c0a
typedef struct Unk0x10138830 {
	undefined m_unk0x00[0x3c0a]; // 0x00
} Unk0x10138830;

typedef struct DifficultyCfg DifficultyCfg;

DECOMP_SIZE_ASSERT(Unk0x100a14d4, 0x3c)
DECOMP_SIZE_ASSERT(Unk0x10138830, 0x3c0a)

// The globals SimMain and SimWindowProc use are defined here until the objects that own
// them are decompiled.

// GLOBAL: MW2 0x100a175c
MechS32 g_isNetworkGame = 0;

// GLOBAL: MW2 0x100a17a0
MechS32 g_unk0x100a17a0 = 0;

// GLOBAL: MW2 0x100a1498
Unk0x100a14d4 g_unk0x100a1498 = {0x10000, 0x10000, 0x10000, 0x10000, 11, 1, 1, 1, 1, 1, 9, "mcga.dll"};

// GLOBAL: MW2 0x100a14d4
Unk0x100a14d4* g_mw2SndCfgData = NULL;

// GLOBAL: MW2 0x100a244c
MechS32 g_drawModeIndex = -1;

// GLOBAL: MW2 0x100a2450
MechS32 g_initDrawModeParam2 = 1;

// GLOBAL: MW2 0x100a554c
undefined4 g_unk0x100a554c = 0xef;

// GLOBAL: MW2 0x100a6d1c
void (*g_frameDrawCallback)(void) = NULL;

// GLOBAL: MW2 0x100a8694
const char* g_unk0x100a8694 = "PAL";

// GLOBAL: MW2 0x100a8740
undefined4 g_unk0x100a8740 = 0xffffffff;

// GLOBAL: MW2 0x100a9468
MechS32 g_displayBrightness = 9;

// GLOBAL: MW2 0x100a946c
MechS32 g_unk0x100a946c = 9;

// GLOBAL: MW2 0x100aa2ac
MechS32 g_missionTimerStopped = 0;

// GLOBAL: MW2 0x100aa2c0
MechS32 g_unk0x100aa2c0 = 0;

// GLOBAL: MW2 0x100acb18
MechS32 g_shouldQuit = 0;

// GLOBAL: MW2 0x100acb1c
MechS32 g_quitStage = 0;

// GLOBAL: MW2 0x100acb20
undefined g_unk0x100acb20[4] = {0};

// GLOBAL: MW2 0x100acb24
DifficultyCfg* g_difficulty = NULL;

// GLOBAL: MW2 0x100ba54c
MechS32 g_unk0x100ba54c = 0; // declared before g_unk0x100acb2c for the comparison operand order

// GLOBAL: MW2 0x100acb2c
MechS32 g_unk0x100acb2c = 0;

// GLOBAL: MW2 0x100acb60
HWND g_gameWindow = NULL;

// GLOBAL: MW2 0x100acb64
HINSTANCE g_unk0x100acb64 = NULL;

// GLOBAL: MW2 0x100acb68
HANDLE g_hPrimaryHeap = NULL;

// GLOBAL: MW2 0x100acb6c
MechS32 g_dwGameWindowWidth = 0;

// GLOBAL: MW2 0x100acb70
MechS32 g_dwGameWindowHeight = 0;

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
undefined4 g_unk0x100acb90 = 0;

// GLOBAL: MW2 0x100acb94
MechS32 g_mouseOutsideClientWindow = 0;

// GLOBAL: MW2 0x100acb98
MechS32 g_goLaunch = 0;

// GLOBAL: MW2 0x100ae400
char g_gameDir[256] = {0};

// GLOBAL: MW2 0x100ae6d4
MechS32 g_logFileEnabled = 0;

// GLOBAL: MW2 0x100b1770
DrawModeExtension* g_currentDrawModeExtension = NULL;

// GLOBAL: MW2 0x100b1774
DrawMode* g_currentDrawMode = NULL;

// GLOBAL: MW2 0x100b177c
MechS32 g_allowDrawModeFallback = 0;

// GLOBAL: MW2 0x100b1350
MechS32 g_unk0x100b1350 = 0;

// GLOBAL: MW2 0x100b1788
undefined g_paletteColors[0x300] = {0};

// GLOBAL: MW2 0x100b1aa4
WindowMode g_windowMode = c_windowModeUnknown;

// GLOBAL: MW2 0x100ba584
BOOL g_ticksTimerInitialized = 0;

// GLOBAL: MW2 0x100c2890
RECT g_gameWindowRect;

// GLOBAL: MW2 0x100e9240
MechU32 g_unk0x100e9240;

// GLOBAL: MW2 0x100e926d
MechU8 g_unk0x100e926d;

// GLOBAL: MW2 0x100e9322
MechS32 g_unk0x100e9322;

// GLOBAL: MW2 0x100e933c
MechU32 g_unk0x100e933c;

// GLOBAL: MW2 0x1012b7c0
Unk0x1012b7c0 g_unk0x1012b7c0;

// GLOBAL: MW2 0x10138830
Unk0x10138830 g_objectiveTable[1]; // length unknown

// GLOBAL: MW2 0x10176ed0
undefined g_unk0x10176ed0[4]; // size unknown

// GLOBAL: MW2 0x10176ef0
undefined g_mainPixelBuffer[4]; // size unknown

// GLOBAL: MW2 0x10181b40
MechU32 g_paletteResourceIds[20];

// GLOBAL: MW2 0x10138820
MechS32 g_objectiveCount; // defined last for the operand order of the DoFirstObjtv loop test

void ApplyPendingPalette(void);
void UpdatePaletteFade(void);
void StartPalettes(MechS32 p_unk0x00);
void FirstGPAnim(void);
void StartSupAnim(MechS32 p_unk0x00);
void StopSupAnim(void);
void UpdateDebris(void);
void ZeroChunx(void);
void StartMissionMusic(void);
void LoopCdMusic(void);
void FirstAudio(void);
void DoAudio(void);
void ShutdownAudio(void);
void CollectMissionAudio(void);
void DrawPausedBanner(void);
void LoadWorld(char* p_unk0x00);
void AfterWorldLoader(void);
void FirstNetwork(NetLaunchInfo* p_unk0x00);
MechS32 FirstExternalCtrl(void);
void UpdateNetwork(void);
void ShutdownNetwork(void);
void FirstEyepoint(void);
void UpdateEyepoint(void);
MechS32 InitDisplayGeometry(void);
void FirstRender(void);
void SecondRender(void);
void Blit(void);
void ShutdownRender(void);
void* FUN_1001a19f(undefined4 p_unk0x00, MechS32 p_unk0x04, const char* p_unk0x08, undefined4 p_unk0x0c);
void DoFirstObjtv(Unk0x10138830* p_unk0x00, MechS32 p_unk0x04);
void UpdateObjectives(void);
void EndTheMission1(void);
void EndTheMission2(void);
MechS32 ProcessCmdLineArgs(LPSTR p_unk0x00, undefined4* p_unk0x04, char* p_unk0x08);
void UpdateGeoCache(void);
void FirstStaticCache(void);
void Error(MechS32 p_unk0x00, const char* p_unk0x04, ...);
void RegisterMenu(MechS32 p_unk0x00);
void FreeMenus(void);
void FirstMenu(void);
void UpdateMenuKey(void);
void UpdateMenus(void);
undefined4 FUN_1003da65(undefined4 p_unk0x00);
void HandleInput(UINT p_msg, WPARAM p_wParam, LPARAM p_lParam);
void FUN_10042d55(void);
void InitStaticMem(char* p_unk0x00);
void FadeToEndPalette(MechS32 p_unk0x00);
void FirstResource(void);
void CloseResourceFile(void);
void CachePreloads(void);
void DebugLog(const char* p_format, ...);
void FirstAI(void);
void GenerateGammas(void);
void FUN_10058750(void);
void SimEntranceDbug(char* p_unk0x00, MechS32 p_unk0x04);
void AdvanceSpeechQueue(void);
void HandleGameKeys(MechS32 p_unk0x00, MechS32 p_unk0x04, MechS32 p_unk0x08);
void SetRes(void);
void FillRenderTargetRect(void* p_unk0x00, MechS32 p_unk0x04);
void HandleMessages(void);
void UpdatePauseState(void);
void SetGameResolution(char* p_unk0x00);
void PauseTimer(MechS32 p_unk0x00, MechS32 p_unk0x04);
void AdvanceAnimations(void);
void FirstShots(void);
void UpdateAllShots(void);
void UpdateEffects(void);
void SaveCarCfg(void);
void FirstClassFunctions(void);
void UpdateAllPlayers(void);
void LateUpdateAllPlayers(void);
void UpdateLocalPlayer(void);
void DrawLocalPlayer(void);
void ShutdownAllPlayers(void);
void ZeroGamethings(void);
void DrawTimedOverlays(void);
MechS32 LoadDifficultyCfg(const char* p_unk0x00, DifficultyCfg** p_unk0x04);
MechS32 LoadSndCfg(const char* p_unk0x00, Unk0x100a14d4** p_unk0x04);
MechS32 StartupCheckStub(void);
void OpenMw2Log(void);
void CloseMw2Log(void);
void InitRandom(MechS32 p_unk0x00);
MechS32 FirstPerfSetting(void);
MechS32 InitDrawMode(
	MechS32 p_unk0x00,
	MechS32 p_unk0x04,
	void* p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14
);
void ProfileDrawModePerformance(void);
void ToggleFullScreen(void);
void DdrawFill(MechS32 p_unk0x00, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c, undefined4 p_unk0x10);
void FirstInputs(void);
void UpdateInputs(void);
void CloseInputDevices(void);
void FirstClock(void);
void NextClock(void);
void StopTimers(void);
void ResetClocks(void);
void RunTimedCallbacks(undefined* p_unk0x00);
void FirstEnvironment(void);
void FUN_1007d6bb(void);

// Matches except for the stack slots of seven locals (a consistent permutation; the original
// assigns them in declaration order, which VC++ 4.1 doesn't reproduce from this source).
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
	g_hPrimaryHeap = HeapCreate(HEAP_NO_SERIALIZE, 1000000, 0);
	if (g_hPrimaryHeap == NULL) {
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
			*g_mw2SndCfgData = g_unk0x100a1498;
		}
	}
	else {
		g_unk0x100a1498 = *g_mw2SndCfgData;
	}

	g_displayBrightness = g_unk0x100a946c = g_mw2SndCfgData->m_displayBrightness;
	g_unk0x1012b7c0.m_unk0x00 = 0;
	if (g_mw2SndCfgData->m_unk0x2c[0]) {
		g_unk0x1012b7c0.m_unk0x00 |= 1;
		if (_stricmp(g_mw2SndCfgData->m_unk0x2c, "scan") == 0) {
			g_unk0x1012b7c0.m_unk0x04[0] = 0;
		}
		else {
			strncpy(g_unk0x1012b7c0.m_unk0x04, g_mw2SndCfgData->m_unk0x2c, 12);
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

	InitDrawMode(5, 0, g_mainPixelBuffer, 640, 480, 0);
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

	DebugLog("FirstClock()\n");
	__try {
		FirstClock();
		DebugLog("StartSupAnim()\n");
		StartSupAnim(GetDeviceCaps(GetDC(g_gameWindow), NUMCOLORS) == -1 || g_windowMode == c_windowModeFullscreen);
		DebugLog("FirstResource()\n");
		FirstResource();
		DebugLog("InitStaticMem()\n");
		InitStaticMem(missionName);
		DebugLog("generate_gammas()\n");
		GenerateGammas();
		DebugLog("InitRandom()\n");
		InitRandom(seed);
		DebugLog("FirstAudio()\n");
		FirstAudio();
		DebugLog("FirstRender()\n");
		FirstRender();
		DebugLog("FirstNetwork()\n");
		FirstNetwork(p_netLaunch);
		DebugLog("ResetClocks()\n");
		ResetClocks();
		DebugLog("WinMain(1): pause_timer(TRUE)");
		PauseTimer(0x80, TRUE);
		DebugLog("FirstShots()\n");
		FirstShots();
		DebugLog("ZeroGamethings()\n");
		ZeroGamethings();
		DebugLog("ZeroChunx()\n");
		ZeroChunx();
		DebugLog("LoadWorld()\n");
		LoadWorld(missionName);
		DebugLog("CollectMissionAudio()\n");
		CollectMissionAudio();
		DebugLog("SetRes()\n");
		SetRes();
		DebugLog("FirstEnvironment()\n");
		FirstEnvironment();
		DebugLog("FirstStaticCache()\n");
		FirstStaticCache();
		DebugLog("CachePreloads()\n");
		CachePreloads();
		DebugLog("AfterWorldLoader()\n");
		AfterWorldLoader();
		DebugLog("FirstGPAnim()\n");
		FirstGPAnim();
		DebugLog("FirstEyepoint()\n");
		FirstEyepoint();
		DebugLog("FirstInputs()\n");
		FirstInputs();
		DebugLog("FirstMenu()\n");
		FirstMenu();
		DebugLog("RegisterMenu()...\n");
		RegisterMenu(4);
		RegisterMenu(5);
		RegisterMenu(1);
		RegisterMenu(7);
		RegisterMenu(8);
		RegisterMenu(3);
		DebugLog("DoFirstObjtv()...\n");
		for (i = 0; i < g_objectiveCount; i++) {
			DoFirstObjtv(&g_objectiveTable[i], i);
		}

		DebugLog("FirstClassFunctions()\n");
		FirstClassFunctions();
		DebugLog("FirstAI()\n");
		FirstAI();
		DebugLog("FirstExternalCtrl()\n");
		if (!FirstExternalCtrl()) {
			g_shouldQuit = 1;
			g_quitStage = 3;
		}

		DebugLog("UpdateGeoCache()\n");
		UpdateGeoCache();
		DebugLog("SecondRender()\n");
		SecondRender();
		DebugLog("FirstPerfSetting()\n");
		FirstPerfSetting();
		if (g_missionTimerStopped) {
			DebugLog("SimEntranceDbug()\n");
			SimEntranceDbug(missionName, 0);
		}

		DebugLog("StopSupAnim()\n");
		StopSupAnim();
		if (GetDeviceCaps(GetDC(g_gameWindow), NUMCOLORS) == -1 || g_windowMode == c_windowModeFullscreen) {
			DebugLog("StartPalettes()\n");
			StartPalettes(0);
		}
		else {
			hasPalette = g_paletteResourceIds[0x10] != -1;
			if (hasPalette) {
				palette = FUN_1001a19f(g_unk0x100a8740, hasPalette, g_unk0x100a8694, 0);
				if (palette) {
					g_currentDrawModeExtension->m_setPalette(0, 0x100, palette, 1);
				}
			}
		}

		g_unk0x100acb78 = 1;
		DebugLog("InitDrawMode()\n");
		if (!InitDrawMode(
				g_drawModeIndex,
				g_initDrawModeParam2,
				g_mainPixelBuffer,
				g_dwGameWindowWidth,
				g_dwGameWindowHeight,
				0
			)) {
			Error(0x50, "Error profiling video modes.");
		}

		while (ShowCursor(FALSE) >= 0) {
		}

		g_mouseOutsideClientWindow = 0;
		if (!g_allowDrawModeFallback) {
			g_goLaunch |= 2;
		}

		g_unk0x100e926d = 1;
		g_unk0x100e9322 = -1;
		while (g_quitStage < 3) {
			if (g_goLaunch == 3) {
				DebugLog("GoLaunch == GO_READY\n");
				g_goLaunch |= 0x80000000;
				DebugLog("WinMain(2): pause_timer(false)");
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
			RunTimedCallbacks(g_unk0x100acb20);
			if (!g_isNetworkGame || (g_goLaunch & 0x80000000)) {
				UpdateAllPlayers();
			}

			UpdateEyepoint();
			UpdateAllShots();
			UpdateEffects();
			UpdateLocalPlayer();
			LateUpdateAllPlayers();
			UpdateDebris();
			if (g_allowDrawModeFallback) {
				while (g_allowDrawModeFallback) {
					if (g_currentDrawModeExtension->m_index == 0) {
						DdrawFill(0, 0, g_dwGameWindowWidth, g_dwGameWindowHeight, g_unk0x100a554c);
					}

					if ((g_windowActive ? g_currentDrawModeExtension->m_lockBuffer() : -1) == 0) {
						g_frameDrawCallback();
						DrawLocalPlayer();
						UpdateMenus();
						DrawTimedOverlays();
						FUN_10058750();
					}

					Blit();
					ProfileDrawModePerformance();
				}
			}

			if (g_goLaunch & 0x80000000) {
				UpdateGeoCache();
			}

			AdvanceAnimations();
			UpdatePaletteFade();
			ApplyPendingPalette();
			if (g_windowActive && g_currentDrawModeExtension->m_index == 0) {
				DdrawFill(0, 0, g_dwGameWindowWidth, g_dwGameWindowHeight, g_unk0x100a554c);
			}

			if ((g_windowActive ? g_currentDrawModeExtension->m_lockBuffer() : -1) == 0) {
				g_frameDrawCallback();
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
		if ((g_windowActive ? g_currentDrawModeExtension->m_lockBuffer() : -1) == 0) {
			FillRenderTargetRect(g_unk0x10176ed0, 0);
		}

		DebugLog("Calling Blit()\n");
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

		DebugLog("Calling EndTheMission()\n");
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

	HeapDestroy(g_hPrimaryHeap);
	g_hPrimaryHeap = NULL;
	while (ShowCursor(TRUE) < 1) {
	}

	result = g_unk0x100b1350 ? 0xff : 0;
	return result;
}

// FUNCTION: MW2 0x10067757
LRESULT CALLBACK SimWindowProc(HWND p_hWnd, UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	WINDOWPOS* windowPos;

	if (p_msg >= WM_KEYFIRST && p_msg <= WM_KEYLAST) {
		HandleInput(p_msg, p_wParam, p_lParam);
		return 0;
	}

	switch (p_msg) {
	case WM_ACTIVATEAPP:
		if (g_shouldQuit) {
			break;
		}

		g_windowActive = p_wParam;
		if (g_windowActive == TRUE) {
			FUN_10042d55();
			if (g_desktopWidth <= 640 && g_desktopHeight <= 480) {
				if (g_shouldToggleFullscreen == TRUE) {
					ToggleFullScreen();
					g_shouldToggleFullscreen = FALSE;
					ShowWindow(g_gameWindow, SW_RESTORE);
				}
				else if (g_currentDrawModeExtension && g_currentDrawModeExtension->m_index == 1) {
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
				if (g_currentDrawModeExtension && g_currentDrawModeExtension->m_index == 0 &&
					g_shouldToggleFullscreen == FALSE) {
					ShowWindow(g_gameWindow, SW_MINIMIZE);
					g_shouldToggleFullscreen = TRUE;
					ToggleFullScreen();
				}
				else if (g_currentDrawModeExtension && g_currentDrawModeExtension->m_index == 1) {
					ShowWindow(g_gameWindow, SW_HIDE);
				}
			}
			else {
				if (g_windowMode == c_windowModeFullscreen) {
					ShowWindow(g_gameWindow, SW_MINIMIZE);
				}
			}
		}

		if (g_currentDrawModeExtension) {
			g_currentDrawModeExtension->m_setPalette(0, 0x100, g_paletteColors, 1);
		}
		return 0;
	case WM_PAINT:
		if (g_unk0x100acb78 && g_windowMode == c_windowModeWindowed) {
			g_currentDrawMode->m_blitFlip();
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
			if (!g_simPaused || FUN_1003da65(4) == 1) {
				while (ShowCursor(FALSE) >= 0) {
				}
				g_mouseOutsideClientWindow = FALSE;
			}
		}
		return 0;
	case WM_WINDOWPOSCHANGING:
		windowPos = (WINDOWPOS*) p_lParam;
		if (g_unk0x100acb90) {
			LONG time;

			time = GetMessageTime();
			if (time > g_unk0x100e933c && (g_unk0x100e933c > g_unk0x100e9240 || time < g_unk0x100e9240)) {
				g_unk0x100acb90 = FALSE;
			}
			else {
				windowPos->x = g_gameWindowRect.left;
				windowPos->y = g_gameWindowRect.top;
				windowPos->cx = g_gameWindowRect.right;
				windowPos->cy = g_gameWindowRect.bottom;
			}
			return 0;
		}
		else {
			break;
		}
	case WM_QUERYNEWPALETTE:
		if (g_currentDrawModeExtension) {
			g_currentDrawModeExtension->m_setPalette(0, 0x100, g_paletteColors, 1);
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

// STUB: MW2 0x10067bbc
void HandleMessages(void)
{
	STUB(0x10067bbc);
}

// STUB: MW2 0x10067c79
void UpdatePauseState(void)
{
	STUB(0x10067c79);
}

// STUB: MW2 0x10067e23
void SetGameResolution(char* p_unk0x00)
{
	STUB(0x10067e23);
}

// STUB: MW2 0x10068058
void PauseTimer(MechS32 p_unk0x00, MechS32 p_unk0x04)
{
	STUB(0x10068058);
}
