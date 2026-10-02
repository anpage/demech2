#ifndef BLIT_H
#define BLIT_H

#include "decomp.h"
#include "pane.h"
#include "types.h"

// The routines of blit.asm (common/src, shared with the shell; its portable C, blit.c, in
// COMPAT_MODE) that other units use. The shell's blit.h declares them with the same types, for
// blit.c.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 PutViewPixel(Pane* p_target, MechS32 p_x, MechS32 p_y, MechU32 p_color);
	MechS32 GetViewPixel(Pane* p_target, MechS32 p_x, MechS32 p_y);
	MechS32 BlitLine(
		Pane* p_target,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_x2,
		MechS32 p_y2,
		MechS32 p_mode,
		MechS32 p_color
	);
	void FUN_10032e4b(Pane* p_target, MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom, MechU8 p_color);
	void BlitShpFrame(Pane* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y);
	void FillView(Pane* p_target, MechS32 p_color);
	void DrawEllipse(
		Pane* p_target,
		MechS32 p_centerX,
		MechS32 p_centerY,
		MechS32 p_radiusX,
		MechS32 p_radiusY,
		MechS32 p_color
	);
	void FillEllipse(
		Pane* p_target,
		MechS32 p_centerX,
		MechS32 p_centerY,
		MechS32 p_radiusX,
		MechS32 p_radiusY,
		MechS32 p_color
	);
	void GetCosSin(MechS32 p_angle, MechS32* p_cos, MechS32* p_sin);
	void RotateScalePoint(
		MechS32* p_point,
		MechS32* p_result,
		MechS32* p_origin,
		MechS32 p_angle,
		MechS32 p_scaleX,
		MechS32 p_scaleY
	);
	MechS32 FontGetHeight(void* p_font);
	MechS32 FontGetCharWidth(void* p_font, MechS32 p_char);
	MechS32 BlitChar(Pane* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_char, void* p_unk0x14);
	void BlitString(Pane* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechChar* p_text, void* p_unk0x14);
	MechS32 GetGifSize(void* p_shape);
	MechS32 GetShpFrameSize(void* p_shape, MechS32 p_frame);
	MechS32 FUN_10037526(void* p_shape, MechS32 p_frame);
	MechS32 GetShpFrameExtent(void* p_shape, MechS32 p_frame);
	MechS32 GetShpFrameOrigin(void* p_shape, MechS32 p_frame);
	MechS32 GetShpFrameCount(void* p_shape);
	MechS32 DissolveView(Pane* p_dst, Pane* p_src, MechS32 p_unk0x08, MechS32 p_unk0x0c);
	MechChar* GetDisplayDriverName(MechChar* (**p_driver)(void) );
	void SetDisplayDriver(MechChar* (**p_driver)(void) );
	void BlitShpFrameUnclipped(Pane* p_target, void* p_frame, MechS32 p_x, MechS32 p_y, undefined4 p_unk0x10);
	void SetRemapTable(MechU8* p_map);
	MechS32 BlitShpFrameRemappedUnclipped(
		Pane* p_target,
		void* p_frame,
		MechS32 p_x,
		MechS32 p_y,
		undefined4 p_unk0x10
	);
	MechS32 BlitShpFrameRemapped(Pane* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y);
	MechS32 FUN_10034622(
		void* p_shape,
		MechS32 p_frame,
		MechS32 p_x,
		MechS32 p_y,
		undefined4 p_flags,
		MechS32* p_bounds
	);
	MechS32 RemapShpFrame(void* p_shape, MechS32 p_frame);
	MechS32 BlitView(
		Pane* p_source,
		MechS32 p_sourceX,
		MechS32 p_sourceY,
		Pane* p_dest,
		MechS32 p_destX,
		MechS32 p_destY,
		MechS32 p_fillColor
	);
	MechS32 ScrollView(Pane* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height);
	void BlitFixedMul16(MechS32 p_a, MechS32 p_b, MechS32* p_result);
	MechS32 WriteViewRow(Pane* p_target, MechS32 p_row, MechU8* p_src, MechS32 p_width);
	MechU8* FindIffChunk(MechChar* p_tag, MechU8* p_iff);
	MechS32 BlitIff(Pane* p_target, MechU8* p_iff);
	void ReadIffPalette(MechU8* p_iff, MechU8* p_palette);
	MechS32 GetIffSize(MechU8* p_iff);
	MechS32 BlitPicture(Pane* p_target, MechU8* p_pcx);
	void ReadPicturePalette(MechU8* p_pcx, MechS32 p_size, void* p_palette);
	MechS32 GetPictureSize(MechU8* p_pcx);
	void GifInitCodes(void);
	void GifReadByte(void);
	void GifReadCode(void);
	void GifAddCode(void);
	void GifPutPixel(void);
	MechS32 BlitGif(Pane* p_target, MechU8* p_gif, MechU8* p_state);
	void ReadGifPalette(MechU8* p_gif, MechU8* p_palette);
	void FUN_100375a7(void* p_shape, MechS32 p_frame, MechU8* p_palette);
	MechS32 FUN_100375f2(void* p_shape, MechS32 p_frame, MechU32* p_out);
	MechS32 FUN_1003763a(void* p_shape, MechS32 p_frame, MechU32* p_in);
	MechS32 CountShpUniqueFrames(void* p_shape, MechS32* p_out);
	MechS32 FUN_100376f9(void* p_shape, MechS32* p_out);
	MechS32 CountViewColors(Pane* p_target, MechU32* p_out);

