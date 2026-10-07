#ifndef INTEGRATE_H
#define INTEGRATE_H

#include "types.h"

// The functions and globals of integrate.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MW2_MATROX
	// The Matrox edition integrates floats in place: its callers read the arguments where they are.
#define IntegrateMidpoint(p_position, p_velocity, p_acceleration, p_time)                                              \
	{                                                                                                                  \
		MechFloat midpointStep;                                                                                        \
		midpointStep = (p_time) * (p_acceleration);                                                                    \
		*(p_position) += (midpointStep * 0.5f + *(p_velocity)) * (p_time);                                             \
		*(p_velocity) += midpointStep;                                                                                 \
	}
#else
void IntegrateMidpoint(MechS32* p_position, MechS32* p_velocity, MechS32 p_acceleration, MechS32 p_time);
#endif

#ifdef __cplusplus
}
#endif

#endif // INTEGRATE_H
