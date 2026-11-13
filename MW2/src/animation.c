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
#ifdef MW2_ACCEL
#include "accel/a3d.h"
#include "accel/texturepoly.h"
#include "eyepoint.h"
#include "fixedfloat.h"
#include "palette.h"
#include "polydraw.h"
#include "prjfile.h"
#include "projectedvertex.h"
#include "simmain.h"
#include "view.h"

#include <math.h>
#include <windows.h>
#endif

#ifdef MW2_3DFX
#endif

// Animated textures: up to 0x200 animations, each playing one of 0x200 sets of up to 0x20 CEL
// frames. The frames load on first use; each animation advances with the clock.

#ifdef MW2_ACCEL
DECOMP_SIZE_ASSERT(AnimFrame, 0x14)
#else
DECOMP_SIZE_ASSERT(AnimFrame, 0x08)
#endif
DECOMP_SIZE_ASSERT(Animation, 0x0e)

// GLOBAL: MW2 0x100ad288
// GLOBAL: MW2MATROX 0x100aa1b8
// GLOBAL: MW23DFX 0x1024e480
MechS32 g_animInitialized = -2;

// GLOBAL: MW2 0x100ad28c
// GLOBAL: MW2MATROX 0x100aa1bc
// GLOBAL: MW23DFX 0x1024e484
MechS32 g_lumaResourceId = 0;

// GLOBAL: MW2 0x100ad290
// GLOBAL: MW2MATROX 0x100aa1c0
// GLOBAL: MW23DFX 0x1024e488
MechS32 g_currentAnimSet = 0;

// GLOBAL: MW2 0x100ad294
// GLOBAL: MW2MATROX 0x100aa1c4
// GLOBAL: MW23DFX 0x1024e48c
MechS32 g_animSetUsed = 0;

// GLOBAL: MW2 0x100ad298
// GLOBAL: MW2MATROX 0x100aa1c8
// GLOBAL: MW23DFX 0x1024e490
MechS32 g_animSetIsSequence = 0;

#ifdef MW2_ACCEL
// The accelerated editions' tint of the animated textures, for the palettes 0xc, 0x10 and 0x11
// (StartPaletteFade): whether it is on, and its color.
// GLOBAL: MW2MATROX 0x100aa1d0
// GLOBAL: MW23DFX 0x1024e494
MechS32 g_unk0x100aa1d0 = 0;
#endif

// GLOBAL: MW2 0x100ad29c
MechU16* g_lumaTables = NULL;

