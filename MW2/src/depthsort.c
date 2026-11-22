/* Depth-sorted drawing: the shapes of a list or a scene tree queue their polygons in the draw
   buffer (AllocQueuedPolygon) with their depths, the queue is sorted farthest first and drawn,
   clipping each polygon to the screen if a vertex lies too far outside it.
   The Matrox edition's object is optimized code (VC++ 4.0 /Ox /Oa /Op, vc40/targets.cmake), in
   another order (DrawQueuedPolygon after QueueShapeLod, QueueObjTree before DrawObjTreeShapes,
   which is in drawobjtreeshapes.c for that reason), and without the screen clipping, which its
   renderer does: its DrawQueuedPolygon is its own. */
#include "depthsort.h"

#include "crossdiv.h"
#include "decomp.h"
#include "eyepoint.h"
#include "face.h"
#include "fixedmul.h"
#include "lerp.h"
#include "object.h"
#include "objectanim.h"
#include "polydraw.h"
#include "projectedvertex.h"
#include "queuedpolygon.h"
#include "recordstacks.h"
#include "scaledelta.h"
#include "shape.h"
#include "shapegeom.h"
#include "types.h"
#include "vertex.h"
#include "view.h"

#ifdef MW2_VFX16
#include "accel/vfx16.h"
#include "palette.h"
#endif
#ifdef MW2_A3D
#include "render.h"
#endif

#include <string.h>

#ifdef MW2_DEPTH_ENTRY_TAG
DECOMP_SIZE_ASSERT(DepthEntry, 0xc)
#else
DECOMP_SIZE_ASSERT(DepthEntry, 0x8)
#endif
#ifdef MW2_A3D
DECOMP_SIZE_ASSERT(ProjectedVertex, 0x3c)
DECOMP_SIZE_ASSERT(QueuedPolygon, 0x10)
#else
DECOMP_SIZE_ASSERT(ProjectedVertex, 0x20)
DECOMP_SIZE_ASSERT(QueuedPolygon, 0xc)
#endif

// The number of entries in the list being built.
// GLOBAL: MW2 0x100a54b0
// GLOBAL: MW2GBL 0x100adaf0
// GLOBAL: MW2MATROX 0x100a5540
// GLOBAL: MW23DFX 0x1024e404
// GLOBAL: MW2ATI 0x10182714
// GLOBAL: MW2S3 0x10065768
MechS32 g_depthEntryCount = 0;

// GLOBAL: MW2 0x100a54b4
// GLOBAL: MW2MATROX 0x100a5544
// GLOBAL: MW23DFX 0x1024e408
// GLOBAL: MW2ATI 0x10182718
// GLOBAL: MW2S3 0x1006576c
MechS32 g_polygonCount = 0;

// GLOBAL: MW2 0x100a54b8
// GLOBAL: MW2MATROX 0x100a5548
// GLOBAL: MW23DFX 0x1024e40c
// GLOBAL: MW2ATI 0x1018271c
// GLOBAL: MW2S3 0x10065770
MechS32 g_maxPolygons = 0;

// GLOBAL: MW2 0x1010b5a0
// GLOBAL: MW2GBL 0x100e9a50
// GLOBAL: MW2MATROX 0x101d696c
// GLOBAL: MW23DFX 0x101b0ecc
// GLOBAL: MW2ATI 0x100e4eac
// GLOBAL: MW2S3 0x100b1d60
MechS32 g_shapesDrawn;

// The depth the shapes are queued at.
// GLOBAL: MW2 0x1010b5a4
// GLOBAL: MW2GBL 0x100e9a54
// GLOBAL: MW2MATROX 0x101d6964
// GLOBAL: MW23DFX 0x101b0ec4
// GLOBAL: MW2ATI 0x100e4ea4
// GLOBAL: MW2S3 0x100b1d64
MechScalar g_queueDepth;

// The list being built: g_depthQueue or g_drawList.
// GLOBAL: MW2 0x1010b5c4
// GLOBAL: MW2GBL 0x100e9a70
// GLOBAL: MW2MATROX 0x101d6980
// GLOBAL: MW23DFX 0x101b0ee0
// GLOBAL: MW2ATI 0x100e4ec0
// GLOBAL: MW2S3 0x100b1d80
DepthEntry* g_depthList;

