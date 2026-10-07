#include "quadtree.h"

#include "collision.h"
#include "decomp.h"
#include "face.h"
#include "fixedfloat.h"
#include "overlay.h"
#include "ray.h"
#include "shape.h"
#include "shapegeom.h"
#include "simmain.h"
#include "types.h"
#include "vertex.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(QuadtreeNode, 0x2c)

// GLOBAL: MW2 0x100a37dc
// GLOBAL: MW2MATROX 0x100a4834
MechS32 g_quadtreesDisabled = 0;

// Builds p_shape's quadtree (m_collisionData) over its model's bounds, unless g_quadtreesDisabled is set.
// Stack-slot permutation of the locals. Operand order: root->m_unk0x14 < vertex->m_worldZ loads
// the vertex's first in the original.
// MW2MATROX: the comparisons of root's bounds with the vertex's coordinates have their operands the
// other way round.
// FUNCTION: MW2 0x1001df00
// FUNCTION: MW2MATROX 0x100158f0
void BuildShapeQuadtree(Shape* p_shape)
{
	Model* model;
	QuadtreeNode* root;
	Vertex* vertices;
	Vertex* vertex;
	MechS32 i;
	MechS32 j;

	if (g_quadtreesDisabled) {
		return;
	}

	model = p_shape->m_models;
	if (model) {
		vertex = (Vertex*) (model + 1);
		vertices = vertex;
		root = AllocQuadtreeNode(
			vertex->m_worldX,
			vertex->m_worldX,
			vertex->m_worldY,
			vertex->m_worldY,
			vertex->m_worldZ,
			vertex->m_worldZ,
			0
		);
		p_shape->m_collisionData = root;
		if (!root) {
			return;
		}

		for (i = 1; i < model->m_vertexCount; i++) {
			vertex++;
			if (root->m_maxX < vertex->m_worldX) {
				root->m_maxX = vertex->m_worldX;
			}

			if (vertex->m_worldX < root->m_minX) {
				root->m_minX = vertex->m_worldX;
			}

			if (root->m_maxZ < vertex->m_worldZ) {
				root->m_maxZ = vertex->m_worldZ;
			}

			if (vertex->m_worldZ < root->m_minZ) {
				root->m_minZ = vertex->m_worldZ;
			}

			if (root->m_maxY < vertex->m_worldY) {
				root->m_maxY = vertex->m_worldY;
			}

			if (root->m_minY > vertex->m_worldY) {
				root->m_minY = vertex->m_worldY;
			}
		}

		for (j = 0; j < 4; j++) {
			root->m_children[j] = BuildQuadtreeChild(root, j, model);
		}
	}
}

// Builds the quadtree child p_quadrant of p_node from p_model's faces. A leaf takes at most 25
// faces; with more, the child is split again.
// Stack-slot permutation; minX >= maxX, minZ >= maxZ and faceHigh > highY compare in the other
// operand order.
// MW2MATROX: the sums of the node's bounds add their operands in the other order, the minX >= maxX,
// minZ >= maxZ and faceHigh > highY comparisons have their operands the other way round, and
// entries sits in another stack slot.
// FUNCTION: MW2 0x1001e0a7
// FUNCTION: MW2MATROX 0x10015ab5
QuadtreeNode* BuildQuadtreeChild(QuadtreeNode* p_node, MechS32 p_quadrant, Model* p_model)
{
	MechS32 j;
	MechS32 i;
	Face* entries[25];
	QuadtreeNode* node;
	MechScalar faceLow;
	MechScalar minX;
	MechScalar lowY;
	MechScalar minZ;
	MechS32 count;
	QuadtreeNode* child;
	Face* face;
	MechScalar faceHigh;
	MechScalar maxX;
	MechScalar highY;
	MechScalar maxZ;
	Face** faces;

	if (p_quadrant & 1) {
		minX = FIXED_SHR(p_node->m_maxX + p_node->m_minX, 1) + 1;
		maxX = p_node->m_maxX;
	}
	else {
		minX = p_node->m_minX;
		maxX = FIXED_SHR(p_node->m_maxX + p_node->m_minX, 1);
	}

	if (p_quadrant & 2) {
		minZ = FIXED_SHR(p_node->m_maxZ + p_node->m_minZ, 1) + 1;
		maxZ = p_node->m_maxZ;
	}
	else {
		minZ = p_node->m_minZ;
		maxZ = FIXED_SHR(p_node->m_maxZ + p_node->m_minZ, 1);
	}

	if (minX >= maxX || minZ >= maxZ) {
		return NULL;
	}

	count = 0;
	lowY = FIXED_MAX;
	highY = FIXED_MIN;
	for (i = 0; i < p_model->m_faceCount; i++) {
		face = (Face*) ((MechU8*) p_model + p_model->m_faceOffset) + i;
		if (FaceOverlapsBox(face, p_model, minX, maxX, minZ, maxZ, &faceLow, &faceHigh)) {
			count++;
			if (count > 25) {
				lowY = FIXED_MAX;
				highY = FIXED_MIN;
				node = AllocQuadtreeNode(minX, maxX, lowY, highY, minZ, maxZ, 0);
				if (!node) {
					return NULL;
				}

				for (j = 0; j < 4; j++) {
					node->m_children[j] = BuildQuadtreeChild(node, j, p_model);
					child = node->m_children[j];
					if (child) {
						if (child->m_maxY > highY) {
							highY = child->m_maxY;
						}

						if (child->m_minY < lowY) {
							lowY = child->m_minY;
						}
					}
				}

				node->m_maxY = highY;
				node->m_minY = lowY;
				if (node->m_minY == 0x7fffffff) {
					node->m_minY = node->m_maxY = 0;
				}

				return node;
			}

			if (faceHigh > highY) {
				highY = faceHigh;
			}

			if (lowY > faceLow) {
				lowY = faceLow;
			}

			entries[count - 1] = face;
		}
	}

	if (count == 0) {
		lowY = highY = 0;
	}

	node = AllocQuadtreeNode(minX, maxX, lowY, highY, minZ, maxZ, count);
	if (!node) {
		return NULL;
	}

	if (count > 0) {
		faces = (Face**) (node + 1);
		memcpy(faces, entries, count * sizeof(Face*));
	}

	return node;
}

