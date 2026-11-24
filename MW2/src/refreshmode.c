#include "refreshmode.h"

#include "debugprint.h"
#include "decomp.h"
#include "directdraw.h"
#include "dispdibmode.h"
#include "displaybackend.h"
#include "drawbitmapinfo.h"
#include "gdi.h"
#ifdef MW2_MATROX
#include "matrox/msidraw.h"
#endif
#ifdef MW2_3DFX
#include "3dfx/glidedraw.h"
#endif
#include "menu.h"
#include "mouse.h"
#include "palettecolor.h"
#include "simmain.h"
#include "ticks.h"
#include "types.h"
#include "window.h"

#include <windows.h>

// The Matrox edition's messages end in a newline.
#ifdef MW2_DEBUG_LOG_TEXT
#define TEXT_LINE(text) text "\n"
#else
#define TEXT_LINE(text) text
#endif

// The refresh mode manager, the simulator's copy of the shell's. The DirectDraw back end lives in
// directdraw.c, the DisplayDib one in dispdib.c, the GDI one in gdi.c.

// Indexed by DisplayBackend::m_id. The accelerated editions' first back end and refresh modes
// are their own, in DirectDraw's place: matrox/msidraw.c's and 3dfx/glidedraw.c's.
// GLOBAL: MW2 0x100b1748
// GLOBAL: MW2GBL 0x100a4a38
// GLOBAL: MW2PREMERCS 0x100ae230
// GLOBAL: MW2MATROX 0x100b1a50
// GLOBAL: MW23DFX 0x102492b8
// GLOBAL: MW2ATI 0x1017d2b8
// GLOBAL: MW2S3 0x1006a4b8
// GLOBAL: MW2POWERVR 0x101a8310
DisplayBackend* g_displayBackends[3] = {
#if defined(MW2_MATROX)
	&g_msiBackend,
#elif defined(MW2_3DFX)
	&g_glideBackend,
#else
	&g_directDrawBackend,
#endif
	&g_dispDibBackend,
	&g_gdiBackend
};

// GLOBAL: MW2 0x100b1758
// GLOBAL: MW2GBL 0x100a4a48
// GLOBAL: MW2PREMERCS 0x100ae240
// GLOBAL: MW2MATROX 0x100b1a60
// GLOBAL: MW23DFX 0x102492c8
// GLOBAL: MW2ATI 0x1017d2c8
// GLOBAL: MW2S3 0x1006a4c8
// GLOBAL: MW2POWERVR 0x101a8320
RefreshMode* g_refreshModes[6] = {
#if defined(MW2_MATROX)
	&g_msiFlipRefreshMode,
	&g_msiBlitFlipRefreshMode,
	&g_msiVideoMemoryRefreshMode,
	&g_msiSystemMemoryRefreshMode,
#elif defined(MW2_3DFX)
	&g_glideFlipRefreshMode,
	&g_glideBlitFlipRefreshMode,
	&g_glideVideoMemoryRefreshMode,
	&g_glideSystemMemoryRefreshMode,
#else
	&g_ddrawFlipRefreshMode,
	&g_ddrawBlitFlipRefreshMode,
	&g_ddrawVideoMemoryRefreshMode,
	&g_ddrawSystemMemoryRefreshMode,
#endif
	&g_dispDibRefreshMode,
	&g_gdiRefreshMode,
};

// GLOBAL: MW2 0x100b1770
// GLOBAL: MW2GBL 0x100a4a60
// GLOBAL: MW2PREMERCS 0x100ae258
// GLOBAL: MW2MATROX 0x100b1a78
// GLOBAL: MW23DFX 0x102492e0
// GLOBAL: MW2ATI 0x1017d2e0
// GLOBAL: MW2S3 0x1006a4e0
// GLOBAL: MW2POWERVR 0x101a8338
DisplayBackend* g_currentDisplayBackend = NULL;

// GLOBAL: MW2 0x100b1774
// GLOBAL: MW2GBL 0x100a4a64
// GLOBAL: MW2PREMERCS 0x100ae25c
// GLOBAL: MW2MATROX 0x100b1a7c
// GLOBAL: MW23DFX 0x102492e4
// GLOBAL: MW2ATI 0x1017d2e4
// GLOBAL: MW2S3 0x1006a4e4
// GLOBAL: MW2POWERVR 0x101a833c
RefreshMode* g_currentRefreshMode = NULL;

