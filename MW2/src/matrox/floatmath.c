/* The Matrox edition's floating-point trigonometry and vector math: one object of its own,
   between menucontrols.c's and missionsetup.c's, with the edition's versions of functions 1.1
   has in clock.c (the tables, Hypot2D), fixedtrig.c (the sine and arctangent), approxlen.c,
   shapegeom.c (ComputeTriangleNormal) and fixedsqrt.c (NormalizeVectorGuarded). 1.1's versions
   stay in those units, which the Matrox edition doesn't build or builds without them. */
#define FIXEDTRIG_FLOAT_SINE
#include "approxlen.h"
#include "clock.h"
#include "decomp.h"
#include "fixedfloat.h"
#include "fixedsqrt.h"
#include "fixedtrig.h"
#include "shapegeom.h"
#include "types.h"

#include <math.h>

// A quarter wave of sines and the arctangents of 0 to 1 in 512 steps (degrees). The last
// entries are written one past each table: g_sinTable's lands on g_atanTable[0] in the original,
// and on other globals in this build, which the two stores' diffs show.
// FUNCTION: MW2MATROX 0x1007edd0
MechS32 InitSinAtanTables(void)
{
	MechS32 i;

	for (i = 0; i < 0x200; i++) {
		g_sinTable[i] = sin(i * (3.14159265 / 1024));
		g_atanTable[i] = atan(i / 512.0) * (180 / 3.14159265);
	}

	g_sinTable[0x200] = 1.0f;
	g_atanTable[0x200] = 45.0f;
	return TRUE;
}

// Returns the sine of p_angle (degrees) from the table's quarter wave, whose 512 steps make
// 512 / 90 of them to the degree, mirrored and negated by quadrant.
// FUNCTION: MW2MATROX 0x1007ee7b
MechScalar FixedSin(MechScalar p_angle)
{
	MechScalar sine;
	MechS32 negative;
	MechScalar result;

	negative = 0;
	if ((MechFloat) fabs(p_angle) < 1e-07f) {
		return 0.0f;
	}

	if (FIXED_IS_NEGATIVE(p_angle)) {
		p_angle = -p_angle;
		negative = 1;
	}

	while (p_angle > 360.0f) {
		p_angle -= 360.0f;
	}

	if (p_angle > 180.0f) {
		p_angle -= 180.0f;
		negative ^= 1;
	}

	if (p_angle <= 90.0f) {
		sine = g_sinTable[(MechS32) (p_angle * 5.688889f + 0.5f)];
	}
	else {
		sine = g_sinTable[(MechS32) ((180.0f - p_angle) * 5.688889f + 0.5f)];
	}

	result = negative ? -sine : sine;
	return result;
}
// Returns the bearing of (p_x, p_z) in degrees: the arctangent of the smaller over the larger
// component from the table, folded into its quadrant.
// Stack-slot permutation; p_z > p_x compares in the other operand order.
// FUNCTION: MW2MATROX 0x1007ef93
MechScalar FixedAtan2(MechScalar p_x, MechScalar p_z)
{
	MechS32 zNegative;
	MechS32 xNegative;
	MechFloat angle;

	zNegative = 0;
	xNegative = 0;
	if ((MechFloat) fabs(p_x) < 1e-7f) {
		if ((MechFloat) fabs(p_z) < 1e-7f) {
			return 45.0f;
		}
		else if (FIXED_IS_NEGATIVE(p_z)) {
			return 180.0f;
		}
		else {
			return 0.0f;
		}
	}
	else if ((MechFloat) fabs(p_z) < 1e-7f) {
		if (FIXED_IS_NEGATIVE(p_x)) {
			return -90.0f;
		}
		else {
			return 90.0f;
		}
	}

	if (FIXED_IS_NEGATIVE(p_x)) {
		p_x = -p_x;
		xNegative = 1;
	}

	if (FIXED_IS_NEGATIVE(p_z)) {
		p_z = -p_z;
		zNegative = 1;
	}

	if ((MechFloat) fabs(p_x - p_z) < 1e-7f) {
		angle = 45.0f;
	}
	else if (p_z > p_x) {
		angle = g_atanTable[(MechS32) (p_x / p_z * 512.0f + 0.5f)];
	}
	else {
		angle = 90.0f - g_atanTable[(MechS32) (p_z / p_x * 512.0f + 0.5f)];
	}

	if (zNegative) {
		angle = 180.0f - angle;
	}

	if (xNegative) {
		angle = -angle;
	}

	return angle;
}

