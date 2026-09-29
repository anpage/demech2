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

	MechS32 GetShpFrameSize(void* p_data, MechS32 p_index);
	MechS32 GetShpFrameCount(void* p_data);
	MechS32 FontGetHeight(void* p_data);
	MechS32 FontGetCharWidth(void* p_data, MechS32 p_char);
	MechS32 BlitShpFrame(PixelView* p_view, undefined4 p_shp, undefined4 p_frame, MechS32 p_left, MechS32 p_top);
	void BlitView(
		PixelView* p_source,
		MechS32 p_sourceLeft,
		MechS32 p_sourceTop,
		PixelView* p_dest,
		MechS32 p_destLeft,
		MechS32 p_destTop,
		MechS32 p_fillColor
	);
	MechS32 BlitLine(
		PixelView* p_view,
		MechS32 p_left,
		MechS32 p_top,
		MechS32 p_right,
		MechS32 p_bottom,
		MechS32 p_unk0x14,
		MechS32 p_color
	);
	void FillView(PixelView* p_view, MechS32 p_color);
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
	void BlitPicture(PixelView* p_view, undefined* p_data);
	void ReadPicturePalette(undefined* p_data, MechS32 p_size, PaletteColor* p_palette);
	MechS32 GetPictureSize(undefined* p_data);

#ifdef __cplusplus
}
#endif

#endif // BLIT_H
