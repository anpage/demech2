#ifndef UNK10041FA0_H
#define UNK10041FA0_H

#include "copperwren.h"
#include "mappoint.h"
#include "types.h"
#include "unk1003a530.h"

// The functions and globals of unk10041fa0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10041fa0(MechS32* p_pose, MechS32 p_slot, MechS32 p_worldSpan, MechS32 p_far);
	void FUN_1004215f(MechU32 p_flags);
	void FUN_10042195(void);
	MechS32 FUN_10042206(ScarletOrchid0x4c* p_shape);
	CopperWren0x20* FUN_100423b3(CopperWren0x20* p_vertex);
	MechS32 FUN_1004251e(MapPoint* p_point);

#ifdef __cplusplus
}
#endif

#endif // UNK10041FA0_H
