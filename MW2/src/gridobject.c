#include "gridobject.h"

#include "decomp.h"
#include "eyepoint.h"
#include "object.h"
#include "polydraw.h"
#include "shape.h"
#include "shapegeom.h"
#include "types.h"
#include "vector3.h"
#include "vertex.h"

#include <stdlib.h>

// The grid FUN_1004b130 sets up: its cell size, whether the object is on, has been shown and
// snaps to the nearest cell.

// GLOBAL: MW2 0x100a7114
MechS32 g_unk0x100a7114 = 0;

// GLOBAL: MW2 0x100a7118
MechS32 g_unk0x100a7118 = 0;

// GLOBAL: MW2 0x100a711c
MechS32 g_unk0x100a711c = 0;

// GLOBAL: MW2 0x100a7120
MechS32 g_unk0x100a7120 = 0;

// GLOBAL: MW2 0x100a7124
MechS32 g_unk0x100a7124 = 0;

// GLOBAL: MW2 0x100a7128
SceneObject* g_unk0x100a7128 = NULL;

// The eyepoint's cell and the object's position.

// GLOBAL: MW2 0x100be9e0
static Vector3 g_unk0x100be9e0;

// GLOBAL: MW2 0x100be9f0
static Vector3 g_unk0x100be9f0;

// How far the eyepoint may stray from the object before it snaps to another cell.
// GLOBAL: MW2 0x100be9fc
static MechS32 g_unk0x100be9fc;

// Makes p_obj the object that follows the eyepoint on a grid, a cell at a time: the cells are its
// model's span (with children) or a third of it, in steps of 0x4000, and it is snapped to the
// nearest cell once the eyepoint strays past 9/16 of a cell (with children) or kept on the cell
// under the eyepoint.
// Stack-slot permutation; depth > span compares in the other operand order.
// FUNCTION: MW2 0x1004b130
void FUN_1004b130(SceneObject* p_obj)
{
	MechS32 depth;
	MechS32 minX;
	Model* model;
	MechS32 minZ;
	Vertex* vertex;
	MechS32 span;
	MechS32 count;
	MechS32 maxX;
	MechS32 maxZ;

	if (!p_obj) {
		return;
	}

	if (!p_obj->m_shape) {
		return;
	}

	model = p_obj->m_shape->m_models;
	if (!model) {
		return;
	}

	vertex = (Vertex*) (model + 1);
	minX = minZ = 0x7fffffff;
	maxX = maxZ = -0x7fffffff;
	for (count = model->m_vertexCount; count--; vertex++) {
		if (vertex->m_modelX > maxX) {
			maxX = vertex->m_modelX;
		}

		if (vertex->m_modelZ > maxZ) {
			maxZ = vertex->m_modelZ;
		}

		if (vertex->m_modelX < minX) {
			minX = vertex->m_modelX;
		}

		if (vertex->m_modelZ < minZ) {
			minZ = vertex->m_modelZ;
		}
	}

	span = maxX - minX;
	depth = maxZ - minZ;
	if (depth > span) {
		span = depth;
	}

	if (span <= 0) {
		return;
	}

	if (!p_obj->m_firstChild) {
		g_unk0x100a7114 = (span / 3 / 0x4000 + 1) * 0x4000;
		g_unk0x100a7124 = 0;
	}
	else {
		g_unk0x100a7114 = (span / 0x4000 + 1) * 0x4000;
		g_unk0x100a7124 = 1;
	}

	g_unk0x100be9fc = ((g_unk0x100a7114 >> 3) + g_unk0x100a7114) >> 1;
	g_unk0x100a7128 = p_obj;
	g_unk0x100a7118 = 1;
	g_unk0x100a7120 = 1;
	g_unk0x100a711c = 0;
	g_unk0x100be9f0.m_x = g_unk0x100be9f0.m_y = g_unk0x100be9f0.m_z = 0;
	g_unk0x100be9e0.m_x = g_unk0x100be9e0.m_y = g_unk0x100be9e0.m_z = 0;
	HideObjTree(p_obj);
	DisableObjTreeCollision(p_obj);
}

// Moves the grid object (FUN_1004b130) to the eyepoint's cell when that changes.
// Stack-slot permutation; dz > g_unk0x100be9fc compares in the other operand order.
// FUNCTION: MW2 0x1004b344
void FUN_1004b344(void)
{
	MechS32 dx;
	MechS32 dz;
	MechS32 halfZ;
	MechS32 halfX;
	MechS32 x;
	MechS32 cellZ;
	MechS32 cellX;
	MechS32 z;

	if (g_unk0x100a7120 && g_unk0x100a7118) {
		x = g_eyepoint->m_x;
		z = g_eyepoint->m_z;
		if (g_unk0x100a7124) {
			dx = abs(x - g_unk0x100be9f0.m_x);
			dz = abs(z - g_unk0x100be9f0.m_z);
			if (dx > g_unk0x100be9fc || dz > g_unk0x100be9fc) {
				if (x >= 0) {
					halfX = g_unk0x100a7114 >> 1;
				}
				else {
					halfX = -g_unk0x100a7114 >> 1;
				}

				if (z >= 0) {
					halfZ = g_unk0x100a7114 >> 1;
				}
				else {
					halfZ = -g_unk0x100a7114 >> 1;
				}

				cellX = (x + halfX) / g_unk0x100a7114;
				cellZ = (z + halfZ) / g_unk0x100a7114;
			}
			else {
				cellX = g_unk0x100be9e0.m_x;
				cellZ = g_unk0x100be9e0.m_z;
			}
		}
		else {
			cellX = x / g_unk0x100a7114;
			cellZ = z / g_unk0x100a7114;
		}

		if (!g_unk0x100a711c) {
			ShowObjTree(g_unk0x100a7128);
			g_unk0x100a711c = 1;
		}

		if (g_unk0x100be9e0.m_x != cellX || g_unk0x100be9e0.m_z != cellZ) {
			g_unk0x100be9f0.m_x = cellX * g_unk0x100a7114;
			g_unk0x100be9f0.m_y = 0;
			g_unk0x100be9f0.m_z = cellZ * g_unk0x100a7114;
			if (g_unk0x100a7128) {
				ShowObjTree(g_unk0x100a7128);
				SetObjPosition(g_unk0x100a7128, g_unk0x100be9f0.m_x, g_unk0x100be9f0.m_y, g_unk0x100be9f0.m_z);
				UpdateObj(g_unk0x100a7128);
			}
		}

		g_unk0x100be9e0.m_x = cellX;
		g_unk0x100be9e0.m_y = 0;
		g_unk0x100be9e0.m_z = cellZ;
	}
}

// FUNCTION: MW2 0x1004b539
void FUN_1004b539(MechS32 p_enable)
{
	if (g_unk0x100a7128) {
		if (p_enable) {
			ShowObjTree(g_unk0x100a7128);
		}
		else {
			HideObjTree(g_unk0x100a7128);
		}

		UpdateObj(g_unk0x100a7128);
		g_unk0x100a7120 = p_enable;
	}
}
