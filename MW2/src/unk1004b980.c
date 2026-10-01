#include "unk1004b980.h"

#include "decomp.h"
#include "eyepoint.h"
#include "fixeddot27.h"
#include "fixeddot29.h"
#include "fixedmul.h"
#include "muldiv.h"
#include "simmain.h"
#include "transform.h"
#include "types.h"
#include "unk10004ec0.h"
#include "unk10019ad0.h"
#include "unk100335d0.h"
#include "unk10036230.h"
#include "unk1003a530.h"
#include "unk10042740.h"
#include "unk10046750.h"
#include "unk1004c800.h"
#include "unk1004c820.h"
#include "unk1004c860.h"

#include <windows.h>

// GLOBAL: MW2 0x100a712c
MechS32 g_unk0x100a712c = 1;

// GLOBAL: MW2 0x100ea820
MechS32 g_unk0x100ea820;

// GLOBAL: MW2 0x100ea824
MechS32 g_unk0x100ea824;

// GLOBAL: MW2 0x100ea828
MechS32 g_unk0x100ea828;

// GLOBAL: MW2 0x100ea82c
MechS32 g_unk0x100ea82c;

// GLOBAL: MW2 0x100ea830
MechS32 g_unk0x100ea830;

// GLOBAL: MW2 0x100ea834
MechS32 g_unk0x100ea834;

// GLOBAL: MW2 0x100ea838
MechS32 g_unk0x100ea838;

// GLOBAL: MW2 0x100ea83c
MechS32 g_unk0x100ea83c;

// GLOBAL: MW2 0x100ea840
MechS32 g_unk0x100ea840;

// GLOBAL: MW2 0x100ea844
MechS32 g_unk0x100ea844;

// GLOBAL: MW2 0x100ea848
MechS32 g_unk0x100ea848;

// GLOBAL: MW2 0x100ea84c
MechS32 g_unk0x100ea84c;

// GLOBAL: MW2 0x100ea850
MechS32 g_unk0x100ea850;

// GLOBAL: MW2 0x100ea854
MechS32 g_unk0x100ea854;

// GLOBAL: MW2 0x100ea858
MechS32 g_unk0x100ea858;

// GLOBAL: MW2 0x100ea85c
MechS32 g_unk0x100ea85c;

// GLOBAL: MW2 0x100ea860
MechS32 g_unk0x100ea860;

// GLOBAL: MW2 0x100ea864
MechS32 g_unk0x100ea864;

// GLOBAL: MW2 0x100ea868
MechS32 g_unk0x100ea868;

// GLOBAL: MW2 0x100ea86c
MechS32 g_unk0x100ea86c;

// GLOBAL: MW2 0x100ea870
MechS32 g_unk0x100ea870;

// GLOBAL: MW2 0x100ea874
MechS32 g_unk0x100ea874;

// GLOBAL: MW2 0x100ea878
MechS32 g_unk0x100ea878;

// GLOBAL: MW2 0x100ea87c
MechS32 g_unk0x100ea87c;

// GLOBAL: MW2 0x100ea880
MechS32 g_unk0x100ea880;

// GLOBAL: MW2 0x100ea884
MechS32 g_unk0x100ea884;

// GLOBAL: MW2 0x100ea888
MechS32 g_unk0x100ea888;

// GLOBAL: MW2 0x100ea88c
MechS32 g_unk0x100ea88c;

// GLOBAL: MW2 0x100ea890
MechS32 g_unk0x100ea890;

// GLOBAL: MW2 0x100ea894
MechS32 g_unk0x100ea894;

// GLOBAL: MW2 0x100ea898
MechS32 g_unk0x100ea898;

// GLOBAL: MW2 0x100ea89c
MechS32 g_unk0x100ea89c;

// GLOBAL: MW2 0x100ea8a0
MechS32 g_unk0x100ea8a0;

// GLOBAL: MW2 0x100ea8a4
MechS32 g_unk0x100ea8a4;

// GLOBAL: MW2 0x100ea8a8
MechS32 g_unk0x100ea8a8;

