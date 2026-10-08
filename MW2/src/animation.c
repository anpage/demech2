#include "animation.h"

#include "clock.h"
#include "decomp.h"
#include "loadres.h"
#include "mw2prj.h"
#include "render.h"
#include "resource.h"
#include "texpoly.h"
#include "types.h"
#include "vfx3d.h"
#include "window.h"
#ifdef MW2_MATROX
#include "eyepoint.h"
#include "fixedfloat.h"
#include "matrox/a3d.h"
#include "matrox/texturepoly.h"
#include "palette.h"
#include "polydraw.h"
#include "prjfile.h"
#include "projectedvertex.h"
#include "simmain.h"
#include "view.h"

#include <math.h>
#include <windows.h>
#endif

// Animated textures: up to 0x200 animations, each playing one of 0x200 sets of up to 0x20 CEL
// frames. The frames load on first use; each animation advances with the clock.

#ifdef MW2_MATROX
DECOMP_SIZE_ASSERT(AnimFrame, 0x14)
#else
DECOMP_SIZE_ASSERT(AnimFrame, 0x08)
#endif
DECOMP_SIZE_ASSERT(Animation, 0x0e)

// GLOBAL: MW2 0x100ad288
// GLOBAL: MW2MATROX 0x100aa1b8
MechS32 g_animInitialized = -2;

// GLOBAL: MW2 0x100ad28c
// GLOBAL: MW2MATROX 0x100aa1bc
MechS32 g_lumaResourceId = 0;

// GLOBAL: MW2 0x100ad290
// GLOBAL: MW2MATROX 0x100aa1c0
MechS32 g_currentAnimSet = 0;

// GLOBAL: MW2 0x100ad294
// GLOBAL: MW2MATROX 0x100aa1c4
MechS32 g_animSetUsed = 0;

// GLOBAL: MW2 0x100ad298
// GLOBAL: MW2MATROX 0x100aa1c8
MechS32 g_animSetIsSequence = 0;

// GLOBAL: MW2 0x100ad29c
MechU16* g_lumaTables = NULL;

// GLOBAL: MW2 0x100ad2a0
// GLOBAL: MW2MATROX 0x100aa1d8
MechS32 g_preloadCels[] = {
#ifdef MW2_MATROX
	// The Matrox edition's resource file numbers the cels differently.
	598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616, 617, 618,
	619, 620, 621, 622, 623, 624, 625, 626, 627, 628, 629, 94,  95,  96,  97,  98,  99,  100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
	125, 427, 428, 429, 430, 431, 432, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203,
	204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, -1,
#else
	562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582,
	583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 88,  89,  90,  91,  92,  93,  94,  95,  96,  97,
	98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118,
	119, 428, 429, 430, 431, 432, 433, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
	238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, -1,
#endif
};

// GLOBAL: MW2 0x100c7610
// GLOBAL: MW2MATROX 0x10184710
AnimFrame g_animFrames[0x200][0x20];

// GLOBAL: MW2 0x100e7610
// GLOBAL: MW2MATROX 0x10182b00
Animation g_animations[0x200];

// GLOBAL: MW2 0x100e9210
WINDOW g_animFrameBuffer;

#ifdef MW2_MATROX
// Rotates the texture point (*p_u, *p_v) about the texture's centre by the angle whose cosine and
// sine are p_cos and p_sin.
// The products' operands are loaded in the other order (commutative operand order).
// FUNCTION: MW2MATROX 0x10050d60
void RotateTexturePoint(MechFloat p_cos, MechFloat p_sin, MechFloat* p_u, MechFloat* p_v)
{
	MechFloat u;
	MechFloat v;

	u = *p_u - 0.5f;
	v = *p_v - 0.5f;
	*p_u = u * p_cos - v * p_sin + 0.5;
	*p_v = v * p_cos + u * p_sin + 0.5;
}

