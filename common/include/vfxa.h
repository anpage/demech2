#ifndef VFXA_H
#define VFXA_H

#include "compat.h"
#include "decomp.h"
#include "pane.h"
#include "types.h"
#include "window.h"

// VFXA, Miles Design VFX's 2D primitives (3rdparty/vfx/VFXA.ASM; its portable C, common/src/vfxa.c,
// in COMPAT_MODE), which both DLLs link.
// The accelerated editions link it too, assembled with MW2_ACCEL: their shape drawers, VFX_pane_wipe
// and VFX_character_draw keep the release's fills and copies where 1.1's align them, and they have a
// routine 1.1's doesn't, VFX_lookaside_write (one entry of the shape lookaside table; our name).
#ifdef __cplusplus
extern "C"
{
#endif

	// The registered display driver's entry points (VFX.H's hardware-specific functions), in the
	// order VFX_register_driver copies them from the driver's table.
	extern void*(__cdecl* VFX_describe_driver)(void);
	extern void(__cdecl* VFX_init_driver)(void);
	extern void(__cdecl* VFX_shutdown_driver)(void);
	extern void(__cdecl* VFX_area_wipe)(MechS32 p_x0, MechS32 p_y0, MechS32 p_x1, MechS32 p_y1, MechS32 p_color);
	extern void(__cdecl* VFX_wait_vblank)(void);
	extern void(__cdecl* VFX_wait_vblank_leading)(void);
	extern void(__cdecl* VFX_window_refresh)(WINDOW* p_target, MechS32 p_x0, MechS32 p_y0, MechS32 p_x1, MechS32 p_y1);
	extern void(__cdecl*
					VFX_window_read)(WINDOW* p_destination, MechS32 p_x0, MechS32 p_y0, MechS32 p_x1, MechS32 p_y1);
	extern void(__cdecl* VFX_DAC_read)(MechS32 p_colorNumber, MechU8* p_triplet);
	extern void(__cdecl* VFX_DAC_write)(MechS32 p_colorNumber, MechU8* p_triplet);
	extern void(__cdecl* VFX_bank_reset)(void);
	extern void(__cdecl* VFX_pane_refresh)(PANE* p_target, MechS32 p_x0, MechS32 p_y0, MechS32 p_x1, MechS32 p_y1);
	extern void(__cdecl* VFX_line_address)(MechS32 p_x, MechS32 p_y, MechU8** p_addr, MechU32* p_nbytes);

	MechChar* __cdecl VFX_driver_name(MechChar* (**p_vfxScanDll)(void) );
	void __cdecl VFX_register_driver(MechChar* (**p_dllBase)(void) );
	MechS32 __cdecl VFX_pixel_write(PANE* p_pane, MechS32 p_x, MechS32 p_y, MechU32 p_color);
	MechS32 __cdecl VFX_pixel_read(PANE* p_pane, MechS32 p_x, MechS32 p_y);
	MechS32 __cdecl VFX_line_draw(
		PANE* p_pane,
		MechS32 p_x0,
		MechS32 p_y0,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_mode,
		MechS32 p_parm
	);
	void __cdecl VFX_rectangle_hash(
		PANE* p_pane,
		MechS32 p_x0,
		MechS32 p_y0,
		MechS32 p_x1,
		MechS32 p_y1,
		MechU8 p_color
	);
	void __cdecl DrawShapeUnclipped(PANE* p_pane, void* p_shape, MechS32 p_hotX, MechS32 p_hotY, undefined4 p_cpW);
	MechS32 __cdecl XlatShapeUnclipped(PANE* p_pane, void* p_shape, MechS32 p_hotX, MechS32 p_hotY, undefined4 p_cpW);
	void __cdecl VFX_shape_draw(
		PANE* p_pane,
		void* p_shapeTable,
		MechS32 p_shapeNumber,
		MechS32 p_hotX,
		MechS32 p_hotY
	);
	void __cdecl VFX_shape_lookaside(MechU8* p_table);
	MechS32 __cdecl VFX_shape_translate_draw(
		PANE* p_pane,
		void* p_shapeTable,
		MechS32 p_shapeNumber,
		MechS32 p_hotX,
		MechS32 p_hotY
	);
	MechS32 __cdecl VFX_shape_remap_colors(void* p_shapeTable, MechS32 p_shapeNumber);
	MechS32 __cdecl VFX_shape_visible_rectangle(
		void* p_shapeTable,
		MechS32 p_shapeNumber,
		MechS32 p_hotX,
		MechS32 p_hotY,
		undefined4 p_mirror,
		MechS32* p_rectangle
	);
	MechS32 __cdecl VFX_shape_bounds(void* p_shapeTable, MechS32 p_shapeNum);
	MechS32 __cdecl VFX_shape_origin(void* p_shapeTable, MechS32 p_shapeNum);
	MechS32 __cdecl VFX_shape_resolution(void* p_shapeTable, MechS32 p_shapeNum);
	MechS32 __cdecl VFX_shape_minxy(void* p_shapeTable, MechS32 p_shapeNum);
	void __cdecl VFX_shape_palette(void* p_shapeTable, MechS32 p_shapeNum, MechU8* p_palette);
	MechS32 __cdecl VFX_shape_colors(void* p_shapeTable, MechS32 p_shapeNum, MechU32* p_colors);
	MechS32 __cdecl VFX_shape_set_colors(void* p_shapeTable, MechS32 p_shapeNumber, MechU32* p_colors);
	MechS32 __cdecl VFX_shape_count(void* p_shapeTable);
	MechS32 __cdecl VFX_shape_list(void* p_shapeTable, MechS32* p_indexList);
	MechS32 __cdecl VFX_shape_palette_list(void* p_shapeTable, MechS32* p_indexList);
	void __cdecl VFX_pane_wipe(PANE* p_pane, MechS32 p_color);
	MechS32 __cdecl VFX_pane_copy(
		PANE* p_source,
		MechS32 p_sx,
		MechS32 p_sy,
		PANE* p_target,
		MechS32 p_tx,
		MechS32 p_ty,
		MechS32 p_fill
	);
	MechS32 __cdecl VFX_pane_scroll(PANE* p_pane, MechS32 p_dx, MechS32 p_dy, MechS32 p_mode, MechS32 p_parm);
	void __cdecl VFX_ellipse_draw(
		PANE* p_pane,
		MechS32 p_xc,
		MechS32 p_yc,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 p_color
	);
	void __cdecl VFX_ellipse_fill(
		PANE* p_pane,
		MechS32 p_xc,
		MechS32 p_yc,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 p_color
	);
	void __cdecl VFX_Cos_Sin(MechS32 p_angle, MechS32* p_cos, MechS32* p_sin);
	void __cdecl VFX_fixed_mul(MechS32 p_m1, MechS32 p_m2, MechS32* p_result);
	void __cdecl VFX_point_transform(
		MechS32* p_in,
		MechS32* p_out,
		MechS32* p_origin,
		MechS32 p_rot,
		MechS32 p_xScale,
		MechS32 p_yScale
	);
	MechS32 __cdecl VFX_font_height(void* p_font);
	MechS32 __cdecl VFX_character_width(void* p_font, MechS32 p_character);
	MechS32 __cdecl VFX_character_draw(
		PANE* p_pane,
		MechS32 p_x,
		MechS32 p_y,
		void* p_font,
		MechS32 p_character,
		void* p_colorTranslate
	);
	void __cdecl VFX_string_draw(
		PANE* p_pane,
		MechS32 p_x,
		MechS32 p_y,
		void* p_font,
		MechChar* p_string,
		void* p_colorTranslate
	);
	MechS32 __cdecl VFX_line_to_pane(PANE* p_target, MechS32 p_y, MechU8* p_lineBuffer, MechS32 p_lineLength);
	MechU8* __cdecl find_ILBM_property(MechChar* p_tag, MechU8* p_iff);
	MechS32 __cdecl VFX_ILBM_draw(PANE* p_pane, MechU8* p_ilbmBuffer);
	void __cdecl VFX_ILBM_palette(MechU8* p_ilbmBuffer, MechU8* p_palette);
	MechS32 __cdecl VFX_ILBM_resolution(MechU8* p_ilbmBuffer);
	MechS32 __cdecl VFX_PCX_draw(PANE* p_pane, MechU8* p_pcxBuffer);
	void __cdecl VFX_PCX_palette(MechU8* p_pcxBuffer, MechS32 p_pcxFileSize, void* p_palette);
	MechS32 __cdecl VFX_PCX_resolution(MechU8* p_pcxBuffer);
	MechS32 __cdecl VFX_GIF_draw(PANE* p_pane, MechU8* p_gifBuffer, MechU8* p_gifScratch);
	void __cdecl VFX_GIF_palette(MechU8* p_gifBuffer, MechU8* p_palette);
	MechS32 __cdecl VFX_GIF_resolution(void* p_gifBuffer);
	MechS32 __cdecl VFX_shape_transform(
		PANE* p_pane,
		void* p_shapeTable,
		MechS32 p_shapeNumber,
		MechS32 p_hotX,
		MechS32 p_hotY,
		MechU8* p_buffer,
		MechS32 p_rot,
		MechS32 p_xScale,
		MechS32 p_yScale,
		MechU32 p_flags
	);
	MechS32 __cdecl VFX_shape_scan(
		PANE* p_pane,
		MechU8 p_transparentColor,
		MechS32 p_hotX,
		MechS32 p_hotY,
		MechU8* p_buffer
	);
	MechS32 __cdecl VFX_pixel_fade(PANE* p_source, PANE* p_destination, MechS32 p_intervals, MechS32 p_rnd);
	void __cdecl VFX_window_fade(WINDOW* p_buffer, MechU8* p_palette, MechS32 p_intervals);
	MechS32 __cdecl VFX_color_scan(PANE* p_pane, MechU32* p_colors);

#ifdef __cplusplus
}
#endif