// GLOBAL: MW2 0x100ad2a0
// GLOBAL: MW2MATROX 0x100aa1d8
// GLOBAL: MW23DFX 0x1024e498
MechS32 g_preloadCels[] = {
#ifdef MW2_ACCEL
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

#ifdef MW2_3DFX
// The mode the ground's (2) and the sky's (1) polygons are drawn in (FUN_100674c0's p_mode).
// GLOBAL: MW23DFX 0x1024e634
MechS32 g_unk0x1024e634 = 0;
#endif

// GLOBAL: MW2 0x100c7610
// GLOBAL: MW2MATROX 0x10184710
// GLOBAL: MW23DFX 0x10160d20
AnimFrame g_animFrames[0x200][0x20];

// GLOBAL: MW2 0x100e7610
// GLOBAL: MW2MATROX 0x10182b00
// GLOBAL: MW23DFX 0x1015f0f0
Animation g_animations[0x200];

#ifdef MW2_ACCEL
// GLOBAL: MW2MATROX 0x10184700
// GLOBAL: MW23DFX 0x10160cf0
MechFloat g_unk0x10184700[3];
#endif

// GLOBAL: MW2 0x100e9210
// GLOBAL: MW23DFX 0x10160d00
WINDOW g_animFrameBuffer;

#ifdef MW2_ACCEL
// Rotates the texture point (*p_u, *p_v) about the texture's centre by the angle whose cosine and
// sine are p_cos and p_sin.
// The products' operands are loaded in the other order (commutative operand order); MW23DFX
// computes u * p_sin after v * p_cos.
// FUNCTION: MW2MATROX 0x10050d60
// FUNCTION: MW23DFX 0x1002c830
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
// MW23DFX's (0x1002c880) draws through its own renderer, in Glide's texture coordinates, and
// clips the polygon to the pane edges p_unk0x18 selects. MW23DFX: register allocation (the
// original keeps data on the stack and anim in ebx, the rebuild the other way around), which
// shifts every stack offset by 4.
// FUNCTION: MW2 0x10068d10
// FUNCTION: MW2MATROX 0x10050dbc
// FUNCTION: MW23DFX 0x1002c880
#ifdef MW2_ACCEL
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
#if defined(MW2_3DFX)
	Animation* anim;
	MechS16 height;
	MechS16 width;
	MechU32 flags;
	MechS32 rotate;
	MechS32 useLuma;
	MechS32 shade;
	MechFloat offset;
	MechFloat angle;
	MechFloat cosine;
	MechFloat sine;
	A3DVertex color;
	MechFloat uScale;
	MechFloat vScale;
	MechFloat w;
	AnimFrame* frame;
	A3DVertex* vertex;
	ProjectedVertex* point;
	ProjectedVertex** points;
	MechS32 i;
	MechU16* data;

	points = p_points;
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
	// A translucent texture is drawn from MW2.MIP, if it has it.
	data = NULL;
	if (p_unk0x1c) {
		data = (MechU16*) FUN_100669f0(frame->m_resourceId);
	}

	if (!data) {
		data = frame->m_data;
		if (!data) {
			data = LoadCachedResource(g_mw2PrjHandle, frame->m_resourceId, g_resTagCel, 0);
			if (!data) {
				return 0;
			}

			frame->m_data = data;
		}
	}

	width = data[0];
	height = data[1];
	data += 2;
	if (!(anim->m_flags & 4) && p_luma > -1 && p_luma < 15) {
		useLuma = TRUE;
	}
	else {
		useLuma = FALSE;
	}

	if (g_unk0x100aa1d0) {
		flags |= 1;
		shade = FALSE;
		color.m_red = g_unk0x10184700[0];
		color.m_green = g_unk0x10184700[1];
		color.m_blue = g_unk0x10184700[2];
	}
	else if ((p_direct || useLuma) && p_luma != (MechS32) 0x80000000) {
		shade = TRUE;
	}
	else {
		shade = FALSE;
		color.m_red = 255;
		color.m_green = 255;
		color.m_blue = 255;
	}

	vertex = g_a3dVertices;
	if (shade) {
		for (i = 0; i < p_count; i++) {
			point = *p_points++;
			vertex->m_x = point->m_screenX;
			vertex->m_blue = vertex->m_green = vertex->m_red = point->m_light * 255;
			vertex->m_y = point->m_screenY;
			vertex++;
		}
	}
	else {
		for (i = 0; i < p_count; i++) {
			point = *p_points++;
			vertex->m_red = color.m_red;
			vertex->m_green = color.m_green;
			vertex->m_blue = color.m_blue;
			vertex->m_x = point->m_screenX;
			vertex->m_y = point->m_screenY;
			vertex++;
		}
	}

	// Glide's texture coordinates run from 0 to 255 over the texture.
	vertex = g_a3dVertices;
	if (p_direct && !g_renderSettings.m_affineTextures) {
		uScale = 255.0f / width;
		vScale = 255.0f / height;
		for (i = 0; i < p_count; i++) {
			point = *points++;
			w = 2000.0f / point->m_z;
			vertex->m_tmuW = w;
			vertex->m_w = w;
			vertex->m_u = point->m_u * w * uScale;
			vertex->m_v = point->m_v * w * vScale;
			vertex++;
		}
	}
	else if (p_scale) {
		for (i = 0; i < p_count; i++) {
			point = *points++;
			vertex->m_tmuW = 1.0f;
			vertex->m_w = 1.0f;
			vertex->m_u = point->m_u * 255;
			vertex->m_v = point->m_v * 255;
			vertex++;
		}
	}
	else {
		uScale = 255.0f / width;
		vScale = 255.0f / height;
		if (rotate) {
			offset *= 255;
			for (i = 0; i < p_count; i++) {
				point = *points++;
				vertex->m_tmuW = 1.0f;
				vertex->m_u = point->m_u * uScale;
				vertex->m_v = point->m_v * vScale;
				vertex->m_u += offset;
				vertex->m_w = 1.0f;
				RotateTexturePoint(cosine, sine, &vertex->m_u, &vertex->m_v);
				vertex++;
			}
		}
		else {
			for (i = 0; i < p_count; i++) {
				point = *points++;
				vertex->m_tmuW = 1.0f;
				vertex->m_w = 1.0f;
				vertex->m_u = point->m_u * uScale;
				vertex->m_v = point->m_v * vScale;
				vertex++;
			}
		}
	}

	if (p_unk0x18) {
		p_count = FUN_10067b90(p_count, p_unk0x18, 0xd);
		if (p_count < 3) {
			return 0;
		}
	}

	if (p_direct) {
		FUN_1002d760(
			&g_currentPane,
			data,
			width,
			height,
			p_count,
			p_unk0x18 ? g_a3dClipped : g_a3dVertices,
			0,
			0,
			p_unk0x1c
		);
		return 1;
	}

	if (useLuma) {
		flags |= 1;
	}

	g_animFrameBuffer.m_buffer = (undefined*) data;
	g_animFrameBuffer.m_xMax = width - 1;
	g_animFrameBuffer.m_yMax = height - 1;
	A3D_map_polygon(&g_currentPane, p_count, p_unk0x18 ? g_a3dClipped : g_a3dVertices, &g_animFrameBuffer, flags);
	return 1;
#elif defined(MW2_MATROX)
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
	texture = A3D_LoadTexture(frame->m_resourceId, 1, 1);
	if (!texture) {
		return 0;
	}

	width = texture->m_width;
	height = texture->m_height;
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

		A3D_map_polygon(&g_currentPane, p_count, g_a3dVertices, texture, flags);
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
		data = LoadCachedResource(g_mw2PrjHandle, frame->m_resourceId, g_resTagCel, 0);
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
		g_lumaTables = LoadCachedResource(g_mw2PrjHandle, g_lumaResourceId, g_resTagLuma, 0);
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
// operands the other way around. MW23DFX: the same comparison and an addition's operand order,
// and the loop's end (0x10160cf0, g_unk0x10184700 in the original).
// FUNCTION: MW2 0x10068fb8
// FUNCTION: MW2MATROX 0x10051278
// FUNCTION: MW23DFX 0x1002ce50
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
// loads p_set first. MW23DFX: register allocation of p_set, found and slot.
// FUNCTION: MW2 0x10069124
// FUNCTION: MW2MATROX 0x100513ed
// FUNCTION: MW23DFX 0x1002cf10
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

#ifdef MW2_3DFX
// Creates MW2.MIP from the CEL resources 1-0x51b, unless it exists (FUN_10066b90): converts each
// into the file (FUN_10066c70), stopping at the first failure, and closes it.
// FUNCTION: MW23DFX 0x1002cfd0
void FUN_1002cfd0(void)
{
	MechS32 i;
	MechS32 failed;
	MechS16* data;

	if (FUN_10066b90()) {
		return;
	}

	for (i = 1; i < 0x51c; i++) {
		data = LoadCachedResource(g_mw2PrjHandle, i, g_resTagCel, 0);
		if (data) {
			failed = FUN_10066c70(data + 2, data[0], data[1], i);
			UnlockCachedResource(i, g_resTagCel);
			if (failed) {
				break;
			}
		}
	}

	FUN_10066c20();
}
#endif

// FUNCTION: MW2 0x1006923c
// FUNCTION: MW2MATROX 0x1005151a
// FUNCTION: MW23DFX 0x1002d030
void PreloadAnimCels(void)
{
	MechS32 i;

	for (i = 0; g_preloadCels[i] != -1; i++) {
		PreloadResource(g_preloadCels[i], g_resTagCel);
	}
}

// FUNCTION: MW2 0x10069288
// FUNCTION: MW2MATROX 0x10051566
// FUNCTION: MW23DFX 0x1002d060
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
// FUNCTION: MW23DFX 0x1002d0e0
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
// FUNCTION: MW23DFX 0x1002d150
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
// FUNCTION: MW23DFX 0x1002d190
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
// FUNCTION: MW23DFX 0x1002d1e0
void SetAnimDelay(MechS16 p_index, MechU16 p_delay)
{
	g_animations[p_index].m_delay = p_delay;
}

// FUNCTION: MW2 0x10069586
// FUNCTION: MW2MATROX 0x10051875
// FUNCTION: MW23DFX 0x1002d200
void FUN_10069586(void)
{
}

// FUNCTION: MW2 0x10069591
// FUNCTION: MW2MATROX 0x10051880
// FUNCTION: MW23DFX 0x1002d210
void FUN_10069591(void)
{
#ifdef MW2_3DFX
	FUN_100667f0();
#endif
}

// Index order: &g_animFrames[i][j] loads i first in the original (InitAnimations matches with the
// same statement). MW2MATROX loads them the other way around.
// FUNCTION: MW2 0x1006959c
// FUNCTION: MW2MATROX 0x1005188b
// FUNCTION: MW23DFX 0x1002d220
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
				UnlockCachedResource(frame->m_resourceId, g_resTagCel);
				frame->m_data = NULL;
			}

			frame->m_resourceId = 0;
			frame->m_useCount = 0;
		}
	}

	UnlockCachedResource(g_lumaResourceId, g_resTagLuma);
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