// Allocates a quadtree node with room for p_faceCount entries, cleared.
// The only diff is a stack-slot permutation of entries and node.
// FUNCTION: MW2 0x1001e429
// FUNCTION: MW2MATROX 0x10015ee6
QuadtreeNode* AllocQuadtreeNode(
	MechScalar p_minX,
	MechScalar p_maxX,
	MechScalar p_minY,
	MechScalar p_maxY,
	MechScalar p_minZ,
	MechScalar p_maxZ,
	MechS32 p_faceCount
)
{
	undefined4* entries;
	QuadtreeNode* node;
	MechS32 i;

	node = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, p_faceCount * sizeof(undefined4) + sizeof(QuadtreeNode));
	if (node) {
		node->m_minX = p_minX;
		node->m_maxX = p_maxX;
		node->m_minY = p_minY;
		node->m_maxY = p_maxY;
		node->m_minZ = p_minZ;
		node->m_maxZ = p_maxZ;
		node->m_faceCount = p_faceCount;
		i = 4;
		while (i--) {
			node->m_children[i] = NULL;
		}

		if (p_faceCount > 0) {
			entries = (undefined4*) (node + 1);
			memset(entries, 0, p_faceCount * sizeof(undefined4));
		}
	}
	else {
		MonoPrint("Not enough memory for quadtrees!!!!");
	}

	return node;
}

// Frees a quadtree.
// FUNCTION: MW2 0x1001e50d
// FUNCTION: MW2MATROX 0x10015fca
void FreeQuadtree(QuadtreeNode* p_node)
{
	MechS32 i;

	if (!p_node) {
		return;
	}

	if (p_node->m_faceCount == 0) {
		for (i = 0; i < 4; i++) {
			FreeQuadtree(p_node->m_children[i]);
		}
	}

	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_node);
}

// Returns whether face p_face of p_model overlaps the box from (p_minX, p_minZ) to
// (p_maxX, p_maxZ), seen from above; if it does, stores the face's height range.
// Stack-slot permutation; the min/max tests and the final bounds tests compare in the other operand
// order.
// MW2MATROX: the min/max tests and the final bounds tests have their operands the other way round.
// FUNCTION: MW2 0x1001e57a
// FUNCTION: MW2MATROX 0x10016037
MechS32 FaceOverlapsBox(
	Face* p_face,
	Model* p_model,
	MechScalar p_minX,
	MechScalar p_maxX,
	MechScalar p_minZ,
	MechScalar p_maxZ,
	MechScalar* p_minY,
	MechScalar* p_maxY
)
{
	MechS32 count;
	MechScalar maxY;
	MechScalar maxZ;
	MechScalar minX;
	Vertex* vertices;
	Vertex* vertex;
	MechScalar x;
	MechScalar y;
	MechS32 i;
	MechScalar z;
	MechScalar minY;
	MechScalar minZ;
	MechScalar maxX;

	count = p_face->m_indexCount;
	vertices = (Vertex*) (p_model + 1);
	minX = minY = minZ = FIXED_MAX;
	maxX = maxY = maxZ = FIXED_MIN;
	for (i = 0; i < count; i++) {
		vertex = &vertices[((MechU8*) p_face)[p_face->m_indexOffset + i]];
		x = vertex->m_worldX;
		y = vertex->m_worldY;
		z = vertex->m_worldZ;
		if (x < minX) {
			minX = x;
		}

		if (y < minY) {
			minY = y;
		}

		if (z < minZ) {
			minZ = z;
		}

		if (x > maxX) {
			maxX = x;
		}

		if (y > maxY) {
			maxY = y;
		}

		if (z > maxZ) {
			maxZ = z;
		}
	}

	if (minX <= p_maxX && maxX >= p_minX && minZ <= p_maxZ && maxZ >= p_minZ) {
		*p_minY = minY;
		*p_maxY = maxY;
		return TRUE;
	}

	return FALSE;
}

