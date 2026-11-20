#ifndef BANDPOLY_H
#define BANDPOLY_H

#include "decomp.h"
#include "point.h"
#ifdef MW2_FLOAT_SCALARS
#include "accel/floatpoint.h"
#endif
#include "projectedvertex.h"
#include "types.h"
#ifdef MW2_ATI
#include "accel/a3dvertex.h"
#include "pane.h"
#endif

// The functions and globals of bandpoly.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MW2_FLOAT_SCALARS
	extern FloatPoint g_bandTexCoords[4];
#else
extern Point g_bandTexCoords[4];
#endif

#ifdef MW2_A3D
	MechS32 DrawBand(
		MechU32 p_color,
		MechS32 p_count,
		ProjectedVertex** p_points,
		MechS32 p_luma,
		MechS32 p_mode,
		MechS32 p_unk0x14
	);
#else
void DrawBandPolygon(MechU32 p_flags, MechS32 p_count, MechU32* p_points, MechS32 p_unk0x0c);
MechS32 DrawBand(MechU32 p_color, MechS32 p_count, MechS32* p_points, MechS32 p_luma, MechS32 p_mode);
#endif
#ifndef MW2_A3D
	MechS32 DrawBandWithFlash(undefined4 p_unk0x00, MechS32 p_unk0x04, undefined4* p_unk0x08);
#endif
#ifdef MW2_ATI
	void FUN_1002a480(
		PANE* p_pane,
		MechU16* p_pixels,
		MechS16 p_width,
		MechS16 p_height,
		MechS32 p_count,
		A3DVertex* p_vertices,
		MechS32 p_mip,
		undefined4 p_unk0x1c,
		MechS32 p_translucent
	);
#endif

#ifdef __cplusplus
}
#endif

#endif // BANDPOLY_H
