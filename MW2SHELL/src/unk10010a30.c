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

// The draw mode manager and the DirectDraw back end. The DisplayDib back end lives in dispdib.cpp,
// the GDI one in gdi.c.

void DdrawStop();
void DdrawDestroySurfaces();
MechS32 DdrawCreateSurfaces(MechS32 p_width, MechS32 p_height);
MechS32 DdrawInit(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height);
MechS32 DdrawFlip();
MechS32 DdrawBlit();
MechS32 DdrawBlitFlip();
MechS32 DdrawStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 DdrawStretchBlit320(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 DdrawBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 DdrawFill(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechU32 p_color);
MechS32 DdrawWritePaletteEntries(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors);
MechS32 DdrawWritePaletteGamma(PaletteColor* p_palette);
MechS32 DdrawPaletteFade(PaletteColor* p_palette, MechS32 p_steps);
MechS32 DdrawLockBuffer();

extern HWND g_pWnd;
extern MechS32 g_nWindowMode;
extern MechS32 g_unk0x1006a9d8;

undefined4 FUN_1003bf90(MechS32 p_unk0x00);
void DebugPrint(const MechChar* p_format, ...);
void CopyPaletteColorWithBrightness(PaletteColor* p_src, PaletteColor* p_dst);

extern PaletteColor g_paletteColorsPreBrightness[0x100];

extern DrawModeExtension g_dispDibDrawModeExtension;
extern DrawModeExtension g_gdiDrawModeExtension;
void FUN_10010f83();
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

// GLOBAL: MW2SHELL 0x10063000
MechS32 g_unk0x10063000 = 1;

// GLOBAL: MW2SHELL 0x10063004
MechS32 g_profileFrame = 0;

// The DirectDraw back end (its functions are named after their debug messages, "DDRAW_Flip" and
// so on): the frame is drawn into g_ddrawBuffer, an offscreen surface (or the back buffer when
// the draw mode flips), then blitted or flipped to the primary surface.
// GLOBAL: MW2SHELL 0x10063208
LPDIRECTDRAW g_ddraw = NULL;

// GLOBAL: MW2SHELL 0x1006320c
LPDIRECTDRAWSURFACE g_ddrawPrimary = NULL;

// GLOBAL: MW2SHELL 0x10063210
LPDIRECTDRAWSURFACE g_ddrawBack = NULL;

// GLOBAL: MW2SHELL 0x10063214
LPDIRECTDRAWSURFACE g_ddrawBuffer = NULL;

// The second back buffer of the 320x200 mode, which stretches the frame into it and flips.
// GLOBAL: MW2SHELL 0x10063218
LPDIRECTDRAWSURFACE g_ddrawStretch = NULL;

// GLOBAL: MW2SHELL 0x1006321c
undefined4 g_unk0x1006321c = 0;

// GLOBAL: MW2SHELL 0x10063220
LPDIRECTDRAWPALETTE g_ddrawPalette = NULL;

// TRUE while g_ddrawBuffer is locked for drawing.
// GLOBAL: MW2SHELL 0x10063224
MechS32 g_ddrawLocked = FALSE;

// GLOBAL: MW2SHELL 0x10063228
MechS32 g_ddrawInitialized = FALSE;

// GLOBAL: MW2SHELL 0x10063230
DrawModeExtension g_unk0x10063230 = {
	c_drawModeExtensionDirectDraw,
	c_windowModeFullscreen,
	WS_POPUP,
	DdrawInit,
	(MechS32 (*)()) DdrawStop,
	DdrawWritePaletteEntries,
	DdrawWritePaletteGamma,
	DdrawPaletteFade,
	DdrawLockBuffer,
	0
};

// GLOBAL: MW2SHELL 0x10063258
DrawMode g_unk0x10063258 =
	{0, 0, 1, 0, DdrawInit, (MechS32 (*)()) DdrawDestroySurfaces, DdrawFlip, DdrawBlitRect, DdrawStretchBlit};

// GLOBAL: MW2SHELL 0x10063280
DrawMode g_unk0x10063280 =
	{1, 0, 1, 0, DdrawInit, (MechS32 (*)()) DdrawDestroySurfaces, DdrawBlitFlip, DdrawBlitRect, DdrawStretchBlit};

// GLOBAL: MW2SHELL 0x100632a8
DrawMode g_unk0x100632a8 =
	{2, 0, 1, 0, DdrawInit, (MechS32 (*)()) DdrawDestroySurfaces, DdrawBlit, DdrawBlitRect, DdrawStretchBlit};

// GLOBAL: MW2SHELL 0x100632d0
DrawMode g_unk0x100632d0 =
	{3, 0, 1, 0, DdrawInit, (MechS32 (*)()) DdrawDestroySurfaces, DdrawBlit, DdrawBlitRect, DdrawStretchBlit};

// GLOBAL: MW2SHELL 0x1007cc90
LARGE_INTEGER g_profileStart;

// GLOBAL: MW2SHELL 0x10096870
RECT g_ddrawScreenRect;

// GLOBAL: MW2SHELL 0x10096880
HRESULT g_ddrawResult;

// GLOBAL: MW2SHELL 0x10096890
DDBLTFX g_ddrawBltFx;

// GLOBAL: MW2SHELL 0x10096900
DDSURFACEDESC g_ddrawPrimaryDesc;

// GLOBAL: MW2SHELL 0x10096970
DDSURFACEDESC g_ddrawBackDesc;

// GLOBAL: MW2SHELL 0x100969dc
DDSCAPS g_ddrawCaps;

// GLOBAL: MW2SHELL 0x100969e0
DDSURFACEDESC g_ddrawBufferDesc;

// GLOBAL: MW2SHELL 0x100965d0
MechU32 g_unk0x100965d0;

// GLOBAL: MW2SHELL 0x100965e4
MechS32 g_unk0x100965e4;

// GLOBAL: MW2SHELL 0x100965e8
MechU32 g_unk0x100965e8;

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
// joined this unit (one declaration-order attempt didn't flip them).
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

// Releases the pending lock on g_ddrawBuffer.
__inline static HRESULT DdrawUnlock()
{
	if (g_ddrawLocked) {
		g_ddrawResult = IDirectDrawSurface_Unlock(g_ddrawBuffer, g_ddrawBufferDesc.lpSurface);
		g_unk0x10062cdc->m_pixels = NULL;
		g_ddrawLocked = FALSE;
		return g_ddrawResult;
	}
	else {
		return DD_OK;
	}
}

// Restores the primary surface and g_ddrawBuffer when they were lost.
__inline static HRESULT DdrawRestore()
{
	if (IDirectDrawSurface_IsLost(g_ddrawPrimary) == DDERR_SURFACELOST) {
		g_ddrawResult = IDirectDrawSurface_Restore(g_ddrawPrimary);
		if (IDirectDrawSurface_IsLost(g_ddrawBuffer) == DDERR_SURFACELOST) {
			g_ddrawResult = IDirectDrawSurface_Restore(g_ddrawBuffer);
		}
		return g_ddrawResult;
	}
	else {
		return DD_OK;
	}
}

// FUNCTION: MW2SHELL 0x100114f0
void DdrawStop()
{
	g_ddrawInitialized = FALSE;
	if (g_ddraw != NULL) {
		DdrawDestroySurfaces();
		IDirectDraw_SetCooperativeLevel(g_ddraw, g_pWnd, DDSCL_NORMAL);
		IDirectDraw_RestoreDisplayMode(g_ddraw);
		if (g_ddrawPalette != NULL) {
			IDirectDrawPalette_Release(g_ddrawPalette);
			g_ddrawPalette = NULL;
		}
		IDirectDraw_Release(g_ddraw);
		g_ddraw = NULL;
	}
}

// FUNCTION: MW2SHELL 0x10011580
void DdrawDestroySurfaces()
{
	g_unk0x10062cdc->m_pixels = NULL;
	if (g_ddrawPrimary != NULL) {
		IDirectDrawSurface_Release(g_ddrawPrimary);
		g_ddrawPrimary = NULL;
		g_ddrawBack = NULL;
		g_ddrawStretch = NULL;
		if (g_currentDrawMode->m_index == 0) {
			g_ddrawBuffer = NULL;
		}
	}

	if (g_ddrawBuffer != NULL && (g_drawModeWidth != 320 || g_drawModeHeight != 200)) {
		IDirectDrawSurface_Release(g_ddrawBuffer);
		g_ddrawBuffer = NULL;
	}
}

// Creates the primary surface and g_ddrawBuffer: the 320x200 mode flips through two back
// buffers, the flipping modes (0 and 1) through one, and the blitting modes (2 and 3) draw into
// an offscreen surface.
// FUNCTION: MW2SHELL 0x10011630
MechS32 DdrawCreateSurfaces(MechS32 p_width, MechS32 p_height)
{
	MechS32 flip = FALSE;

	if (p_width == 320 && p_height == 200) {
		if (g_currentDrawMode->m_index == 2 || g_currentDrawMode->m_index == 3) {
			return -1;
		}

		g_ddrawPrimaryDesc.dwSize = sizeof(DDSURFACEDESC);
		g_ddrawPrimaryDesc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		g_ddrawPrimaryDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		g_ddrawPrimaryDesc.dwBackBufferCount = 2;
		g_ddrawResult = IDirectDraw_CreateSurface(g_ddraw, &g_ddrawPrimaryDesc, &g_ddrawPrimary, NULL);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_CreateSurfaces CreateSurface(Primary): %d\n", g_ddrawResult & 0xfff);
			return g_ddrawResult;
		}

		g_ddrawCaps.dwCaps = DDSCAPS_BACKBUFFER;
		g_ddrawResult = IDirectDrawSurface_GetAttachedSurface(g_ddrawPrimary, &g_ddrawCaps, &g_ddrawBack);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_CreateSurfaces GetAttachedSurface(Back): %d\n", g_ddrawResult & 0xfff);
			return g_ddrawResult;
		}

		g_ddrawBuffer = g_ddrawBack;
		g_ddrawCaps.dwCaps = DDSCAPS_FLIP;
		g_ddrawResult = IDirectDrawSurface_GetAttachedSurface(g_ddrawPrimary, &g_ddrawCaps, &g_ddrawStretch);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_CreateSurfaces GetAttachedSurface(Stretch): %d\n", g_ddrawResult & 0xfff);
			return g_ddrawResult;
		}
	}
	else {
		if (g_currentDrawMode->m_index == 0 || g_currentDrawMode->m_index == 1) {
			flip = TRUE;
		}

		g_ddrawPrimaryDesc.dwSize = sizeof(DDSURFACEDESC);
		g_ddrawPrimaryDesc.dwFlags = DDSD_CAPS;
		g_ddrawPrimaryDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
		if (flip) {
			g_ddrawPrimaryDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
			g_ddrawPrimaryDesc.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
			g_ddrawPrimaryDesc.dwBackBufferCount = 2;
		}

		g_ddrawResult = IDirectDraw_CreateSurface(g_ddraw, &g_ddrawPrimaryDesc, &g_ddrawPrimary, NULL);
		if (g_ddrawResult != DD_OK) {
			if (flip) {
				DebugPrint("DDRAW_CreateSurfaces primary failed; trying 1 back buffer\n");
				g_ddrawPrimaryDesc.dwBackBufferCount = 1;
				g_ddrawResult = IDirectDraw_CreateSurface(g_ddraw, &g_ddrawPrimaryDesc, &g_ddrawPrimary, NULL);
			}
			else if (g_currentDrawMode->m_index == 3) {
				DebugPrint("DDRAW_CreateSurfaces primary failed; trying system memory\n");
				g_ddrawPrimaryDesc.ddsCaps.dwCaps &= ~DDSCAPS_VIDEOMEMORY;
				g_ddrawResult = IDirectDraw_CreateSurface(g_ddraw, &g_ddrawPrimaryDesc, &g_ddrawPrimary, NULL);
			}
		}

		if (g_ddrawResult != DD_OK) {
			return g_ddrawResult;
		}

		if (flip) {
			g_ddrawCaps.dwCaps = DDSCAPS_BACKBUFFER;
			g_ddrawResult = IDirectDrawSurface_GetAttachedSurface(g_ddrawPrimary, &g_ddrawCaps, &g_ddrawBack);
			if (g_ddrawResult != DD_OK) {
				DebugPrint("DDRAW_CreateSurfaces GetAttachedSurface(): %d\n", g_ddrawResult & 0xfff);
				return g_ddrawResult;
			}
		}

		if (g_currentDrawMode->m_index == 0) {
			g_ddrawBuffer = g_ddrawBack;
		}
		else {
			g_ddrawBufferDesc.dwSize = sizeof(DDSURFACEDESC);
			g_ddrawBufferDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
			g_ddrawBufferDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			g_ddrawBufferDesc.dwWidth = p_width;
			g_ddrawBufferDesc.dwHeight = p_height;
			if (g_currentDrawMode->m_index == 2) {
				g_ddrawBufferDesc.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
			}
			else {
				g_ddrawBufferDesc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
			}

			g_ddrawResult = IDirectDraw_CreateSurface(g_ddraw, &g_ddrawBufferDesc, &g_ddrawBuffer, NULL);
			if (g_ddrawResult != DD_OK) {
				DebugPrint("DDRAW_CreateSurfaces CreateSurface(offscreen): %d\n", g_ddrawResult & 0xfff);
			}
		}
	}

	return g_ddrawResult;
}

