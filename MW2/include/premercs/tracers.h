#ifndef PREMERCS_TRACERS_H
#define PREMERCS_TRACERS_H

#include "fixedfloat.h"
#include "types.h"

// The functions of tracers.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10088c5c(
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_dx,
		MechScalar p_dy,
		MechScalar p_dz
	);
	void FUN_10089624(
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar p_dx,
		MechScalar p_dy,
		MechScalar p_dz
	);

#ifdef __cplusplus
}
#endif

#endif // PREMERCS_TRACERS_H
