#include "gridobject.h"

#include "decomp.h"
#include "eyepoint.h"
#include "fixedfloat.h"
#include "object.h"
#include "polydraw.h"
#include "shape.h"
#include "shapegeom.h"
#include "types.h"
#include "vector3.h"
#include "vertex.h"

#include <stdlib.h>

// The grid SetGridObject sets up: its cell size, whether the object is on, has been shown and
// snaps to the nearest cell.

// GLOBAL: MW2 0x100a7114
// GLOBAL: MW2MATROX 0x100a594c
MechS32 g_gridCellSize = 0;

// GLOBAL: MW2 0x100a7118
// GLOBAL: MW2MATROX 0x100a5950
MechS32 g_gridObjectSet = 0;

// GLOBAL: MW2 0x100a711c
// GLOBAL: MW2MATROX 0x100a5954
MechS32 g_gridObjectPlaced = 0;

// GLOBAL: MW2 0x100a7120
// GLOBAL: MW2MATROX 0x100a5958
MechS32 g_gridObjectShown = 0;

// GLOBAL: MW2 0x100a7124
// GLOBAL: MW2MATROX 0x100a595c
MechS32 g_gridObjectSnaps = 0;

// GLOBAL: MW2 0x100a7128
// GLOBAL: MW2MATROX 0x100a5960
SceneObject* g_gridObject = NULL;

// The eyepoint's cell and the object's position.

// The Matrox edition keeps only the cells' x and z.
#ifdef MW2_MATROX
typedef struct GridCell {
	MechS32 m_x; // 0x00
	MechS32 m_z; // 0x04
} GridCell;
#else
#define GridCell Vector3
#endif

// GLOBAL: MW2 0x100be9e0
// GLOBAL: MW2MATROX 0x100c1fe8
static GridCell g_gridCell;

// GLOBAL: MW2 0x100be9f0
// GLOBAL: MW2MATROX 0x100c1ff0
static GridCell g_gridAnchor;

// How far the eyepoint may stray from the object before it snaps to another cell.
// GLOBAL: MW2 0x100be9fc
// GLOBAL: MW2MATROX 0x100c1fe0
static MechS32 g_gridSnapDistance;

// Makes p_obj the object that follows the eyepoint on a grid, a cell at a time: the cells are its
// model's span (with children) or a third of it, in steps of 0x4000, and it is snapped to the
// nearest cell once the eyepoint strays past 9/16 of a cell (with children) or kept on the cell
// under the eyepoint.
// Stack-slot permutation; depth > span compares in the other operand order.
// FUNCTION: MW2 0x1004b130
// FUNCTION: MW2MATROX 0x100275e0
void SetGridObject(SceneObject* p_obj)
{
	MechS32 depth;
	MechScalar minX;
	Model* model;
	MechScalar minZ;
	Vertex* vertex;
	MechS32 span;
	MechS32 count;
	MechScalar maxX;
	MechScalar maxZ;

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
	minX = minZ = FIXED_MAX;
	maxX = maxZ = FIXED_MIN;
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
		g_gridCellSize = (span / 3 / 0x4000 + 1) * 0x4000;
		g_gridObjectSnaps = 0;
	}
	else {
		g_gridCellSize = (span / 0x4000 + 1) * 0x4000;
		g_gridObjectSnaps = 1;
	}

	g_gridSnapDistance = ((g_gridCellSize >> 3) + g_gridCellSize) >> 1;
	g_gridObject = p_obj;
	g_gridObjectSet = 1;
	g_gridObjectShown = 1;
	g_gridObjectPlaced = 0;
#ifdef MW2_MATROX
	g_gridAnchor.m_x = g_gridAnchor.m_z = 0;
	g_gridCell.m_x = g_gridCell.m_z = 0;
#else
	g_gridAnchor.m_x = g_gridAnchor.m_y = g_gridAnchor.m_z = 0;
	g_gridCell.m_x = g_gridCell.m_y = g_gridCell.m_z = 0;
#endif
	HideObjTree(p_obj);
	DisableObjTreeCollision(p_obj);
}

// Moves the grid object (SetGridObject) to the eyepoint's cell when that changes.
// Stack-slot permutation; dz > g_gridSnapDistance compares in the other operand order (in the
// Matrox edition, both tests against g_gridSnapDistance, and cellX * g_gridCellSize multiplies in
// the other order).
// FUNCTION: MW2 0x1004b344
// FUNCTION: MW2MATROX 0x100277fe
void UpdateGridObject(void)
{
	MechS32 dx;
	MechS32 dz;
	MechS32 halfZ;
	MechS32 halfX;
	MechS32 x;
	MechS32 cellZ;
	MechS32 cellX;
	MechS32 z;

	if (g_gridObjectShown && g_gridObjectSet) {
		x = g_eyepoint->m_x;
		z = g_eyepoint->m_z;
		if (g_gridObjectSnaps) {
			dx = abs(x - g_gridAnchor.m_x);
			dz = abs(z - g_gridAnchor.m_z);
			if (dx > g_gridSnapDistance || dz > g_gridSnapDistance) {
				if (x >= 0) {
					halfX = g_gridCellSize >> 1;
				}
				else {
					halfX = -g_gridCellSize >> 1;
				}

				if (z >= 0) {
					halfZ = g_gridCellSize >> 1;
				}
				else {
					halfZ = -g_gridCellSize >> 1;
				}

				cellX = (x + halfX) / g_gridCellSize;
				cellZ = (z + halfZ) / g_gridCellSize;
			}
			else {
				cellX = g_gridCell.m_x;
				cellZ = g_gridCell.m_z;
			}
		}
		else {
			cellX = x / g_gridCellSize;
			cellZ = z / g_gridCellSize;
		}

		if (!g_gridObjectPlaced) {
			ShowObjTree(g_gridObject);
			g_gridObjectPlaced = 1;
		}

		if (g_gridCell.m_x != cellX || g_gridCell.m_z != cellZ) {
			g_gridAnchor.m_x = cellX * g_gridCellSize;
#ifndef MW2_MATROX
			g_gridAnchor.m_y = 0;
#endif
			g_gridAnchor.m_z = cellZ * g_gridCellSize;
			if (g_gridObject) {
				ShowObjTree(g_gridObject);
#ifdef MW2_MATROX
				SetObjPosition(g_gridObject, g_gridAnchor.m_x, 0, g_gridAnchor.m_z);
#else
				SetObjPosition(g_gridObject, g_gridAnchor.m_x, g_gridAnchor.m_y, g_gridAnchor.m_z);
#endif
				UpdateObj(g_gridObject);
			}
		}

		g_gridCell.m_x = cellX;
#ifndef MW2_MATROX
		g_gridCell.m_y = 0;
#endif
		g_gridCell.m_z = cellZ;
	}
}

// FUNCTION: MW2 0x1004b539
// FUNCTION: MW2MATROX 0x100279f9
void ShowGridObject(MechS32 p_enable)
{
	if (g_gridObject) {
		if (p_enable) {
			ShowObjTree(g_gridObject);
		}
		else {
			HideObjTree(g_gridObject);
		}

		UpdateObj(g_gridObject);
		g_gridObjectShown = p_enable;
	}
}
