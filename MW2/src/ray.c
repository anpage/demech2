#include "ray.h"

#include "approxlen.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "fixedfloat.h"
#include "fixedmul.h"
#include "muldiv.h"
#include "types.h"

#include <math.h>

DECOMP_SIZE_ASSERT(Ray, 0x38)

// FUNCTION: MW2 0x10002e20
// FUNCTION: MW2MATROX 0x10087830
void BuildRayFromSegment(
	Ray* p_ray,
	MechScalar p_x0,
	MechScalar p_y0,
	MechScalar p_z0,
	MechScalar p_x1,
	MechScalar p_y1,
	MechScalar p_z1
)
{
	p_ray->m_x0 = p_x0;
	p_ray->m_y0 = p_y0;
	p_ray->m_z0 = p_z0;
	p_ray->m_x1 = p_x1;
	p_ray->m_y1 = p_y1;
	p_ray->m_z1 = p_z1;
	p_ray->m_dx = p_x1 - p_x0;
	p_ray->m_dy = p_y1 - p_y0;
	p_ray->m_dz = p_z1 - p_z0;
	p_ray->m_dirX = p_ray->m_dirY = p_ray->m_dirZ = p_ray->m_length = 0;
	p_ray->m_state = c_rayNone;
}

// MW2MATROX: the products p_length * p_dir load their operands in the other order.
// FUNCTION: MW2 0x10002ebc
// FUNCTION: MW2MATROX 0x100878cc
void BuildRayFromDirection(
	Ray* p_ray,
	MechScalar p_x0,
	MechScalar p_y0,
	MechScalar p_z0,
	MechScalar p_dirX,
	MechScalar p_dirY,
	MechScalar p_dirZ,
	MechScalar p_length
)
{
	p_ray->m_x0 = p_x0;
	p_ray->m_y0 = p_y0;
	p_ray->m_z0 = p_z0;
	p_ray->m_dirX = p_dirX;
	p_ray->m_dirY = p_dirY;
	p_ray->m_dirZ = p_dirZ;
#ifdef MW2_MATROX
	p_ray->m_dx = p_length * p_dirX;
	p_ray->m_dy = p_length * p_dirY;
	p_ray->m_dz = p_length * p_dirZ;
#else
	p_ray->m_dx = FixedMul16(p_length, p_dirX);
	p_ray->m_dy = FixedMul16(p_length, p_dirY);
	p_ray->m_dz = FixedMul16(p_length, p_dirZ);
#endif
	p_ray->m_x1 = p_ray->m_dx + p_x0;
	p_ray->m_y1 = p_ray->m_dy + p_y0;
	p_ray->m_z1 = p_ray->m_dz + p_z0;
	p_ray->m_length = p_length;
	p_ray->m_state = c_rayFixed;
}

// FUNCTION: MW2 0x10002f7e
// FUNCTION: MW2MATROX 0x10087970
MechScalar GetRayLength(Ray* p_ray)
{
	if (p_ray->m_state == c_rayNone) {
		BuildRayFixed(p_ray);
	}

	return p_ray->m_length;
}

// FUNCTION: MW2 0x10002fad
// FUNCTION: MW2MATROX 0x1008799f
void BuildRayFixed(Ray* p_ray)
{
	MechScalar length;

	if (p_ray->m_state == c_rayNone) {
		p_ray->m_length = ApproximateVectorLength(p_ray->m_dx, p_ray->m_dy, p_ray->m_dz);
		length = p_ray->m_length;
		if (length > 0) {
#ifdef MW2_MATROX
			p_ray->m_dirX = p_ray->m_dx / length;
			p_ray->m_dirY = p_ray->m_dy / length;
			p_ray->m_dirZ = p_ray->m_dz / length;
#else
			p_ray->m_dirX = FixedDiv16(p_ray->m_dx, length);
			p_ray->m_dirY = FixedDiv16(p_ray->m_dy, length);
			p_ray->m_dirZ = FixedDiv16(p_ray->m_dz, length);
#endif
			p_ray->m_state = c_rayFixed;
		}
	}
}