#endif
// Stack-slot permutation: anim, data, height, i, luma, mode and useLuma and width.
// MW2MATROX's (0x10050dbc) is another function: it draws through the Matrox edition's renderer.
// MW2MATROX: a stack-slot permutation of the locals, and the loop's i < p_count has its operands
// the other way around.
// FUNCTION: MW2 0x10068d10
// FUNCTION: MW2MATROX 0x10050dbc
#ifdef MW2_MATROX
MechS32 DrawAnimatedPolygon(
	MechS32 p_index,
	MechS32 p_count,
	ProjectedVertex** p_points,
	MechS32 p_luma,
	MechS32 p_scale,
	MechS32 p_direct,
	MechS32 p_unk0x18,
	MechS32 p_unk0x1c
)
#else
MechS32 DrawAnimatedPolygon(
	MechS32 p_index,
	MechS32 p_count,
	MechU32* p_points,
	MechS32 p_luma,
	MechS32 p_scale,
	MechS32 p_direct
)
#endif
{
#ifdef MW2_MATROX
	A3DTexture* texture;
	MechFloat w;
	AnimFrame* frame;
	MechU32 flags;
	undefined4 luma;
	MechFloat offset;
	Animation* anim;
	MechFloat angle;
	MechS32 rotate;
	MechFloat cosine;
	MechS32 i;
	MechS16 height;
	MechS32 useLuma;
	MechFloat sine;
	MechS16 width;
	A3DVertex* vertex;
	ProjectedVertex* point;
	undefined4 data;

	data = 0;
	luma = 0;
	flags = 0;
	rotate = FALSE;
	if (p_unk0x1c) {
		flags = 2;
	}

	if (p_count < 3) {
		return 0;
	}

	if (p_index == 0x8c) {
		rotate = TRUE;
		angle = p_unk0x1c * 0.017453292f;
		offset = p_scale / 100000.0f;
		cosine = cos(angle);
		sine = sin(angle);
		p_unk0x1c = 0;
		p_scale = 0;
		p_index += 0x100;
	}
	else {
		rotate = FALSE;
		if (!p_scale) {
			p_index += 0x100;
		}
	}

	anim = &g_animations[p_index];
	if (anim->m_mode == 0 || anim->m_flags < 0) {
		return 0;
	}

	frame = &g_animFrames[anim->m_set][anim->m_frame];
	if (frame->m_resourceId < 1) {
		return 0;
	}

	frame->m_useCount++;
	texture = FUN_1005d600(frame->m_resourceId, 1, 1);
	if (!texture) {
		return 0;
	}

	width = texture->m_unk0x1c;
	height = texture->m_unk0x20;
	if (!(anim->m_flags & 4) && p_luma > -1 && p_luma < 15) {
		useLuma = TRUE;
	}
	else {
		useLuma = FALSE;
	}

	vertex = g_a3dVertices;
	for (i = 0; i < p_count; i++) {
		point = *p_points++;
		vertex->m_x = point->m_screenX;
		vertex->m_y = point->m_screenY;
		if (p_direct && !g_renderSettings.m_affineTextures) {
			w = g_eyepoint->m_projectScaleX / point->m_z;
			vertex->m_z = point->m_z;
			vertex->m_w = w;
			vertex->m_u = w * point->m_u / width;
			vertex->m_v = point->m_v * w / height;
		}
		else if (p_scale) {
			vertex->m_u = point->m_u;
			vertex->m_v = point->m_v;
			vertex->m_w = 1;
		}
		else {
			vertex->m_u = point->m_u / width;
			vertex->m_v = point->m_v / height;
			vertex->m_w = 1;
			if (rotate) {
				vertex->m_u += offset;
				RotateTexturePoint(cosine, sine, &vertex->m_u, &vertex->m_v);
			}
		}

		if (g_unk0x100aa1d0) {
			vertex->m_blue = g_unk0x10184700[2];
			vertex->m_green = g_unk0x10184700[1];
			vertex->m_red = g_unk0x10184700[0];
			flags |= 1;
		}
		else if ((p_direct || useLuma) && p_luma != (MechS32) 0x80000000) {
			vertex->m_blue = point->m_light * 255;
			vertex->m_green = point->m_light * 255;
			vertex->m_red = point->m_light * 255;
		}
		else {
			vertex->m_blue = 255;
			vertex->m_green = 255;
			vertex->m_red = 255;
		}

		vertex++;
	}

	if (p_direct) {
		FUN_100172c0(&g_currentPane, texture, p_count, g_a3dVertices, 0, luma, p_unk0x1c);
	}
	else {
		if (useLuma) {
			flags |= 1;
		}

		FUN_10062010(&g_currentPane, p_count, g_a3dVertices, texture, flags);
	}

	return 1;
#else
	MechS16 height;
	MechS32 i;
	MechS32 useLuma;
	AnimFrame* frame;
	MechU16* data;
	Animation* anim;
	MechS16 width;
	MechU16* luma;
	MechS32 mode;

	data = NULL;
	luma = NULL;
	mode = 2;
	if (p_count < 3) {
		return 0;
	}

	if (!p_scale) {
		p_index += 0x100;
	}

	anim = &g_animations[p_index];
	if (anim->m_mode == 0 || anim->m_flags < 0) {
		return 0;
	}

	frame = &g_animFrames[anim->m_set][anim->m_frame];
	if (frame->m_resourceId < 1) {
		return 0;
	}

	frame->m_useCount++;
	data = frame->m_data;
	if (!data) {
		data = LoadCachedResource(g_mw2PrjHandle, frame->m_resourceId, g_resourceTypeTags[c_resTagCel], 0);
		if (!data) {
			return 0;
		}

		frame->m_data = data;
	}

	width = data[0];
	height = data[1];
	data += 2;
	if (p_scale) {
		for (i = 0; i < 4; i++) {
			p_points[i * 6 + 3] *= (MechS16) (width - 1);
			p_points[i * 6 + 4] *= (MechS16) (height - 1);
		}
	}

	if (!(anim->m_flags & 4) && p_luma > -1 && p_luma < 15) {
		useLuma = TRUE;
	}
	else {
		useLuma = FALSE;
	}

	if (!g_lumaTables) {
		g_lumaTables = LoadCachedResource(g_mw2PrjHandle, g_lumaResourceId, g_resourceTypeTags[c_resTagLuma], 0);
	}

	if (p_direct) {
		luma = g_lumaTables + p_luma * 0x80;
		DrawTexturedPolygon(&g_currentPane, (MechU8*) data, width, height, p_count, p_points, useLuma, luma);
	}
	else {
		if (useLuma) {
			luma = g_lumaTables + p_luma * 0x80;
			VFX_map_lookaside(luma);
			mode |= 1;
		}

		g_animFrameBuffer.m_buffer = (undefined*) data;
		g_animFrameBuffer.m_xMax = width - 1;
		g_animFrameBuffer.m_yMax = height - 1;
		VFX_map_polygon(&g_currentPane, p_count, p_points, &g_animFrameBuffer, mode);
	}

	return 1;
#endif
}

