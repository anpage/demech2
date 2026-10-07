#ifndef FIXEDTRIG_H
#define FIXEDTRIG_H

#include "fixedfloat.h"
#include "types.h"

// The functions and globals of fixedtrig.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 FixedSin(MechS32 p_angle);
	MechS32 FixedCos(MechS32 p_angle);
	MechScalar FixedAtan2(MechScalar p_x, MechScalar p_z);
#ifdef MW2_MATROX
	// The edition's arcsine and arccosine are the CRT's, in degrees.
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
