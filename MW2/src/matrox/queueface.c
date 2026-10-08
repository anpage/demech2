/* The Matrox edition's projection of faces: objectanim.c's GetViewVertex, ClipEdgeToNearPlane,
   ProjectVertex, GetFaceShade and QueueFace in floats, with the vertices' shading for the
   renderer, and two functions of its own (ClipProjectedEdge and ProjectPolygon). 1.1 has these
   at the end of objectanim.c's object, as __asm blocks; the Matrox edition links them as an object
   of their own, after mechcollision.c's. Only the Matrox edition builds this unit; the guard keeps the
   tools that parse every source (with 1.1's structures) out of it. */
#ifdef MW2_MATROX
#include "approxlen.h"
#include "decomp.h"
#include "depthsort.h"
#include "face.h"
#include "faceshade.h"
#include "fixedfloat.h"
#include "matrox/a3d.h"
#include "objectanim.h"
#include "polydraw.h"
#include "projectedvertex.h"
#include "queuedpolygon.h"
#include "recordstacks.h"
#include "rendersettings.h"
#include "shape.h"
#include "types.h"
#include "vertex.h"
#include "view.h"

#include <math.h>
#include <string.h>

// The call QueueFace makes through g_renderSettings.m_drawFace, which has no prototype.
typedef MechS32 (*DrawFaceHook)(Face* p_face, Vertex* p_vertices, MechS32 p_flags, MechFloat p_depth);

// Returns the vertex's projected copy (m_projection), transforming it into view space (by
// g_viewProjX0's rows, from the eyepoint g_viewEyeX) the first time.
// Stack-slot permutation; the products sum in another order (commutative operand order).
// FUNCTION: MW2MATROX 0x10076e20
ProjectedVertex* GetViewVertex(Vertex* p_vertex)
{
	MechFloat dx;
	MechFloat dy;
	MechFloat dz;
	ProjectedVertex* result;

	result = p_vertex->m_projection;
	if (result) {
		return result;
	}

	result = AllocProjectedVertex();
	dx = p_vertex->m_worldX - g_viewEyeX;
	dy = p_vertex->m_worldY - g_viewEyeY;
	dz = p_vertex->m_worldZ - g_viewEyeZ;
	p_vertex->m_projection = result;
	result->m_x = dx * g_viewProjX0 + dy * g_viewProjX1 + dz * g_viewProjX2;
	result->m_y = dx * g_viewProjY0 + dy * g_viewProjY1 + dz * g_viewProjY2;
	result->m_z = p_vertex->m_depth;
	result->m_u = p_vertex->m_u;
	result->m_v = p_vertex->m_v;
	result->m_red = p_vertex->m_red;
	result->m_green = p_vertex->m_green;
	result->m_blue = p_vertex->m_blue;
	result->m_normalX = p_vertex->m_normalX;
	result->m_normalY = p_vertex->m_normalY;
	result->m_normalZ = p_vertex->m_normalZ;
	return result;
}