// Stack-slot permutation: frame, i and now and remainder. MW2MATROX: a comparison has its
// operands the other way around.
// FUNCTION: MW2 0x10068fb8
// FUNCTION: MW2MATROX 0x10051278
void AdvanceAnimations(void)
{
	MechS32 remainder;
	MechS32 j;
	MechS32 now;
	MechS32 i;
	MechU32 frame;
	MechS32 steps;
	Animation* anim;
	MechS32 elapsed;

	now = g_currentClock;
	for (i = 0; i < 0x200; i++) {
		anim = &g_animations[i];
		if (anim->m_flags >= 0 && anim->m_mode != 0 && anim->m_delay > 0) {
			if (anim->m_lastTime == 0) {
				anim->m_lastTime = now;
			}

			elapsed = now - anim->m_lastTime;
			steps = elapsed / anim->m_delay;
			remainder = elapsed - anim->m_delay * steps;
			frame = anim->m_frame;
			if (steps > 0) {
				for (j = 0; j < steps; j++) {
					frame++;
					frame &= 0x1f;
					if (g_animFrames[anim->m_set][frame].m_resourceId < 1) {
						frame = 0;
					}
				}

				if (anim->m_mode == 2 && anim->m_frame > frame) {
					anim->m_mode = 0;
					frame = 0;
				}

				anim->m_frame = frame;
				anim->m_lastTime = now - remainder;
			}
		}
	}
}

// Stack-slot permutation: found and slot. MW2MATROX: index order, g_animFrames[p_set][slot]
// loads p_set first.
// FUNCTION: MW2 0x10069124
// FUNCTION: MW2MATROX 0x100513ed
MechS32 AddAnimFrame(MechS32 p_resourceId, MechS32 p_set)
{
	MechS32 found;
	MechS32 slot;

	slot = 0;
	found = FALSE;
	if (g_animInitialized == -2) {
		InitAnimations();
	}

	if (p_set == -1) {
		if (g_animSetUsed) {
			g_currentAnimSet++;
			g_animSetUsed = 0;
		}

		p_set = g_currentAnimSet;
	}

	if (p_resourceId <= 0 || p_set < 0 || p_set >= 0x200) {
		return -1;
	}

	if (g_animFrames[p_set][0].m_resourceId > 0) {
		g_animSetIsSequence = 1;
		while (!found) {
			slot++;
			if (slot >= 0x20) {
				return -1;
			}

			if (g_animFrames[p_set][slot].m_resourceId == -1) {
				found = TRUE;
			}
		}
	}
	else {
		g_animSetIsSequence = 0;
	}

	g_animFrames[p_set][slot].m_resourceId = p_resourceId;
	return 0;
}

