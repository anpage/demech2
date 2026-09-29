#include "decomp.h"
#include "displaybackend.h"
#include "pixelbuffer.h"
#include "refreshmode.h"
#include "rendertarget.h"
#include "simmain.h"
#include "types.h"

// The dropship animation of the loading screen ("sup anim"), drawn by an AIL timer.

// GLOBAL: MW2 0x100a0154
void* g_supAnimBackdrop = NULL;

// GLOBAL: MW2 0x100a0158
void* g_supAnimShape = NULL;

// GLOBAL: MW2 0x100a015c
MechS32 g_supAnimFrameCount = 0;

// GLOBAL: MW2 0x100a0198
MechS32 g_supAnimFrame = 0;

// GLOBAL: MW2 0x100bcd50
MechS32 g_supAnimY;

// GLOBAL: MW2 0x100bcd54
MechS32 g_supAnimX;

// GLOBAL: MW2 0x100bcd58
RenderTarget g_supAnimTarget;

// GLOBAL: MW2 0x100bcd70
PixelBuffer g_supAnimBuffer;

// STUB: MW2 0x10003a70
void StartSupAnim(MechS32 p_unk0x00)
{
	STUB(0x10003a70);
}

// Draws the backdrop and the next frame of the dropship, and presents them. AIL calls it as a
// timer callback, which takes an argument this one doesn't pop.
// FUNCTION: MW2 0x10003f3d
void SupAnimTimerCallback(void)
{
	if (!g_unk0x100a2464 || !g_supAnimBackdrop || !g_supAnimShape || g_supAnimFrameCount < 2) {
		return;
	}

	if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
		g_supAnimBuffer.m_pixels = g_mainPixelBuffer.m_pixels;
		FillRenderTargetRect(&g_supAnimTarget, 0);
		DrawShapeFrame(&g_supAnimTarget, g_supAnimBackdrop, 0, 0, 0);
		DrawShapeFrame(&g_supAnimTarget, g_supAnimShape, g_supAnimFrame, g_supAnimX, g_supAnimY);
		if (g_windowActive) {
			g_currentRefreshMode->m_flip();
		}

		g_supAnimFrame = (g_supAnimFrame + 1) % g_supAnimFrameCount;
	}
}

// STUB: MW2 0x10004031
void StopSupAnim(void)
{
	STUB(0x10004031);
}
