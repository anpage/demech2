#include "unk1004b980.h"

#include "decomp.h"
#include "eyepoint.h"
#include "fixedmul.h"
#include "muldiv.h"
#include "simmain.h"
#include "transform.h"
#include "types.h"
#include "unk10019ad0.h"
#include "unk1004c800.h"
#include "unk1004c820.h"

#include <windows.h>

// GLOBAL: MW2 0x100a712c
MechS32 g_unk0x100a712c = 1;

// GLOBAL: MW2 0x100ea820
MechS32 g_unk0x100ea820;

// GLOBAL: MW2 0x100ea82c
MechS32 g_unk0x100ea82c;

// GLOBAL: MW2 0x100ea860
MechS32 g_unk0x100ea860;

// GLOBAL: MW2 0x100ea8d0
MechS32 g_unk0x100ea8d0;

// STUB: MW2 0x1004b980
void FUN_1004b980(Eyepoint* p_eyepoint)
{
	STUB(0x1004b980);
}

// Sets up the eyepoint's projection from its view rectangle, field of view and pixel aspect.
// Stack-slot permutation: every local.
// FUNCTION: MW2 0x1004bc2e
void FUN_1004bc2e(Eyepoint* p_eyepoint)
{
	MechS32 centerX;
	MechS32 bottom;
	MechS32 c;
	MechS32 centerY;
	MechS32 right;
	MechS32 aspect;
	MechS32 left;
	MechS32 scaled2;
	MechS32 low2;
	MechS32 offsetY;
	MechS16 shift1;
	MechS16 shift2;
	Eyepoint* eyepoint;
	MechS32 fovY;
	MechS32 d;
	MechS32 offsetX;
	MechS32 b;
	MechS32 halfWidth;
	MechS32 top;
	MechS32 halfHeight;
	MechS32 scaled1;
	MechS32 low1;
	MechS32 fov;
	MechS32 a;

	eyepoint = p_eyepoint;
	fov = eyepoint->m_fovX;
	aspect = eyepoint->m_pixelAspect;
	offsetX = (MechS16) eyepoint->m_unk0x4c;
	offsetY = (MechS16) eyepoint->m_unk0x50;
	top = eyepoint->m_unk0x34;
	bottom = eyepoint->m_unk0x38;
	left = eyepoint->m_unk0x2c;
	right = eyepoint->m_unk0x30;
	eyepoint->m_centerX = centerX = ((right + left + 1) >> 1) + offsetX;
	eyepoint->m_centerY = centerY = ((bottom + top + 1) >> 1) + offsetY;
	eyepoint->m_halfWidth = halfWidth = max((right - left + 1) >> 1, 1);
	eyepoint->m_halfHeight = halfHeight = max((bottom - top + 1) >> 1, 1);
	if (fov > 0x100000) {
		fov = 0x100000;
	}
	if (fov < 0x8000) {
		fov = 0x8000;
	}

	a = FUN_1004c800(fov, halfWidth, offsetX);
	b = FUN_1004c800(fov, halfWidth, -offsetX);
	eyepoint->m_fovY = fovY = MulDiv64(FixedMul16(fov, aspect), halfWidth, halfHeight);
	c = FUN_1004c800(fovY, halfHeight, offsetY);
	d = FUN_1004c800(fovY, halfHeight, -offsetY);
	if (a > 799) {
		a = 799;
	}
	if (b > 799) {
		b = 799;
	}
	if (c > 799) {
		c = 799;
	}
	if (d > 799) {
		d = 799;
	}

	eyepoint->m_unk0xa8 = FUN_10019ad0(fov, g_unk0x100bfd60[a]) + (g_unk0x100c09e0[a] >> 13);
	eyepoint->m_unk0xac = FUN_10019ad0(fovY, g_unk0x100bfd60[c]) + (g_unk0x100c09e0[c] >> 13);
	shift2 = shift1 = 2;
	FUN_1004c820(&low1, &scaled1, &shift1, halfWidth, FixedMul16(fov, aspect));
	FUN_1004c820(&low2, &scaled2, &shift2, halfWidth, fov);
	eyepoint->m_unk0x3c = (low2 >> 17) + 1;
	eyepoint->m_unk0xb4 = eyepoint->m_unk0x40;
	eyepoint->m_unk0xa4 = shift2;
	eyepoint->m_unk0xa6 = shift1;
	eyepoint->m_unk0x9c = scaled2;
	eyepoint->m_unk0xa0 = scaled1;
	eyepoint->m_unk0x94 = low2;
	eyepoint->m_unk0x98 = low1;
	if (g_unk0x100a712c <= 0) {
		g_unk0x100a712c = 1;
	}

	eyepoint->m_unk0xb8 = low2 / (g_unk0x100a712c * 160);
}

