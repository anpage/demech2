#ifndef FIXEDSQRT_H
#define FIXEDSQRT_H

#include "fixedfloat.h"
#include "types.h"

// The functions and globals of fixedsqrt.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechU32 FixedSqrt16(MechU32 p_value);
	void NormalizeVectorGuarded(MechScalar* p_x, MechScalar* p_y, MechScalar* p_z);

#ifdef __cplusplus
}
#endif

#endif // FIXEDSQRT_H