// The fastest refresh mode found by the profiling, the one to return to from windowed mode.
// GLOBAL: MW2 0x100b1778
// GLOBAL: MW2GBL 0x100a4a68
// GLOBAL: MW2PREMERCS 0x100ae260
// GLOBAL: MW2MATROX 0x100b1a80
// GLOBAL: MW23DFX 0x102492e8
// GLOBAL: MW2ATI 0x1017d2e8
// GLOBAL: MW2S3 0x1006a4e8
RefreshMode* g_fastestRefreshMode = NULL;

// GLOBAL: MW2 0x100b177c
// GLOBAL: MW2GBL 0x100a4a6c
// GLOBAL: MW2PREMERCS 0x100ae264
// GLOBAL: MW2MATROX 0x100b1a84
// GLOBAL: MW23DFX 0x102492ec
// GLOBAL: MW2ATI 0x1017d2ec
// GLOBAL: MW2S3 0x1006a4ec
// GLOBAL: MW2POWERVR 0x101a8344
MechS32 g_refreshModeFallback = FALSE;

// The last refresh mode the profiling tries.
// GLOBAL: MW2 0x100b1780
// GLOBAL: MW2GBL 0x100a4a70
// GLOBAL: MW2PREMERCS 0x100ae268
// GLOBAL: MW2MATROX 0x100b1a88
// GLOBAL: MW23DFX 0x102492f0
// GLOBAL: MW2ATI 0x1017d2f0
// GLOBAL: MW2S3 0x1006a4f0
MechS32 g_lastProfiledRefreshMode = 4;

// The buffer the active refresh mode renders into.
// GLOBAL: MW2 0x100b1784
// GLOBAL: MW2GBL 0x100a4a74
// GLOBAL: MW2PREMERCS 0x100ae26c
// GLOBAL: MW2MATROX 0x100b1a8c
// GLOBAL: MW23DFX 0x102492f4
// GLOBAL: MW2ATI 0x1017d2f4
// GLOBAL: MW2S3 0x1006a4f4
// GLOBAL: MW2POWERVR 0x101a834c
WINDOW* g_refreshModeBuffer = NULL;

// GLOBAL: MW2 0x100b1788
// GLOBAL: MW2GBL 0x100a4a78
// GLOBAL: MW2PREMERCS 0x100ae270
// GLOBAL: MW2MATROX 0x100b1a90
// GLOBAL: MW23DFX 0x102492f8
// GLOBAL: MW2ATI 0x1017d2f8
// GLOBAL: MW2S3 0x1006a4f8
// GLOBAL: MW2POWERVR 0x101a8350
PaletteColor g_paletteColors[0x100] = {0};

// The DIB bits of the GDI and DisplayDib back ends, restored into g_refreshModeBuffer by
// m_acquireFramebuffer.
// GLOBAL: MW2 0x100b1a88
// GLOBAL: MW2GBL 0x100a4d78
// GLOBAL: MW2PREMERCS 0x100ae570
// GLOBAL: MW2MATROX 0x100b1d90
// GLOBAL: MW23DFX 0x102495f8
// GLOBAL: MW2ATI 0x1017d5f8
// GLOBAL: MW2S3 0x1006a7f8
// GLOBAL: MW2POWERVR 0x101a8650
undefined* g_dibBits = NULL;

// GLOBAL: MW2 0x100b1aa4
// GLOBAL: MW2GBL 0x100a4d94
// GLOBAL: MW2PREMERCS 0x100ae58c
// GLOBAL: MW2MATROX 0x100b1dac
// GLOBAL: MW23DFX 0x10249614
// GLOBAL: MW2ATI 0x1017d614
// GLOBAL: MW2S3 0x1006a814
// GLOBAL: MW2POWERVR 0x101a866c
MechS32 g_windowMode = 0;

// GLOBAL: MW2 0x100b1aa8
// GLOBAL: MW2GBL 0x100a4d98
// GLOBAL: MW2PREMERCS 0x100ae590
// GLOBAL: MW2MATROX 0x100b1db0
// GLOBAL: MW23DFX 0x10249618
// GLOBAL: MW2ATI 0x1017d618
// GLOBAL: MW2S3 0x1006a818
// GLOBAL: MW2POWERVR 0x101a8670
MechS32 g_refreshModeInactive = 1;

