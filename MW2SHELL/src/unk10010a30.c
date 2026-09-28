#include "decomp.h"
#include "drawbitmapinfo.h"
#include "drawmode.h"
#include "drawmodeextension.h"
#include "palettecolor.h"
#include "pixelbuffer.h"
#include "types.h"

#include <windows.h>

// The draw mode manager and the DirectDraw back end. The DisplayDib back end lives in dispdib.cpp,
// the GDI one in gdi.c.

MechS32 FUN_100114f0();
MechS32 FUN_10011580();
MechS32 FUN_10011a15(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height);
MechS32 FUN_10011e5c();
MechS32 FUN_10011fdf();
MechS32 FUN_1001216c();
MechS32 FUN_10012337(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 FUN_100126d9(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
MechS32 FUN_10012add(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors);
MechS32 FUN_10012db7(PaletteColor* p_palette);
MechS32 FUN_10013068(PaletteColor* p_palette, MechS32 p_steps);
MechS32 FUN_10013551();

extern HWND g_pWnd;
extern MechS32 g_nWindowMode;
extern MechS32 g_unk0x1006a9d8;

undefined4 FUN_1003bf90(MechS32 p_unk0x00);

extern DrawModeExtension g_dispDibDrawModeExtension;
extern DrawModeExtension g_gdiDrawModeExtension;
extern DrawMode g_dispDibDrawMode;
extern DrawMode g_gdiDrawMode;

extern DrawModeExtension g_unk0x10063230;
extern DrawMode g_unk0x10063258;
extern DrawMode g_unk0x10063280;
extern DrawMode g_unk0x100632a8;
extern DrawMode g_unk0x100632d0;

// Indexed by DrawModeExtension::m_id.
// GLOBAL: MW2SHELL 0x10062ca0
DrawModeExtension* g_drawModeExtensions[3] = {&g_unk0x10063230, &g_dispDibDrawModeExtension, &g_gdiDrawModeExtension};

// GLOBAL: MW2SHELL 0x10062cb0
DrawMode* g_drawModes[6] = {
	&g_unk0x10063258,
	&g_unk0x10063280,
	&g_unk0x100632a8,
	&g_unk0x100632d0,
	&g_dispDibDrawMode,
	&g_gdiDrawMode,
};

// GLOBAL: MW2SHELL 0x10062cc8
DrawModeExtension* g_currentDrawModeExtension = NULL;

// GLOBAL: MW2SHELL 0x10062ccc
DrawMode* g_currentDrawMode = NULL;

// The buffer the active draw mode renders into.
// GLOBAL: MW2SHELL 0x10062cdc
PixelBuffer* g_unk0x10062cdc = NULL;

// GLOBAL: MW2SHELL 0x10062ce0
PaletteColor g_paletteColors[0x100] = {0};

// The DIB bits of the GDI and DisplayDib back ends, restored into g_unk0x10062cdc by
// m_acquireFramebuffer.
// GLOBAL: MW2SHELL 0x10062fe0
undefined* g_unk0x10062fe0 = NULL;

// GLOBAL: MW2SHELL 0x10063000
MechS32 g_unk0x10063000 = 1;

// GLOBAL: MW2SHELL 0x10063230
DrawModeExtension g_unk0x10063230 = {
	c_drawModeExtensionDirectDraw,
	c_windowModeFullscreen,
	WS_POPUP,
	FUN_10011a15,
	FUN_100114f0,
	FUN_10012add,
	FUN_10012db7,
	FUN_10013068,
	FUN_10013551,
	0
};

// GLOBAL: MW2SHELL 0x10063258
DrawMode g_unk0x10063258 = {0, 0, 1, 0, FUN_10011a15, FUN_10011580, FUN_10011e5c, FUN_100126d9, FUN_10012337};

// GLOBAL: MW2SHELL 0x10063280
DrawMode g_unk0x10063280 = {1, 0, 1, 0, FUN_10011a15, FUN_10011580, FUN_1001216c, FUN_100126d9, FUN_10012337};

// GLOBAL: MW2SHELL 0x100632a8
DrawMode g_unk0x100632a8 = {2, 0, 1, 0, FUN_10011a15, FUN_10011580, FUN_10011fdf, FUN_100126d9, FUN_10012337};

// GLOBAL: MW2SHELL 0x100632d0
DrawMode g_unk0x100632d0 = {3, 0, 1, 0, FUN_10011a15, FUN_10011580, FUN_10011fdf, FUN_100126d9, FUN_10012337};

// GLOBAL: MW2SHELL 0x100965d0
MechU32 g_unk0x100965d0;

// GLOBAL: MW2SHELL 0x100965e4
MechS32 g_unk0x100965e4;

// GLOBAL: MW2SHELL 0x100965e8
MechU32 g_unk0x100965e8;

// The shell window's position and size in windowed mode (SetWindowPos arguments, not corners).
// GLOBAL: MW2SHELL 0x10096a50
RECT g_unk0x10096a50;

// GLOBAL: MW2SHELL 0x10096a60
DrawBitmapInfo g_bitmapInfo;

// GLOBAL: MW2SHELL 0x10096e88
MechS32 g_unk0x10096e88;

// GLOBAL: MW2SHELL 0x10096e8c
MechS32 g_drawModeWidth;

// GLOBAL: MW2SHELL 0x10096e90
MechS32 g_drawModeHeight;

// STUB: MW2SHELL 0x10010a30
MechS32 InitDrawMode(
	MechS32 p_unk0x00,
	MechS32 p_unk0x04,
	PixelBuffer* p_buffer,
	MechS32 p_width,
	MechS32 p_height,
	MechS32 p_unk0x14
)
{
	STUB(0x10010a30);
	return 0;
}

// FUNCTION: MW2SHELL 0x10010d49
void FUN_10010d49()
{
	if (g_currentDrawMode != NULL) {
		g_currentDrawMode->m_end();
	}
	if (g_currentDrawModeExtension != NULL) {
		g_currentDrawModeExtension->m_end();
	}

	g_unk0x10063000 = 1;
}

// A top-down 8-bit DIB of p_width x p_height.
// FUNCTION: MW2SHELL 0x1001125f
void InitBitmapInfo(MechS32 p_width, MechS32 p_height)
{
	g_bitmapInfo.m_header.biSize = sizeof(BITMAPINFOHEADER);
	g_bitmapInfo.m_header.biWidth = p_width;
	g_bitmapInfo.m_header.biHeight = p_height * -1;
	g_bitmapInfo.m_header.biPlanes = 1;
	g_bitmapInfo.m_header.biBitCount = 8;
	g_bitmapInfo.m_header.biCompression = BI_RGB;
	g_bitmapInfo.m_header.biSizeImage = 0;
	g_bitmapInfo.m_header.biClrUsed = 0;
	g_bitmapInfo.m_header.biXPelsPerMeter = 0;
	g_bitmapInfo.m_header.biYPelsPerMeter = 0;
	g_bitmapInfo.m_header.biClrImportant = 0;
}

// Restyles the shell window for p_extension's window mode, and sets g_nWindowMode: a mode that
// covers the whole screen counts as fullscreen.
// FUNCTION: MW2SHELL 0x100112da
void AdjustWindowSize(DrawModeExtension* p_extension)
{
	if (p_extension == NULL) {
		return;
	}

	if (p_extension->m_windowMode == c_windowModeWindowed) {
		SetWindowLong(g_pWnd, GWL_STYLE, p_extension->m_style | WS_VISIBLE);
	}
	else {
		SetWindowLong(g_pWnd, GWL_STYLE, (p_extension->m_style | WS_VISIBLE) & ~WS_SYSMENU);
	}

	if (p_extension->m_windowMode == c_windowModeWindowed) {
		if (g_currentDrawModeExtension != NULL && g_currentDrawModeExtension->m_id == c_drawModeExtensionDirectDraw) {
			g_unk0x100965d0 = timeGetTime();
			g_unk0x100965e8 = g_unk0x100965d0 + 3000;
			g_unk0x100965e4 = 1;
		}

		SetWindowPos(
			g_pWnd,
			HWND_NOTOPMOST,
			g_unk0x10096a50.left,
			g_unk0x10096a50.top,
			g_unk0x10096a50.right,
			g_unk0x10096a50.bottom,
			SWP_NOACTIVATE
		);
		if (g_unk0x1006a9d8 && !FUN_1003bf90(4)) {
			while (ShowCursor(TRUE) < 0) {
			}
		}
	}
	else if (g_unk0x1006a9d8) {
		while (ShowCursor(FALSE) >= 0) {
		}
	}

	if (GetSystemMetrics(SM_CXSCREEN) <= g_drawModeWidth && GetSystemMetrics(SM_CYSCREEN) <= g_drawModeHeight) {
		g_nWindowMode = c_windowModeFullscreen;
	}
	else {
		g_nWindowMode = p_extension->m_windowMode;
	}
}

// Operand order: the original compares i < p_count as `cmp [p_count], eax` and adds p_first + i
// with p_first loaded first; here both come out the other way round since the draw mode tables
// joined this unit (one declaration-order attempt didn't flip them).
// FUNCTION: MW2SHELL 0x10011450
MechS32 FUN_10011450(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette)
{
	MechS32 i;

	if (p_palette == NULL || p_first < 0 || p_first > 0xff || p_count <= 0 || p_count > 0x100 - p_first) {
		return -1;
	}

	for (i = 0; i < p_count; i++) {
		p_palette[i] = g_paletteColors[p_first + i];
	}

	return 0;
}

// STUB: MW2SHELL 0x100114f0
MechS32 FUN_100114f0()
{
	STUB(0x100114f0);
	return 0;
}

// STUB: MW2SHELL 0x10011580
MechS32 FUN_10011580()
{
	STUB(0x10011580);
	return 0;
}

// STUB: MW2SHELL 0x10011a15
MechS32 FUN_10011a15(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height)
{
	STUB(0x10011a15);
	return 0;
}

// STUB: MW2SHELL 0x10011e5c
MechS32 FUN_10011e5c()
{
	STUB(0x10011e5c);
	return 0;
}

// STUB: MW2SHELL 0x10011fdf
MechS32 FUN_10011fdf()
{
	STUB(0x10011fdf);
	return 0;
}

// STUB: MW2SHELL 0x1001216c
MechS32 FUN_1001216c()
{
	STUB(0x1001216c);
	return 0;
}

// STUB: MW2SHELL 0x10012337
MechS32 FUN_10012337(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	STUB(0x10012337);
	return 0;
}

// STUB: MW2SHELL 0x100126d9
MechS32 FUN_100126d9(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	STUB(0x100126d9);
	return 0;
}

// STUB: MW2SHELL 0x10012add
MechS32 FUN_10012add(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette, MechS32 p_allColors)
{
	STUB(0x10012add);
	return 0;
}

// STUB: MW2SHELL 0x10012db7
MechS32 FUN_10012db7(PaletteColor* p_palette)
{
	STUB(0x10012db7);
	return 0;
}

// STUB: MW2SHELL 0x10013068
MechS32 FUN_10013068(PaletteColor* p_palette, MechS32 p_steps)
{
	STUB(0x10013068);
	return 0;
}

// STUB: MW2SHELL 0x10013551
MechS32 FUN_10013551()
{
	STUB(0x10013551);
	return 0;
}