// Stack-slot permutation: caps and entries.
// FUNCTION: MW2SHELL 0x10011a15
MechS32 DdrawInit(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height)
{
	LPPALETTEENTRY entries;
	DDCAPS caps;

	if (g_currentDrawModeExtension->m_id != c_drawModeExtensionDirectDraw) {
		g_currentDrawModeExtension->m_end();
		g_currentDrawModeExtension = g_drawModeExtensions[c_drawModeExtensionDirectDraw];
		if (g_currentDrawModeExtension->m_windowMode != g_nWindowMode) {
			AdjustWindowSize(g_currentDrawModeExtension);
		}
	}

	if (g_ddraw == NULL) {
		g_ddrawResult = DirectDrawCreate(NULL, &g_ddraw, NULL);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_Init DirectDrawCreate(): %d\n", g_ddrawResult & 0xfff);
			return 1;
		}

		g_ddrawResult = IDirectDraw_SetCooperativeLevel(g_ddraw, g_pWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | 0x40);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_Init SetCooperativeLevel(): %d\n", g_ddrawResult & 0xfff);
			DdrawStop();
			return 1;
		}

		g_ddrawResult = IDirectDraw_SetDisplayMode(g_ddraw, p_width, p_height, 8);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_Init SetDisplayMode(): %d\n", g_ddrawResult & 0xfff);
			DdrawStop();
			return 1;
		}
	}

	memset(&caps, 0, sizeof(caps));
	caps.dwSize = sizeof(caps);
	if (IDirectDraw_GetCaps(g_ddraw, &caps, NULL) == DD_OK && (caps.dwCaps & DDCAPS_BANKSWITCHED) &&
		(g_currentDrawMode->m_index == 0 || g_currentDrawMode->m_index == 2)) {
		DebugPrint("DDRAW_Init: bank-switched video card.\n");
		return 1;
	}

	if (DdrawCreateSurfaces(p_width, p_height)) {
		DdrawDestroySurfaces();
		return 1;
	}

	g_ddrawBltFx.dwSize = sizeof(DDBLTFX);
	if (DdrawFill(0, 0, p_width - 1, p_height - 1, 0)) {
		DdrawDestroySurfaces();
		return 1;
	}

	if (g_currentDrawMode->m_flip()) {
		DdrawDestroySurfaces();
		return 1;
	}

	memset(&g_ddrawPrimaryDesc, 0, sizeof(DDSURFACEDESC));
	g_ddrawPrimaryDesc.dwSize = sizeof(DDSURFACEDESC);
	g_ddrawResult = IDirectDrawSurface_GetSurfaceDesc(g_ddrawPrimary, &g_ddrawPrimaryDesc);
	memset(&g_ddrawBufferDesc, 0, sizeof(DDSURFACEDESC));
	g_ddrawBufferDesc.dwSize = sizeof(DDSURFACEDESC);
	g_ddrawResult = IDirectDrawSurface_GetSurfaceDesc(g_ddrawBuffer, &g_ddrawBufferDesc);
	if (g_ddrawBack != NULL) {
		memset(&g_ddrawBackDesc, 0, sizeof(DDSURFACEDESC));
		g_ddrawBackDesc.dwSize = sizeof(DDSURFACEDESC);
		g_ddrawResult = IDirectDrawSurface_GetSurfaceDesc(g_ddrawBack, &g_ddrawBackDesc);
	}

	g_ddrawScreenRect.left = 0;
	g_ddrawScreenRect.top = 0;
	g_ddrawScreenRect.right = p_width;
	g_ddrawScreenRect.bottom = p_height;

	p_buffer->m_pixels = NULL;
	p_buffer->m_maxX = g_ddrawBackDesc.lPitch - 1;
	p_buffer->m_maxY = p_height - 1;
	p_buffer->m_unk0x10 = 0;
	p_buffer->m_bitmapInfo = &g_bitmapInfo;

	if (g_ddrawPalette == NULL) {
		if ((entries = calloc(0x100, sizeof(PALETTEENTRY))) != NULL) {
			g_ddrawResult =
				IDirectDraw_CreatePalette(g_ddraw, DDPCAPS_8BIT | DDPCAPS_ALLOW256, entries, &g_ddrawPalette, NULL);
			if (g_ddrawResult != DD_OK) {
				DebugPrint("DDraw CreatePalette(): %d\n", g_ddrawResult & 0xfff);
				DdrawStop();
				return -1;
			}
		}
		else {
			return -1;
		}
	}

	g_ddrawResult = IDirectDrawSurface_SetPalette(g_ddrawPrimary, g_ddrawPalette);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDraw SetPalette(Primary): %d\n", g_ddrawResult & 0xfff);
	}

	if (p_width == 320 && p_height == 200) {
		g_currentDrawMode->m_stretchBlit = DdrawStretchBlit320;
	}

	g_ddrawInitialized = TRUE;
	return 0;
}

