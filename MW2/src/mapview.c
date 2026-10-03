#include "mapview.h"

#include "animation.h"
#include "decomp.h"
#include "depthsort.h"
#include "eyepoint.h"
#include "fixeddot27.h"
#include "fixeddot29.h"
#include "objectanim.h"
#include "palette.h"
#include "polydraw.h"
#include "recordstacks.h"
#include "render.h"
#include "rendersettings.h"
#include "rendertarget.h"
#include "shapelists.h"
#include "shiftdiv.h"
#include "types.h"
#include "view.h"

// The map (satellite) view's projection: a top-down view of p_worldSpan units across.

// GLOBAL: MW2 0x10109ab0
MechS32 g_unk0x10109ab0;

// GLOBAL: MW2 0x10109ac0
Eyepoint g_unk0x10109ac0;

// GLOBAL: MW2 0x10109ba0
MechS32 g_unk0x10109ba0;

// GLOBAL: MW2 0x10109ba4
MechS32 g_unk0x10109ba4;

// GLOBAL: MW2 0x10109ba8
MechS32 g_unk0x10109ba8;

// GLOBAL: MW2 0x10109bac
MechS32 g_unk0x10109bac;

// GLOBAL: MW2 0x10109bb0
MechS32 g_unk0x10109bb0;

// GLOBAL: MW2 0x10109bb4
MechS32 g_unk0x10109bb4;

// GLOBAL: MW2 0x10109bb8
MechS32 g_unk0x10109bb8;

// GLOBAL: MW2 0x10109bc0
RenderSettings g_savedRenderSettings;

// Saves the eyepoint and the rendering settings, and sets up a view from p_pose (position, then
// rotation) of p_worldSpan units across pane p_slot, as far as p_far.
// FUNCTION: MW2 0x10041fa0
void FUN_10041fa0(MechS32* p_pose, MechS32 p_slot, MechS32 p_worldSpan, MechS32 p_far)
{
	g_unk0x10109bb8 = p_worldSpan / (g_panes[p_slot].m_x1 - g_panes[p_slot].m_x0 + 1);
	g_unk0x10109ba4 = -(g_unk0x10109ba0 = -(p_worldSpan / 2));
	g_unk0x10109bac = -(g_unk0x10109ba8 = (g_panes[p_slot].m_y1 - g_panes[p_slot].m_y0 + 1) * g_unk0x10109bb8 / 2);
	g_unk0x10109bb0 = 0;
	g_unk0x10109bb4 = p_far;
	g_unk0x10109ab0 = g_palettePending;
	g_unk0x10109ac0 = *g_eyepoint;
	g_eyepoint->m_unk0x0c = p_pose[3];
	g_eyepoint->m_unk0x10 = p_pose[4];
	g_eyepoint->m_unk0x14 = p_pose[5];
	g_eyepoint->m_unk0x00 = p_pose[0];
	g_eyepoint->m_unk0x04 = p_pose[1];
	g_eyepoint->m_unk0x08 = p_pose[2];
	SelectPane(p_slot);
	g_eyepoint->m_unk0x40 = g_unk0x10109bb4;
	g_savedRenderSettings = g_renderSettings;
	g_renderSettings.m_shapeFilter = FUN_10042206;
	g_renderSettings.m_projectVertex = FUN_100423b3;
	FUN_1004bc2e(g_eyepoint);
	g_eyepoint->m_unk0x9c = 0x2000;
	g_eyepoint->m_unk0xa4 = 3;
	g_eyepoint->m_unk0xa0 = g_eyepoint->m_pixelAspect >> 3;
	g_eyepoint->m_unk0xa6 = 3;
	FUN_1004bfe8(g_eyepoint);
	FUN_1004b980(g_eyepoint);
	g_unk0x100a2460 = 0;
}

// Draws the map view's scene: the terrain when bit 0 of p_flags is set, then the shapes.
// FUNCTION: MW2 0x1004215f
void FUN_1004215f(MechU32 p_flags)
{
	if (p_flags & 1) {
		FUN_1004320b(g_eyepoint);
	}

	FUN_100338bb(g_unk0x100ad5e8);
	FUN_10069591();
}

// Restores the eyepoint and the rendering settings FUN_10041fa0 saved.
// FUNCTION: MW2 0x10042195
void FUN_10042195(void)
{
	g_renderSettings = g_savedRenderSettings;
	*g_eyepoint = g_unk0x10109ac0;
	g_palettePending = g_unk0x10109ab0;
	FUN_10012e00();
	FUN_1004bc2e(g_eyepoint);
	FUN_1004bfe8(g_eyepoint);
	FUN_1004b980(g_eyepoint);
	g_unk0x100a2460 = 0;
}

