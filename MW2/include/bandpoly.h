#ifndef BANDPOLY_H
#define BANDPOLY_H

#include "decomp.h"
#include "point.h"
#ifdef MW2_MATROX
#include "matrox/floatpoint.h"
#endif
#include "projectedvertex.h"
#include "types.h"

// The functions and globals of bandpoly.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MW2_MATROX
	extern FloatPoint g_bandTexCoords[4];
#else
extern Point g_bandTexCoords[4];
#endif

#ifdef MW2_MATROX
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
#ifndef MW2_MATROX
	MechS32 DrawBandWithFlash(undefined4 p_unk0x00, MechS32 p_unk0x04, undefined4* p_unk0x08);
#endif

#ifdef __cplusplus
}
#endif

#endif // BANDPOLY_H
