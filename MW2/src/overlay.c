#include "overlay.h"

#include "clock.h"
#include "decomp.h"
#include "eyepoint.h"
#include "fixedfloat.h"
#include "gamekeys.h"
#include "loadres.h"
#include "mw2prj.h"
#include "palette.h"
#include "point.h"
#include "polydraw.h"
#include "render.h"
#include "screenscale.h"
#include "shape.h"
#include "shapelists.h"
#include "targeting.h"
#include "timedoverlays.h"
#include "types.h"

#include <stdio.h>
#include <string.h>

#ifdef MW2_VFX16
#include "accel/vfx16.h"
#endif

// The debug overlays' text color (the Matrox edition's palette has it elsewhere).
#ifdef MW2_DEBUG_OVERLAY_COLOR
#define OVERLAY_TEXT_COLOR 0x52
#else
#define OVERLAY_TEXT_COLOR 0x4f
#endif

// The debug overlays: frame rate, scene counts, the eyepoint's position, memory and cache
// sizes, the palette and an activity spinner. Each has a flag the debug keys toggle
// (GAMEKEY.MAP's TOGGLE_FRAMERATE, TOGGLE_SCENE_INFO, TOGGLE_EYE_POSITION, TOGGLE_MEM_INFO,
// TOGGLE_PALETTE); the _MAIN keys also set a second flag that draws the text on the main
// screen, and a third remembers that the text is up, to blank it once the flag goes off.
// The rest went to a monochrome debug screen (80x25, as in the launcher's debug.c), which
// the release build only formats for: the drawing is compiled out.

// The spaces the Hide functions blank a readout with.
// GLOBAL: MW2 0x100a9478
// GLOBAL: MW2GBL 0x100b5a90
// GLOBAL: MW2MATROX 0x100bcbf0
// GLOBAL: MW23DFX 0x10259430
// GLOBAL: MW2ATI 0x1018d730
// GLOBAL: MW2S3 0x10065300
// GLOBAL: MW2POWERVR 0x101b89a0
MechChar g_blankText[] = "                                 ";

// GLOBAL: MW2 0x100a949c
// GLOBAL: MW2GBL 0x100b5ab4
// GLOBAL: MW2MATROX 0x100bcc14
// GLOBAL: MW23DFX 0x10259454
// GLOBAL: MW2ATI 0x1018d754
// GLOBAL: MW2S3 0x10065324
// GLOBAL: MW2POWERVR 0x101b89c4
MechS32 g_showPalette = 0;

// GLOBAL: MW2 0x100a94a0
MechS32 g_unk0x100a94a0 = 0;

// The row MonoPrint last wrote to; MonoClear resets it and its neighbours to the top row.
// GLOBAL: MW2 0x100a94a4
// GLOBAL: MW2GBL 0x100b5abc
// GLOBAL: MW2MATROX 0x100bcc1c
// GLOBAL: MW23DFX 0x1025945c
// GLOBAL: MW2ATI 0x1018d75c
// GLOBAL: MW2S3 0x1006532c
// GLOBAL: MW2POWERVR 0x101b89cc
MechS32 g_monoLastRow = 3;

// GLOBAL: MW2 0x100a94a8
// GLOBAL: MW2GBL 0x100b5ac0
// GLOBAL: MW2MATROX 0x100bcc20
// GLOBAL: MW23DFX 0x10259460
// GLOBAL: MW2ATI 0x1018d760
// GLOBAL: MW2S3 0x10065330
// GLOBAL: MW2POWERVR 0x101b89d0
MechS32 g_unk0x100a94a8 = 3;

// GLOBAL: MW2 0x100a94ac
// GLOBAL: MW2GBL 0x100b5ac4
// GLOBAL: MW2MATROX 0x100bcc24
// GLOBAL: MW23DFX 0x10259464
// GLOBAL: MW2ATI 0x1018d764
// GLOBAL: MW2S3 0x10065334
// GLOBAL: MW2POWERVR 0x101b89d4
MechChar g_unk0x100a94ac[4] = "";

// GLOBAL: MW2 0x100a94b0
// GLOBAL: MW2GBL 0x100b5ac8
// GLOBAL: MW2MATROX 0x100bcc28
// GLOBAL: MW23DFX 0x10259468
// GLOBAL: MW2ATI 0x1018d768
// GLOBAL: MW2S3 0x10065338
// GLOBAL: MW2POWERVR 0x101b89d8
MechS32 g_frameRateShown = 0;

// GLOBAL: MW2 0x100a94b4
// GLOBAL: MW2GBL 0x100b5acc
// GLOBAL: MW2MATROX 0x100bcc2c
// GLOBAL: MW23DFX 0x1025946c
// GLOBAL: MW2ATI 0x1018d76c
// GLOBAL: MW2S3 0x1006533c
// GLOBAL: MW2POWERVR 0x101b89dc
MechS32 g_showFrameRate = 0;

// GLOBAL: MW2 0x100a94b8
// GLOBAL: MW2GBL 0x100b5ad0
// GLOBAL: MW2MATROX 0x100bcc30
// GLOBAL: MW23DFX 0x10259470
// GLOBAL: MW2ATI 0x1018d770
// GLOBAL: MW2S3 0x10065340
// GLOBAL: MW2POWERVR 0x101b89e0
MechS32 g_showFrameRateMain = 0;

