#include "supanim.h"

#include "debugprint.h"
#include "decomp.h"
#include "displaybackend.h"
#include "gamecd.h"
#include "mss.h"
#include "network.h"
#include "palettecolor.h"
#include "refreshmode.h"
#include "render.h"
#include "simmain.h"
#include "targeting.h"
#include "types.h"
#include "vfxa.h"
#include "window.h"

#include <stdio.h>
#include <windows.h>

// The dropship animation of the loading screen ("sup anim"), drawn by an AIL timer.

// GLOBAL: MW2 0x100a0150
// GLOBAL: MW2GBL 0x100b2d50
// GLOBAL: MW2MATROX 0x100a8650
// GLOBAL: MW23DFX 0x1024a850
// GLOBAL: MW2ATI 0x1017eca8
// GLOBAL: MW2S3 0x100614f0
// GLOBAL: MW2POWERVR 0x101a99c8
HTIMER g_supAnimTimer = -1;

// GLOBAL: MW2 0x100a0154
// GLOBAL: MW2GBL 0x100b2d54
// GLOBAL: MW2MATROX 0x100a8654
// GLOBAL: MW23DFX 0x1024a854
// GLOBAL: MW2ATI 0x1017ecac
// GLOBAL: MW2S3 0x100614f4
// GLOBAL: MW2POWERVR 0x101a99cc
void* g_supAnimBackdrop = NULL;

// GLOBAL: MW2 0x100a0158
// GLOBAL: MW2GBL 0x100b2d58
// GLOBAL: MW2MATROX 0x100a8658
// GLOBAL: MW23DFX 0x1024a858
// GLOBAL: MW2ATI 0x1017ecb0
// GLOBAL: MW2S3 0x100614f8
// GLOBAL: MW2POWERVR 0x101a99d0
void* g_supAnimShape = NULL;

// GLOBAL: MW2 0x100a015c
// GLOBAL: MW2GBL 0x100b2d5c
// GLOBAL: MW2MATROX 0x100a865c
// GLOBAL: MW23DFX 0x1024a85c
// GLOBAL: MW2ATI 0x1017ecb4
// GLOBAL: MW2S3 0x100614fc
// GLOBAL: MW2POWERVR 0x101a99d4
MechS32 g_supAnimFrameCount = 0;

// The command line's overrides of the backdrop and dropship file names (ProcessCmdLineArgs).

// GLOBAL: MW2 0x100a0160
// GLOBAL: MW2GBL 0x100b2d60
// GLOBAL: MW2MATROX 0x100a8660
// GLOBAL: MW23DFX 0x1024a860
// GLOBAL: MW2ATI 0x1017ecb8
// GLOBAL: MW2S3 0x10061500
// GLOBAL: MW2POWERVR 0x101a99d8
MechChar* g_supAnimBackdropName = NULL;

// GLOBAL: MW2 0x100a0164
// GLOBAL: MW2GBL 0x100b2d64
// GLOBAL: MW2MATROX 0x100a8664
// GLOBAL: MW23DFX 0x1024a864
// GLOBAL: MW2ATI 0x1017ecbc
// GLOBAL: MW2S3 0x10061504
// GLOBAL: MW2POWERVR 0x101a99dc
MechChar* g_supAnimShapeName = NULL;

// The first 16 palette entries in single player.
// GLOBAL: MW2 0x100a0168
// GLOBAL: MW2GBL 0x100b2d68
// GLOBAL: MW2MATROX 0x100a8668
// GLOBAL: MW23DFX 0x1024a868
// GLOBAL: MW2ATI 0x1017ecc0
// GLOBAL: MW2S3 0x10061508
// GLOBAL: MW2POWERVR 0x101a99e0
PaletteColor g_supAnimPalette[16] = {
	{0x00, 0x00, 0x00},
	{0x33, 0x34, 0x36},
	{0x27, 0x28, 0x2b},
	{0x20, 0x20, 0x23},
	{0x1b, 0x1c, 0x1e},
	{0x17, 0x19, 0x1b},
	{0x15, 0x15, 0x17},
	{0x12, 0x14, 0x16},
	{0x11, 0x12, 0x14},
	{0x10, 0x10, 0x12},
	{0x0e, 0x10, 0x12},
	{0x0d, 0x0e, 0x10},
	{0x0c, 0x0c, 0x0e},
	{0x09, 0x09, 0x0a},
	{0x04, 0x04, 0x05},
	{0x01, 0x01, 0x02},
};

// GLOBAL: MW2 0x100a0198
// GLOBAL: MW2GBL 0x100b2d98
// GLOBAL: MW2MATROX 0x100a8698
// GLOBAL: MW23DFX 0x1024a898
// GLOBAL: MW2ATI 0x1017ecf0
// GLOBAL: MW2S3 0x10061538
// GLOBAL: MW2POWERVR 0x101a9a10
MechS32 g_supAnimFrame = 0;

