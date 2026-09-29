#ifndef SIMMAIN_H
#define SIMMAIN_H

#include "types.h"

#include <windows.h>

// The functions and globals of simmain.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_isNetworkGame;
	extern MechS32 g_unk0x100a17a0;
	extern MechS32 g_localPlayerId;
	extern HANDLE g_primaryHeap;
	extern MechS32 g_gameWindowWidth;
	extern MechS32 g_gameWindowHeight;
	extern MechS32 g_windowActive;
	extern MechS32 g_simPaused;
	extern MechS32 g_pauseRequested;
	extern MechS32 g_mouseOutsideClientWindow;

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
