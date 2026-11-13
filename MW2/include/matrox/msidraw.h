#ifndef MATROX_MSIDRAW_H
#define MATROX_MSIDRAW_H

#include "displaybackend.h"
#include "matrox/colorkey.h"
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

	void MsiClearFrame(void);
	void MsiSwapBuffers(void);

#ifdef __cplusplus
}
#endif

#endif // MATROX_MSIDRAW_H
