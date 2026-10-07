#ifndef MATROX_VFX16_H
#define MATROX_VFX16_H

#include "pane.h"
#include "types.h"
#include "vfxa.h"

// The Matrox edition's 16-bit 2D primitives (0x100563dc-0x100591a0, after loadres.c): hand-written
// assembly in VFX's style (PROC frames that push es and clear the direction flag) that stores 16-bit
// pixels, with VFXA's interface. The game code draws through them wherever 1.1 calls VFXA's
// drawing primitives; VFXA itself is still linked, for its non-drawing helpers and supanim.c's
// offscreen panes. A unit that draws to the screen includes this header after vfxa.h (behind
// #ifdef MW2_MATROX), and its VFXA calls compile as calls to these.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 FUN_100565e7(
		PANE* p_pane,
		MechS32 p_x0,
		MechS32 p_y0,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_mode,
		MechS32 p_parm
	);
	void FUN_100570c5(PANE* p_pane, void* p_shapeTable, MechS32 p_shapeNumber, MechS32 p_hotX, MechS32 p_hotY);
	MechS32 FUN_10057725(
		void* p_shapeTable,
		MechS32 p_shapeNumber,
		MechS32 p_hotX,
		MechS32 p_hotY,
		undefined4 p_mirror,
		MechS32* p_rectangle
	);
	void FUN_1005789d(PANE* p_pane, MechS32 p_color);
	void FUN_10057986(PANE* p_pane, MechS32 p_xc, MechS32 p_yc, MechS32 p_width, MechS32 p_height, MechS32 p_color);
	void FUN_10058e45(PANE* p_pane, MechS32 p_x, MechS32 p_y, void* p_font, MechChar* p_string, void* p_colorTranslate);

#ifdef __cplusplus
}
#endif

#ifdef MW2_MATROX
#define VFX_line_draw FUN_100565e7
#define VFX_shape_draw FUN_100570c5
#define VFX_shape_visible_rectangle FUN_10057725
#define VFX_pane_wipe FUN_1005789d
#define VFX_ellipse_draw FUN_10057986
#define VFX_string_draw FUN_10058e45
#endif

#endif // MATROX_VFX16_H