// The flags of the shape being queued.
// GLOBAL: MW2 0x1010b5c8
// GLOBAL: MW2GBL 0x100e9a74
// GLOBAL: MW2MATROX 0x101d697c
// GLOBAL: MW23DFX 0x101b0edc
// GLOBAL: MW2ATI 0x100e4ebc
// GLOBAL: MW2S3 0x100b1d84
MechU32 g_queuedShapeFlags;

// GLOBAL: MW2 0x1010b5cc
// GLOBAL: MW2GBL 0x100e9a78
// GLOBAL: MW2MATROX 0x101d6970
// GLOBAL: MW23DFX 0x101b0ed0
// GLOBAL: MW2ATI 0x100e4eb0
// GLOBAL: MW2S3 0x100b1d88
MechS32 g_shapesConsidered;

// Selects the shape's level of detail for p_depth (the first model whose distance, scaled by the
// eyepoint, lies beyond it, or the last) and queues its faces. Returns 1 if it has no model.
// FUNCTION: MW2 0x100335d0
// FUNCTION: MW2GBL 0x10039280
// FUNCTION: MW2MATROX 0x100232f0
// FUNCTION: MW23DFX 0x10029f00
// FUNCTION: MW2ATI 0x10029b30
// FUNCTION: MW2S3 0x10021880
MechS32 QueueShapeLod(Shape* p_shape, MechScalar p_depth)
{
	Model* model;
	Face* face;
	Model* cursor;
	Vertex* vertices;
	Vertex* vertex;
	MechS32 n;
	MechS32 i;
	Model* prev;
	Shape* shape;

	model = NULL;
	prev = NULL;
	shape = p_shape;
	if (!shape) {
		return 1;
	}

	cursor = shape->m_models;
	if (!cursor) {
		return 1;
	}

	if (!cursor->m_key) {
		model = cursor;
	}
	else {
		prev = cursor;
		while (cursor) {
#ifdef MW2_FLOAT_SCALARS
			if (cursor->m_key * g_eyepoint->m_detailScale > p_depth) {
#else
			if (FixedMul16(g_eyepoint->m_detailScale, cursor->m_key) > p_depth) {
#endif
				model = prev;
				break;
			}

			prev = cursor;
			cursor = cursor->m_next;
		}
	}

	if (!model) {
		model = prev;
	}

	if (!model) {
		return 1;
	}

	shape->m_model = model;
	vertices = (Vertex*) (model + 1);
	if (shape->m_transformCount != model->m_transformCount) {
		TransformShapeModel(shape);
	}

	vertex = vertices;
	i = n = model->m_vertexCount;
	while (i--) {
		vertex->m_projection = 0;
		vertex->m_flags &= 0xfb;
		vertex++;
	}

	face = (Face*) (model->m_faceOffset + (MechU8*) model);
	i = n = model->m_faceCount;
	while (i--) {
		QueueFace(face, vertices);
		face++;
		if (!g_queueHasRoom) {
			break;
		}
	}

	return 0;
}

#ifdef MW2_OUTLINE_FLUSH
// animation.c's: flushes the renderer.
void FUN_10069591(void);
#endif

#ifdef MW2_A3D
// The Matrox edition's draws a queued polygon itself: a pixel or a line (when g_renderSettings
// draws those), else through m_drawPolygon, first filled and outlined in wireframe mode 1.
// FUNCTION: MW2MATROX 0x100233e0
void DrawQueuedPolygon(QueuedPolygon* p_poly)
{
	MechU16 flags;
	ProjectedVertex** vertices;
	MechS32 count;

	flags = p_poly->m_flags;
	vertices = (ProjectedVertex**) (p_poly + 1);
	count = p_poly->m_count;
	if (count == 1 && g_renderSettings.m_drawPixels) {
		if (flags != 0x1000) {
			VFX_pixel_write16(
				&g_currentPane,
				vertices[0]->m_screenX,
				vertices[0]->m_screenY,
				VFX_lookaside_read16(flags)
			);
		}
	}
	else if (count == 2 && g_renderSettings.m_drawLines) {
		VFX_line_draw16(
			&g_currentPane,
			vertices[0]->m_screenX,
			vertices[0]->m_screenY,
			vertices[1]->m_screenX,
			vertices[1]->m_screenY,
			0,
			VFX_lookaside_read16(flags)
		);
	}
	else if (!g_renderSettings.m_wireframe) {
#ifdef MW2_QUEUED_POLYGON_EDGES_0XFF
		g_renderSettings.m_drawPolygon(count, vertices, flags, 0xff);
#else
		g_renderSettings.m_drawPolygon(count, vertices, flags, 0xf);
#endif
	}
	else if (g_renderSettings.m_wireframe == 1) {
		g_renderSettings.m_drawPolygon(count, vertices, 0, 0xf);
#if defined(MW2_OUTLINE_FLUSH)
		// The S3 edition's flushes the renderer (FUN_10069591) between the fill and the outline.
		FUN_10069591();
		g_renderSettings.m_drawPolygon(count, vertices, (flags & ~0xd000) | 0x2000, p_poly->m_outcodes);
#elif defined(MW2_OUTLINE_CLEARS_FLAGS)
		// The ATI, S3 and 3Dfx editions' outline also clears bits 12, 14 and 15 of the flags.
		g_renderSettings.m_drawPolygon(count, vertices, (flags & ~0xd000) | 0x2000, p_poly->m_outcodes);
#else
		g_renderSettings.m_drawPolygon(count, vertices, flags | 0x2000, p_poly->m_outcodes);
#endif
	}
	else {
		g_renderSettings.m_drawPolygon(count, vertices, flags | 0x2000, p_poly->m_outcodes);
	}
}
#endif

// Sorts the entries from p_first to p_last, farthest first (a quicksort).
// FUNCTION: MW2 0x1003378e
// FUNCTION: MW2GBL 0x1003943e
// FUNCTION: MW2MATROX 0x10023500
// FUNCTION: MW23DFX 0x10029ff0
// FUNCTION: MW2ATI 0x10029c20
// FUNCTION: MW2S3 0x10021970
void SortDepthEntries(DepthEntry* p_first, DepthEntry* p_last)
{
	MechScalar pivot;
	DepthEntry* lo;
	DepthEntry* hi;

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
			QueuedPolygon* poly = lo->m_poly;
			lo->m_poly = hi->m_poly;
			hi->m_poly = poly;
		}
		{
			MechScalar depth = lo->m_depth;
			lo->m_depth = hi->m_depth;
			hi->m_depth = depth;
		}
	}

	lo = p_first;
	{
		QueuedPolygon* pivotPoly = lo->m_poly;
		lo->m_poly = hi->m_poly;
		hi->m_poly = pivotPoly;
	}
	{
		MechScalar pivotDepth = lo->m_depth;
		lo->m_depth = hi->m_depth;
		hi->m_depth = pivotDepth;
	}

	if (hi - 1 > p_first) {
		SortDepthEntries(p_first, hi - 1);
	}

	if (hi + 1 < p_last) {
		SortDepthEntries(hi + 1, p_last);
	}
}