#ifdef MW2_ACCEL
#ifdef MW2_3DFX
// Returns the pixels of animation p_index's current frame (its CEL resource, from the cache), and
// its size; NULL if it has none.
// FUNCTION: MW23DFX 0x1002d2b0
MechU16* FUN_10051dfe(MechS32 p_index, MechS16* p_width, MechS16* p_height)
{
	MechU16* data;
	AnimFrame* frame;
	Animation* anim;

	anim = &g_animations[p_index];
	frame = &g_animFrames[anim->m_set][anim->m_frame];
	if (frame->m_resourceId < 1) {
		return NULL;
	}

	data = frame->m_data;
	if (!data) {
		data = LoadCachedResource(g_mw2PrjHandle, frame->m_resourceId, g_resTagCel, 0);
		if (!data) {
			return NULL;
		}
	}

	*p_width = data[0];
	*p_height = data[1];
	return data + 2;
}

#endif
// Computes the average color of animation p_index's current texture, 0-31 per component (the
// sums of the 5-bit components over an eighth of the pixel count); returns 0 if it has none.
// pixels < end compares in the other operand order.
// Stack-slot permutation of the locals.
// MW23DFX takes the texture from FUN_10051dfe (the resource cache) rather than reading it.
// FUNCTION: MW2MATROX 0x100519b0
// FUNCTION: MW23DFX 0x1002d330
MechS32 FUN_100519b0(MechS32 p_index, MechFloat* p_red, MechFloat* p_green, MechFloat* p_blue)
{
#ifdef MW2_3DFX
	MechU16* pixels;
	MechU16* end;
	MechU16 pixel;
	MechS16 width;
	MechS16 height;
	MechU32 count;
	MechU32 red;
	MechU32 green;
	MechU32 blue;

	red = 0;
	green = 0;
	blue = 0;
	pixels = FUN_10051dfe(p_index, &width, &height);
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
	return 1;
#else
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

	size = GetPrjResourceSize(g_mw2PrjHandle, g_resTagCel, frame->m_resourceId);
	if (!size) {
		return 0;
	}

	data = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, size);
	ReadPrjResource(g_mw2PrjHandle, g_resTagCel, frame->m_resourceId, data);
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

