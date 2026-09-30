#include "unk1001df00.h"

#include "decomp.h"
#include "duskmoth.h"
#include "emberfern.h"
#include "overlay.h"
#include "ray.h"
#include "simmain.h"
#include "types.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(AzureThicket0x2c, 0x2c)

// GLOBAL: MW2 0x100a37dc
MechS32 g_unk0x100a37dc = 0;

// Allocates a quadtree node with room for p_unk0x18 entries, cleared.
// The only diff is a stack-slot permutation of entries and node.
// FUNCTION: MW2 0x1001e429
AzureThicket0x2c* FUN_1001e429(
	undefined4 p_unk0x00,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14,
	MechS32 p_unk0x18
)
{
	undefined4* entries;
	AzureThicket0x2c* node;
	MechS32 i;

	node = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, p_unk0x18 * sizeof(undefined4) + sizeof(AzureThicket0x2c));
	if (node) {
		node->m_unk0x00 = p_unk0x00;
		node->m_unk0x04 = p_unk0x04;
		node->m_unk0x08 = p_unk0x08;
		node->m_unk0x0c = p_unk0x0c;
		node->m_unk0x10 = p_unk0x10;
		node->m_unk0x14 = p_unk0x14;
		node->m_unk0x18 = p_unk0x18;
		i = 4;
		while (i--) {
			node->m_children[i] = NULL;
		}

		if (p_unk0x18 > 0) {
			entries = (undefined4*) (node + 1);
			memset(entries, 0, p_unk0x18 * sizeof(undefined4));
		}
	}
	else {
		FUN_100591d1("Not enough memory for quadtrees!!!!");
	}

	return node;
}

// Frees a quadtree.
// FUNCTION: MW2 0x1001e50d
void FUN_1001e50d(AzureThicket0x2c* p_node)
{
	MechS32 i;

	if (!p_node) {
		return;
	}

	if (p_node->m_unk0x18 == 0) {
		for (i = 0; i < 4; i++) {
			FUN_1001e50d(p_node->m_children[i]);
		}
	}

	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_node);
}

// Returns whether face p_face of p_model overlaps the box from (p_minX, p_minZ) to
// (p_maxX, p_maxZ), seen from above; if it does, stores the face's height range.
// Stack-slot permutation; the min/max tests and the final bounds tests compare in the other operand
// order.
// FUNCTION: MW2 0x1001e57a
MechS32 FUN_1001e57a(
	DuskMoth0x24* p_face,
	GraniteLattice0x18* p_model,
	MechS32 p_minX,
	MechS32 p_maxX,
	MechS32 p_minZ,
	MechS32 p_maxZ,
	MechS32* p_minY,
	MechS32* p_maxY
)
{
	MechS32 count;
	MechS32 maxY;
	MechS32 maxZ;
	MechS32 minX;
	EmberFern0x2c* vertices;
	EmberFern0x2c* vertex;
	MechS32 x;
	MechS32 y;
	MechS32 i;
	MechS32 z;
	MechS32 minY;
	MechS32 minZ;
	MechS32 maxX;

	count = p_face->m_unk0x02;
	vertices = (EmberFern0x2c*) (p_model + 1);
	minX = minY = minZ = 0x7fffffff;
	maxX = maxY = maxZ = -0x7fffffff;
	for (i = 0; i < count; i++) {
		vertex = &vertices[((MechU8*) p_face)[p_face->m_unk0x04 + i]];
		x = vertex->m_unk0x0c;
		y = vertex->m_unk0x10;
		z = vertex->m_unk0x14;
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

// STUB: MW2 0x1001e6dc
MechS32 FUN_1001e6dc(AzureThicket0x2c* p_node, GraniteLattice0x18* p_model, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	STUB(0x1001e6dc);
	return 0;
}

// STUB: MW2 0x1001e90f
MechS32 FUN_1001e90f(AzureThicket0x2c* p_node, GraniteLattice0x18* p_model, Ray* p_ray)
{
	STUB(0x1001e90f);
	return 0;
}

// Finds the nearest hit of p_ray among p_node's children and shortens p_ray to it. Whether any
// child was hit.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1001eb25
MechS32 FUN_1001eb25(AzureThicket0x2c* p_node, GraniteLattice0x18* p_model, Ray* p_ray)
{
	MechS32 length;
	MechS32 i;
	Ray best;
	Ray ray;
	MechS32 nearest;

	nearest = 0x7fffffff;
	CopyRay(&ray, p_ray);
	for (i = 0; i < 4; i++) {
		if (FUN_1001e90f(p_node->m_children[i], p_model, &ray)) {
			length = GetRayLength(&ray);
			if (length < nearest) {
				nearest = length;
				CopyRay(&best, &ray);
			}

			CopyRay(&ray, p_ray);
		}
	}

	if (nearest < 0x7fffffff) {
		CopyRay(p_ray, &best);
		return TRUE;
	}

	return FALSE;
}

// STUB: MW2 0x1001ebfa
MechS32 FUN_1001ebfa(
	AzureThicket0x2c* p_node,
	GraniteLattice0x18* p_model,
	MechS32 p_x,
	MechS32 p_y,
	MechS32 p_z,
	MechS32* p_top
)
{
	STUB(0x1001ebfa);
	return 0;
}

// FUNCTION: MW2 0x1001edfa
void FUN_1001edfa(void)
{
	g_unk0x100a37dc = 1;
}

// Returns the bytes a quadtree takes.
// FUNCTION: MW2 0x1001ee0f
MechS32 FUN_1001ee0f(AzureThicket0x2c* p_node)
{
	MechS32 i;
	MechS32 size;

	if (!p_node) {
		return 0;
	}

	size = p_node->m_unk0x18 * sizeof(undefined4) + sizeof(AzureThicket0x2c);
	for (i = 0; i < 4; i++) {
		size += FUN_1001ee0f(p_node->m_children[i]);
	}

	return size;
}