// GLOBAL: MW2 0x100ea8ac
MechS32 g_unk0x100ea8ac;

// GLOBAL: MW2 0x100ea8b0
MechS32 g_unk0x100ea8b0;

// GLOBAL: MW2 0x100ea8b4
MechS32 g_unk0x100ea8b4;

// GLOBAL: MW2 0x100ea8b8
MechS32 g_unk0x100ea8b8;

// GLOBAL: MW2 0x100ea8bc
MechS32 g_unk0x100ea8bc;

// GLOBAL: MW2 0x100ea8c0
MechS32 g_unk0x100ea8c0;

// GLOBAL: MW2 0x100ea8c4
MechS32 g_unk0x100ea8c4;

// GLOBAL: MW2 0x100ea8c8
MechS32 g_unk0x100ea8c8;

// GLOBAL: MW2 0x100ea8cc
MechS32 g_unk0x100ea8cc;

// GLOBAL: MW2 0x100ea8d0
MechS32 g_unk0x100ea8d0;

// GLOBAL: MW2 0x100ea8d4
MechS32 g_unk0x100ea8d4;

// Makes p_eyepoint the current eyepoint and copies what the renderer uses each frame out of it:
// its rotation (also scaled by the projection factors), position, view rectangle and shading.
// FUNCTION: MW2 0x1004b980
void FUN_1004b980(Eyepoint* p_eyepoint)
{
	Eyepoint* eyepoint;

	g_eyepoint = eyepoint = p_eyepoint;
	g_unk0x1010b540 = eyepoint->m_unk0x2a;
	g_unk0x1010b530 = eyepoint->m_unk0x28;
	g_unk0x100ea888 = eyepoint->m_unk0x9c;
	g_unk0x100ea88c = eyepoint->m_unk0xa0;
	g_unk0x100ea844 = eyepoint->m_unk0x94;
	g_unk0x100ea848 = eyepoint->m_unk0x98;
	g_unk0x100ea890 = eyepoint->m_unk0x54.m_rows[0][0];
	g_unk0x100ea894 = eyepoint->m_unk0x54.m_rows[0][1];
	g_unk0x100ea898 = eyepoint->m_unk0x54.m_rows[0][2];
	g_unk0x100ea89c = eyepoint->m_unk0x54.m_rows[1][0];
	g_unk0x100ea8a0 = eyepoint->m_unk0x54.m_rows[1][1];
	g_unk0x100ea8a4 = eyepoint->m_unk0x54.m_rows[1][2];
	g_unk0x100ea87c = g_unk0x100ea8a8 = eyepoint->m_unk0x54.m_rows[2][0];
	g_unk0x100ea880 = g_unk0x100ea8ac = eyepoint->m_unk0x54.m_rows[2][1];
	g_unk0x100ea884 = g_unk0x100ea8b0 = eyepoint->m_unk0x54.m_rows[2][2];
	g_unk0x100ea864 = FixedMul16(g_unk0x100ea888, g_unk0x100ea890);
	g_unk0x100ea868 = FixedMul16(g_unk0x100ea888, g_unk0x100ea894);
	g_unk0x100ea86c = FixedMul16(g_unk0x100ea888, g_unk0x100ea898);
	g_unk0x100ea870 = FixedMul16(g_unk0x100ea88c, g_unk0x100ea89c);
	g_unk0x100ea874 = FixedMul16(g_unk0x100ea88c, g_unk0x100ea8a0);
	g_unk0x100ea878 = FixedMul16(g_unk0x100ea88c, g_unk0x100ea8a4);
	g_unk0x100ea8b8 = eyepoint->m_unk0x54.m_rows[3][0];
	g_unk0x100ea8b4 = eyepoint->m_unk0x54.m_rows[3][1];
	g_unk0x100ea8bc = eyepoint->m_unk0x54.m_rows[3][2];
	g_unk0x100ea8c4 = eyepoint->m_unk0x1c;
	g_unk0x100ea8c8 = eyepoint->m_unk0x20;
	g_unk0x100ea8c0 = eyepoint->m_unk0x24;
	g_unk0x100ea8d0 = eyepoint->m_unk0x3c;
	g_unk0x100ea830 = eyepoint->m_unk0x2c;
	g_unk0x100ea860 = eyepoint->m_unk0x40;
	g_unk0x100ea850 = eyepoint->m_unk0x34;
	g_unk0x100ea84c = eyepoint->m_unk0x30;
	g_unk0x100ea840 = eyepoint->m_unk0x38;
	g_unk0x100ea82c = g_unk0x100ea860 << 2;
	g_unk0x100ea820 = g_unk0x100ea8d0 << 2;
	g_unk0x100ea8cc = g_unk0x100ea850 << 2;
	g_unk0x100ea85c = g_unk0x100ea840 << 2;
	g_unk0x100ea854 = g_unk0x100ea830 << 2;
	g_unk0x100ea8d4 = g_unk0x100ea84c << 2;
	g_unk0x100ea83c = eyepoint->m_halfWidth;
	g_unk0x100ea838 = eyepoint->m_halfHeight;
	g_unk0x100ea834 = eyepoint->m_centerX;
	g_unk0x100ea858 = eyepoint->m_centerY;
	g_unk0x100ea824 = eyepoint->m_unk0xa4;
	g_unk0x100ea828 = eyepoint->m_unk0xa6;
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
// Projects the world point (*p_x, *p_y, *p_z) onto the screen in place (*p_z the depth). A point
// at or behind the near plane is projected mirrored. Returns whether it is in front and on the
// screen.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004c11d
MechS32 FUN_1004c11d(MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 dx;
	MechS32 dy;
	MechS32 behind;
	MechS32 dz;

	x = *p_x;
	y = *p_y;
	z = *p_z;
	dx = x - g_unk0x100ea8b8;
	dy = y - g_unk0x100ea8b4;
	dz = z - g_unk0x100ea8bc;
	x = FixedDot27(dx, g_unk0x100ea864, dy, g_unk0x100ea868, dz, g_unk0x100ea86c);
	y = FixedDot27(dx, g_unk0x100ea870, dy, g_unk0x100ea874, dz, g_unk0x100ea878);
	z = FixedDot27(dx, g_unk0x100ea87c, dy, g_unk0x100ea880, dz, g_unk0x100ea884);
	*p_z = z;
	if (z <= g_unk0x100ea820) {
		behind = TRUE;
		if (z < 0) {
			z = -z;
		}
		else if (z == 0) {
			z = 1;
		}
	}
	else {
		behind = FALSE;
	}

	*p_x = FUN_10042740(x, z, g_unk0x100ea824, g_unk0x100ea834);
	*p_y = g_unk0x100ea840 - g_unk0x100ea850 - FUN_10042740(y, z, g_unk0x100ea828, g_unk0x100ea858);
	if (behind) {
		return 0;
	}

	return *p_x >= g_unk0x100ea830 && *p_x <= g_unk0x100ea84c && *p_y >= g_unk0x100ea850 && *p_y <= g_unk0x100ea840;
}

// The scene's shape filter (SlateHeron0x68::m_unk0x58): culls a shape against the view frustum,
// like FUN_10042206 the map view's. 1: a shape of kind 0xa0 with g_unk0x100a2420, 5: out of
// range or past the far plane, 4: in front of the near plane, 6 and 7: outside the side planes.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004c2ef
MechS32 FUN_1004c2ef(ScarletOrchid0x4c* p_shape)
{
	MechS32 y;
	MechS32 z;
	MechS32 radius;
	MechS32 limit;
	MechS32 dx;
	MechS32 side;
	MechS32 dy;
	MechS32 height;
	MechS32 fov;
	MechS32 dz;
	MechS32 depth;
	MechS32 x;

	if (g_unk0x100a2420 && (p_shape->m_unk0x02 & 0xf0) == 0xa0) {
		return 1;
	}

	x = p_shape->m_unk0x34;
	y = p_shape->m_unk0x38;
	z = p_shape->m_unk0x3c;
	radius = p_shape->m_unk0x40;
	dx = x - g_unk0x100ea8b8;
	dy = y - g_unk0x100ea8b4;
	dz = z - g_unk0x100ea8bc;
	if (!FUN_10004ec0(dx, dy, dz, g_eyepoint->m_unk0xb4 + radius)) {
		return 5;
	}

	depth = g_unk0x1010b5a4 = FixedDot29(dx, g_unk0x100ea8a8, dy, g_unk0x100ea8ac, dz, g_unk0x100ea8b0);
	if (radius + depth < g_unk0x100ea8d0) {
		return 4;
	}

	if (depth - radius > g_unk0x100ea860) {
		return 5;
	}

	side = FixedDot29(dx, g_unk0x100ea890, dy, g_unk0x100ea894, dz, g_unk0x100ea898);
	fov = g_eyepoint->m_fovX;
	if (side > 0) {
		limit = FUN_1004c860(fov, side, -depth, g_eyepoint->m_unk0xa8);
	}
	else {
		limit = FUN_1004c860(fov, -side, -depth, g_eyepoint->m_unk0xa8);
	}

	if (limit > radius) {
		return 6;
	}

	height = FixedDot29(dx, g_unk0x100ea89c, dy, g_unk0x100ea8a0, dz, g_unk0x100ea8a4);
	fov = g_eyepoint->m_fovY;
	if (height > 0) {
		limit = FUN_1004c860(fov, height, -depth, g_eyepoint->m_unk0xac);
	}
	else {
		limit = FUN_1004c860(fov, -height, -depth, g_eyepoint->m_unk0xac);
	}

	if (limit > radius) {
		return 7;
	}

	return 0;
}

// Culls a shape against the view frustum like FUN_1004c2ef, without its range test. 1 for a
// hidden shape (bit 0x1000).
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004c565
MechS32 FUN_1004c565(ScarletOrchid0x4c* p_shape)
{
	MechS32 y;
	MechS32 z;
	MechS32 radius;
	MechS32 limit;
	MechS32 dx;
	MechS32 side;
	MechS32 dy;
	MechS32 height;
	MechS32 fov;
	MechS32 dz;
	MechS32 depth;
	MechS32 x;

	x = p_shape->m_unk0x34;
	y = p_shape->m_unk0x38;
	z = p_shape->m_unk0x3c;
	radius = p_shape->m_unk0x40;
	dx = x - g_unk0x100ea8b8;
	dy = y - g_unk0x100ea8b4;
	dz = z - g_unk0x100ea8bc;
	if (p_shape->m_unk0x00 & 0x1000) {
		return 1;
	}

	depth = g_unk0x1010b5a4 = FixedDot29(dx, g_unk0x100ea8a8, dy, g_unk0x100ea8ac, dz, g_unk0x100ea8b0);
	if (radius + depth < g_unk0x100ea8d0) {
		return 4;
	}

	side = FixedDot29(dx, g_unk0x100ea890, dy, g_unk0x100ea894, dz, g_unk0x100ea898);
	fov = g_eyepoint->m_fovX;
	if (side > 0) {
		limit = FUN_1004c860(fov, side, -depth, g_eyepoint->m_unk0xa8);
	}
	else {
		limit = FUN_1004c860(fov, -side, -depth, g_eyepoint->m_unk0xa8);
	}

	if (limit > radius) {
		return 6;
	}

	height = FixedDot29(dx, g_unk0x100ea89c, dy, g_unk0x100ea8a0, dz, g_unk0x100ea8a4);
	fov = g_eyepoint->m_fovY;
	if (height > 0) {
		limit = FUN_1004c860(fov, height, -depth, g_eyepoint->m_unk0xac);
	}
	else {
		limit = FUN_1004c860(fov, -height, -depth, g_eyepoint->m_unk0xac);
	}

	if (limit > radius) {
		return 7;
	}

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
