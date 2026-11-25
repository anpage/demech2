#ifndef SHAPELISTS_H
#define SHAPELISTS_H

#include "shape.h"
#include "shapelisthead.h"
#include "types.h"

#ifdef MW2_SHAPE_GRID
// The early Mercenaries build's grid of shapes: 12 by 12 cells of 25000 units, the first cell's
// corner at -150000 in x and z. A shape outside it is in the off-grid list, its cell 9999.
#define SHAPE_GRID_SIZE 12
#define SHAPE_GRID_CELL 25000
#define SHAPE_GRID_ORIGIN 150000
#define SHAPE_GRID_NONE 9999

struct Eyepoint;

// A cell of the grid: a list head like ShapeListHead, whose last word points back at the cell
// (InitShapeLists).
// SIZE 0x18
typedef struct ShapeGridCell {
	MechU16 m_flags;              // 0x00
	MechU16 m_kind;               // 0x02
	struct Shape* m_prev;         // 0x04
	struct Shape* m_next;         // 0x08 — the first shape in the cell
	struct Shape* m_prevCollider; // 0x0c
	struct Shape* m_nextCollider; // 0x10 — the first shape in the cell's collision list
	struct Shape* m_list;         // 0x14 — the cell itself, as a list
} ShapeGridCell;
#endif

// The functions and globals of shapelists.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern Shape* g_sceneShapes;
	extern Shape* g_hiddenShapes;

	extern ShapeListHead g_sceneShapeHead;
	extern ShapeListHead g_hiddenShapeHead;
	extern ShapeListHead g_detachedShapeHead;
#ifdef MW2_SHAPE_GRID
	extern MechS32 g_shapeGridEnabled;
	extern Shape* g_offGridShapes;
	extern ShapeListHead g_offGridShapeHead;
	extern ShapeGridCell g_shapeGrid[SHAPE_GRID_SIZE][SHAPE_GRID_SIZE];
	extern Shape* g_shapeGridLists[SHAPE_GRID_SIZE * SHAPE_GRID_SIZE + 2];
#endif

#ifdef MW2_SHAPE_GRID
	void EnableShapeGrid(void);
	void DisableShapeGrid(void);
	void InitShapeGridNeighbours(void);
	void ListAllShapeGridCells(void);
	void ListShapeGridCellsAhead(struct Eyepoint* p_eyepoint);
	void ListShapeGridCellsAround(MechS32 p_x, MechS32 p_z);
#endif
	void InitShapeLists(void);
#ifdef MW2_SHAPE_GRID
	Shape* GetShapeGridList(MechS32 p_x, MechS32 p_z);
	Shape* GetShapeGridHead(Shape* p_shape);
	void MoveObjTreeOffGrid(struct SceneObject* p_obj);
	void UpdateShapeGridCell(Shape* p_shape);
	void MoveObjTreeToCell(struct SceneObject* p_obj, MechS32 p_x, MechS32 p_z);
#endif
	void AddSceneShape(Shape* p_shape);
	void RemoveSceneShape(Shape* p_shape);
	void DetachShape(Shape* p_shape);
	void EnableShapeCollision(Shape* p_shape);
	void DisableShapeCollision(Shape* p_shape);
	void HideShape(Shape* p_shape);
	void ShowShape(Shape* p_shape);
	void FreeSceneShapes(void);
	void UnlinkShape(Shape* p_shape);
	void LinkShape(Shape* p_shape, Shape* p_list);
	void ShowDensityShapes(MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // SHAPELISTS_H