// Returns a new projected vertex where the edge from p_a to p_b crosses the near plane
// (g_viewNearPlane), its values interpolated.
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x10076f41
ProjectedVertex* ClipEdgeToNearPlane(Vertex* p_a, Vertex* p_b)
{
	MechFloat x0;
	MechFloat y0;
	MechFloat z0;
	MechFloat u0;
	MechFloat v0;
	MechFloat reda;
	MechFloat greena;
	MechFloat bluea;
	MechFloat normalXa;
	MechFloat normalYa;
	MechFloat normalZa;
	MechFloat x1;
	MechFloat y1;
	MechFloat z1;
	MechFloat u1;
	MechFloat v1;
	MechFloat redb;
	MechFloat greenb;
	MechFloat blueb;
	MechFloat normalXb;
	MechFloat normalYb;
	MechFloat normalZb;
	MechFloat toPlane;
	MechFloat span;
	ProjectedVertex* a;
	ProjectedVertex* b;
	ProjectedVertex* result;

	a = GetViewVertex(p_a);
	x0 = a->m_x;
	y0 = a->m_y;
	z0 = a->m_z;
	u0 = a->m_u;
	v0 = a->m_v;
	reda = a->m_red;
	greena = a->m_green;
	bluea = a->m_blue;
	normalXa = a->m_normalX;
	normalYa = a->m_normalY;
	normalZa = a->m_normalZ;
	b = GetViewVertex(p_b);
	x1 = b->m_x;
	y1 = b->m_y;
	z1 = b->m_z;
	u1 = b->m_u;
	v1 = b->m_v;
	redb = b->m_red;
	greenb = b->m_green;
	blueb = b->m_blue;
	normalXb = b->m_normalX;
	normalYb = b->m_normalY;
	normalZb = b->m_normalZ;

	// From the nearer end.
	if (z1 <= z0) {
		if (FIXED_IS_NONZERO(span = z0 - z1)) {
			toPlane = g_viewNearPlane - z1;
			x0 = (x0 - x1) * toPlane / span + x1;
			y0 = (y0 - y1) * toPlane / span + y1;
			u0 = (u0 - u1) * toPlane / span + u1;
			v0 = (v0 - v1) * toPlane / span + v1;
			reda = (reda - redb) * toPlane / span + redb;
			greena = (greena - greenb) * toPlane / span + greenb;
			bluea = (bluea - blueb) * toPlane / span + blueb;
			normalXa = (normalXa - normalXb) * toPlane / span + normalXb;
			normalYa = (normalYa - normalYb) * toPlane / span + normalYb;
			normalZa = (normalZa - normalZb) * toPlane / span + normalZb;
		}
	}
	else {
		if (FIXED_IS_NONZERO(span = z1 - z0)) {
			toPlane = g_viewNearPlane - z0;
			x0 = (x1 - x0) * toPlane / span + x0;
			y0 = (y1 - y0) * toPlane / span + y0;
			u0 = (u1 - u0) * toPlane / span + u0;
			v0 = (v1 - v0) * toPlane / span + v0;
			reda = (redb - reda) * toPlane / span + reda;
			greena = (greenb - greena) * toPlane / span + greena;
			bluea = (blueb - bluea) * toPlane / span + bluea;
			normalXa = (normalXb - normalXa) * toPlane / span + normalXa;
			normalYa = (normalYb - normalYa) * toPlane / span + normalYa;
			normalZa = (normalZb - normalZa) * toPlane / span + normalZa;
		}
	}

	result = AllocProjectedVertex();
	result->m_x = x0;
	result->m_y = y0;
	result->m_z = g_viewNearPlane;
	result->m_u = u0;
	result->m_v = v0;
	result->m_red = reda;
	result->m_green = greena;
	result->m_blue = bluea;
	result->m_normalX = normalXa;
	result->m_normalY = normalYa;
	result->m_normalZ = normalZa;
	return result;
}

// Projects a view-space vertex onto the screen once per frame (m_projected bit 0), with its clip
// outcodes (1 left, 2 right, 4 top, 8 bottom), accumulates the outcodes of the polygon being built
// and adds the vertex to its list (up to 20).
// Stack-slot permutation; the edge tests compare in the other operand order.
// FUNCTION: MW2MATROX 0x100773f0
ProjectedVertex* ProjectVertex(ProjectedVertex* p_vertex)
{
	MechS32 screenX;
	MechU8 outcode;
	MechS32 screenY;
	MechU8 unused;
	MechFloat depth;

	outcode = 0;
	unused = 0;
	if (!(p_vertex->m_projected & 1)) {
		depth = p_vertex->m_z;
		p_vertex->m_screenX = g_viewCenterX + (MechS32) (p_vertex->m_x / depth);
		screenX = p_vertex->m_screenX;
		p_vertex->m_screenY = -(MechS32) (p_vertex->m_y / depth) + g_viewCenterY;
		screenY = p_vertex->m_screenY;
		if (screenX > g_viewRight) {
			outcode |= 2;
		}

		if (screenX < g_viewLeft) {
			outcode |= 1;
		}

		if (screenY > g_viewBottom) {
			outcode |= 8;
		}

		if (screenY < g_viewTop) {
			outcode |= 4;
		}

		p_vertex->m_outcode = outcode;
		p_vertex->m_projected |= 1;
	}
	else {
		outcode = p_vertex->m_outcode;
	}

	g_polygonOrCodes |= outcode;
	g_polygonAndCodes &= outcode;
	if (g_polygonPointCount >= 20) {
		g_queueHasRoom = 0;
	}
	else {
		g_polygonPoints[g_polygonPointCount++] = p_vertex;
	}

	return p_vertex;
}

