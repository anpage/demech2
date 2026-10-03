#ifndef HORIZON_H
#define HORIZON_H

#include "types.h"

struct Eyepoint;

// The functions of horizon.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 FUN_10071930(MechS32 p_x, MechS32 p_y, struct Eyepoint* p_eyepoint);
	MechS32 FUN_100719ca(MechS32 p_x, struct Eyepoint* p_eyepoint);
	MechS32 FUN_10071a4c(MechS32 p_y, struct Eyepoint* p_eyepoint);

#ifdef __cplusplus
}
#endif

#endif // HORIZON_H
