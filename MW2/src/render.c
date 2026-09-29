#include "render.h"

#include "decomp.h"
#include "palette.h"
#include "refreshmode.h"
#include "rendertarget.h"
#include "simmain.h"
#include "types.h"

#include <windows.h>

// GLOBAL: MW2 0x100a2468
MechS32 g_unk0x100a2468 = 0;

// GLOBAL: MW2 0x100a2480
MechS32 g_unk0x100a2480 = 0;

// GLOBAL: MW2 0x10176eb4
GameWindowGeometry* g_gameWindowGeometry;

// GLOBAL: MW2 0x10176eb8
MechS32 g_screenHeight;

// Set when the next Blit should stretch the current render target over the window.
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
	g_gameWindowGeometry->m_unk0x0c = 0x100;
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

// STUB: MW2 0x10012802
MechS32 InitDisplayGeometry(void)
{
	STUB(0x10012802);
	return 0;
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

// STUB: MW2 0x10012e91
void ShutdownRender(void)
{
	STUB(0x10012e91);
}