// Returns the shade (0x7f: full; 128 times the cosine) of p_face from the angle between its normal and the direction
// from its first vertex to the light (g_viewLightX), or the light's direction itself with
// g_directionalLight.
// Stack-slot permutation; the dot product sums in another order (commutative operand order).
// FUNCTION: MW2MATROX 0x10077579
MechS32 GetFaceShade(Face* p_face, Vertex* p_vertices)
{
	MechFloat normalX;
	MechFloat normalY;
	MechFloat normalZ;
	MechFloat dot;
	MechS16 shade;
	Vertex* vertex;
	MechFloat dx;
	MechFloat dy;
	MechFloat dz;

	normalX = p_face->m_normal[0];
	normalY = p_face->m_normal[1];
	normalZ = p_face->m_normal[2];
	if (g_directionalLight) {
		if ((MechFloat) fabs(g_viewLightScale) < 1e-07f) {
			return 0x7f;
		}

		dot = normalX * g_viewLightX + normalY * g_viewLightY + normalZ * g_viewLightZ;
		shade = (MechS16) (dot / g_viewLightScale);
	}
	else {
		vertex = &p_vertices[((MechU8*) p_face)[p_face->m_indexOffset]];
		dx = g_viewLightX - vertex->m_worldX;
		dy = g_viewLightY - vertex->m_worldY;
		dz = g_viewLightZ - vertex->m_worldZ;
		if ((MechFloat) fabs(dx) < 1e-07f && (MechFloat) fabs(dy) < 1e-07f && (MechFloat) fabs(dz) < 1e-07f) {
			return 0x7f;
		}

		dot = normalX * dx + normalY * dy + normalZ * dz;
		shade = (MechS16) (dot / ApproximateVectorLength(dx, dy, dz) * 128.0f);
	}

	return shade;
}