// Draws the shapes of the list p_root heads, farthest first. Shapes flagged 0x100 are queued
// whole and expanded after the sort (DrawDepthQueue).
// FUNCTION: MW2 0x100338bb
// FUNCTION: MW2GBL 0x1003956b
// FUNCTION: MW2MATROX 0x100235a0
// FUNCTION: MW23DFX 0x1002a080
// FUNCTION: MW2ATI 0x10029cb0
// FUNCTION: MW2S3 0x10021a00
void DrawShapeList(Shape* p_root)
{
	Shape* shape;

	ResetDrawBuffer();
	if (!p_root || !p_root->m_next || p_root->m_prev == p_root->m_next) {
		return;
	}

	g_depthEntryCount = 0;
	g_polygonCount = 0;
	g_queueHasRoom = 1;
	g_depthList = g_depthQueue;
	for (shape = p_root->m_next; shape; shape = shape->m_next) {
		g_shapesConsidered++;
		if (!g_renderSettings.m_shapeFilter(shape)) {
			g_shapesDrawn++;
			g_queuedShapeFlags = shape->m_flags;
			if (g_queuedShapeFlags & 0x100) {
				shape->m_flags |= 0x8000;
				g_depthList[g_depthEntryCount].m_poly = (QueuedPolygon*) shape;
				g_depthList[g_depthEntryCount].m_depth = g_queueDepth;
				g_depthEntryCount++;
			}
			else {
				QueueShapeLod(shape, g_queueDepth);
				if (!g_queueHasRoom || g_polygonCount >= g_maxPolygons) {
					break;
				}
			}
		}
	}

	DrawDepthQueue();
}

