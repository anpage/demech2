#include "shapecollision.h"

#include "boundbox.h"
#include "collision.h"
#include "decomp.h"
#include "duskmoth.h"
#include "emberfern.h"
#include "quadtree.h"
#include "ray.h"
#include "shape.h"
#include "shapegeom.h"
#include "types.h"

/* The collision tests of the shape types (collision.c's g_shapeCollisionFns): a point or a
   ray against a shape's bounding box, its floor or ceiling faces, or the quadtree of a
   terrain shape. Each hit leaves the surface normal in g_hitNormal. */

// GLOBAL: MW2 0x100ad43c
MechS32 g_unk0x100ad43c = 0;

// Stack-slot permutation: inColumn and top.
// FUNCTION: MW2 0x100699a0
MechS32 FUN_100699a0(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 inColumn;
	MechS32 top;
	MechS32 inside;

	FUN_10069a4b(p_shape, p_x, p_y, p_z, &inside, &inColumn, &top);
	return inside;
}

// Tests whether (p_x, p_z) lies over the shape's bounding box; if it does, returns the top in
// p_top and an upward normal.
// FUNCTION: MW2 0x100699da
MechS32 FUN_100699da(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32* p_top)
{
	MechS32 top;
	MechS32 inColumn;
	MechS32 inside;

	inColumn = 0;
	FUN_10069a4b(p_shape, p_x, p_y, p_z, &inside, &inColumn, &top);
	if (inColumn) {
		*p_top = top;
		g_hitNormalX = g_hitNormalZ = 0;
		g_hitNormalY = 0x10000;
	}

	return inColumn;
}

// Tests the point against the shape's bounding box: p_inColumn is set when (p_x, p_z) lies
// within it, with the box's top in p_top, and p_inside when p_y does as well.
// FUNCTION: MW2 0x10069a4b
void FUN_10069a4b(
	ScarletOrchid0x4c* p_shape,
	MechS32 p_x,
	MechS32 p_y,
	MechS32 p_z,
	MechS32* p_inside,
	MechS32* p_inColumn,
	MechS32* p_top
)
{
	CinderBox0x18* box;

	FUN_1006e9e6(p_shape);
	box = p_shape->m_unk0x44;
	if (!box) {
		*p_inColumn = 0;
		*p_inside = 0;
	}
	else if (p_x >= box->m_minX && p_x <= box->m_maxX && p_z >= box->m_minZ && p_z <= box->m_maxZ) {
		*p_inColumn = 1;
		*p_top = box->m_maxY;
		if (p_y >= box->m_minY && p_y <= box->m_maxY) {
			*p_inside = 1;
		}
		else {
			*p_inside = 0;
		}
	}
	else {
		*p_inColumn = 0;
		*p_inside = 0;
	}
}

// Clips the ray against the shape's bounding box, one slab per axis. On a hit, advances the
// ray's start to the entry point and sets the normal of the face it entered through.
// Stack-slot permutation of the locals; the tNear/tEnter and tFar/tExit comparisons
// have their operands the other way around.
// FUNCTION: MW2 0x10069b2a
MechS32 FUN_10069b2a(ScarletOrchid0x4c* p_shape, Ray* p_ray)
{
	MechS32 tNear;
	MechS32 axis;
	MechS32 tEnter;
	CinderBox0x18* box;
	MechS32 tFar;
	MechS32 tExit;

	FUN_1006e9e6(p_shape);
	box = p_shape->m_unk0x44;
	if (!box) {
		return 0;
	}

	if (FUN_10003445(p_ray->m_x0, p_ray->m_dirX, box->m_minX, box->m_maxX, &tNear, &tFar)) {
		return 0;
	}

	axis = 0;
	if (FUN_10003445(p_ray->m_y0, p_ray->m_dirY, box->m_minY, box->m_maxY, &tEnter, &tExit)) {
		return 0;
	}

	if (tNear < tEnter) {
		axis = 1;
		tNear = tEnter;
	}
	if (tFar > tExit) {
		tFar = tExit;
	}

	if (FUN_10003445(p_ray->m_z0, p_ray->m_dirZ, box->m_minZ, box->m_maxZ, &tEnter, &tExit)) {
		return 0;
	}

	if (tNear < tEnter) {
		tNear = tEnter;
		axis = 2;
	}
	if (tFar > tExit) {
		tFar = tExit;
	}

	if (tFar < tNear) {
		return 0;
	}

	if (tNear < 0) {
		g_unk0x100ad43c = tNear;
		tNear = 0;
	}

	tEnter = GetRayLength(p_ray);
	if (tFar > tEnter) {
		tFar = tEnter;
	}

	if (tFar < tNear) {
		return 0;
	}

	SetRayLength(p_ray, tNear);
	switch (axis) {
	case 0:
		g_hitNormalY = g_hitNormalZ = 0;
		if (p_ray->m_dx > 0) {
			g_hitNormalX = -0x10000;
		}
		else {
			g_hitNormalX = 0x10000;
		}
		break;
	case 1:
		g_hitNormalX = g_hitNormalZ = 0;
		if (p_ray->m_dy > 0) {
			g_hitNormalY = -0x10000;
		}
		else {
			g_hitNormalY = 0x10000;
		}
		break;
	case 2:
		g_hitNormalX = g_hitNormalY = 0;
		if (p_ray->m_dz > 0) {
			g_hitNormalZ = -0x10000;
		}
		else {
			g_hitNormalZ = 0x10000;
		}
		break;
	}

	return 1;
}

