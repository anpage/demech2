#ifndef BOUNDBOX_H
#define BOUNDBOX_H

#include "decomp.h"
#include "shape.h"
#include "types.h"

// The bounding box of a shape's selected model (Shape::m_collisionData), from the
// transformed vertices. FUN_1006ea90 allocates it inside a larger 0x78-byte block.
// SIZE 0x18
typedef struct BoundBox {
	MechS32 m_minX; // 0x00
	MechS32 m_maxX; // 0x04
	MechS32 m_minY; // 0x08
	MechS32 m_maxY; // 0x0c
	MechS32 m_minZ; // 0x10
	MechS32 m_maxZ; // 0x14
} BoundBox;

// The functions and globals of boundbox.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	BoundBox* CreateBoundBox(void);
	void FUN_1006e9e6(Shape* p_shape);
	BoundBox* FUN_1006ea90(Shape* p_shape);
	void FUN_1006eb02(Shape* p_shape);
	void FUN_1006eb80(
		Shape* p_shape,
		MechS32* p_minX,
		MechS32* p_maxX,
		MechS32* p_minY,
		MechS32* p_maxY,
		MechS32* p_minZ,
		MechS32* p_maxZ
	);
	void FUN_1006ed30(Shape* p_shape);
	MechS32 FUN_1006edc3(Shape* p_shape);

#ifdef __cplusplus
}
#endif

#endif // BOUNDBOX_H
