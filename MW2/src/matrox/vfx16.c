#include "matrox/vfx16.h"

#include "decomp.h"

// The Matrox edition's 16-bit 2D primitives (see vfx16.h). Stubs until the object's source is
// found or reproduced as a MASM unit.

// The 16-bit VFX_line_draw.
// STUB: MW2MATROX 0x100565e7
MechS32 FUN_100565e7(
	PANE* p_pane,
	MechS32 p_x0,
	MechS32 p_y0,
	MechS32 p_x1,
	MechS32 p_y1,
	MechS32 p_mode,
	MechS32 p_parm
)
{
	STUB(0x100565e7);
	return 0;
}

// The 16-bit VFX_shape_draw.
// STUB: MW2MATROX 0x100570c5
void FUN_100570c5(PANE* p_pane, void* p_shapeTable, MechS32 p_shapeNumber, MechS32 p_hotX, MechS32 p_hotY)
{
	STUB(0x100570c5);
}

// VFX_shape_visible_rectangle, in the 16-bit object.
// STUB: MW2MATROX 0x10057725
MechS32 FUN_10057725(
	void* p_shapeTable,
	MechS32 p_shapeNumber,
	MechS32 p_hotX,
	MechS32 p_hotY,
	undefined4 p_mirror,
	MechS32* p_rectangle
)
{
	STUB(0x10057725);
	return 0;
}

// The 16-bit VFX_pane_wipe.
// STUB: MW2MATROX 0x1005789d
void FUN_1005789d(PANE* p_pane, MechS32 p_color)
{
	STUB(0x1005789d);
}

// The 16-bit VFX_ellipse_draw.
// STUB: MW2MATROX 0x10057986
void FUN_10057986(PANE* p_pane, MechS32 p_xc, MechS32 p_yc, MechS32 p_width, MechS32 p_height, MechS32 p_color)
{
	STUB(0x10057986);
}

// The 16-bit VFX_string_draw: draws each character through the 16-bit VFX_character_draw (0x10058ccb).
// STUB: MW2MATROX 0x10058e45
void FUN_10058e45(PANE* p_pane, MechS32 p_x, MechS32 p_y, void* p_font, MechChar* p_string, void* p_colorTranslate)
{
	STUB(0x10058e45);
}
