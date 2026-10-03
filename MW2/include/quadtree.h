#ifndef QUADTREE_H
#define QUADTREE_H

#include "decomp.h"
#include "ray.h"
#include "types.h"

struct Face;
struct Shape;
#include "shapegeom.h"

// A quadtree node: its bounds, its four children (m_unk0x18 == 0) and m_unk0x18 entries
// after the header (undefined4 each). FUN_1001e429 allocates it; Shape::m_unk0x44 holds the root
// when the shape's m_unk0x24 is 5.
// SIZE 0x2c
typedef struct QuadtreeNode {
	MechS32 m_unk0x00;                  // 0x00
	MechS32 m_unk0x04;                  // 0x04
	MechS32 m_unk0x08;                  // 0x08
	MechS32 m_unk0x0c;                  // 0x0c
	MechS32 m_unk0x10;                  // 0x10
	MechS32 m_unk0x14;                  // 0x14
	MechS32 m_unk0x18;                  // 0x18
	struct QuadtreeNode* m_children[4]; // 0x1c
} QuadtreeNode;

// The functions and globals of quadtree.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1001df00(struct Shape* p_shape);
	QuadtreeNode* FUN_1001e0a7(QuadtreeNode* p_node, MechS32 p_quadrant, Model* p_model);
	QuadtreeNode* FUN_1001e429(
		undefined4 p_unk0x00,
		undefined4 p_unk0x04,
		undefined4 p_unk0x08,
		undefined4 p_unk0x0c,
		undefined4 p_unk0x10,
		undefined4 p_unk0x14,
		MechS32 p_unk0x18
	);
	void FUN_1001e50d(QuadtreeNode* p_node);
	MechS32 FUN_1001e57a(
		struct Face* p_face,
		Model* p_model,
		MechS32 p_minX,
		MechS32 p_maxX,
		MechS32 p_minZ,
		MechS32 p_maxZ,
		MechS32* p_minY,
		MechS32* p_maxY
	);
	MechS32 FUN_1001e6dc(QuadtreeNode* p_node, Model* p_model, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_1001e90f(QuadtreeNode* p_node, Model* p_model, Ray* p_ray);
	MechS32 FUN_1001eb25(QuadtreeNode* p_node, Model* p_model, Ray* p_ray);
	MechS32 FUN_1001ebfa(QuadtreeNode* p_node, Model* p_model, MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32* p_top);
	void FUN_1001edfa(void);
	MechS32 FUN_1001ee0f(QuadtreeNode* p_node);

#ifdef __cplusplus
}
#endif

#endif // QUADTREE_H