// GLOBAL: MW2 0x100a94bc
MechS32 g_unk0x100a94bc = 0;

// Where the _MAIN readouts are drawn, in 16.16 fractions of the screen until
// ScaleOverlayPositions scales them to pixels.
// GLOBAL: MW2 0x100a94c0
// GLOBAL: MW2GBL 0x100b5ad8
// GLOBAL: MW2MATROX 0x100bcc38
// GLOBAL: MW23DFX 0x10259478
// GLOBAL: MW2ATI 0x1018d778
// GLOBAL: MW2S3 0x10065348
// GLOBAL: MW2POWERVR 0x101b89e8
Point g_frameRateOrigin = {0x51f, 0x147b};

// GLOBAL: MW2 0x100a94c8
// GLOBAL: MW2GBL 0x100b5ae0
// GLOBAL: MW2MATROX 0x100bcc40
// GLOBAL: MW23DFX 0x10259480
// GLOBAL: MW2ATI 0x1018d780
// GLOBAL: MW2S3 0x10065350
// GLOBAL: MW2POWERVR 0x101b89f0
Point g_memInfoOrigin = {0x51f, 0x2148};

// GLOBAL: MW2 0x100a94d0
// GLOBAL: MW2GBL 0x100b5ae8
// GLOBAL: MW2MATROX 0x100bcc48
// GLOBAL: MW23DFX 0x10259488
// GLOBAL: MW2ATI 0x1018d788
// GLOBAL: MW2S3 0x10065358
// GLOBAL: MW2POWERVR 0x101b89f8
Point g_eyePositionOrigin = {0xccd, 0xb333};

// GLOBAL: MW2 0x100a94d8
// GLOBAL: MW2GBL 0x100b5af0
// GLOBAL: MW2MATROX 0x100bcc50
// GLOBAL: MW23DFX 0x10259490
// GLOBAL: MW2ATI 0x1018d790
// GLOBAL: MW2S3 0x10065360
// GLOBAL: MW2POWERVR 0x101b8a00
MechS32 g_showSceneInfo = 0;

// GLOBAL: MW2 0x100a94dc
// GLOBAL: MW2GBL 0x100b5af4
// GLOBAL: MW2MATROX 0x100bcc54
// GLOBAL: MW23DFX 0x10259494
// GLOBAL: MW2ATI 0x1018d794
// GLOBAL: MW2S3 0x10065364
// GLOBAL: MW2POWERVR 0x101b8a04
MechS32 g_showSceneInfoMain = 0;

// GLOBAL: MW2 0x100a94e0
// GLOBAL: MW2GBL 0x100b5af8
// GLOBAL: MW2MATROX 0x100bcc58
// GLOBAL: MW23DFX 0x10259498
// GLOBAL: MW2ATI 0x1018d798
// GLOBAL: MW2S3 0x10065368
// GLOBAL: MW2POWERVR 0x101b8a08
MechS32 g_sceneInfoShown = 0;

// GLOBAL: MW2 0x100a94e4
// GLOBAL: MW2GBL 0x100b5afc
// GLOBAL: MW2MATROX 0x100bcc5c
// GLOBAL: MW23DFX 0x1025949c
// GLOBAL: MW2ATI 0x1018d79c
// GLOBAL: MW2S3 0x1006536c
// GLOBAL: MW2POWERVR 0x101b8a0c
MechS32 g_showEyePosition = 0;

// GLOBAL: MW2 0x100a94e8
// GLOBAL: MW2GBL 0x100b5b00
// GLOBAL: MW2MATROX 0x100bcc60
// GLOBAL: MW23DFX 0x102594a0
// GLOBAL: MW2ATI 0x1018d7a0
// GLOBAL: MW2S3 0x10065370
// GLOBAL: MW2POWERVR 0x101b8a10
MechS32 g_showEyePositionMain = 0;

// GLOBAL: MW2 0x100a94ec
// GLOBAL: MW2GBL 0x100b5b04
MechS32 g_eyePositionShown = 0;

// GLOBAL: MW2 0x100a94f0
// GLOBAL: MW2GBL 0x100b5b08
// GLOBAL: MW2MATROX 0x100bcc68
// GLOBAL: MW23DFX 0x102594a8
// GLOBAL: MW2ATI 0x1018d7a8
// GLOBAL: MW2S3 0x10065378
// GLOBAL: MW2POWERVR 0x101b8a18
MechS32 g_showMemInfo = 1;

// GLOBAL: MW2 0x100a94f4
// GLOBAL: MW2GBL 0x100b5b0c
// GLOBAL: MW2MATROX 0x100bcc6c
// GLOBAL: MW23DFX 0x102594ac
// GLOBAL: MW2ATI 0x1018d7ac
// GLOBAL: MW2S3 0x1006537c
// GLOBAL: MW2POWERVR 0x101b8a1c
MechS32 g_showMemInfoMain = 0;

