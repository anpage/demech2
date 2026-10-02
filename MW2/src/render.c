#include "render.h"

#include "animation.h"
#include "blit.h"
#include "cockpit.h"
#include "codeblock.h"
#include "decomp.h"
#include "displaybackend.h"
#include "error.h"
#include "eyepoint.h"
#include "fixedmul.h"
#include "geocache.h"
#include "mss.h"
#include "object.h"
#include "palette.h"
#include "palettecolor.h"
#include "refreshmode.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "setres.h"
#include "simmain.h"
#include "types.h"
#include "unk10013340.h"
#include "unk100335d0.h"
#include "unk10034a40.h"
#include "unk10036230.h"
#include "unk1003a530.h"
#include "unk10040b30.h"
#include "unk10042e00.h"
#include "unk10046750.h"
#include "unk1004b980.h"
#include "unk10065f10.h"
#include "unk1006d680.h"
#include "unk1007d120.h"

#include <string.h>
#include <windows.h>

// GLOBAL: MW2 0x100a244c
MechS32 g_drawModeIndex = -1;

// GLOBAL: MW2 0x100a2450
MechS32 g_initDrawModeParam2 = 1;

// GLOBAL: MW2 0x100a2454
MechS32 g_unk0x100a2454 = 0;

// The banner's file name, instead of sbannr (FUN_10012f3c).
// GLOBAL: MW2 0x100a2458
MechChar* g_unk0x100a2458 = NULL;

// GLOBAL: MW2 0x100a245c
void* g_unk0x100a245c = NULL;

// GLOBAL: MW2 0x100a2460
MechS32 g_unk0x100a2460 = 1;

// GLOBAL: MW2 0x100a2464
MechS32 g_unk0x100a2464 = 0;

// GLOBAL: MW2 0x100a2468
MechS32 g_unk0x100a2468 = 0;

// GLOBAL: MW2 0x100a246c
MechS32 g_unk0x100a246c = 0;

// Cleared while an effect has the camera, set again when it gives it back.
// GLOBAL: MW2 0x100a2470
MechS32 g_unk0x100a2470 = 1;

// GLOBAL: MW2 0x100a2474
MechS32 g_unk0x100a2474 = -1;

// The object of the scene's shape of kind 0x90, made by SecondRender.
// GLOBAL: MW2 0x100a2478
AmberWillow0x7c* g_unk0x100a2478 = NULL;

// The object of the scene's shape of kind 0xa0.
// GLOBAL: MW2 0x100a247c
AmberWillow0x7c* g_unk0x100a247c = NULL;

// GLOBAL: MW2 0x100a2480
MechS32 g_unk0x100a2480 = 0;

// GLOBAL: MW2 0x100bdff8
Pane g_unk0x100bdff8;

// GLOBAL: MW2 0x10176eb4
GameWindowGeometry* g_gameWindowGeometry;

// GLOBAL: MW2 0x10176eb8
MechS32 g_screenHeight;

// Set when the next Blit should stretch the current pane over the window.
// GLOBAL: MW2 0x10176eb0
undefined4 g_unk0x10176eb0;

// GLOBAL: MW2 0x10176ebc
MechS32 g_unk0x10176ebc;

// GLOBAL: MW2 0x10176ec0
MechS32 g_screenHeightMinus1;

// GLOBAL: MW2 0x10176ec4
MechS32 g_screenPixelCount;

// GLOBAL: MW2 0x10176ec8
MechS32 g_screenWidth;

// GLOBAL: MW2 0x10176ed0
Pane g_currentPane;

// GLOBAL: MW2 0x10176ee4
MechS32 g_screenWidthMinus1;

// GLOBAL: MW2 0x10176ee8
MechS32 g_screenHalfWidth;

// GLOBAL: MW2 0x10176eec
MechS32 g_screenHalfHeight;

// GLOBAL: MW2 0x10176ef0
PixelBuffer g_mainPixelBuffer;

