#ifdef MW2_MATROX
#define FIXEDTRIG_FLOAT_SINE /* the Matrox edition's sine (fixedtrig.h) */
#endif
#include "polydraw.h"

#include "animation.h"
#include "bandpoly.h"
#include "collision.h"
#include "decomp.h"
#include "eyepoint.h"
#include "fixeddivu.h"
#include "fixedfloat.h"
#include "fixedmul.h"
#include "fixedmul30.h"
#include "fixedtrig.h"
#include "horizon.h"
#include "render.h"
#include "rendersettings.h"
#include "targeting.h"
#include "transform.h"
#include "types.h"
#include "vfx3d.h"
#include "vfxa.h"

#ifdef MW2_MATROX
#include "debugprint.h"
#include "matrox/a3d.h"
#include "matrox/skycorner.h"
#include "matrox/vfx16.h"
#include "objectanim.h"
#include "palette.h"
#include "vertex.h"

#include <stdio.h>
#include <string.h>
#endif

#ifdef MW2_MATROX
// The Matrox edition's textured sky and ground (skygnd.par): the sky's texture repeats every
// g_skyTileU by g_skyTileV, the ground's every g_groundTile, and the sky's texture scrolls from
// g_skyScroll to g_skyScrollEnd by g_skyScrollSpeed a frame.

// GLOBAL: MW2MATROX 0x100addc0
MechFloat g_skyTileU = 12.0f;

// GLOBAL: MW2MATROX 0x100addc4
MechFloat g_skyTileV = 2.4f;

// GLOBAL: MW2MATROX 0x100addc8
MechFloat g_groundTile = 50.0f;

// GLOBAL: MW2MATROX 0x100addcc
MechFloat g_skyScroll = 0;
#endif

// GLOBAL: MW2 0x100a6be0
// GLOBAL: MW2MATROX 0x100addd0
Eyepoint g_mainEyepoint = {0,       0, 0, 0, 0, 0,  FIXED_CONST(1), 1000, 10000, -1000, 1, 0x48, 0, 319, 0, 199, 0x40,
						   0x249f0, 0, 0, 0, 0, {0}};

// GLOBAL: MW2 0x100a6cc0
// GLOBAL: MW2MATROX 0x100ade74
Eyepoint* g_eyepoint = &g_mainEyepoint;

// GLOBAL: MW2 0x100a6cc8
// GLOBAL: MW2MATROX 0x100ade78
RenderSettings g_renderSettings = {
	0,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
#ifdef MW2_MATROX
	1,
	1,
#endif
	{0xe0, 0xef},
	1,
	0,
	0,
	0,
	0,
	0x186a0,
	0x10000,
	0,
	0,
	NULL,
	NULL,
	NULL,
	0,
	NULL
};

// GLOBAL: MW2 0x100a6d30
// GLOBAL: MW2MATROX 0x100adee8
MechS32 g_horizonBandHeight = 0x24;

#ifdef MW2_MATROX
// GLOBAL: MW2MATROX 0x100adeec
MechS32 g_unk0x100adeec = 0;

// The textured sky's four quads, four corners each: a position, and whether the corner takes the
// texture's far edge in u and in v.
// GLOBAL: MW2MATROX 0x100adef0
SkyCorner g_skyCorners[16] = {
	{-64000000.0f, 2200000.0f, -64000000.0f, 0, 0},
	{64000000.0f, 2200000.0f, -64000000.0f, 1, 0},
	{76800000.0f, -400000.0f, -76800000.0f, 1, 1},
	{-76800000.0f, -400000.0f, -76800000.0f, 0, 1},
	{-64000000.0f, 2200000.0f, 64000000.0f, 0, 0},
	{64000000.0f, 2200000.0f, 64000000.0f, 1, 0},
	{76800000.0f, -400000.0f, 76800000.0f, 1, 1},
	{-76800000.0f, -400000.0f, 76800000.0f, 0, 1},
	{-64000000.0f, 2200000.0f, -64000000.0f, 0, 0},
	{-64000000.0f, 2200000.0f, 64000000.0f, 1, 0},
	{-76800000.0f, -400000.0f, 76800000.0f, 1, 1},
	{-76800000.0f, -400000.0f, -76800000.0f, 0, 1},
	{64000000.0f, 2200000.0f, -64000000.0f, 0, 0},
	{64000000.0f, 2200000.0f, 64000000.0f, 1, 0},
	{76800000.0f, -400000.0f, 76800000.0f, 1, 1},
	{76800000.0f, -400000.0f, -76800000.0f, 0, 1}
};

// DrawSkyAndGround has read the sky's and the ground's colors.
// GLOBAL: MW2MATROX 0x100ae030
MechS32 g_skyColorsRead = 0;

// The sky's and the ground's colors (0-255 per component), from their textures (DrawSkyAndGround).
// GLOBAL: MW2MATROX 0x1015d5d8
MechFloat g_skyBlue;

// GLOBAL: MW2MATROX 0x1015d5dc
MechFloat g_skyGreen;

// GLOBAL: MW2MATROX 0x1015d5e0
MechFloat g_skyRed;

// GLOBAL: MW2MATROX 0x1015d5e4
MechFloat g_skyScrollEnd;

// GLOBAL: MW2MATROX 0x1015d5e8
MechFloat g_groundBlue;

// GLOBAL: MW2MATROX 0x1015d5ec
MechFloat g_groundGreen;

// GLOBAL: MW2MATROX 0x1015d5f0
MechFloat g_groundRed;

// The polygon the Matrox edition's clipper leaves.
// GLOBAL: MW2MATROX 0x1015d600
A3DVertex g_a3dClipped[16];

// GLOBAL: MW2MATROX 0x1015d900
MechFloat g_skyScrollSpeed;

// The polygon the Matrox edition hands its renderer.
// GLOBAL: MW2MATROX 0x1015d910
A3DVertex g_a3dVertices[16];

// The point count of g_a3dClipped.
// GLOBAL: MW2MATROX 0x1015dc10
MechU32 g_a3dClippedCount;

// The palette's colors as the renderer takes them (0-255 per component).
// GLOBAL: MW2MATROX 0x1015dcb0
MechFloat g_paletteRgb[256][3];

void FUN_10068856(
	MechS32 p_count,
	ProjectedVertex** p_points,
	MechFloat p_red,
	MechFloat p_green,
	MechFloat p_blue,
	MechS32 p_unk0x14
);
void FUN_100688fe(MechS32 p_count, ProjectedVertex** p_points, MechU32 p_color, MechS32 p_unk0x0c, MechS32 p_lit);
void FUN_100689f5(MechS32 p_count, ProjectedVertex** p_points, MechU32 p_color, MechS32 p_unk0x0c);
void FUN_10068b57(MechS32 p_count, ProjectedVertex** p_points, MechU32 p_color);
void FUN_10068e33(Eyepoint* p_eyepoint);
void FUN_10069172(Eyepoint* p_eyepoint);
void FUN_100692db(Eyepoint* p_eyepoint);

// The pane's width and height, as FUN_10067b90 clips to them.
#define CLIP_WIDTH (g_currentPane.m_x1 - g_currentPane.m_x0 + 1)
#define CLIP_HEIGHT (g_currentPane.m_y1 - g_currentPane.m_y0 + 1)