// GLOBAL: MW2 0x100a94f8
// GLOBAL: MW2GBL 0x100b5b10
// GLOBAL: MW2MATROX 0x100bcc70
// GLOBAL: MW23DFX 0x102594b0
// GLOBAL: MW2ATI 0x1018d7b0
// GLOBAL: MW2S3 0x10065380
// GLOBAL: MW2POWERVR 0x101b8a20
MechS32 g_memInfoShown = 0;

// GLOBAL: MW2 0x100a94fc
// GLOBAL: MW2GBL 0x100b5b14
// GLOBAL: MW2MATROX 0x100bcc74
// GLOBAL: MW23DFX 0x102594b4
// GLOBAL: MW2ATI 0x1018d7b4
// GLOBAL: MW2S3 0x10065384
// GLOBAL: MW2POWERVR 0x101b8a24
MechS32 g_showCacheInfo = 0;

// GLOBAL: MW2 0x100a9500
// GLOBAL: MW2GBL 0x100b5b18
// GLOBAL: MW2MATROX 0x100bcc78
// GLOBAL: MW23DFX 0x102594b8
// GLOBAL: MW2ATI 0x1018d7b8
// GLOBAL: MW2S3 0x10065388
// GLOBAL: MW2POWERVR 0x101b8a28
MechS32 g_cacheInfoShown = 0;

// GLOBAL: MW2 0x100a9504
// GLOBAL: MW2GBL 0x100b5b1c
// GLOBAL: MW2MATROX 0x100bcc7c
// GLOBAL: MW23DFX 0x102594bc
// GLOBAL: MW2ATI 0x1018d7bc
// GLOBAL: MW2S3 0x1006538c
// GLOBAL: MW2POWERVR 0x101b8a2c
MechS32 g_showSpinner = 1;

// The spinner StepSpinner steps through: the CP437 arrows up, left, down and right.
// GLOBAL: MW2 0x100a9508
// GLOBAL: MW2GBL 0x100b5b20
// GLOBAL: MW2MATROX 0x100bcc80
MechChar g_spinnerArrows[4] = {0x1e, 0x11, 0x1f, 0x10};

// A repeated cache dump (DUMP_CACHE_REPEAT, whose key does nothing in this build) every
// second.
// GLOBAL: MW2 0x100a950c
// GLOBAL: MW2GBL 0x100b5b24
// GLOBAL: MW2MATROX 0x100bcc84
// GLOBAL: MW23DFX 0x102594c4
// GLOBAL: MW2ATI 0x1018d7c4
// GLOBAL: MW2S3 0x10065394
// GLOBAL: MW2POWERVR 0x101b8a34
MechS32 g_nextCacheDump = 0;

// GLOBAL: MW2 0x100a9510
// GLOBAL: MW2GBL 0x100b5b28
// GLOBAL: MW2MATROX 0x100bcc88
// GLOBAL: MW23DFX 0x102594c8
// GLOBAL: MW2ATI 0x1018d7c8
// GLOBAL: MW2S3 0x10065398
// GLOBAL: MW2POWERVR 0x101b8a38
MechS32 g_dumpCacheRepeat = 0;

// GLOBAL: MW2 0x100a9514
// GLOBAL: MW2GBL 0x100b5b2c
// GLOBAL: MW2MATROX 0x100bcc8c
// GLOBAL: MW23DFX 0x102594cc
// GLOBAL: MW2ATI 0x1018d7cc
// GLOBAL: MW2S3 0x1006539c
// GLOBAL: MW2POWERVR 0x101b8a3c
MechS32 g_unk0x100a9514 = 0;

// GLOBAL: MW2 0x100a9518
// GLOBAL: MW2GBL 0x100b5b30
// GLOBAL: MW2MATROX 0x100bcc90
// GLOBAL: MW23DFX 0x102594d0
// GLOBAL: MW2ATI 0x1018d7d0
// GLOBAL: MW2S3 0x100653a0
// GLOBAL: MW2POWERVR 0x101b8a40
MechS32 g_unk0x100a9518 = 0;

// The frame rate, in whole frames per second and tenths, over the last ten frames.
// GLOBAL: MW2 0x100a951c
// GLOBAL: MW2GBL 0x100b5b34
// GLOBAL: MW2MATROX 0x100bcc94
// GLOBAL: MW23DFX 0x102594d4
// GLOBAL: MW2ATI 0x1018d7d4
// GLOBAL: MW2S3 0x100653a4
// GLOBAL: MW2POWERVR 0x101b8a44
MechS32 g_frameRate = 0;

// GLOBAL: MW2 0x100a9520
// GLOBAL: MW2GBL 0x100b5b38
// GLOBAL: MW2MATROX 0x100bcc98
// GLOBAL: MW23DFX 0x102594d8
// GLOBAL: MW2ATI 0x1018d7d8
// GLOBAL: MW2S3 0x100653a8
// GLOBAL: MW2POWERVR 0x101b8a48
MechS32 g_frameRateTenths = 0;

// GLOBAL: MW2 0x100a9524
// GLOBAL: MW2GBL 0x100b5b3c
// GLOBAL: MW2MATROX 0x100bcc9c
// GLOBAL: MW23DFX 0x102594dc
// GLOBAL: MW2ATI 0x1018d7dc
// GLOBAL: MW2S3 0x100653ac
// GLOBAL: MW2POWERVR 0x101b8a4c
MechS32 g_frameRateTime = 0;