// FUNCTION: MW2 0x1004bf61
void FUN_1004bf61(Eyepoint* p_eyepoint, MechS32 p_value)
{
	p_eyepoint->m_unk0x3c = g_unk0x100ea8d0 = p_value;
	g_unk0x100ea820 = p_value << 2;
}

// FUNCTION: MW2 0x1004bf8a
void FUN_1004bf8a(Eyepoint* p_eyepoint, MechS32 p_value)
{
	p_eyepoint->m_unk0x40 = g_unk0x100ea860 = p_value;
	if (p_value < 0x1fffffff) {
		g_unk0x100ea82c = p_value << 2;
		p_eyepoint->m_unk0xb4 = p_value;
	}
	else {
		g_unk0x100ea82c = 0x7fffffff;
		p_eyepoint->m_unk0xb4 = 0x7fffffff;
	}
}

// Builds the eyepoint's view rotation (at 0x54, transposed) and position (0x78) from its
// position (0x00) and rotation (0x0c).
// FUNCTION: MW2 0x1004bfe8
void FUN_1004bfe8(Eyepoint* p_eyepoint)
{
	Matrix matrix;

	FUN_1000e2b9(
		&matrix,
		p_eyepoint->m_unk0x10,
		p_eyepoint->m_unk0x0c,
		p_eyepoint->m_unk0x14,
		p_eyepoint->m_unk0x00,
		p_eyepoint->m_unk0x04,
		p_eyepoint->m_unk0x08
	);
	FUN_1000dc33(&matrix, &p_eyepoint->m_unk0x54);
	p_eyepoint->m_unk0x54.m_rows[3][0] = matrix.m_rows[3][0];
	p_eyepoint->m_unk0x54.m_rows[3][1] = matrix.m_rows[3][1];
	p_eyepoint->m_unk0x54.m_rows[3][2] = matrix.m_rows[3][2];
}

// FUNCTION: MW2 0x1004c05c
void FUN_1004c05c(Eyepoint* p_eyepoint)
{
	p_eyepoint->m_unk0x4c = 0;
	p_eyepoint->m_unk0x50 = 0;
	FUN_1004bc2e(p_eyepoint);
	FUN_1004bfe8(p_eyepoint);
}

// Sets the eyepoint's view from a transform.
// FUNCTION: MW2 0x1004c093
void FUN_1004c093(Eyepoint* p_eyepoint, Matrix* p_matrix)
{
	FUN_1000dc33(p_matrix, &p_eyepoint->m_unk0x54);
	p_eyepoint->m_unk0x54.m_rows[3][0] = p_matrix->m_rows[3][0];
	p_eyepoint->m_unk0x54.m_rows[3][1] = p_matrix->m_rows[3][1];
	p_eyepoint->m_unk0x54.m_rows[3][2] = p_matrix->m_rows[3][2];
}

// Gets the eyepoint's view as a transform.
// FUNCTION: MW2 0x1004c0d8
void FUN_1004c0d8(Eyepoint* p_eyepoint, Matrix* p_matrix)
{
	FUN_1000dc33(&p_eyepoint->m_unk0x54, p_matrix);
	p_matrix->m_rows[3][0] = p_eyepoint->m_unk0x54.m_rows[3][0];
	p_matrix->m_rows[3][1] = p_eyepoint->m_unk0x54.m_rows[3][1];
	p_matrix->m_rows[3][2] = p_eyepoint->m_unk0x54.m_rows[3][2];
}

// Projects the point (p_x, p_y, p_z) through the eyepoint, in place.
// STUB: MW2 0x1004c11d
MechS32 FUN_1004c11d(MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	STUB(0x1004c11d);
	return 0;
}

// FUNCTION: MW2 0x1004c779
MechS32 FUN_1004c779(MechU16* p_flags)
{
	if (*p_flags & 0x1000) {
		return TRUE;
	}

	return FALSE;
}

// FUNCTION: MW2 0x1004c7a6
MechS32 FUN_1004c7a6(undefined4 p_unk0x00)
{
	return g_unk0x100a712c == 1;
}

// FUNCTION: MW2 0x1004c7cf
void FUN_1004c7cf(undefined4 p_unk0x00, MechS32 p_enable)
{
	if (p_enable) {
		g_unk0x100a712c = 1;
	}
	else {
		g_unk0x100a712c = 2;
	}
}