// FUNCTION: MW2 0x10012720
MechS32 InitGameWindowGeometry(void)
{
	g_gameWindowGeometry = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(GameWindowGeometry));
	if (g_gameWindowGeometry == NULL) {
		return 0;
	}

	g_gameWindowGeometry->m_width = g_gameWindowWidth;
	g_gameWindowGeometry->m_height = g_gameWindowHeight;
	g_gameWindowGeometry->m_unk0x08 = 1;
	g_gameWindowGeometry->m_numColors = 0x100;
	g_gameWindowGeometry->m_unk0x10 = 1;
	g_gameWindowGeometry->m_unk0x14 = 0;
	g_screenPixelCount = g_gameWindowHeight * g_gameWindowWidth;
	g_screenWidth = g_gameWindowWidth;
	g_screenHeight = g_gameWindowHeight;
	g_screenWidthMinus1 = g_gameWindowWidth - 1;
	g_screenHeightMinus1 = g_gameWindowHeight - 1;
	g_screenHalfWidth = g_screenWidth / 2;
	g_screenHalfHeight = g_screenHeight / 2;
	return 1;
}

// FUNCTION: MW2 0x10012802
MechS32 InitDisplayGeometry(void)
{
	MechS32 result;

	result = 0;
	if (InitGameWindowGeometry()) {
		FUN_1005d44e(g_gameWindowGeometry);
		FUN_1005d410(g_gameWindowGeometry);
		g_currentPane.m_buffer = &g_mainPixelBuffer;
		g_currentPane.m_left = 0;
		g_currentPane.m_top = 0;
		g_currentPane.m_right = g_gameWindowGeometry->m_width - 1;
		g_currentPane.m_bottom = g_gameWindowGeometry->m_height - 1;
		g_unk0x100bdff8 = g_currentPane;
		result = 1;
		InitPanes(&g_currentPane);
		FUN_10065f10();
		g_unk0x100a2464 = 1;
	}

	return result;
}

// Makes the code block writable (the drawing routines patch themselves), sets up the vertex
// buffers and the scene, and installs the normal render hooks.
// Stack-slot permutation: size, start, segment and oldProtect.
// FUNCTION: MW2 0x100128b5
void FirstRender(void)
{
	MechS32 segment;
	MechS32 size;
	DWORD oldProtect;
	undefined4 start;

	size = 0;
	start = 0;
	segment = 0;
	oldProtect = 0;
	size = GetCodeBlock(&start, &segment);
	if (!VirtualProtect(
			(void*) start,
			size,
			(GetVersion() & 0x80000000) ? PAGE_READWRITE : PAGE_EXECUTE_READWRITE,
			&oldProtect
		)) {
		Error(0x4d, NULL);
	}

	FUN_1007d150(0x80, 0x5dc);
	g_unk0x100a54b8 = 0x578;
	FUN_1006d680();
	g_unk0x100a6cc8.m_frameDrawCallback = FUN_10012afe;
	g_unk0x100a6cc8.m_unk0x58 = FUN_1004c2ef;
	g_unk0x100a6cc8.m_unk0x5c = FUN_10048ebe;
	g_unk0x100a6cc8.m_unk0x60 = (MechS32 (*)()) FUN_10036230;
	g_unk0x100a6cc8.m_drawPolygon = FUN_10042e00;
	g_unk0x100a5558 = 0xff;
	if (g_unk0x100a6cc8.m_unk0x1c || g_unk0x100a6cc8.m_unk0x20) {
		g_unk0x100a6cc8.m_unk0x30 = 0;
	}

	g_unk0x100a6cc8.m_unk0x10 |= 8;
}

// Makes the objects of the scene's shapes of kinds 0x90 and 0xa0, and sets up its shapes of kind
// 0x70 and type 4.
// FUNCTION: MW2 0x100129b7
void SecondRender(void)
{
	ScarletOrchid0x4c* root;
	ScarletOrchid0x4c* shape;
	ScarletOrchid0x4c* next;

	root = g_unk0x100ad5e8;
	if (!root) {
		return;
	}

	for (shape = root->m_unk0x08; shape; shape = shape->m_unk0x08) {
		if ((shape->m_unk0x02 & 0xf0) == 0x90) {
			g_unk0x100a2478 = FUN_1003b6e5(shape);
			FUN_10001a52(g_unk0x100a2478);
			break;
		}
	}

	for (shape = root->m_unk0x08; shape; shape = shape->m_unk0x08) {
		if ((shape->m_unk0x02 & 0xf0) == 0xa0) {
			g_unk0x100a247c = FUN_1003b6e5(shape);
			break;
		}
	}

	for (shape = root->m_unk0x08; shape; shape = next) {
		next = shape->m_unk0x08;
		if ((shape->m_unk0x02 & 0xf0) == 0x70) {
			FUN_1006da2d(shape);
			FUN_1006d989(shape);
		}

		if (shape->m_unk0x24 == 4) {
			FUN_1006d989(shape);
		}
	}
}