#ifdef __cplusplus
}
#endif

// blit.asm's routines and data. reccmp reads annotations from C sources only, so they're here, by
// name.

// FUNCTION: MW2 0x100604f4
// GetDisplayDriverName

// FUNCTION: MW2 0x1006051d
// SetDisplayDriver

// FUNCTION: MW2 0x1006053c
// PutViewPixel

// FUNCTION: MW2 0x10060617
// GetViewPixel

// FUNCTION: MW2 0x100606ed
// BlitLine

// FUNCTION: MW2 0x100610ef
// FUN_10032e4b

// FUNCTION: MW2 0x10061228
// BlitShpFrame

// FUNCTION: MW2 0x1006169c
// BlitShpFrameUnclipped

// FUNCTION: MW2 0x1006179f
// SetRemapTable

// FUNCTION: MW2 0x100617be
// BlitShpFrameRemapped

// FUNCTION: MW2 0x10061c24
// BlitShpFrameRemappedUnclipped

// FUNCTION: MW2 0x10061d1a
// BlitRotated

// FUNCTION: MW2 0x100628c6
// FUN_10034622

// FUNCTION: MW2 0x10062a3e
// EncodeViewRle

// FUNCTION: MW2 0x10062cc1
// RemapShpFrame

// FUNCTION: MW2 0x10062d53
// EncodeRleRow

// FUNCTION: MW2 0x10062edc
// EmitRleRun

// FUNCTION: MW2 0x100630b9
// FillView

// FUNCTION: MW2 0x100631bc
// BlitView

// FUNCTION: MW2 0x10063558
// ScrollView

// FUNCTION: MW2 0x10063755
// DrawEllipse

// FUNCTION: MW2 0x10063a96
// FillEllipse

// GLOBAL: MW2 0x10063d94
// g_cosTable

// FUNCTION: MW2 0x10064ba8
// GetCosSin

// FUNCTION: MW2 0x10064c60
// BlitFixedMul16

// FUNCTION: MW2 0x10064c86
// RotateScalePoint

// FUNCTION: MW2 0x10064d4d
// FontGetHeight

// FUNCTION: MW2 0x10064d60
// FontGetCharWidth

// FUNCTION: MW2 0x10064d80
// BlitChar

// FUNCTION: MW2 0x10064f0b
// BlitString

// FUNCTION: MW2 0x10064f42
// WriteViewRow

// GLOBAL: MW2 0x10065045
// g_iffBmhdTag

// GLOBAL: MW2 0x10065049
// g_iffCmapTag