// FUNCTION: MW2SHELL 0x10011e5c
MechS32 DdrawFlip()
{
	g_ddrawResult = DdrawUnlock();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Flip Unlock(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Flip Restore(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = IDirectDrawSurface_Flip(g_ddrawPrimary, NULL, DDFLIP_WAIT);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Flip Flip(): %d\n", g_ddrawResult & 0xfff);
	}

	return g_ddrawResult == DD_OK ? 0 : -1;
}

// FUNCTION: MW2SHELL 0x10011fdf
MechS32 DdrawBlit()
{
	g_ddrawResult = DdrawUnlock();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Blit Unlock(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Blit Restore(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = IDirectDrawSurface_BltFast(g_ddrawPrimary, 0, 0, g_ddrawBuffer, NULL, DDBLTFAST_WAIT);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Blit BltFast(): %d\n", g_ddrawResult & 0xfff);
	}

	return g_ddrawResult == DD_OK ? 0 : -1;
}

// FUNCTION: MW2SHELL 0x1001216c
MechS32 DdrawBlitFlip()
{
	g_ddrawResult = DdrawUnlock();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_BlitFlip Unlock(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_BlitFlip Restore(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = IDirectDrawSurface_BltFast(g_ddrawBack, 0, 0, g_ddrawBuffer, NULL, DDBLTFAST_WAIT);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_BlitFlip BltFast(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = IDirectDrawSurface_Flip(g_ddrawPrimary, NULL, DDFLIP_WAIT);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_BlitFlip Flip(): %d\n", g_ddrawResult & 0xfff);
	}

	return g_ddrawResult == DD_OK ? 0 : -1;
}

// FUNCTION: MW2SHELL 0x10012337
MechS32 DdrawStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	RECT rect;

	g_ddrawResult = DdrawUnlock();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_StretchBlit Unlock(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_StretchBlit Restore(): %d\n", g_ddrawResult & 0xfff);
	}

	rect.left = p_left;
	rect.top = p_top;
	rect.right = p_right;
	rect.bottom = p_bottom;
	g_ddrawResult = IDirectDrawSurface_Blt(g_ddrawPrimary, &g_ddrawScreenRect, g_ddrawBuffer, &rect, DDBLT_WAIT, NULL);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_StretchBlit Blt(): %d\n", g_ddrawResult & 0xfff);
	}

	return g_ddrawResult == DD_OK ? 0 : -1;
}

// FUNCTION: MW2SHELL 0x100124e7
MechS32 DdrawStretchBlit320(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	RECT rect;

	g_ddrawResult = DdrawUnlock();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_StretchBlit320 Unlock(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_StretchBlit320 Restore(): %d\n", g_ddrawResult & 0xfff);
	}

	rect.left = p_left;
	rect.top = p_top;
	rect.right = p_right;
	rect.bottom = p_bottom;
	g_ddrawResult = IDirectDrawSurface_Blt(g_ddrawStretch, &g_ddrawScreenRect, g_ddrawBuffer, &rect, DDBLT_WAIT, NULL);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_StretchBlit320 Blt(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = IDirectDrawSurface_Flip(g_ddrawPrimary, g_ddrawStretch, DDFLIP_WAIT);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Flip Flip(): %d\n", g_ddrawResult & 0xfff);
	}

	return g_ddrawResult == DD_OK ? 0 : -1;
}

// FUNCTION: MW2SHELL 0x100126d9
MechS32 DdrawBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	RECT rect;

	if (g_currentDrawMode->m_index == 0) {
		return DdrawFlip();
	}

	g_ddrawResult = DdrawUnlock();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_BlitRect Unlock(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_BlitRect Restore(): %d\n", g_ddrawResult & 0xfff);
	}

	rect.left = p_left;
	rect.top = p_top;
	rect.right = p_right;
	rect.bottom = p_bottom;
	if (g_currentDrawMode->m_index == 1) {
		g_ddrawResult = IDirectDrawSurface_BltFast(g_ddrawBack, 0, 0, g_ddrawBuffer, &rect, DDBLTFAST_WAIT);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_BlitRect BltFast(): %d\n", g_ddrawResult & 0xfff);
		}

		g_ddrawResult = IDirectDrawSurface_Flip(g_ddrawPrimary, NULL, DDFLIP_WAIT);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_BlitRect Flip(): %d\n", g_ddrawResult & 0xfff);
		}
	}
	else {
		g_ddrawResult = IDirectDrawSurface_BltFast(g_ddrawPrimary, 0, 0, g_ddrawBuffer, &rect, DDBLTFAST_WAIT);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_BlitRect BltFast(): %d\n", g_ddrawResult & 0xfff);
		}
	}

	return g_ddrawResult == DD_OK ? 0 : -1;
}

