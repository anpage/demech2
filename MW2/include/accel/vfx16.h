#ifndef ACCEL_VFX16_H
#define ACCEL_VFX16_H

#include "compat.h"
#include "decomp.h"
#include "pane.h"
#include "types.h"
#include "vfxa.h"
#include "window.h"

// The accelerated editions' 16-bit 2D primitives (0x100563dc-0x100591a0 in the Matrox edition, after
// loadres.c): a 16-bit port
// of VFX 1.15's VFXA (3rdparty/vfx/VFXA.ASM), with VFXA's routines in VFXA's order and its
// interface, storing 16-bit pixels. MW2/src/accel/vfx16.asm, a MASM object; the 3Dfx, ATI and S3
// editions carry it too. The game code draws through it wherever 1.1 calls VFXA's drawing primitives; VFXA
// itself is still linked, for its non-drawing helpers and supanim.c's offscreen panes. A unit that
// draws to the screen includes this header after vfxa.h (behind #ifdef MW2_ACCEL), and its VFXA
// calls compile as calls to these.
//
// The routines take VFXA's names with a 16 suffix (VFX_line_draw16): VFXA's own publics are linked
// alongside, and the suffix keeps each one next to its counterpart. The two lookaside word routines
// are the port's own (declared in palette.h, whose PIXEL_COLOR uses them).
#ifdef __cplusplus
extern "C"
{
#endif

	MechChar* __cdecl VFX_driver_name16(void* p_vfxScanDll);
	void __cdecl VFX_register_driver16(void* p_dllBase);
	MechS32 __cdecl VFX_pixel_write16(PANE* p_pane, MechS32 p_x, MechS32 p_y, MechU32 p_color);
	MechS32 __cdecl VFX_pixel_read16(PANE* p_pane, MechS32 p_x, MechS32 p_y);
	MechS32 __cdecl VFX_line_draw16(
		PANE* p_pane,
		MechS32 p_x0,
		MechS32 p_y0,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_mode,
		MechS32 p_parm
	);
	void __cdecl VFX_shape_lookaside16(MechU16* p_table);
	void __cdecl VFX_shape_draw16(
		PANE* p_pane,
		void* p_shapeTable,
		MechS32 p_shapeNumber,
		MechS32 p_hotX,
		MechS32 p_hotY
	);
	void __cdecl DrawShapeUnclipped16(PANE* p_pane, void* p_shape, MechS32 p_hotX, MechS32 p_hotY, undefined4 p_cpW);
	MechS32 __cdecl VFX_shape_visible_rectangle16(
		void* p_shapeTable,
		MechS32 p_shapeNumber,
		MechS32 p_hotX,
		MechS32 p_hotY,
		undefined4 p_mirror,
		MechS32* p_rectangle
	);
	void __cdecl VFX_pane_wipe16(PANE* p_pane, MechS32 p_color);
	void __cdecl VFX_ellipse_draw16(
		PANE* p_pane,
		MechS32 p_xc,
		MechS32 p_yc,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 p_color
	);
	void __cdecl VFX_Cos_Sin16(MechS32 p_angle, MechS32* p_cos, MechS32* p_sin);
	void __cdecl VFX_fixed_mul16(MechS32 p_m1, MechS32 p_m2, MechS32* p_result);
	void __cdecl VFX_point_transform16(
		MechS32* p_in,
		MechS32* p_out,
		MechS32* p_origin,
		MechS32 p_rot,
		MechS32 p_xScale,
		MechS32 p_yScale
	);
	MechS32 __cdecl VFX_font_height16(void* p_font);
	MechS32 __cdecl VFX_character_width16(void* p_font, MechS32 p_character);
	MechS32 __cdecl VFX_character_draw16(
		PANE* p_pane,
		MechS32 p_x,
		MechS32 p_y,
		void* p_font,
		MechS32 p_character,
		void* p_colorTranslate
	);
	void __cdecl VFX_string_draw16(
		PANE* p_pane,
		MechS32 p_x,
		MechS32 p_y,
		void* p_font,
		MechChar* p_string,
		void* p_colorTranslate
	);
	MechS32 __cdecl VFX_line_to_pane16(PANE* p_target, MechS32 p_y, MechU8* p_lineBuffer, MechS32 p_lineLength);
	void __cdecl VFX_window_fade16(WINDOW* p_buffer, MechU8* p_palette, MechS32 p_intervals);
	MechS32 __cdecl VFX_color_scan16(PANE* p_pane, MechU32* p_colors);

	// The object's two tables, declared for their types (tools/asm2masm.py reads them): the cosine
	// table of VFX_Cos_Sin16, one 16.16 entry per tenth of a degree from 0 to 90, and the shape
	// lookaside table, a 16-bit pixel for each color.
	extern MechS32 CosTable16[0x385];
	extern MechU16 lookaside16[0x100];

#ifdef __cplusplus
}
#endif

