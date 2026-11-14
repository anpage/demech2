/* The 3Dfx edition's display back end and refresh modes, in place of 1.1's DirectDraw ones
   (directdraw.c), whose functions it keeps one for one: one object of its own, after gdi.c's.
   It opens the display through the renderer (3dfx/a3d.c) and draws the 2D panels straight into
   the frame buffer, which Glide maps (FUN_10066930); flipping swaps Glide's buffers, and the rest
   does nothing. */
#include "3dfx/glidedraw.h"

#include "accel/a3d.h"
#include "decomp.h"
#include "displaybackend.h"
#include "palettecolor.h"
#include "refreshmode.h"
#include "setres.h"
#include "simmain.h"
#include "types.h"
#include "window.h"

#include <glide.h>
#include <windows.h>

void GlideStop(void);
void GlideDestroySurfaces(void);
MechS32 GlideCreateSurfaces(MechS32 p_width, MechS32 p_height);
MechS32 GlideInit(WINDOW* p_buffer, MechS32 p_width, MechS32 p_height);
MechS32 GlideFlip(void);
MechS32 GlideBlit(void);
MechS32 GlideBlitFlip(void);
MechS32 GlideStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 GlideBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 GlideWritePaletteEntries(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors);
MechS32 GlideWritePaletteGamma(PaletteColor* p_palette);
MechS32 GlidePaletteFade(PaletteColor* p_palette, MechS32 p_steps);
MechS32 GlideLockBuffer(void);

// GLOBAL: MW23DFX 0x1024a778
DisplayBackend g_glideBackend = {
	c_displayBackendDirectDraw,
	c_windowModeFullscreen,
	WS_POPUP,
	GlideInit,
	(MechS32 (*)()) GlideStop,
	GlideWritePaletteEntries,
	GlideWritePaletteGamma,
	GlidePaletteFade,
	GlideLockBuffer,
	0
};

// GLOBAL: MW23DFX 0x1024a7a0
RefreshMode g_glideFlipRefreshMode = {
	0,
	c_displayBackendDirectDraw,
	1,
	0,
	GlideInit,
	(MechS32 (*)()) GlideDestroySurfaces,
	GlideFlip,
	GlideBlitRect,
	GlideStretchBlit
};

// GLOBAL: MW23DFX 0x1024a7c8
RefreshMode g_glideBlitFlipRefreshMode = {
	1,
	c_displayBackendDirectDraw,
	1,
	0,
	GlideInit,
	(MechS32 (*)()) GlideDestroySurfaces,
	GlideBlitFlip,
	GlideBlitRect,
	GlideStretchBlit
};

// GLOBAL: MW23DFX 0x1024a7f0
RefreshMode g_glideVideoMemoryRefreshMode = {
	2,
	c_displayBackendDirectDraw,
	1,
	0,
	GlideInit,
	(MechS32 (*)()) GlideDestroySurfaces,
	GlideBlit,
	GlideBlitRect,
	GlideStretchBlit
};

// GLOBAL: MW23DFX 0x1024a818
RefreshMode g_glideSystemMemoryRefreshMode = {
	3,
	c_displayBackendDirectDraw,
	1,
	0,
	GlideInit,
	(MechS32 (*)()) GlideDestroySurfaces,
	GlideBlit,
	GlideBlitRect,
	GlideStretchBlit
};

// FUNCTION: MW23DFX 0x100059a0
void GlideStop(void)
{
	A3D_shutdown();
}

// FUNCTION: MW23DFX 0x100059b0
void GlideDestroySurfaces(void)
{
	g_refreshModeBuffer->m_buffer = NULL;
}

// FUNCTION: MW23DFX 0x100059c0
MechS32 GlideCreateSurfaces(MechS32 p_width, MechS32 p_height)
{
	return 0;
}

// FUNCTION: MW23DFX 0x100059d0
MechS32 GlideInit(WINDOW* p_buffer, MechS32 p_width, MechS32 p_height)
{
	if (g_currentDisplayBackend->m_id != c_displayBackendDirectDraw) {
		g_currentDisplayBackend->m_end();
		g_currentDisplayBackend = g_displayBackends[c_displayBackendDirectDraw];
		if (g_currentDisplayBackend->m_windowMode != g_windowMode) {
			AdjustWindowSize(g_currentDisplayBackend);
		}
	}

	if (A3D_Init()) {
		return 1;
	}

	if (GlideCreateSurfaces(p_width, p_height)) {
		GlideDestroySurfaces();
	}

	grBufferClear(0, 0, 0xffff);
	if (g_currentRefreshMode->m_flip()) {
		GlideDestroySurfaces();
		return 1;
	}

	p_buffer->m_buffer = NULL;
	p_buffer->m_xMax = 0x7ff;
	p_buffer->m_yMax = p_height - 1;
	p_buffer->m_shadow = 0;
	p_buffer->m_bitmapInfo = &g_bitmapInfo;
	return 0;
}

// Never called.
// FUNCTION: MW23DFX 0x10005a80
MechS32 FUN_10005a80(MechS32 p_unk0x00, MechS32 p_unk0x04, MechS32 p_unk0x08)
{
	return g_a3dRes800x600 != 0;
}

// Swaps Glide's buffers once at most one swap is pending.
// FUNCTION: MW23DFX 0x10005a90
MechS32 GlideFlip(void)
{
	MechS32 wait;

	wait = 0xfffff;
	while (wait-- && grBufferNumPending() > 1) {
	}

	grBufferSwap(1);
	return 0;
}

// FUNCTION: MW23DFX 0x10005ac0
MechS32 GlideBlit(void)
{
	return 0;
}

// FUNCTION: MW23DFX 0x10005ad0
MechS32 GlideBlitFlip(void)
{
	return 0;
}

// FUNCTION: MW23DFX 0x10005ae0
MechS32 GlideStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	return 0;
}

// FUNCTION: MW23DFX 0x10005af0
MechS32 GlideBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	return 0;
}

// FUNCTION: MW23DFX 0x10005b00
MechS32 GlideFill(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechU32 p_color)
{
	return 0;
}

// FUNCTION: MW23DFX 0x10005b10
MechS32 GlideWritePaletteEntries(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors)
{
	return 0;
}

// FUNCTION: MW23DFX 0x10005b20
MechS32 GlideWritePaletteGamma(PaletteColor* p_palette)
{
	return 0;
}

// FUNCTION: MW23DFX 0x10005b30
MechS32 GlidePaletteFade(PaletteColor* p_palette, MechS32 p_steps)
{
	return 0;
}

// Opens the frame buffer for the 2D drawing, which draws straight into it.
// The 3Dfx edition's diff is register allocation and instruction scheduling only.
// FUNCTION: MW23DFX 0x10005b40
MechS32 GlideLockBuffer(void)
{
	if (!g_a3dLfbOpen) {
		FUN_10066930();
	}

	g_refreshModeBuffer->m_buffer = g_a3dLfb;
	g_refreshModeBuffer->m_xMax = 0x7ff;
	return 0;
}
