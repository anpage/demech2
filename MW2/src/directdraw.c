#include "directdraw.h"

#include "brightness.h"
#include "debugprint.h"
#include "decomp.h"
#include "displaybackend.h"
#include "drawbitmapinfo.h"
#include "palettecolor.h"
#include "refreshmode.h"
#include "simmain.h"
#include "types.h"
#include "window.h"

#ifdef MW2_POWERVR
#include "accel/a3d.h"
#endif

#include <ddraw.h>
#include <string.h>
#include <windows.h>

#ifdef MW2_S3
#include "accel/a3d.h"
#endif

// The PowerVR edition's DdrawUnlock isn't __inline: it is a function of its own, after the unit's
// others, where the other editions expand it.
#ifdef MW2_POWERVR
#define DDRAW_UNLOCK_INLINE
#else
#define DDRAW_UNLOCK_INLINE __inline
#endif

// The DirectDraw back end of the refresh modes (refreshmode.c), the simulator's copy of the
// shell's. Its functions are named after their debug messages, "DDRAW_Flip" and so on: the frame
// is drawn into g_ddrawBuffer, an offscreen surface (or the back buffer when the refresh mode
// flips), then blitted or flipped to the primary surface. Without /Ob1, the __inline helpers
// DdrawUnlock and DdrawRestore aren't expanded as in the shell: the compiler emits them after the
// unit's functions, each in its own 16-byte aligned section.