#endif
}

// Returns the average color of animation p_index's current frame (CalcAverageBitmapColors).
// MW23DFX: index order, g_animFrames[anim->m_set][anim->m_frame] loads m_set first.
// FUNCTION: MW2MATROX 0x10051ba1
// FUNCTION: MW23DFX 0x1002d410
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

#ifdef MW2_MATROX

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

// Returns the texture of animation p_index's current frame (A3D_LoadTexture), and its size.
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

	texture = A3D_LoadTexture(frame->m_resourceId, 1, p_mode);
	if (!texture) {
		return NULL;
	}

	*p_width = texture->m_width;
	*p_height = texture->m_height;
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

#endif
#ifdef MW2_3DFX
// Draws the polygon of p_count projected points with animation p_index + 0x100's texture (the
// ground's), clipped to the pane, subdivided for its perspective (FUN_100674c0's mode 2). Returns 0.
// Register allocation: the original keeps vertex in edi and g_unk0x100aa1d0 in ebx.
// FUNCTION: MW23DFX 0x1002d460
MechS32 FUN_10051c07(MechS32 p_index, ProjectedVertex** p_points, MechS32 p_count)
{
	MechU16* pixels;
	MechS32 i;
	MechS16 height;
	MechS16 width;
	ProjectedVertex* point;
	A3DVertex* vertex;
	MechFloat w;

	p_index += 0x100;
	pixels = FUN_10051dfe(p_index, &width, &height);
	vertex = g_a3dVertices;
	for (i = 0; i < p_count; i++) {
		point = *p_points;
		p_points++;
		vertex->m_x = point->m_screenX;
		vertex->m_y = point->m_screenY;
		w = 2000.0f / point->m_z;
		vertex->m_w = w;
		vertex->m_u = point->m_u * w;
		vertex->m_v = point->m_v * w;
		if (g_unk0x100aa1d0) {
			vertex->m_blue = g_unk0x10184700[2];
			vertex->m_green = g_unk0x10184700[1];
			vertex->m_red = g_unk0x10184700[0];
		}
		else {
			vertex->m_red = 255.0f;
			vertex->m_green = 255.0f;
			vertex->m_blue = 255.0f;
		}

		vertex++;
	}

	p_count = FUN_10067b90(p_count, 0xf, 0xc);
	if (p_count < 3) {
		return 0;
	}

	g_unk0x1024e634 = 2;
	FUN_1002d760(&g_currentPane, pixels, width, height, p_count, g_a3dClipped, 1, 0, 0);
	g_unk0x1024e634 = 0;
	return 0;
}

