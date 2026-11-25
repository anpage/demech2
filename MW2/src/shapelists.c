#include "shapelists.h"

#include "decomp.h"
#include "shape.h"
#include "shapelisthead.h"
#include "types.h"

#include <stddef.h>

#ifdef MW2_POWERVR
#include "accel/a3d.h"
#endif

#ifdef MW2_SHAPE_GRID
#include "eyepoint.h"
#include "object.h"
#endif

DECOMP_SIZE_ASSERT(ShapeListHead, 0x18)
#ifdef MW2_SHAPE_GRID
DECOMP_SIZE_ASSERT(ShapeGridCell, 0x18)

// Whether the shapes are filed in the grid. Nothing in the early Mercenaries build turns it on.
// GLOBAL: MW2PREMERCS 0x100b0b98
MechS32 g_shapeGridEnabled = 0;

// The off-grid list, as g_sceneShapes is the scene's (InitShapeLists).
// GLOBAL: MW2PREMERCS 0x100b0b9c
Shape* g_offGridShapes = NULL;

// The rings of cells ListShapeGridCellsAhead takes around the eyepoint's cell, as x and z offsets,
// the farthest first: the corners two cells away, the cells two away
// beside them, those between, and the four next to it.
// GLOBAL: MW2PREMERCS 0x100b0ba0
MechS32 g_shapeGridRing0[4][2] = {{-2, 2}, {2, 2}, {2, -2}, {-2, -2}};

// GLOBAL: MW2PREMERCS 0x100b0bc0
MechS32 g_shapeGridRing1[8][2] = {{-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}};

// GLOBAL: MW2PREMERCS 0x100b0c00
MechS32 g_shapeGridRing2[8][2] = {{-2, 0}, {-1, 1}, {0, 2}, {1, 1}, {2, 0}, {1, -1}, {0, -2}, {-1, -1}};

