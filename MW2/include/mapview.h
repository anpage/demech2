#ifndef MAPVIEW_H
#define MAPVIEW_H

#include "mappoint.h"
#include "projectedvertex.h"
#include "shape.h"
#include "types.h"

// The functions and globals of mapview.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10041fa0(MechS32* p_pose, MechS32 p_slot, MechS32 p_worldSpan, MechS32 p_far);
	void FUN_1004215f(MechU32 p_flags);
	void FUN_10042195(void);
	MechS32 FUN_10042206(Shape* p_shape);
	ProjectedVertex* FUN_100423b3(ProjectedVertex* p_vertex);
	MechS32 FUN_1004251e(MapPoint* p_point);

#ifdef __cplusplus
}
#endif

#endif // MAPVIEW_H
