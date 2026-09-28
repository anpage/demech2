#ifndef DRAWMODE_H
#define DRAWMODE_H

#include "decomp.h"
#include "drawbitmapinfo.h"
#include "drawmodeextension.h"
#include "palettecolor.h"
#include "pixelbuffer.h"
#include "types.h"

#include <windows.h>

#pragma pack(1)
// A display mode of one of the back ends in g_drawModeExtensions: how the framebuffer reaches
// the screen.
// SIZE 0x24
struct DrawMode {
	MechS32 m_index;                                                              // 0x00 — in g_drawModes
	MechS32 m_extension;                                                          // 0x04 — in g_drawModeExtensions
	MechS32 m_available;                                                          // 0x08 — cleared when m_begin fails
	MechU32 m_profileTime;                                                        // 0x0c
	MechS32 (*m_begin)(PixelBuffer* p_buffer, MechS32 p_width, MechS32 p_height); // 0x10
	MechS32 (*m_end)();                                                           // 0x14
	MechS32 (*m_flip)();                                                          // 0x18
	MechS32 (*m_blitRect)(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);    // 0x1c
	MechS32 (*m_stretchBlit)(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom); // 0x20
};
typedef struct DrawMode DrawMode;
#pragma pack()

// The functions and globals of drawmode.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern DrawModeExtension* g_drawModeExtensions[3];
	extern DrawModeExtension* g_currentDrawModeExtension;
	extern DrawMode* g_currentDrawMode;
	extern PixelBuffer* g_unk0x10062cdc;
	extern PaletteColor g_paletteColors[0x100];
	extern undefined* g_unk0x10062fe0;
	extern MechS32 g_nWindowMode;
	extern MechS32 g_windowHeight;
	extern MechS32 g_windowWidth;
	extern HINSTANCE g_pModule;
	extern HWND g_pWnd;
	extern HMENU g_windowMenu;
	extern DrawBitmapInfo g_bitmapInfo;
	extern MechS32 g_unk0x10096e88;
	extern MechS32 g_drawModeWidth;
	extern MechS32 g_drawModeHeight;

	void InitBitmapInfo(MechS32 p_width, MechS32 p_height);
	void AdjustWindowSize(DrawModeExtension* p_extension);
	MechS32 FUN_10011450(MechS32 p_first, MechS32 p_count, PaletteColor* p_palette);
	MechS32 InitDrawMode(
		MechS32 p_mode,
		MechS32 p_allowFallback,
		PixelBuffer* p_buffer,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 p_menu
	);
	void FUN_10010d49();

#ifdef __cplusplus
}
#endif

#endif // DRAWMODE_H