// MW2MATROX: the sum of squares starts from dx where the original's starts from dz (kept on the FPU
// stack after its store), a commutative operand order.
// FUNCTION: MW2 0x10003053
// FUNCTION: MW2MATROX 0x10087a6a
void BuildRayFloat(Ray* p_ray)
{
	MechDouble dx;
	MechDouble dy;
	MechDouble dz;
	MechDouble length;

	if (p_ray->m_state == c_rayFloat) {
		return;
	}

	dx = p_ray->m_x1 - p_ray->m_x0;
	dy = p_ray->m_y1 - p_ray->m_y0;
	dz = p_ray->m_z1 - p_ray->m_z0;
	dx /= length = sqrt(dx * dx + dy * dy + dz * dz);
	dy /= length;
	dz /= length;
#ifdef MW2_MATROX
	p_ray->m_dirX = dx;
	p_ray->m_dirY = dy;
	p_ray->m_dirZ = dz;
	p_ray->m_length = length;
#else
	p_ray->m_dirX = (MechS32) (dx * 65536.0);
	p_ray->m_dirY = (MechS32) (dy * 65536.0);
	p_ray->m_dirZ = (MechS32) (dz * 65536.0);
	p_ray->m_length = (MechS32) (length + 0.5);
#endif
	p_ray->m_state = c_rayFloat;
}

// FUNCTION: MW2 0x1000313e
// FUNCTION: MW2MATROX 0x10087b59
void SetRayLength(Ray* p_ray, MechScalar p_length)
{
	if (p_ray->m_state == c_rayNone) {
		BuildRayFixed(p_ray);
	}

	if (p_ray->m_state != c_rayNone) {
#ifdef MW2_MATROX
		p_ray->m_x1 = p_ray->m_dirX * p_length + p_ray->m_x0;
		p_ray->m_y1 = p_ray->m_dirY * p_length + p_ray->m_y0;
		p_ray->m_z1 = p_ray->m_dirZ * p_length + p_ray->m_z0;
#else
		p_ray->m_x1 = p_ray->m_x0 + FixedMul16(p_length, p_ray->m_dirX);
		p_ray->m_y1 = p_ray->m_y0 + FixedMul16(p_length, p_ray->m_dirY);
		p_ray->m_z1 = p_ray->m_z0 + FixedMul16(p_length, p_ray->m_dirZ);
#endif
		p_ray->m_dx = p_ray->m_x1 - p_ray->m_x0;
		p_ray->m_dy = p_ray->m_y1 - p_ray->m_y0;
		p_ray->m_dz = p_ray->m_z1 - p_ray->m_z0;
		p_ray->m_length = p_length;
	}
}

// FUNCTION: MW2 0x1000320f
// FUNCTION: MW2MATROX 0x10087c06
void AdvanceRayStart(Ray* p_ray, MechScalar p_distance)
{
	MechS32 state;

	state = p_ray->m_state;
	if (state == c_rayNone) {
		BuildRayFixed(p_ray);
	}

	if (p_ray->m_length > 0) {
#ifdef MW2_MATROX
		p_ray->m_x0 += p_ray->m_dirX * p_distance;
		p_ray->m_y0 += p_ray->m_dirY * p_distance;
		p_ray->m_z0 += p_ray->m_dirZ * p_distance;
#else
		p_ray->m_x0 += FixedMul16(p_distance, p_ray->m_dirX);
		p_ray->m_y0 += FixedMul16(p_distance, p_ray->m_dirY);
		p_ray->m_z0 += FixedMul16(p_distance, p_ray->m_dirZ);
#endif
		p_ray->m_dx = p_ray->m_x1 - p_ray->m_x0;
		p_ray->m_dy = p_ray->m_y1 - p_ray->m_y0;
		p_ray->m_dz = p_ray->m_z1 - p_ray->m_z0;
		p_ray->m_state = c_rayNone;
		if (state == c_rayFloat) {
			BuildRayFloat(p_ray);
		}
		else {
			BuildRayFixed(p_ray);
		}
	}
}