// FUN_10051c07 for the sky: its texture coordinates scaled to Glide's 0-255, drawn unclipped when
// the clipper leaves it whole, in FUN_100674c0's mode 1.
// Register allocation of p_count and vertex.
// FUNCTION: MW23DFX 0x1002d590
MechS32 FUN_10051ea2(MechS32 p_index, ProjectedVertex** p_points, MechS32 p_count)
{
	MechU16* pixels;
	MechS32 i;
	MechS32 count;
	MechS16 height;
	MechS16 width;
	ProjectedVertex* point;
	A3DVertex* vertex;
	MechFloat w;

	p_index += 0x100;
	pixels = FUN_10051dfe(p_index, &width, &height);
	vertex = g_a3dVertices;
	for (i = 0; i < p_count; i++) {
		point = *p_points;
		p_points++;
		vertex->m_x = point->m_screenX;
		vertex->m_y = point->m_screenY;
		w = 2000.0f / point->m_z;
		vertex->m_tmuW = w;
		vertex->m_w = w;
		vertex->m_u = point->m_u * w * 255.0f;
		vertex->m_v = point->m_v * w * 255.0f;
		if (g_unk0x100aa1d0) {
			vertex->m_blue = g_unk0x10184700[2];
			vertex->m_green = g_unk0x10184700[1];
			vertex->m_red = g_unk0x10184700[0];
		}
		else {
			vertex->m_red = 255.0f;
			vertex->m_green = 255.0f;
			vertex->m_blue = 255.0f;
		}

		vertex++;
	}

	count = FUN_10067b90(p_count, 0xf, 0xc);
	if (count < 0) {
		return 0;
	}

	g_unk0x1024e634 = 1;
	if (!count) {
		FUN_1002d760(&g_currentPane, pixels, width, height, p_count, g_a3dVertices, 1, 0, 0);
	}
	else {
		FUN_1002d760(&g_currentPane, pixels, width, height, count, g_a3dClipped, 1, 0, 0);
	}

	g_unk0x1024e634 = 0;
	return 0;
}
#endif

// Computes the average color of every playing animation's current frame (FUN_100519b0).
// Stack-slot permutation of the locals.
// FUNCTION: MW2MATROX 0x1005207d
// FUNCTION: MW23DFX 0x1002d6e0
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
#ifndef MW2_3DFX
		if (!anim) {
			continue;
		}
#endif

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

#ifdef MW2_3DFX
// Draws the textured polygon of p_count vertices p_vertices on p_pane (FUN_100674c0, in
// g_unk0x1024e634's mode), its texture the p_width x p_height pixels p_pixels, translucent with
// p_unk0x20.
// FUNCTION: MW23DFX 0x1002d760
void FUN_1002d760(
	PANE* p_pane,
	MechU16* p_pixels,
	MechS16 p_width,
	MechS16 p_height,
	MechS32 p_count,
	A3DVertex* p_vertices,
	undefined4 p_unk0x18,
	undefined4 p_unk0x1c,
	MechS32 p_unk0x20
)
{
	WINDOW texture;
	MechU32 flags;

	flags = 0;
	if (p_unk0x20) {
		flags = 2;
	}

	texture.m_buffer = (undefined*) p_pixels;
	texture.m_xMax = p_width - 1;
	texture.m_yMax = p_height - 1;
	FUN_100674c0(p_pane, p_count, p_vertices, &texture, flags, g_unk0x1024e634);
}
#endif