// The cosines and sines of the angles whose tangents are 0 to 50 in steps of 1/16.
// FUNCTION: MW2MATROX 0x1007f15b
MechS32 InitSlopeTables(void)
{
	MechS32 i;
	MechDouble cosine;

	for (i = 0; i < 800; i++) {
		cosine = 1.0 / sqrt(i / 16.0 * (i / 16.0) + 1.0);
		g_slopeCosines[i] = cosine;
		g_slopeSines[i] = i / 16.0 * cosine;
	}

	return TRUE;
}

// The Matrox edition's is C, on floats, and takes the absolute values in the order x, z, y.
// FUNCTION: MW2MATROX 0x1007f25b
MechScalar ApproximateVectorLength(MechScalar p_x, MechScalar p_y, MechScalar p_z)
{
	MechScalar z;
	MechScalar y;
	MechScalar x;
	MechScalar swap;
	MechScalar swap2;

	if (FIXED_IS_NEGATIVE(p_x)) {
		x = -p_x;
	}
	else {
		x = p_x;
	}

	if (FIXED_IS_NEGATIVE(p_z)) {
		z = -p_z;
	}
	else {
		z = p_z;
	}

	if (FIXED_IS_NEGATIVE(p_y)) {
		y = -p_y;
	}
	else {
		y = p_y;
	}

	if (x < z) {
		swap = x;
		x = z;
		z = swap;
	}

	if (x < y) {
		swap2 = x;
		x = y;
		y = swap2;
	}

	return (x * 4 + y + z) / 4;
}

// Operand order: the original reloads y for its square after x's, this build keeps it on the FPU stack.
// FUNCTION: MW2MATROX 0x1007f336
MechScalar Hypot2D(MechScalar p_x, MechScalar p_y)
{
	MechDouble x;
	MechDouble y;

	x = p_x;
	y = p_y;
	return sqrt(x * x + y * y);
}

// The unit normal of the triangle (p_x0, p_y0, p_z0), (p_x1, p_y1, p_z1), (p_x2, p_y2, p_z2): the
// cross product of its edges over its length; returns the length.
// Commutative operand order: the original computes each product's factors the other way round, and
// reloads nz for its square. Stack-slot permutation: length, nx, ny and nz.
// FUNCTION: MW2MATROX 0x1007f368
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
)
{
	MechScalar length;
	MechScalar nx;
	MechScalar ny;
	MechScalar nz;

	nx = -((p_z2 - p_z1) * (p_y1 - p_y0) - (p_z1 - p_z0) * (p_y2 - p_y1));
	ny = -((p_z1 - p_z0) * (p_x2 - p_x1) - (p_z2 - p_z1) * (p_x1 - p_x0));
	nz = -((p_x1 - p_x0) * (p_y2 - p_y1) - (p_y1 - p_y0) * (p_x2 - p_x1));
	length = sqrt(ny * ny + nz * nz + nx * nx);
	*p_nx = nx / length;
	*p_ny = ny / length;
	*p_nz = nz / length;
	return (MechS32) length;
}

// Scales the vector to length 1.0; a zero vector stays as it is.
// FUNCTION: MW2MATROX 0x1007f461
void NormalizeVectorGuarded(MechScalar* p_x, MechScalar* p_y, MechScalar* p_z)
{
	MechScalar length;

	length = ApproximateVectorLength(*p_x, *p_y, *p_z);
	if (length > 1e-07f) {
		*p_x /= length;
		*p_y /= length;
		*p_z /= length;
	}
}

// Not in the Matrox edition, whose callers expand the cosine: the units that still declare FixedSin and
// FixedCos with 16.16 values link to this until they move to the float sine.
#undef FixedCos
MechS32 FixedCos(MechS32 p_angle)
{
	return (MechS32) FixedSin(p_angle + 90.0f);
}
