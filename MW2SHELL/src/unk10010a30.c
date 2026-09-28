#include "decomp.h"
#include "drawbitmapinfo.h"
#include "drawmode.h"
#include "drawmodeextension.h"
#include "palettecolor.h"
#include "pixelbuffer.h"
#include "types.h"

#include <ddraw.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The draw mode manager. The DirectDraw back end lives in unk100114f0.c, the DisplayDib one in
// dispdib.c, the GDI one in gdi.c.

extern MechS32 g_unk0x1006a9d8;
extern undefined4 g_unk0x10071d48;

undefined4 FUN_1003bf90(MechS32 p_unk0x00);
void DebugPrint(const MechChar* p_format, ...);

extern DrawModeExtension g_dispDibDrawModeExtension;
extern DrawModeExtension g_gdiDrawModeExtension;
void FUN_10010f83();
void FUN_10015c90(const MechChar* p_format, ...);

// The original declares a function the shell never defines: the linker binds the calls to the
// variable of the same name (gdi.c), so they land in BSS. The debug strings call it pause_timer.
void PauseTimer(MechS32 p_flags, MechS32 p_pause);
void AdjustWindowSize(DrawModeExtension* p_extension);
extern DrawMode g_dispDibDrawMode;
extern DrawMode g_gdiDrawMode;

extern DrawModeExtension g_unk0x10063230;
extern DrawMode g_unk0x10063258;
extern DrawMode g_unk0x10063280;
extern DrawMode g_unk0x100632a8;
extern DrawMode g_unk0x100632d0;

// Indexed by DrawModeExtension::m_id.
// GLOBAL: MW2SHELL 0x10062ca0
DrawModeExtension* g_drawModeExtensions[3] = {&g_unk0x10063230, &g_dispDibDrawModeExtension, &g_gdiDrawModeExtension};

// GLOBAL: MW2SHELL 0x10062cb0
DrawMode* g_drawModes[6] = {
	&g_unk0x10063258,
	&g_unk0x10063280,
	&g_unk0x100632a8,
	&g_unk0x100632d0,
	&g_dispDibDrawMode,
	&g_gdiDrawMode,
};

// GLOBAL: MW2SHELL 0x10062cc8
DrawModeExtension* g_currentDrawModeExtension = NULL;

// GLOBAL: MW2SHELL 0x10062ccc
DrawMode* g_currentDrawMode = NULL;

// The fastest draw mode found by the profiling, the one to return to from windowed mode.
// GLOBAL: MW2SHELL 0x10062cd0
DrawMode* g_unk0x10062cd0 = NULL;

// GLOBAL: MW2SHELL 0x10062cd4
MechS32 g_drawModeFallback = FALSE;

// The last draw mode the profiling tries.
// GLOBAL: MW2SHELL 0x10062cd8
MechS32 g_lastProfiledDrawMode = 4;

// The buffer the active draw mode renders into.
// GLOBAL: MW2SHELL 0x10062cdc
PixelBuffer* g_unk0x10062cdc = NULL;

// GLOBAL: MW2SHELL 0x10062ce0
PaletteColor g_paletteColors[0x100] = {0};

// The DIB bits of the GDI and DisplayDib back ends, restored into g_unk0x10062cdc by
// m_acquireFramebuffer.
// GLOBAL: MW2SHELL 0x10062fe0
undefined* g_unk0x10062fe0 = NULL;

// GLOBAL: MW2SHELL 0x10062ffc
MechS32 g_nWindowMode = 0;

// GLOBAL: MW2SHELL 0x10063000
MechS32 g_unk0x10063000 = 1;

// GLOBAL: MW2SHELL 0x10063004
MechS32 g_profileFrame = 0;

// GLOBAL: MW2SHELL 0x1007cc90
static LARGE_INTEGER g_profileStart;

// GLOBAL: MW2SHELL 0x100965d0
MechU32 g_unk0x100965d0;

// GLOBAL: MW2SHELL 0x100965d8
MechS32 g_windowHeight;

// GLOBAL: MW2SHELL 0x100965dc
MechS32 g_windowWidth;

// GLOBAL: MW2SHELL 0x100965e0
HINSTANCE g_pModule;

// GLOBAL: MW2SHELL 0x100965e4
MechS32 g_unk0x100965e4;

// GLOBAL: MW2SHELL 0x100965e8
MechU32 g_unk0x100965e8;

// GLOBAL: MW2SHELL 0x100965ec
HWND g_pWnd;