// Classifies the point (p_x, p_y, p_z) against the quadtree's faces: 0 outside the tree's
// ground area, 2 above the faces under it, 3 below one of them.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1001e6dc
// FUNCTION: MW2MATROX 0x100161c7
MechS32 ClassifyQuadtreePoint(QuadtreeNode* p_node, Model* p_model, MechScalar p_x, MechScalar p_y, MechScalar p_z)
{
	MechS32 result;
	Vertex* vertices;
	Face** faces;
	MechS32 i;
	Face* face;
	MechScalar height;

	if (!p_node) {
		return 0;
	}

	if (p_node->m_minX > p_x || p_node->m_maxX < p_x || p_node->m_minZ > p_z || p_node->m_maxZ < p_z) {
		return 0;
	}

	if (p_node->m_maxY < p_y) {
		return 2;
	}

	if (p_node->m_minY > p_y) {
		return 3;
	}

	if (p_node->m_faceCount == 0) {
		result = ClassifyQuadtreePoint(p_node->m_children[0], p_model, p_x, p_y, p_z);
		if (result) {
			return result;
		}

		result = ClassifyQuadtreePoint(p_node->m_children[1], p_model, p_x, p_y, p_z);
		if (result) {
			return result;
		}

		result = ClassifyQuadtreePoint(p_node->m_children[2], p_model, p_x, p_y, p_z);
		if (result) {
			return result;
		}

		result = ClassifyQuadtreePoint(p_node->m_children[3], p_model, p_x, p_y, p_z);
		if (result) {
			return result;
		}

		return 2;
	}

	vertices = (Vertex*) (p_model + 1);
	faces = (Face**) (p_node + 1);
	for (i = 0; i < p_node->m_faceCount; i++) {
		face = *faces++;
		if (face->m_normal[1] > 0 && IsPointInFaceXZ(face, vertices, p_x, p_z)) {
			if (IsBelowFacePlane(face, vertices, p_x, p_y, p_z, &height)) {
				return 3;
			}
			else {
				return 2;
			}
		}
	}

	return 2;
}

// Returns whether p_ray hits one of the quadtree's faces, shortening it to the hit.
// Stack-slot permutation; the t1 < tMax and t0 < tMax tests compare in the other operand order.
// MW2MATROX: the comparisons of t0 and t1 with tMin and tMax have their operands the other way
// round, and t0 < tMax stores t0 before it compares (fst, fcomp) where the original compares first
// (fcom, fstp).
// FUNCTION: MW2 0x1001e90f
// FUNCTION: MW2MATROX 0x10016422
MechS32 TestQuadtreeRay(QuadtreeNode* p_node, Model* p_model, Ray* p_ray)
{
	MechScalar tMin;
	Vertex* vertices;
	MechScalar t0;
	Face** faces;
	MechS32 i;
	MechScalar t1;
	MechScalar tMax;
	Face* face;

	if (!p_node) {
		return FALSE;
	}

	if (ClipRaySlab(p_ray->m_x0, p_ray->m_dirX, p_node->m_minX, p_node->m_maxX, &tMin, &tMax)) {
		return FALSE;
	}

	if (ClipRaySlab(p_ray->m_y0, p_ray->m_dirY, p_node->m_minY, p_node->m_maxY, &t0, &t1)) {
		return FALSE;
	}

	if (t0 > tMin) {
		tMin = t0;
	}

	if (t1 < tMax) {
		tMax = t1;
	}

	if (ClipRaySlab(p_ray->m_z0, p_ray->m_dirZ, p_node->m_minZ, p_node->m_maxZ, &t0, &t1)) {
		return FALSE;
	}

	if (t0 > tMin) {
		tMin = t0;
	}

	if (t1 < tMax) {
		tMax = t1;
	}

	if (tMax < tMin) {
		return FALSE;
	}

	if (tMin < 0) {
		tMin = 0;
	}

	t0 = GetRayLength(p_ray);
	if (t0 < tMax) {
		tMax = t0;
	}

	if (tMax < tMin) {
		return FALSE;
	}

	if (p_node->m_faceCount == 0) {
		if (TestQuadtreeChildrenRay(p_node, p_model, p_ray)) {
			return TRUE;
		}

		return FALSE;
	}

	vertices = (Vertex*) (p_model + 1);
	faces = (Face**) (p_node + 1);
	for (i = 0; i < p_node->m_faceCount; i++) {
		face = *faces++;
		if (IntersectRayFace(face, vertices, p_ray)) {
			return TRUE;
		}
	}

	return FALSE;
}