// Sorts the queue DrawShapeList built, moves its polygons to g_drawList in order, expanding
// each whole shape into its own sorted run of polygons, and draws them.
// FUNCTION: MW2 0x10033a06
// FUNCTION: MW2MATROX 0x100236a0
// FUNCTION: MW23DFX 0x1002a170
// FUNCTION: MW2ATI 0x10029da0
// FUNCTION: MW2S3 0x10021af0
void DrawDepthQueue(void)
{
	MechS32 i;
	MechS32 start;
	MechS32 count;

	start = 0;
	if (g_depthEntryCount > 1) {
		SortDepthEntries(g_depthQueue, &g_depthQueue[g_depthEntryCount - 1]);
	}

	count = g_depthEntryCount;
	g_depthEntryCount = 0;
	g_depthList = g_drawList;
	for (i = 0; i < count; i++) {
		if (!(g_depthQueue[i].m_poly->m_count & 0x8000)) {
			memcpy(&g_drawList[g_depthEntryCount], &g_depthQueue[i], sizeof(DepthEntry));
			g_depthEntryCount++;
		}
		else if (g_queueHasRoom) {
			start = g_depthEntryCount;
			QueueShapeLod((Shape*) g_depthQueue[i].m_poly, g_depthQueue[i].m_depth);
			if (!g_queueHasRoom) {
				break;
			}

			if (g_depthEntryCount - start > 1) {
				SortDepthEntries(&g_drawList[start], &g_drawList[g_depthEntryCount - 1]);
			}
		}

		if (g_polygonCount >= g_maxPolygons) {
			break;
		}
	}

	for (i = 0; i < g_depthEntryCount; i++) {
		DrawQueuedPolygon(g_drawList[i].m_poly);
	}
}

#ifndef MW2_ACCEL
#include "drawobjtreeshapes.c"
#endif

// Queues the polygons of p_object's shape and of its descendants' shapes.
// FUNCTION: MW2 0x10033c4b
// FUNCTION: MW2MATROX 0x100237e0
// FUNCTION: MW23DFX 0x1002a300
// FUNCTION: MW2ATI 0x10029f30
// FUNCTION: MW2S3 0x10021c80
void QueueObjTree(SceneObject* p_object)
{
	SceneObject* child;
	Shape* shape;

	if (!p_object || !g_queueHasRoom || g_polygonCount >= g_maxPolygons) {
		return;
	}

	shape = p_object->m_shape;
	if (shape) {
		g_shapesConsidered++;
		if (!(shape->m_flags & 0x1000) && !g_renderSettings.m_shapeFilter(shape)) {
			g_shapesDrawn++;
			QueueShapeLod(shape, g_queueDepth);
		}
	}

	for (child = p_object->m_firstChild; child; child = child->m_nextSibling) {
		QueueObjTree(child);
	}
}

#ifdef MW2_ACCEL
#include "drawobjtreeshapes.c"
#endif

// FUNCTION: MW2 0x10033d0f
// FUNCTION: MW2GBL 0x100399d9
// FUNCTION: MW2MATROX 0x100238f0
// FUNCTION: MW23DFX 0x1002a370
void DrawShapeListFrom(Shape* p_root, Eyepoint* p_eyepoint)
{
	SelectEyepoint(p_eyepoint);
	DrawShapeList(p_root);
}

#ifndef MW2_A3D
// Draws a queued polygon, clipping it to the screen first if a vertex lies far outside it.
// FUNCTION: MW2 0x10033d32
// FUNCTION: MW2GBL 0x100399fc
void DrawQueuedPolygon(QueuedPolygon* p_poly)
{
	ProjectedVertex* vertex;
	MechS32 count;
	ProjectedVertex** vertices;
	MechU32 points[10 * 6];
	MechU32* point;
	MechS32 i;
	MechS32 x;
	MechS32 y;

	count = p_poly->m_count;
	vertices = (ProjectedVertex**) (p_poly + 1);
	point = points;
	i = count;
	while (i--) {
		vertex = *vertices;
		vertices++;
		x = vertex->m_screenX;
		y = vertex->m_screenY;
		if (x <= -0x4000 || x >= 0x3fff || y <= -0x4000 || y >= 0x3fff) {
			count = ClipPolygonToScreen(p_poly, points);
			break;
		}

		point[0] = x;
		point[1] = y;
		point[3] = vertex->m_u;
		point[4] = vertex->m_v;
		point[2] = 0;
		point[5] = vertex->m_z;
		point += 6;
	}

	if (count > 0) {
		DrawPolygonOrLine(count, points, p_poly->m_flags);
	}
}

