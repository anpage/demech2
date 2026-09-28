#ifndef BLIT_H
#define BLIT_H

#include "decomp.h"
#include "palettecolor.h"
#include "pixelview.h"
#include "types.h"

// The functions and globals of blit.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 FUN_10037504(void* p_data, MechS32 p_index);
	MechS32 FUN_10037684(void* p_data);
	MechS32 FontGetHeight(void* p_data);
	MechS32 FontGetCharWidth(void* p_data, MechS32 p_char);
	MechS32 FUN_10032f84(PixelView* p_view, undefined4 p_unk0x04, undefined4 p_unk0x08, MechS32 p_left, MechS32 p_top);
	void FUN_10034f18(
		PixelView* p_unk0x00,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		PixelView* p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_unk0x14,
		MechS32 p_unk0x18
	);
	MechS32 FUN_10032449(
		PixelView* p_view,
		MechS32 p_left,
		MechS32 p_top,
		MechS32 p_right,
		MechS32 p_bottom,
		MechS32 p_unk0x14,
		MechS32 p_color
	);
	void FUN_10034e15(PixelView* p_view, MechS32 p_unk0x04);
	MechS32 BlitChar(
		PixelView* p_view,
		MechS32 p_left,
		MechS32 p_top,
		void* p_font,
		MechS32 p_char,
		undefined* p_palette
	);
	void BlitString(
		PixelView* p_view,
		MechS32 p_left,
		MechS32 p_top,
		void* p_font,
		MechChar* p_text,
		undefined* p_palette
	);
	void FUN_10037014(PixelView* p_view, undefined* p_data);
	void FUN_10037096(undefined* p_data, MechS32 p_size, PaletteColor* p_palette);
	MechS32 FUN_100370c1(undefined* p_data);

#ifdef __cplusplus
}
#endif

#endif // BLIT_H
