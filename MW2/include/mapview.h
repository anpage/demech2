#ifndef MAPVIEW_H
#define MAPVIEW_H

#include "fixedfloat.h"
#include "mappoint.h"
#include "projectedvertex.h"
#include "shape.h"
#include "types.h"
#include "vector3.h"

// The functions and globals of mapview.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void BeginMapView(MechScalar* p_pose, MechS32 p_slot, MechS32 p_worldSpan, MechS32 p_far);
	void DrawMapViewScene(MechU32 p_flags);
	void EndMapView(void);
	MechS32 CullMapViewShape(Shape* p_shape);
	ProjectedVertex* ProjectMapViewVertex(ProjectedVertex* p_vertex);
#ifdef MW2_MATROX
	MechS32 ProjectMapPoint(Vector3* p_position, Point* p_screen);
#else
MechS32 ProjectMapPoint(MapPoint* p_point);
#endif

#ifdef __cplusplus
}
#endif

#endif // MAPVIEW_H