// The other editions define these three in simmain.c (the Mercenaries disc's MW2.DLL builds GBL's
// units, until its own are placed).
#if defined(MW2_GBL) || defined(MW2_PREMERCS)
// GLOBAL: MW2GBL 0x100a4d9c
undefined4 g_shouldToggleFullscreen = 0;

// GLOBAL: MW2GBL 0x100a4da0
MechS32 g_desktopWidth = 0;

// GLOBAL: MW2GBL 0x100a4da4
MechS32 g_desktopHeight = 0;
#endif

// GLOBAL: MW2 0x100b1aac
// GLOBAL: MW2GBL 0x100a4da8
// GLOBAL: MW2PREMERCS 0x100ae5a0
// GLOBAL: MW2MATROX 0x100b1db4
// GLOBAL: MW23DFX 0x1024961c
// GLOBAL: MW2ATI 0x1017d61c
// GLOBAL: MW2S3 0x1006a81c
// GLOBAL: MW2POWERVR 0x101a8674
MechS32 g_profileFrame = 0;

// GLOBAL: MW2 0x100bf1c8
// GLOBAL: MW2GBL 0x100b9440
// GLOBAL: MW2PREMERCS 0x100c8db8
// GLOBAL: MW2MATROX 0x101249f8
// GLOBAL: MW23DFX 0x1007b000
// GLOBAL: MW2ATI 0x10070000
// GLOBAL: MW2S3 0x1007cf30
static LARGE_INTEGER g_profileStart;

// The window's position and size in windowed mode (SetWindowPos arguments, not corners).
// GLOBAL: MW2 0x100c2890
// GLOBAL: MW2GBL 0x10132d70
// GLOBAL: MW2PREMERCS 0x10213c50
// GLOBAL: MW2MATROX 0x1012f950
// GLOBAL: MW23DFX 0x102453b0
// GLOBAL: MW2ATI 0x101791d0
// GLOBAL: MW2S3 0x100ab7d0
// GLOBAL: MW2POWERVR 0x101a3d50
RECT g_windowedRect;

// GLOBAL: MW2 0x100c28a0
// GLOBAL: MW2GBL 0x10132d80
// GLOBAL: MW2PREMERCS 0x10213c60
// GLOBAL: MW2MATROX 0x1012f520
// GLOBAL: MW23DFX 0x10244f80
// GLOBAL: MW2ATI 0x10178da0
// GLOBAL: MW2S3 0x100ab7e0
// GLOBAL: MW2POWERVR 0x101a3920
DrawBitmapInfo g_bitmapInfo;

// The frame's size in pixels, p_width * p_height of InitRefreshMode.
// GLOBAL: MW2 0x100c2cc8
// GLOBAL: MW2GBL 0x101331a8
// GLOBAL: MW2PREMERCS 0x10214088
// GLOBAL: MW2MATROX 0x1012f510
// GLOBAL: MW23DFX 0x10244f78
// GLOBAL: MW2ATI 0x10178d98
// GLOBAL: MW2S3 0x100abc08
// GLOBAL: MW2POWERVR 0x101a3918
MechS32 g_refreshModePixelCount;

// GLOBAL: MW2 0x100c2ce4
// GLOBAL: MW2GBL 0x101331ac
// GLOBAL: MW2PREMERCS 0x102140a4
// GLOBAL: MW2MATROX 0x1012f50c
// GLOBAL: MW23DFX 0x10244f74
// GLOBAL: MW2ATI 0x10178d94
// GLOBAL: MW2S3 0x100abc0c
// GLOBAL: MW2POWERVR 0x101a3914
MechS32 g_refreshModeWidth;

// GLOBAL: MW2 0x100c2ce8
// GLOBAL: MW2GBL 0x101331b0
// GLOBAL: MW2PREMERCS 0x102140a8
// GLOBAL: MW2MATROX 0x1012f508
// GLOBAL: MW23DFX 0x10244f50
// GLOBAL: MW2ATI 0x10178d90
// GLOBAL: MW2S3 0x100abc10
// GLOBAL: MW2POWERVR 0x101a3910
MechS32 g_refreshModeHeight;