// GLOBAL: MW2 0x100a9528
// GLOBAL: MW2GBL 0x100b5b40
// GLOBAL: MW2MATROX 0x100bcca0
// GLOBAL: MW23DFX 0x102594e0
// GLOBAL: MW2ATI 0x1018d7e0
// GLOBAL: MW2S3 0x100653b0
// GLOBAL: MW2POWERVR 0x101b8a50
MechS32 g_frameRateFrames = 0;

// GLOBAL: MW2 0x100a952c
// GLOBAL: MW2GBL 0x100b5b44
// GLOBAL: MW2MATROX 0x100bcca4
// GLOBAL: MW23DFX 0x102594e4
// GLOBAL: MW2ATI 0x1018d7e4
// GLOBAL: MW2S3 0x100653b4
MechS32 g_spinnerDelay = 4;

// GLOBAL: MW2 0x100a9530
// GLOBAL: MW2GBL 0x100b5b48
// GLOBAL: MW2MATROX 0x100bcca8
// GLOBAL: MW23DFX 0x102594e8
// GLOBAL: MW2ATI 0x1018d7e8
// GLOBAL: MW2S3 0x100653b8
MechS32 g_spinnerArrow = 3;

// The cursor of MonoPrint.
// GLOBAL: MW2 0x100a9534
// GLOBAL: MW2GBL 0x100b5b4c
// GLOBAL: MW2MATROX 0x100bccac
// GLOBAL: MW23DFX 0x102594ec
// GLOBAL: MW2ATI 0x1018d7ec
// GLOBAL: MW2S3 0x100653bc
// GLOBAL: MW2POWERVR 0x101b8a5c
MechS32 g_monoRow = 3;

// GLOBAL: MW2 0x100a9538
// GLOBAL: MW2GBL 0x100b5b50
// GLOBAL: MW2MATROX 0x100bccb0
// GLOBAL: MW23DFX 0x102594f0
// GLOBAL: MW2ATI 0x1018d7f0
// GLOBAL: MW2S3 0x100653c0
// GLOBAL: MW2POWERVR 0x101b8a60
MechS32 g_monoColumn = 0;

// The debug build drew each text it formatted while g_monoEnabled was set; the release
// build keeps only the test.
#define DRAW_DEBUG_TEXT()                                                                                              \
	do {                                                                                                               \
		if (g_monoEnabled) {                                                                                           \
		}                                                                                                              \
	} while (0)

// The totals CountSceneShape adds up for ShowSceneInfo.
// GLOBAL: MW2 0x100bea00
// GLOBAL: MW2GBL 0x100bb568
// GLOBAL: MW2MATROX 0x101256c8
// GLOBAL: MW23DFX 0x1007d720
// GLOBAL: MW2ATI 0x10072720
// GLOBAL: MW2S3 0x1007cb0c
// GLOBAL: MW2POWERVR 0x10070768
static MechS32 g_sceneVertexCount;

// GLOBAL: MW2 0x100bea04
// GLOBAL: MW2GBL 0x100bb56c
// GLOBAL: MW2MATROX 0x101256d4
// GLOBAL: MW23DFX 0x1007d72c
// GLOBAL: MW2ATI 0x1007272c
// GLOBAL: MW2S3 0x1007cb10
// GLOBAL: MW2POWERVR 0x10070774
static MechS32 g_sceneMemory;

// GLOBAL: MW2 0x100bea08
// GLOBAL: MW2GBL 0x100bb570
// GLOBAL: MW2MATROX 0x101256d0
// GLOBAL: MW23DFX 0x1007d728
// GLOBAL: MW2ATI 0x10072728
// GLOBAL: MW2S3 0x1007cb14
// GLOBAL: MW2POWERVR 0x10070770
static MechS32 g_sceneShapeCount;

// GLOBAL: MW2 0x100bea0c
// GLOBAL: MW2GBL 0x100bb574
// GLOBAL: MW2MATROX 0x101256cc
// GLOBAL: MW23DFX 0x1007d724
// GLOBAL: MW2ATI 0x10072724
// GLOBAL: MW2S3 0x1007cb18
// GLOBAL: MW2POWERVR 0x1007076c
static MechS32 g_sceneFaceCount;

// Set when the monochrome debug screen is in use; the AI logs its state to it too. Only the
// /M switch's InitializeMono could have set it, in a branch compiled out.
// GLOBAL: MW2 0x100e9630
// GLOBAL: MW2GBL 0x100bc1e4
// GLOBAL: MW2MATROX 0x1012b980
// GLOBAL: MW23DFX 0x10159d9c
// GLOBAL: MW2ATI 0x1008dd9c
// GLOBAL: MW2S3 0x100b1db0
// GLOBAL: MW2POWERVR 0x1008437c
MechS32 g_monoEnabled;

// A line of spaces, with its terminator.
// GLOBAL: MW2 0x100e9640
// GLOBAL: MW2GBL 0x100bc1f0
// GLOBAL: MW2MATROX 0x1012b990
// GLOBAL: MW23DFX 0x10159da0
// GLOBAL: MW2ATI 0x1008dda0
// GLOBAL: MW2S3 0x100b1dc0
// GLOBAL: MW2POWERVR 0x10084380
MechChar g_monoBlankLine[0x50];