// VFXA's routines and data. reccmp reads annotations from C sources only, so they're here, by
// name.

// FUNCTION: MW2SHELL 0x10032250
// FUNCTION: MW2 0x100604f4
// FUNCTION: MW2MATROX 0x1004ab24
// FUNCTION: MW23DFX 0x1005b884
// VFX_driver_name

// FUNCTION: MW2SHELL 0x10032279
// FUNCTION: MW2 0x1006051d
// FUNCTION: MW2MATROX 0x1004ab4d
// FUNCTION: MW23DFX 0x1005b8ad
// VFX_register_driver

// FUNCTION: MW2SHELL 0x10032298
// FUNCTION: MW2 0x1006053c
// FUNCTION: MW2MATROX 0x1004ab6c
// FUNCTION: MW23DFX 0x1005b8cc
// VFX_pixel_write

// FUNCTION: MW2SHELL 0x10032373
// FUNCTION: MW2 0x10060617
// FUNCTION: MW2MATROX 0x1004ac47
// FUNCTION: MW23DFX 0x1005b9a7
// VFX_pixel_read

// FUNCTION: MW2SHELL 0x10032449
// FUNCTION: MW2 0x100606ed
// FUNCTION: MW2MATROX 0x1004ad1d
// FUNCTION: MW23DFX 0x1005ba7d
// VFX_line_draw