// GLOBAL: MW2SHELL 0x100965f0
HMENU g_windowMenu;

// The shell window's position and size in windowed mode (SetWindowPos arguments, not corners).
// GLOBAL: MW2SHELL 0x10096a50
RECT g_unk0x10096a50;

// GLOBAL: MW2SHELL 0x10096a60
DrawBitmapInfo g_bitmapInfo;

// GLOBAL: MW2SHELL 0x10096e88
MechS32 g_unk0x10096e88;

// GLOBAL: MW2SHELL 0x10096e8c
MechS32 g_drawModeWidth;

// GLOBAL: MW2SHELL 0x10096e90
MechS32 g_drawModeHeight;

// Switches to draw mode p_mode (-1: the first), falling through the later modes while a mode
// is unavailable if p_allowFallback is set. The window covers the screen unless p_width x
// p_height is smaller.
// Stack-slot permutation: mode, extension, screenWidth and unused. The original also compares
// the screen size against p_width and p_height the other way round.
// FUNCTION: MW2SHELL 0x10010a30
MechS32 InitDrawMode(
	MechS32 p_mode,
	MechS32 p_allowFallback,
	PixelBuffer* p_buffer,
	MechS32 p_width,
	MechS32 p_height,
	MechS32 p_menu
)
{
	MechS32 unused;
	MechS32 screenHeight;
	DrawMode* mode;
	DrawModeExtension* extension;
	MechS32 screenWidth;
	RECT rect;

	if (p_mode == -1) {
		mode = g_drawModes[0];
	}
	else if (p_mode >= 0 && p_mode < 6) {
		mode = g_drawModes[p_mode];
	}
	else {
		return 0;
	}

	extension = g_drawModeExtensions[mode->m_extension];
	g_drawModeFallback = p_allowFallback;
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	g_unk0x10096a50.left = 0;
	g_unk0x10096a50.top = 0;
	g_unk0x10096a50.right = p_width;
	g_unk0x10096a50.bottom = p_height;
	if (screenWidth <= p_width && screenHeight <= p_height) {
		g_gdiDrawModeExtension.m_style = WS_POPUP;
		g_nWindowMode = c_windowModeFullscreen;
	}
	else {
		g_gdiDrawModeExtension.m_style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		AdjustWindowRect(&g_unk0x10096a50, g_gdiDrawModeExtension.m_style, p_menu);
		g_unk0x10096a50.right -= g_unk0x10096a50.left;
		g_unk0x10096a50.bottom -= g_unk0x10096a50.top;
		g_unk0x10096a50.top = (screenHeight - g_unk0x10096a50.bottom) / 2;
		g_unk0x10096a50.left = (screenWidth - g_unk0x10096a50.right) / 2;
		g_nWindowMode = extension->m_windowMode;
	}

	if (extension->m_windowMode == c_windowModeFullscreen) {
		rect.left = 0;
		rect.top = 0;
		rect.right = GetSystemMetrics(SM_CXSCREEN);
		rect.bottom = GetSystemMetrics(SM_CYSCREEN);
		unused = 8;
	}
	else {
		rect = g_unk0x10096a50;
		unused = 0;
	}

	if (g_unk0x10063000) {
		g_drawModeWidth = p_width;
		g_drawModeHeight = p_height;
		AdjustWindowSize(extension);
		g_currentDrawModeExtension = extension;
	}
	else if (mode != g_currentDrawMode || g_drawModeWidth != p_width || g_drawModeHeight != p_height) {
		AdjustWindowSize(extension);
		g_currentDrawMode->m_end();
	}

	g_currentDrawMode = mode;
	g_drawModeWidth = p_width;
	g_drawModeHeight = p_height;
	g_unk0x10096e88 = p_height * p_width;
	g_unk0x10062cdc = p_buffer;

	while (g_currentDrawMode->m_available && g_currentDrawMode->m_begin(p_buffer, p_width, p_height)) {
		DebugPrint("RefreshMode %d not available\n", g_currentDrawMode->m_index);
		g_currentDrawMode->m_available = FALSE;
		if (!g_drawModeFallback || g_currentDrawMode->m_index == 5) {
			return 0;
		}

		g_currentDrawMode = g_drawModes[g_currentDrawMode->m_index + 1];
	}

	if (g_unk0x10063000 && extension->m_id != c_drawModeExtensionGdi) {
		ShowWindow(g_pWnd, SW_SHOWDEFAULT);
		UpdateWindow(g_pWnd);
	}

	g_unk0x10063000 = 0;
	return 1;
}