// FUNCTION: MW2 0x1006923c
// FUNCTION: MW2MATROX 0x1005151a
void PreloadAnimCels(void)
{
	MechS32 i;

	for (i = 0; g_preloadCels[i] != -1; i++) {
		PreloadResource(g_preloadCels[i], g_resourceTypeTags[c_resTagCel]);
	}
}

// FUNCTION: MW2 0x10069288
// FUNCTION: MW2MATROX 0x10051566
MechS32 StartAnimation(MechS32 p_index, MechS32 p_set)
{
	if (p_set == -1) {
		p_set = g_currentAnimSet;
		g_animSetUsed = 1;
	}

	if (p_set < 0 || p_set >= 0x200 || p_index < 0 || p_index >= 0x200) {
		return 0;
	}

	g_animations[p_index].m_set = p_set;
	g_animations[p_index].m_flags = 1;
	g_animations[p_index].m_mode = 1;
	g_animations[p_index].m_frame = 0;
	if (g_animSetIsSequence) {
		g_animations[p_index].m_delay = 0x2d;
	}

	return 1;
}

// MW2MATROX: stack-slot permutation of i and j.
// FUNCTION: MW2 0x10069360
// FUNCTION: MW2MATROX 0x1005163e
void InitAnimations(void)
{
	MechS32 j;
	MechS32 i;
	AnimFrame* frame;
	Animation* anim;

	for (i = 0; i < 0x200; i++) {
		for (j = 0; j < 0x20; j++) {
			frame = &g_animFrames[i][j];
			frame->m_resourceId = -1;
			frame->m_useCount = 0;
			frame->m_data = NULL;
		}
	}

	for (i = 0; i < 0x200; i++) {
		anim = &g_animations[i];
		anim->m_set = -1;
		anim->m_frame = 0;
		anim->m_delay = 0;
		anim->m_mode = 0;
		anim->m_flags = -2;
		anim->m_lastTime = -1;
	}

	g_animInitialized = 0;
	g_currentAnimSet = 0;
	g_animSetUsed = 0;
	g_animSetIsSequence = 0;
}

// FUNCTION: MW2 0x1006946f
// FUNCTION: MW2MATROX 0x10051755
void SetAnimMode(MechS16 p_index, MechS16 p_mode)
{
	if (g_animations[p_index].m_flags != -2) {
		g_animations[p_index].m_mode = p_mode;
	}

	if (g_animations[p_index].m_delay == 0) {
		g_animations[p_index].m_delay = 0x2d;
	}
}

// FUNCTION: MW2 0x100694df
// FUNCTION: MW2MATROX 0x100517c5
void SetAnimFrame(MechS16 p_index, MechU16 p_frame)
{
	Animation* anim;

	if (p_frame >= 0x20) {
		p_frame = 0;
	}

	anim = &g_animations[p_index];
	if (anim->m_flags < 0) {
		return;
	}

	if (g_animFrames[anim->m_set][p_frame].m_resourceId > 0) {
		anim->m_frame = p_frame;
		anim->m_lastTime = 0;
	}
}

// FUNCTION: MW2 0x10069564
// FUNCTION: MW2MATROX 0x10051853
void SetAnimDelay(MechS16 p_index, MechU16 p_delay)
{
	g_animations[p_index].m_delay = p_delay;
}

// FUNCTION: MW2 0x10069586
// FUNCTION: MW2MATROX 0x10051875
void FUN_10069586(void)
{
}

// FUNCTION: MW2 0x10069591
// FUNCTION: MW2MATROX 0x10051880
void FUN_10069591(void)
{
}

