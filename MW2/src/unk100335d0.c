/* Depth-sorted drawing: the shapes of a list or a scene tree queue their polygons in the draw
   buffer (FUN_1007d296) with their depths, the queue is sorted farthest first and drawn,
   clipping each polygon to the screen if a vertex lies too far outside it. */
#include "unk100335d0.h"

#include "coppervale.h"
#include "decomp.h"
#include "duskmoth.h"
#include "emberfern.h"
#include "eyepoint.h"
#include "fixedmul.h"
#include "ivorydelta.h"
#include "object.h"
#include "types.h"
#include "unk10034990.h"
#include "unk100349c0.h"
#include "unk100349f0.h"
#include "unk10039a30.h"
#include "unk1003a530.h"
#include "unk10042e00.h"
#include "unk10046750.h"
#include "unk1004b980.h"
#include "unk1007d120.h"

#include <string.h>

DECOMP_SIZE_ASSERT(AmberDune0x8, 0x8)
DECOMP_SIZE_ASSERT(CopperVale0x20, 0x20)
DECOMP_SIZE_ASSERT(IvoryDelta0xc, 0xc)

// The number of entries in the list being built.
// GLOBAL: MW2 0x100a54b0
MechS32 g_unk0x100a54b0 = 0;

// GLOBAL: MW2 0x100a54b4
MechS32 g_unk0x100a54b4 = 0;

// GLOBAL: MW2 0x100a54b8
MechS32 g_unk0x100a54b8 = 0;

// GLOBAL: MW2 0x1010b5a0
MechS32 g_unk0x1010b5a0;

// The depth the shapes are queued at.
// GLOBAL: MW2 0x1010b5a4
MechS32 g_unk0x1010b5a4;

// The list being built: g_unk0x100c269c or g_unk0x100c2280.
// GLOBAL: MW2 0x1010b5c4
AmberDune0x8* g_unk0x1010b5c4;

// The flags of the shape being queued.
// GLOBAL: MW2 0x1010b5c8
MechU32 g_unk0x1010b5c8;

// GLOBAL: MW2 0x1010b5cc
MechS32 g_unk0x1010b5cc;

// Selects the shape's level of detail for p_depth (the first model whose distance, scaled by the
// eyepoint, lies beyond it, or the last) and queues its faces. Returns 1 if it has no model.
// Stack-slot permutation: every local.
// FUNCTION: MW2 0x100335d0
MechS32 FUN_100335d0(ScarletOrchid0x4c* p_shape, MechS32 p_depth)
{
	GraniteLattice0x18* model;
	DuskMoth0x24* face;
	GraniteLattice0x18* cursor;
	EmberFern0x2c* vertices;
	EmberFern0x2c* vertex;
	MechS32 n;
	MechS32 i;
	GraniteLattice0x18* prev;
	ScarletOrchid0x4c* shape;

	model = NULL;
	prev = NULL;
	shape = p_shape;
	if (!shape) {
		return 1;
	}

	cursor = shape->m_unk0x1c;
	if (!cursor) {
		return 1;
	}

	if (!cursor->m_unk0x00) {
		model = cursor;
	}
	else {
		prev = cursor;
		while (cursor) {
			if (FixedMul16(g_eyepoint->m_unk0xb8, cursor->m_unk0x00) > p_depth) {
				model = prev;
				break;
			}

			prev = cursor;
			cursor = cursor->m_unk0x0c;
		}
	}

	if (!model) {
		model = prev;
	}

	if (!model) {
		return 1;
	}

	shape->m_unk0x20 = model;
	vertices = (EmberFern0x2c*) (model + 1);
	if (shape->m_unk0x48 != model->m_unk0x10) {
		FUN_1000188b(shape);
	}

	vertex = vertices;
	i = n = model->m_unk0x04;
	while (i--) {
		vertex->m_unk0x24 = 0;
		vertex->m_unk0x28 &= 0xfb;
		vertex++;
	}

	face = (DuskMoth0x24*) (model->m_unk0x08 + (MechU8*) model);
	i = n = model->m_unk0x06;
	while (i--) {
		FUN_10049155(face, vertices);
		face++;
		if (!g_unk0x1010b5ac) {
			break;
		}
	}

	return 0;
}