// FUNCTION: MW2SHELL 0x10010d49
void FUN_10010d49()
{
	if (g_currentDrawMode != NULL) {
		g_currentDrawMode->m_end();
	}
	if (g_currentDrawModeExtension != NULL) {
		g_currentDrawModeExtension->m_end();
	}

	g_unk0x10063000 = 1;
}

// Called once a frame while the draw modes are profiled: times four frames of the current mode,
// then moves to the next one; after the last, FUN_10010f83 picks the fastest.
// FUNCTION: MW2SHELL 0x10010d88
void FUN_10010d88()
{
	LARGE_INTEGER end;

	if (g_currentDrawMode->m_index > g_lastProfiledDrawMode) {
		g_drawModeFallback = FALSE;
	}
	else if (g_profileFrame == 1) {
		if (!QueryPerformanceCounter(&g_profileStart)) {
			g_drawModeFallback = FALSE;
			return;
		}

		g_profileFrame++;
	}
	else if (g_profileFrame == 5) {
		if (!QueryPerformanceCounter(&end)) {
			g_drawModeFallback = FALSE;
			return;
		}

		if (end.HighPart != g_profileStart.HighPart) {
			g_currentDrawMode->m_profileTime = ~g_profileStart.LowPart + end.LowPart + 1;
		}
		else {
			g_currentDrawMode->m_profileTime = end.LowPart - g_profileStart.LowPart;
		}

		DebugPrint(
			"Refresh mode %d start=(%u,%d) end=(%u,%d) diff=%u\n",
			g_currentDrawMode->m_index,
			g_profileStart.LowPart,
			g_profileStart.HighPart,
			end.LowPart,
			end.HighPart,
			g_currentDrawMode->m_profileTime
		);
		if (g_currentDrawMode->m_index == g_lastProfiledDrawMode) {
			g_drawModeFallback = FALSE;
		}
		else {
			while (g_currentDrawMode->m_index < g_lastProfiledDrawMode) {
				g_currentDrawMode->m_end();
				g_currentDrawMode = g_drawModes[g_currentDrawMode->m_index + 1];
				if (g_currentDrawMode->m_available &&
					!g_currentDrawMode->m_begin(g_unk0x10062cdc, g_drawModeWidth, g_drawModeHeight)) {
					g_profileFrame = 0;
					break;
				}
				else {
					DebugPrint("Refresh mode %d not available\n", g_currentDrawMode->m_index);
					g_currentDrawMode->m_available = FALSE;
					if (g_currentDrawMode->m_index == g_lastProfiledDrawMode) {
						g_drawModeFallback = FALSE;
					}
				}
			}
		}

		if (!g_drawModeFallback) {
			FUN_10010f83();
		}
	}
	else {
		g_profileFrame++;
	}
}

// Switches to the available draw mode with the shortest profile time.
// Operand order: the original compares best != g_currentDrawMode with g_currentDrawMode loaded
// first.
// FUNCTION: MW2SHELL 0x10010f83
void FUN_10010f83()
{
	MechS32 i;
	DrawMode* best;

	best = NULL;
	for (i = 0; i < 6; i++) {
		if (g_drawModes[i]->m_available && g_drawModes[i]->m_profileTime > 0 &&
			(best == NULL || g_drawModes[i]->m_profileTime < best->m_profileTime)) {
			best = g_drawModes[i];
		}
	}

	if (best != g_currentDrawMode) {
		g_currentDrawMode->m_end();
		g_currentDrawMode = best;
		g_currentDrawMode->m_begin(g_unk0x10062cdc, g_drawModeWidth, g_drawModeHeight);
	}

	g_unk0x10062cd0 = g_currentDrawMode;
	DebugPrint(
		"Refresh mode %d selected with profile time: %u\n",
		g_currentDrawMode->m_index,
		g_currentDrawMode->m_profileTime
	);
}