// GLOBAL: MW2 0x100bcd50
// GLOBAL: MW2GBL 0x100bb388
// GLOBAL: MW2MATROX 0x100c2068
// GLOBAL: MW23DFX 0x1007b058
// GLOBAL: MW2ATI 0x10070058
// GLOBAL: MW2S3 0x1007ca78
// GLOBAL: MW2POWERVR 0x1006e058
MechS32 g_supAnimY;

// GLOBAL: MW2 0x100bcd54
// GLOBAL: MW2GBL 0x100bb38c
// GLOBAL: MW2MATROX 0x100c2064
// GLOBAL: MW23DFX 0x1007b054
// GLOBAL: MW2ATI 0x10070054
// GLOBAL: MW2S3 0x1007ca7c
// GLOBAL: MW2POWERVR 0x1006e054
MechS32 g_supAnimX;

// GLOBAL: MW2 0x100bcd58
// GLOBAL: MW2GBL 0x100bb390
// GLOBAL: MW2MATROX 0x100c2038
// GLOBAL: MW23DFX 0x1007b028
// GLOBAL: MW2ATI 0x10070028
// GLOBAL: MW2S3 0x1007ca80
// GLOBAL: MW2POWERVR 0x1006e028
PANE g_supAnimTarget;

// GLOBAL: MW2 0x100bcd70
// GLOBAL: MW2GBL 0x100bb3a8
// GLOBAL: MW2MATROX 0x100c2050
// GLOBAL: MW23DFX 0x1007b040
// GLOBAL: MW2ATI 0x10070040
// GLOBAL: MW2S3 0x1007ca98
// GLOBAL: MW2POWERVR 0x1006e040
WINDOW g_supAnimBuffer;

#ifdef MW2_STDCALL_TIMER_CALLBACK
void __stdcall SupAnimTimerCallback(MechU32 p_user);
#else
void SupAnimTimerCallback(void);
#endif

// Starts the dropship loading screen: loads the backdrop (<drive>:\\launch\\supanm6.shp, netmech6.shp
// in a network game, or the command line's override), from the CD if it isn't on the current
// drive, draws it and fades its palette in (slowly with p_slowFade), then loads the dropship
// (launch6.shp) and animates it on an AIL timer. On the DisplayDib back end it draws one frame.
// FUNCTION: MW2 0x10003a70
// FUNCTION: MW2GBL 0x10053810
// FUNCTION: MW2MATROX 0x1004a510
// FUNCTION: MW23DFX 0x100073b0
// FUNCTION: MW2ATI 0x10007d10
// FUNCTION: MW2S3 0x100084d0
// FUNCTION: MW2POWERVR 0x10006a50
void StartSupAnim(MechS32 p_slowFade)
{
	MechChar backdropPath[256];
	MechChar shapePath[256];
	PaletteColor* palette;
	MechChar drive;

	GetCurrentDirectory(sizeof(backdropPath), backdropPath);
	drive = backdropPath[0];
	if (g_supAnimBackdropName == NULL || *g_supAnimBackdropName == '\0') {
		sprintf(backdropPath, "%c:%s\\%s6.%s", drive, "launch", !g_isNetworkGame ? "supanm" : "netmech", "shp");
	}
	else {
		sprintf(
			backdropPath,
			"%c:%s\\%s6.%s",
			drive,
			"launch",
			!g_isNetworkGame ? g_supAnimBackdropName : "netmech",
			"shp"
		);
	}

	if (g_supAnimShapeName == NULL || *g_supAnimShapeName == '\0') {
		sprintf(shapePath, "%c:%s\\%s6.%s", drive, "launch", "launch", "shp");
	}
	else {
		sprintf(shapePath, "%c:%s\\%s6.%s", drive, "launch", g_supAnimShapeName, "shp");
	}

	g_supAnimBackdrop = FILE_read(backdropPath, NULL);
	if (g_supAnimBackdrop == NULL) {
		drive = FindGameCdDrive();
		if (!drive) {
			return;
		}

		backdropPath[0] = shapePath[0] = drive;
		g_supAnimBackdrop = FILE_read(backdropPath, NULL);
		if (g_supAnimBackdrop == NULL) {
			return;
		}
	}

	palette = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 0x100 * sizeof(PaletteColor));
	if (palette == NULL) {
		return;
	}

	g_currentDisplayBackend->m_setPalette(0, 0x100, palette, 1);
	g_supAnimBuffer = g_mainPixelBuffer;
	g_supAnimBuffer.m_xMax = g_refreshModeWidth - 1;
	g_supAnimBuffer.m_yMax = g_refreshModeHeight - 1;
	g_supAnimTarget.m_window = &g_supAnimBuffer;
	g_supAnimTarget.m_x1 = g_refreshModeWidth - 1;
	g_supAnimTarget.m_y1 = g_refreshModeHeight - 1;
	if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) != 0) {
		DebugPrint("StartSupanim: LockDisplayBuffer failed.\n");
		return;
	}

	g_supAnimBuffer.m_buffer = g_mainPixelBuffer.m_buffer;
	VFX_pane_wipe(&g_supAnimTarget, 0);
	VFX_shape_draw(&g_supAnimTarget, g_supAnimBackdrop, 0, 0, 0);
	if (g_windowActive) {
		g_currentRefreshMode->m_flip();
	}

	VFX_shape_palette(g_supAnimBackdrop, 0, (MechU8*) palette);
	if (p_slowFade) {
		g_currentDisplayBackend->m_blendPalettes(palette, 30);
	}
	else {
		g_currentDisplayBackend->m_blendPalettes(palette, 2);
	}