// FUNCTION: MW2SHELL 0x10032e4b
// FUNCTION: MW2 0x100610ef
// FUNCTION: MW2MATROX 0x1004b71f
// FUNCTION: MW23DFX 0x1005c47f
// VFX_rectangle_hash

// FUNCTION: MW2SHELL 0x10032f84
// FUNCTION: MW2 0x10061228
// FUNCTION: MW2MATROX 0x1004b858
// FUNCTION: MW23DFX 0x1005c5b8
// VFX_shape_draw

// FUNCTION: MW2SHELL 0x100333f8
// FUNCTION: MW2 0x1006169c
// FUNCTION: MW2MATROX 0x1004bc5c
// FUNCTION: MW23DFX 0x1005c9bc
// DrawShapeUnclipped

// FUNCTION: MW2SHELL 0x100334fb
// FUNCTION: MW2 0x1006179f
// FUNCTION: MW2MATROX 0x1004bd23
// FUNCTION: MW23DFX 0x1005ca83
// VFX_shape_lookaside

// FUNCTION: MW2MATROX 0x1004bd42
// FUNCTION: MW23DFX 0x1005caa2
// VFX_lookaside_write

// FUNCTION: MW2SHELL 0x1003351a
// FUNCTION: MW2 0x100617be
// FUNCTION: MW2MATROX 0x1004bd5e
// FUNCTION: MW23DFX 0x1005cabe
// VFX_shape_translate_draw