// Clips the polygon of the p_count first vertices of g_a3dVertices to the pane's edges in
// p_edges (1 left, 2 right, 4 top, 8 bottom), adding the vertices on the edges after them
// (FUN_10066a50), and copies the result to g_a3dClipped. Returns its point count, -1 if nothing
// is left, or 0 without p_edges. Nothing calls it. The only diff is a stack-slot permutation of
// the locals.
// FUNCTION: MW2MATROX 0x10067b90
MechS32 FUN_10067b90(MechU32 p_count, MechU32 p_edges, MechS32 p_unk0x08)
{
	A3DVertex* next;
	MechU32 passes;
	A3DVertex** prev;
	A3DVertex* cur;
	A3DVertex** out;
	MechU32 added;
	A3DVertex* listB[256];
	MechU32 i;
	MechU32 outCount;
	A3DVertex** in;
	A3DVertex* listA[256];

	passes = 0;
	added = p_count;
	for (i = 0; i < p_count; i++) {
		listA[i] = &g_a3dVertices[i];
	}

	prev = &listA[p_count - 1];
	in = listA;
	out = listB;
	outCount = 0;
	if (p_edges & 2) {
		for (i = p_count; i > 0; i--, in++) {
			cur = *prev;
			next = *in;
			if (cur->m_x <= CLIP_WIDTH) {
				*out = *prev;
				out++;
				outCount++;
			}

			if ((cur->m_x <= CLIP_WIDTH && next->m_x > CLIP_WIDTH) ||
				(cur->m_x > CLIP_WIDTH && next->m_x <= CLIP_WIDTH)) {
				FUN_10066a50(1, CLIP_WIDTH, next, cur, &g_a3dVertices[added], p_unk0x08);
				*out = &g_a3dVertices[added];
				added++;
				out++;
				outCount++;
			}

			prev = in;
		}

		if (!outCount) {
			return -1;
		}

		p_count = outCount;
		outCount = 0;
		passes++;
		if (passes & 1) {
			prev = out - 1;
			in = listB;
			out = listA;
		}
		else {
			prev = &listA[p_count - 1];
			in = listA;
			out = listB;
		}
	}

	if (p_edges & 4) {
		for (i = p_count; i > 0; i--, in++) {
			cur = *prev;
			next = *in;
			if (cur->m_y >= 0.0f) {
				*out = *prev;
				out++;
				outCount++;
			}

			if ((cur->m_y < 0.0f && next->m_y >= 0.0f) || (cur->m_y >= 0.0f && next->m_y < 0.0f)) {
				FUN_10066a50(2, 0, next, cur, &g_a3dVertices[added], p_unk0x08);
				*out = &g_a3dVertices[added];
				added++;
				out++;
				outCount++;
			}

			prev = in;
		}

		if (!outCount) {
			return -1;
		}

		p_count = outCount;
		outCount = 0;
		passes++;
		if (passes & 1) {
			prev = out - 1;
			in = listB;
			out = listA;
		}
		else {
			prev = &listA[p_count - 1];
			in = listA;
			out = listB;
		}
	}

	if (p_edges & 1) {
		for (i = p_count; i > 0; i--, in++) {
			cur = *prev;
			next = *in;
			if (cur->m_x >= 0.0f) {
				*out = *prev;
				out++;
				outCount++;
			}

			if ((cur->m_x < 0.0f && next->m_x >= 0.0f) || (cur->m_x >= 0.0f && next->m_x < 0.0f)) {
				FUN_10066a50(1, 0, next, cur, &g_a3dVertices[added], p_unk0x08);
				*out = &g_a3dVertices[added];
				added++;
				out++;
				outCount++;
			}

			prev = in;
		}

		if (!outCount) {
			return -1;
		}

		p_count = outCount;
		outCount = 0;
		passes++;
		if (passes & 1) {
			prev = out - 1;
			in = listB;
			out = listA;
		}
		else {
			prev = &listA[p_count - 1];
			in = listA;
			out = listB;
		}
	}

	if (p_edges & 8) {
		for (i = p_count; i > 0; i--, in++) {
			cur = *prev;
			next = *in;
			if (cur->m_y <= CLIP_HEIGHT) {
				*out = *prev;
				out++;
				outCount++;
			}

			if ((cur->m_y <= CLIP_HEIGHT && next->m_y > CLIP_HEIGHT) ||
				(cur->m_y > CLIP_HEIGHT && next->m_y <= CLIP_HEIGHT)) {
				FUN_10066a50(2, CLIP_HEIGHT, next, cur, &g_a3dVertices[added], p_unk0x08);
				*out = &g_a3dVertices[added];
				added++;
				out++;
				outCount++;
			}

			prev = in;
		}

		if (!outCount) {
			return -1;
		}

		p_count = outCount;
		passes++;
		if (passes & 1) {
			in = listB;
		}
		else {
			in = listA;
		}
	}

	if (passes) {
		for (i = 0; i < p_count; i++, in++) {
			g_a3dClipped[i] = **in;
		}

		g_a3dClippedCount = p_count;
		return p_count;
	}
	else {
		return 0;
	}
}
#endif

// Polygon drawing: a polygon is p_count points of 6 dwords each (x, y, a shade, two texture
// coordinates and a depth), and the mode in bits 12 to 14 of p_flags picks how it is drawn.

#ifndef MW2_MATROX
// The end of the last line DrawLineTo drew.
// GLOBAL: MW2 0x100be5d4
MechS32 g_lineEndX;

// GLOBAL: MW2 0x100be5d8
MechS32 g_lineEndY;
#endif

// Stack-slot permutation: luma, saved, mode, index, scale, shade and fraction.
// The Matrox edition's takes the projected vertices themselves, and modes in bits 12 to 15: 8 to 11
// and 0xf are its own (lit textures and animation colors), and it draws through its renderer.
// FUNCTION: MW2 0x10042e00
// FUNCTION: MW2MATROX 0x100683ba
#ifdef MW2_MATROX
void DrawScenePolygon(MechS32 p_count, ProjectedVertex** p_points, MechU32 p_flags, MechS32 p_unk0x0c)
#else
void DrawScenePolygon(MechS32 p_count, MechU32* p_points, MechU32 p_flags)
#endif
{
#ifdef MW2_MATROX
	MechFloat blue;
	MechFloat red;
	MechS32 index;
	MechFloat green;
	MechS32 luma;
	MechS32 saved;
	MechU32 mode;

	mode = p_flags & 0xf000;
	switch (mode) {
	case 0:
		p_flags &= 0xff;
		FUN_100688fe(p_count, p_points, p_flags, p_unk0x0c, FALSE);
		break;
	case 0x1000:
		p_flags &= 0xff;
		p_flags |= 0xf;
		FUN_100688fe(p_count, p_points, p_flags, p_unk0x0c, TRUE);
		break;
	case 0x2000:
		p_flags &= 0xff;
		FUN_10068b57(p_count, p_points, p_flags);
		break;
	case 0x4000:
		p_flags &= 0xff;
		FUN_100689f5(p_count, p_points, p_flags, p_unk0x0c);
		break;
	case 0x3000:
		index = (p_flags & 0xff0) >> 4;
		if (index >= 0x100) {
			index -= 0x100;
		}

		DrawBand(index, p_count, p_points, -1, 0, p_unk0x0c);
		break;
	case 0x5000:
		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 1, p_unk0x0c, 0);
		break;
	case 0x6000:
		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		saved = g_renderSettings.m_affineTextures;
		g_renderSettings.m_affineTextures = TRUE;
		DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 1, p_unk0x0c, 0);
		g_renderSettings.m_affineTextures = saved;
		break;
	case 0x7000:
		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		if (index == 0x8c) {
			if (g_renderSettings.m_texturedSky) {
				DrawAnimatedPolygon(
					index,
					p_count,
					p_points,
					luma,
					(MechS32) (g_skyScroll * 100000.0f),
					0,
					p_unk0x0c,
					(MechS32) (-g_eyepoint->m_heading - g_eyepoint->m_pitch)
				);
			}
			else {
				FUN_100688fe(p_count, p_points, g_skyColor, p_unk0x0c, TRUE);
			}
		}
		else {
			DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 0, p_unk0x0c, 0);
		}
		break;
	case 0x8000:
		luma = 0xf;
		index = p_flags & 0xff;
		DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 0, p_unk0x0c, 1);
		break;
	case 0x9000:
		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 1, p_unk0x0c, 1);
		break;
	case 0xa000:
		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		saved = g_renderSettings.m_affineTextures;
		g_renderSettings.m_affineTextures = TRUE;
		DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 1, p_unk0x0c, 1);
		g_renderSettings.m_affineTextures = saved;
		break;
	case 0xb000:
		luma = 0x80000000;
		index = p_flags & 0xff;
		DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 1, p_unk0x0c, 1);
		break;
	case 0xf000:
		FUN_10051ba1((p_flags & 0xff) + 0x100, &red, &green, &blue);
		FUN_10068856(p_count, p_points, red, green, blue, p_unk0x0c);
		break;
	default:
		break;
	}
