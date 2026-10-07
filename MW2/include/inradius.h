#ifndef INRADIUS_H
#define INRADIUS_H

#include "types.h"

// The functions and globals of inradius.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MW2_MATROX
	// The Matrox edition's is an __inline function of floats, which its callers expand (/Ob1).
	__inline MechS32 IsWithinRadius(MechFloat p_x, MechFloat p_y, MechFloat p_z, MechFloat p_radius)
	{
		return p_x * p_x + p_y * p_y + p_z * p_z <= p_radius * p_radius;
	}
#else
MechS32 IsWithinRadius(MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32 p_radius);
#endif

#ifdef __cplusplus
}
#endif

#endif // INRADIUS_H
