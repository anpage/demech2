#ifndef FIXEDTRIG_H
#define FIXEDTRIG_H

#include "fixedfloat.h"
#include "types.h"

// The functions and globals of fixedtrig.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#if defined(MW2_MATROX) && defined(FIXEDTRIG_FLOAT_SINE)
	// The Matrox edition's sine takes and returns degrees as floats, and its callers expand the cosine
	// (FixedSin(p_angle + 90.0f)). Only fixedtrig.c sees this declaration for now: door.c,
	// mechclass.c and screenscale.c still shift the 16.16 result (FixedSin(x) >> n), which
	// a float doesn't compile with.
	MechScalar FixedSin(MechScalar p_angle);
#define FixedCos(p_angle) FixedSin((p_angle) + 90.0f)
#else
MechS32 FixedSin(MechS32 p_angle);
MechS32 FixedCos(MechS32 p_angle);
#endif
	MechScalar FixedAtan2(MechScalar p_x, MechScalar p_z);
#ifdef MW2_MATROX
	// The Matrox edition's arcsine and arccosine are the CRT's, in degrees.
#define FixedAsin(p_sine) (asin(p_sine) * 57.29577951308232)
#define FixedAcos(p_cosine) (acos(p_cosine) * 57.29577951308232)
#else
MechS32 FixedAsin(MechS32 p_sine);
MechS32 FixedAcos(MechS32 p_cosine);
#endif

#ifdef __cplusplus
}
#endif

#endif // FIXEDTRIG_H