// Sorts the entries from p_first to p_last, farthest first (a quicksort).
// The p_last/p_first comparison loads its operands in the opposite order (one attempt at swapping
// them didn't flip it), and stack-slot permutation: the swaps' temporaries.
// FUNCTION: MW2 0x1003378e
void FUN_1003378e(AmberDune0x8* p_first, AmberDune0x8* p_last)
{
	MechS32 pivot;
	AmberDune0x8* lo;
	AmberDune0x8* hi;

	if (p_last <= p_first) {
		return;
	}

	lo = p_first;
	hi = p_last + 1;
	pivot = lo->m_depth;
	for (;;) {
		do {
			lo++;
		} while (lo->m_depth > pivot);

		do {
			hi--;
		} while (hi->m_depth < pivot);

		if (hi <= lo) {
			break;
		}

		{
			IvoryDelta0xc* poly = lo->m_poly;
			lo->m_poly = hi->m_poly;
			hi->m_poly = poly;
		}
		{
			MechS32 depth = lo->m_depth;
			lo->m_depth = hi->m_depth;
			hi->m_depth = depth;
		}
	}

	lo = p_first;
	{
		IvoryDelta0xc* poly = lo->m_poly;
		lo->m_poly = hi->m_poly;
		hi->m_poly = poly;
	}
	{
		MechS32 depth = lo->m_depth;
		lo->m_depth = hi->m_depth;
		hi->m_depth = depth;
	}

	if (hi - 1 > p_first) {
		FUN_1003378e(p_first, hi - 1);
	}

	if (hi + 1 < p_last) {
		FUN_1003378e(hi + 1, p_last);
	}
}

// Draws the shapes of the list p_root heads, farthest first. Shapes flagged 0x100 are queued
// whole and expanded after the sort (FUN_10033a06).
// FUNCTION: MW2 0x100338bb
void FUN_100338bb(ScarletOrchid0x4c* p_root)
{
	ScarletOrchid0x4c* shape;

	FUN_1007d220();
	if (!p_root || !p_root->m_unk0x08 || p_root->m_unk0x04 == p_root->m_unk0x08) {
		return;
	}

	g_unk0x100a54b0 = 0;
	g_unk0x100a54b4 = 0;
	g_unk0x1010b5ac = 1;
	g_unk0x1010b5c4 = g_unk0x100c269c;
	for (shape = p_root->m_unk0x08; shape; shape = shape->m_unk0x08) {
		g_unk0x1010b5cc++;
		if (!g_unk0x100a6cc8.m_unk0x58(shape)) {
			g_unk0x1010b5a0++;
			g_unk0x1010b5c8 = shape->m_unk0x00;
			if (g_unk0x1010b5c8 & 0x100) {
				shape->m_unk0x00 |= 0x8000;
				g_unk0x1010b5c4[g_unk0x100a54b0].m_poly = (IvoryDelta0xc*) shape;
				g_unk0x1010b5c4[g_unk0x100a54b0].m_depth = g_unk0x1010b5a4;
				g_unk0x100a54b0++;
			}
			else {
				FUN_100335d0(shape, g_unk0x1010b5a4);
				if (!g_unk0x1010b5ac || g_unk0x100a54b4 >= g_unk0x100a54b8) {
					break;
				}
			}
		}
	}

	FUN_10033a06();
}