// FUNCTION: MW2 0x100032f9
// FUNCTION: MW2MATROX 0x10087ced
void SetRayEnd(Ray* p_ray, MechScalar p_x1, MechScalar p_y1, MechScalar p_z1)
{
	MechS32 state;

	p_ray->m_x1 = p_x1;
	p_ray->m_y1 = p_y1;
	p_ray->m_z1 = p_z1;
	p_ray->m_dx = p_x1 - p_ray->m_x0;
	p_ray->m_dy = p_y1 - p_ray->m_y0;
	p_ray->m_dz = p_z1 - p_ray->m_z0;
	state = p_ray->m_state;
	p_ray->m_state = c_rayNone;
	switch (state) {
	case c_rayFixed:
		BuildRayFixed(p_ray);
		break;
	case c_rayFloat:
		BuildRayFloat(p_ray);
		break;
	default:
		p_ray->m_dirX = p_ray->m_dirY = p_ray->m_dirZ = p_ray->m_length = 0;
		break;
	}
}

// FUNCTION: MW2 0x100033df
// FUNCTION: MW2MATROX 0x10087dd3
void ClipRayToGround(Ray* p_ray, MechScalar p_y)
{
	MechScalar length;

#ifdef MW2_MATROX
	if ((MechFloat) fabs(p_ray->m_dy) >= 1e-07f) {
		length = (p_y - p_ray->m_y0) * p_ray->m_length / p_ray->m_dy;
		SetRayLength(p_ray, length);
	}
#else
	if (p_ray->m_dy) {
		length = MulDiv64(p_ray->m_length, p_y - p_ray->m_y0, p_ray->m_dy);
		SetRayLength(p_ray, length);
	}
#endif
}

// FUNCTION: MW2 0x1000342d
// FUNCTION: MW2MATROX 0x10087e35
void CopyRay(Ray* p_dst, Ray* p_src)
{
	*p_dst = *p_src;
}

// Matches except for the stack slots of toMax and quotient (a consistent permutation).
// MW2MATROX: the comparisons of p_origin with p_min and p_max have their operands the other way
// round.
// FUNCTION: MW2 0x10003445
// FUNCTION: MW2MATROX 0x10087e4d
MechS32 ClipRaySlab(
	MechScalar p_origin,
	MechScalar p_delta,
	MechScalar p_min,
	MechScalar p_max,
	MechScalar* p_tMin,
	MechScalar* p_tMax
)
{
#ifdef MW2_MATROX
	MechScalar t0;
	MechScalar t1;
	MechScalar toMin;
	MechScalar toMax;

	if ((MechFloat) fabs(p_delta) < 1e-07f) {
		if (p_origin < p_min || p_origin > p_max) {
			return 1;
		}

		t0 = -3.4e+38f;
		t1 = 3.4e+38f;
	}
	else {
		toMin = p_min - p_origin;
		toMax = p_max - p_origin;
		t0 = toMin / p_delta;
		t1 = toMax / p_delta;
	}

	if (t1 < t0) {
		*p_tMin = t1;
		*p_tMax = t0;
	}
	else {
		*p_tMin = t0;
		*p_tMax = t1;
	}

	return 0;
#else
	MechS32 toMax;
	MechS32 t0;
	MechS32 t1;
	MechS32 toMin;
	MechS32 overflow;
	MechS32 quotient;

	overflow = FALSE;
	if (p_delta == 0) {
		overflow = TRUE;
	}
	else {
		toMin = p_min - p_origin;
		toMax = p_max - p_origin;
		quotient = toMin / p_delta;
		if (quotient > 0x7fff || quotient < -0x8000) {
			overflow = TRUE;
		}
		else {
			quotient = toMax / p_delta;
			if (quotient > 0x7fff || quotient < -0x8000) {
				overflow = TRUE;
			}
		}
	}

	if (overflow) {
		if (p_origin < p_min || p_origin > p_max) {
			return 1;
		}

		t0 = 0x80000001;
		t1 = 0x7fffffff;
	}
	else {
		t0 = FixedDiv16(toMin, p_delta);
		t1 = FixedDiv16(toMax, p_delta);
	}

	if (t1 < t0) {
		*p_tMin = t1;
		*p_tMax = t0;
	}
	else {
		*p_tMin = t0;
		*p_tMax = t1;
	}

	return 0;
#endif
}