#else
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

		VFX_flat_polygon(&g_currentPane, p_count, p_points);
		break;
	case 0x2000:
		p_flags &= 0xff;
		g_lineEndX = p_points[0];
		g_lineEndY = p_points[1];
		for (i = 1; i < p_count; i++) {
			DrawLineTo(p_points[i * 6], p_points[i * 6 + 1], p_flags);
		}

		DrawLineTo(p_points[0], p_points[1], p_flags);
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

		if (g_renderSettings.m_gouraud) {
			VFX_dithered_Gouraud_polygon(&g_currentPane, 0x7fff, p_count, p_points);
		}
		else {
			VFX_flat_polygon(&g_currentPane, p_count, p_points);
		}
		break;
	case 0x3000:
		DrawBandPolygon(p_flags, p_count, p_points, -1);
		break;
	case 0x5000:
		if (!g_renderSettings.m_textures) {
			break;
		}

		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		if (!g_renderSettings.m_affineTextures) {
			for (i = 0; i < p_count; i++) {
				point[5] = FixedDivU16(g_eyepoint->m_projectScaleX, point[5]);
				point[3] = FixedMul30(point[3], point[5]);
				point[4] = FixedMul30(point[4], point[5]);
				point += 6;
			}
		}

		DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 1);
		break;
	case 0x6000:
		if (!g_renderSettings.m_textures) {
			break;
		}

		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		saved = g_renderSettings.m_affineTextures;
		g_renderSettings.m_affineTextures = TRUE;
		DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 1);
		g_renderSettings.m_affineTextures = saved;
		break;
	case 0x7000:
		if (!g_renderSettings.m_textures) {
			break;
		}

		luma = (p_flags & 0xf00) >> 8;
		index = p_flags & 0xff;
		DrawAnimatedPolygon(index, p_count, p_points, luma, 0, 0);
		break;
	default:
		break;
	}
#endif
}

#ifdef MW2_MATROX
// Draws a polygon in the color p_red, p_green, p_blue, lit by each vertex's light.
// FUNCTION: MW2MATROX 0x10068856
void FUN_10068856(
	MechS32 p_count,
	ProjectedVertex** p_points,
	MechFloat p_red,
	MechFloat p_green,
	MechFloat p_blue,
	MechS32 p_unk0x14
)
{
	ProjectedVertex* point;
	A3DVertex* vertex;
	MechS32 i;

	vertex = g_a3dVertices;
	for (i = p_count; i > 0; i--) {
		point = *p_points++;
		vertex->m_x = point->m_screenX;
		vertex->m_y = point->m_screenY;
		vertex->m_red = point->m_light * p_red;
		vertex->m_green = point->m_light * p_green;
		vertex->m_blue = point->m_light * p_blue;
		vertex++;
	}

	FUN_10061cb0(&g_currentPane, p_count, g_a3dVertices);
}

// Draws a polygon in palette color p_color, lit by each vertex's light if p_lit is set.
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x100688fe
void FUN_100688fe(MechS32 p_count, ProjectedVertex** p_points, MechU32 p_color, MechS32 p_unk0x0c, MechS32 p_lit)
{
	ProjectedVertex* point;
	A3DVertex* vertex;
	MechS32 i;

	vertex = g_a3dVertices;
	for (i = 0; i < p_count; i++) {
		point = *p_points++;
		vertex->m_x = point->m_screenX;
		vertex->m_y = point->m_screenY;
		vertex->m_blue = g_paletteRgb[p_color][2];
		vertex->m_green = g_paletteRgb[p_color][1];
		vertex->m_red = g_paletteRgb[p_color][0];
		if (p_lit) {
			vertex->m_red *= point->m_light;
			vertex->m_green *= point->m_light;
			vertex->m_blue *= point->m_light;
		}

		vertex++;
	}

	FUN_10061cb0(&g_currentPane, p_count, g_a3dVertices);
}

// Draws a shaded polygon in each vertex's color: lit by its light, unless the vertex's shade (in
// m_u) is between 0 and 48. With p_unk0x0c set, only a polygon of three points or more.
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x100689f5
void FUN_100689f5(MechS32 p_count, ProjectedVertex** p_points, MechU32 p_color, MechS32 p_unk0x0c)
{
	MechFloat light;
	MechS32 i;
	A3DVertex* vertex;
	ProjectedVertex* point;

	vertex = g_a3dVertices;
	for (i = 0; i < p_count; i++) {
		point = *p_points++;
		vertex->m_x = point->m_screenX;
		vertex->m_y = point->m_screenY;
		if (point->m_u > 0.0f && point->m_u < 48.0f) {
			vertex->m_blue = point->m_blue;
			vertex->m_green = point->m_green;
			vertex->m_red = point->m_red;
		}
		else {
			light = point->m_light;
			vertex->m_blue = point->m_blue * light;
			vertex->m_green = point->m_green * light;
			vertex->m_red = point->m_red * light;
		}

		vertex++;
	}

	if (p_unk0x0c) {
		if (p_count > 2) {
			FUN_10061cb0(&g_currentPane, p_count, g_a3dVertices);
		}
	}
	else if (g_renderSettings.m_gouraud) {
		FUN_10061cb0(&g_currentPane, p_count, g_a3dVertices);
	}
	else {
		FUN_10061cb0(&g_currentPane, p_count, g_a3dVertices);
	}
}

// Draws the outline of a polygon in palette color p_color.
// FUNCTION: MW2MATROX 0x10068b57
void FUN_10068b57(MechS32 p_count, ProjectedVertex** p_points, MechU32 p_color)
{
	ProjectedVertex* point;
	A3DVertex* vertex;
	MechS32 i;

	vertex = g_a3dVertices;
	for (i = 0; i < p_count; i++) {
		point = *p_points++;
		vertex->m_x = 0;
		vertex->m_y = 0;
		vertex->m_z = 1.0f;
		vertex->m_red = 0;
		vertex->m_green = 0;
		vertex->m_blue = 0;
		vertex->m_unk0x1c = 0;
		vertex->m_unk0x20 = 0;
		vertex->m_unk0x24 = 0;
		vertex->m_w = 1.0f;
		vertex->m_x = point->m_screenX;
		vertex->m_y = point->m_screenY;
		vertex->m_blue = g_paletteRgb[p_color][2];
		vertex->m_green = g_paletteRgb[p_color][1];
		vertex->m_red = g_paletteRgb[p_color][0];
		vertex++;
	}

	if (p_count > 2) {
		FUN_10061890(&g_currentPane, g_a3dVertices, p_count);
	}
}

// Reads a line of at most p_size - 1 characters, with its newline, from p_file into p_buffer.
// Returns its length plus one, or 0 at the end of the file.
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x10068c7b
MechU32 ReadTextLine(FILE* p_file, MechChar* p_buffer, MechS32 p_size)
{
	MechU32 i;
	size_t read;

	for (i = 0; i < p_size - 1U && (read = fread(&p_buffer[i], 1, 1, p_file)) != 0; i++) {
		if (p_buffer[i] == '\n') {
			break;
		}
	}

	if (i || read) {
		i++;
		p_buffer[i] = '\0';
	}

	return i;
}

// Reads the textured sky's and ground's settings for mission p_mission from skygnd.par, or sets
// the defaults without the file (but not when the file has no line for the mission). The only diff
// is rate * g_skyTileU's operand order.
// FUNCTION: MW2MATROX 0x10068d0f
void LoadSkyGroundSettings(const MechChar* p_mission)
{
	MechChar mission[4];
	MechFloat rate;
	MechChar line[256];
	FILE* file;

	file = fopen("skygnd.par", "r");
	if (file) {
		for (;;) {
			if (!ReadTextLine(file, line, sizeof(line))) {
				return;
			}

			sscanf(
				line,
				"mission=%4s tile_sky=%f tile_ground=%f anim_rate=%f",
				mission,
				&g_skyTileU,
				&g_groundTile,
				&rate
			);
			if (!strncmp(p_mission, mission, 4)) {
				g_skyTileV = g_skyTileU * 0.2f;
				g_skyScroll = 0;
				g_skyScrollEnd = g_skyTileU;
				g_skyScrollSpeed = rate * g_skyTileU;
				return;
			}
		}
	}

	g_skyTileU = 12.0f;
	g_skyTileV = 2.4f;
	g_skyScroll = 0;
	g_skyScrollEnd = 12.0f;
	g_skyScrollSpeed = g_skyTileU * 0.0001f;
	g_groundTile = 50.0f;
}

