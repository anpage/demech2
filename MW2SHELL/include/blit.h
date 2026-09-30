#ifndef BLIT_H
#define BLIT_H

#include "decomp.h"
#include "palettecolor.h"
#include "pixelview.h"
#include "types.h"

// The functions and globals of blit.asm that other units use.
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

// blit.asm's routines and data. reccmp reads annotations from C sources only, so they're here,
// by name.

// FUNCTION: MW2SHELL 0x10032250
// GetDisplayDriverName

// FUNCTION: MW2SHELL 0x10032279
// SetDisplayDriver

// FUNCTION: MW2SHELL 0x10032298
// PutViewPixel

// FUNCTION: MW2SHELL 0x10032373
// GetViewPixel

// FUNCTION: MW2SHELL 0x10032449
// BlitLine

// FUNCTION: MW2SHELL 0x10032e4b
// FUN_10032e4b

// FUNCTION: MW2SHELL 0x10032f84
// BlitShpFrame

// FUNCTION: MW2SHELL 0x100333f8
// BlitShpFrameUnclipped

// FUNCTION: MW2SHELL 0x100334fb
// SetRemapTable

// FUNCTION: MW2SHELL 0x1003351a
// BlitShpFrameRemapped

// FUNCTION: MW2SHELL 0x10033980
// BlitShpFrameRemappedUnclipped

// FUNCTION: MW2SHELL 0x10033a76
// BlitRotated

// FUNCTION: MW2SHELL 0x10034622
// FUN_10034622

// FUNCTION: MW2SHELL 0x1003479a
// EncodeViewRle

// FUNCTION: MW2SHELL 0x10034a1d
// RemapShpFrame

// FUNCTION: MW2SHELL 0x10034aaf
// EncodeRleRow

// FUNCTION: MW2SHELL 0x10034c38
// EmitRleRun

// FUNCTION: MW2SHELL 0x10034e15
// FillView

// FUNCTION: MW2SHELL 0x10034f18
// BlitView

// FUNCTION: MW2SHELL 0x100352b4
// ScrollView

// FUNCTION: MW2SHELL 0x100354b1
// DrawEllipse

// FUNCTION: MW2SHELL 0x100357f2
// FillEllipse

// GLOBAL: MW2SHELL 0x10035af0
// g_cosTable

// FUNCTION: MW2SHELL 0x10036904
// GetCosSin

// FUNCTION: MW2SHELL 0x100369bc
// BlitFixedMul16

// FUNCTION: MW2SHELL 0x100369e2
// RotateScalePoint

// FUNCTION: MW2SHELL 0x10036aa9
// FontGetHeight

// FUNCTION: MW2SHELL 0x10036abc
// FontGetCharWidth

// FUNCTION: MW2SHELL 0x10036adc
// BlitChar

// FUNCTION: MW2SHELL 0x10036c67
// BlitString

// FUNCTION: MW2SHELL 0x10036c9e
// WriteViewRow

// GLOBAL: MW2SHELL 0x10036da1
// g_iffBmhdTag

// GLOBAL: MW2SHELL 0x10036da5
// g_iffCmapTag

// GLOBAL: MW2SHELL 0x10036da9
// g_iffBodyTag

// FUNCTION: MW2SHELL 0x10036dad
// FindIffChunk

// FUNCTION: MW2SHELL 0x10036def
// BlitIff

// FUNCTION: MW2SHELL 0x10036fb6
// ReadIffPalette

// FUNCTION: MW2SHELL 0x10036fe7
// GetIffSize

// FUNCTION: MW2SHELL 0x10037014
// BlitPicture

// FUNCTION: MW2SHELL 0x10037096
// ReadPicturePalette

// FUNCTION: MW2SHELL 0x100370c1
// GetPictureSize

// FUNCTION: MW2SHELL 0x100370e8
// GifInitCodes

// FUNCTION: MW2SHELL 0x10037130
// GifReadByte

// FUNCTION: MW2SHELL 0x10037149
// GifReadCode

// FUNCTION: MW2SHELL 0x1003718f
// GifAddCode

// FUNCTION: MW2SHELL 0x100371d5
// GifPutPixel

// FUNCTION: MW2SHELL 0x10037252
// BlitGif

// FUNCTION: MW2SHELL 0x1003746b
// ReadGifPalette

// FUNCTION: MW2SHELL 0x100374cc
// GetGifSize

// FUNCTION: MW2SHELL 0x10037504
// GetShpFrameSize

// FUNCTION: MW2SHELL 0x10037526
// FUN_10037526

// FUNCTION: MW2SHELL 0x10037549
// GetShpFrameExtent

// FUNCTION: MW2SHELL 0x1003757d
// GetShpFrameOrigin

// FUNCTION: MW2SHELL 0x100375a7
// FUN_100375a7

// FUNCTION: MW2SHELL 0x100375f2
// FUN_100375f2

// FUNCTION: MW2SHELL 0x1003763a
// FUN_1003763a

// FUNCTION: MW2SHELL 0x10037684
// GetShpFrameCount

// FUNCTION: MW2SHELL 0x10037697
// CountShpUniqueFrames

// FUNCTION: MW2SHELL 0x100376f9
// FUN_100376f9

// GLOBAL: MW2SHELL 0x1003775b
// g_dissolveTaps

// FUNCTION: MW2SHELL 0x100377d7
// DissolveView

// FUNCTION: MW2SHELL 0x10037a4e
// FadeViewColors

// FUNCTION: MW2SHELL 0x10037bd2
// CountViewColors

// GLOBAL: MW2SHELL 0x100687cc
// g_displayDriver

// GLOBAL: MW2SHELL 0x10068800
// g_displayDriverName

// GLOBAL: MW2SHELL 0x1006880d
// g_rleOutput

// GLOBAL: MW2SHELL 0x10068811
// g_rleSkip

// GLOBAL: MW2SHELL 0x10068815
// g_rleRow

// GLOBAL: MW2SHELL 0x10068819
// g_rleRun

// GLOBAL: MW2SHELL 0x1006881d
// g_rleCursor

// GLOBAL: MW2SHELL 0x10068821
// g_rleRunStart

// GLOBAL: MW2SHELL 0x10068835
// g_rleLeft

// GLOBAL: MW2SHELL 0x10068839
// g_rleTop

// GLOBAL: MW2SHELL 0x1006883d
// g_rleRight

// GLOBAL: MW2SHELL 0x10068841
// g_rleBottom

// GLOBAL: MW2SHELL 0x10068845
// g_unk0x10068845

// GLOBAL: MW2SHELL 0x10068d45
// g_scanline

// GLOBAL: MW2SHELL 0x10069045
// g_fadeColors

// GLOBAL: MW2SHELL 0x10069145
// g_fadeDistances

// GLOBAL: MW2SHELL 0x10069445
// g_unk0x10069445

// GLOBAL: MW2SHELL 0x10069a45
// g_unk0x10069a45

// GLOBAL: MW2SHELL 0x10069d45
// g_fadeErrors

// GLOBAL: MW2SHELL 0x1006a045
// g_gifCodeMasks

// GLOBAL: MW2SHELL 0x1006a04e
// g_gifPassSteps

// GLOBAL: MW2SHELL 0x1006a053
// g_gifPassStarts

// GLOBAL: MW2SHELL 0x1006a058
// g_gifView

// GLOBAL: MW2SHELL 0x1006a05c
// g_remapTable

// GLOBAL: MW2SHELL 0x1006a15c
// g_rotatedCorners

// GLOBAL: MW2SHELL 0x1006a1ac
// g_rotatedCornerSteps

#endif // BLIT_H
