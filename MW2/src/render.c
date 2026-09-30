#include "render.h"

#include "decomp.h"
#include "eyepoint.h"
#include "fixedmul.h"
#include "palette.h"
#include "refreshmode.h"
#include "rendertarget.h"
#include "setres.h"
#include "simmain.h"
#include "types.h"
#include "unk10013340.h"
#include "unk1003a530.h"
#include "unk1004b980.h"
#include "unk10065f10.h"
#include "unk1006d680.h"
#include "unk1007d120.h"

#include <windows.h>

// GLOBAL: MW2 0x100a2468
MechS32 g_unk0x100a2468 = 0;

// Cleared while an effect has the camera, set again when it gives it back.
// GLOBAL: MW2 0x100a2470
MechS32 g_unk0x100a2470 = 1;

// GLOBAL: MW2 0x100a2480
MechS32 g_unk0x100a2480 = 0;

// GLOBAL: MW2 0x10176eb4
GameWindowGeometry* g_gameWindowGeometry;

// GLOBAL: MW2 0x10176eb8
MechS32 g_screenHeight;

// Set when the next Blit should stretch the current render target over the window.
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

// GLOBAL: MW2 0x10176ee4
MechS32 g_screenWidthMinus1;

// GLOBAL: MW2 0x10176ee8
MechS32 g_screenHalfWidth;

// GLOBAL: MW2 0x10176eec
MechS32 g_screenHalfHeight;

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
		g_currentRenderTarget.m_buffer = &g_mainPixelBuffer;
		g_currentRenderTarget.m_left = 0;
		g_currentRenderTarget.m_top = 0;
		g_currentRenderTarget.m_right = g_gameWindowGeometry->m_width - 1;
		g_currentRenderTarget.m_bottom = g_gameWindowGeometry->m_height - 1;
		g_unk0x100bdff8 = g_currentRenderTarget;
		result = 1;
		InitRenderTargets(&g_currentRenderTarget);
		FUN_10065f10();
		g_unk0x100a2464 = 1;
	}

	return result;
}

// STUB: MW2 0x100128b5
void FirstRender(void)
{
	STUB(0x100128b5);
}

// STUB: MW2 0x100129b7
void SecondRender(void)
{
	STUB(0x100129b7);
}

// The normal frame draw callback: renders the 3D view.
// STUB: MW2 0x10012afe
void FUN_10012afe(void)
{
	STUB(0x10012afe);
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
	SelectRenderTarget(0);
}

// Presents the frame, or stretches the current render target over the window when a stretch is
// pending, restoring the render target afterwards.
// FUNCTION: MW2 0x10012e15
void Blit(void)
{
	if (g_unk0x10176ebc) {
		g_currentRefreshMode->m_stretchBlit(
			g_currentRenderTarget.m_left + 1,
			g_currentRenderTarget.m_top + 1,
			g_currentRenderTarget.m_right,
			g_currentRenderTarget.m_bottom
		);
		g_currentRenderTarget = g_unk0x100bdff8;
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
	if (g_unk0x100a245c && g_currentRenderTarget.m_buffer) {
		FillView(&g_currentRenderTarget, 0);
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
				DrawEllipse(&g_currentRenderTarget, x, y, radius, radiusY, color);
			}
		}
	}
}