// FUNCTION: MW2SHELL 0x10033980
// FUNCTION: MW2 0x10061c24
// FUNCTION: MW2MATROX 0x1004c236
// FUNCTION: MW23DFX 0x1005cf96
// XlatShapeUnclipped

// FUNCTION: MW2SHELL 0x10033a76
// FUNCTION: MW2 0x10061d1a
// FUNCTION: MW2MATROX 0x1004c369
// FUNCTION: MW23DFX 0x1005d0c9
// VFX_shape_transform

// FUNCTION: MW2SHELL 0x10034622
// FUNCTION: MW2 0x100628c6
// FUNCTION: MW2MATROX 0x1004cf15
// FUNCTION: MW23DFX 0x1005dc75
// VFX_shape_visible_rectangle

// FUNCTION: MW2SHELL 0x1003479a
// FUNCTION: MW2 0x10062a3e
// FUNCTION: MW2MATROX 0x1004d08d
// FUNCTION: MW23DFX 0x1005dded
// VFX_shape_scan

// FUNCTION: MW2SHELL 0x10034a1d
// FUNCTION: MW2 0x10062cc1
// FUNCTION: MW2MATROX 0x1004d310
// FUNCTION: MW23DFX 0x1005e070
// VFX_shape_remap_colors

// FUNCTION: MW2SHELL 0x10034aaf
// FUNCTION: MW2 0x10062d53
// FUNCTION: MW2MATROX 0x1004d3a2
// FUNCTION: MW23DFX 0x1005e102
// ScanLine

// FUNCTION: MW2SHELL 0x10034c38
// FUNCTION: MW2 0x10062edc
// FUNCTION: MW2MATROX 0x1004d52b
// FUNCTION: MW23DFX 0x1005e28b
// FlushPacket

// FUNCTION: MW2SHELL 0x10034e15
// FUNCTION: MW2 0x100630b9
// FUNCTION: MW2MATROX 0x1004d708
// FUNCTION: MW23DFX 0x1005e468
// VFX_pane_wipe

// FUNCTION: MW2SHELL 0x10034f18
// FUNCTION: MW2 0x100631bc
// FUNCTION: MW2MATROX 0x1004d7e7
// FUNCTION: MW23DFX 0x1005e547
// VFX_pane_copy

// FUNCTION: MW2SHELL 0x100352b4
// FUNCTION: MW2 0x10063558
// FUNCTION: MW2MATROX 0x1004db83
// FUNCTION: MW23DFX 0x1005e8e3
// VFX_pane_scroll

// FUNCTION: MW2SHELL 0x100354b1
// FUNCTION: MW2 0x10063755
// FUNCTION: MW2MATROX 0x1004dd80
// FUNCTION: MW23DFX 0x1005eae0
// VFX_ellipse_draw

// FUNCTION: MW2SHELL 0x100357f2
// FUNCTION: MW2 0x10063a96
// FUNCTION: MW2MATROX 0x1004e0c1
// FUNCTION: MW23DFX 0x1005ee21
// VFX_ellipse_fill

// GLOBAL: MW2SHELL 0x10035af0
// GLOBAL: MW2 0x10063d94
// GLOBAL: MW2MATROX 0x1004e3bf
// GLOBAL: MW23DFX 0x1005f11f
// CosTable

// FUNCTION: MW2SHELL 0x10036904
// FUNCTION: MW2 0x10064ba8
// FUNCTION: MW2MATROX 0x1004f1d3
// FUNCTION: MW23DFX 0x1005ff33
// VFX_Cos_Sin

// FUNCTION: MW2SHELL 0x100369bc
// FUNCTION: MW2 0x10064c60
// FUNCTION: MW2MATROX 0x1004f28b
// FUNCTION: MW23DFX 0x1005ffeb
// VFX_fixed_mul

// FUNCTION: MW2SHELL 0x100369e2
// FUNCTION: MW2 0x10064c86
// FUNCTION: MW2MATROX 0x1004f2b1
// FUNCTION: MW23DFX 0x10060011
// VFX_point_transform

