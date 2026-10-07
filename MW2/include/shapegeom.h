#ifndef SHAPEGEOM_H
#define SHAPEGEOM_H

#include "decomp.h"
#include "fixedfloat.h"
#include "model.h"
#include "transform.h"
#include "types.h"

struct Ray;
struct Shape;

// The functions of shapegeom.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void TransformModel(Model* p_model, Matrix* p_matrix);
	void TransformShapeCenter(struct Shape* p_shape, Matrix* p_matrix);
	void TransformShape(struct Shape* p_shape, Matrix* p_matrix);
	MechS32 SolvePlaneY(
		MechS32 p_normalX,
		MechS32 p_normalY,
		MechS32 p_normalZ,
		MechS32 p_distance,
		MechS32 p_dx,
		MechS32 p_dz
	);
	MechScalar ApproximateShapeDistance(struct Shape* p_shape, MechScalar p_x, MechScalar p_y, MechScalar p_z);
	MechS32 ComputeTriangleNormal(
		MechScalar p_x0,
		MechScalar p_y0,
		MechScalar p_z0,
		MechScalar p_x1,
		MechScalar p_y1,
		MechScalar p_z1,
		MechScalar p_x2,
		MechScalar p_y2,
		MechScalar p_z2,
		MechScalar* p_nx,
		MechScalar* p_ny,
		MechScalar* p_nz
	);
	MechScalar RayShapeDistance(struct Shape* p_shape, struct Ray* p_ray);

#ifdef __cplusplus
}
#endif

#endif // SHAPEGEOM_H