// GLOBAL: MW2 0x100e9690
// GLOBAL: MW2GBL 0x100bc240
// GLOBAL: MW2MATROX 0x1012b9e0
// GLOBAL: MW23DFX 0x10159df0
// GLOBAL: MW2ATI 0x1008ddf0
// GLOBAL: MW2S3 0x100b1e10
// GLOBAL: MW2POWERVR 0x100843d0
MechChar g_monoBlankLineEnd;

// Draws each debug overlay that is on, or blanks it once it goes off; called every frame.
// FUNCTION: MW2 0x10058750
// FUNCTION: MW2GBL 0x10073470
// FUNCTION: MW2MATROX 0x1008dbe0
// FUNCTION: MW23DFX 0x1003c550
// FUNCTION: MW2ATI 0x1003a520
// FUNCTION: MW2S3 0x10020d30
// FUNCTION: MW2POWERVR 0x10035360
void DrawDebugOverlays(void)
{
	if (g_showPalette) {
		DrawPaletteGrid();
	}
	if (g_showSpinner) {
		StepSpinner();
	}

	if (g_showFrameRate) {
		ShowFrameRate();
	}
	else if (g_frameRateShown) {
		HideFrameRate();
	}

	if (g_showSceneInfo) {
		ShowSceneInfo();
	}
	else if (g_sceneInfoShown) {
		HideSceneInfo();
	}

#ifndef MW2_NO_EYE_POSITION_OVERLAY
	if (g_showEyePosition) {
		ShowEyePosition(g_eyepoint);
	}
	else if (g_eyePositionShown) {
		HideEyePosition();
	}

#endif
	if (g_showMemInfo) {
		ShowMemInfo();
	}
	else if (g_memInfoShown) {
		HideMemInfo();
	}

	if (g_showCacheInfo) {
		ShowCacheInfo();
	}
	else if (g_cacheInfoShown) {
		HideCacheInfo();
	}

	if (g_unk0x100a9518) {
		g_unk0x100a9514 = 1;
	}
	if (g_dumpCacheRepeat && g_nextCacheDump < g_currentClock) {
		g_nextCacheDump = g_currentClock + 0xb5;
		FUN_1001a521(g_mw2PrjHandle);
	}
}

// Draws the 256 palette colours as a grid of 16 swatches per row.
// FUNCTION: MW2 0x100588a7
// FUNCTION: MW2GBL 0x100735c7
// FUNCTION: MW2MATROX 0x1008dd05
// FUNCTION: MW23DFX 0x1003c630
// FUNCTION: MW2ATI 0x1003a600
// FUNCTION: MW2S3 0x10020e10
// FUNCTION: MW2POWERVR 0x10035440
void DrawPaletteGrid(void)
{
	MechS32 x;
	MechS32 y;
	MechS32 color;
	MechS32 i;

	x = 10;
	y = 10;
	for (color = 0; color < 0x100; color++) {
		for (i = 0; i < 2; i++) {
			VFX_line_draw(&g_currentPane, x, i + y, x + 3, i + y, 0, PIXEL_COLOR(color));
		}

		if ((color + 1) % 16 == 0) {
			x = 10;
			y += 2;
		}
		else {
			x += 4;
		}
	}
}

// Fills g_monoBlankLine (the /M switch's call; the debug build also turned the screen on and
// stopped the mission timer here).
// FUNCTION: MW2 0x10058958
// FUNCTION: MW2GBL 0x10073678
// FUNCTION: MW2MATROX 0x1008ddbf
// FUNCTION: MW23DFX 0x1003c6a0
// FUNCTION: MW2ATI 0x1003a670
// FUNCTION: MW2S3 0x10020e80
// FUNCTION: MW2POWERVR 0x100354b0
void InitializeMono(void)
{
	MechS32 i;

	if (FALSE) {
		g_monoEnabled = TRUE;
		g_missionTimerStopped = TRUE;
	}

	for (i = 0; i < 0x50; i++) {
		g_monoBlankLine[i] = ' ';
	}
	g_monoBlankLineEnd = '\0';
}

// FUNCTION: MW2 0x100589ae
// FUNCTION: MW2GBL 0x100736ce
// FUNCTION: MW2MATROX 0x1008de15
// FUNCTION: MW23DFX 0x1003c6c0
// FUNCTION: MW2ATI 0x1003a690
// FUNCTION: MW2S3 0x10020ea0
// FUNCTION: MW2POWERVR 0x100354d0
void ShowFrameRate(void)
{
	MechChar text[64];
	MechS32 rate;

	if (!g_monoEnabled && !g_showFrameRateMain) {
		return;
	}

	g_frameRateFrames++;
	g_frameRateTime += g_deltaTime;
	if (g_frameRateFrames >= 10) {
		if (g_frameRateTime > 0) {
			rate = (g_frameRateFrames * 0x712) / g_frameRateTime;
			g_frameRate = rate / 10;
			g_frameRateTenths = rate - g_frameRate * 10;
			sprintf(text, "Framerate %2.2ld.%1.1ld", g_frameRate, g_frameRateTenths);
			DRAW_DEBUG_TEXT();
		}

		g_frameRateFrames = 0;
		g_frameRateTime = 0;
	}

	if (g_showFrameRateMain && g_frameRate > 0) {
		sprintf(text, "%ld.%ld", g_frameRate, g_frameRateTenths);
		DrawTextBox(OVERLAY_TEXT_COLOR, 1, text, g_frameRateOrigin.m_x, g_frameRateOrigin.m_y);
	}

	g_frameRateShown = 1;
}

