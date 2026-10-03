#ifndef SHAPEGEOM_H
#define SHAPEGEOM_H

#include "decomp.h"
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

	void FUN_10039a30(Model* p_model, Matrix* p_matrix);
	void FUN_10039b94(struct Shape* p_shape, Matrix* p_matrix);
	void FUN_10039c36(struct Shape* p_shape, Matrix* p_matrix);
	MechS32 FUN_10039c96(
		MechS32 p_normalX,
		MechS32 p_normalY,
		MechS32 p_normalZ,
		MechS32 p_unk0x0c,
		MechS32 p_dx,
		MechS32 p_dz
	);
	MechS32 FUN_10039ccc(struct Shape* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_10039dda(
		MechS32 p_x0,
		MechS32 p_y0,
		MechS32 p_z0,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_z1,
		MechS32 p_x2,
		MechS32 p_y2,
		MechS32 p_z2,
		MechS32* p_nx,
		MechS32* p_ny,
		MechS32* p_nz
	);
	MechS32 FUN_1003a096(struct Shape* p_shape, struct Ray* p_ray);

#ifdef __cplusplus
}
#endif

#endif // SHAPEGEOM_H