// The normal frame draw callback: renders the 3D view.
// Draws the 3D view, the normal frame draw callback: clears the frame first when drawing to
// another pane, updates the eyepoint, draws the scene (without the extra pass on the
// DirectDraw backend), then the objects of the shapes of kinds 0x90 and 0xa0 with their own clip
// distances, the scene's objects, and the animations.
// FUNCTION: MW2 0x10012afe
void FUN_10012afe(void)
{
	MechS32 saved;
	MechS32 pass;

	if (g_unk0x100a2468) {
		memset(g_mainPixelBuffer.m_pixels, g_unk0x100a5544, g_refreshModePixelCount);
		SelectPane(g_unk0x100a2468);
	}

	if (g_unk0x100a2460) {
		FUN_1004bc2e(g_eyepoint);
		g_unk0x100a2460 = 0;
	}

	if (g_unk0x100a6cc8.m_unk0x00) {
		FillView(&g_currentPane, g_unk0x100a5544);
		return;
	}

	FUN_1004bfe8(g_eyepoint);
	FUN_1004b980(g_eyepoint);
	if (g_unk0x100a6cc8.m_unk0x30 || g_unk0x100a6cc8.m_unk0x34) {
		FillView(&g_currentPane, g_unk0x100a5544);
	}
	else if (g_unk0x100a6cc8.m_unk0x1c || g_unk0x100a6cc8.m_unk0x20) {
		if (g_currentDisplayBackend->m_id == c_displayBackendDirectDraw) {
			pass = g_unk0x100a6cc8.m_unk0x20;
			g_unk0x100a6cc8.m_unk0x20 = 0;
			FUN_1004320b(g_eyepoint);
			g_unk0x100a6cc8.m_unk0x20 = pass;
		}
		else {
			FUN_1004320b(g_eyepoint);
		}
	}

	if (g_unk0x100a246c && g_unk0x100a2470 && g_unk0x100a2474 != -1) {
		FUN_10020c6f(g_unk0x100a2474, &g_eyepoint->m_unk0x1c, &g_eyepoint->m_unk0x20, &g_eyepoint->m_unk0x24);
	}

	g_unk0x100a2480 = 0;
	if (g_unk0x100a2478) {
		saved = g_eyepoint->m_unk0x40;
		FUN_1004bf8a(g_eyepoint, 0x7fffffff);
		g_unk0x100a6cc8.m_unk0x58 = FUN_1004c565;
		FUN_10033b9e(g_unk0x100a2478);
		g_unk0x100a2480 += g_unk0x100a54b0;
		FUN_1004bf8a(g_eyepoint, saved);
		g_unk0x100a6cc8.m_unk0x58 = FUN_1004c2ef;
	}

	FUN_100338bb(g_unk0x100ad5e8);
	g_unk0x100a2480 += g_unk0x100a54b0;
	if (g_unk0x100a2420 && g_unk0x100a247c) {
		saved = g_eyepoint->m_unk0x3c;
		FUN_1004bf61(g_eyepoint, 8);
		g_unk0x100a6cc8.m_unk0x58 = FUN_1004c779;
		FUN_10033b9e(g_unk0x100a247c);
		g_unk0x100a2480 += g_unk0x100a54b0;
		FUN_1004bf61(g_eyepoint, saved);
		g_unk0x100a6cc8.m_unk0x58 = FUN_1004c2ef;
	}

	if (g_unk0x100a2454) {
		FUN_100131f1(g_unk0x100ad5e8);
	}

	FUN_10069591();
	SelectPane(0);
}

// FUNCTION: MW2 0x10012dca
void FUN_10012dca(MechS32 p_value)
{
	if (p_value >= 0 && p_value < 11) {
		g_unk0x100a2468 = p_value;
	}
	else {
		g_unk0x100a2468 = 0;
	}
}

// FUNCTION: MW2 0x10012e00
void FUN_10012e00(void)
{
	SelectPane(0);
}