#ifdef MW2_ACCEL
#define VFX_line_draw VFX_line_draw16
#define VFX_shape_draw VFX_shape_draw16
#define VFX_shape_visible_rectangle VFX_shape_visible_rectangle16
#define VFX_pane_wipe VFX_pane_wipe16
#define VFX_ellipse_draw VFX_ellipse_draw16
#define VFX_string_draw VFX_string_draw16
#endif

// The object's routines and data, by name (reccmp reads annotations from C sources only).

// FUNCTION: MW2MATROX 0x100563dc
// FUNCTION: MW23DFX 0x10061558
// VFX_driver_name16

// FUNCTION: MW2MATROX 0x10056405
// FUNCTION: MW23DFX 0x10061581
// VFX_register_driver16

// FUNCTION: MW2MATROX 0x10056424
// FUNCTION: MW23DFX 0x100615a0
// VFX_pixel_write16

// FUNCTION: MW2MATROX 0x10056509
// FUNCTION: MW23DFX 0x10061685
// VFX_pixel_read16

// FUNCTION: MW2MATROX 0x100565e7
// FUNCTION: MW23DFX 0x10061763
// VFX_line_draw16

// FUNCTION: MW2MATROX 0x1005706d
// FUNCTION: MW23DFX 0x100621e9
// VFX_shape_lookaside16

// FUNCTION: MW2MATROX 0x1005708c
// FUNCTION: MW23DFX 0x10062208
// VFX_lookaside_write16

// FUNCTION: MW2MATROX 0x100570aa
// FUNCTION: MW23DFX 0x10062226
// VFX_lookaside_read16

// FUNCTION: MW2MATROX 0x100570c5
// FUNCTION: MW23DFX 0x10062241
// VFX_shape_draw16

// FUNCTION: MW2MATROX 0x100575e7
// FUNCTION: MW23DFX 0x10062763
// DrawShapeUnclipped16

// FUNCTION: MW2MATROX 0x10057725
// VFX_shape_visible_rectangle16

// FUNCTION: MW2MATROX 0x1005789d
// FUNCTION: MW23DFX 0x100628a1
// VFX_pane_wipe16

// FUNCTION: MW2MATROX 0x10057986
// FUNCTION: MW23DFX 0x1006298a
// VFX_ellipse_draw16

// GLOBAL: MW2MATROX 0x10057cdf
// GLOBAL: MW23DFX 0x10062ce3
// CosTable16

// FUNCTION: MW2MATROX 0x10058af3
// FUNCTION: MW23DFX 0x10063af7
// VFX_Cos_Sin16

// FUNCTION: MW2MATROX 0x10058bab
// FUNCTION: MW23DFX 0x10063baf
// VFX_fixed_mul16

// FUNCTION: MW2MATROX 0x10058bd1
// FUNCTION: MW23DFX 0x10063bd5
// VFX_point_transform16

// FUNCTION: MW2MATROX 0x10058c98
// FUNCTION: MW23DFX 0x10063c9c
// VFX_font_height16

// FUNCTION: MW2MATROX 0x10058cab
// FUNCTION: MW23DFX 0x10063caf
// VFX_character_width16

// FUNCTION: MW2MATROX 0x10058ccb
// FUNCTION: MW23DFX 0x10063ccf
// VFX_character_draw16

// FUNCTION: MW2MATROX 0x10058e45
// FUNCTION: MW23DFX 0x10063e49
// VFX_string_draw16

// FUNCTION: MW2MATROX 0x10058e7c
// FUNCTION: MW23DFX 0x10063e80
// VFX_line_to_pane16

// FUNCTION: MW2MATROX 0x10058f87
// FUNCTION: MW23DFX 0x10063f8b
// VFX_window_fade16

// FUNCTION: MW2MATROX 0x1005910b
// FUNCTION: MW23DFX 0x1006410f
// VFX_color_scan16

// The object's data: VFXA's, in VFXA's order (the lookaside table holds words).

// GLOBAL: MW2MATROX 0x100aab34
// GLOBAL: MW23DFX 0x1025ea94
// VFX_describe_driver16

// GLOBAL: MW2MATROX 0x100aab38
// GLOBAL: MW23DFX 0x1025ea98
// VFX_init_driver16

// GLOBAL: MW2MATROX 0x100aab3c
// GLOBAL: MW23DFX 0x1025ea9c
// VFX_shutdown_driver16

// GLOBAL: MW2MATROX 0x100aab40
// GLOBAL: MW23DFX 0x1025eaa0
// VFX_area_wipe16

// GLOBAL: MW2MATROX 0x100aab44
// GLOBAL: MW23DFX 0x1025eaa4
// VFX_wait_vblank16

// GLOBAL: MW2MATROX 0x100aab48
// GLOBAL: MW23DFX 0x1025eaa8
// VFX_wait_vblank_leading16

// GLOBAL: MW2MATROX 0x100aab4c
// GLOBAL: MW23DFX 0x1025eaac
// VFX_window_refresh16

// GLOBAL: MW2MATROX 0x100aab50
// GLOBAL: MW23DFX 0x1025eab0
// VFX_window_read16

