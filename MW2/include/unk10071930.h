#ifndef UNK10071930_H
#define UNK10071930_H

#include "types.h"

struct Eyepoint;

// The functions of unk10071930.c that other units use.
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

#endif // UNK10071930_H
