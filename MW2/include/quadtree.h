#ifndef QUADTREE_H
#define QUADTREE_H

#include "decomp.h"
#include "fixedfloat.h"
#include "ray.h"
#include "types.h"

struct Face;
struct Shape;
#include "shapegeom.h"

// A quadtree node: its bounds, its four children (m_unk0x18 == 0) and m_unk0x18 entries
// after the header (undefined4 each). AllocQuadtreeNode allocates it; Shape::m_collisionData holds the root
// when the shape's m_collisionType is 5.
// SIZE 0x2c
typedef struct QuadtreeNode {
	MechScalar m_minX;                  // 0x00
	MechScalar m_maxX;                  // 0x04
	MechScalar m_minY;                  // 0x08
	MechScalar m_maxY;                  // 0x0c
	MechScalar m_minZ;                  // 0x10
	MechScalar m_maxZ;                  // 0x14
	MechS32 m_faceCount;                // 0x18
	struct QuadtreeNode* m_children[4]; // 0x1c
} QuadtreeNode;

// The functions and globals of quadtree.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void BuildShapeQuadtree(struct Shape* p_shape);
	QuadtreeNode* BuildQuadtreeChild(QuadtreeNode* p_node, MechS32 p_quadrant, Model* p_model);
	QuadtreeNode* AllocQuadtreeNode(
		MechScalar p_minX,
		MechScalar p_maxX,
		MechScalar p_minY,
		MechScalar p_maxY,
		MechScalar p_minZ,
		MechScalar p_maxZ,
		MechS32 p_faceCount
	);
	void FreeQuadtree(QuadtreeNode* p_node);
	MechS32 FaceOverlapsBox(
		struct Face* p_face,
		Model* p_model,
		MechScalar p_minX,
		MechScalar p_maxX,
		MechScalar p_minZ,
		MechScalar p_maxZ,
		MechScalar* p_minY,
		MechScalar* p_maxY
	);
	MechS32 ClassifyQuadtreePoint(QuadtreeNode* p_node, Model* p_model, MechScalar p_x, MechScalar p_y, MechScalar p_z);
	MechS32 TestQuadtreeRay(QuadtreeNode* p_node, Model* p_model, Ray* p_ray);
	MechS32 TestQuadtreeChildrenRay(QuadtreeNode* p_node, Model* p_model, Ray* p_ray);
	MechS32 GetQuadtreeTop(
		QuadtreeNode* p_node,
		Model* p_model,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar* p_top
	);
	void DisableQuadtrees(void);
	MechS32 GetQuadtreeSize(QuadtreeNode* p_node);

#ifdef __cplusplus
}
#endif

#endif // QUADTREE_H
