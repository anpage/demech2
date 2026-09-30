#include "unk10042e00.h"

#include "animation.h"
#include "decomp.h"
#include "eyepoint.h"
#include "fixeddivu.h"
#include "fixedmul.h"
#include "fixedmul30.h"
#include "rendertarget.h"
#include "simmain.h"
#include "slateheron.h"
#include "types.h"
#include "unk10010750.h"
#include "unk10036230.h"

// Polygon drawing: a polygon is p_count points of 6 dwords each (x, y, a shade, two texture
// coordinates and a depth), and the mode in bits 12 to 14 of p_flags picks how it is drawn.

// The end of the last line FUN_10044590 drew.
// GLOBAL: MW2 0x100be5d4
MechS32 g_unk0x100be5d4;

// GLOBAL: MW2 0x100be5d8
MechS32 g_unk0x100be5d8;

// Stack-slot permutation: luma, saved, mode, index, scale, shade and fraction.
// FUNCTION: MW2 0x10042e00
void FUN_10042e00(MechS32 p_count, MechU32* p_points, MechU32 p_flags)
{
	MechS32 luma;
	MechS32 saved;
	MechU32 mode;
	MechS32 i;
	MechU32* point;
	MechS32 index;
	MechS32 scale;
	MechS32 shade;
	MechS32 fraction;

	mode = p_flags & 0x7000;
	point = p_points;
	switch (mode) {
	case 0:
	case 0x1000:
		p_flags &= 0xff;
		for (i = 0; i < p_count; i++) {
			point[2] = p_flags << 16;
			point += 6;
		}

		FUN_10036918(&g_currentRenderTarget, p_count, p_points);
		break;
	case 0x2000:
		p_flags &= 0xff;
		g_unk0x100be5d4 = p_points[0];
		g_unk0x100be5d8 = p_points[1];
		for (i = 1; i < p_count; i++) {
			FUN_10044590(p_points[i * 6], p_points[i * 6 + 1], p_flags);
		}

		FUN_10044590(p_points[0], p_points[1], p_flags);
		break;
	case 0x4000:
		p_flags &= 0xff;
		for (i = 0; i < p_count; i++) {
			if ((MechS32) point[3] < 0x300000) {
				point[2] = point[3];
			}
			else {
				shade = point[3];
				fraction = shade & 0xf0000;
				scale = ((p_flags & 0xf) + 1) << 12;
				fraction = FixedMul16(fraction, scale);
				point[2] = (shade & 0xfff00000) + fraction;
			}

			point += 6;
		}

		if (g_unk0x100a6cc8.m_unk0x04) {
			FUN_1003763b(&g_currentRenderTarget, 0x7fff, p_count, p_points);
		}
		else {
			FUN_10036918(&g_currentRenderTarget, p_count, p_points);
		}
		break;
	case 0x3000:
		FUN_10010750(p_flags, p_count, p_points, -1);
		break;
	case 0x5000:
		if (!g_unk0x100a6cc8.m_unk0x0c) {
			break;
		}

		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		if (!g_unk0x100a6cc8.m_unk0x4c) {
			for (i = 0; i < p_count; i++) {
				point[5] = FixedDivU16(g_eyepoint->m_unk0x94, point[5]);
				point[3] = FixedMul30(point[3], point[5]);
				point[4] = FixedMul30(point[4], point[5]);
				point += 6;
			}
		}

		FUN_10068d10(index, p_count, p_points, luma, 0, 1);
		break;
	case 0x6000:
		if (!g_unk0x100a6cc8.m_unk0x0c) {
			break;
		}

		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		saved = g_unk0x100a6cc8.m_unk0x4c;
		g_unk0x100a6cc8.m_unk0x4c = TRUE;
		FUN_10068d10(index, p_count, p_points, luma, 0, 1);
		g_unk0x100a6cc8.m_unk0x4c = saved;
		break;
	case 0x7000:
		if (!g_unk0x100a6cc8.m_unk0x0c) {
			break;
		}

		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		FUN_10068d10(index, p_count, p_points, luma, 0, 0);
		break;
	default:
		break;
	}
}

// Draws the scene from p_eyepoint.
// STUB: MW2 0x1004320b
void FUN_1004320b(Eyepoint* p_eyepoint)
{
	STUB(0x1004320b);
}