// Draws the textured sky's four quads (g_skyCorners).
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x10068e33
void FUN_10068e33(Eyepoint* p_eyepoint)
{
	MechS32 count;
	MechS32 corner;
	MechU32 i;
	Vertex vertices[4];

	corner = 0;
	for (i = 0; i < 4; i++) {
		vertices[0].m_worldX = g_skyCorners[corner].m_x;
		vertices[0].m_worldY = g_skyCorners[corner].m_y;
		vertices[0].m_worldZ = g_skyCorners[corner].m_z;
		if (g_skyCorners[corner].m_farU) {
			vertices[0].m_u = g_skyTileU;
		}
		else {
			vertices[0].m_u = 0;
		}

		if (g_skyCorners[corner].m_farV) {
			vertices[0].m_v = g_skyTileV;
		}
		else {
			vertices[0].m_v = 0;
		}

		corner++;
		vertices[1].m_worldX = g_skyCorners[corner].m_x;
		vertices[1].m_worldY = g_skyCorners[corner].m_y;
		vertices[1].m_worldZ = g_skyCorners[corner].m_z;
		if (g_skyCorners[corner].m_farU) {
			vertices[1].m_u = g_skyTileU;
		}
		else {
			vertices[1].m_u = 0;
		}

		if (g_skyCorners[corner].m_farV) {
			vertices[1].m_v = g_skyTileV;
		}
		else {
			vertices[1].m_v = 0;
		}

		corner++;
		vertices[2].m_worldX = g_skyCorners[corner].m_x;
		vertices[2].m_worldY = g_skyCorners[corner].m_y;
		vertices[2].m_worldZ = g_skyCorners[corner].m_z;
		if (g_skyCorners[corner].m_farU) {
			vertices[2].m_u = g_skyTileU;
		}
		else {
			vertices[2].m_u = 0;
		}

		if (g_skyCorners[corner].m_farV) {
			vertices[2].m_v = g_skyTileV;
		}
		else {
			vertices[2].m_v = 0;
		}

		corner++;
		vertices[3].m_worldX = g_skyCorners[corner].m_x;
		vertices[3].m_worldY = g_skyCorners[corner].m_y;
		vertices[3].m_worldZ = g_skyCorners[corner].m_z;
		if (g_skyCorners[corner].m_farU) {
			vertices[3].m_u = g_skyTileU;
		}
		else {
			vertices[3].m_u = 0;
		}

		if (g_skyCorners[corner].m_farV) {
			vertices[3].m_v = g_skyTileV;
		}
		else {
			vertices[3].m_v = 0;
		}

		corner++;
		vertices[0].m_projection = vertices[1].m_projection = vertices[2].m_projection = vertices[3].m_projection =
			NULL;
		count = ProjectPolygon(vertices, 4, 1);
		if (!count) {
			continue;
		}

		FUN_10051ea2(0x8c, g_polygonPoints, count, 0, 0);
	}
}

// Draws the sky as one quad, its texture scrolled by g_skyScrollSpeed.
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x10069172
void FUN_10069172(Eyepoint* p_eyepoint)
{
	MechS32 count;
	Vertex vertices[4];

	vertices[0].m_worldX = vertices[3].m_worldX = -64000000.0f;
	vertices[1].m_worldX = vertices[2].m_worldX = 64000000.0f;
	vertices[0].m_worldZ = vertices[1].m_worldZ = -64000000.0f;
	vertices[3].m_worldZ = vertices[2].m_worldZ = 64000000.0f;
	vertices[0].m_worldY = vertices[1].m_worldY = vertices[2].m_worldY = vertices[3].m_worldY = 2000000.0f;
	g_skyScroll += g_skyScrollSpeed;
	g_skyScrollEnd += g_skyScrollSpeed;
	vertices[0].m_u = vertices[3].m_u = g_skyScroll;
	vertices[1].m_u = vertices[2].m_u = g_skyScrollEnd;
	vertices[0].m_v = vertices[1].m_v = g_skyScroll;
	vertices[3].m_v = vertices[2].m_v = g_skyScrollEnd;
	vertices[0].m_projection = vertices[1].m_projection = vertices[2].m_projection = vertices[3].m_projection = NULL;
	count = ProjectPolygon(vertices, 4, 1);
	if (!count) {
		return;
	}

	FUN_10051ea2(0x8c, g_polygonPoints, count, 0, 0);
}

// Draws the ground as one quad, its texture repeated every g_groundTile.
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x100692db
void FUN_100692db(Eyepoint* p_eyepoint)
{
	MechS32 count;
	Vertex vertices[4];

	vertices[0].m_worldX = vertices[3].m_worldX = -16000000.0f;
	vertices[1].m_worldX = vertices[2].m_worldX = 16000000.0f;
	vertices[0].m_worldZ = vertices[1].m_worldZ = -16000000.0f;
	vertices[3].m_worldZ = vertices[2].m_worldZ = 16000000.0f;
	vertices[0].m_worldY = vertices[1].m_worldY = vertices[2].m_worldY = vertices[3].m_worldY = 0;
	vertices[0].m_u = vertices[3].m_u = 0;
	vertices[1].m_u = vertices[2].m_u = g_groundTile;
	vertices[0].m_v = vertices[1].m_v = 0;
	vertices[3].m_v = vertices[2].m_v = g_groundTile;
	vertices[0].m_projection = vertices[1].m_projection = vertices[2].m_projection = vertices[3].m_projection = NULL;
	count = ProjectPolygon(vertices, 4, 1);
	if (!count) {
		return;
	}

	FUN_10051c07(8, g_polygonPoints, count, 0);
}
#endif