// Queues p_face for drawing unless it faces away: transforms its vertices' depths (with their near
// (1) and far (2) clip codes) if not done this frame, clips it to the near plane through the
// projection hook (g_renderSettings) and adds the polygon to the list being built (g_depthList)
// with its depth, by g_queuedShapeFlags's rule: the average (2), nearest (4) or farthest of its
// vertices' depths. The vertices take their shades from the light, scaled by the shape's damage.
// Stack-slot permutation; the dot products sum in another order (commutative operand order),
// and the loops' and the depth list's tests compare in the other operand order.
// FUNCTION: MW2MATROX 0x1007770c
void QueueFace(Face* p_face, Vertex* p_vertices)
{
	MechS8 clipped;
	MechS32 kind;
	MechFloat depth;
	MechS8 andCodes;
	MechS8 firstClipped;
	Vertex* first;
	MechU8 codes;
	MechFloat dot;
	Vertex* vertex;
	MechU8* index;
	QueuedPolygon* poly;
	MechFloat scale;
	MechU8 orCodes;
	MechS32 count;
	MechFloat viewDepth;
	MechS8 previousClipped;
	Vertex* previous;
	ProjectedVertex** points;
	ProjectedVertex* point;
	Vertex* lit;
	MechFloat dx;
	MechFloat dy;
	MechFloat dz;
	DepthEntry* entry;

	orCodes = 0;
	andCodes = 3;
	g_facesTried++;

	// Faces of three vertices or more face away when the eyepoint's offset from the first one has
	// a dot product with the normal that isn't negative.
	if (p_face->m_indexCount >= 3) {
		vertex = &p_vertices[((MechU8*) p_face)[p_face->m_indexOffset]];
		dot = (vertex->m_worldX - g_viewEyeX) * p_face->m_normal[0] +
			  (vertex->m_worldY - g_viewEyeY) * p_face->m_normal[1] +
			  (vertex->m_worldZ - g_viewEyeZ) * p_face->m_normal[2];
		if (!FIXED_IS_NEGATIVE(dot)) {
			return;
		}
	}

	g_facesFrontFacing++;
	for (index = (MechU8*) p_face + p_face->m_indexOffset, count = p_face->m_indexCount; count--; index++) {
		vertex = &p_vertices[*index];
		if (!(vertex->m_flags & 4)) {
			g_verticesTransformed++;
			viewDepth = (vertex->m_worldX - g_viewEyeX) * g_viewProjZ0 +
						(vertex->m_worldY - g_viewEyeY) * g_viewProjZ1 + (vertex->m_worldZ - g_viewEyeZ) * g_viewProjZ2;
			vertex->m_depth = viewDepth;
			vertex->m_flags |= 4;
			codes = 0;
			if (viewDepth < g_viewNearPlane) {
				codes |= 1;
			}
			else if (viewDepth > g_viewFarPlane) {
				codes |= 2;
			}

			vertex->m_flags &= 0xfc;
			vertex->m_flags |= codes;
		}

		codes = vertex->m_flags;
		orCodes |= codes;
		andCodes &= codes;
	}

	if (andCodes == 1 || andCodes == 2) {
		return;
	}

	// The clipping walk: each vertex in front of the near plane, and each crossing of it.
	g_polygonOrCodes = 0;
	g_polygonAndCodes = 0xf;
	g_polygonPointCount = 0;
	index = (MechU8*) p_face + p_face->m_indexOffset;
	count = p_face->m_indexCount;
	first = previous = &p_vertices[*index];
	firstClipped = previousClipped = first->m_flags & 1;
	if (!firstClipped) {
		g_renderSettings.m_projectVertex(GetViewVertex(first));
	}

	while (--count) {
		index++;
		vertex = &p_vertices[*index];
		clipped = vertex->m_flags & 1;
		if (previousClipped != clipped) {
			g_renderSettings.m_projectVertex(ClipEdgeToNearPlane(previous, vertex));
		}

		previous = vertex;
		previousClipped = clipped;
		if (!clipped) {
			g_renderSettings.m_projectVertex(GetViewVertex(previous));
		}
	}

	if (previousClipped != firstClipped) {
		g_renderSettings.m_projectVertex(ClipEdgeToNearPlane(previous, first));
	}

	if ((g_polygonPointCount < 3 && p_face->m_indexCount > 2) || g_polygonAndCodes) {
		return;
	}

	points = g_polygonPoints;
	poly = (QueuedPolygon*) AllocQueuedPolygon();
	poly->m_face = p_face;
	g_polygonPointCursor = g_drawBufferBottom;
	poly->m_count = g_polygonPointCount;
	poly->m_outcodes = 0;
	if (g_queuedShapeFlags & 2) {
		depth = 0;
		for (count = 0; count < g_polygonPointCount; count++) {
			point = *points;
			points++;
			depth += point->m_z;
			poly->m_outcodes |= point->m_outcode;
		}

		depth /= g_polygonPointCount;
	}
	else if (g_queuedShapeFlags & 4) {
		depth = FIXED_MAX;
		for (count = 0; count < g_polygonPointCount; count++) {
			point = *points;
			points++;
			if (point->m_z < depth) {
				depth = point->m_z;
			}

			poly->m_outcodes |= point->m_outcode;
		}
	}
	else {
		depth = 0;
		for (count = 0; count < g_polygonPointCount; count++) {
			point = *points;
			points++;
			if (point->m_z > depth) {
				depth = point->m_z;
			}

			poly->m_outcodes |= point->m_outcode;
		}
	}

	if (g_queuedShapeFlags & 1) {
		depth += 268435456.0f;
	}

	poly->m_depth = depth;
	points = g_polygonPoints;
	memcpy(g_polygonPointCursor, points, g_polygonPointCount * sizeof(ProjectedVertex*));
	g_polygonPointCursor += g_polygonPointCount * sizeof(ProjectedVertex*);
	points += g_polygonPointCount;
	g_drawBufferBottom = g_polygonPointCursor;
	poly->m_flags = ((DrawFaceHook) g_renderSettings.m_drawFace)(p_face, p_vertices, p_face->m_color, depth);

	// The shape's damage level (its flags' bits 4-7, of 15) dims its vertices, or brightens them
	// with g_brightenDamage: for parts of a player and for buildings (type 0x50).
	kind = p_face->m_shape->m_kind;
	if ((kind & 0x100) || (kind & 0xf0) == 0x50) {
		scale = ((p_face->m_shape->m_flags & 0xf0) >> 4) / 15.0f;
		if (g_brightenDamage) {
			scale = scale + 1.0f;
		}
		else {
			scale = 1.0f - scale;
		}
	}
	else {
		scale = 1.0f;
	}

	for (count = 0; count < poly->m_count; count++) {
		point = g_polygonPoints[count];
		if (!(point->m_projected & 2)) {
			if (g_directionalLight) {
				if ((MechFloat) fabs(g_viewLightLength) < 1e-07f) {
					point->m_light = 1.0f;
				}
				else {
					point->m_light = (point->m_normalX * g_viewLightX + point->m_normalY * g_viewLightY +
									  point->m_normalZ * g_viewLightZ) /
									 g_viewLightLength;
				}
			}
			else {
				lit = &p_vertices[((MechU8*) p_face)[p_face->m_indexOffset]];
				dx = g_viewLightX - lit->m_worldX;
				dy = g_viewLightY - lit->m_worldY;
				dz = g_viewLightZ - lit->m_worldZ;
				if ((MechFloat) fabs(dx) < 1e-07f && (MechFloat) fabs(dy) < 1e-07f && (MechFloat) fabs(dz) < 1e-07f) {
					point->m_light = 1.0f;
				}
				else {
					point->m_light = (point->m_normalX * dx + point->m_normalY * dy + point->m_normalZ * dz) /
									 ApproximateVectorLength(dx, dy, dz);
				}
			}

			point->m_light *= 5.0f;
			if (point->m_light < g_viewAmbientScale) {
				point->m_light = g_viewAmbientScale;
			}

			point->m_light *= scale;
			if (point->m_light > 1.0f) {
				point->m_light = 1.0f;
			}

			point->m_projected |= 2;
		}
	}

	if (g_depthEntryCount < g_depthListCapacity) {
		g_polygonsQueued++;
		if (g_polygonPointCount > 1) {
			g_polygonCount++;
		}

		entry = &g_depthList[g_depthEntryCount];
		entry->m_poly = poly;
		entry->m_depth = depth;
		g_depthEntryCount++;
	}
	else {
		g_queueHasRoom = 0;
	}
}