// Sorts the queue FUN_100338bb built, moves its polygons to g_unk0x100c2280 in order, expanding
// each whole shape into its own sorted run of polygons, and draws them.
// The i/count and g_unk0x100a54b4/g_unk0x100a54b8 comparisons load their operands in the opposite
// order (reccmp scores it as an effective match).
// FUNCTION: MW2 0x10033a06
void FUN_10033a06(void)
{
	MechS32 count;
	MechS32 i;
	MechS32 start;

	start = 0;
	if (g_unk0x100a54b0 > 1) {
		FUN_1003378e(g_unk0x100c269c, &g_unk0x100c269c[g_unk0x100a54b0 - 1]);
	}

	count = g_unk0x100a54b0;
	g_unk0x100a54b0 = 0;
	g_unk0x1010b5c4 = g_unk0x100c2280;
	for (i = 0; i < count; i++) {
		if (!(g_unk0x100c269c[i].m_poly->m_count & 0x8000)) {
			memcpy(&g_unk0x100c2280[g_unk0x100a54b0], &g_unk0x100c269c[i], sizeof(AmberDune0x8));
			g_unk0x100a54b0++;
		}
		else if (g_unk0x1010b5ac) {
			start = g_unk0x100a54b0;
			FUN_100335d0((ScarletOrchid0x4c*) g_unk0x100c269c[i].m_poly, g_unk0x100c269c[i].m_depth);
			if (!g_unk0x1010b5ac) {
				break;
			}

			if (g_unk0x100a54b0 - start > 1) {
				FUN_1003378e(&g_unk0x100c2280[start], &g_unk0x100c2280[g_unk0x100a54b0 - 1]);
			}
		}

		if (g_unk0x100a54b4 >= g_unk0x100a54b8) {
			break;
		}
	}

	for (i = 0; i < g_unk0x100a54b0; i++) {
		FUN_10033d32(g_unk0x100c2280[i].m_poly);
	}
}

// Draws the shapes of the scene tree p_root, farthest first.
// FUNCTION: MW2 0x10033b9e
void FUN_10033b9e(AmberWillow0x7c* p_root)
{
	MechS32 i;

	FUN_1007d220();
	g_unk0x100a54b0 = 0;
	g_unk0x100a54b4 = 0;
	g_unk0x1010b5ac = 1;
	g_unk0x1010b5c4 = g_unk0x100c2280;
	FUN_10033c4b(p_root);
	if (g_unk0x100a54b0 > 1) {
		FUN_1003378e(g_unk0x100c2280, &g_unk0x100c2280[g_unk0x100a54b0 - 1]);
	}

	for (i = 0; i < g_unk0x100a54b0; i++) {
		FUN_10033d32(g_unk0x100c2280[i].m_poly);
	}
}

// Queues the polygons of p_object's shape and of its descendants' shapes.
// FUNCTION: MW2 0x10033c4b
void FUN_10033c4b(AmberWillow0x7c* p_object)
{
	AmberWillow0x7c* child;
	ScarletOrchid0x4c* shape;

	if (!p_object || !g_unk0x1010b5ac || g_unk0x100a54b4 >= g_unk0x100a54b8) {
		return;
	}

	shape = p_object->m_unk0x6c;
	if (shape) {
		g_unk0x1010b5cc++;
		if (!(shape->m_unk0x00 & 0x1000) && !g_unk0x100a6cc8.m_unk0x58(shape)) {
			g_unk0x1010b5a0++;
			FUN_100335d0(shape, g_unk0x1010b5a4);
		}
	}

	for (child = p_object->m_firstChild; child; child = child->m_nextSibling) {
		FUN_10033c4b(child);
	}
}

// FUNCTION: MW2 0x10033d0f
void FUN_10033d0f(ScarletOrchid0x4c* p_root, Eyepoint* p_eyepoint)
{
	FUN_1004b980(p_eyepoint);
	FUN_100338bb(p_root);
}

