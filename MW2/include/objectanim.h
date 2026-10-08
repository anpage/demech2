#ifndef OBJECTANIM_H
#define OBJECTANIM_H

#include "face.h"
#include "path.h"
#include "projectedvertex.h"
#include "reel.h"
#include "resourceref.h"
#include "types.h"
#include "vertex.h"

// The functions and globals of objectanim.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_pathCount;
	extern MechS32 g_maxAnimNumber;
	extern MechS32 g_animBase;
	extern MechS32 g_animFileCount;
	extern MechS32 g_directionalLight;
	extern MechU8 g_polygonOrCodes;
	extern MechU8 g_polygonAndCodes;
	extern ProjectedVertex* g_polygonPoints[20];
	extern MechS32 g_polygonPointCount;
	extern MechU8* g_polygonPointCursor;
	extern MechS32 g_facesTried;
	extern MechS32 g_facesFrontFacing;
	extern MechS32 g_verticesTransformed;
	extern MechS32 g_polygonsQueued;
	extern Path g_paths[0x40];
	extern Reel* g_reels[0x780];

	MechS32 ReelMotionTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 ScaleBySpeedLevel(MechS32 p_mode, MechS32 p_value);
	MechS32 LoadAnimFile(ResourceRef* p_ref);
	MechS32 GetAnimBase(void);
	MechS32 GetReelMotionSize(void);
	MechS32 ColorCycleTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 SpinTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 OrbitTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 AmbientSoundTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 PathTask(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	ProjectedVertex* GetViewVertex(Vertex* p_vertex);
	ProjectedVertex* ClipEdgeToNearPlane(Vertex* p_a, Vertex* p_b);
	ProjectedVertex* ProjectVertex(ProjectedVertex* p_vertex);
	MechS32 GetFaceShade(Face* p_face, Vertex* p_vertices);
	void QueueFace(Face* p_face, Vertex* p_vertices);
#ifdef MW2_MATROX
	ProjectedVertex* ClipProjectedEdge(ProjectedVertex* p_a, ProjectedVertex* p_b, MechFloat p_plane);
	MechS32 ProjectPolygon(Vertex* p_vertices, MechU32 p_count, MechS32 p_unk0x08);
#endif

#ifdef __cplusplus
}
#endif

#endif // OBJECTANIM_H