// Index order: &g_animFrames[i][j] loads i first in the original (InitAnimations matches with the
// same statement). MW2MATROX loads them the other way around.
// FUNCTION: MW2 0x1006959c
// FUNCTION: MW2MATROX 0x1005188b
void FreeAnimations(void)
{
	MechS32 j;
	MechS32 i;
	AnimFrame* frame;
	Animation* anim;

	for (i = 0; i < 0x200; i++) {
		for (j = 0; j < 0x20; j++) {
			frame = &g_animFrames[i][j];
			if (frame->m_data) {
				UnlockCachedResource(frame->m_resourceId, g_resourceTypeTags[c_resTagCel]);
				frame->m_data = NULL;
			}

			frame->m_resourceId = 0;
			frame->m_useCount = 0;
		}
	}

	UnlockCachedResource(g_lumaResourceId, g_resourceTypeTags[c_resTagLuma]);
	for (i = 0; i < 0x200; i++) {
		anim = &g_animations[i];
		anim->m_set = -1;
		anim->m_frame = 0;
		anim->m_delay = 0;
		anim->m_mode = 0;
		anim->m_flags = -2;
		anim->m_lastTime = -1;
	}
}

#ifdef MW2_MATROX
// Computes the average color of animation p_index's current texture, 0-31 per component (the
// sums of the 5-bit components over an eighth of the pixel count); returns 0 if it has none.
// pixels < end compares in the other operand order.
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x100519b0
MechS32 FUN_100519b0(MechS32 p_index, MechFloat* p_red, MechFloat* p_green, MechFloat* p_blue)
{
	AnimFrame* frame;
	MechU16 pixel;
	MechU32 blue;
	MechU32 red;
	Animation* anim;
	MechU32 green;
	MechU16* end;
	MechS16 height;
	MechS32 size;
	MechS16 width;
	MechU16* data;
	MechU32 count;
	MechU16* pixels;

	pixels = NULL;
	red = 0;
	green = 0;
	blue = 0;
	anim = &g_animations[p_index];
	frame = &g_animFrames[anim->m_set][anim->m_frame];
	if (frame->m_resourceId < 1) {
		return 0;
	}

	size = GetPrjResourceSize(g_mw2PrjHandle, g_resourceTypeTags[c_resTagCel], frame->m_resourceId);
	if (!size) {
		return 0;
	}

	data = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, size);
	ReadPrjResource(g_mw2PrjHandle, g_resourceTypeTags[c_resTagCel], frame->m_resourceId, data);
	width = data[0];
	height = data[1];
	pixels = data + 2;
	if (!pixels) {
		return 0;
	}

	count = height * width;
	end = pixels + count;
	while (pixels < end) {
		pixel = *pixels;
		blue += pixel & 0x1f;
		pixel >>= 5;
		green += pixel & 0x1f;
		pixel >>= 5;
		red += pixel & 0x1f;
		pixels++;
	}

	red /= count >> 3;
	green /= count >> 3;
	blue /= count >> 3;
	*p_red = red;
	*p_green = green;
	*p_blue = blue;
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
	return 1;
}

// Returns the average color of animation p_index's current frame (CalcAverageBitmapColors).
// FUNCTION: MW2MATROX 0x10051ba1
void FUN_10051ba1(MechS32 p_index, MechFloat* p_red, MechFloat* p_green, MechFloat* p_blue)
{
	AnimFrame* frame;
	Animation* anim;

	anim = &g_animations[p_index];
	frame = &g_animFrames[anim->m_set][anim->m_frame];
	*p_red = frame->m_red;
	*p_green = frame->m_green;
	*p_blue = frame->m_blue;
}

// Draws the polygon of p_count projected points with animation p_index + 0x100's texture, through
// the renderer (A3D_GroundSkyPolyPlot), at three quarters of the far plane's depth; returns 0 for fewer than
// three points or no texture.
// The v product takes its operands in the other order (commutative operand order).
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x10051c07
MechS32 FUN_10051c07(MechS32 p_index, ProjectedVertex** p_points, MechS32 p_count, MechS32 p_unk0x0c)
{
	A3DTexture* texture;
	MechS32 i;
	MechS16 height;
	MechS16 width;
	ProjectedVertex* point;
	A3DPolyVertex vertices[16];
	A3DPolyVertex* vertex;

	if (p_count < 3) {
		return 0;
	}

	p_index += 0x100;
	texture = FUN_10051dfe(p_index, &width, &height, g_unk0x100ac924);
	if (!texture) {
		return 0;
	}

	vertex = vertices;
	for (i = 0; i < p_count; i++) {
		point = *p_points;
		p_points++;
		vertex->m_x = point->m_screenX;
		vertex->m_y = point->m_screenY;
		vertex->m_z = point->m_z;
		vertex->m_w = g_eyepoint->m_projectScaleX / vertex->m_z;
		vertex->m_u = point->m_u * vertex->m_w;
		vertex->m_v = point->m_v * vertex->m_w;
		if (g_unk0x100aa1d0) {
			vertex->m_blue = g_unk0x10184700[2];
			vertex->m_green = g_unk0x10184700[1];
			vertex->m_red = g_unk0x10184700[0];
		}
		else {
			vertex->m_blue = 255.0;
			vertex->m_green = 255.0;
			vertex->m_red = 255.0;
		}

		vertex++;
	}

	// The original returns without a return statement (eax as A3D_GroundSkyPolyPlot leaves it).
	A3D_GroundSkyPolyPlot(
		&g_currentPane,
		p_count,
		vertices,
		texture,
		p_unk0x0c,
		g_viewFarPlane * 0.75,
		g_unk0x100ac924 > 1
	);
}