// FUNCTION: MW2SHELL 0x10012936
MechS32 DdrawFill(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechU32 p_color)
{
	RECT rect;

	g_ddrawResult = DdrawUnlock();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Fill Unlock(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Fill Restore(): %d\n", g_ddrawResult & 0xfff);
	}

	rect.left = p_left;
	rect.top = p_top;
	rect.right = p_right;
	rect.bottom = p_bottom;
	g_ddrawBltFx.dwFillColor = p_color;
	g_ddrawResult =
		IDirectDrawSurface_Blt(g_ddrawBuffer, &rect, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &g_ddrawBltFx);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_Fill Blt(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

	return 0;
}

// Stack-slot permutation: entries, relock and i. The original also compares i < p_count as
// `cmp [i], ...` and adds p_first + i with i loaded first; the declaration order doesn't flip it.
// FUNCTION: MW2SHELL 0x10012add
MechS32 DdrawWritePaletteEntries(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors)
{
	MechS32 i;
	MechS32 relock;
	PALETTEENTRY entries[0x100];

	relock = FALSE;
	if (!g_ddrawInitialized) {
		return -1;
	}

	if (p_palette == NULL || p_first < 0 || p_first > 0xff || p_count <= 0 || p_count > 0x100 - p_first) {
		return -1;
	}

	for (i = 0; i < p_count; i++) {
		g_paletteColors[p_first + i] = p_palette[i];
		entries[i].peRed = p_palette[i].m_red * 4;
		entries[i].peGreen = p_palette[i].m_green * 4;
		entries[i].peBlue = p_palette[i].m_blue * 4;
	}

	if (g_ddrawLocked) {
		relock = TRUE;
		g_ddrawResult = IDirectDrawSurface_Unlock(g_ddrawBuffer, g_ddrawBufferDesc.lpSurface);
		g_unk0x10062cdc->m_pixels = NULL;
		g_ddrawLocked = FALSE;
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_WritePaletteEntries Unlock(): %d\n", g_ddrawResult & 0xfff);
		}
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_WritePaletteEntries Restore(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = IDirectDrawPalette_SetEntries(g_ddrawPalette, 0, p_first, p_count, entries);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_WritePaletteEntries SetEntries(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

	if (relock && (g_ddrawResult = DdrawLockBuffer()) != DD_OK) {
		DebugPrint("DDRAW_WritePaletteEntries LockBuffer(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

	return 0;
}

// Stack-slot permutation: entries, relock and i.
// FUNCTION: MW2SHELL 0x10012db7
MechS32 DdrawWritePaletteGamma(PaletteColor* p_palette)
{
	MechS32 i;
	MechS32 relock;
	PALETTEENTRY entries[0x100];

	relock = FALSE;
	if (p_palette == NULL || g_ddrawPalette == NULL) {
		return -1;
	}

	for (i = 0; i < 0x100; i++) {
		g_paletteColorsPreBrightness[i] = p_palette[i];
		CopyPaletteColorWithBrightness(&p_palette[i], &g_paletteColors[i]);
		entries[i].peRed = g_paletteColors[i].m_red * 4;
		entries[i].peGreen = g_paletteColors[i].m_green * 4;
		entries[i].peBlue = g_paletteColors[i].m_blue * 4;
	}

	if (g_ddrawLocked) {
		relock = TRUE;
		g_ddrawResult = IDirectDrawSurface_Unlock(g_ddrawBuffer, g_ddrawBufferDesc.lpSurface);
		g_unk0x10062cdc->m_pixels = NULL;
		g_ddrawLocked = FALSE;
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_WritePaletteGamma Unlock(): %d\n", g_ddrawResult & 0xfff);
		}
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_WritePaletteGamma Restore(): %d\n", g_ddrawResult & 0xfff);
	}

	g_ddrawResult = IDirectDrawPalette_SetEntries(g_ddrawPalette, 0, 0, 0x100, entries);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_WritePaletteGamma SetEntries(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

	if (relock && (g_ddrawResult = DdrawLockBuffer()) != DD_OK) {
		DebugPrint("DDRAW_WritePaletteGamma LockBuffer(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

	return 0;
}

// STUB: MW2SHELL 0x10013068
MechS32 DdrawPaletteFade(PaletteColor* p_palette, MechS32 p_steps)
{
	STUB(0x10013068);
	return 0;
}

// Locks g_ddrawBuffer and points the output buffer at it.
// FUNCTION: MW2SHELL 0x10013551
MechS32 DdrawLockBuffer()
{
	if (!g_ddrawLocked) {
		g_ddrawResult = DdrawRestore();
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_LockBuffer Restore(): %d\n", g_ddrawResult & 0xfff);
			return -1;
		}

		g_ddrawResult = IDirectDrawSurface_Lock(g_ddrawBuffer, NULL, &g_ddrawBufferDesc, DDLOCK_WAIT, NULL);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_LockBuffer Lock(): %d\n", g_ddrawResult & 0xfff);
			return -1;
		}

		g_ddrawLocked = TRUE;
	}

	g_unk0x10062cdc->m_pixels = g_ddrawBufferDesc.lpSurface;
	g_unk0x10062cdc->m_maxX = g_ddrawBufferDesc.lPitch - 1;
	return 0;
}