// Culls a shape against the map view's frustum: 1 hidden, 4 in front of the near plane, 5 past
// the far plane, 6 and 7 outside the side planes, 0 visible. Keeps its depth in g_unk0x1010b5a4.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10042206
MechS32 FUN_10042206(Shape* p_shape)
{
	MechS32 y;
	MechS32 z;
	MechS32 radius;
	MechS32 dist;
	MechS32 dx;
	MechS32 side;
	MechS32 dy;
	MechS32 height;
	MechS32 dz;
	MechS32 depth;
	MechS32 x;

	if (p_shape->m_unk0x00 & 0x1000) {
		return 1;
	}

	x = p_shape->m_unk0x34;
	y = p_shape->m_unk0x38;
	z = p_shape->m_unk0x3c;
	radius = p_shape->m_unk0x40;
	dx = x - g_unk0x100ea8b8;
	dy = y - g_unk0x100ea8b4;
	dz = z - g_unk0x100ea8bc;
	depth = g_unk0x1010b5a4 = FixedDot29(dx, g_unk0x100ea8a8, dy, g_unk0x100ea8ac, dz, g_unk0x100ea8b0);
	if (depth + radius < g_unk0x100ea8d0) {
		return 4;
	}

	if (depth - radius > g_unk0x100ea860) {
		return 5;
	}

	side = FixedDot29(dx, g_unk0x100ea890, dy, g_unk0x100ea894, dz, g_unk0x100ea898);
	if (side > 0) {
		dist = side - g_unk0x10109ba4;
	}
	else {
		dist = g_unk0x10109ba0 - side;
	}

	if (dist > radius) {
		return 6;
	}

	height = FixedDot29(dx, g_unk0x100ea89c, dy, g_unk0x100ea8a0, dz, g_unk0x100ea8a4);
	if (height > 0) {
		dist = height - g_unk0x10109ba8;
	}
	else {
		dist = g_unk0x10109bac - height;
	}

	if (dist > radius) {
		return 7;
	}

	return 0;
}

// Projects a vertex onto the map view (FUN_10042740's scaling) once per frame, with its clip
// outcodes, and adds it to the polygon being built: FUN_10048ebe's map-view counterpart.
// Stack-slot permutation: outcode and y.
// FUNCTION: MW2 0x100423b3
ProjectedVertex* FUN_100423b3(ProjectedVertex* p_vertex)
{
	MechS32 x;
	MechU8 outcode;
	MechS32 y;

	outcode = 0;
	if (!p_vertex->m_projected) {
		x = p_vertex->m_x;
		y = p_vertex->m_y;
		x = FUN_10042740(x, g_unk0x10109bb8, g_unk0x100ea824, g_unk0x100ea834);
		y = g_unk0x100ea840 - g_unk0x100ea850 - FUN_10042740(y, g_unk0x10109bb8, g_unk0x100ea828, g_unk0x100ea858);
		if (x - g_unk0x100ea830 < 0) {
			outcode |= 1;
		}

		if (x - g_unk0x100ea84c > 0) {
			outcode |= 2;
		}

		if (y - g_unk0x100ea850 < 0) {
			outcode |= 4;
		}

		if (y - g_unk0x100ea840 > 0) {
			outcode |= 8;
		}

		p_vertex->m_screenX = x;
		p_vertex->m_screenY = y;
		p_vertex->m_outcode = outcode;
		p_vertex->m_projected = 1;
	}

	g_unk0x1010b53c |= outcode;
	g_unk0x1010b5b8 &= outcode;
	if (g_unk0x1010b5b0 >= 20) {
		g_unk0x1010b5ac = 0;
	}
	else {
		g_unk0x1010b550[g_unk0x1010b5b0] = p_vertex;
		g_unk0x1010b5b0++;
	}

	return p_vertex;
}

// Projects p_point onto the map view in place. Returns whether it is in front of the eye and
// on the screen.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004251e
MechS32 FUN_1004251e(MapPoint* p_point)
{
	MechS32 visible;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 dx;
	MechS32 dy;
	MechS32 dz;

	visible = FALSE;
	x = p_point->m_xy.m_x;
	y = p_point->m_xy.m_y;
	z = p_point->m_z;
	dx = x - g_unk0x100ea8b8;
	dy = y - g_unk0x100ea8b4;
	dz = z - g_unk0x100ea8bc;
	x = FixedDot27(dx, g_unk0x100ea864, dy, g_unk0x100ea868, dz, g_unk0x100ea86c);
	y = FixedDot27(dx, g_unk0x100ea870, dy, g_unk0x100ea874, dz, g_unk0x100ea878);
	z = FixedDot27(dx, g_unk0x100ea87c, dy, g_unk0x100ea880, dz, g_unk0x100ea884);
	p_point->m_xy.m_x = FUN_10042740(x, g_unk0x10109bb8, g_unk0x100ea824, g_unk0x100ea834);
	p_point->m_xy.m_y =
		g_unk0x100ea840 - g_unk0x100ea850 - FUN_10042740(y, g_unk0x10109bb8, g_unk0x100ea828, g_unk0x100ea858);
	p_point->m_z = z;
	if (z > 0) {
		if (p_point->m_xy.m_x >= g_unk0x100ea830 && p_point->m_xy.m_x <= g_unk0x100ea84c &&
			p_point->m_xy.m_y >= g_unk0x100ea850 && p_point->m_xy.m_y <= g_unk0x100ea840) {
			visible = TRUE;
		}
		else {
			visible = FALSE;
		}
	}

	return visible;
}
