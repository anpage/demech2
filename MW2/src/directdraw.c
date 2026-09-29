#include "directdraw.h"

#include "decomp.h"
#include "displaybackend.h"
#include "palettecolor.h"
#include "pixelbuffer.h"
#include "refreshmode.h"
#include "types.h"

#include <windows.h>

// The DirectDraw back end of the refresh modes (refreshmode.c), the simulator's copy of the
// shell's.

void DdrawStop(void);
void DdrawDestroySurfaces(void);
MechS32 DdrawInit(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height);
MechS32 DdrawFlip(void);
MechS32 DdrawBlit(void);
MechS32 DdrawBlitFlip(void);
MechS32 DdrawStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 DdrawBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 DdrawWritePaletteEntries(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors);
MechS32 DdrawWritePaletteGamma(PaletteColor* p_palette);
MechS32 DdrawPaletteFade(PaletteColor* p_palette, MechS32 p_steps);
MechS32 DdrawLockBuffer(void);

// GLOBAL: MW2 0x100b1cd8
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

// STUB: MW2 0x10077850
void DdrawStop(void)
{
	STUB(0x10077850);
}

// STUB: MW2 0x100778e0
void DdrawDestroySurfaces(void)
{
	STUB(0x100778e0);
}

// STUB: MW2 0x10077d75
MechS32 DdrawInit(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height)
{
	STUB(0x10077d75);
	return 0;
}

// STUB: MW2 0x100781c0
MechS32 DdrawFlip(void)
{
	STUB(0x100781c0);
	return 0;
}

// STUB: MW2 0x10078285
MechS32 DdrawBlit(void)
{
	STUB(0x10078285);
	return 0;
}

// STUB: MW2 0x10078354
MechS32 DdrawBlitFlip(void)
{
	STUB(0x10078354);
	return 0;
}

// STUB: MW2 0x10078461
MechS32 DdrawStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	STUB(0x10078461);
	return 0;
}

// STUB: MW2 0x10078687
MechS32 DdrawBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	STUB(0x10078687);
	return 0;
}

// STUB: MW2 0x10078826
MechS32 DdrawFill(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechU32 p_color)
{
	STUB(0x10078826);
	return 0;
}

// STUB: MW2 0x1007890f
MechS32 DdrawWritePaletteEntries(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors)
{
	STUB(0x1007890f);
	return 0;
}

// STUB: MW2 0x10078b38
MechS32 DdrawWritePaletteGamma(PaletteColor* p_palette)
{
	STUB(0x10078b38);
	return 0;
}

// STUB: MW2 0x10078d38
MechS32 DdrawPaletteFade(PaletteColor* p_palette, MechS32 p_steps)
{
	STUB(0x10078d38);
	return 0;
}

// STUB: MW2 0x10079170
MechS32 DdrawLockBuffer(void)
{
	STUB(0x10079170);
	return 0;
}
