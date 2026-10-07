#ifndef APPROXLEN_H
#define APPROXLEN_H

#include "fixedfloat.h"
#include "types.h"

// The functions and globals of approxlen.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechScalar ApproximateVectorLength(MechScalar p_x, MechScalar p_y, MechScalar p_z);

#ifdef __cplusplus
}
#endif

#endif // APPROXLEN_H