// Switches between fullscreen and the first available windowed draw mode, or back to the
// fullscreen mode last selected.
// Not 100%: the stack slots of i, mode and extension are permuted.
// FUNCTION: MW2SHELL 0x10011071
void ToggleFullScreen()
{
	MechS32 i;
	DrawMode* mode;
	DrawModeExtension* extension;

	if (g_drawModeFallback) {
		return;
	}

	DebugPrint("ToggleFullScreen(1): pause_timer(TRUE)");
	PauseTimer(0x80, TRUE);

	if (g_currentDrawModeExtension->m_windowMode == c_windowModeFullscreen) {
		for (i = 0; i < 6; i++) {
			mode = g_drawModes[i];
			extension = g_drawModeExtensions[mode->m_extension];
			if (mode->m_available && extension->m_windowMode == c_windowModeWindowed) {
				break;
			}
		}

		if (i == 6) {
			FUN_10015c90("MechWarrior2 cannot run in a window in the current resolution on your video hardware");
			if (!g_unk0x1006a9d8) {
				DebugPrint("ToggleFullScreen(2): pause_timer(FALSE)");
				PauseTimer(0x80, FALSE);
			}
			return;
		}
	}
	else {
		if (!g_unk0x10062cd0) {
			FUN_10015c90(
				"MechWarrior2 cannot support full screen mode in the current resolution on your video hardware"
			);
			if (!g_unk0x1006a9d8) {
				DebugPrint("ToggleFullScreen(3): pause_timer(FALSE)");
				PauseTimer(0x80, FALSE);
			}
			return;
		}
		else {
			mode = g_unk0x10062cd0;
		}

		GetWindowRect(g_pWnd, &g_unk0x10096a50);
		g_unk0x10096a50.right -= g_unk0x10096a50.left;
		g_unk0x10096a50.bottom -= g_unk0x10096a50.top;
	}

	g_currentDrawMode->m_end();
	g_currentDrawMode = mode;
	g_currentDrawMode->m_begin(g_unk0x10062cdc, g_drawModeWidth, g_drawModeHeight);
	g_currentDrawModeExtension->m_setPalette(0, 0x100, g_paletteColors, TRUE);
	g_unk0x10071d48 = 1;
	if (!g_unk0x1006a9d8) {
		DebugPrint("ToggleFullScreen(4): pause_timer(FALSE)");
		PauseTimer(0x80, FALSE);
	}
}

// A top-down 8-bit DIB of p_width x p_height.
// FUNCTION: MW2SHELL 0x1001125f
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

// Restyles the shell window for p_extension's window mode, and sets g_nWindowMode: a mode that
// covers the whole screen counts as fullscreen.
// FUNCTION: MW2SHELL 0x100112da
void AdjustWindowSize(DrawModeExtension* p_extension)
{
	if (p_extension == NULL) {
		return;
	}

	if (p_extension->m_windowMode == c_windowModeWindowed) {
		SetWindowLong(g_pWnd, GWL_STYLE, p_extension->m_style | WS_VISIBLE);
	}
	else {
		SetWindowLong(g_pWnd, GWL_STYLE, (p_extension->m_style | WS_VISIBLE) & ~WS_SYSMENU);
	}

	if (p_extension->m_windowMode == c_windowModeWindowed) {
		if (g_currentDrawModeExtension != NULL && g_currentDrawModeExtension->m_id == c_drawModeExtensionDirectDraw) {
			g_unk0x100965d0 = timeGetTime();
			g_unk0x100965e8 = g_unk0x100965d0 + 3000;
			g_unk0x100965e4 = 1;
		}

		SetWindowPos(
			g_pWnd,
			HWND_NOTOPMOST,
			g_unk0x10096a50.left,
			g_unk0x10096a50.top,
			g_unk0x10096a50.right,
			g_unk0x10096a50.bottom,
			SWP_NOACTIVATE
		);
		if (g_unk0x1006a9d8 && !FUN_1003bf90(4)) {
			while (ShowCursor(TRUE) < 0) {
			}
		}
	}
	else if (g_unk0x1006a9d8) {
		while (ShowCursor(FALSE) >= 0) {
		}
	}

	if (GetSystemMetrics(SM_CXSCREEN) <= g_drawModeWidth && GetSystemMetrics(SM_CYSCREEN) <= g_drawModeHeight) {
		g_nWindowMode = c_windowModeFullscreen;
	}
	else {
		g_nWindowMode = p_extension->m_windowMode;
	}
}

// Operand order: the original compares i < p_count as `cmp [p_count], eax` and adds p_first + i
// with p_first loaded first; here both come out the other way round since the draw mode tables
// joined this unit, and still since the DirectDraw back end left it (one declaration-order
// attempt didn't flip them either time).
// FUNCTION: MW2SHELL 0x10011450
MechS32 FUN_10011450(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette)
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