// FUNCTION: MW2 0x10058ae5
// FUNCTION: MW2GBL 0x10073805
// FUNCTION: MW2MATROX 0x1008df4c
// FUNCTION: MW23DFX 0x1003c7c0
// FUNCTION: MW2ATI 0x1003a790
// FUNCTION: MW2S3 0x10020fa0
// FUNCTION: MW2POWERVR 0x100355d0
void HideFrameRate(void)
{
	MechChar text[16];
	MechChar format[16];

	sprintf(format, "%%%d.%ds", 0xe, 0xe);
	sprintf(text, format, g_blankText);
	DRAW_DEBUG_TEXT();
	g_frameRateShown = 0;
}

// FUNCTION: MW2 0x10058b34
// FUNCTION: MW2GBL 0x10073854
// FUNCTION: MW2MATROX 0x1008df9b
// FUNCTION: MW23DFX 0x1003c800
// FUNCTION: MW2ATI 0x1003a7d0
// FUNCTION: MW2S3 0x10020fe0
// FUNCTION: MW2POWERVR 0x10035610
void ShowSceneInfo(void)
{
	MechChar text[80];

	if (!g_monoEnabled && !g_showSceneInfoMain) {
		return;
	}

	g_sceneShapeCount = g_sceneVertexCount = g_sceneFaceCount = g_sceneMemory = 0;
	ForEachShape(g_sceneShapes, CountSceneShape);
	ForEachShape(g_hiddenShapes, CountSceneShape);

	if (g_monoEnabled) {
		if (!g_sceneInfoShown) {
			if (g_monoEnabled) {
				DRAW_DEBUG_TEXT();
			}

			if (g_showSceneInfoMain) {
				sprintf(
					text,
					"Objects: %d  Polygons: %d  Vertices: %d Curpolys: %d",
					g_sceneShapeCount,
					g_sceneFaceCount,
					g_sceneVertexCount,
					g_drawnPolygonCount
				);
				ShowInGameMessage(text, 1, 0xb5, 0x50);
			}

			g_sceneInfoShown = 1;
		}

		if (!g_monoEnabled) {
			return;
		}

		sprintf(text, "%4.4d", g_sceneShapeCount);
		DRAW_DEBUG_TEXT();
		sprintf(text, "%4.4d", g_sceneVertexCount);
		DRAW_DEBUG_TEXT();
		sprintf(text, "%4.4d", g_sceneFaceCount);
		DRAW_DEBUG_TEXT();
		sprintf(text, "%4.4d", g_sceneMemory);
		DRAW_DEBUG_TEXT();
	}
}

// FUNCTION: MW2 0x10058cda
// FUNCTION: MW2GBL 0x100739fa
// FUNCTION: MW2MATROX 0x1008e141
// FUNCTION: MW23DFX 0x1003c940
// FUNCTION: MW2ATI 0x1003a910
// FUNCTION: MW2S3 0x10021130
// FUNCTION: MW2POWERVR 0x10035750
void HideSceneInfo(void)
{
	MechChar text[32];
	MechChar format[32];

	sprintf(format, "%%%d.%ds", 0x1e, 0x1e);
	sprintf(text, format, g_blankText);
	DRAW_DEBUG_TEXT();
	DRAW_DEBUG_TEXT();
	g_sceneInfoShown = 0;
}

// Adds a shape's counts to the totals ShowSceneInfo shows.
// FUNCTION: MW2 0x10058d36
// FUNCTION: MW2GBL 0x10073a56
// FUNCTION: MW2MATROX 0x1008e19d
// FUNCTION: MW23DFX 0x1003c980
// FUNCTION: MW2ATI 0x1003a950
// FUNCTION: MW2S3 0x10021170
// FUNCTION: MW2POWERVR 0x10035790
void CountSceneShape(Shape* p_shape)
{
	MechS32 vertexCount;
	MechS32 faceCount;

	vertexCount = 0;
	faceCount = 0;
	GetModelCounts(p_shape, &vertexCount, &faceCount);

	g_sceneShapeCount++;
	g_sceneVertexCount += vertexCount;
	g_sceneFaceCount += faceCount;
	g_sceneMemory += GetShapeMemorySize(p_shape);
}

