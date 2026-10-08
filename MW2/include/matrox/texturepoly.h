#ifndef MATROX_TEXTUREPOLY_H
#define MATROX_TEXTUREPOLY_H

#include "decomp.h"
#include "matrox/a3d.h"
#include "pane.h"
#include "types.h"

// The functions of matrox/texturepoly.c (the Matrox edition's) that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_100172c0(
		PANE* p_pane,
		A3DTexture* p_texture,
		MechS32 p_count,
		A3DVertex* p_vertices,
		undefined4 p_unk0x10,
		undefined4 p_unk0x14,
		MechS32 p_unk0x18
	);

#ifdef __cplusplus
}
#endif

#endif // MATROX_TEXTUREPOLY_H