// Presents the frame, or stretches the current pane over the window when a stretch is
// pending, restoring the pane afterwards.
// FUNCTION: MW2 0x10012e15
void Blit(void)
{
	if (g_unk0x10176ebc) {
		g_currentRefreshMode->m_stretchBlit(
			g_currentPane.m_left + 1,
			g_currentPane.m_top + 1,
			g_currentPane.m_right,
			g_currentPane.m_bottom
		);
		g_currentPane = g_unk0x100bdff8;
		g_unk0x100a5f18 = g_unk0x100a5a24;
		g_unk0x10176ebc = 0;
	}
	else if (g_windowActive) {
		g_currentRefreshMode->m_flip();
	}
}

// FUNCTION: MW2 0x10012e91
void ShutdownRender(void)
{
	FUN_1006db28();
	FUN_1007d120();
	if (g_unk0x100a245c && g_currentPane.m_buffer) {
		FillView(&g_currentPane, 0);
		if (g_windowActive) {
			g_currentRefreshMode->m_flip();
		}
	}

	if (g_unk0x100a245c) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a245c);
	}

	g_unk0x100a2464 = 0;
	ShutdownRefreshMode();
}

// FUNCTION: MW2 0x10012f14
undefined4 FUN_10012f14(void)
{
	return g_unk0x10176eb0;
}

// FUNCTION: MW2 0x10012f29
void FUN_10012f29(undefined4 p_unk0x00, undefined4 p_value)
{
	g_unk0x10176eb0 = p_value;
}

// Shows the banner GIF (sbannr, or g_unk0x100a2458's name, with the art resolution's suffix) and
// fades its palette in. Nothing calls it.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10012f3c
void FUN_10012f3c(void)
{
	void* gif;
	Pane target;
	MechU8* state;
	MechChar path[256];
	PaletteColor* palette;

	if (g_unk0x100a2458 == NULL || *g_unk0x100a2458 == '\0') {
		strcpy(path, "sbannr");
		strcat(path, g_unk0x100aa710[g_unk0x100e9614]);
		strcat(path, ".");
		strcat(path, "gif");
	}
	else {
		strcpy(path, g_unk0x100a2458);
		strcat(path, g_unk0x100aa710[g_unk0x100e9614]);
		strcat(path, ".");
		strcat(path, "gif");
	}

	gif = FILE_read(path, NULL);
	if (gif) {
		state = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, 0x502e);
		if (state) {
			palette = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 0x100 * sizeof(PaletteColor));
			if (palette) {
				g_currentDisplayBackend->m_setPalette(0, 0x100, palette, 1);
				target = g_currentPane;
				FUN_1005705e(&target, &target, gif);
				if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
					BlitGif(&target, gif, state);
					if (g_windowActive) {
						g_currentRefreshMode->m_flip();
					}
				}

				ReadGifPalette(gif, (MechU8*) palette);
				g_currentDisplayBackend->m_blendPalettes(palette, 30);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, palette);
			}

			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, state);
		}

		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, gif);
	}
}

// Draws the shapes of the list p_root that are drawn as dots (flag 0x100 without 0x800, or kind
// 0x50) as circles of their radius (m_unk0x40), in color 0xf.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100131f1
void FUN_100131f1(ScarletOrchid0x4c* p_root)
{
	MechS32 color;
	ScarletOrchid0x4c* shape;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 radius;
	MechS32 radiusY;

	if (!p_root || !p_root->m_unk0x08 || p_root->m_unk0x04 == p_root->m_unk0x08) {
		return;
	}

	for (shape = p_root->m_unk0x08; shape; shape = shape->m_unk0x08) {
		if ((((shape->m_unk0x02 & 0x100) && !(shape->m_unk0x00 & 0x800)) || (shape->m_unk0x02 & 0xf0) == 0x50) &&
			!g_unk0x100a6cc8.m_unk0x58(shape)) {
			x = shape->m_unk0x34;
			y = shape->m_unk0x38;
			z = shape->m_unk0x3c;
			radius = shape->m_unk0x40;
			color = 0xf;
			if (FUN_1004c11d(&x, &y, &z)) {
				radius = FUN_10013340(g_eyepoint->m_unk0x94, radius, z);
				radiusY = FixedMul16(radius, g_eyepoint->m_pixelAspect);
				DrawEllipse(&g_currentPane, x, y, radius, radiusY, color);
			}
		}
	}
}
