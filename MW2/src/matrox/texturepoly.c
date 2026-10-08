/* The Matrox edition's textured polygons: one object of its own, between fixedmul.c's and
   render.c's, that hands DrawAnimatedPolygon's polygons to the A3D renderer. */
#include "matrox/texturepoly.h"

#include "decomp.h"
#include "matrox/a3d.h"
#include "pane.h"
#include "polydraw.h"
#include "types.h"

// Draws the textured polygon of p_count vertices p_vertices (the A3D renderer's, 12 floats each)
// in p_pane, with perspective correction unless the textures are affine. The texture's height is
// limited to 128 and to its width.
// FUNCTION: MW2MATROX 0x100172c0
void FUN_100172c0(
	PANE* p_pane,
	A3DTexture* p_texture,
	MechS32 p_count,
	A3DVertex* p_vertices,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14,
	MechS32 p_unk0x18
)
{
	MechU32 flags;

	flags = 0;
	if (p_unk0x18) {
		flags = 1;
	}

	if (g_renderSettings.m_affineTextures) {
		flags |= 0x400;
	}
	else {
		flags |= 0x600;
	}

	if (p_texture->m_height > 0x80) {
		p_texture->m_height = 0x80;
	}

	if (p_texture->m_width < p_texture->m_height) {
		p_texture->m_height = p_texture->m_width;
	}

	A3D_polygon_clip_XY_and_render(p_pane, p_count, p_vertices, flags, p_unk0x10, p_texture, p_unk0x14, 0);
}