// FUNCTION: MW2SHELL 0x10036aa9
// FUNCTION: MW2 0x10064d4d
// FUNCTION: MW2MATROX 0x1004f378
// FUNCTION: MW23DFX 0x100600d8
// VFX_font_height

// FUNCTION: MW2SHELL 0x10036abc
// FUNCTION: MW2 0x10064d60
// FUNCTION: MW2MATROX 0x1004f38b
// FUNCTION: MW23DFX 0x100600eb
// VFX_character_width

// FUNCTION: MW2SHELL 0x10036adc
// FUNCTION: MW2 0x10064d80
// FUNCTION: MW2MATROX 0x1004f3ab
// FUNCTION: MW23DFX 0x1006010b
// VFX_character_draw

// FUNCTION: MW2SHELL 0x10036c67
// FUNCTION: MW2 0x10064f0b
// FUNCTION: MW2MATROX 0x1004f53e
// FUNCTION: MW23DFX 0x1006029e
// VFX_string_draw

// FUNCTION: MW2SHELL 0x10036c9e
// FUNCTION: MW2 0x10064f42
// FUNCTION: MW2MATROX 0x1004f575
// FUNCTION: MW23DFX 0x100602d5
// VFX_line_to_pane

// GLOBAL: MW2SHELL 0x10036da1
// GLOBAL: MW2 0x10065045
// GLOBAL: MW2MATROX 0x1004f678
// GLOBAL: MW23DFX 0x100603d8
// BMHD_prop

// GLOBAL: MW2SHELL 0x10036da5
// GLOBAL: MW2 0x10065049
// GLOBAL: MW2MATROX 0x1004f67c
// GLOBAL: MW23DFX 0x100603dc
// CMAP_prop

// GLOBAL: MW2SHELL 0x10036da9
// GLOBAL: MW2 0x1006504d
// GLOBAL: MW2MATROX 0x1004f680
// GLOBAL: MW23DFX 0x100603e0
// BODY_prop

// FUNCTION: MW2SHELL 0x10036dad
// FUNCTION: MW2 0x10065051
// FUNCTION: MW2MATROX 0x1004f684
// FUNCTION: MW23DFX 0x100603e4
// find_ILBM_property

// FUNCTION: MW2SHELL 0x10036def
// FUNCTION: MW2 0x10065093
// FUNCTION: MW2MATROX 0x1004f6c6
// FUNCTION: MW23DFX 0x10060426
// VFX_ILBM_draw

// FUNCTION: MW2SHELL 0x10036fb6
// FUNCTION: MW2 0x1006525a
// FUNCTION: MW2MATROX 0x1004f88d
// FUNCTION: MW23DFX 0x100605ed
// VFX_ILBM_palette

// FUNCTION: MW2SHELL 0x10036fe7
// FUNCTION: MW2 0x1006528b
// FUNCTION: MW2MATROX 0x1004f8be
// FUNCTION: MW23DFX 0x1006061e
// VFX_ILBM_resolution

// FUNCTION: MW2SHELL 0x10037014
// FUNCTION: MW2 0x100652b8
// FUNCTION: MW2MATROX 0x1004f8eb
// FUNCTION: MW23DFX 0x1006064b
// VFX_PCX_draw

// FUNCTION: MW2SHELL 0x10037096
// FUNCTION: MW2 0x1006533a
// FUNCTION: MW2MATROX 0x1004f96d
// FUNCTION: MW23DFX 0x100606cd
// VFX_PCX_palette

// FUNCTION: MW2SHELL 0x100370c1
// FUNCTION: MW2 0x10065365
// FUNCTION: MW2MATROX 0x1004f998
// FUNCTION: MW23DFX 0x100606f8
// VFX_PCX_resolution

// FUNCTION: MW2SHELL 0x100370e8
// FUNCTION: MW2 0x1006538c
// FUNCTION: MW2MATROX 0x1004f9bf
// FUNCTION: MW23DFX 0x1006071f
// GIF_init_codetable

// FUNCTION: MW2SHELL 0x10037130
// FUNCTION: MW2 0x100653d4
// FUNCTION: MW2MATROX 0x1004fa07
// FUNCTION: MW23DFX 0x10060767
// GIF_getb

