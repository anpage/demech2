/* The Matrox edition's A3D renderer (0x1005a5f0 to 0x100673c0): optimized code that no compiler at
   hand reproduces (CLAUDE.md, "The A3D layer"). Stubs for the functions the /Od units call. */
#include "matrox/a3d.h"

#include "decomp.h"
#include "palettecolor.h"
#include "types.h"

#include <windows.h>

// GLOBAL: MW2MATROX 0x100ac924
MechU32 g_unk0x100ac924 = 1;

// GLOBAL: MW2MATROX 0x100ac928
MechU32 g_unk0x100ac928 = 1;

// STUB: MW2MATROX 0x1005a5f0
MechU32 FUN_1005a5f0(ProjectedVertex* p_out, ProjectedVertex* p_in, MechU32 p_count, MechS32 p_unk0x10)
{
	STUB(0x1005a5f0);
	return 0;
}

// STUB: MW2MATROX 0x1005d090
void FUN_1005d090(void)
{
	STUB(0x1005d090);
}

// STUB: MW2MATROX 0x1005d600
A3DTexture* FUN_1005d600(MechS32 p_id, MechS32 p_unk0x08, MechU32 p_mode)
{
	STUB(0x1005d600);
	return NULL;
}

// STUB: MW2MATROX 0x1005f710
void FUN_1005f710(void)
{
	STUB(0x1005f710);
}

// STUB: MW2MATROX 0x1005f760
void FUN_1005f760(void)
{
	STUB(0x1005f760);
}

// STUB: MW2MATROX 0x1005f790
void FUN_1005f790(PaletteColor* p_palette)
{
	STUB(0x1005f790);
}

// STUB: MW2MATROX 0x1005f810
void FUN_1005f810(MechS32 p_filter)
{
	STUB(0x1005f810);
}

// STUB: MW2MATROX 0x1005f820
void FUN_1005f820(MechS32 p_clear, MechU32 p_color)
{
	STUB(0x1005f820);
}

// Draws the outline of a polygon.
// STUB: MW2MATROX 0x10061890
void FUN_10061890(PANE* p_pane, A3DVertex* p_vertices, MechS32 p_count)
{
	STUB(0x10061890);
}

// Draws a shaded polygon.
// STUB: MW2MATROX 0x10061cb0
void FUN_10061cb0(PANE* p_pane, MechS32 p_count, A3DVertex* p_vertices)
{
	STUB(0x10061cb0);
}

// Draws a textured polygon.
// STUB: MW2MATROX 0x10062630
void FUN_10062630(
	PANE* p_pane,
	MechS32 p_count,
	A3DVertex* p_vertices,
	MechU32 p_flags,
	undefined4 p_unk0x10,
	A3DTexture* p_texture,
	undefined4 p_unk0x18,
	undefined4 p_unk0x1c
)
{
	STUB(0x10062630);
}

// STUB: MW2MATROX 0x10062cd0
MechS32 FUN_10062cd0(
	PANE* p_pane,
	MechS32 p_count,
	A3DPolyVertex* p_vertices,
	A3DTexture* p_texture,
	MechS32 p_unk0x10,
	MechDouble p_depth,
	MechS32 p_unk0x1c
)
{
	STUB(0x10062cd0);
	return 0;
}

// Puts in p_out the point where the edge from p_b to p_a crosses p_edge on the axis p_axis (1 x,
// 2 y).
// STUB: MW2MATROX 0x10066a50
void FUN_10066a50(MechS32 p_axis, MechFloat p_edge, A3DVertex* p_a, A3DVertex* p_b, A3DVertex* p_out, MechS32 p_unk0x14)
{
	STUB(0x10066a50);
}

// STUB: MW2MATROX 0x10066c00
void FUN_10066c00(undefined* p_heap)
{
	STUB(0x10066c00);
}

// STUB: MW2MATROX 0x10066c10
undefined* FUN_10066c10(void)
{
	STUB(0x10066c10);
	return NULL;
}

// STUB: MW2MATROX 0x10066c40
void FUN_10066c40(
	undefined* p_heap,
	undefined* p_buffer,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height,
	MechS32 p_color
)
{
	STUB(0x10066c40);
}

// STUB: MW2MATROX 0x10066d30
MechS32 FUN_10066d30(WNDPROC p_windowProc, MechS32 p_width, MechS32 p_height)
{
	STUB(0x10066d30);
	return -1;
}

// STUB: MW2MATROX 0x100671e0
void FUN_100671e0(void)
{
	STUB(0x100671e0);
}