// FUNCTION: MW2 0x10058d90
// FUNCTION: MW2GBL 0x10073ab0
void ShowEyePosition(Eyepoint* p_eyepoint)
{
	MechChar text[132];

	if (!g_showEyePositionMain && !g_monoEnabled) {
		return;
	}

	sprintf(text, "txyz: %04.4ld %04.4ld %04.4ld", p_eyepoint->m_x, p_eyepoint->m_y, p_eyepoint->m_z);
	DRAW_DEBUG_TEXT();
	sprintf(
		text,
		"rxyz:  %04.4ld %04.4ld %04.4ld",
		FIXED_TO_INT(p_eyepoint->m_pitch) % 360,
		FIXED_TO_INT(p_eyepoint->m_heading) % 360,
		FIXED_TO_INT(p_eyepoint->m_roll) % 360
	);
	DRAW_DEBUG_TEXT();

	if (g_showEyePositionMain) {
		sprintf(
			text,
			"txyz: %5ld %5ld %5ld\nrxyz: %5d %5d %5d ",
			p_eyepoint->m_x,
			p_eyepoint->m_y,
			p_eyepoint->m_z,
			FIXED_TO_INT(p_eyepoint->m_pitch) % 360,
			FIXED_TO_INT(p_eyepoint->m_heading) % 360,
			FIXED_TO_INT(p_eyepoint->m_roll) % 360
		);
		DrawTextBox(OVERLAY_TEXT_COLOR, 1, text, g_eyePositionOrigin.m_x, g_eyePositionOrigin.m_y);
	}

	g_eyePositionShown = 1;
}

// FUNCTION: MW2 0x10058ee0
// FUNCTION: MW2GBL 0x10073c00
void HideEyePosition(void)
{
	MechChar text[28];
	MechChar format[28];

	sprintf(format, "%%%d.%ds", 0x18, 0x18);
	sprintf(text, format, g_blankText);
	DRAW_DEBUG_TEXT();
	DRAW_DEBUG_TEXT();
	g_eyePositionShown = 0;
}

// FUNCTION: MW2 0x10058f3c
// FUNCTION: MW2GBL 0x10073c5c
// FUNCTION: MW2MATROX 0x1008e1f7
// FUNCTION: MW23DFX 0x1003c9e0
// FUNCTION: MW2ATI 0x1003a9b0
// FUNCTION: MW2S3 0x100211d0
// FUNCTION: MW2POWERVR 0x100357f0
void ShowCacheInfo(void)
{
	MechChar text[28];

	sprintf(text, "Items in cache: %li     ", g_cacheEntryCount);
	DRAW_DEBUG_TEXT();
	g_cacheInfoShown = 1;
}

// FUNCTION: MW2 0x10058f78
// FUNCTION: MW2GBL 0x10073c98
// FUNCTION: MW2MATROX 0x1008e233
// FUNCTION: MW23DFX 0x1003ca10
// FUNCTION: MW2ATI 0x1003a9e0
// FUNCTION: MW2S3 0x10021200
// FUNCTION: MW2POWERVR 0x10035820
void HideCacheInfo(void)
{
	MechChar text[28];

#ifdef MW2_HIDE_CACHE_INFO_NO_ARGS
	sprintf(text, "                           ");
#else
	sprintf(text, "                           ", 0x19, 0x19);
#endif
	DRAW_DEBUG_TEXT();
	g_cacheInfoShown = 0;
}

// FUNCTION: MW2 0x10058fb2
// FUNCTION: MW2GBL 0x10073cd2
// FUNCTION: MW2MATROX 0x1008e26d
// FUNCTION: MW23DFX 0x1003ca40
// FUNCTION: MW2ATI 0x1003aa10
// FUNCTION: MW2S3 0x10021230
// FUNCTION: MW2POWERVR 0x10035850
void ShowMemInfo(void)
{
	MechChar text[40];
	MechS32 value;

	value = 0;
	if (!g_memInfoShown) {
		DRAW_DEBUG_TEXT();
		g_memInfoShown = 1;
	}

	sprintf(text, "%8.8ld", value);
	DRAW_DEBUG_TEXT();
	if (g_showMemInfoMain) {
		DrawTextBox(OVERLAY_TEXT_COLOR, 1, text, g_memInfoOrigin.m_x, g_memInfoOrigin.m_y);
	}
}

// FUNCTION: MW2 0x10059036
// FUNCTION: MW2GBL 0x10073d56
// FUNCTION: MW2MATROX 0x1008e2f1
// FUNCTION: MW23DFX 0x1003caa0
// FUNCTION: MW2ATI 0x1003aa70
// FUNCTION: MW2S3 0x10021290
// FUNCTION: MW2POWERVR 0x100358b0
void HideMemInfo(void)
{
	MechChar text[16];
	MechChar format[16];

	sprintf(format, "%%%d.%ds", 0xc, 0xc);
	sprintf(text, format, g_blankText);
	DRAW_DEBUG_TEXT();
	g_memInfoShown = 0;
}

// Steps the activity spinner.
// Matches but for the registers, which VC++ 2.2 allocates differently.
// FUNCTION: MW2 0x10059085
// FUNCTION: MW2GBL 0x10073da5
// FUNCTION: MW2MATROX 0x1008e340
// FUNCTION: MW23DFX 0x1003cae0
// FUNCTION: MW2ATI 0x1003aab0
// FUNCTION: MW2S3 0x100212d0
// FUNCTION: MW2POWERVR 0x100358f0
void StepSpinner(void)
{
	MechChar text[2];

	if (--g_spinnerDelay == 0) {
		g_spinnerDelay = 4;
		text[0] = g_spinnerArrows[g_spinnerArrow];
		text[1] = '\0';
		DRAW_DEBUG_TEXT();

		if (g_spinnerArrow-- == 0) {
			g_spinnerArrow = 3;
		}
	}
}