// Draws a queued polygon, clipping it to the screen first if a vertex lies far outside it.
// Stack-slot permutation: every local.
// FUNCTION: MW2 0x10033d32
void FUN_10033d32(IvoryDelta0xc* p_poly)
{
	CopperVale0x20* vertex;
	MechS32 count;
	CopperVale0x20** vertices;
	MechU32 points[10 * 6];
	MechU32* point;
	MechS32 i;
	MechS32 x;
	MechS32 y;

	count = p_poly->m_count;
	vertices = (CopperVale0x20**) (p_poly + 1);
	point = points;
	i = count;
	while (i--) {
		vertex = *vertices;
		vertices++;
		x = vertex->m_unk0x0c;
		y = vertex->m_unk0x10;
		if (x <= -0x4000 || x >= 0x3fff || y <= -0x4000 || y >= 0x3fff) {
			count = FUN_10033e92(p_poly, points);
			break;
		}

		point[0] = x;
		point[1] = y;
		point[3] = vertex->m_unk0x14;
		point[4] = vertex->m_unk0x18;
		point[2] = 0;
		point[5] = vertex->m_unk0x08;
		point += 6;
	}

	if (count > 0) {
		FUN_100445d2(count, points, p_poly->m_unk0x02);
	}
}

// Clips a queued polygon to the screen (right, left, top and bottom in turn) and stores the
// result in p_points, six dwords per vertex. Returns the number of vertices.
// Stack-slot permutation: every local.
// FUNCTION: MW2 0x10033e92
MechS32 FUN_10033e92(IvoryDelta0xc* p_poly, MechU32* p_points)
{
	CopperVale0x20* cur;
	MechS32 bottom;
	MechS32 right;
	CopperVale0x20* out;
	CopperVale0x20* prev;
	MechS32 top;
	CopperVale0x20* src;
	MechS32 n;
	CopperVale0x20** vertices;
	CopperVale0x20* in;
	CopperVale0x20 bufferA[20];
	CopperVale0x20 clip;
	CopperVale0x20 bufferB[20];
	MechS32 i;
	MechS32 left;
	MechS32 count;

	right = bottom = 0x3fff;
	left = top = -0x4000;
	out = bufferA;
	count = p_poly->m_count;
	vertices = (CopperVale0x20**) (p_poly + 1);
	i = count;
	while (i--) {
		src = *vertices;
		vertices++;
		*out = *src;
		out++;
	}

	n = 0;
	in = bufferA;
	out = bufferB;
	prev = &in[count - 1];
	for (i = 0; i < count; i++) {
		cur = &in[i];
		if (cur->m_unk0x0c <= right) {
			if (prev->m_unk0x0c <= right) {
				*out = *cur;
				out++;
				n++;
			}
			else {
				FUN_10034571(prev, cur, right, &clip);
				*out = clip;
				out++;
				n++;
				*out = *cur;
				out++;
				n++;
			}
		}
		else {
			if (prev->m_unk0x0c <= right) {
				FUN_10034571(prev, cur, right, &clip);
				*out = clip;
				out++;
				n++;
			}
		}

		prev = cur;
	}

	count = n;
	n = 0;
	in = bufferB;
	out = bufferA;
	prev = &in[count - 1];
	for (i = 0; i < count; i++) {
		cur = &in[i];
		if (cur->m_unk0x0c >= left) {
			if (prev->m_unk0x0c >= left) {
				*out = *cur;
				out++;
				n++;
			}
			else {
				FUN_10034571(prev, cur, left, &clip);
				*out = clip;
				out++;
				n++;
				*out = *cur;
				out++;
				n++;
			}
		}
		else {
			if (prev->m_unk0x0c >= left) {
				FUN_10034571(prev, cur, left, &clip);
				*out = clip;
				out++;
				n++;
			}
		}

		prev = cur;
	}

	count = n;
	n = 0;
	in = bufferA;
	out = bufferB;
	prev = &in[count - 1];
	for (i = 0; i < count; i++) {
		cur = &in[i];
		if (cur->m_unk0x10 >= top) {
			if (prev->m_unk0x10 >= top) {
				*out = *cur;
				out++;
				n++;
			}
			else {
				FUN_10034779(prev, cur, top, &clip);
				*out = clip;
				out++;
				n++;
				*out = *cur;
				out++;
				n++;
			}
		}
		else {
			if (prev->m_unk0x10 >= top) {
				FUN_10034779(prev, cur, top, &clip);
				*out = clip;
				out++;
				n++;
			}
		}

		prev = cur;
	}

	count = n;
	n = 0;
	in = bufferB;
	out = bufferA;
	prev = &in[count - 1];
	for (i = 0; i < count; i++) {
		cur = &in[i];
		if (cur->m_unk0x10 <= bottom) {
			if (prev->m_unk0x10 <= bottom) {
				*out = *cur;
				out++;
				n++;
			}
			else {
				FUN_10034779(prev, cur, bottom, &clip);
				*out = clip;
				out++;
				n++;
				*out = *cur;
				out++;
				n++;
			}
		}
		else {
			if (prev->m_unk0x10 <= bottom) {
				FUN_10034779(prev, cur, bottom, &clip);
				*out = clip;
				out++;
				n++;
			}
		}

		prev = cur;
	}

	for (cur = bufferA, i = n; i--; cur++, p_points += 6) {
		p_points[0] = cur->m_unk0x0c;
		p_points[1] = cur->m_unk0x10;
		p_points[3] = cur->m_unk0x14;
		p_points[4] = cur->m_unk0x18;
		p_points[2] = 0;
		p_points[5] = cur->m_unk0x08;
	}

	return n;
}