// GLOBAL: MW2 0x1006504d
// g_iffBodyTag

// FUNCTION: MW2 0x10065051
// FindIffChunk

// FUNCTION: MW2 0x10065093
// BlitIff

// FUNCTION: MW2 0x1006525a
// ReadIffPalette

// FUNCTION: MW2 0x1006528b
// GetIffSize

// FUNCTION: MW2 0x100652b8
// BlitPicture

// FUNCTION: MW2 0x1006533a
// ReadPicturePalette

// FUNCTION: MW2 0x10065365
// GetPictureSize

// FUNCTION: MW2 0x1006538c
// GifInitCodes

// FUNCTION: MW2 0x100653d4
// GifReadByte

// FUNCTION: MW2 0x100653ed
// GifReadCode

// FUNCTION: MW2 0x10065433
// GifAddCode

// FUNCTION: MW2 0x10065479
// GifPutPixel

// FUNCTION: MW2 0x100654f6
// BlitGif

// FUNCTION: MW2 0x1006570f
// ReadGifPalette

// FUNCTION: MW2 0x10065770
// GetGifSize

// FUNCTION: MW2 0x100657a8
// GetShpFrameSize

// FUNCTION: MW2 0x100657ca
// FUN_10037526

// FUNCTION: MW2 0x100657ed
// GetShpFrameExtent

// FUNCTION: MW2 0x10065821
// GetShpFrameOrigin

// FUNCTION: MW2 0x1006584b
// FUN_100375a7

// FUNCTION: MW2 0x10065896
// FUN_100375f2

// FUNCTION: MW2 0x100658de
// FUN_1003763a

// FUNCTION: MW2 0x10065928
// GetShpFrameCount

// FUNCTION: MW2 0x1006593b
// CountShpUniqueFrames

// FUNCTION: MW2 0x1006599d
// FUN_100376f9

// GLOBAL: MW2 0x100659ff
// g_dissolveTaps

// FUNCTION: MW2 0x10065a7b
// DissolveView

// FUNCTION: MW2 0x10065cf2
// FadeViewColors

// FUNCTION: MW2 0x10065e76
// CountViewColors

// GLOBAL: MW2 0x100ab100
// g_displayDriver

// GLOBAL: MW2 0x100ab134
// g_displayDriverName

// GLOBAL: MW2 0x100ab141
// g_rleOutput

// GLOBAL: MW2 0x100ab145
// g_rleSkip

// GLOBAL: MW2 0x100ab149
// g_rleRow

// GLOBAL: MW2 0x100ab14d
// g_rleRun

// GLOBAL: MW2 0x100ab151
// g_rleCursor

// GLOBAL: MW2 0x100ab155
// g_rleRunStart

// GLOBAL: MW2 0x100ab169
// g_rleLeft

// GLOBAL: MW2 0x100ab16d
// g_rleTop

// GLOBAL: MW2 0x100ab171
// g_rleRight

// GLOBAL: MW2 0x100ab175
// g_rleBottom

// GLOBAL: MW2 0x100ab179
// g_unk0x10068845

// GLOBAL: MW2 0x100ab679
// g_scanline

// GLOBAL: MW2 0x100ab979
// g_fadeColors

// GLOBAL: MW2 0x100aba79
// g_fadeDistances

// GLOBAL: MW2 0x100abd79
// g_unk0x10069445

// GLOBAL: MW2 0x100ac379
// g_unk0x10069a45

// GLOBAL: MW2 0x100ac679
// g_fadeErrors

// GLOBAL: MW2 0x100ac979
// g_gifCodeMasks

// GLOBAL: MW2 0x100ac982
// g_gifPassSteps

// GLOBAL: MW2 0x100ac987
// g_gifPassStarts

// GLOBAL: MW2 0x100ac98c
// g_gifView

// GLOBAL: MW2 0x100ac990
// g_remapTable

// GLOBAL: MW2 0x100aca90
// g_rotatedCorners

// GLOBAL: MW2 0x100acae0
// g_rotatedCornerSteps

#endif // BLIT_H