// Formats a line for the debug console: a leading space, padded with spaces to 79 characters.
// FUNCTION: MW2 0x100590ea
// FUNCTION: MW2GBL 0x10073e0a
// FUNCTION: MW2MATROX 0x1008e3a5
// FUNCTION: MW23DFX 0x1003cb10
// FUNCTION: MW2ATI 0x1003aae0
// FUNCTION: MW2S3 0x10021300
// FUNCTION: MW2POWERVR 0x10035920
void MonoPrintLine(MechChar* p_text)
{
	MechS32 i;
	MechChar text[80];
	MechChar* dst;

	for (i = 0; i < 80; i++) {
		text[i] = '\0';
	}

	text[0] = ' ';
	dst = text + 1;
	strncpy(dst, p_text, 78);
	for (i = 0; i < 79; i++) {
		if (text[i] == '\0') {
			text[i] = ' ';
		}
	}

	DRAW_DEBUG_TEXT();
}

// Clears the debug console's rows 3 to 23.
// FUNCTION: MW2 0x1005917d
// FUNCTION: MW2GBL 0x10073e9d
// FUNCTION: MW2MATROX 0x1008e438
// FUNCTION: MW23DFX 0x1003cb60
// FUNCTION: MW2ATI 0x1003ab30
// FUNCTION: MW2S3 0x10021350
// FUNCTION: MW2POWERVR 0x10035970
void MonoClear(void)
{
	MechS32 i;

	for (i = 3; i <= 0x17; i++) {
		DRAW_DEBUG_TEXT();
	}

	g_unk0x100a94ac[0] = '\0';
	g_unk0x100a94a8 = 3;
	g_monoLastRow = 3;
}

// Writes text to the debug console, 80 columns and rows 3 to 23, clearing it when it fills.
// FUNCTION: MW2 0x100591d1
// FUNCTION: MW2GBL 0x10073ef1
// FUNCTION: MW2MATROX 0x1008e48c
// FUNCTION: MW23DFX 0x1003cb80
// FUNCTION: MW2ATI 0x1003ab50
// FUNCTION: MW2S3 0x10021370
// FUNCTION: MW2POWERVR 0x10035990
void MonoPrint(MechChar* p_text)
{
	MechS32 newline;
	MechChar ch[2];
	MechS32 i;
	MechS32 length;

	newline = FALSE;
	length = strlen(p_text);
	ch[1] = '\0';
	for (i = 0; i < length; i++) {
		if (p_text[i] == '\n') {
			newline = TRUE;
		}
		else {
			ch[0] = p_text[i];
			DRAW_DEBUG_TEXT();
		}

		g_monoColumn++;
		if (g_monoColumn > 0x4f || newline) {
			g_monoColumn = 0;
			g_monoRow++;
			if (g_monoRow > 0x17) {
				MonoClear();
				g_monoRow = 3;
			}

			newline = FALSE;
		}
	}

	g_monoLastRow = g_monoRow;
}

// Scales the _MAIN readouts' positions to the screen, after a change of resolution.
// FUNCTION: MW2 0x100592b0
// FUNCTION: MW2GBL 0x10073fcb
// FUNCTION: MW2MATROX 0x1008e56b
// FUNCTION: MW23DFX 0x1003cc00
// FUNCTION: MW2ATI 0x1003abd0
// FUNCTION: MW2S3 0x100213f0
// FUNCTION: MW2POWERVR 0x10035a10
void ScaleOverlayPositions(void)
{
	ScalePointToScreen(&g_mainPixelBuffer, &g_frameRateOrigin, &g_frameRateOrigin);
	ScalePointToScreen(&g_mainPixelBuffer, &g_eyePositionOrigin, &g_eyePositionOrigin);
	ScalePointToScreen(&g_mainPixelBuffer, &g_memInfoOrigin, &g_memInfoOrigin);
}

// FUNCTION: MW2 0x10059300
// FUNCTION: MW2GBL 0x1007401b
// FUNCTION: MW2MATROX 0x1008e5bb
// FUNCTION: MW23DFX 0x1003cc40
// FUNCTION: MW2ATI 0x1003ac10
// FUNCTION: MW2S3 0x10021430
// FUNCTION: MW2POWERVR 0x10035a50
void SimEntranceDbug(MechChar* p_mission, MechS32 p_memory)
{
	MechChar text[80];

	sprintf(
		text,
		"%s  Res:%dx%d  Mem:%ld",
		p_mission,
		g_gameWindowGeometry->m_width,
		g_gameWindowGeometry->m_height,
		p_memory
	);
	ShowInGameMessage(text, 0, 0x4f3, 0x50);

	if (!g_monoEnabled) {
		return;
	}

	sprintf(text, "WarThink / MechWarrior II %s %ld", p_mission, p_memory);
	MonoPrintLine(text);
}
