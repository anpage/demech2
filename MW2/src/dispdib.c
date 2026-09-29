#include "dispdib.h"

#include "decomp.h"
#include "displaybackend.h"
#include "palettecolor.h"
#include "pixelbuffer.h"
#include "refreshmode.h"
#include "types.h"

#include <windows.h>

// The DisplayDib display back end and its refresh mode, the simulator's copy of the shell's.

MechS32 DispDibBegin(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height);
MechS32 DispDibEnd(void);
MechS32 DispDibFlip(void);
MechS32 DispDibBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 DispDibStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 DispDibAcquireFramebuffer(void);
MechS32 DispDibSetPalette(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors);
MechS32 DispDibSetPaletteWithBrightness(PaletteColor* p_palette);
MechS32 DispDibBlendPalettes(PaletteColor* p_palette, MechS32 p_steps);

// GLOBAL: MW2 0x100a83a0
DisplayBackend g_dispDibBackend = {
	c_displayBackendDisplayDib,
	c_windowModeFullscreen,
	WS_POPUP,
	DispDibBegin,
	DispDibEnd,
	DispDibSetPalette,
	DispDibSetPaletteWithBrightness,
	DispDibBlendPalettes,
	DispDibAcquireFramebuffer,
	0
};

// GLOBAL: MW2 0x100a83c8
RefreshMode g_dispDibRefreshMode =
	{4, c_displayBackendDisplayDib, 1, 0, DispDibBegin, DispDibEnd, DispDibFlip, DispDibBlitRect, DispDibStretchBlit};

// STUB: MW2 0x1004e760
MechS32 DispDibBegin(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height)
{
	STUB(0x1004e760);
	return 0;
}

// STUB: MW2 0x1004ea78
MechS32 DispDibEnd(void)
{
	STUB(0x1004ea78);
	return 0;
}

// STUB: MW2 0x1004eb72
MechS32 DispDibFlip(void)
{
	STUB(0x1004eb72);
	return 0;
}

// STUB: MW2 0x1004ebdb
MechS32 DispDibBlitRect(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	STUB(0x1004ebdb);
	return 0;
}

// STUB: MW2 0x1004ec44
MechS32 DispDibStretchBlit(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	STUB(0x1004ec44);
	return 0;
}

// STUB: MW2 0x1004ed1c
MechS32 DispDibAcquireFramebuffer(void)
{
	STUB(0x1004ed1c);
	return 0;
}

// STUB: MW2 0x1004ed3b
MechS32 DispDibSetPalette(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors)
{
	STUB(0x1004ed3b);
	return 0;
}

// STUB: MW2 0x1004eea7
MechS32 DispDibSetPaletteWithBrightness(PaletteColor* p_palette)
{
	STUB(0x1004eea7);
	return 0;
}

// STUB: MW2 0x1004efd5
MechS32 DispDibBlendPalettes(PaletteColor* p_palette, MechS32 p_steps)
{
	STUB(0x1004efd5);
	return 0;
}