// Returns the depth where the edge from (p_a0, p_z0) to (p_a1, p_z1), in view space, crosses
// the screen column (p_isX) or row p_edge.
// Stack-slot permutation: da, dz, t and z.
// FUNCTION: MW2 0x10034499
MechS32 FUN_10034499(MechS32 p_a0, MechS32 p_a1, MechS32 p_edge, MechS32 p_isX, MechS32 p_z0, MechS32 p_z1)
{
	MechS32 dz;
	MechS32 t;
	MechS32 z;
	MechS32 da;

	da = p_a1 - p_a0;
	dz = p_z1 - p_z0;
	if (dz < 0) {
		da = -da;
		dz = -dz;
		p_a0 = p_a1;
		p_z0 = p_z1;
	}

	if (p_isX) {
		p_edge = FUN_10034990(p_edge, g_unk0x100ea834, dz, g_unk0x100ea824);
	}
	else {
		p_edge = FUN_10034990(g_unk0x100ea858, p_edge, dz, g_unk0x100ea828);
	}

	t = p_edge - da;
	if (!t) {
		z = p_z0;
	}
	else {
		z = FUN_100349c0(p_a0, p_z0, da, dz, t);
	}

	return z;
}

// Stores in p_out the point where the edge from p_a to p_b crosses the screen column p_x.
// FUNCTION: MW2 0x10034571
void FUN_10034571(CopperVale0x20* p_a, CopperVale0x20* p_b, MechS32 p_x, CopperVale0x20* p_out)
{
	MechS32 z;
	MechS32 dx;
	MechS32 dy;

	p_out->m_unk0x08 = FUN_10034499(p_a->m_unk0x00, p_b->m_unk0x00, p_x, 1, p_a->m_unk0x08, p_b->m_unk0x08);
	z = p_out->m_unk0x08;
	p_out->m_unk0x0c = p_x;
	p_out->m_unk0x10 = FUN_100349f0(p_a->m_unk0x0c, p_b->m_unk0x0c, p_x, p_a->m_unk0x10, p_b->m_unk0x10);
	p_out->m_unk0x00 = FUN_10034990(p_x, g_unk0x100ea834, z, g_unk0x100ea824);
	p_out->m_unk0x04 = FUN_10034990(g_unk0x100ea858, p_out->m_unk0x10, z, g_unk0x100ea828);

	dx = p_a->m_unk0x00 - p_b->m_unk0x00;
	if (dx < 0) {
		dx = -dx;
	}

	dy = p_a->m_unk0x04 - p_b->m_unk0x04;
	if (dy < 0) {
		dy = -dy;
	}

	if (dx > dy) {
		p_out->m_unk0x14 =
			FUN_100349f0(p_a->m_unk0x00, p_b->m_unk0x00, p_out->m_unk0x00, p_a->m_unk0x14, p_b->m_unk0x14);
		p_out->m_unk0x18 =
			FUN_100349f0(p_a->m_unk0x00, p_b->m_unk0x00, p_out->m_unk0x00, p_a->m_unk0x18, p_b->m_unk0x18);
	}
	else if (dy) {
		p_out->m_unk0x14 =
			FUN_100349f0(p_a->m_unk0x04, p_b->m_unk0x04, p_out->m_unk0x04, p_a->m_unk0x14, p_b->m_unk0x14);
		p_out->m_unk0x18 =
			FUN_100349f0(p_a->m_unk0x04, p_b->m_unk0x04, p_out->m_unk0x04, p_a->m_unk0x18, p_b->m_unk0x18);
	}
	else {
		p_out->m_unk0x14 = (p_a->m_unk0x14 + p_b->m_unk0x14) >> 1;
		p_out->m_unk0x18 = (p_a->m_unk0x18 + p_b->m_unk0x18) >> 1;
	}
}