// GLOBAL: MW2MATROX 0x100aab54
// GLOBAL: MW23DFX 0x1025eab4
// VFX_DAC_read16

// GLOBAL: MW2MATROX 0x100aab58
// GLOBAL: MW23DFX 0x1025eab8
// VFX_DAC_write16

// GLOBAL: MW2MATROX 0x100aab5c
// GLOBAL: MW23DFX 0x1025eabc
// VFX_bank_reset16

// GLOBAL: MW2MATROX 0x100aab60
// GLOBAL: MW23DFX 0x1025eac0
// VFX_pane_refresh16

// GLOBAL: MW2MATROX 0x100aab64
// GLOBAL: MW23DFX 0x1025eac4
// VFX_line_address16

// GLOBAL: MW2MATROX 0x100aab68
// GLOBAL: MW23DFX 0x1025eac8
// driver_name16

// GLOBAL: MW2MATROX 0x100aab75
// GLOBAL: MW23DFX 0x1025ead5
// SHP_building16

// GLOBAL: MW2MATROX 0x100aab79
// GLOBAL: MW23DFX 0x1025ead9
// SHP_skipCount16

// GLOBAL: MW2MATROX 0x100aab7d
// GLOBAL: MW23DFX 0x1025eadd
// SHP_LinePtr16

// GLOBAL: MW2MATROX 0x100aab81
// GLOBAL: MW23DFX 0x1025eae1
// SHP_ScanPtr16

// GLOBAL: MW2MATROX 0x100aab85
// GLOBAL: MW23DFX 0x1025eae5
// SHP_ShapePtr16

// GLOBAL: MW2MATROX 0x100aab89
// GLOBAL: MW23DFX 0x1025eae9
// SHP_FlushPtr16

// GLOBAL: MW2MATROX 0x100aab8d
// GLOBAL: MW23DFX 0x1025eaed
// SHP_CP_L16

// GLOBAL: MW2MATROX 0x100aab91
// GLOBAL: MW23DFX 0x1025eaf1
// SHP_CP_T16

// GLOBAL: MW2MATROX 0x100aab95
// GLOBAL: MW23DFX 0x1025eaf5
// SHP_CP_R16

// GLOBAL: MW2MATROX 0x100aab99
// GLOBAL: MW23DFX 0x1025eaf9
// SHP_CP_B16

// GLOBAL: MW2MATROX 0x100aab9d
// GLOBAL: MW23DFX 0x1025eafd
// SHP_minX16

// GLOBAL: MW2MATROX 0x100aaba1
// GLOBAL: MW23DFX 0x1025eb01
// SHP_minY16

// GLOBAL: MW2MATROX 0x100aaba5
// GLOBAL: MW23DFX 0x1025eb05
// SHP_maxX16

// GLOBAL: MW2MATROX 0x100aaba9
// GLOBAL: MW23DFX 0x1025eb09
// SHP_maxY16

// GLOBAL: MW2MATROX 0x100aabad
// GLOBAL: MW23DFX 0x1025eb0d
// temp_bufferA16

// GLOBAL: MW2MATROX 0x100ab0ad
// GLOBAL: MW23DFX 0x1025f00d
// color_buffer16

// GLOBAL: MW2MATROX 0x100ab3ad
// GLOBAL: MW23DFX 0x1025f30d
// color_list16

// GLOBAL: MW2MATROX 0x100ab4ad
// GLOBAL: MW23DFX 0x1025f40d
// color_delta16

// GLOBAL: MW2MATROX 0x100ab7ad
// GLOBAL: MW23DFX 0x1025f70d
// dest_ytable16

// GLOBAL: MW2MATROX 0x100abdad
// GLOBAL: MW23DFX 0x1025fd0d
// color_sb16

// GLOBAL: MW2MATROX 0x100ac0ad
// GLOBAL: MW23DFX 0x1026000d
// color_error16

// GLOBAL: MW2MATROX 0x100ac3ad
// GLOBAL: MW23DFX 0x1026030d
// GIF_cmask16

// GLOBAL: MW2MATROX 0x100ac3b6
// GLOBAL: MW23DFX 0x10260316
// GIF_inctable16

// GLOBAL: MW2MATROX 0x100ac3bb
// GLOBAL: MW23DFX 0x1026031b
// GIF_starttable16

// GLOBAL: MW2MATROX 0x100ac3c0
// GLOBAL: MW23DFX 0x10260320
// GIF_pane16

// GLOBAL: MW2MATROX 0x100ac3c4
// GLOBAL: MW23DFX 0x10260324
// lookaside16

// GLOBAL: MW2MATROX 0x100ac5c4
// vert0_16

// GLOBAL: MW2MATROX 0x100ac5d8
// vert1_16

// GLOBAL: MW2MATROX 0x100ac5ec
// vert2_16

// GLOBAL: MW2MATROX 0x100ac600
// vert3_16

// GLOBAL: MW2MATROX 0x100ac614
// src_step16

#endif // ACCEL_VFX16_H
