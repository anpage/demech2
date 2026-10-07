#ifndef COLLISION_H
#define COLLISION_H

#include "fixedfloat.h"
#include "ray.h"
#include "shape.h"
#include "types.h"

struct Face;
struct Vertex;

// A shape type's collision tests: a point, a ray, and the height of the surface under a point.
// SIZE 0xc
typedef struct ShapeCollisionFns {
	MechS32 (*m_testPoint)(Shape*, MechScalar, MechScalar, MechScalar);              // 0x00
	MechS32 (*m_testRay)(Shape*, Ray*);                                              // 0x04
	MechS32 (*m_getHeight)(Shape*, MechScalar, MechScalar, MechScalar, MechScalar*); // 0x08
} ShapeCollisionFns;

// The functions and globals of collision.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_groundColor;
	extern MechS32 g_horizonMapColor;
	extern MechS32 g_unk0x100a5558;
	extern ShapeCollisionFns g_shapeCollisionFns[8];
	extern MechScalar g_hitNormalX;
	extern MechScalar g_hitNormalY;
	extern MechScalar g_hitNormalZ;
	extern MechScalar g_groundNormalX;
	extern MechScalar g_groundNormalY;
	extern MechScalar g_groundNormalZ;
	extern MechScalar g_segmentNormalX;
	extern MechScalar g_segmentNormalY;
	extern MechScalar g_segmentNormalZ;
	extern MechS32 g_backgroundColor;
	extern MechS32 g_skyColor;

	void SetShapeCollisionType(Shape* p_shape, MechS32 p_collisionType);
	MechS32 IsBelowFacePlane(
		struct Face* p_face,
		struct Vertex* p_vertices,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z,
		MechScalar* p_height
	);
	MechScalar GetTerrainHeight(MechScalar p_x, MechScalar p_y, MechScalar p_z);
	MechScalar GetHighestSurface(MechScalar p_x, MechScalar p_y, MechScalar p_z);
	MechS32 HasHeightTest(Shape* p_shape);
	MechS32 TestPointCollision(MechScalar p_x, MechScalar p_y, MechScalar p_z, Shape** p_hit);
	Shape* FindNearestShape(Shape* p_root, MechScalar p_x, MechScalar p_y, MechScalar p_z);
	MechS32 TestShapePoint(Shape* p_shape, MechScalar p_x, MechScalar p_y, MechScalar p_z);
	MechS32 TestSegmentCollision(Ray* p_ray, Shape** p_hit, MechS32 p_exclude);
	MechS32 TestSceneryCollision(Ray* p_ray, Shape** p_hit);
	MechS32 TestShapeRay(Shape* p_shape, Ray* p_ray, MechScalar p_distance);
	void EndRayAtShape(Shape* p_shape, Ray* p_ray, MechScalar p_distance);
	MechS32 IntersectRayFace(struct Face* p_face, struct Vertex* p_vertices, Ray* p_ray);
	MechS32 IsPointInFace(
		struct Face* p_face,
		struct Vertex* p_vertices,
		MechScalar p_x,
		MechScalar p_y,
		MechScalar p_z
	);
	MechS32 IsPointInFaceXZ(struct Face* p_face, struct Vertex* p_vertices, MechScalar p_x, MechScalar p_z);
	MechS32 IsPointInFaceXY(struct Face* p_face, struct Vertex* p_vertices, MechScalar p_x, MechScalar p_y);
	MechS32 IsPointInFaceYZ(struct Face* p_face, struct Vertex* p_vertices, MechScalar p_y, MechScalar p_z);

#ifdef __cplusplus
}
#endif

#endif // COLLISION_H