void DdrawStop(void);
void DdrawDestroySurfaces(void);
#ifndef MW2_POWERVR
MechS32 DdrawCreateSurfaces(MechS32 p_width, MechS32 p_height);
#endif
MechS32 DdrawInit(WINDOW* p_buffer, MechS32 p_width, MechS32 p_height);
MechS32 DdrawFlip(void);
MechS32 DdrawBlit(void);
MechS32 DdrawBlitFlip(void);
MechS32 DdrawStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
#ifndef MW2_POWERVR
MechS32 DdrawStretchBlit320(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
#endif
MechS32 DdrawBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 DdrawWritePaletteEntries(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors);
MechS32 DdrawWritePaletteGamma(PaletteColor* p_palette);
MechS32 DdrawPaletteFade(PaletteColor* p_palette, MechS32 p_steps);
MechS32 DdrawLockBuffer(void);
DDRAW_UNLOCK_INLINE static HRESULT DdrawUnlock(void);
__inline static HRESULT DdrawRestore(void);

// GLOBAL: MW2 0x100b1cb0
// GLOBAL: MW2ATI 0x1017e5c0
// GLOBAL: MW2S3 0x1006a968
// GLOBAL: MW2POWERVR 0x101a9654
LPDIRECTDRAW g_ddraw = NULL;

// GLOBAL: MW2 0x100b1cb4
// GLOBAL: MW2ATI 0x1017e5c4
// GLOBAL: MW2S3 0x1006a96c
// GLOBAL: MW2POWERVR 0x101a9658
LPDIRECTDRAWSURFACE g_ddrawPrimary = NULL;

// GLOBAL: MW2 0x100b1cb8
// GLOBAL: MW2ATI 0x1017e5c8
// GLOBAL: MW2S3 0x1006a970
// GLOBAL: MW2POWERVR 0x101a965c
LPDIRECTDRAWSURFACE g_ddrawBack = NULL;

// GLOBAL: MW2 0x100b1cbc
// GLOBAL: MW2ATI 0x1017e5cc
// GLOBAL: MW2S3 0x1006a974
// GLOBAL: MW2POWERVR 0x101a9660
LPDIRECTDRAWSURFACE g_ddrawBuffer = NULL;

// The second back buffer of the 320x200 mode, which stretches the frame into it and flips.
// GLOBAL: MW2 0x100b1cc0
// GLOBAL: MW2ATI 0x1017e5d0
// GLOBAL: MW2S3 0x1006a978
// GLOBAL: MW2POWERVR 0x101a9664
LPDIRECTDRAWSURFACE g_ddrawStretch = NULL;

// Never accessed: there is nothing to name it after.
// GLOBAL: MW2 0x100b1cc4
// GLOBAL: MW2POWERVR 0x101a9668
undefined4 g_unk0x100b1cc4 = 0;

// GLOBAL: MW2 0x100b1cc8
// GLOBAL: MW2ATI 0x1017e5d8
// GLOBAL: MW2S3 0x1006a980
// GLOBAL: MW2POWERVR 0x101a966c
LPDIRECTDRAWPALETTE g_ddrawPalette = NULL;

// TRUE while g_ddrawBuffer is locked for drawing.
// GLOBAL: MW2 0x100b1ccc
// GLOBAL: MW2ATI 0x1017e5dc
// GLOBAL: MW2S3 0x1006a984
// GLOBAL: MW2POWERVR 0x101a9670
MechS32 g_ddrawLocked = FALSE;

// GLOBAL: MW2 0x100b1cd0
// GLOBAL: MW2ATI 0x1017e5e0
// GLOBAL: MW2S3 0x1006a988
// GLOBAL: MW2POWERVR 0x101a9674
MechS32 g_ddrawInitialized = FALSE;

// GLOBAL: MW2 0x100b1cd8
// GLOBAL: MW2ATI 0x1017e5e8
// GLOBAL: MW2S3 0x1006a990
// GLOBAL: MW2POWERVR 0x101a9678
DisplayBackend g_directDrawBackend = {
	c_displayBackendDirectDraw,
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

// Draws into the back buffer and flips.
// GLOBAL: MW2 0x100b1d00
// GLOBAL: MW2ATI 0x1017e610
// GLOBAL: MW2S3 0x1006a9b8
// GLOBAL: MW2POWERVR 0x101a96a0
RefreshMode g_ddrawFlipRefreshMode = {
	0,
	c_displayBackendDirectDraw,
	1,
	0,
	DdrawInit,
	(MechS32 (*)()) DdrawDestroySurfaces,
	DdrawFlip,
	DdrawBlitRect,
	DdrawStretchBlit
};

// Draws into an offscreen surface in system memory, blits it to the back buffer and flips.
// GLOBAL: MW2 0x100b1d28
// GLOBAL: MW2ATI 0x1017e638
// GLOBAL: MW2S3 0x1006a9e0
// GLOBAL: MW2POWERVR 0x101a96c8
RefreshMode g_ddrawBlitFlipRefreshMode = {
	1,
	c_displayBackendDirectDraw,
	1,
	0,
	DdrawInit,
	(MechS32 (*)()) DdrawDestroySurfaces,
	DdrawBlitFlip,
	DdrawBlitRect,
	DdrawStretchBlit
};

// Draws into an offscreen surface in video memory and blits it to the primary surface.
// GLOBAL: MW2 0x100b1d50
// GLOBAL: MW2ATI 0x1017e660
// GLOBAL: MW2S3 0x1006aa08
// GLOBAL: MW2POWERVR 0x101a96f0
RefreshMode g_ddrawVideoMemoryRefreshMode = {
	2,
	c_displayBackendDirectDraw,
	1,
	0,
	DdrawInit,
	(MechS32 (*)()) DdrawDestroySurfaces,
	DdrawBlit,
	DdrawBlitRect,
	DdrawStretchBlit
};

// Draws into an offscreen surface in system memory and blits it to the primary surface.
// GLOBAL: MW2 0x100b1d78
// GLOBAL: MW2ATI 0x1017e688
// GLOBAL: MW2S3 0x1006aa30
// GLOBAL: MW2POWERVR 0x101a9718
RefreshMode g_ddrawSystemMemoryRefreshMode = {
	3,
	c_displayBackendDirectDraw,
	1,
	0,
	DdrawInit,
	(MechS32 (*)()) DdrawDestroySurfaces,
	DdrawBlit,
	DdrawBlitRect,
	DdrawStretchBlit
};

// GLOBAL: MW2 0x100c26b0
// GLOBAL: MW2ATI 0x10178770
// GLOBAL: MW2S3 0x100ab5d0
// GLOBAL: MW2POWERVR 0x101a3430
RECT g_ddrawScreenRect;

// GLOBAL: MW2 0x100c26c0
// GLOBAL: MW2ATI 0x101787e4
// GLOBAL: MW2S3 0x100ab5e0
// GLOBAL: MW2POWERVR 0x101a3440
HRESULT g_ddrawResult;

// GLOBAL: MW2 0x100c26d0
// GLOBAL: MW2ATI 0x10178780
// GLOBAL: MW2S3 0x100ab5f0
DDBLTFX g_ddrawBltFx;

// GLOBAL: MW2 0x100c2740
// GLOBAL: MW2ATI 0x101788d0
// GLOBAL: MW2S3 0x100ab660
DDSURFACEDESC g_ddrawPrimaryDesc;

// GLOBAL: MW2 0x100c27b0
// GLOBAL: MW2ATI 0x10178860
// GLOBAL: MW2S3 0x100ab6d0
DDSURFACEDESC g_ddrawBackDesc;

// GLOBAL: MW2 0x100c281c
// GLOBAL: MW2S3 0x100ab73c
DDSCAPS g_ddrawCaps;

// GLOBAL: MW2 0x100c2820
// GLOBAL: MW2ATI 0x101787f0
// GLOBAL: MW2S3 0x100ab740
DDSURFACEDESC g_ddrawBufferDesc;

#if defined(MW2_RES_512X384) || defined(MW2_S3)
// The display mode FUN_100059b0 enumerates was found (FUN_10005950).
// GLOBAL: MW2ATI 0x1017e6ac
// GLOBAL: MW2S3 0x1006aa54
MechS32 g_ddrawModeFound = FALSE;
#endif

// FUNCTION: MW2 0x10077850
// FUNCTION: MW2ATI 0x100051f0
// FUNCTION: MW2S3 0x1002f8c0
// FUNCTION: MW2POWERVR 0x100049a0
void DdrawStop(void)
{
	g_ddrawInitialized = FALSE;
	if (g_ddraw != NULL) {
#ifdef MW2_S3
		// The S3 edition frees the renderer's textures first and closes the renderer before the
		// DirectDraw object.
		FUN_10010150();
#endif
		DdrawDestroySurfaces();
		IDirectDraw_SetCooperativeLevel(g_ddraw, g_gameWindow, DDSCL_NORMAL);
		IDirectDraw_RestoreDisplayMode(g_ddraw);
		if (g_ddrawPalette != NULL) {
			IDirectDrawPalette_Release(g_ddrawPalette);
			g_ddrawPalette = NULL;
		}
#ifdef MW2_S3
		A3D_shutdown();
#endif
		IDirectDraw_Release(g_ddraw);
		g_ddraw = NULL;
	}
}

// FUNCTION: MW2 0x100778e0
// FUNCTION: MW2ATI 0x10005260
// FUNCTION: MW2S3 0x1002f930
// FUNCTION: MW2POWERVR 0x10004a10
void DdrawDestroySurfaces(void)
{
	g_refreshModeBuffer->m_buffer = NULL;
	if (g_ddrawPrimary != NULL) {
		IDirectDrawSurface_Release(g_ddrawPrimary);
		g_ddrawPrimary = NULL;
		g_ddrawBack = NULL;
		g_ddrawStretch = NULL;
		if (g_currentRefreshMode->m_index == 0) {
			g_ddrawBuffer = NULL;
		}
	}

	if (g_ddrawBuffer != NULL && (g_refreshModeWidth != 320 || g_refreshModeHeight != 200)) {
		IDirectDrawSurface_Release(g_ddrawBuffer);
		g_ddrawBuffer = NULL;
	}
}

#ifdef MW2_POWERVR
// The PowerVR edition's: SGL opens the display (FUN_10060e60, the renderer's), and its DirectDraw
// mode callback (FUN_10060000) hands over the DirectDraw object.
// FUNCTION: MW2POWERVR 0x10004a90
MechS32 DdrawInit(WINDOW* p_buffer, MechS32 p_width, MechS32 p_height)
{
	if (g_currentDisplayBackend->m_id != c_displayBackendDirectDraw) {
		g_currentDisplayBackend->m_end();
		g_currentDisplayBackend = g_displayBackends[c_displayBackendDirectDraw];
		if (g_currentDisplayBackend->m_windowMode != g_windowMode) {
			AdjustWindowSize(g_currentDisplayBackend);
		}
	}

	return FUN_10060e60(g_gameWindow, p_width, p_height) ? 0 : -1;
}
#else
// Creates the primary surface and g_ddrawBuffer: the 320x200 mode flips through two back
// buffers, the flipping modes (0 and 1) through one, and the blitting modes (2 and 3) draw into
// an offscreen surface.
#ifdef MW2_S3
// The S3 edition's: 16 bits per pixel and no 320x200 mode, the flipping modes (0 and 1) through one
// back buffer.
// FUNCTION: MW2S3 0x1002f9b0
MechS32 DdrawCreateSurfaces(MechS32 p_width, MechS32 p_height)
{
	MechS32 flip = FALSE;

	if (g_currentRefreshMode->m_index == 0 || g_currentRefreshMode->m_index == 1) {
		flip = TRUE;
	}

	g_ddrawPrimaryDesc.dwSize = sizeof(DDSURFACEDESC);
	g_ddrawPrimaryDesc.dwFlags = DDSD_CAPS;
	g_ddrawPrimaryDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;
	if (flip) {
		g_ddrawPrimaryDesc.dwFlags |= DDSD_BACKBUFFERCOUNT;
		g_ddrawPrimaryDesc.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		g_ddrawPrimaryDesc.dwBackBufferCount = 1;
	}

	g_ddrawResult = IDirectDraw_CreateSurface(g_ddraw, &g_ddrawPrimaryDesc, &g_ddrawPrimary, NULL);
	if (g_ddrawResult != DD_OK) {
		if (flip) {
			DebugPrint("DDRAW_CreateSurfaces primary failed; trying 1 back buffer\n");
			g_ddrawPrimaryDesc.dwBackBufferCount = 1;
			g_ddrawResult = IDirectDraw_CreateSurface(g_ddraw, &g_ddrawPrimaryDesc, &g_ddrawPrimary, NULL);
		}
		else if (g_currentRefreshMode->m_index == 3) {
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

	if (g_currentRefreshMode->m_index == 0) {
		g_ddrawBuffer = g_ddrawBack;
	}
	else {
		g_ddrawBufferDesc.dwSize = sizeof(DDSURFACEDESC);
		g_ddrawBufferDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
		g_ddrawBufferDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		g_ddrawBufferDesc.dwWidth = p_width;
		g_ddrawBufferDesc.dwHeight = p_height;
		if (g_currentRefreshMode->m_index == 2) {
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

	return g_ddrawResult;
}
#else
// FUNCTION: MW2 0x10077990
// FUNCTION: MW2ATI 0x100052e0
MechS32 DdrawCreateSurfaces(MechS32 p_width, MechS32 p_height)
{
	MechS32 flip = FALSE;

	if (p_width == 320 && p_height == 200) {
		if (g_currentRefreshMode->m_index == 2 || g_currentRefreshMode->m_index == 3) {
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
		if (g_currentRefreshMode->m_index == 0 || g_currentRefreshMode->m_index == 1) {
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
			else if (g_currentRefreshMode->m_index == 3) {
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

		if (g_currentRefreshMode->m_index == 0) {
			g_ddrawBuffer = g_ddrawBack;
		}
		else {
			g_ddrawBufferDesc.dwSize = sizeof(DDSURFACEDESC);
			g_ddrawBufferDesc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
			g_ddrawBufferDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
			g_ddrawBufferDesc.dwWidth = p_width;
			g_ddrawBufferDesc.dwHeight = p_height;
			if (g_currentRefreshMode->m_index == 2) {
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

#endif

// FUNCTION: MW2 0x10077d75
// FUNCTION: MW2ATI 0x10005600
// FUNCTION: MW2S3 0x1002fbb0
MechS32 DdrawInit(WINDOW* p_buffer, MechS32 p_width, MechS32 p_height)
{
	LPPALETTEENTRY entries;
#ifndef MW2_S3
	DDCAPS caps;
#endif

	if (g_currentDisplayBackend->m_id != c_displayBackendDirectDraw) {
		g_currentDisplayBackend->m_end();
		g_currentDisplayBackend = g_displayBackends[c_displayBackendDirectDraw];
		if (g_currentDisplayBackend->m_windowMode != g_windowMode) {
			AdjustWindowSize(g_currentDisplayBackend);
		}
	}

	if (g_ddraw == NULL) {
		g_ddrawResult = DirectDrawCreate(NULL, &g_ddraw, NULL);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_Init DirectDrawCreate(): %d\n", g_ddrawResult & 0xfff);
			return 1;
		}

		g_ddrawResult =
			IDirectDraw_SetCooperativeLevel(g_ddraw, g_gameWindow, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | 0x40);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_Init SetCooperativeLevel(): %d\n", g_ddrawResult & 0xfff);
			DdrawStop();
			return 1;
		}

#ifdef MW2_S3
		g_ddrawResult = IDirectDraw_SetDisplayMode(g_ddraw, p_width, p_height, 16);
#else
		g_ddrawResult = IDirectDraw_SetDisplayMode(g_ddraw, p_width, p_height, 8);
#endif
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_Init SetDisplayMode(): %d\n", g_ddrawResult & 0xfff);
			DdrawStop();
			return 1;
		}
	}

#ifndef MW2_S3
	memset(&caps, 0, sizeof(caps));
	caps.dwSize = sizeof(caps);
	if (IDirectDraw_GetCaps(g_ddraw, &caps, NULL) == DD_OK && (caps.dwCaps & DDCAPS_BANKSWITCHED) &&
		(g_currentRefreshMode->m_index == 0 || g_currentRefreshMode->m_index == 2)) {
		DebugPrint("DDRAW_Init: bank-switched video card.\n");
		return 1;
	}
#endif

	if (DdrawCreateSurfaces(p_width, p_height)) {
		DdrawDestroySurfaces();
		return 1;
	}

	g_ddrawBltFx.dwSize = sizeof(DDBLTFX);
#ifdef MW2_S3
	if (DdrawFill(0, 0, p_width, p_height, 0)) {
#else
	if (DdrawFill(0, 0, p_width - 1, p_height - 1, 0)) {
#endif
		DdrawDestroySurfaces();
		return 1;
	}

	if (g_currentRefreshMode->m_flip()) {
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

	p_buffer->m_buffer = NULL;
	p_buffer->m_xMax = g_ddrawBackDesc.lPitch - 1;
	p_buffer->m_yMax = p_height - 1;
	p_buffer->m_shadow = 0;
	p_buffer->m_bitmapInfo = &g_bitmapInfo;

	if (g_ddrawPalette == NULL) {
		if ((entries = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 0x100 * sizeof(PALETTEENTRY))) !=
			NULL) {
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
		g_currentRefreshMode->m_stretchBlit = DdrawStretchBlit320;
	}

	g_ddrawInitialized = TRUE;
	return 0;
}

#endif

#if defined(MW2_RES_512X384) || defined(MW2_S3)
// EnumDisplayModes' callback for FUN_100059b0: records whether p_desc is the mode p_context holds
// (its width, height and bits per pixel).
// FUNCTION: MW2ATI 0x10005950
// FUNCTION: MW2S3 0x1002feb0
static HRESULT WINAPI FUN_10005950(LPDDSURFACEDESC p_desc, LPVOID p_context)
{
	MechS32* mode;

	mode = (MechS32*) p_context;
	DebugPrint(
		"DDRAW_EnumCallBack(%d x %d x %d)\n",
		p_desc->dwWidth,
		p_desc->dwHeight,
		p_desc->ddpfPixelFormat.dwRGBBitCount
	);
	if (p_desc->dwWidth == mode[0] && mode[1] == p_desc->dwHeight && mode[2] == p_desc->ddpfPixelFormat.dwRGBBitCount) {
		g_ddrawModeFound = TRUE;
	}

	return DDENUMRET_OK;
}

// Returns whether DirectDraw has the full-screen mode p_width x p_height in p_bitCount bits per
// pixel, creating the DirectDraw object for the question if there is none.
// FUNCTION: MW2ATI 0x100059b0
// FUNCTION: MW2S3 0x1002ff10
MechS32 FUN_100059b0(MechS32 p_width, MechS32 p_height, MechS32 p_bitCount)
{
	MechS32 mode[3];
	MechS32 created;

	created = FALSE;
	if (!g_ddraw) {
		created = TRUE;
		g_ddrawResult = DirectDrawCreate(NULL, &g_ddraw, NULL);
		if (g_ddrawResult != DD_OK) {
#ifdef MW2_S3
			DebugPrint("DDRAW_Init DirectDrawCreate(): %d\n", g_ddrawResult & 0xfff);
#else
			DebugPrint("DDRAW_Init DirectDrawCreate(): %d\n", g_ddrawResult);
#endif
			return FALSE;
		}
	}

	g_ddrawPrimaryDesc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	g_ddrawPrimaryDesc.ddpfPixelFormat.dwRGBBitCount = p_bitCount;
	g_ddrawPrimaryDesc.ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8;
	g_ddrawPrimaryDesc.dwSize = sizeof(DDSURFACEDESC);
	g_ddrawPrimaryDesc.dwFlags = DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT;
	if (p_bitCount != 8) {
		g_ddrawPrimaryDesc.ddpfPixelFormat.dwFlags = DDPF_RGB;
	}

	mode[2] = p_bitCount;
	g_ddrawModeFound = FALSE;
	g_ddrawPrimaryDesc.dwHeight = p_height;
	g_ddrawPrimaryDesc.dwWidth = p_width;
	mode[0] = p_width;
	mode[1] = p_height;
	g_ddrawResult = IDirectDraw_EnumDisplayModes(g_ddraw, 0, &g_ddrawPrimaryDesc, mode, FUN_10005950);
	if (created) {
		IDirectDraw_Release(g_ddraw);
		g_ddraw = NULL;
	}

	return g_ddrawModeFound;
}
#endif

// FUNCTION: MW2 0x100781c0
// FUNCTION: MW2ATI 0x10005ab0
// FUNCTION: MW2S3 0x10030010
// FUNCTION: MW2POWERVR 0x10004ae0
MechS32 DdrawFlip(void)
{
#ifdef MW2_POWERVR
	// The PowerVR edition's back end has nothing to draw to until SGL hands over DirectDraw.
	if (g_ddraw == NULL) {
		return -1;
	}

#endif
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

// FUNCTION: MW2 0x10078285
// FUNCTION: MW2ATI 0x10005bb0
// FUNCTION: MW2S3 0x10030130
// FUNCTION: MW2POWERVR 0x10004bc0
MechS32 DdrawBlit(void)
{
#ifdef MW2_POWERVR
	if (g_ddraw == NULL) {
		return -1;
	}

#endif
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

// FUNCTION: MW2 0x10078354
// FUNCTION: MW2ATI 0x10005cc0
// FUNCTION: MW2S3 0x10030260
// FUNCTION: MW2POWERVR 0x10004cc0
MechS32 DdrawBlitFlip(void)
{
#ifdef MW2_POWERVR
	if (g_ddraw == NULL) {
		return -1;
	}

#endif
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

// FUNCTION: MW2 0x10078461
// FUNCTION: MW2ATI 0x10005e00
// FUNCTION: MW2S3 0x100303b0
// FUNCTION: MW2POWERVR 0x10004de0
MechS32 DdrawStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	RECT rect;

#ifdef MW2_POWERVR
	if (g_ddraw == NULL) {
		return -1;
	}

#endif
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

#ifndef MW2_POWERVR
// FUNCTION: MW2 0x10078553
// FUNCTION: MW2ATI 0x10005f40
// FUNCTION: MW2S3 0x10030510
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
#endif

// FUNCTION: MW2 0x10078687
// FUNCTION: MW2ATI 0x100060b0
// FUNCTION: MW2S3 0x100306a0
// FUNCTION: MW2POWERVR 0x10004f00
MechS32 DdrawBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	RECT rect;

#ifdef MW2_POWERVR
	if (g_ddraw == NULL) {
		return -1;
	}

#endif
	if (g_currentRefreshMode->m_index == 0) {
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
	if (g_currentRefreshMode->m_index == 1) {
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

#ifndef MW2_POWERVR
// FUNCTION: MW2 0x10078826
// FUNCTION: MW2ATI 0x10006260
// FUNCTION: MW2S3 0x10030860
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
#endif

#ifdef MW2_S3
// The S3 edition's records the colors and computes the entries, but sets no palette (its display
// is 16-bit).
// FUNCTION: MW2S3 0x100309c0
MechS32 DdrawWritePaletteEntries(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors)
{
	MechS32 i;
	PALETTEENTRY entries[0x100];

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

	return 0;
}
#else
// The original also compares i < p_count as `cmp [i], ...` and adds p_first + i with i loaded
// first. The PowerVR edition's palette functions keep the colors but write no DirectDraw palette.
// FUNCTION: MW2 0x1007890f
// FUNCTION: MW2ATI 0x100063a0
// FUNCTION: MW2POWERVR 0x10005090
MechS32 DdrawWritePaletteEntries(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors)
{
	MechS32 i;
#ifndef MW2_POWERVR
	MechS32 relock;
#endif
	PALETTEENTRY entries[0x100];

#ifndef MW2_POWERVR
	relock = FALSE;
#endif
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

#ifndef MW2_POWERVR
	if (g_ddrawLocked) {
		relock = TRUE;
		g_ddrawResult = DdrawUnlock();
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_WritePaletteEntries Unlock(): %d\n", g_ddrawResult & 0xfff);
		}
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_WritePaletteEntries Restore(): %d\n", g_ddrawResult & 0xfff);
	}

#endif
#ifndef MW2_POWERVR
	g_ddrawResult = IDirectDrawPalette_SetEntries(g_ddrawPalette, 0, p_first, p_count, entries);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_WritePaletteEntries SetEntries(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

	if (relock && (g_ddrawResult = DdrawLockBuffer()) != DD_OK) {
		DebugPrint("DDRAW_WritePaletteEntries LockBuffer(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

#endif
	return 0;
}
#endif

#ifdef MW2_S3
// The S3 edition's records the colors with the brightness and computes the entries, but sets no
// palette (its display is 16-bit).
// FUNCTION: MW2S3 0x10030a50
MechS32 DdrawWritePaletteGamma(PaletteColor* p_palette)
{
	MechS32 i;
	PALETTEENTRY entries[0x100];

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

	return 0;
}
#else
// FUNCTION: MW2 0x10078b38
// FUNCTION: MW2ATI 0x10006430
// FUNCTION: MW2POWERVR 0x10005120
MechS32 DdrawWritePaletteGamma(PaletteColor* p_palette)
{
	MechS32 i;
#ifndef MW2_POWERVR
	MechS32 relock;
#endif
	PALETTEENTRY entries[0x100];

#ifndef MW2_POWERVR
	relock = FALSE;
#endif
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

#ifndef MW2_POWERVR
	if (g_ddrawLocked) {
		relock = TRUE;
		g_ddrawResult = DdrawUnlock();
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_WritePaletteGamma Unlock(): %d\n", g_ddrawResult & 0xfff);
		}
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_WritePaletteGamma Restore(): %d\n", g_ddrawResult & 0xfff);
	}

#endif
#ifndef MW2_POWERVR
	g_ddrawResult = IDirectDrawPalette_SetEntries(g_ddrawPalette, 0, 0, 0x100, entries);
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_WritePaletteGamma SetEntries(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

	if (relock && (g_ddrawResult = DdrawLockBuffer()) != DD_OK) {
		DebugPrint("DDRAW_WritePaletteGamma LockBuffer(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

#endif
	return 0;
}
#endif

#ifdef MW2_S3
// The S3 edition's computes the fade's steps without setting the palette (its display is 16-bit):
// only g_paletteColors takes the last step.
// FUNCTION: MW2S3 0x10030af0
MechS32 DdrawPaletteFade(PaletteColor* p_palette, MechS32 p_steps)
{
	PALETTEENTRY entries[0x100];
	MechS32 i;
	MechS32 j;
	MechDouble deltas[0x100][3];

	if (g_ddrawPalette == NULL || p_palette == NULL) {
		return -1;
	}

	for (i = 0; i < 0x100; i++) {
		deltas[i][0] = (MechDouble) ((p_palette[i].m_red - g_paletteColors[i].m_red) * 4) / p_steps;
		deltas[i][1] = (MechDouble) ((p_palette[i].m_green - g_paletteColors[i].m_green) * 4) / p_steps;
		deltas[i][2] = (MechDouble) ((p_palette[i].m_blue - g_paletteColors[i].m_blue) * 4) / p_steps;
	}

	i = p_steps;
	while (i--) {
		j = 0x100;
		while (j--) {
			entries[j].peRed = (MechU8) ((p_steps - i) * deltas[j][0]) + g_paletteColors[j].m_red * 4;
			entries[j].peGreen = (MechU8) ((p_steps - i) * deltas[j][1]) + g_paletteColors[j].m_green * 4;
			entries[j].peBlue = (MechU8) ((p_steps - i) * deltas[j][2]) + g_paletteColors[j].m_blue * 4;
			if (i == 0) {
				g_paletteColors[j].m_red = entries[j].peRed >> 2;
				g_paletteColors[j].m_green = entries[j].peGreen >> 2;
				g_paletteColors[j].m_blue = entries[j].peBlue >> 2;
			}
		}
	}

	return 0;
}
#else
// Fades the palette from g_paletteColors to p_palette in p_steps steps, a step every 16 ms.
// FUNCTION: MW2 0x10078d38
// FUNCTION: MW2ATI 0x100064d0
// FUNCTION: MW2POWERVR 0x100051c0
MechS32 DdrawPaletteFade(PaletteColor* p_palette, MechS32 p_steps)
{
	PALETTEENTRY entries[0x100];
	MechS32 i;
	MechS32 j;
	MechDouble deltas[0x100][3];
#ifndef MW2_POWERVR
	MechS32 relock;
#endif

#ifndef MW2_POWERVR
	relock = FALSE;
#endif
	if (g_ddrawPalette == NULL || p_palette == NULL) {
		return -1;
	}

#ifndef MW2_POWERVR
	if (g_ddrawLocked) {
		relock = TRUE;
		g_ddrawResult = DdrawUnlock();
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_PaletteFade Unlock(): %d\n", g_ddrawResult & 0xfff);
		}
	}

	g_ddrawResult = DdrawRestore();
	if (g_ddrawResult != DD_OK) {
		DebugPrint("DDRAW_PaletteFade Restore(): %d\n", g_ddrawResult & 0xfff);
	}

#endif
	for (i = 0; i < 0x100; i++) {
		deltas[i][0] = (MechDouble) ((p_palette[i].m_red - g_paletteColors[i].m_red) * 4) / p_steps;
		deltas[i][1] = (MechDouble) ((p_palette[i].m_green - g_paletteColors[i].m_green) * 4) / p_steps;
		deltas[i][2] = (MechDouble) ((p_palette[i].m_blue - g_paletteColors[i].m_blue) * 4) / p_steps;
	}

	i = p_steps;
	while (i--) {
		j = 0x100;
		while (j--) {
			entries[j].peRed = (MechU8) ((p_steps - i) * deltas[j][0]) + g_paletteColors[j].m_red * 4;
			entries[j].peGreen = (MechU8) ((p_steps - i) * deltas[j][1]) + g_paletteColors[j].m_green * 4;
			entries[j].peBlue = (MechU8) ((p_steps - i) * deltas[j][2]) + g_paletteColors[j].m_blue * 4;
			if (i == 0) {
				g_paletteColors[j].m_red = entries[j].peRed >> 2;
				g_paletteColors[j].m_green = entries[j].peGreen >> 2;
				g_paletteColors[j].m_blue = entries[j].peBlue >> 2;
			}
		}

#ifndef MW2_POWERVR
		g_ddrawResult = IDirectDrawPalette_SetEntries(g_ddrawPalette, 0, 0, 0x100, entries);
		if (g_ddrawResult != DD_OK) {
			DebugPrint("DDRAW_PaletteFade SetEntries(): %d\n", g_ddrawResult & 0xfff);
			return -1;
		}

		Sleep(0x10);
#endif
	}

#ifndef MW2_POWERVR
	if (relock && (g_ddrawResult = DdrawLockBuffer()) != DD_OK) {
		DebugPrint("DDRAW_PaletteFade LockBuffer(): %d\n", g_ddrawResult & 0xfff);
		return -1;
	}

#endif
	return 0;
}
#endif

// Locks g_ddrawBuffer and points the output buffer at it.
// FUNCTION: MW2 0x10079170
// FUNCTION: MW2ATI 0x100066b0
// FUNCTION: MW2S3 0x10030c80
// FUNCTION: MW2POWERVR 0x100053a0
MechS32 DdrawLockBuffer(void)
{
#ifdef MW2_POWERVR
	if (g_ddraw == NULL) {
		return -1;
	}

#endif
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
#ifdef MW2_S3
		FUN_1000f850(g_ddrawBufferDesc.lpSurface);
#endif
	}

	g_refreshModeBuffer->m_buffer = g_ddrawBufferDesc.lpSurface;
	g_refreshModeBuffer->m_xMax = g_ddrawBufferDesc.lPitch - 1;
	return 0;
}

// Releases the pending lock on g_ddrawBuffer.
// FUNCTION: MW2 0x10079240
// FUNCTION: MW2POWERVR 0x10005490
DDRAW_UNLOCK_INLINE static HRESULT DdrawUnlock(void)
{
	if (g_ddrawLocked) {
		g_ddrawResult = IDirectDrawSurface_Unlock(g_ddrawBuffer, g_ddrawBufferDesc.lpSurface);
#ifdef MW2_S3
		// The S3 edition's clears the lock first, then has the renderer finish drawing on the buffer.
		g_ddrawLocked = FALSE;
		g_refreshModeBuffer->m_buffer = NULL;
		FUN_1000f840(g_ddrawBufferDesc.lpSurface);
#else
		g_refreshModeBuffer->m_buffer = NULL;
		g_ddrawLocked = FALSE;
#endif
		return g_ddrawResult;
	}
	else {
		return DD_OK;
	}
}

// Restores the primary surface and g_ddrawBuffer when they were lost.
// FUNCTION: MW2 0x100792b0
__inline static HRESULT DdrawRestore(void)
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
