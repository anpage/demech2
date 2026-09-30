#ifndef SIMMAIN_H
#define SIMMAIN_H

#include "cockpit.h"
#include "decomp.h"
#include "displaybackend.h"
#include "effectinfo.h"
#include "eyepoint.h"
#include "pixelbuffer.h"
#include "point.h"
#include "rendertarget.h"
#include "slateheron.h"
#include "soundconfig.h"
#include "starmission.h"
#include "types.h"

#include <windows.h>

struct DifficultyCfg;
struct ScarletOrchid0x4c;

// The functions and globals of simmain.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern EffectInfo g_effectInfo[0x20];
	extern MechS32 g_normalFov;
	extern MechS32 g_zoomFov;
	extern MechS32 g_unk0x100a2414;
	extern undefined4 g_unk0x100a2420;
	extern MechS32 g_unk0x100a2424;
	extern MechS32 g_unk0x100a2c04;
	extern MechS32 g_unk0x100a2c10;
	extern MechS32 g_unk0x100a2c18;
	extern void* g_unk0x100a245c;
	extern MechS32 g_unk0x100a2460;
	extern MechS32 g_unk0x100a2464;
	extern Eyepoint* g_eyepoint;
	extern struct DifficultyCfg* g_difficulty;
	extern SlateHeron0x68 g_unk0x100a6cc8;
	extern MechS32 g_unk0x100bfd60[800];
	extern MechS32 g_unk0x100c09e0[800];
	extern const char* g_unk0x100a8680;
	extern const char* g_unk0x100a8684;
	extern const char* g_unk0x100a8690;
	extern const char* g_unk0x100a8694;
	extern const char* g_unk0x100a8698;
	extern const char* g_unk0x100a86a0;
	extern const char* g_unk0x100a86bc;
	extern char g_unk0x100a87c0[];
	extern MechS32 g_unk0x100e9340;
	extern undefined g_unk0x100e9350[0x100];
	extern MechS32 g_unk0x100e9614;
	extern undefined4 g_unk0x100a8740;
	extern MechChar* g_unk0x100a8744;
	extern MechS32 g_missionTimerStopped;
	extern MechS32 g_shouldQuit;
	extern MechS32 g_goLaunch;
	extern MechS32 g_quitStage;
	extern MechS32 g_localPlayerId;
	extern struct Player* g_localPlayer;
	extern MechS32* g_unk0x100a2434;
	extern MechS32 g_unk0x100a2bf8;
	extern MechS32 g_missionTime;
	extern MechS32 g_unk0x100aa2a4;
	extern MechS32 g_unk0x100aa2bc;
	extern MechS32 g_unk0x100c3358;
	extern MechS32 g_unk0x100ea3e4;
	extern MechS32 g_currentObjective[64];
	extern StarMission g_objectiveTable[16];
	extern MechS32 g_objectiveCount;
	extern const char* g_unk0x100a86c4;
	extern const char* g_unk0x100a86d0;
	extern const char* g_unk0x100a8674;
	extern const char* g_unk0x100a8678;
	extern const char* g_unk0x100a86cc;
	extern HANDLE g_primaryHeap;
	extern MechS32 g_gameWindowWidth;
	extern MechS32 g_gameWindowHeight;
	extern MechS32 g_windowActive;
	extern MechS32 g_simPaused;
	extern MechS32 g_pauseRequested;
	extern MechS32 g_mouseOutsideClientWindow;
	extern HWND g_gameWindow;
	extern undefined4 g_windowedSwitchPending;
	extern undefined4 g_shouldToggleFullscreen;
	extern MechS32 g_desktopWidth;
	extern MechS32 g_desktopHeight;
	extern MechU32 g_windowedSwitchTime;
	extern MechU32 g_windowedSwitchDeadline;
	extern MechS32 g_menuRepeatTimer;
	extern CockpitGaugeFn g_cockpitGauges[10];
	extern RenderTarget g_unk0x100a5a68[5];
	extern RenderTarget g_unk0x100adf58;
	extern void* g_unk0x100a5bb8[4];
	extern Point g_unk0x100a5ee8[6];
	extern MechS32 g_unk0x100a6d30;
	extern undefined4 g_unk0x100a5a24;
	extern undefined4 g_unk0x100a5f18;
	extern RenderTarget g_unk0x100bdff8;
	extern RenderTarget g_currentRenderTarget;
	extern PixelBuffer g_mainPixelBuffer;

	void HandleMessages(void);
	void UpdatePauseState(void);
	void SetGameResolution(char* p_driverName);
	void GameTickTimerCallback(void);
	MechS16 AllocTicks(MechU32 p_flags);
	MechS32 GetTicks(MechU32 p_handle);
	void ResetTicks(MechU32 p_handle);
	void SetTicks(MechU32 p_handle, MechS32 p_ticks);
	void FreeTicks(MechU32 p_handle);
	void PauseTimer(MechS32 p_flags, MechS32 p_paused);

#ifdef __cplusplus
}
#endif

#endif // SIMMAIN_H
