#ifndef UNK10010A30_H
#define UNK10010A30_H

#include "decomp.h"
#include "drawbitmapinfo.h"
#include "drawmode.h"
#include "drawmodeextension.h"
#include "palettecolor.h"
#include "pixelbuffer.h"
#include "types.h"

#include <windows.h>

// The functions and globals of unk10010a30.c that other units use.
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

#endif // UNK10010A30_H