// Draws a closed polygon of five points.
// FUNCTION: MW2 0x1004440d
void FUN_1004440d(
	MechS32 p_x0,
	MechS32 p_y0,
	MechS32 p_x1,
	MechS32 p_y1,
	MechS32 p_x2,
	MechS32 p_y2,
	MechS32 p_x3,
	MechS32 p_y3,
	MechS32 p_x4,
	MechS32 p_y4,
	MechU32 p_flags
)
{
	MechU32 points[5 * 6];

	points[0] = points[3] = p_x0;
	points[1] = points[4] = p_y0;
	points[6] = points[9] = p_x1;
	points[7] = points[10] = p_y1;
	points[12] = points[15] = p_x2;
	points[13] = points[16] = p_y2;
	points[18] = points[21] = p_x3;
	points[19] = points[22] = p_y3;
	points[24] = points[27] = p_x4;
	points[25] = points[28] = p_y4;
	g_unk0x100a6cc8.m_drawPolygon(5, points, p_flags);
}

// FUNCTION: MW2 0x100444a6
void FUN_100444a6(
	MechS32 p_x0,
	MechS32 p_y0,
	MechS32 p_x1,
	MechS32 p_y1,
	MechS32 p_x2,
	MechS32 p_y2,
	MechS32 p_x3,
	MechS32 p_y3,
	MechU32 p_flags
)
{
	MechU32 points[4 * 6];

	points[0] = points[3] = p_x0;
	points[1] = points[4] = p_y0;
	points[6] = points[9] = p_x1;
	points[7] = points[10] = p_y1;
	points[12] = points[15] = p_x2;
	points[13] = points[16] = p_y2;
	points[18] = points[21] = p_x3;
	points[19] = points[22] = p_y3;
	g_unk0x100a6cc8.m_drawPolygon(4, points, p_flags);
}

// FUNCTION: MW2 0x10044527
void FUN_10044527(MechS32 p_x0, MechS32 p_y0, MechS32 p_x1, MechS32 p_y1, MechS32 p_x2, MechS32 p_y2, MechU32 p_flags)
{
	MechU32 points[3 * 6];

	points[0] = points[3] = p_x0;
	points[1] = points[4] = p_y0;
	points[6] = points[9] = p_x1;
	points[7] = points[10] = p_y1;
	points[12] = points[15] = p_x2;
	points[13] = points[16] = p_y2;
	g_unk0x100a6cc8.m_drawPolygon(3, points, p_flags);
}

// Draws a line from the end of the last one to (p_x, p_y).
// FUNCTION: MW2 0x10044590
void FUN_10044590(MechS32 p_x, MechS32 p_y, MechU32 p_color)
{
	BlitLine(&g_currentRenderTarget, p_x, p_y, g_unk0x100be5d4, g_unk0x100be5d8, 0, p_color);
	g_unk0x100be5d4 = p_x;
	g_unk0x100be5d8 = p_y;
}

// Draws a polygon: a point or a line with the render target's own routines where the settings
// allow, else through g_unk0x100a6cc8's polygon callback, filled, outlined or both.
// FUNCTION: MW2 0x100445d2
void FUN_100445d2(MechS32 p_count, MechU32* p_points, MechU32 p_flags)
{
	if (p_count == 1 && g_unk0x100a6cc8.m_unk0x18) {
		if (p_flags == 0x1000) {
			return;
		}

		PutViewPixel(&g_currentRenderTarget, p_points[0], p_points[1], p_flags);
	}
	else if (p_count == 2 && g_unk0x100a6cc8.m_unk0x14) {
		BlitLine(&g_currentRenderTarget, p_points[0], p_points[1], p_points[6], p_points[7], 0, p_flags);
	}
	else if (g_unk0x100a6cc8.m_unk0x34 == 0) {
		g_unk0x100a6cc8.m_drawPolygon(p_count, p_points, p_flags);
	}
	else if (g_unk0x100a6cc8.m_unk0x34 == 1) {
		g_unk0x100a6cc8.m_drawPolygon(p_count, p_points, 0);
		g_unk0x100a6cc8.m_drawPolygon(p_count, p_points, p_flags | 0x2000);
	}
	else {
		g_unk0x100a6cc8.m_drawPolygon(p_count, p_points, p_flags | 0x2000);
	}
}
