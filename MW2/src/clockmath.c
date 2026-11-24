/* clock.c's tables (sines, arctangents, slopes and square roots), Hypot2D and
   BuildMatrixFromDirection, with the rotation's normalization. Not a unit of its own in 1.1, which
   has them in clock.c's object, ahead of the clocks, so clock.c includes this file in 1.1's build
   and the accelerated editions'; GBL has them in an object of their own, at the end of its code,
   and builds this file as a unit. */
#include "clock.h"
#include "debugprint.h"
#include "decomp.h"
#include "mss.h"
#include "network.h"
#include "ticks.h"
#include "transform.h"
#include "types.h"

#include <math.h>

// The Matrox edition's tables, Hypot2D and trigonometry are its own float code, in accel/floatmath.c.
#ifndef MW2_FLOAT_SCALARS
// A quarter wave of sines (2.29 fixed point, 1024 steps to the circle) and the arctangents of
// 0 to 1 in 256 steps (16.16 degrees), each padded with two copies of its last value.
// FUNCTION: MW2 0x1007c930
// FUNCTION: MW2GBL 0x100809d0
// FUNCTION: MW2PREMERCS 0x10058300
MechS32 InitSinAtanTables(void)
{
	MechS32 i;

	for (i = 0; i < 0x100; i++) {
		g_sinTable[i] = (MechS32) (sin(i * (3.14159265 / 512)) * 536870912.0);
		g_atanTable[i] = (MechS32) (atan(i / 256.0) * 3754936.210460003);
	}

	g_sinTable[0x100] = g_sinTable[0x101] = 0x20000000;
	g_atanTable[0x100] = g_atanTable[0x101] = 0x2d0000;
	return TRUE;
}

// The cosines and sines of the angles whose tangents are 0 to 50 in steps of 1/16 (2.29).
// FUNCTION: MW2 0x1007c9e3
// FUNCTION: MW2GBL 0x10080a95
// FUNCTION: MW2PREMERCS 0x100583c5
MechS32 InitSlopeTables(void)
{
	MechS32 i;
	MechDouble cosine;

	for (i = 0; i < 800; i++) {
		g_slopeCosines[i] = (MechS32) ((cosine = 1.0 / sqrt(i / 16.0 * (i / 16.0) + 1.0)) * 536870912.0);
		g_slopeSines[i] = (MechS32) (i / 16.0 * cosine * 536870912.0);
	}

	return TRUE;
}

#endif

// The square roots of 0 to 1023 (6.10 fixed point).
// FUNCTION: MW2 0x1007ca8e
// FUNCTION: MW2GBL 0x10080b47
// FUNCTION: MW2PREMERCS 0x10058477
MechS32 InitSqrtTable(void)
{
	MechS32 i;

	g_sqrtTable = g_sqrtTableData;
	for (i = 0; i < 0x400; i++) {
		g_sqrtTable[i] = (MechS16) (sqrt(i) * 1024.0);
	}

	return TRUE;
}

#ifndef MW2_FLOAT_SCALARS
// FUNCTION: MW2 0x1007caf7
// FUNCTION: MW2GBL 0x10080bb9
// FUNCTION: MW2PREMERCS 0x100584e9
MechScalar Hypot2D(MechScalar p_x, MechScalar p_y)
{
	MechS32 length;
	MechDouble x;
	MechDouble y;

	x = p_x;
	length = (MechS32) sqrt((y = p_y) * y + x * x);
	return length;
}
#endif

// Sets p_matrix to the rotation that points along (p_x, p_y, p_z).
// FUNCTION: MW2 0x1007cb3d
// FUNCTION: MW2GBL 0x10080c08
// FUNCTION: MW2PREMERCS 0x10058538
void BuildMatrixFromDirection(Matrix* p_matrix, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechDouble pitch;
	MechDouble yaw;

	yaw = atan2(p_x, p_z);
#ifdef MW2_LENGTH_SUM_FROM_Z
	pitch = -asin(p_y / sqrt((MechDouble) p_z * p_z + (MechDouble) p_y * p_y + (MechDouble) p_x * p_x));
#else
	pitch = -asin(p_y / sqrt((MechDouble) p_x * p_x + (MechDouble) p_y * p_y + (MechDouble) p_z * p_z));
#endif
	BuildMatrix(p_matrix, (MechS32) (pitch * 3754939.378), (MechS32) (yaw * 3754939.378), 0, 0, 0, 0);
}

// The rotation's normalization: 1.1 has it here, the Matrox edition at the end of transform.c's object.
#ifndef MW2_ACCEL
#include "normalizerotation.c"
#endif
