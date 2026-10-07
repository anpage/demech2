#ifndef RAY_H
#define RAY_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

// A segment from (x0, y0, z0) to (x1, y1, z1) in 16.16 fixed point, with its delta and, once
// BuildRayFixed or BuildRayFloat has run, its unit direction and length.
// SIZE 0x38
typedef struct Ray {
	MechScalar m_x0;     // 0x00
	MechScalar m_y0;     // 0x04
	MechScalar m_z0;     // 0x08
	MechScalar m_x1;     // 0x0c
	MechScalar m_y1;     // 0x10
	MechScalar m_z1;     // 0x14
	MechScalar m_dx;     // 0x18
	MechScalar m_dy;     // 0x1c
	MechScalar m_dz;     // 0x20
	MechScalar m_dirX;   // 0x24
	MechScalar m_dirY;   // 0x28
	MechScalar m_dirZ;   // 0x2c
	MechScalar m_length; // 0x30
	MechS32 m_state;     // 0x34
} Ray;

// Ray::m_state: which of the builders computed the direction and length.
enum {
	c_rayNone = 0,
	c_rayFixed = 1,
	c_rayFloat = 2
};

// The functions and globals of ray.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void BuildRayFromSegment(
		Ray* p_ray,
		MechScalar p_x0,
		MechScalar p_y0,
		MechScalar p_z0,
		MechScalar p_x1,
		MechScalar p_y1,
		MechScalar p_z1
	);
	void BuildRayFromDirection(
		Ray* p_ray,
		MechScalar p_x0,
		MechScalar p_y0,
		MechScalar p_z0,
		MechScalar p_dirX,
		MechScalar p_dirY,
		MechScalar p_dirZ,
		MechScalar p_length
	);
	MechScalar GetRayLength(Ray* p_ray);
	void BuildRayFixed(Ray* p_ray);
	void BuildRayFloat(Ray* p_ray);
	void SetRayLength(Ray* p_ray, MechScalar p_length);
	void AdvanceRayStart(Ray* p_ray, MechScalar p_distance);
	void SetRayEnd(Ray* p_ray, MechScalar p_x1, MechScalar p_y1, MechScalar p_z1);
	void ClipRayToGround(Ray* p_ray, MechScalar p_y);
	void CopyRay(Ray* p_dst, Ray* p_src);
	MechS32 ClipRaySlab(
		MechScalar p_origin,
		MechScalar p_delta,
		MechScalar p_min,
		MechScalar p_max,
		MechScalar* p_tMin,
		MechScalar* p_tMax
	);

#ifdef __cplusplus
}
#endif

#endif // RAY_H