// Tests whether (p_x, p_z) lies over the shape's bounding box.
// FUNCTION: MW2 0x10069dd4
MechS32 FUN_10069dd4(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	CinderBox0x18* box;

	FUN_1006e9e6(p_shape);
	box = p_shape->m_unk0x44;
	if (!box) {
		return 0;
	}

	return p_x >= box->m_minX && p_x <= box->m_maxX && p_z >= box->m_minZ && p_z <= box->m_maxZ;
}

// FUNCTION: MW2 0x10069e54
MechS32 FUN_10069e54(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	return 0;
}

// FUNCTION: MW2 0x10069e66
MechS32 FUN_10069e66(ScarletOrchid0x4c* p_shape, Ray* p_ray)
{
	return 0;
}

// Finds the upward-facing face under (p_x, p_z) and tests whether p_y lies below it.
// Stack-slot permutation: i, done, face and height.
// FUNCTION: MW2 0x10069e78
MechS32 FUN_10069e78(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 i;
	MechS32 height;
	MechS32 done;
	DuskMoth0x24* face;

	i = 0;
	done = FALSE;
	while (!done) {
		face = (DuskMoth0x24*) ((MechU8*) p_shape->m_unk0x20 + p_shape->m_unk0x20->m_unk0x08) + i;
		if (face->m_normal[1] > 0 && FUN_100357f8(face, (EmberFern0x2c*) (p_shape->m_unk0x20 + 1), p_x, p_z)) {
			done = TRUE;
			if (FUN_10034a7b(face, (EmberFern0x2c*) (p_shape->m_unk0x20 + 1), p_x, p_y, p_z, &height)) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			i++;
			if (i == p_shape->m_unk0x20->m_unk0x06) {
				done = TRUE;
			}
		}
	}

	return 0;
}

// FUNCTION: MW2 0x10069f67
MechS32 FUN_10069f67(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	GraniteLattice0x18* model;

	model = p_shape->m_unk0x1c;
	if (p_shape->m_unk0x44) {
		return FUN_1001e6dc(p_shape->m_unk0x44, model, p_x, p_y, p_z) & 1;
	}
	else {
		return FUN_10069e78(p_shape, p_x, p_y, p_z);
	}
}

// FUNCTION: MW2 0x10069fd4
MechS32 FUN_10069fd4(ScarletOrchid0x4c* p_shape, Ray* p_ray)
{
	return FUN_1001e90f(p_shape->m_unk0x44, p_shape->m_unk0x1c, p_ray) & 1;
}

// FUNCTION: MW2 0x1006a001
MechS32 FUN_1006a001(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32* p_top)
{
	return FUN_1001ebfa(p_shape->m_unk0x44, p_shape->m_unk0x1c, p_x, p_y, p_z, p_top);
}

// Finds the downward-facing face over (p_x, p_z) and tests whether p_y lies above it.
// Stack-slot permutation of the locals; the p_y > height comparison has its operands the
// other way around.
// FUNCTION: MW2 0x1006a037
MechS32 FUN_1006a037(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 i;
	EmberFern0x2c* vertices;
	MechS32 done;
	DuskMoth0x24* face;
	MechS32 height;
	EmberFern0x2c* vertex;

	i = 0;
	done = FALSE;
	vertices = (EmberFern0x2c*) (p_shape->m_unk0x20 + 1);
	while (!done) {
		face = (DuskMoth0x24*) ((MechU8*) p_shape->m_unk0x20 + p_shape->m_unk0x20->m_unk0x08) + i;
		if (face->m_normal[1] < 0 && FUN_100357f8(face, vertices, p_x, p_z)) {
			done = TRUE;
			vertex = &vertices[*((MechU8*) face + face->m_unk0x04)];
			height = vertex->m_unk0x10 - FUN_10039c96(
											 face->m_normal[0],
											 face->m_normal[1],
											 face->m_normal[2],
											 0,
											 p_x - vertex->m_unk0x0c,
											 p_z - vertex->m_unk0x14
										 );
			g_hitNormalX = face->m_normal[0] >> 13;
			g_hitNormalY = face->m_normal[1] >> 13;
			g_hitNormalZ = face->m_normal[2] >> 13;
			if (p_y > height) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			i++;
			if (i == p_shape->m_unk0x20->m_unk0x06) {
				done = TRUE;
			}
		}
	}

	return 0;
}

// Tests the ray against each face of the shape's model.
// Stack-slot permutation: vertices, i and face.
// FUNCTION: MW2 0x1006a190
MechS32 FUN_1006a190(ScarletOrchid0x4c* p_shape, Ray* p_ray)
{
	GraniteLattice0x18* model;
	EmberFern0x2c* vertices;
	MechS32 i;
	DuskMoth0x24* face;

	model = p_shape->m_unk0x20;
	if (!model) {
		return 0;
	}

	vertices = (EmberFern0x2c*) (model + 1);
	for (i = 0; i < model->m_unk0x06; i++) {
		face = (DuskMoth0x24*) ((MechU8*) model + model->m_unk0x08) + i;
		if (FUN_100354d3(face, vertices, p_ray)) {
			return 1;
		}
	}

	return 0;
}