// ClipEdgeToNearPlane for two projected vertices, at the depth p_plane (the new vertex still
// takes g_viewNearPlane's depth).
// Stack-slot permutation; z1 <= z0 compares in the other operand order.
// FUNCTION: MW2MATROX 0x10077fa7
ProjectedVertex* ClipProjectedEdge(ProjectedVertex* p_a, ProjectedVertex* p_b, MechFloat p_plane)
{
	MechFloat x0;
	MechFloat y0;
	MechFloat z0;
	MechFloat u0;
	MechFloat v0;
	MechFloat reda;
	MechFloat greena;
	MechFloat bluea;
	MechFloat normalXa;
	MechFloat normalYa;
	MechFloat normalZa;
	MechFloat x1;
	MechFloat y1;
	MechFloat z1;
	MechFloat u1;
	MechFloat v1;
	MechFloat redb;
	MechFloat greenb;
	MechFloat blueb;
	MechFloat normalXb;
	MechFloat normalYb;
	MechFloat normalZb;
	MechFloat toPlane;
	MechFloat span;
	ProjectedVertex* result;

	x0 = p_a->m_x;
	y0 = p_a->m_y;
	z0 = p_a->m_z;
	u0 = p_a->m_u;
	v0 = p_a->m_v;
	reda = p_a->m_red;
	greena = p_a->m_green;
	bluea = p_a->m_blue;
	normalXa = p_a->m_normalX;
	normalYa = p_a->m_normalY;
	normalZa = p_a->m_normalZ;
	x1 = p_b->m_x;
	y1 = p_b->m_y;
	z1 = p_b->m_z;
	u1 = p_b->m_u;
	v1 = p_b->m_v;
	redb = p_b->m_red;
	greenb = p_b->m_green;
	blueb = p_b->m_blue;
	normalXb = p_b->m_normalX;
	normalYb = p_b->m_normalY;
	normalZb = p_b->m_normalZ;

	// From the nearer end.
	if (z1 <= z0) {
		if (FIXED_IS_NONZERO(span = z0 - z1)) {
			toPlane = p_plane - z1;
			x0 = (x0 - x1) * toPlane / span + x1;
			y0 = (y0 - y1) * toPlane / span + y1;
			u0 = (u0 - u1) * toPlane / span + u1;
			v0 = (v0 - v1) * toPlane / span + v1;
			reda = (reda - redb) * toPlane / span + redb;
			greena = (greena - greenb) * toPlane / span + greenb;
			bluea = (bluea - blueb) * toPlane / span + blueb;
			normalXa = (normalXa - normalXb) * toPlane / span + normalXb;
			normalYa = (normalYa - normalYb) * toPlane / span + normalYb;
			normalZa = (normalZa - normalZb) * toPlane / span + normalZb;
		}
	}
	else {
		if (FIXED_IS_NONZERO(span = z1 - z0)) {
			toPlane = p_plane - z0;
			x0 = (x1 - x0) * toPlane / span + x0;
			y0 = (y1 - y0) * toPlane / span + y0;
			u0 = (u1 - u0) * toPlane / span + u0;
			v0 = (v1 - v0) * toPlane / span + v0;
			reda = (redb - reda) * toPlane / span + reda;
			greena = (greenb - greena) * toPlane / span + greena;
			bluea = (blueb - bluea) * toPlane / span + bluea;
			normalXa = (normalXb - normalXa) * toPlane / span + normalXa;
			normalYa = (normalYb - normalYa) * toPlane / span + normalYa;
			normalZa = (normalZb - normalZa) * toPlane / span + normalZa;
		}
	}

	result = AllocProjectedVertex();
	result->m_x = x0;
	result->m_y = y0;
	result->m_z = g_viewNearPlane;
	result->m_u = u0;
	result->m_v = v0;
	result->m_red = reda;
	result->m_green = greena;
	result->m_blue = bluea;
	result->m_normalX = normalXa;
	result->m_normalY = normalYa;
	result->m_normalZ = normalZa;
	return result;
}