// Clips a queued polygon to the screen (right, left, top and bottom in turn) and stores the
// result in p_points, six dwords per vertex. Returns the number of vertices.
// FUNCTION: MW2 0x10033e92
MechS32 ClipPolygonToScreen(QueuedPolygon* p_poly, MechU32* p_points)
{
	ProjectedVertex* cur;
	MechS32 bottom;
	MechS32 right;
	ProjectedVertex* out;
	ProjectedVertex* prev;
	MechS32 top;
	ProjectedVertex* src;
	MechS32 n;
	ProjectedVertex** vertices;
	ProjectedVertex* in;
	ProjectedVertex bufferA[20];
	ProjectedVertex clip;
	ProjectedVertex bufferB[20];
	MechS32 i;
	MechS32 left;
	MechS32 count;

	right = bottom = 0x3fff;
	left = top = -0x4000;
	out = bufferA;
	count = p_poly->m_count;
	vertices = (ProjectedVertex**) (p_poly + 1);
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
		if (cur->m_screenX <= right) {
			if (prev->m_screenX <= right) {
				*out = *cur;
				out++;
				n++;
			}
			else {
				ClipEdgeToColumn(prev, cur, right, &clip);
				*out = clip;
				out++;
				n++;
				*out = *cur;
				out++;
				n++;
			}
		}
		else {
			if (prev->m_screenX <= right) {
				ClipEdgeToColumn(prev, cur, right, &clip);
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
		if (cur->m_screenX >= left) {
			if (prev->m_screenX >= left) {
				*out = *cur;
				out++;
				n++;
			}
			else {
				ClipEdgeToColumn(prev, cur, left, &clip);
				*out = clip;
				out++;
				n++;
				*out = *cur;
				out++;
				n++;
			}
		}
		else {
			if (prev->m_screenX >= left) {
				ClipEdgeToColumn(prev, cur, left, &clip);
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
		if (cur->m_screenY >= top) {
			if (prev->m_screenY >= top) {
				*out = *cur;
				out++;
				n++;
			}
			else {
				ClipEdgeToRow(prev, cur, top, &clip);
				*out = clip;
				out++;
				n++;
				*out = *cur;
				out++;
				n++;
			}
		}
		else {
			if (prev->m_screenY >= top) {
				ClipEdgeToRow(prev, cur, top, &clip);
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
		if (cur->m_screenY <= bottom) {
			if (prev->m_screenY <= bottom) {
				*out = *cur;
				out++;
				n++;
			}
			else {
				ClipEdgeToRow(prev, cur, bottom, &clip);
				*out = clip;
				out++;
				n++;
				*out = *cur;
				out++;
				n++;
			}
		}
		else {
			if (prev->m_screenY <= bottom) {
				ClipEdgeToRow(prev, cur, bottom, &clip);
				*out = clip;
				out++;
				n++;
			}
		}

		prev = cur;
	}

	for (cur = bufferA, i = n; i--; cur++, p_points += 6) {
		p_points[0] = cur->m_screenX;
		p_points[1] = cur->m_screenY;
		p_points[3] = cur->m_u;
		p_points[4] = cur->m_v;
		p_points[2] = 0;
		p_points[5] = cur->m_z;
	}

	return n;
}

// Returns the depth where the edge from (p_a0, p_z0) to (p_a1, p_z1), in view space, crosses
// the screen column (p_isX) or row p_edge.
// FUNCTION: MW2 0x10034499
// FUNCTION: MW2GBL 0x1003a284
MechS32 GetEdgeCrossingDepth(MechS32 p_a0, MechS32 p_a1, MechS32 p_edge, MechS32 p_isX, MechS32 p_z0, MechS32 p_z1)
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
		p_edge = UnprojectCoordinate(p_edge, g_viewCenterX, dz, g_viewShiftX);
	}
	else {
		p_edge = UnprojectCoordinate(g_viewCenterY, p_edge, dz, g_viewShiftY);
	}

	t = p_edge - da;
	if (!t) {
		z = p_z0;
	}
	else {
		z = CrossDiv(p_a0, p_z0, da, dz, t);
	}

	return z;
}

// Stores in p_out the point where the edge from p_a to p_b crosses the screen column p_x.
// FUNCTION: MW2 0x10034571
// FUNCTION: MW2GBL 0x1003a35c
void ClipEdgeToColumn(ProjectedVertex* p_a, ProjectedVertex* p_b, MechS32 p_x, ProjectedVertex* p_out)
{
	MechS32 z;
	MechS32 dx;
	MechS32 dy;

	p_out->m_z = GetEdgeCrossingDepth(p_a->m_x, p_b->m_x, p_x, 1, p_a->m_z, p_b->m_z);
	z = p_out->m_z;
	p_out->m_screenX = p_x;
	p_out->m_screenY = Lerp(p_a->m_screenX, p_b->m_screenX, p_x, p_a->m_screenY, p_b->m_screenY);
	p_out->m_x = UnprojectCoordinate(p_x, g_viewCenterX, z, g_viewShiftX);
	p_out->m_y = UnprojectCoordinate(g_viewCenterY, p_out->m_screenY, z, g_viewShiftY);

	dx = p_a->m_x - p_b->m_x;
	if (dx < 0) {
		dx = -dx;
	}

	dy = p_a->m_y - p_b->m_y;
	if (dy < 0) {
		dy = -dy;
	}

	if (dx > dy) {
		p_out->m_u = Lerp(p_a->m_x, p_b->m_x, p_out->m_x, p_a->m_u, p_b->m_u);
		p_out->m_v = Lerp(p_a->m_x, p_b->m_x, p_out->m_x, p_a->m_v, p_b->m_v);
	}
	else if (dy) {
		p_out->m_u = Lerp(p_a->m_y, p_b->m_y, p_out->m_y, p_a->m_u, p_b->m_u);
		p_out->m_v = Lerp(p_a->m_y, p_b->m_y, p_out->m_y, p_a->m_v, p_b->m_v);
	}
	else {
		p_out->m_u = FIXED_SHR(p_a->m_u + p_b->m_u, 1);
		p_out->m_v = FIXED_SHR(p_a->m_v + p_b->m_v, 1);
	}
}

// Stores in p_out the point where the edge from p_a to p_b crosses the screen row p_y.
// FUNCTION: MW2 0x10034779
// FUNCTION: MW2GBL 0x1003a564
void ClipEdgeToRow(ProjectedVertex* p_a, ProjectedVertex* p_b, MechS32 p_y, ProjectedVertex* p_out)
{
	MechS32 z;
	MechS32 dx;
	MechS32 dy;

	p_out->m_z = GetEdgeCrossingDepth(p_a->m_y, p_b->m_y, p_y, 0, p_a->m_z, p_b->m_z);
	z = p_out->m_z;
	p_out->m_screenX = Lerp(p_a->m_screenY, p_b->m_screenY, p_y, p_a->m_screenX, p_b->m_screenX);
	p_out->m_screenY = p_y;
	p_out->m_x = UnprojectCoordinate(p_out->m_screenX, g_viewCenterX, z, g_viewShiftX);
	p_out->m_y = UnprojectCoordinate(g_viewCenterY, p_y, z, g_viewShiftY);

	dx = p_a->m_x - p_b->m_x;
	if (dx < 0) {
		dx = -dx;
	}

	dy = p_a->m_y - p_b->m_y;
	if (dy < 0) {
		dy = -dy;
	}

	if (dy > dx) {
		p_out->m_u = Lerp(p_a->m_y, p_b->m_y, p_out->m_y, p_a->m_u, p_b->m_u);
		p_out->m_v = Lerp(p_a->m_y, p_b->m_y, p_out->m_y, p_a->m_v, p_b->m_v);
	}
	else if (dx) {
		p_out->m_u = Lerp(p_a->m_x, p_b->m_x, p_out->m_x, p_a->m_u, p_b->m_u);
		p_out->m_v = Lerp(p_a->m_x, p_b->m_x, p_out->m_x, p_a->m_v, p_b->m_v);
	}
	else {
		p_out->m_u = FIXED_SHR(p_a->m_u + p_b->m_u, 1);
		p_out->m_v = FIXED_SHR(p_a->m_v + p_b->m_v, 1);
	}
}
#endif