// Switches to refresh mode p_mode (-1: the first), falling through the later modes while a mode
// is unavailable if p_allowFallback is set. The window covers the screen unless p_width x
// p_height is smaller.
// FUNCTION: MW2 0x10076d50
// FUNCTION: MW2GBL 0x1002ef30
// FUNCTION: MW2PREMERCS 0x1001d630
// FUNCTION: MW2MATROX 0x1007add0
// FUNCTION: MW23DFX 0x100025e0
// FUNCTION: MW2ATI 0x100024f0
// FUNCTION: MW2S3 0x1002ecd0
// FUNCTION: MW2POWERVR 0x100024d0
MechS32 InitRefreshMode(
	MechS32 p_mode,
	MechS32 p_allowFallback,
	WINDOW* p_buffer,
	MechS32 p_width,
	MechS32 p_height,
	MechS32 p_menu
)
{
	MechS32 screenHeight;
	RefreshMode* mode;
	MechS32 screenWidth;
	MechS32 unused;
	DisplayBackend* backend;
	RECT rect;

	if (p_mode == -1) {
		mode = g_refreshModes[0];
	}
	else if (p_mode >= 0 && p_mode < 6) {
		mode = g_refreshModes[p_mode];
	}
	else {
		return 0;
	}

	backend = g_displayBackends[mode->m_backend];
	g_refreshModeFallback = p_allowFallback;
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	g_windowedRect.left = 0;
	g_windowedRect.top = 0;
	g_windowedRect.right = p_width;
	g_windowedRect.bottom = p_height;
	if (screenWidth <= p_width && screenHeight <= p_height) {
		g_gdiBackend.m_style = WS_POPUP;
		g_windowMode = c_windowModeFullscreen;
	}
	else {
		g_gdiBackend.m_style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		AdjustWindowRect(&g_windowedRect, g_gdiBackend.m_style, p_menu);
		g_windowedRect.right -= g_windowedRect.left;
		g_windowedRect.bottom -= g_windowedRect.top;
		g_windowedRect.top = (screenHeight - g_windowedRect.bottom) / 2;
		g_windowedRect.left = (screenWidth - g_windowedRect.right) / 2;
		g_windowMode = backend->m_windowMode;
	}

	if (backend->m_windowMode == c_windowModeFullscreen) {
		rect.left = 0;
		rect.top = 0;
		rect.right = GetSystemMetrics(SM_CXSCREEN);
		rect.bottom = GetSystemMetrics(SM_CYSCREEN);
		unused = 8;
	}
	else {
		rect = g_windowedRect;
		unused = 0;
	}

	if (g_refreshModeInactive) {
		g_refreshModeWidth = p_width;
		g_refreshModeHeight = p_height;
		AdjustWindowSize(backend);
		g_currentDisplayBackend = backend;
	}
	else if (mode != g_currentRefreshMode || g_refreshModeWidth != p_width || g_refreshModeHeight != p_height) {
		AdjustWindowSize(backend);
		g_currentRefreshMode->m_end();
	}

	g_currentRefreshMode = mode;
	g_refreshModeWidth = p_width;
	g_refreshModeHeight = p_height;
	g_refreshModePixelCount = p_height * p_width;
	g_refreshModeBuffer = p_buffer;

#ifdef MW2_REFRESH_MODE_RETRY
	while (g_currentRefreshMode->m_begin(p_buffer, p_width, p_height)) {
#else
	while (g_currentRefreshMode->m_available && g_currentRefreshMode->m_begin(p_buffer, p_width, p_height)) {
#endif
		DebugPrint("RefreshMode %d not available\n", g_currentRefreshMode->m_index);
		g_currentRefreshMode->m_available = FALSE;
		if (!g_refreshModeFallback || g_currentRefreshMode->m_index == 5) {
			return 0;
		}

		g_currentRefreshMode = g_refreshModes[g_currentRefreshMode->m_index + 1];
	}

#ifdef MW2_REFRESH_MODE_RETRY
	g_currentRefreshMode->m_available = TRUE;
#endif
	if (g_refreshModeInactive && backend->m_id != c_displayBackendGdi) {
		ShowWindow(g_gameWindow, SW_SHOWDEFAULT);
		UpdateWindow(g_gameWindow);
	}

	g_refreshModeInactive = 0;
	return 1;
}

// FUNCTION: MW2 0x10077069
// FUNCTION: MW2GBL 0x1002f247
// FUNCTION: MW2PREMERCS 0x1001d947
// FUNCTION: MW2MATROX 0x1007b0e5
// FUNCTION: MW23DFX 0x10002840
// FUNCTION: MW2ATI 0x10002750
// FUNCTION: MW2S3 0x1002ef30
// FUNCTION: MW2POWERVR 0x10002730
void ShutdownRefreshMode(void)
{
	if (g_currentRefreshMode != NULL) {
		g_currentRefreshMode->m_end();
	}
	if (g_currentDisplayBackend != NULL) {
		g_currentDisplayBackend->m_end();
	}

	g_refreshModeInactive = 1;
}

// Called once a frame while the refresh modes are profiled: times four frames of the current mode,
// then moves to the next one; after the last, SelectFastestRefreshMode picks the fastest.
// FUNCTION: MW2 0x100770a8
// FUNCTION: MW2GBL 0x1002f286
// FUNCTION: MW2MATROX 0x1007b124
// FUNCTION: MW23DFX 0x10002870
// FUNCTION: MW2ATI 0x10002780
// FUNCTION: MW2S3 0x1002ef60
void ProfileRefreshModes(void)
{
	LARGE_INTEGER end;

	if (g_currentRefreshMode->m_index > g_lastProfiledRefreshMode) {
		g_refreshModeFallback = FALSE;
	}
	else if (g_profileFrame == 1) {
		if (!QueryPerformanceCounter(&g_profileStart)) {
			g_refreshModeFallback = FALSE;
			return;
		}

		g_profileFrame++;
	}
	else if (g_profileFrame == 5) {
		if (!QueryPerformanceCounter(&end)) {
			g_refreshModeFallback = FALSE;
			return;
		}

		if (end.HighPart != g_profileStart.HighPart) {
			g_currentRefreshMode->m_profileTime = ~g_profileStart.LowPart + end.LowPart + 1;
		}
		else {
			g_currentRefreshMode->m_profileTime = end.LowPart - g_profileStart.LowPart;
		}

		DebugPrint(
			"Refresh mode %d start=(%u,%d) end=(%u,%d) diff=%u\n",
			g_currentRefreshMode->m_index,
			g_profileStart.LowPart,
			g_profileStart.HighPart,
			end.LowPart,
			end.HighPart,
			g_currentRefreshMode->m_profileTime
		);
		if (g_currentRefreshMode->m_index == g_lastProfiledRefreshMode) {
			g_refreshModeFallback = FALSE;
		}
		else {
			while (g_currentRefreshMode->m_index < g_lastProfiledRefreshMode) {
				g_currentRefreshMode->m_end();
				g_currentRefreshMode = g_refreshModes[g_currentRefreshMode->m_index + 1];
				if (g_currentRefreshMode->m_available &&
					!g_currentRefreshMode->m_begin(g_refreshModeBuffer, g_refreshModeWidth, g_refreshModeHeight)) {
					g_profileFrame = 0;
					break;
				}
				else {
					DebugPrint("Refresh mode %d not available\n", g_currentRefreshMode->m_index);
					g_currentRefreshMode->m_available = FALSE;
					if (g_currentRefreshMode->m_index == g_lastProfiledRefreshMode) {
						g_refreshModeFallback = FALSE;
					}
				}
			}
		}

		if (!g_refreshModeFallback) {
			SelectFastestRefreshMode();
		}
	}
	else {
		g_profileFrame++;
	}
}

// Switches to the available refresh mode with the shortest profile time.
// FUNCTION: MW2 0x100772a4
// FUNCTION: MW2GBL 0x1002f482
// FUNCTION: MW2PREMERCS 0x1001db81
// FUNCTION: MW2MATROX 0x1007b31f
// FUNCTION: MW23DFX 0x10002a20
// FUNCTION: MW2ATI 0x10002930
// FUNCTION: MW2S3 0x1002f110
void SelectFastestRefreshMode(void)
{
	MechS32 i;
	RefreshMode* best;

	best = NULL;
	for (i = 0; i < 6; i++) {
		if (g_refreshModes[i]->m_available && g_refreshModes[i]->m_profileTime > 0 &&
			(best == NULL || g_refreshModes[i]->m_profileTime < best->m_profileTime)) {
			best = g_refreshModes[i];
		}
	}

	if (best != g_currentRefreshMode) {
		g_currentRefreshMode->m_end();
		g_currentRefreshMode = best;
		g_currentRefreshMode->m_begin(g_refreshModeBuffer, g_refreshModeWidth, g_refreshModeHeight);
	}

	g_fastestRefreshMode = g_currentRefreshMode;
	DebugPrint(
		"Refresh mode %d selected with profile time: %u\n",
		g_currentRefreshMode->m_index,
		g_currentRefreshMode->m_profileTime
	);
}

// Switches between fullscreen and the first available windowed refresh mode, or back to the
// fullscreen mode last selected. Unlike the shell's, it does nothing while the frame covers the
// whole screen, unless g_shouldToggleFullscreen is set.
// FUNCTION: MW2 0x10077392
// FUNCTION: MW2GBL 0x1002f570
// FUNCTION: MW2MATROX 0x1007b40e
// FUNCTION: MW23DFX 0x10002aa0
void ToggleFullScreen(void)
{
	MechS32 i;
	RefreshMode* mode;
	DisplayBackend* backend;

	if (g_refreshModeFallback) {
		return;
	}

	if (!g_shouldToggleFullscreen && g_refreshModeWidth >= g_desktopWidth && g_refreshModeHeight >= g_desktopHeight) {
		return;
	}

	DebugPrint(TEXT_LINE("ToggleFullScreen(1): pause_timer(TRUE)"));
	PauseTimer(0x80, TRUE);

	if (g_currentDisplayBackend->m_windowMode == c_windowModeFullscreen) {
#ifdef MW2_FULLSCREEN_SAVES_MODE
		g_fastestRefreshMode = g_currentRefreshMode;
#endif
		for (i = 0; i < 6; i++) {
			mode = g_refreshModes[i];
			backend = g_displayBackends[mode->m_backend];
			if (mode->m_available && backend->m_windowMode == c_windowModeWindowed) {
				break;
			}
		}

		if (i == 6) {
#ifdef MW2_GHOST_BEAR_TITLES
			// (sic)
			ShowMessage(
				TEXT_LINE("Ghost Bear Legacy cannot run in a window in the current resolution on your video hardware")
			);
#else
			ShowMessage(
				TEXT_LINE("MechWarrior2 cannot run in a window in the current resolution on your video hardware")
			);
#endif
			if (!g_simPaused) {
				DebugPrint(TEXT_LINE("ToggleFullScreen(2): pause_timer(FALSE)"));
				PauseTimer(0x80, FALSE);
			}
			return;
		}
	}
	else {
		if (!g_fastestRefreshMode) {
#ifdef MW2_GHOST_BEAR_TITLES
			// (sic)
			ShowMessage(
				"Ghost Bear' Legacy cannot support full screen mode in the current resolution on your video hardware"
			);
#else
			ShowMessage(
				"MechWarrior2 cannot support full screen mode in the current resolution on your video hardware"
			);
#endif
			if (!g_simPaused) {
				DebugPrint(TEXT_LINE("ToggleFullScreen(3): pause_timer(FALSE)"));
				PauseTimer(0x80, FALSE);
			}
			return;
		}
		else {
			mode = g_fastestRefreshMode;
		}

		GetWindowRect(g_gameWindow, &g_windowedRect);
		g_windowedRect.right -= g_windowedRect.left;
		g_windowedRect.bottom -= g_windowedRect.top;
	}

	g_currentRefreshMode->m_end();
	g_currentRefreshMode = mode;
	g_currentRefreshMode->m_begin(g_refreshModeBuffer, g_refreshModeWidth, g_refreshModeHeight);
	g_currentDisplayBackend->m_setPalette(0, 0x100, g_paletteColors, TRUE);
	g_reclipCursor = 1;
	if (!g_simPaused) {
		DebugPrint(TEXT_LINE("ToggleFullScreen(4): pause_timer(FALSE)"));
		PauseTimer(0x80, FALSE);
	}
}

// A top-down 8-bit DIB of p_width x p_height.
// FUNCTION: MW2 0x100775b4
// FUNCTION: MW2GBL 0x1002f792
// FUNCTION: MW2PREMERCS 0x1001de91
// FUNCTION: MW2MATROX 0x1007b63a
// FUNCTION: MW23DFX 0x10002c70
// FUNCTION: MW2ATI 0x100029b0
// FUNCTION: MW2S3 0x1002f190
// FUNCTION: MW2POWERVR 0x10002760
void InitBitmapInfo(MechS32 p_width, MechS32 p_height)
{
	g_bitmapInfo.m_header.biSize = sizeof(BITMAPINFOHEADER);
	g_bitmapInfo.m_header.biWidth = p_width;
	g_bitmapInfo.m_header.biHeight = p_height * -1;
	g_bitmapInfo.m_header.biPlanes = 1;
	g_bitmapInfo.m_header.biBitCount = 8;
	g_bitmapInfo.m_header.biCompression = BI_RGB;
	g_bitmapInfo.m_header.biSizeImage = 0;
	g_bitmapInfo.m_header.biClrUsed = 0;
	g_bitmapInfo.m_header.biXPelsPerMeter = 0;
	g_bitmapInfo.m_header.biYPelsPerMeter = 0;
	g_bitmapInfo.m_header.biClrImportant = 0;
}

// Restyles the game window for p_backend's window mode, and sets g_windowMode: a mode that
// covers the whole screen counts as fullscreen.
// FUNCTION: MW2 0x1007762f
// FUNCTION: MW2GBL 0x1002f80d
// FUNCTION: MW2PREMERCS 0x1001df0c
// FUNCTION: MW2MATROX 0x1007b6b5
// FUNCTION: MW23DFX 0x10002cc0
// FUNCTION: MW2ATI 0x10002a00
// FUNCTION: MW2S3 0x1002f1e0
// FUNCTION: MW2POWERVR 0x100027b0
void AdjustWindowSize(DisplayBackend* p_backend)
{
	if (p_backend == NULL) {
		return;
	}

	if (p_backend->m_windowMode == c_windowModeWindowed) {
		SetWindowLong(g_gameWindow, GWL_STYLE, p_backend->m_style | WS_VISIBLE);
	}
	else {
		SetWindowLong(g_gameWindow, GWL_STYLE, (p_backend->m_style | WS_VISIBLE) & ~WS_SYSMENU);
	}

	if (p_backend->m_windowMode == c_windowModeWindowed) {
		if (g_currentDisplayBackend != NULL && g_currentDisplayBackend->m_id == c_displayBackendDirectDraw) {
			g_windowedSwitchTime = timeGetTime();
			g_windowedSwitchDeadline = g_windowedSwitchTime + 3000;
			g_windowedSwitchPending = 1;
		}

		SetWindowPos(
			g_gameWindow,
			HWND_NOTOPMOST,
			g_windowedRect.left,
			g_windowedRect.top,
			g_windowedRect.right,
			g_windowedRect.bottom,
			SWP_NOACTIVATE
		);
		if (g_simPaused && !GetMenuSlotState(4)) {
			while (ShowCursor(TRUE) < 0) {
			}
		}
	}
	else if (g_simPaused) {
		while (ShowCursor(FALSE) >= 0) {
		}
	}

	if (GetSystemMetrics(SM_CXSCREEN) <= g_refreshModeWidth && GetSystemMetrics(SM_CYSCREEN) <= g_refreshModeHeight) {
		g_windowMode = c_windowModeFullscreen;
	}
	else {
		g_windowMode = p_backend->m_windowMode;
	}
}

// Copies p_count colors of g_paletteColors from p_first into p_palette.
// FUNCTION: MW2 0x100777a5
// FUNCTION: MW2GBL 0x1002f983
// FUNCTION: MW2PREMERCS 0x1001e082
// FUNCTION: MW2MATROX 0x1007b82b
// FUNCTION: MW23DFX 0x10002de0
MechS32 GetPaletteColors(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette)
{
	MechS32 i;

	if (p_palette == NULL || p_first < 0 || p_first > 0xff || p_count <= 0 || p_count > 0x100 - p_first) {
		return -1;
	}

	for (i = 0; i < p_count; i++) {
		p_palette[i] = g_paletteColors[p_first + i];
	}

	return 0;
}