// Draws the sky and the ground of the view from p_eyepoint: the horizon, rolled with the view,
// splits the view rectangle (m_viewLeft-m_viewBottom). Each corner's side of it (IsAboveHorizon) makes
// one bit of the case; the parts are filled in g_skyColor (the sky, with m_drawSky) and
// g_groundColor (the ground, with m_drawGround), and with m_horizonBand a shaded band over the
// horizon blends the sky into the ground.
// Stack-slot permutation; the original calls IsAboveHorizon for the corners in the order of the
// terms, (x0, y1) first (commutative operand order).
// The Matrox edition's draws the textured sky and ground first (skygnd.par), takes the colors from
// their textures, and draws through its renderer; its diffs are the operand orders of the corner
// sum and of the 16-bit color's terms.
// FUNCTION: MW2 0x1004320b
// FUNCTION: MW2MATROX 0x10069419
void DrawSkyAndGround(Eyepoint* p_eyepoint)
{
#ifdef MW2_MATROX
	MechS32 y1;
	MechS32 x1;
	MechS32 yLeft;
	MechS32 corners;
	MechS32 y0;
	MechFloat dx;
	PANE rect;
	A3DVertex* band;
	MechFloat dy;
	MechFloat dz;
	MechS32 xBottom;
	MechS32 yRight;
	MechS32 xTop;
	MechS32 gradient;
	MechS32 x0;
	Matrix roll;

	x0 = p_eyepoint->m_viewLeft;
	x1 = p_eyepoint->m_viewRight;
	y0 = p_eyepoint->m_viewTop;
	y1 = p_eyepoint->m_viewBottom;
	band = g_a3dClipped;
	gradient = FALSE;
	if (!g_skyColorsRead) {
		FUN_100692db(p_eyepoint);
		FUN_10069172(p_eyepoint);
	}

	if (g_renderSettings.m_texturedSky) {
		FUN_10068e33(p_eyepoint);
	}

	if (g_renderSettings.m_texturedGround) {
		g_renderSettings.m_drawGround = FALSE;
		FUN_100692db(p_eyepoint);
	}
	else {
		g_renderSettings.m_drawGround = TRUE;
	}

	if (g_renderSettings.m_texturedSky) {
		g_renderSettings.m_drawSky = FALSE;
		FUN_10069172(p_eyepoint);
	}
	else {
		g_renderSettings.m_horizonBand = FALSE;
		g_renderSettings.m_drawSky = TRUE;
	}

	rect = g_currentPane;
	dx = dz = 0;
	dy = g_horizonBandHeight;
	SetIdentityMatrix(&roll);
	roll.m_rows[0][0] = roll.m_rows[1][1] = FixedCos(p_eyepoint->m_roll);
	roll.m_rows[1][0] = FixedSin(p_eyepoint->m_roll);
	roll.m_rows[0][1] = -roll.m_rows[1][0];
	roll.m_rows[3][0] = roll.m_rows[3][1] = roll.m_rows[3][2] = 0;
	TransformPoint(&roll, &dx, &dy, &dz);
	corners = IsAboveHorizon(x0, y1, p_eyepoint) * 4 + IsAboveHorizon(x1, y1, p_eyepoint) * 8 +
			  IsAboveHorizon(x1, y0, p_eyepoint) * 2 + IsAboveHorizon(x0, y0, p_eyepoint);
	if (!g_skyColorsRead) {
		FUN_100519b0(0x18c, &g_skyRed, &g_skyGreen, &g_skyBlue);
		FUN_100519b0(0x108, &g_groundRed, &g_groundGreen, &g_groundBlue);
		DebugPrint(
			"sky %d=(%f,%f,%f)\n  ground %d=(%f,%f,%f)\n",
			g_skyColor,
			g_skyRed,
			g_skyGreen,
			g_skyBlue,
			g_groundColor,
			g_groundRed,
			g_groundGreen,
			g_groundBlue
		);
		g_skyColorsRead = TRUE;
	}

	g_paletteRgb[g_skyColor][0] = g_skyRed;
	g_paletteRgb[g_skyColor][1] = g_skyGreen;
	g_paletteRgb[g_skyColor][2] = g_skyBlue;
	VFX_lookaside_write16(
		g_skyColor,
		((MechU8) g_skyRed & ~7) << 8 | ((MechU8) g_skyGreen & ~3) << 3 | (MechU8) g_skyBlue >> 3
	);
	g_paletteRgb[g_groundColor][0] = g_groundRed;
	g_paletteRgb[g_groundColor][1] = g_groundGreen;
	g_paletteRgb[g_groundColor][2] = g_groundBlue;
	VFX_lookaside_write16(
		g_groundColor,
		((MechU8) g_groundRed & ~7) << 8 | ((MechU8) g_groundGreen & ~3) << 3 | (MechU8) g_groundBlue >> 3
	);
	band[0].m_blue = g_paletteRgb[g_skyColor][2];
	band[0].m_green = g_paletteRgb[g_skyColor][1];
	band[0].m_red = g_paletteRgb[g_skyColor][0];
	band[1].m_blue = g_paletteRgb[g_skyColor][2];
	band[1].m_green = g_paletteRgb[g_skyColor][1];
	band[1].m_red = g_paletteRgb[g_skyColor][0];
	if (!g_renderSettings.m_drawGround && !g_renderSettings.m_drawSky) {
		return;
	}

	switch (corners) {
	case 0:
		if (g_renderSettings.m_drawGround) {
			rect.m_x0 += x0;
			rect.m_y0 += y0;
			rect.m_x1 = rect.m_x0 + x1 - x0;
			rect.m_y1 = rect.m_y0 + y1 - y0;
			FUN_1005f8a0(&rect, PIXEL_COLOR(g_groundColor));
		}
		break;
	case 15:
		if (g_renderSettings.m_drawSky) {
			if (g_renderSettings.m_horizonBand && !IsAboveHorizon(x0, y1 + (MechS32) dy, p_eyepoint)) {
				yLeft = HorizonYAtX(x0, p_eyepoint);
				yRight = HorizonYAtX(x1, p_eyepoint);
				if (yRight - dy < y1 || yLeft - dy < y1) {
					band[0].m_x = x1;
					band[3].m_x = x1;
					band[1].m_x = x0;
					band[2].m_x = x0;
					band[0].m_y = yRight - (MechS32) dy;
					band[1].m_y = yLeft - (MechS32) dy;
					band[2].m_y = yLeft;
					band[3].m_y = yRight;
					gradient = TRUE;
				}
			}

			rect.m_x0 += x0;
			rect.m_y0 += y0;
			rect.m_x1 = rect.m_x0 + x1 - x0;
			rect.m_y1 = rect.m_y0 + y1 - y0;
			FUN_1005f8a0(&rect, PIXEL_COLOR(g_skyColor));
		}
		break;
	case 3:
		yLeft = HorizonYAtX(x0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_horizonBand && g_renderSettings.m_drawSky) {
			band[0].m_x = x1;
			band[3].m_x = x1;
			band[1].m_x = x0;
			band[2].m_x = x0;
			band[0].m_y = yRight - (MechS32) dy;
			band[1].m_y = yLeft - (MechS32) dy;
			band[2].m_y = yLeft;
			band[3].m_y = yRight;
			gradient = TRUE;
		}

		if (yRight == yLeft) {
			if (g_renderSettings.m_drawSky) {
				rect.m_x0 += x0;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + x1 - x0;
				rect.m_y1 = rect.m_y0 + yLeft - y0;
				FUN_1005f8a0(&rect, PIXEL_COLOR(g_skyColor));
			}

			if (g_renderSettings.m_drawGround) {
				rect = g_currentPane;
				rect.m_x0 += x0;
				rect.m_y0 += yLeft;
				rect.m_x1 = rect.m_x0 + x1 - x0;
				rect.m_y1 = rect.m_y0 + y1 - yLeft;
				FUN_1005f8a0(&rect, PIXEL_COLOR(g_groundColor));
			}
		}
		else {
			if (g_renderSettings.m_drawSky) {
				DrawQuad(x1, y0, x0, y0, x0, yLeft, x1, yRight, g_skyColor);
			}

			if (g_renderSettings.m_drawGround) {
				DrawQuad(x0, y1, x1, y1, x1, yRight, x0, yLeft, g_groundColor);
			}
		}
		break;
	case 12:
		yLeft = HorizonYAtX(x0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_horizonBand && g_renderSettings.m_drawSky) {
			band[0].m_x = x0 + (MechS32) dx;
			band[3].m_x = x0 + (MechS32) dx;
			band[1].m_x = x1 - (MechS32) dx;
			band[2].m_x = x1 - (MechS32) dx;
			band[0].m_y = yLeft - (MechS32) dy;
			band[1].m_y = yRight - (MechS32) dy;
			band[2].m_y = yRight;
			band[3].m_y = yLeft;
			gradient = TRUE;
		}

		if (yRight == yLeft) {
			if (g_renderSettings.m_drawGround) {
				rect.m_x0 += x0;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + x1 - x0;
				rect.m_y1 = rect.m_y0 + yLeft - y0;
				FUN_1005f8a0(&rect, PIXEL_COLOR(g_groundColor));
			}

			if (g_renderSettings.m_drawSky) {
				rect = g_currentPane;
				rect.m_x0 += x0;
				rect.m_y0 += yLeft;
				rect.m_x1 = rect.m_x0 + x1 - x0;
				rect.m_y1 = rect.m_y0 + y1 - yLeft;
				FUN_1005f8a0(&rect, PIXEL_COLOR(g_skyColor));
			}
		}
		else {
			if (g_renderSettings.m_drawGround) {
				DrawQuad(x1, y0, x0, y0, x0, yLeft, x1, yRight, g_groundColor);
			}

			if (g_renderSettings.m_drawSky) {
				DrawQuad(x0, y1, x1, y1, x1, yRight, x0, yLeft, g_skyColor);
			}
		}
		break;
	case 5:
		xTop = HorizonXAtY(y0, p_eyepoint);
		xBottom = HorizonXAtY(y1, p_eyepoint);
		if (xBottom == xTop) {
			if (g_renderSettings.m_drawSky) {
				rect.m_x0 += x0;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + xTop - x0 - 1;
				rect.m_y1 = rect.m_y0 + y1 - y0 - 1;
				FUN_1005f8a0(&rect, PIXEL_COLOR(g_skyColor));
			}

			if (g_renderSettings.m_drawGround) {
				rect = g_currentPane;
				rect.m_x0 += xTop - x0 + 1;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + x1 - xTop - 1;
				rect.m_y1 = rect.m_y0 + y1 - y0 - 1;
				FUN_1005f8a0(&rect, PIXEL_COLOR(g_groundColor));
			}
		}
		else {
			if (g_renderSettings.m_drawSky) {
				DrawQuad(x0, y0, x0, y1, xBottom, y1, xTop, y0, g_skyColor);
			}

			if (g_renderSettings.m_drawGround) {
				DrawQuad(x1, y1, x1, y0, xTop, y0, xBottom, y1, g_groundColor);
			}
		}

		if (g_renderSettings.m_horizonBand && g_renderSettings.m_drawSky) {
			band[0].m_x = xTop + (MechS32) dx;
			band[1].m_x = xBottom - (MechS32) dx;
			band[2].m_x = xBottom;
			band[3].m_x = xTop;
			band[0].m_y = y0;
			band[1].m_y = y1;
			band[2].m_y = y1;
			band[3].m_y = y0;
			gradient = TRUE;
		}
		break;
	case 10:
		xTop = HorizonXAtY(y0, p_eyepoint);
		xBottom = HorizonXAtY(y1, p_eyepoint);
		if (xBottom == xTop) {
			if (g_renderSettings.m_drawGround) {
				rect.m_x0 += x0;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + xTop - x0 - 1;
				rect.m_y1 = rect.m_y0 + y1 - y0 - 1;
				FUN_1005f8a0(&rect, PIXEL_COLOR(g_groundColor));
			}

			if (g_renderSettings.m_drawSky) {
				rect = g_currentPane;
				rect.m_x0 += xTop - x0 + 1;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + x1 - xTop - 1;
				rect.m_y1 = rect.m_y0 + y1 - y0 - 1;
				FUN_1005f8a0(&rect, PIXEL_COLOR(g_skyColor));
			}
		}
		else {
			if (g_renderSettings.m_drawGround) {
				DrawQuad(x0, y0, x0, y1, xBottom, y1, xTop, y0, g_groundColor);
			}

			if (g_renderSettings.m_drawSky) {
				DrawQuad(x1, y1, x1, y0, xTop, y0, xBottom, y1, g_skyColor);
			}
		}

		if (g_renderSettings.m_horizonBand && g_renderSettings.m_drawSky) {
			band[0].m_x = xTop + (MechS32) dx;
			band[1].m_x = xBottom - (MechS32) dx;
			band[2].m_x = xBottom;
			band[3].m_x = xTop;
			band[0].m_y = y0;
			band[1].m_y = y1;
			band[2].m_y = y1;
			band[3].m_y = y0;
			gradient = TRUE;
		}
		break;
	case 1:
		xTop = HorizonXAtY(y0, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		if (g_renderSettings.m_drawSky) {
			DrawTriangle(x0, y0, x0, yLeft, xTop, y0, g_skyColor);
		}

		if (g_renderSettings.m_drawGround) {
			DrawPentagon(x0, y1, x1, y1, x1, y0, xTop, y0, x0, yLeft, g_groundColor);
		}
		break;
	case 14:
		xTop = HorizonXAtY(y0, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		if (g_renderSettings.m_drawGround) {
			DrawTriangle(x0, y0, x0, yLeft, xTop, y0, g_groundColor);
		}

		if (g_renderSettings.m_drawSky) {
			DrawPentagon(x0, y1, x1, y1, x1, y0, xTop, y0, x0, yLeft, g_skyColor);
		}
		break;
	case 2:
		xTop = HorizonXAtY(y0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_drawSky) {
			DrawTriangle(x1, y0, xTop, y0, x1, yRight, g_skyColor);
		}

		if (g_renderSettings.m_drawGround) {
			DrawPentagon(x0, y0, x0, y1, x1, y1, x1, yRight, xTop, y0, g_groundColor);
		}
		break;
	case 13:
		xTop = HorizonXAtY(y0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_drawGround) {
			DrawTriangle(x1, y0, xTop, y0, x1, yRight, g_groundColor);
		}

		if (g_renderSettings.m_drawSky) {
			DrawPentagon(x0, y0, x0, y1, x1, y1, x1, yRight, xTop, y0, g_skyColor);
		}
		break;
	case 4:
		xBottom = HorizonXAtY(y1, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		if (g_renderSettings.m_drawSky) {
			DrawTriangle(x0, yLeft, x0, y1, xBottom, y1, g_skyColor);
		}

		if (g_renderSettings.m_drawGround) {
			DrawPentagon(x1, y1, x1, y0, x0, y0, x0, yLeft, xBottom, y1, g_groundColor);
		}
		break;
	case 11:
		xBottom = HorizonXAtY(y1, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_drawGround) {
			DrawTriangle(x0, yLeft, x0, y1, xBottom, y1, g_groundColor);
		}

		if (g_renderSettings.m_drawSky) {
			DrawPentagon(x1, y1, x1, y0, x0, y0, x0, yLeft, xBottom, y1, g_skyColor);
			if (g_renderSettings.m_horizonBand) {
				band[0].m_x = x1;
				band[3].m_x = x1;
				band[1].m_x = x0;
				band[2].m_x = x0;
				band[0].m_y = yRight - (MechS32) dy;
				band[1].m_y = yLeft - (MechS32) dy;
				band[2].m_y = yLeft;
				band[3].m_y = yRight;
				gradient = TRUE;
			}
		}
		break;
	case 8:
		xBottom = HorizonXAtY(y1, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_drawSky) {
			DrawTriangle(xBottom, y1, x1, y1, x1, yRight, g_skyColor);
		}

		if (g_renderSettings.m_drawGround) {
			DrawPentagon(x1, y0, x0, y0, x0, y1, xBottom, y1, x1, yRight, g_groundColor);
		}
		break;
	case 7:
		xBottom = HorizonXAtY(y1, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		if (g_renderSettings.m_drawGround) {
			DrawTriangle(xBottom, y1, x1, y1, x1, yRight, g_groundColor);
		}

		if (g_renderSettings.m_drawSky) {
			DrawPentagon(x1, y0, x0, y0, x0, y1, xBottom, y1, x1, yRight, g_skyColor);
			if (g_renderSettings.m_horizonBand) {
				band[0].m_x = x1;
				band[3].m_x = x1;
				band[1].m_x = x0;
				band[2].m_x = x0;
				band[0].m_y = yRight - (MechS32) dy;
				band[1].m_y = yLeft - (MechS32) dy;
				band[2].m_y = yLeft;
				band[3].m_y = yRight;
				gradient = TRUE;
			}
		}
		break;
	}

	if (gradient && g_renderSettings.m_gouraud) {
		FUN_10061cb0(&g_currentPane, 4, band);
	}
#else
	MechS32 y1;
	MechS32 x1;
	MechS32 yLeft;
	MechS32 corners;
	MechS32 y0;
	MechS32 dx;
	PANE rect;
	MechU32 band[4 * 6];
	MechS32 dy;
	MechS32 dz;
	MechS32 xBottom;
	MechS32 yRight;
	MechS32 xTop;
	MechS32 gradient;
	MechS32 x0;
	Matrix roll;

	x0 = p_eyepoint->m_viewLeft;
	x1 = p_eyepoint->m_viewRight;
	y0 = p_eyepoint->m_viewTop;
	y1 = p_eyepoint->m_viewBottom;
	gradient = FALSE;
	rect = g_currentPane;
	dx = dz = 0;
	dy = g_horizonBandHeight;
	SetIdentityMatrix(&roll);
	roll.m_rows[0][0] = roll.m_rows[1][1] = FixedCos(p_eyepoint->m_roll);
	roll.m_rows[1][0] = FixedSin(p_eyepoint->m_roll);
	roll.m_rows[0][1] = -roll.m_rows[1][0];
	roll.m_rows[3][0] = roll.m_rows[3][1] = roll.m_rows[3][2] = 0;
	TransformPoint(&roll, &dx, &dy, &dz);
	corners = IsAboveHorizon(x0, y1, p_eyepoint) * 4 + IsAboveHorizon(x1, y1, p_eyepoint) * 8 +
			  IsAboveHorizon(x1, y0, p_eyepoint) * 2 + IsAboveHorizon(x0, y0, p_eyepoint);
	band[2] = band[8] = g_skyColor << 16;
	band[14] = band[20] = (g_groundColor - 1) << 16;
	switch (corners) {
	case 0:
		if (g_renderSettings.m_drawGround) {
			rect.m_x0 += x0;
			rect.m_y0 += y0;
			rect.m_x1 = rect.m_x0 + x1 - x0;
			rect.m_y1 = rect.m_y0 + y1 - y0;
			VFX_pane_wipe(&rect, g_groundColor);
		}
		break;
	case 15:
		if (g_renderSettings.m_drawSky) {
			if (g_renderSettings.m_horizonBand && !IsAboveHorizon(x0, dy + y1, p_eyepoint)) {
				yLeft = HorizonYAtX(x0, p_eyepoint);
				yRight = HorizonYAtX(x1, p_eyepoint);
				if (yRight - dy < y1 || yLeft - dy < y1) {
					band[0] = band[18] = x1;
					band[6] = band[12] = x0;
					band[1] = yRight - dy;
					band[7] = yLeft - dy;
					band[13] = yLeft;
					band[19] = yRight;
					gradient = TRUE;
				}
			}

			rect.m_x0 += x0;
			rect.m_y0 += y0;
			rect.m_x1 = rect.m_x0 + x1 - x0;
			rect.m_y1 = rect.m_y0 + y1 - y0;
			VFX_pane_wipe(&rect, g_skyColor);
		}
		break;
	case 3:
		yLeft = HorizonYAtX(x0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_horizonBand && g_renderSettings.m_drawSky) {
			band[0] = band[18] = x1;
			band[6] = band[12] = x0;
			band[1] = yRight - dy;
			band[7] = yLeft - dy;
			band[13] = yLeft;
			band[19] = yRight;
			gradient = TRUE;
		}

		if (yRight == yLeft) {
			if (g_renderSettings.m_drawSky) {
				rect.m_x0 += x0;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + x1 - x0;
				rect.m_y1 = rect.m_y0 + yLeft - y0;
				VFX_pane_wipe(&rect, g_skyColor);
			}

			if (g_renderSettings.m_drawGround) {
				rect = g_currentPane;
				rect.m_x0 += x0;
				rect.m_y0 += yLeft;
				rect.m_x1 = rect.m_x0 + x1 - x0;
				rect.m_y1 = rect.m_y0 + y1 - yLeft;
				VFX_pane_wipe(&rect, g_groundColor);
			}
		}
		else {
			if (g_renderSettings.m_drawSky) {
				DrawQuad(x1, y0, x0, y0, x0, yLeft, x1, yRight, g_skyColor);
			}

			if (g_renderSettings.m_drawGround) {
				DrawQuad(x0, y1, x1, y1, x1, yRight, x0, yLeft, g_groundColor);
			}
		}
		break;
	case 12:
		yLeft = HorizonYAtX(x0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_horizonBand && g_renderSettings.m_drawSky) {
			band[12] = band[6] = x1 - dx;
			band[18] = band[0] = dx + x0;
			band[13] = yRight;
			band[19] = yLeft;
			band[1] = yLeft - dy;
			band[7] = yRight - dy;
			gradient = TRUE;
		}

		if (yRight == yLeft) {
			if (g_renderSettings.m_drawGround) {
				rect.m_x0 += x0;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + x1 - x0;
				rect.m_y1 = rect.m_y0 + yLeft - y0;
				VFX_pane_wipe(&rect, g_groundColor);
			}

			if (g_renderSettings.m_drawSky) {
				rect = g_currentPane;
				rect.m_x0 += x0;
				rect.m_y0 += yLeft;
				rect.m_x1 = rect.m_x0 + x1 - x0;
				rect.m_y1 = rect.m_y0 + y1 - yLeft;
				VFX_pane_wipe(&rect, g_skyColor);
			}
		}
		else {
			if (g_renderSettings.m_drawGround) {
				DrawQuad(x1, y0, x0, y0, x0, yLeft, x1, yRight, g_groundColor);
			}

			if (g_renderSettings.m_drawSky) {
				DrawQuad(x0, y1, x1, y1, x1, yRight, x0, yLeft, g_skyColor);
			}
		}
		break;
	case 5:
		xTop = HorizonXAtY(y0, p_eyepoint);
		xBottom = HorizonXAtY(y1, p_eyepoint);
		if (xBottom == xTop) {
			if (g_renderSettings.m_drawSky) {
				rect.m_x0 += x0;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + xTop - x0 - 1;
				rect.m_y1 = rect.m_y0 + y1 - y0 - 1;
				VFX_pane_wipe(&rect, g_skyColor);
			}

			if (g_renderSettings.m_drawGround) {
				rect = g_currentPane;
				rect.m_x0 += xTop - x0 + 1;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + x1 - xTop - 1;
				rect.m_y1 = rect.m_y0 + y1 - y0 - 1;
				VFX_pane_wipe(&rect, g_groundColor);
			}
		}
		else {
			if (g_renderSettings.m_drawSky) {
				DrawQuad(x0, y0, x0, y1, xBottom, y1, xTop, y0, g_skyColor);
			}

			if (g_renderSettings.m_drawGround) {
				DrawQuad(x1, y1, x1, y0, xTop, y0, xBottom, y1, g_groundColor);
			}
		}

		if (g_renderSettings.m_horizonBand && g_renderSettings.m_drawSky) {
			band[0] = xTop - dx;
			band[6] = xBottom - dx;
			band[12] = xBottom;
			band[18] = xTop;
			band[1] = y0;
			band[19] = y0;
			band[7] = y1;
			band[13] = y1;
			gradient = TRUE;
		}
		break;
	case 10:
		xTop = HorizonXAtY(y0, p_eyepoint);
		xBottom = HorizonXAtY(y1, p_eyepoint);
		if (xBottom == xTop) {
			if (g_renderSettings.m_drawGround) {
				rect.m_x0 += x0;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + xTop - x0 - 1;
				rect.m_y1 = rect.m_y0 + y1 - y0 - 1;
				VFX_pane_wipe(&rect, g_groundColor);
			}

			if (g_renderSettings.m_drawSky) {
				rect = g_currentPane;
				rect.m_x0 += xTop - x0 + 1;
				rect.m_y0 += y0;
				rect.m_x1 = rect.m_x0 + x1 - xTop - 1;
				rect.m_y1 = rect.m_y0 + y1 - y0 - 1;
				VFX_pane_wipe(&rect, g_skyColor);
			}
		}
		else {
			if (g_renderSettings.m_drawGround) {
				DrawQuad(x0, y0, x0, y1, xBottom, y1, xTop, y0, g_groundColor);
			}

			if (g_renderSettings.m_drawSky) {
				DrawQuad(x1, y1, x1, y0, xTop, y0, xBottom, y1, g_skyColor);
			}
		}

		if (g_renderSettings.m_horizonBand && g_renderSettings.m_drawSky) {
			band[0] = xTop - dx;
			band[6] = xBottom - dx;
			band[12] = xBottom;
			band[18] = xTop;
			band[1] = y0;
			band[19] = y0;
			band[7] = y1;
			band[13] = y1;
			gradient = TRUE;
		}
		break;
	case 1:
		xTop = HorizonXAtY(y0, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		if (g_renderSettings.m_drawSky) {
			DrawTriangle(x0, y0, x0, yLeft, xTop, y0, g_skyColor);
		}

		if (g_renderSettings.m_drawGround) {
			DrawPentagon(x0, y1, x1, y1, x1, y0, xTop, y0, x0, yLeft, g_groundColor);
		}
		break;
	case 14:
		xTop = HorizonXAtY(y0, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		if (g_renderSettings.m_drawGround) {
			DrawTriangle(x0, y0, x0, yLeft, xTop, y0, g_groundColor);
		}

		if (g_renderSettings.m_drawSky) {
			DrawPentagon(x0, y1, x1, y1, x1, y0, xTop, y0, x0, yLeft, g_skyColor);
		}
		break;
	case 2:
		xTop = HorizonXAtY(y0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_drawSky) {
			DrawTriangle(x1, y0, xTop, y0, x1, yRight, g_skyColor);
		}

		if (g_renderSettings.m_drawGround) {
			DrawPentagon(x0, y0, x0, y1, x1, y1, x1, yRight, xTop, y0, g_groundColor);
		}
		break;
	case 13:
		xTop = HorizonXAtY(y0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_drawGround) {
			DrawTriangle(x1, y0, xTop, y0, x1, yRight, g_groundColor);
		}

		if (g_renderSettings.m_drawSky) {
			DrawPentagon(x0, y0, x0, y1, x1, y1, x1, yRight, xTop, y0, g_skyColor);
		}
		break;
	case 4:
		xBottom = HorizonXAtY(y1, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		if (g_renderSettings.m_drawSky) {
			DrawTriangle(x0, yLeft, x0, y1, xBottom, y1, g_skyColor);
		}

		if (g_renderSettings.m_drawGround) {
			DrawPentagon(x1, y1, x1, y0, x0, y0, x0, yLeft, xBottom, y1, g_groundColor);
		}
		break;
	case 11:
		xBottom = HorizonXAtY(y1, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_drawGround) {
			DrawTriangle(x0, yLeft, x0, y1, xBottom, y1, g_groundColor);
		}

		if (g_renderSettings.m_drawSky) {
			DrawPentagon(x1, y1, x1, y0, x0, y0, x0, yLeft, xBottom, y1, g_skyColor);
			if (g_renderSettings.m_horizonBand) {
				band[6] = band[12] = x0;
				band[18] = band[0] = x1;
				band[1] = yRight - dy;
				band[7] = yLeft - dy;
				band[13] = yLeft;
				band[19] = yRight;
				gradient = TRUE;
			}
		}
		break;
	case 8:
		xBottom = HorizonXAtY(y1, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		if (g_renderSettings.m_drawSky) {
			DrawTriangle(xBottom, y1, x1, y1, x1, yRight, g_skyColor);
		}

		if (g_renderSettings.m_drawGround) {
			DrawPentagon(x1, y0, x0, y0, x0, y1, xBottom, y1, x1, yRight, g_groundColor);
		}
		break;
	case 7:
		xBottom = HorizonXAtY(y1, p_eyepoint);
		yRight = HorizonYAtX(x1, p_eyepoint);
		yLeft = HorizonYAtX(x0, p_eyepoint);
		if (g_renderSettings.m_drawGround) {
			DrawTriangle(xBottom, y1, x1, y1, x1, yRight, g_groundColor);
		}

		if (g_renderSettings.m_drawSky) {
			DrawPentagon(x1, y0, x0, y0, x0, y1, xBottom, y1, x1, yRight, g_skyColor);
			if (g_renderSettings.m_horizonBand) {
				band[6] = band[12] = x0;
				band[18] = band[0] = x1;
				band[1] = yRight - dy;
				band[7] = yLeft - dy;
				band[13] = yLeft;
				band[19] = yRight;
				gradient = TRUE;
			}
		}
		break;
	}

	if (gradient && g_renderSettings.m_gouraud) {
		VFX_dithered_Gouraud_polygon(&g_currentPane, 0x8000, 4, band);
	}
#endif
}

// Draws a closed polygon of five points.
// MW2MATROX: stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004440d
// FUNCTION: MW2MATROX 0x1006ab1d
void DrawPentagon(
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
#ifdef MW2_MATROX
	ProjectedVertex* points[5];
	MechS32 i;
	ProjectedVertex vertices[5];

	for (i = 0; i < 5; i++) {
		points[i] = &vertices[i];
	}

	vertices[0].m_screenX = p_x0;
	vertices[0].m_screenY = p_y0;
	vertices[1].m_screenX = p_x1;
	vertices[1].m_screenY = p_y1;
	vertices[2].m_screenX = p_x2;
	vertices[2].m_screenY = p_y2;
	vertices[3].m_screenX = p_x3;
	vertices[3].m_screenY = p_y3;
	vertices[4].m_screenX = p_x4;
	vertices[4].m_screenY = p_y4;
	g_renderSettings.m_drawPolygon(5, points, p_flags, 0);
#else
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
	g_renderSettings.m_drawPolygon(5, points, p_flags);
#endif
}

// MW2MATROX: stack-slot permutation of the locals.
// FUNCTION: MW2 0x100444a6
// FUNCTION: MW2MATROX 0x1006abde
void DrawQuad(
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
#ifdef MW2_MATROX
	ProjectedVertex* points[4];
	MechS32 i;
	ProjectedVertex vertices[4];

	for (i = 0; i < 4; i++) {
		points[i] = &vertices[i];
	}

	vertices[0].m_screenX = p_x0;
	vertices[0].m_screenY = p_y0;
	vertices[1].m_screenX = p_x1;
	vertices[1].m_screenY = p_y1;
	vertices[2].m_screenX = p_x2;
	vertices[2].m_screenY = p_y2;
	vertices[3].m_screenX = p_x3;
	vertices[3].m_screenY = p_y3;
	g_renderSettings.m_drawPolygon(4, points, p_flags, 0);
#else
	MechU32 points[4 * 6];

	points[0] = points[3] = p_x0;
	points[1] = points[4] = p_y0;
	points[6] = points[9] = p_x1;
	points[7] = points[10] = p_y1;
	points[12] = points[15] = p_x2;
	points[13] = points[16] = p_y2;
	points[18] = points[21] = p_x3;
	points[19] = points[22] = p_y3;
	g_renderSettings.m_drawPolygon(4, points, p_flags);
#endif
}

// MW2MATROX: stack-slot permutation of the locals.
// FUNCTION: MW2 0x10044527
// FUNCTION: MW2MATROX 0x1006ac8d
void DrawTriangle(MechS32 p_x0, MechS32 p_y0, MechS32 p_x1, MechS32 p_y1, MechS32 p_x2, MechS32 p_y2, MechU32 p_flags)
{
#ifdef MW2_MATROX
	ProjectedVertex* points[3];
	MechS32 i;
	ProjectedVertex vertices[3];

	for (i = 0; i < 3; i++) {
		points[i] = &vertices[i];
	}

	vertices[0].m_screenX = p_x0;
	vertices[0].m_screenY = p_y0;
	vertices[1].m_screenX = p_x1;
	vertices[1].m_screenY = p_y1;
	vertices[2].m_screenX = p_x2;
	vertices[2].m_screenY = p_y2;
	g_renderSettings.m_drawPolygon(3, points, p_flags, 0);
#else
	MechU32 points[3 * 6];

	points[0] = points[3] = p_x0;
	points[1] = points[4] = p_y0;
	points[6] = points[9] = p_x1;
	points[7] = points[10] = p_y1;
	points[12] = points[15] = p_x2;
	points[13] = points[16] = p_y2;
	g_renderSettings.m_drawPolygon(3, points, p_flags);
#endif
}

#ifndef MW2_MATROX
// Draws a line from the end of the last one to (p_x, p_y).
// FUNCTION: MW2 0x10044590
void DrawLineTo(MechS32 p_x, MechS32 p_y, MechU32 p_color)
{
	VFX_line_draw(&g_currentPane, p_x, p_y, g_lineEndX, g_lineEndY, 0, p_color);
	g_lineEndX = p_x;
	g_lineEndY = p_y;
}

// Draws a polygon: a point or a line with the pane's own routines where the settings
// allow, else through g_renderSettings's polygon callback, filled, outlined or both.
// FUNCTION: MW2 0x100445d2
void DrawPolygonOrLine(MechS32 p_count, MechU32* p_points, MechU32 p_flags)
{
	if (p_count == 1 && g_renderSettings.m_drawPixels) {
		if (p_flags == 0x1000) {
			return;
		}

		VFX_pixel_write(&g_currentPane, p_points[0], p_points[1], p_flags);
	}
	else if (p_count == 2 && g_renderSettings.m_drawLines) {
		VFX_line_draw(&g_currentPane, p_points[0], p_points[1], p_points[6], p_points[7], 0, p_flags);
	}
	else if (g_renderSettings.m_wireframe == 0) {
		g_renderSettings.m_drawPolygon(p_count, p_points, p_flags);
	}
	else if (g_renderSettings.m_wireframe == 1) {
		g_renderSettings.m_drawPolygon(p_count, p_points, 0);
		g_renderSettings.m_drawPolygon(p_count, p_points, p_flags | 0x2000);
	}
	else {
		g_renderSettings.m_drawPolygon(p_count, p_points, p_flags | 0x2000);
	}
}
#endif