// Finds the nearest hit of p_ray among p_node's children and shortens p_ray to it. Whether any
// child was hit.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1001eb25
// FUNCTION: MW2MATROX 0x10016662
MechS32 TestQuadtreeChildrenRay(QuadtreeNode* p_node, Model* p_model, Ray* p_ray)
{
	MechScalar length;
	MechS32 i;
	Ray best;
	Ray ray;
	MechScalar nearest;

	nearest = FIXED_MAX;
	CopyRay(&ray, p_ray);
	for (i = 0; i < 4; i++) {
		if (TestQuadtreeRay(p_node->m_children[i], p_model, &ray)) {
			length = GetRayLength(&ray);
			if (length < nearest) {
				nearest = length;
				CopyRay(&best, &ray);
			}

			CopyRay(&ray, p_ray);
		}
	}

	// The edition compares with the integer 0x7fffffff (2147483648.0f), not with FIXED_MAX.
	if (nearest < 0x7fffffff) {
		CopyRay(p_ray, &best);
		return TRUE;
	}

	return FALSE;
}

// Finds the upward face of the quadtree under (p_x, p_z) and stores its height at the point in
// p_top, or 0 if there is none. Returns FALSE outside the tree's ground area.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1001ebfa
// FUNCTION: MW2MATROX 0x10016740
MechS32 GetQuadtreeTop(
	QuadtreeNode* p_node,
	Model* p_model,
	MechScalar p_x,
	MechScalar p_y,
	MechScalar p_z,
	MechScalar* p_top
)
{
	Vertex* vertices;
	Face** faces;
	MechS32 i;
	Face* face;

	if (!p_node) {
		return FALSE;
	}

	if (p_node->m_minX > p_x || p_node->m_maxX < p_x || p_node->m_minZ > p_z || p_node->m_maxZ < p_z) {
		return FALSE;
	}

	if (p_node->m_faceCount == 0) {
		if (GetQuadtreeTop(p_node->m_children[0], p_model, p_x, p_y, p_z, p_top)) {
			return TRUE;
		}

		if (GetQuadtreeTop(p_node->m_children[1], p_model, p_x, p_y, p_z, p_top)) {
			return TRUE;
		}

		if (GetQuadtreeTop(p_node->m_children[2], p_model, p_x, p_y, p_z, p_top)) {
			return TRUE;
		}

		if (GetQuadtreeTop(p_node->m_children[3], p_model, p_x, p_y, p_z, p_top)) {
			return TRUE;
		}

		*p_top = 0;
		return TRUE;
	}

	vertices = (Vertex*) (p_model + 1);
	faces = (Face**) (p_node + 1);
	for (i = 0; i < p_node->m_faceCount; i++) {
		face = *faces++;
		if (face->m_normal[1] > 0 && IsPointInFaceXZ(face, vertices, p_x, p_z)) {
			IsBelowFacePlane(face, vertices, p_x, p_y, p_z, p_top);
			return TRUE;
		}
	}

	*p_top = 0;
	return TRUE;
}

// FUNCTION: MW2 0x1001edfa
// FUNCTION: MW2MATROX 0x1001695e
void DisableQuadtrees(void)
{
	g_quadtreesDisabled = 1;
}

// Returns the bytes a quadtree takes.
// FUNCTION: MW2 0x1001ee0f
// FUNCTION: MW2MATROX 0x10016973
MechS32 GetQuadtreeSize(QuadtreeNode* p_node)
{
	MechS32 i;
	MechS32 size;

	if (!p_node) {
		return 0;
	}

	size = p_node->m_faceCount * sizeof(undefined4) + sizeof(QuadtreeNode);
	for (i = 0; i < 4; i++) {
		size += GetQuadtreeSize(p_node->m_children[i]);
	}

	return size;
}