// FUNCTION: MW2SHELL 0x10037149
// FUNCTION: MW2 0x100653ed
// FUNCTION: MW2MATROX 0x1004fa20
// FUNCTION: MW23DFX 0x10060780
// GIF_getbcode

// FUNCTION: MW2SHELL 0x1003718f
// FUNCTION: MW2 0x10065433
// FUNCTION: MW2MATROX 0x1004fa66
// FUNCTION: MW23DFX 0x100607c6
// GIF_insertcode

// FUNCTION: MW2SHELL 0x100371d5
// FUNCTION: MW2 0x10065479
// FUNCTION: MW2MATROX 0x1004faac
// FUNCTION: MW23DFX 0x1006080c
// GIF_dopixel

// FUNCTION: MW2SHELL 0x10037252
// FUNCTION: MW2 0x100654f6
// FUNCTION: MW2MATROX 0x1004fb29
// FUNCTION: MW23DFX 0x10060889
// VFX_GIF_draw

// FUNCTION: MW2SHELL 0x1003746b
// FUNCTION: MW2 0x1006570f
// FUNCTION: MW2MATROX 0x1004fd42
// FUNCTION: MW23DFX 0x10060aa2
// VFX_GIF_palette

// FUNCTION: MW2SHELL 0x100374cc
// FUNCTION: MW2 0x10065770
// FUNCTION: MW2MATROX 0x1004fda3
// FUNCTION: MW23DFX 0x10060b03
// VFX_GIF_resolution

// FUNCTION: MW2SHELL 0x10037504
// FUNCTION: MW2 0x100657a8
// FUNCTION: MW2MATROX 0x1004fddb
// FUNCTION: MW23DFX 0x10060b3b
// VFX_shape_bounds

// FUNCTION: MW2SHELL 0x10037526
// FUNCTION: MW2 0x100657ca
// FUNCTION: MW2MATROX 0x1004fdfd
// FUNCTION: MW23DFX 0x10060b5d
// VFX_shape_origin

// FUNCTION: MW2SHELL 0x10037549
// FUNCTION: MW2 0x100657ed
// FUNCTION: MW2MATROX 0x1004fe20
// FUNCTION: MW23DFX 0x10060b80
// VFX_shape_resolution

// FUNCTION: MW2SHELL 0x1003757d
// FUNCTION: MW2 0x10065821
// FUNCTION: MW2MATROX 0x1004fe54
// FUNCTION: MW23DFX 0x10060bb4
// VFX_shape_minxy

// FUNCTION: MW2SHELL 0x100375a7
// FUNCTION: MW2 0x1006584b
// FUNCTION: MW2MATROX 0x1004fe7e
// FUNCTION: MW23DFX 0x10060bde
// VFX_shape_palette

// FUNCTION: MW2SHELL 0x100375f2
// FUNCTION: MW2 0x10065896
// FUNCTION: MW2MATROX 0x1004fec9
// FUNCTION: MW23DFX 0x10060c29
// VFX_shape_colors

// FUNCTION: MW2SHELL 0x1003763a
// FUNCTION: MW2 0x100658de
// FUNCTION: MW2MATROX 0x1004ff11
// FUNCTION: MW23DFX 0x10060c71
// VFX_shape_set_colors

// FUNCTION: MW2SHELL 0x10037684
// FUNCTION: MW2 0x10065928
// FUNCTION: MW2MATROX 0x1004ff5b
// FUNCTION: MW23DFX 0x10060cbb
// VFX_shape_count

// FUNCTION: MW2SHELL 0x10037697
// FUNCTION: MW2 0x1006593b
// FUNCTION: MW2MATROX 0x1004ff6e
// FUNCTION: MW23DFX 0x10060cce
// VFX_shape_list

// FUNCTION: MW2SHELL 0x100376f9
// FUNCTION: MW2 0x1006599d
// FUNCTION: MW2MATROX 0x1004ffd0
// FUNCTION: MW23DFX 0x10060d30
// VFX_shape_palette_list

// GLOBAL: MW2SHELL 0x1003775b
// GLOBAL: MW2 0x100659ff
// GLOBAL: MW2MATROX 0x10050032
// GLOBAL: MW23DFX 0x10060d92
// pf_constants

