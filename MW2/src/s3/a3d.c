/* The S3 ViRGE edition's A3D renderer (accel/a3d.h): the game's polygons drawn through S3's toolkit
   (S3DTKW.DLL) on DirectDraw surfaces. Not decompiled yet: these stubs stand in for the entry points the shared code
   calls, so that the edition builds. */
#include "accel/a3d.h"

#include "accel/a3dvertex.h"
#include "decomp.h"
#include "palettecolor.h"
#include "pane.h"
#include "types.h"
#include "window.h"

void A3D_SetPalette(PaletteColor* p_palette)
{
}

void A3D_DrawOutline(PANE* p_pane, A3DVertex* p_vertices, MechU32 p_count)
{
}

void A3D_DrawPolygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices)
{
}

void A3D_EdgeCrossing(
	MechS32 p_axis,
	MechFloat p_edge,
	A3DVertex* p_a,
	A3DVertex* p_b,
	A3DVertex* p_out,
	MechU32 p_flags
)
{
}

void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, WINDOW* p_texture, MechU32 p_flags)
{
}

void A3D_polygon_clip_XY_and_render(
	PANE* p_pane,
	MechU32 p_count,
	A3DVertex* p_vertices,
	MechU32 p_flags,
	MechS32 p_mip,
	A3DTextureRows* p_texture,
	undefined4 p_unk0x18,
	undefined4 p_unk0x1c
)
{
}

void A3D_shutdown(void)
{
}
