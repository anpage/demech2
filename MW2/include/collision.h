#ifndef COLLISION_H
#define COLLISION_H

#include "ray.h"
#include "shape.h"
#include "types.h"

struct Face;
struct Vertex;

// A shape type's collision tests: a point, a ray, and the height of the surface under a point.
// SIZE 0xc
typedef struct ShapeCollisionFns {
	MechS32 (*m_testPoint)(Shape*, MechS32, MechS32, MechS32);           // 0x00
	MechS32 (*m_testRay)(Shape*, Ray*);                                  // 0x04
	MechS32 (*m_getHeight)(Shape*, MechS32, MechS32, MechS32, MechS32*); // 0x08
} ShapeCollisionFns;

// The functions and globals of collision.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a554c;
	extern MechS32 g_unk0x100a5550;
	extern MechS32 g_unk0x100a5558;
	extern ShapeCollisionFns g_shapeCollisionFns[8];
	extern MechS32 g_hitNormalX;
	extern MechS32 g_hitNormalY;
	extern MechS32 g_hitNormalZ;
	extern MechS32 g_groundNormalX;
	extern MechS32 g_groundNormalY;
	extern MechS32 g_groundNormalZ;
	extern MechS32 g_segmentNormalX;
	extern MechS32 g_segmentNormalY;
	extern MechS32 g_segmentNormalZ;
	extern MechS32 g_unk0x100a5544;
	extern MechS32 g_unk0x100a5548;

	void FUN_10034a40(Shape* p_shape, MechS32 p_unk0x24);
	MechS32 FUN_10034a7b(
		struct Face* p_face,
		struct Vertex* p_vertices,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z,
		MechS32* p_height
	);
	MechS32 GetTerrainHeight(MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_10034cbc(MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_10034db8(Shape* p_shape);
	MechS32 TestPointCollision(MechS32 p_x, MechS32 p_y, MechS32 p_z, Shape** p_hit);
	Shape* FUN_10034e59(Shape* p_root, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_10034ee7(Shape* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 TestSegmentCollision(Ray* p_ray, Shape** p_hit, MechS32 p_exclude);
	MechS32 FUN_10035107(Ray* p_ray, Shape** p_hit);
	MechS32 FUN_100352ad(Shape* p_shape, Ray* p_ray, MechS32 p_distance);
	void FUN_10035423(Shape* p_shape, Ray* p_ray, MechS32 p_distance);
	MechS32 FUN_100354d3(struct Face* p_face, struct Vertex* p_vertices, Ray* p_ray);
	MechS32 FUN_10035722(struct Face* p_face, struct Vertex* p_vertices, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_100357f8(struct Face* p_face, struct Vertex* p_vertices, MechS32 p_x, MechS32 p_z);
	MechS32 FUN_10035b5b(struct Face* p_face, struct Vertex* p_vertices, MechS32 p_x, MechS32 p_y);
	MechS32 FUN_10035ebe(struct Face* p_face, struct Vertex* p_vertices, MechS32 p_y, MechS32 p_z);

#ifdef __cplusplus
}
#endif

#endif // COLLISION_H
