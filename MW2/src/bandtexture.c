/* The S3 edition's copy of the ATI edition's textured-polygon drawer (FUN_1002a480, bandpoly.c),
   which DrawAnimatedPolygon calls. Not a unit of its own: the ATI and PowerVR editions have it in
   bandpoly.c's object, ahead of DrawBand, the S3 edition at the start of palette.c's (its
   bandpoly.c object, DrawBand's, follows mainmenu.c's), so palette.c includes this file in the S3
   edition's build. bandpoly.c keeps its own text: including this file there changes the code VC++
   2.2 gives DrawBand. */
#include "accel/a3d.h"
#include "bandpoly.h"
#include "decomp.h"
#include "rendersettings.h"
#include "types.h"

// The ATI edition's: draws the textured polygon of p_count vertices p_vertices on p_pane, its
// texture the p_width x p_height pixels p_pixels (at most 128 texels square, as wide as it is high),
// with levels when p_mip, translucent with p_translucent; perspective-corrected when the render
// settings' member at 0x54 is clear (the 3Dfx edition's DrawAnimatedPolygon tests m_affineTextures,
// at 0x4c, in its place).
// FUNCTION: MW2S3 0x1003eaf0
void FUN_1002a480(
	PANE* p_pane,
	MechU16* p_pixels,
	MechS16 p_width,
	MechS16 p_height,
	MechS32 p_count,
	A3DVertex* p_vertices,
	MechS32 p_mip,
	undefined4 p_unk0x1c,
	MechS32 p_translucent
)
{
	A3DTextureRows texture;
	MechU16* rows[0x80];
	MechU32 flags;

	flags = 0;
	if (p_translucent) {
		flags = 1;
	}

	if (g_renderSettings.m_frameDrawCallback) {
		flags |= 0x400;
	}
	else {
		flags |= 0x600;
	}

	if (p_height > 0x80) {
		p_height = 0x80;
	}

	if (p_height > p_width) {
		p_height = p_width;
	}

	texture.m_rows = rows;
	texture.m_width = p_height;
	texture.m_height = p_height;
	rows[0] = p_pixels;
	A3D_polygon_clip_XY_and_render(p_pane, p_count, p_vertices, flags, p_mip, &texture, p_unk0x1c, 0);
}
