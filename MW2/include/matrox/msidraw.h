#ifndef MATROX_MSIDRAW_H
#define MATROX_MSIDRAW_H

#include "displaybackend.h"
#include "pane.h"
#include "refreshmode.h"
#include "types.h"

#include <windows.h>

// The functions and globals of matrox/msidraw.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern DisplayBackend g_msiBackend;
	extern RefreshMode g_msiFlipRefreshMode;
	extern RefreshMode g_msiBlitFlipRefreshMode;
	extern RefreshMode g_msiVideoMemoryRefreshMode;
	extern RefreshMode g_msiSystemMemoryRefreshMode;
	extern HWND g_msiWindow;

	// Fills a rectangle of the screen (right and bottom exclusive) with the color 0xfffe.
	void FUN_10088246(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
	// Fills a pane's rectangle of the screen with the color 0xfffe.
	void FUN_10088280(PANE* p_pane);
	void MsiClearFrame(void);
	void MsiSwapBuffers(void);

#ifdef __cplusplus
}
#endif

#endif // MATROX_MSIDRAW_H