#ifndef MW2_SUPANIM_KEEPS_PALETTE
	if (!g_isNetworkGame) {
		g_currentDisplayBackend->m_setPalette(0, 16, g_supAnimPalette, 1);
	}

#endif
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, palette);
	g_supAnimShape = FILE_read(shapePath, NULL);
	if (g_supAnimShape == NULL) {
		return;
	}

	g_supAnimFrameCount = VFX_shape_count(g_supAnimShape);
	g_supAnimX = (g_supAnimTarget.m_x1 - g_supAnimTarget.m_x0) * 0.55;
	g_supAnimY = 0;
	if (!g_isNetworkGame && g_currentDisplayBackend->m_id == c_displayBackendDisplayDib) {
		if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
			VFX_shape_draw(&g_supAnimTarget, g_supAnimBackdrop, 0, 0, 0);
			VFX_shape_draw(&g_supAnimTarget, g_supAnimShape, 0, g_supAnimX, g_supAnimY);
			if (g_windowActive) {
				g_currentRefreshMode->m_flip();
			}
		}
	}
	else if (!g_isNetworkGame) {
		g_supAnimTimer = AIL_register_timer((AILTIMERCB) SupAnimTimerCallback);
		AIL_set_timer_period(g_supAnimTimer, 330000);
		AIL_start_timer(g_supAnimTimer);
	}
}

// Draws the backdrop and the next frame of the dropship, and presents them. AIL calls it as a
// timer callback, which takes an argument this one doesn't pop (1.1).
// FUNCTION: MW2 0x10003f3d
// FUNCTION: MW2GBL 0x10053cba
// FUNCTION: MW2MATROX 0x1004a9b0
// FUNCTION: MW23DFX 0x10007730
// FUNCTION: MW2ATI 0x10008090
// FUNCTION: MW2S3 0x10008850
// FUNCTION: MW2POWERVR 0x10006dd0
#ifdef MW2_STDCALL_TIMER_CALLBACK
// The Matrox edition's pops the timer's argument (AIL's callbacks are __stdcall); the 3Dfx
// edition's is 1.1's.
void __stdcall SupAnimTimerCallback(MechU32 p_user)
#else
void SupAnimTimerCallback(void)
#endif
{
	if (!g_displayReady || !g_supAnimBackdrop || !g_supAnimShape || g_supAnimFrameCount < 2) {
		return;
	}

	if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
		g_supAnimBuffer.m_buffer = g_mainPixelBuffer.m_buffer;
		VFX_pane_wipe(&g_supAnimTarget, 0);
		VFX_shape_draw(&g_supAnimTarget, g_supAnimBackdrop, 0, 0, 0);
		VFX_shape_draw(&g_supAnimTarget, g_supAnimShape, g_supAnimFrame, g_supAnimX, g_supAnimY);
		if (g_windowActive) {
			g_currentRefreshMode->m_flip();
		}

		g_supAnimFrame = (g_supAnimFrame + 1) % g_supAnimFrameCount;
	}
}

// Releases the timer and frees the backdrop and the dropship.
// FUNCTION: MW2 0x10004031
// FUNCTION: MW2GBL 0x10053dae
// FUNCTION: MW2MATROX 0x1004aaa6
// FUNCTION: MW23DFX 0x10007800
// FUNCTION: MW2ATI 0x10008160
// FUNCTION: MW2S3 0x10008920
// FUNCTION: MW2POWERVR 0x10006ea0
void StopSupAnim(void)
{
	if (g_supAnimTimer != -1) {
		AIL_release_timer_handle(g_supAnimTimer);
		g_supAnimTimer = -1;
	}

	if (g_supAnimBackdrop) {
		MEM_free_lock(g_supAnimBackdrop);
		g_supAnimBackdrop = NULL;
	}

	if (g_supAnimShape) {
		MEM_free_lock(g_supAnimShape);
		g_supAnimShape = NULL;
		g_supAnimFrameCount = 0;
	}
}