// Stores in p_out the point where the edge from p_a to p_b crosses the screen row p_y.
// FUNCTION: MW2 0x10034779
void FUN_10034779(CopperVale0x20* p_a, CopperVale0x20* p_b, MechS32 p_y, CopperVale0x20* p_out)
{
	MechS32 z;
	MechS32 dx;
	MechS32 dy;

	p_out->m_unk0x08 = FUN_10034499(p_a->m_unk0x04, p_b->m_unk0x04, p_y, 0, p_a->m_unk0x08, p_b->m_unk0x08);
	z = p_out->m_unk0x08;
	p_out->m_unk0x0c = FUN_100349f0(p_a->m_unk0x10, p_b->m_unk0x10, p_y, p_a->m_unk0x0c, p_b->m_unk0x0c);
	p_out->m_unk0x10 = p_y;
	p_out->m_unk0x00 = FUN_10034990(p_out->m_unk0x0c, g_unk0x100ea834, z, g_unk0x100ea824);
	p_out->m_unk0x04 = FUN_10034990(g_unk0x100ea858, p_y, z, g_unk0x100ea828);

	dx = p_a->m_unk0x00 - p_b->m_unk0x00;
	if (dx < 0) {
		dx = -dx;
	}

	dy = p_a->m_unk0x04 - p_b->m_unk0x04;
	if (dy < 0) {
		dy = -dy;
	}

	if (dy > dx) {
		p_out->m_unk0x14 =
			FUN_100349f0(p_a->m_unk0x04, p_b->m_unk0x04, p_out->m_unk0x04, p_a->m_unk0x14, p_b->m_unk0x14);
		p_out->m_unk0x18 =
			FUN_100349f0(p_a->m_unk0x04, p_b->m_unk0x04, p_out->m_unk0x04, p_a->m_unk0x18, p_b->m_unk0x18);
	}
	else if (dx) {
		p_out->m_unk0x14 =
			FUN_100349f0(p_a->m_unk0x00, p_b->m_unk0x00, p_out->m_unk0x00, p_a->m_unk0x14, p_b->m_unk0x14);
		p_out->m_unk0x18 =
			FUN_100349f0(p_a->m_unk0x00, p_b->m_unk0x00, p_out->m_unk0x00, p_a->m_unk0x18, p_b->m_unk0x18);
	}
	else {
		p_out->m_unk0x14 = (p_a->m_unk0x14 + p_b->m_unk0x14) >> 1;
		p_out->m_unk0x18 = (p_a->m_unk0x18 + p_b->m_unk0x18) >> 1;
	}
}