// Transforms p_count vertices into view space, clips the polygon they make through the renderer
// (FUN_1005a5f0) and projects the clipped vertices (ProjectVertex); returns their count. Its
// callers pass a third argument it doesn't read.
// Stack-slot permutation; the products sum in another order (commutative operand order), and
// i < p_count compares in the other operand order.
// FUNCTION: MW2MATROX 0x10078432
MechS32 ProjectPolygon(Vertex* p_vertices, MechU32 p_count, MechS32 p_unk0x08)
{
	ProjectedVertex* point;
	MechU32 i;
	ProjectedVertex in[20];
	Vertex* vertex;
	MechFloat dz;
	MechFloat dy;
	MechFloat dx;
	MechS32 unused1;
	MechS32 unused2;
	ProjectedVertex out[20];

	g_polygonPointCount = 0;
	vertex = p_vertices;
	point = in;
	for (i = 0; i < p_count; i++) {
		dx = vertex->m_worldX - g_viewEyeX;
		dy = vertex->m_worldY - g_viewEyeY;
		dz = vertex->m_worldZ - g_viewEyeZ;
		point->m_x = dz * g_viewProjX2 + dy * g_viewProjX1 + dx * g_viewProjX0;
		point->m_y = dx * g_viewProjY0 + dy * g_viewProjY1 + dz * g_viewProjY2;
		point->m_z = dx * g_viewProjZ0 + dy * g_viewProjZ1 + dz * g_viewProjZ2;
		vertex->m_projection = point;
		point->m_u = vertex->m_u;
		point->m_v = vertex->m_v;
		point->m_red = vertex->m_red;
		point->m_green = vertex->m_green;
		point->m_blue = vertex->m_blue;
		point->m_normalX = vertex->m_normalX;
		point->m_normalY = vertex->m_normalY;
		point->m_normalZ = vertex->m_normalZ;
		vertex++;
		point++;
	}

	p_count = FUN_1005a5f0(out, in, p_count, 1);
	point = out;
	for (i = 0; i < p_count; i++) {
		point->m_projected = 0;
		ProjectVertex(point);
		point++;
	}

	return g_polygonPointCount;
}
#endif
