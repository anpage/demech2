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
#endif

// FUNCTION: MW2 0x1006d680
// FUNCTION: MW2GBL 0x1003c0b0
// FUNCTION: MW2MATROX 0x100035d0
// FUNCTION: MW23DFX 0x1000db10
// FUNCTION: MW2ATI 0x1000e200
// FUNCTION: MW2S3 0x1002a4d0
// FUNCTION: MW2POWERVR 0x1000ce00
void InitShapeLists(void)
{
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
}

#ifdef MW2_SHAPE_GRID
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
// FUNCTION: MW2MATROX 0x10003682
// FUNCTION: MW23DFX 0x1000db80
// FUNCTION: MW2ATI 0x1000e270
// FUNCTION: MW2S3 0x1002a540
// FUNCTION: MW2POWERVR 0x1000ce70
void AddSceneShape(Shape* p_shape)
{
	Shape* list;

	if (g_sceneShapes != (Shape*) &g_sceneShapeHead || !p_shape) {
		return;
	}

	if (p_shape->m_flags & 0x1000) {
		list = (Shape*) &g_hiddenShapeHead;
	}
	else {
		list = (Shape*) &g_sceneShapeHead;
	}

	LinkShape(p_shape, list);
	if (p_shape->m_collisionType == 4) {
		p_shape->m_flags |= 0x800;
	}

	if (!(p_shape->m_flags & 0x800)) {
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
// FUNCTION: MW2MATROX 0x100039f1
// FUNCTION: MW23DFX 0x1000dd30
// FUNCTION: MW2ATI 0x1000e420
// FUNCTION: MW2S3 0x1002a6f0
// FUNCTION: MW2POWERVR 0x1000d030
void ShowShape(Shape* p_shape)
{
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
	LinkShape(p_shape, (Shape*) &g_sceneShapeHead);
#ifdef MW2_POWERVR
	if (FUN_10062960(p_shape)) {
		FUN_1005f450(p_shape->m_list);
	}
#endif
}

// Frees the shapes of all three lists.
// FUNCTION: MW2 0x1006db28
// FUNCTION: MW2GBL 0x1003c559
// FUNCTION: MW2MATROX 0x10003a79
// FUNCTION: MW23DFX 0x1000dd70
// FUNCTION: MW2ATI 0x1000e460
// FUNCTION: MW2S3 0x1002a730
// FUNCTION: MW2POWERVR 0x1000d090
void FreeSceneShapes(void)
{
	Shape* shape;

	if (g_sceneShapes == (Shape*) &g_sceneShapeHead) {
		while (g_sceneShapeHead.m_next) {
			shape = g_sceneShapeHead.m_next;
			RemoveSceneShape(g_sceneShapeHead.m_next);
			FreeShape(shape);
		}

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
// FUNCTION: MW2MATROX 0x10003bce
// FUNCTION: MW23DFX 0x1000de50
// FUNCTION: MW2ATI 0x1000e540
// FUNCTION: MW2S3 0x1002a810
// FUNCTION: MW2POWERVR 0x1000d170
void ShowDensityShapes(MechS32 p_enable)
{
	Shape* shape;
	Shape* next;

	if (p_enable) {
		for (shape = g_hiddenShapeHead.m_next; shape != NULL; shape = next) {
			next = shape->m_next;
			if ((shape->m_kind & 0xf0) == 0xc0) {
				ShowShape(shape);
				EnableShapeCollision(shape);
			}
		}
	}
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