// FUNCTION: MW2SHELL 0x100377d7
// FUNCTION: MW2 0x10065a7b
// FUNCTION: MW2MATROX 0x100500ae
// FUNCTION: MW23DFX 0x10060e0e
// VFX_pixel_fade

// FUNCTION: MW2SHELL 0x10037a4e
// FUNCTION: MW2 0x10065cf2
// FUNCTION: MW2MATROX 0x10050325
// FUNCTION: MW23DFX 0x10061085
// VFX_window_fade

// FUNCTION: MW2SHELL 0x10037bd2
// FUNCTION: MW2 0x10065e76
// FUNCTION: MW2MATROX 0x100504a9
// FUNCTION: MW23DFX 0x10061209
// VFX_color_scan

// GLOBAL: MW2SHELL 0x100687cc
// GLOBAL: MW2 0x100ab100
// GLOBAL: MW2MATROX 0x100a8758
// GLOBAL: MW23DFX 0x1025d0a4
// VFX_describe_driver

// GLOBAL: MW2SHELL 0x100687d0
// GLOBAL: MW2 0x100ab104
// GLOBAL: MW2MATROX 0x100a875c
// GLOBAL: MW23DFX 0x1025d0a8
// VFX_init_driver

// GLOBAL: MW2SHELL 0x100687d4
// GLOBAL: MW2 0x100ab108
// GLOBAL: MW2MATROX 0x100a8760
// GLOBAL: MW23DFX 0x1025d0ac
// VFX_shutdown_driver

// GLOBAL: MW2SHELL 0x100687d8
// GLOBAL: MW2 0x100ab10c
// GLOBAL: MW2MATROX 0x100a8764
// GLOBAL: MW23DFX 0x1025d0b0
// VFX_area_wipe

// GLOBAL: MW2SHELL 0x100687dc
// GLOBAL: MW2 0x100ab110
// GLOBAL: MW2MATROX 0x100a8768
// GLOBAL: MW23DFX 0x1025d0b4
// VFX_wait_vblank

// GLOBAL: MW2SHELL 0x100687e0
// GLOBAL: MW2 0x100ab114
// GLOBAL: MW2MATROX 0x100a876c
// GLOBAL: MW23DFX 0x1025d0b8
// VFX_wait_vblank_leading

// GLOBAL: MW2SHELL 0x100687e4
// GLOBAL: MW2 0x100ab118
// GLOBAL: MW2MATROX 0x100a8770
// GLOBAL: MW23DFX 0x1025d0bc
// VFX_window_refresh

// GLOBAL: MW2SHELL 0x100687e8
// GLOBAL: MW2 0x100ab11c
// GLOBAL: MW2MATROX 0x100a8774
// GLOBAL: MW23DFX 0x1025d0c0
// VFX_window_read

// GLOBAL: MW2SHELL 0x100687ec
// GLOBAL: MW2 0x100ab120
// GLOBAL: MW2MATROX 0x100a8778
// GLOBAL: MW23DFX 0x1025d0c4
// VFX_DAC_read

// GLOBAL: MW2SHELL 0x100687f0
// GLOBAL: MW2 0x100ab124
// GLOBAL: MW2MATROX 0x100a877c
// GLOBAL: MW23DFX 0x1025d0c8
// VFX_DAC_write

// GLOBAL: MW2SHELL 0x100687f4
// GLOBAL: MW2 0x100ab128
// GLOBAL: MW2MATROX 0x100a8780
// GLOBAL: MW23DFX 0x1025d0cc
// VFX_bank_reset

// GLOBAL: MW2SHELL 0x100687f8
// GLOBAL: MW2 0x100ab12c
// GLOBAL: MW2MATROX 0x100a8784
// GLOBAL: MW23DFX 0x1025d0d0
// VFX_pane_refresh

// GLOBAL: MW2SHELL 0x100687fc
// GLOBAL: MW2 0x100ab130
// GLOBAL: MW2MATROX 0x100a8788
// GLOBAL: MW23DFX 0x1025d0d4
// VFX_line_address

// GLOBAL: MW2SHELL 0x10068800
// GLOBAL: MW2 0x100ab134
// GLOBAL: MW2MATROX 0x100a878c
// GLOBAL: MW23DFX 0x1025d0d8
// driver_name

#endif // VFXA_H
