#ifndef GIFSAVE_H
#define GIFSAVE_H

#include "types.h"

// GIFSAVE's result codes.
enum {
	c_gifOk = 0,
	c_gifErrCreate = 1,
	c_gifErrWrite = 2,
	c_gifOutOfMemory = 3
};

// The functions of gifsave.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 GifCreate(
		const MechChar* p_filename,
		MechU32 p_width,
		MechU32 p_height,
		MechU32 p_numColors,
		MechS32 p_colorRes
	);
	void GifSetColor(MechS32 p_colorNum, MechS32 p_red, MechS32 p_green, MechS32 p_blue);
	MechS32 GifCompressImage(
		MechS32 p_left,
		MechS32 p_top,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 (*p_getPixel)(MechS32 p_x, MechS32 p_y)
	);
	MechS32 GifClose(void);

#ifdef __cplusplus
}
#endif

// The 3Dfx edition places these out of gifsave.c's order, so they are annotated here, by name.

// FUNCTION: MW23DFX 0x1003bb80
// GifCloseFile

// FUNCTION: MW23DFX 0x1003bf60
// LzwCompress

// FUNCTION: MW23DFX 0x1003c070
// BitsNeeded

// FUNCTION: MW23DFX 0x1003c0f0
// WriteScreenDescriptor

// FUNCTION: MW23DFX 0x1003c1e0
// GifCreate

// FUNCTION: MW23DFX 0x1003c360
// GifSetColor

// FUNCTION: MW23DFX 0x1003c3c0
// GifCompressImage

#endif // GIFSAVE_H