// Returns the texture of animation p_index's current frame (FUN_1005d600), and its size.
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x10051dfe
A3DTexture* FUN_10051dfe(MechS32 p_index, MechS16* p_width, MechS16* p_height, MechU32 p_mode)
{
	A3DTexture* texture;
	AnimFrame* frame;
	Animation* anim;

	anim = &g_animations[p_index];
	frame = &g_animFrames[anim->m_set][anim->m_frame];
	if (frame->m_resourceId < 1) {
		return NULL;
	}

	texture = FUN_1005d600(frame->m_resourceId, 1, p_mode);
	if (!texture) {
		return NULL;
	}

	*p_width = texture->m_unk0x1c;
	*p_height = texture->m_unk0x20;
	return texture;
}

// FUN_10051c07 without its point count test (its callers pass a fifth argument it doesn't read), at a fixed depth
// (-500000) and in its own texture mode. Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x10051ea2
MechS32 FUN_10051ea2(MechS32 p_index, ProjectedVertex** p_points, MechS32 p_count, MechS32 p_unk0x0c, MechS32 p_unk0x10)
{
	A3DTexture* texture;
	MechS32 i;
	MechS16 height;
	MechS16 width;
	ProjectedVertex* point;
	A3DPolyVertex vertices[16];
	A3DPolyVertex* vertex;

	p_index += 0x100;
	texture = FUN_10051dfe(p_index, &width, &height, g_unk0x100ac928);
	if (!texture) {
		return 0;
	}

	vertex = vertices;
	for (i = 0; i < p_count; i++) {
		point = *p_points;
		p_points++;
		vertex->m_x = point->m_screenX;
		vertex->m_y = point->m_screenY;
		vertex->m_z = point->m_z;
		vertex->m_w = g_eyepoint->m_projectScaleX / vertex->m_z;
		vertex->m_u = point->m_u * vertex->m_w;
		vertex->m_v = point->m_v * vertex->m_w;
		if (g_unk0x100aa1d0) {
			vertex->m_blue = g_unk0x10184700[2];
			vertex->m_green = g_unk0x10184700[1];
			vertex->m_red = g_unk0x10184700[0];
		}
		else {
			vertex->m_blue = 255.0;
			vertex->m_green = 255.0;
			vertex->m_red = 255.0;
		}

		vertex++;
	}

	// The original returns without a return statement (eax as A3D_GroundSkyPolyPlot leaves it).
	A3D_GroundSkyPolyPlot(&g_currentPane, p_count, vertices, texture, p_unk0x0c, -500000.0, g_unk0x100ac928 > 1);
}

// Computes the average color of every playing animation's current frame (FUN_100519b0).
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x1005207d
void CalcAverageBitmapColors(void)
{
	MechFloat blue;
	MechFloat red;
	AnimFrame* frame;
	Animation* anim;
	MechFloat green;
	MechU32 i;

	for (i = 0; i < 0x200; i++) {
		anim = &g_animations[i];
		if (!anim) {
			continue;
		}

		if (anim->m_mode == 0 || anim->m_flags < 0) {
			continue;
		}

		frame = &g_animFrames[anim->m_set][anim->m_frame];
		if (frame->m_resourceId < 1) {
			continue;
		}

		if (FUN_100519b0(i, &red, &green, &blue)) {
			frame->m_red = red;
			frame->m_green = green;
			frame->m_blue = blue;
		}
	}
}
#endif