// GLOBAL: MW2PREMERCS 0x100b0c40
MechS32 g_shapeGridRing3[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

// GLOBAL: MW2PREMERCS 0x100b0c60
MechS32 g_shapeGridRingSizes[4] = {4, 8, 8, 4};
#endif

// The shape lists, each a ShapeListHead posing as a shape: the scene's shapes (drawn; its head
// also starts the collision list), the hidden ones (flag 0x1000: counted, not drawn), and the
// detached ones, drawn on their own (the sky objects SecondRender finds).
// GLOBAL: MW2 0x100ad5e8
// GLOBAL: MW2GBL 0x100adba0
// GLOBAL: MW2PREMERCS 0x100b0c70
// GLOBAL: MW2MATROX 0x100a20e8
// GLOBAL: MW23DFX 0x1024b320
// GLOBAL: MW2ATI 0x1017f6a0
// GLOBAL: MW2S3 0x10066a34
// GLOBAL: MW2POWERVR 0x101aa414
Shape* g_sceneShapes = NULL;

// GLOBAL: MW2 0x100ad5ec
// GLOBAL: MW2GBL 0x100adba4
// GLOBAL: MW2PREMERCS 0x100b0c74
// GLOBAL: MW2MATROX 0x100a20ec
// GLOBAL: MW23DFX 0x1024b324
// GLOBAL: MW2ATI 0x1017f6a4
// GLOBAL: MW2S3 0x10066a38
// GLOBAL: MW2POWERVR 0x101aa418
Shape* g_hiddenShapes = NULL;

// GLOBAL: MW2 0x100bef10
// GLOBAL: MW2GBL 0x100b9d18
// GLOBAL: MW2PREMERCS 0x100ca070
// GLOBAL: MW2MATROX 0x100c1c68
// GLOBAL: MW23DFX 0x1007c2c8
// GLOBAL: MW2ATI 0x100712c8
// GLOBAL: MW2S3 0x1007cb28
// GLOBAL: MW2POWERVR 0x1006f2c8
ShapeListHead g_sceneShapeHead;

// GLOBAL: MW2 0x100bef28
// GLOBAL: MW2GBL 0x100b9d30
// GLOBAL: MW2PREMERCS 0x100ca0a0
// GLOBAL: MW2MATROX 0x100c1c50
// GLOBAL: MW23DFX 0x1007c2b0
// GLOBAL: MW2ATI 0x100712b0
// GLOBAL: MW2S3 0x1007cb40
// GLOBAL: MW2POWERVR 0x1006f2b0
ShapeListHead g_hiddenShapeHead;

// GLOBAL: MW2 0x100bef40
// GLOBAL: MW2GBL 0x100b9d48
// GLOBAL: MW2PREMERCS 0x100cae38
// GLOBAL: MW2MATROX 0x100c1c80
// GLOBAL: MW23DFX 0x1007c2e0
// GLOBAL: MW2ATI 0x100712e0
// GLOBAL: MW2S3 0x1007cb58
// GLOBAL: MW2POWERVR 0x1006f2e0
ShapeListHead g_detachedShapeHead;

#ifdef MW2_SHAPE_GRID
// The shapes outside the grid, while it is on.
// GLOBAL: MW2PREMERCS 0x100ca088
ShapeListHead g_offGridShapeHead;

// GLOBAL: MW2PREMERCS 0x100ca0b8
ShapeGridCell g_shapeGrid[SHAPE_GRID_SIZE][SHAPE_GRID_SIZE];

// The four cells ListShapeGridCellsAround takes for each quadrant of a cell, as x and z offsets:
// the cell and three of its neighbours (the third quadrant takes the cell on the other side).
// GLOBAL: MW2PREMERCS 0x100cae50
MechS32 g_shapeGridNeighbours[4][4][2];

// The quadrant ListShapeGridCellsAround found.
// GLOBAL: MW2PREMERCS 0x101d67e4
MechS32 g_shapeGridQuadrant;

// The lists of the cells to draw from, the last NULL: the scene's alone while the grid is off.
// GLOBAL: MW2PREMERCS 0x101d67f0
Shape* g_shapeGridLists[SHAPE_GRID_SIZE * SHAPE_GRID_SIZE + 2];

// ListShapeGridCellsAhead's place in the ring it walks.
// GLOBAL: MW2PREMERCS 0x101d6a38
MechS32 g_shapeGridRingStep;

// Where ListShapeGridCellsAhead starts in each ring, by the eyepoint's heading.
// GLOBAL: MW2PREMERCS 0x101d6a40
MechS32 g_shapeGridRingStarts[4];

// The cell a list builder works around.
// GLOBAL: MW2PREMERCS 0x101d6a50
MechS32 g_shapeGridCellX;

// The ring ListShapeGridCellsAhead walks.
// GLOBAL: MW2PREMERCS 0x101d6a54
MechS32 g_shapeGridRing;

// GLOBAL: MW2PREMERCS 0x101d6a58
MechS32 g_shapeGridCellZ;

// ListShapeGridCellsAround's place in its quadrant's neighbours.
// GLOBAL: MW2PREMERCS 0x101d6a5c
MechS32 g_shapeGridNeighbour;

// How many cells of each ring ListShapeGridCellsAhead takes.
// GLOBAL: MW2PREMERCS 0x101d6a60
MechS32 g_shapeGridRingCounts[4];
#endif

#ifdef MW2_SHAPE_GRID
// FUNCTION: MW2PREMERCS 0x1002ea80
void EnableShapeGrid(void)
{
	g_shapeGridEnabled = TRUE;
}

// FUNCTION: MW2PREMERCS 0x1002ea95
void DisableShapeGrid(void)
{
	g_shapeGridEnabled = FALSE;
}

// FUNCTION: MW2PREMERCS 0x1002eaaa
void InitShapeGridNeighbours(void)
{
	g_shapeGridNeighbours[0][0][0] = 0;
	g_shapeGridNeighbours[0][0][1] = 0;
	g_shapeGridNeighbours[0][1][0] = -1;
	g_shapeGridNeighbours[0][1][1] = 0;
	g_shapeGridNeighbours[0][2][0] = 0;
	g_shapeGridNeighbours[0][2][1] = -1;
	g_shapeGridNeighbours[0][3][0] = -1;
	g_shapeGridNeighbours[0][3][1] = -1;
	g_shapeGridNeighbours[1][0][0] = 0;
	g_shapeGridNeighbours[1][0][1] = 0;
	g_shapeGridNeighbours[1][1][0] = -1;
	g_shapeGridNeighbours[1][1][1] = 0;
	g_shapeGridNeighbours[1][2][0] = 0;
	g_shapeGridNeighbours[1][2][1] = 1;
	g_shapeGridNeighbours[1][3][0] = -1;
	g_shapeGridNeighbours[1][3][1] = 1;
	g_shapeGridNeighbours[2][0][0] = 0;
	g_shapeGridNeighbours[2][0][1] = 0;
	g_shapeGridNeighbours[2][1][0] = 1;
	g_shapeGridNeighbours[2][1][1] = 0;
	g_shapeGridNeighbours[2][2][0] = -1;
	g_shapeGridNeighbours[2][2][1] = 0;
	g_shapeGridNeighbours[2][3][0] = 1;
	g_shapeGridNeighbours[2][3][1] = -1;
	g_shapeGridNeighbours[3][0][0] = 0;
	g_shapeGridNeighbours[3][0][1] = 0;
	g_shapeGridNeighbours[3][1][0] = 0;
	g_shapeGridNeighbours[3][1][1] = 1;
	g_shapeGridNeighbours[3][2][0] = 1;
	g_shapeGridNeighbours[3][2][1] = 0;
	g_shapeGridNeighbours[3][3][0] = 1;
	g_shapeGridNeighbours[3][3][1] = 1;
}

// Lists every cell that has shapes, the off-grid list first.
// FUNCTION: MW2PREMERCS 0x1002ebf5
void ListAllShapeGridCells(void)
{
	Shape* list;
	MechS32 x;
	MechS32 z;
	MechS32 count;

	count = 0;
	if (!g_shapeGridEnabled) {
		g_shapeGridLists[0] = g_sceneShapes;
		g_shapeGridLists[1] = NULL;
		return;
	}

	list = g_offGridShapes;
	if (list && list->m_next) {
		g_shapeGridLists[count] = list;
		g_shapeGridLists[count + 1] = NULL;
		count++;
	}

	for (x = 0; x < SHAPE_GRID_SIZE; x++) {
		for (z = 0; z < SHAPE_GRID_SIZE; z++) {
			list = g_shapeGrid[x][z].m_list;
			if (list && list->m_next) {
				g_shapeGridLists[count] = list;
				g_shapeGridLists[count + 1] = NULL;
				count++;
			}
		}
	}

	g_shapeGridLists[count] = NULL;
}

// Lists the cells that have shapes around the eyepoint's, the farthest first, starting each ring
// by the eyepoint's heading (in eighths of a turn), then the eyepoint's own.
// FUNCTION: MW2PREMERCS 0x1002ed05
void ListShapeGridCellsAhead(Eyepoint* p_eyepoint)
{
	MechS32 octant;
	MechS32 heading;
	MechS32 x;
	MechS32 z;
	MechS32 count;

	count = 0;
	if (!g_shapeGridEnabled || !p_eyepoint) {
		g_shapeGridLists[0] = g_sceneShapes;
		g_shapeGridLists[1] = NULL;
		return;
	}

	heading = (p_eyepoint->m_heading >> 16) % 360;
	if (heading < 0) {
		heading += 360;
	}

	heading = (heading + 22) % 360;
	octant = heading / 45;
	if (octant % 2 == 0) {
		g_shapeGridRingCounts[0] = 2;
		g_shapeGridRingCounts[1] = 4;
		g_shapeGridRingCounts[2] = 3;
		g_shapeGridRingCounts[3] = 3;
	}
	else {
		g_shapeGridRingCounts[0] = 2;
		g_shapeGridRingCounts[1] = 4;
		g_shapeGridRingCounts[2] = 5;
		g_shapeGridRingCounts[3] = 2;
	}

	switch (octant) {
	default:
		g_shapeGridRingStarts[0] = g_shapeGridRingStarts[3] = 0;
		g_shapeGridRingStarts[1] = g_shapeGridRingStarts[2] = 7;
		break;
	case 0:
		g_shapeGridRingStarts[0] = g_shapeGridRingStarts[3] = 0;
		g_shapeGridRingStarts[1] = 0;
		g_shapeGridRingStarts[2] = 1;
		break;
	case 1:
		g_shapeGridRingStarts[0] = 1;
		g_shapeGridRingStarts[1] = 1;
		g_shapeGridRingStarts[2] = 1;
		g_shapeGridRingStarts[3] = 1;
		break;
	case 2:
		g_shapeGridRingStarts[0] = 1;
		g_shapeGridRingStarts[1] = 2;
		g_shapeGridRingStarts[2] = 3;
		g_shapeGridRingStarts[3] = 1;
		break;
	case 3:
		g_shapeGridRingStarts[0] = 2;
		g_shapeGridRingStarts[1] = 3;
		g_shapeGridRingStarts[2] = 3;
		g_shapeGridRingStarts[3] = 2;
		break;
	case 4:
		g_shapeGridRingStarts[0] = 2;
		g_shapeGridRingStarts[1] = 4;
		g_shapeGridRingStarts[2] = 5;
		g_shapeGridRingStarts[3] = 2;
		break;
	case 5:
		g_shapeGridRingStarts[0] = 3;
		g_shapeGridRingStarts[1] = 5;
		g_shapeGridRingStarts[2] = 5;
		g_shapeGridRingStarts[3] = 3;
		break;
	case 6:
		g_shapeGridRingStarts[0] = 3;
		g_shapeGridRingStarts[1] = 6;
		g_shapeGridRingStarts[2] = 7;
		g_shapeGridRingStarts[3] = 3;
		break;
	}

	g_shapeGridCellX = (p_eyepoint->m_x + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
	g_shapeGridCellZ = (p_eyepoint->m_z + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
	g_shapeGridRing = 0;
	g_shapeGridRingStep = 0;
	while (g_shapeGridRing < 4) {
		switch (g_shapeGridRing) {
		default:
			break;
		case 0:
			x = g_shapeGridRing0[(g_shapeGridRingStarts[0] + g_shapeGridRingStep) % g_shapeGridRingSizes[0]][0] +
				g_shapeGridCellX;
			z = g_shapeGridRing0[(g_shapeGridRingStarts[0] + g_shapeGridRingStep) % g_shapeGridRingSizes[0]][1] +
				g_shapeGridCellZ;
			break;
		case 1:
			x = g_shapeGridRing1[(g_shapeGridRingStarts[1] + g_shapeGridRingStep) % g_shapeGridRingSizes[1]][0] +
				g_shapeGridCellX;
			z = g_shapeGridRing1[(g_shapeGridRingStarts[1] + g_shapeGridRingStep) % g_shapeGridRingSizes[1]][1] +
				g_shapeGridCellZ;
			break;
		case 2:
			x = g_shapeGridRing2[(g_shapeGridRingStarts[2] + g_shapeGridRingStep) % g_shapeGridRingSizes[2]][0] +
				g_shapeGridCellX;
			z = g_shapeGridRing2[(g_shapeGridRingStarts[2] + g_shapeGridRingStep) % g_shapeGridRingSizes[2]][1] +
				g_shapeGridCellZ;
			break;
		case 3:
			x = g_shapeGridRing3[(g_shapeGridRingStarts[3] + g_shapeGridRingStep) % g_shapeGridRingSizes[3]][0] +
				g_shapeGridCellX;
			z = g_shapeGridRing3[(g_shapeGridRingStarts[3] + g_shapeGridRingStep) % g_shapeGridRingSizes[3]][1] +
				g_shapeGridCellZ;
			break;
		}

		g_shapeGridRingStep++;
		if (g_shapeGridRingStep >= g_shapeGridRingCounts[g_shapeGridRing]) {
			g_shapeGridRingStep = 0;
			g_shapeGridRing++;
		}

		if (x >= 0 && x < SHAPE_GRID_SIZE && z >= 0 && z < SHAPE_GRID_SIZE && g_shapeGrid[x][z].m_list->m_next) {
			g_shapeGridLists[count] = g_shapeGrid[x][z].m_list;
			g_shapeGridLists[count + 1] = NULL;
			count++;
		}
	}

	if (g_shapeGrid[g_shapeGridCellX][g_shapeGridCellZ].m_list->m_next) {
		g_shapeGridLists[count] = g_shapeGrid[g_shapeGridCellX][g_shapeGridCellZ].m_list;
		g_shapeGridLists[count + 1] = NULL;
	}
}

// Lists the cells that have shapes among the point's and three of its neighbours, by the
// quadrant it is in (g_shapeGridNeighbours). The quadrant is taken from the position modulo 12,
// not from the cell.
// FUNCTION: MW2PREMERCS 0x1002f253
void ListShapeGridCellsAround(MechS32 p_x, MechS32 p_z)
{
	MechS32 x;
	MechS32 z;
	MechS32 quadrantX;
	MechS32 quadrantZ;
	MechS32 count;

	count = 0;
	if (!g_shapeGridEnabled) {
		g_shapeGridLists[0] = g_sceneShapes;
		g_shapeGridLists[1] = NULL;
		return;
	}

	g_shapeGridCellX = (p_x + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
	g_shapeGridCellZ = (p_z + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
	quadrantX = (p_x + SHAPE_GRID_ORIGIN) % 12 / 6;
	quadrantZ = (p_z + SHAPE_GRID_ORIGIN) % 12 / 6;
	g_shapeGridQuadrant = quadrantX * 2 | quadrantZ;
	g_shapeGridNeighbour = 0;
	if (!g_shapeGridEnabled) {
		g_shapeGridLists[0] = g_sceneShapes;
		g_shapeGridLists[1] = NULL;
		return;
	}

	while (g_shapeGridNeighbour < 4) {
		x = g_shapeGridNeighbours[g_shapeGridQuadrant][g_shapeGridNeighbour][0] + g_shapeGridCellX;
		z = g_shapeGridNeighbours[g_shapeGridQuadrant][g_shapeGridNeighbour][1] + g_shapeGridCellZ;
		g_shapeGridNeighbour++;
		if (x > -1 && x < SHAPE_GRID_SIZE && z > -1 && z < SHAPE_GRID_SIZE && g_shapeGrid[x][z].m_list->m_next) {
			g_shapeGridLists[count] = g_shapeGrid[x][z].m_list;
			g_shapeGridLists[count + 1] = NULL;
			count++;
		}
	}
}
#endif

// FUNCTION: MW2 0x1006d680
// FUNCTION: MW2GBL 0x1003c0b0
// FUNCTION: MW2PREMERCS 0x1002f3fc
// FUNCTION: MW2MATROX 0x100035d0
// FUNCTION: MW23DFX 0x1000db10
// FUNCTION: MW2ATI 0x1000e200
// FUNCTION: MW2S3 0x1002a4d0
// FUNCTION: MW2POWERVR 0x1000ce00
void InitShapeLists(void)
{
#ifdef MW2_SHAPE_GRID
	MechS32 i;
	MechS32 j;

	g_shapeGridCellX = g_shapeGridCellZ = 0;
	InitShapeGridNeighbours();
	for (i = 0; i < 25; i++) {
		g_shapeGridLists[i] = NULL;
	}

	for (i = 0; i < SHAPE_GRID_SIZE; i++) {
		for (j = 0; j < SHAPE_GRID_SIZE; j++) {
			g_shapeGrid[i][j].m_prev = g_shapeGrid[i][j].m_prevCollider = NULL;
			g_shapeGrid[i][j].m_next = g_shapeGrid[i][j].m_nextCollider = NULL;
			g_shapeGrid[i][j].m_flags = 0x4000;
			g_shapeGrid[i][j].m_list = (Shape*) &g_shapeGrid[i][j];
		}
	}

	g_offGridShapeHead.m_prev = g_offGridShapeHead.m_prevCollider = NULL;
	g_offGridShapeHead.m_next = g_offGridShapeHead.m_nextCollider = NULL;
	g_offGridShapeHead.m_flags = 0x4000;
#endif
	g_sceneShapeHead.m_prev = g_sceneShapeHead.m_prevCollider = NULL;
	g_sceneShapeHead.m_next = g_sceneShapeHead.m_nextCollider = NULL;
	g_sceneShapeHead.m_flags = 0x4000;
	g_hiddenShapeHead.m_prev = g_hiddenShapeHead.m_prevCollider = NULL;
	g_hiddenShapeHead.m_next = g_hiddenShapeHead.m_nextCollider = NULL;
	g_hiddenShapeHead.m_flags = 0x4000;
	g_detachedShapeHead.m_prev = g_detachedShapeHead.m_prevCollider = NULL;
	g_detachedShapeHead.m_next = g_detachedShapeHead.m_nextCollider = NULL;
	g_detachedShapeHead.m_flags = 0x4000;
	g_sceneShapes = (Shape*) &g_sceneShapeHead;
	g_hiddenShapes = (Shape*) &g_hiddenShapeHead;
#ifdef MW2_SHAPE_GRID
	g_offGridShapes = (Shape*) &g_offGridShapeHead;
#endif
}

#ifdef MW2_SHAPE_GRID
// The list of the cell the point is in, or the off-grid list.
// FUNCTION: MW2PREMERCS 0x1002f63f
Shape* GetShapeGridList(MechS32 p_x, MechS32 p_z)
{
	p_x = (p_x + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
	p_z = (p_z + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
	if (p_x >= 0 && p_x < SHAPE_GRID_SIZE && p_z >= 0 && p_z < SHAPE_GRID_SIZE) {
		return g_shapeGrid[p_x][p_z].m_list;
	}
	else {
		return g_offGridShapes;
	}
}

// The list a shape joins: the hidden list, or the cell its object stands in (kept in the shape),
// or the off-grid list.
// FUNCTION: MW2PREMERCS 0x1002f6c2
Shape* GetShapeGridHead(Shape* p_shape)
{
	MechS32 z;
	Shape* list;
	MechS32 x;
	MechS32 cellX;
	MechS32 cellZ;
	MechS32 y;

	if (p_shape->m_flags & 0x1000) {
		list = (Shape*) &g_hiddenShapeHead;
	}
	else {
		GetObjPosition(p_shape->m_object, &x, &y, &z);
		cellX = (x + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
		cellZ = (z + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
		p_shape->m_gridX = cellX;
		p_shape->m_gridZ = cellZ;
		if (cellX < 0 || cellX >= SHAPE_GRID_SIZE || cellZ < 0 || cellZ >= SHAPE_GRID_SIZE) {
			list = (Shape*) &g_offGridShapeHead;
		}
		else {
			list = (Shape*) &g_shapeGrid[cellX][cellZ];
		}
	}

	return list;
}

// Moves the object's shape and its children's to the off-grid list.
// FUNCTION: MW2PREMERCS 0x1002f796
void MoveObjTreeOffGrid(SceneObject* p_obj)
{
	SceneObject* child;

	if (!p_obj || !g_shapeGridEnabled) {
		return;
	}

	UnlinkShape(p_obj->m_shape);
	LinkShape(p_obj->m_shape, (Shape*) &g_offGridShapeHead);
	p_obj->m_shape->m_gridX = SHAPE_GRID_NONE;
	p_obj->m_shape->m_gridZ = SHAPE_GRID_NONE;
	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		MoveObjTreeOffGrid(child);
	}
}

// Files a visible shape in the cell its object stands in, with its object's children, when it
// has moved out of the one it was in.
// FUNCTION: MW2PREMERCS 0x1002f82f
void UpdateShapeGridCell(Shape* p_shape)
{
	MechS32 x;
	MechS32 cellX;
	MechS32 cellZ;
	MechS32 y;
	MechS32 z;

	if (!p_shape || !g_shapeGridEnabled || p_shape->m_flags & 0x1000) {
		return;
	}

	GetObjPosition(p_shape->m_object, &x, &y, &z);
	cellX = (x + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
	cellZ = (z + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
	if (p_shape->m_gridX != cellX || p_shape->m_gridZ != cellZ) {
		if (cellX >= 0 && cellX < SHAPE_GRID_SIZE && cellZ >= 0 && cellZ < SHAPE_GRID_SIZE) {
			MoveObjTreeToCell(p_shape->m_object, cellX, cellZ);
		}
		else {
			MoveObjTreeOffGrid(p_shape->m_object);
		}
	}
}

// Moves the object's shape to the cell, and its children's but the hidden ones of type 0x70.
// FUNCTION: MW2PREMERCS 0x1002f91c
void MoveObjTreeToCell(SceneObject* p_obj, MechS32 p_x, MechS32 p_z)
{
	SceneObject* child;

	if (!p_obj || !g_shapeGridEnabled) {
		return;
	}

	if (!p_obj->m_shape) {
		return;
	}

	UnlinkShape(p_obj->m_shape);
	LinkShape(p_obj->m_shape, (Shape*) &g_shapeGrid[p_x][p_z]);
	p_obj->m_shape->m_gridX = p_x;
	p_obj->m_shape->m_gridZ = p_z;
	for (child = p_obj->m_firstChild; child; child = child->m_nextSibling) {
		if (child->m_shape && (!(child->m_shape->m_flags & 0x1000) || (child->m_shape->m_kind & 0xf0) != 0x70)) {
			MoveObjTreeToCell(child, p_x, p_z);
		}
	}
}
#endif

// Links a new shape into the scene, or the hidden list for flag 0x1000, and into the collision
// list unless it has collision type 4 (none) or flag 0x800.
// FUNCTION: MW2 0x1006d732
// FUNCTION: MW2GBL 0x1003c162
// FUNCTION: MW2PREMERCS 0x1002fa19
// FUNCTION: MW2MATROX 0x10003682
// FUNCTION: MW23DFX 0x1000db80
// FUNCTION: MW2ATI 0x1000e270
// FUNCTION: MW2S3 0x1002a540
// FUNCTION: MW2POWERVR 0x1000ce70
void AddSceneShape(Shape* p_shape)
{
	Shape* list;
#ifdef MW2_SHAPE_GRID
	MechS32 cellX;
	MechS32 cellZ;
	MechS32 z;
	MechS32 x;
	MechS32 y;
#endif

	if (g_sceneShapes != (Shape*) &g_sceneShapeHead || !p_shape) {
		return;
	}

#ifdef MW2_SHAPE_GRID
	p_shape->m_gridX = SHAPE_GRID_NONE;
	p_shape->m_gridZ = SHAPE_GRID_NONE;
	if (g_shapeGridEnabled) {
		list = GetShapeGridHead(p_shape);
	}
	else if (p_shape->m_flags & 0x1000) {
		list = (Shape*) &g_hiddenShapeHead;
	}
	else {
		list = (Shape*) &g_sceneShapeHead;
	}
#else
	if (p_shape->m_flags & 0x1000) {
		list = (Shape*) &g_hiddenShapeHead;
	}
	else {
		list = (Shape*) &g_sceneShapeHead;
	}
#endif

	LinkShape(p_shape, list);
	if (p_shape->m_collisionType == 4) {
		p_shape->m_flags |= 0x800;
	}

	if (!(p_shape->m_flags & 0x800)) {
#ifdef MW2_SHAPE_GRID
		if (g_shapeGridEnabled) {
			GetObjPosition(p_shape->m_object, &x, &y, &z);
			cellX = (x + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
			cellZ = (z + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
			if (cellX >= 0 && cellX < SHAPE_GRID_SIZE && cellZ >= 0 && cellZ < SHAPE_GRID_SIZE) {
				if (g_shapeGrid[cellX][cellZ].m_nextCollider) {
					g_shapeGrid[cellX][cellZ].m_nextCollider->m_prevCollider = p_shape;
				}

				p_shape->m_nextCollider = g_shapeGrid[cellX][cellZ].m_nextCollider;
				g_shapeGrid[cellX][cellZ].m_nextCollider = p_shape;
				p_shape->m_prevCollider = (Shape*) &g_shapeGrid[cellX][cellZ];
			}
			else {
				if (g_offGridShapes->m_nextCollider) {
					g_offGridShapes->m_nextCollider->m_prevCollider = p_shape;
				}

				p_shape->m_nextCollider = g_offGridShapes->m_nextCollider;
				g_offGridShapes->m_nextCollider = p_shape;
				p_shape->m_prevCollider = g_offGridShapes;
			}

			return;
		}
#endif
		if (g_sceneShapeHead.m_nextCollider) {
			g_sceneShapeHead.m_nextCollider->m_prevCollider = p_shape;
		}

		p_shape->m_nextCollider = g_sceneShapeHead.m_nextCollider;
		g_sceneShapeHead.m_nextCollider = p_shape;
		p_shape->m_prevCollider = (Shape*) &g_sceneShapeHead;
	}
}

// FUNCTION: MW2 0x1006d7fb
// FUNCTION: MW2GBL 0x1003c22b
// FUNCTION: MW2PREMERCS 0x1002fc65
// FUNCTION: MW2MATROX 0x1000374b
// FUNCTION: MW23DFX 0x1000dbe0
// FUNCTION: MW2ATI 0x1000e2d0
// FUNCTION: MW2S3 0x1002a5a0
// FUNCTION: MW2POWERVR 0x1000ced0
void RemoveSceneShape(Shape* p_shape)
{
	if (!p_shape || p_shape == g_sceneShapes || p_shape == (Shape*) &g_hiddenShapeHead) {
		return;
	}

	UnlinkShape(p_shape);
	if (p_shape->m_nextCollider) {
		p_shape->m_nextCollider->m_prevCollider = p_shape->m_prevCollider;
	}

	if (p_shape->m_prevCollider) {
		p_shape->m_prevCollider->m_nextCollider = p_shape->m_nextCollider;
	}

	p_shape->m_nextCollider = p_shape->m_prevCollider = NULL;
}

// FUNCTION: MW2 0x1006d88a
// FUNCTION: MW2GBL 0x1003c2bb
// FUNCTION: MW2PREMERCS 0x1002fcf4
// FUNCTION: MW2MATROX 0x100037db
// FUNCTION: MW23DFX 0x1000dc30
// FUNCTION: MW2ATI 0x1000e320
// FUNCTION: MW2S3 0x1002a5f0
// FUNCTION: MW2POWERVR 0x1000cf20
void DetachShape(Shape* p_shape)
{
	if (g_sceneShapes != (Shape*) &g_sceneShapeHead || !p_shape) {
		return;
	}

	UnlinkShape(p_shape);
	LinkShape(p_shape, (Shape*) &g_detachedShapeHead);
}

// FUNCTION: MW2 0x1006d8d1
// FUNCTION: MW2GBL 0x1003c302
// FUNCTION: MW2PREMERCS 0x1002fd3b
// FUNCTION: MW2MATROX 0x10003822
// FUNCTION: MW23DFX 0x1000dc60
// FUNCTION: MW2ATI 0x1000e350
// FUNCTION: MW2S3 0x1002a620
// FUNCTION: MW2POWERVR 0x1000cf50
void EnableShapeCollision(Shape* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (!(p_shape->m_flags & 0x800) || p_shape->m_collisionType == 4) {
		return;
	}

	p_shape->m_flags &= ~0x800;
	if (!p_shape->m_prev) {
		return;
	}

	if (!g_sceneShapes) {
		return;
	}

#ifdef MW2_SHAPE_GRID
	if (g_shapeGridEnabled) {
		if (g_shapeGrid[p_shape->m_gridX][p_shape->m_gridZ].m_list->m_nextCollider) {
			g_shapeGrid[p_shape->m_gridX][p_shape->m_gridZ].m_list->m_nextCollider->m_prevCollider = p_shape;
		}

		p_shape->m_nextCollider = g_shapeGrid[p_shape->m_gridX][p_shape->m_gridZ].m_list->m_nextCollider;
		g_shapeGrid[p_shape->m_gridX][p_shape->m_gridZ].m_list->m_nextCollider = p_shape;
		p_shape->m_prevCollider = g_shapeGrid[p_shape->m_gridX][p_shape->m_gridZ].m_list;
		return;
	}
#endif

	if (g_sceneShapes->m_nextCollider) {
		g_sceneShapes->m_nextCollider->m_prevCollider = p_shape;
	}

	p_shape->m_nextCollider = g_sceneShapes->m_nextCollider;
	g_sceneShapes->m_nextCollider = p_shape;
	p_shape->m_prevCollider = g_sceneShapes;
}

// FUNCTION: MW2 0x1006d989
// FUNCTION: MW2GBL 0x1003c3ba
// FUNCTION: MW2PREMERCS 0x1002feb9
// FUNCTION: MW2MATROX 0x100038da
// FUNCTION: MW23DFX 0x1000dca0
// FUNCTION: MW2ATI 0x1000e390
// FUNCTION: MW2S3 0x1002a660
// FUNCTION: MW2POWERVR 0x1000cf90
void DisableShapeCollision(Shape* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (p_shape->m_flags & 0x800) {
		return;
	}

	p_shape->m_flags |= 0x800;
	if (!g_sceneShapes) {
		return;
	}

	if (p_shape->m_nextCollider) {
		p_shape->m_nextCollider->m_prevCollider = p_shape->m_prevCollider;
	}

	if (p_shape->m_prevCollider) {
		p_shape->m_prevCollider->m_nextCollider = p_shape->m_nextCollider;
	}

	p_shape->m_nextCollider = p_shape->m_prevCollider = NULL;
}

// FUNCTION: MW2 0x1006da2d
// FUNCTION: MW2GBL 0x1003c45e
// FUNCTION: MW2PREMERCS 0x1002ff5d
// FUNCTION: MW2MATROX 0x1000397e
// FUNCTION: MW23DFX 0x1000dcf0
// FUNCTION: MW2ATI 0x1000e3e0
// FUNCTION: MW2S3 0x1002a6b0
// FUNCTION: MW2POWERVR 0x1000cfe0
void HideShape(Shape* p_shape)
{
	if (!p_shape) {
		return;
	}

	if (p_shape->m_flags & 0x1000) {
		return;
	}

	p_shape->m_flags |= 0x1000;
	if (!p_shape->m_prev) {
		return;
	}

	UnlinkShape(p_shape);
	LinkShape(p_shape, (Shape*) &g_hiddenShapeHead);
#ifdef MW2_POWERVR
	if (FUN_10062960(p_shape)) {
		FUN_1005f3f0(p_shape->m_list);
	}
#endif
}

// FUNCTION: MW2 0x1006daa0
// FUNCTION: MW2GBL 0x1003c4d1
// FUNCTION: MW2PREMERCS 0x1002ffd0
// FUNCTION: MW2MATROX 0x100039f1
// FUNCTION: MW23DFX 0x1000dd30
// FUNCTION: MW2ATI 0x1000e420
// FUNCTION: MW2S3 0x1002a6f0
// FUNCTION: MW2POWERVR 0x1000d030
void ShowShape(Shape* p_shape)
{
#ifdef MW2_SHAPE_GRID
	MechS32 cellX;
	MechS32 cellZ;
	MechS32 z;
	MechS32 x;
	MechS32 y;
#endif

	if (!p_shape) {
		return;
	}

	if (!(p_shape->m_flags & 0x1000) || (p_shape->m_kind & 0xf0) == 0x70) {
		return;
	}

	p_shape->m_flags &= ~0x1000;
	if (!p_shape->m_prev) {
		return;
	}

	UnlinkShape(p_shape);
#ifdef MW2_SHAPE_GRID
	if (g_shapeGridEnabled) {
		GetObjPosition(p_shape->m_object, &x, &y, &z);
		cellX = (x + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
		cellZ = (z + SHAPE_GRID_ORIGIN) / SHAPE_GRID_CELL;
		if (cellX >= 0 && cellX < SHAPE_GRID_SIZE && cellZ >= 0 && cellZ < SHAPE_GRID_SIZE) {
			p_shape->m_gridX = cellX;
			p_shape->m_gridZ = cellZ;
			LinkShape(p_shape, (Shape*) &g_shapeGrid[cellX][cellZ]);
		}
		else {
			p_shape->m_gridX = SHAPE_GRID_NONE;
			p_shape->m_gridZ = SHAPE_GRID_NONE;
			LinkShape(p_shape, (Shape*) &g_offGridShapeHead);
		}
	}
	else {
		LinkShape(p_shape, (Shape*) &g_sceneShapeHead);
	}
#else
	LinkShape(p_shape, (Shape*) &g_sceneShapeHead);
#endif
#ifdef MW2_POWERVR
	if (FUN_10062960(p_shape)) {
		FUN_1005f450(p_shape->m_list);
	}
#endif
}

// Frees the shapes of all three lists.
// FUNCTION: MW2 0x1006db28
// FUNCTION: MW2GBL 0x1003c559
// FUNCTION: MW2PREMERCS 0x10030136
// FUNCTION: MW2MATROX 0x10003a79
// FUNCTION: MW23DFX 0x1000dd70
// FUNCTION: MW2ATI 0x1000e460
// FUNCTION: MW2S3 0x1002a730
// FUNCTION: MW2POWERVR 0x1000d090
void FreeSceneShapes(void)
{
	Shape* shape;
#ifdef MW2_SHAPE_GRID
	MechS32 x;
	MechS32 z;

	if (g_shapeGridEnabled) {
		for (x = 0; x < SHAPE_GRID_SIZE; x++) {
			for (z = 0; z < SHAPE_GRID_SIZE; z++) {
				while (g_shapeGrid[x][z].m_next) {
					shape = g_shapeGrid[x][z].m_next;
					RemoveSceneShape(g_shapeGrid[x][z].m_next);
					FreeShape(shape);
				}
			}
		}

		while (g_offGridShapeHead.m_next) {
			shape = g_offGridShapeHead.m_next;
			RemoveSceneShape(g_offGridShapeHead.m_next);
			FreeShape(shape);
		}
	}
	else if (g_sceneShapes == (Shape*) &g_sceneShapeHead) {
		while (g_sceneShapeHead.m_next) {
			shape = g_sceneShapeHead.m_next;
			RemoveSceneShape(g_sceneShapeHead.m_next);
			FreeShape(shape);
		}
	}

	if (g_sceneShapes == (Shape*) &g_sceneShapeHead) {
#else
	if (g_sceneShapes == (Shape*) &g_sceneShapeHead) {
		while (g_sceneShapeHead.m_next) {
			shape = g_sceneShapeHead.m_next;
			RemoveSceneShape(g_sceneShapeHead.m_next);
			FreeShape(shape);
		}
#endif

		while (g_hiddenShapeHead.m_next) {
			shape = g_hiddenShapeHead.m_next;
			RemoveSceneShape(g_hiddenShapeHead.m_next);
			FreeShape(shape);
		}

		while (g_detachedShapeHead.m_next) {
			shape = g_detachedShapeHead.m_next;
			RemoveSceneShape(g_detachedShapeHead.m_next);
			FreeShape(shape);
		}
	}
}

// Unlinks the shape from its list.
// FUNCTION: MW2 0x1006dbe2
// FUNCTION: MW2GBL 0x1003c613
// FUNCTION: MW2PREMERCS 0x100302e8
// FUNCTION: MW2MATROX 0x10003b33
// FUNCTION: MW23DFX 0x1000de00
// FUNCTION: MW2ATI 0x1000e4f0
// FUNCTION: MW2S3 0x1002a7c0
// FUNCTION: MW2POWERVR 0x1000d120
void UnlinkShape(Shape* p_shape)
{
	if (p_shape->m_next) {
		p_shape->m_next->m_prev = p_shape->m_prev;
	}

	if (p_shape->m_prev) {
		p_shape->m_prev->m_next = p_shape->m_next;
	}

	p_shape->m_next = p_shape->m_prev = NULL;
#ifdef MW2_SHAPE_GRID
	if (g_shapeGridEnabled) {
		p_shape->m_gridX = SHAPE_GRID_NONE;
		p_shape->m_gridZ = SHAPE_GRID_NONE;
	}
#endif
}

// Links the shape in at the front of p_list.
// FUNCTION: MW2 0x1006dc3b
// FUNCTION: MW2GBL 0x1003c66c
// FUNCTION: MW2PREMERCS 0x10030362
// FUNCTION: MW2MATROX 0x10003b8c
// FUNCTION: MW23DFX 0x1000de30
// FUNCTION: MW2ATI 0x1000e520
// FUNCTION: MW2S3 0x1002a7f0
// FUNCTION: MW2POWERVR 0x1000d150
void LinkShape(Shape* p_shape, Shape* p_list)
{
	if (p_list->m_next) {
		p_list->m_next->m_prev = p_shape;
	}

	p_shape->m_next = p_list->m_next;
	p_list->m_next = p_shape;
	p_shape->m_prev = p_list;
}

// Shows the shapes of kind 0xc0 (the object density setting) with their collisions, or hides
// them without.
// FUNCTION: MW2 0x1006dc7d
// FUNCTION: MW2GBL 0x1003c6ae
// FUNCTION: MW2PREMERCS 0x100303a4
// FUNCTION: MW2MATROX 0x10003bce
// FUNCTION: MW23DFX 0x1000de50
// FUNCTION: MW2ATI 0x1000e540
// FUNCTION: MW2S3 0x1002a810
// FUNCTION: MW2POWERVR 0x1000d170
void ShowDensityShapes(MechS32 p_enable)
{
	Shape* shape;
	Shape* next;
#ifdef MW2_SHAPE_GRID
	MechS32 x;
	MechS32 z;
#endif

	if (p_enable) {
		for (shape = g_hiddenShapeHead.m_next; shape != NULL; shape = next) {
			next = shape->m_next;
			if ((shape->m_kind & 0xf0) == 0xc0) {
				ShowShape(shape);
				EnableShapeCollision(shape);
			}
		}
	}
#ifdef MW2_SHAPE_GRID
	else if (g_shapeGridEnabled) {
		for (x = 0; x < SHAPE_GRID_SIZE; x++) {
			for (z = 0; z < SHAPE_GRID_SIZE; z++) {
				for (shape = g_shapeGrid[x][z].m_next; shape != NULL; shape = next) {
					next = shape->m_next;
					if ((shape->m_kind & 0xf0) == 0xc0) {
						HideShape(shape);
						DisableShapeCollision(shape);
					}
				}
			}
		}
	}
#endif
	else {
		for (shape = g_sceneShapeHead.m_next; shape != NULL; shape = next) {
			next = shape->m_next;
			if ((shape->m_kind & 0xf0) == 0xc0) {
				HideShape(shape);
				DisableShapeCollision(shape);
			}
		}
	}
}
