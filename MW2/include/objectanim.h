#ifndef OBJECTANIM_H
#define OBJECTANIM_H

#include "face.h"
#include "path.h"
#include "projectedvertex.h"
#include "quartzreel.h"
#include "resourceref.h"
#include "types.h"
#include "vertex.h"

// The functions and globals of objectanim.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_pathCount;
	extern MechS32 g_unk0x100a6d6c;
	extern MechS32 g_unk0x100a6d70;
	extern MechS32 g_unk0x100a6d74;
	extern MechS32 g_unk0x1010b530;
	extern MechU8 g_unk0x1010b53c;
	extern MechU8 g_unk0x1010b5b8;
	extern ProjectedVertex* g_unk0x1010b550[20];
	extern MechS32 g_unk0x1010b5b0;
	extern Path g_paths[0x40];
	extern QuartzReel0x14* g_unk0x101079e0[0x780];

	MechS32 FUN_10046750(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_100472fe(MechS32 p_mode, MechS32 p_value);
	MechS32 LoadAnimFile(ResourceRef* p_ref);
	MechS32 FUN_10047462(void);
	MechS32 FUN_10047477(void);
	MechS32 FUN_1004748c(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_1004771e(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_100479ec(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_10047d10(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_10047f60(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	ProjectedVertex* FUN_10048c50(Vertex* p_vertex);
	ProjectedVertex* FUN_10048d46(Vertex* p_a, Vertex* p_b);
	ProjectedVertex* FUN_10048ebe(ProjectedVertex* p_vertex);
	MechS32 FUN_10048faf(Face* p_face, Vertex* p_vertices);
	void FUN_10049155(Face* p_face, Vertex* p_vertices);

#ifdef __cplusplus
}
#endif

#endif // OBJECTANIM_H
