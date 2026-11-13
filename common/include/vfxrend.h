#ifndef VFXREND_H
#define VFXREND_H

#include "compat.h"
#include "decomp.h"
#include "pane.h"
#include "types.h"

// The texture a polygon maps (VFXREND.H's VFX_TEXTURE): the table of row ("V-baseline") addresses
// and the tiling limits (DrawTexturedPolygon sets both to the texture's height).
// SIZE 0xc
typedef struct VFX_TEXTURE {
	MechU8** m_vAddrs; // 0x00
	MechS32 m_width;   // 0x04
	MechS32 m_height;  // 0x08
} VFX_TEXTURE;

// VFXREND, Miles Design VFX's polygon renderer (3rdparty/vfx/VFXREND.ASM; common/src/vfxrend.c's
// portable C in COMPAT_MODE), which both DLLs link. p_cueing and p_translucency are lookaside
// tables: p_cueing of 256 bytes for flat shading, of 256 rows of 256 for Gouraud shading (a row
// per shade), p_translucency of 256 bytes. MW2 draws its textured polygons through
// VFX_polygon_clip_XY_and_render (DrawTexturedPolygon), and render.c asks GetCodeBlock for the range to
// make writable. In MW2SHELL the object is believed to be dead: nothing outside it refers into it,
// and .text is read-only with no VirtualProtect import, so its routines could not patch themselves.
#ifdef __cplusplus
extern "C"
{
#endif

	void __cdecl VFX_set_Gouraud_dither_level(MechS32 p_dither1, MechS32 p_dither2);
	MechS32 __cdecl GetCodeBlock(undefined4* p_start, undefined4* p_selector);
	void __cdecl VFX_polygon_render(
		PANE* p_pane,
		MechU32* p_vlist,
		MechS32 p_nvertices,
		MechS32 p_operation,
		undefined4 p_color,
		VFX_TEXTURE* p_texture,
		void* p_cueing,
		void* p_translucency
	);
	MechS32 __cdecl F16_div_to_F30(MechS32 p_dividend, MechS32 p_divisor);
	MechS32 __cdecl F30_reciprocal(MechS32 p_value);
	MechS32 __cdecl mul_F30(MechS32 p_m1, MechS32 p_m2);
	void __cdecl VFX_polygon_clip_XY_and_render(
		PANE* p_pane,
		MechU32* p_vlist,
		MechS32 p_nvertices,
		MechS32 p_operation,
		undefined4 p_color,
		VFX_TEXTURE* p_texture,
		void* p_cueing,
		void* p_translucency
	);

#ifdef __cplusplus
}
#endif

// VFXREND's routines and data. reccmp reads annotations from C sources only, so they're here, by
// name. The table of primitives (poly_vectors) is indexed by the operation flags; the primitives
// (poly_<flags>, in decimal) are the ones 3rdparty/vfx/RENDOPTS.INC builds, and patch their own
// operands.

// GLOBAL: MW2SHELL 0x10017a7c
// GLOBAL: MW2 0x10021cf4
// GLOBAL: MW2MATROX 0x1002f83c
// poly_vectors

// FUNCTION: MW2SHELL 0x10019a7c
// FUNCTION: MW2 0x10023cf4
// FUNCTION: MW2MATROX 0x1003183c
// FUNCTION: MW23DFX 0x1004c2f8
// poly_1681

// FUNCTION: MW2SHELL 0x1001a686
// FUNCTION: MW2 0x100248fe
// FUNCTION: MW2MATROX 0x10032446
// FUNCTION: MW23DFX 0x1004cf02
// poly_1553

// FUNCTION: MW2SHELL 0x1001b18e
// FUNCTION: MW2 0x10025406
// FUNCTION: MW2MATROX 0x10032f4e
// FUNCTION: MW23DFX 0x1004da0a
// poly_1169

// FUNCTION: MW2SHELL 0x1001bafb
// FUNCTION: MW2 0x10025d73
// FUNCTION: MW2MATROX 0x100338bb
// FUNCTION: MW23DFX 0x1004e377
// poly_1041

// FUNCTION: MW2SHELL 0x1001c366
// FUNCTION: MW2 0x100265de
// FUNCTION: MW2MATROX 0x10034126
// FUNCTION: MW23DFX 0x1004ebe2
// poly_1696

// FUNCTION: MW2SHELL 0x1001cd7c
// FUNCTION: MW2 0x10026ff4
// FUNCTION: MW2MATROX 0x10034b3c
// FUNCTION: MW23DFX 0x1004f5f8
// poly_1664

// FUNCTION: MW2SHELL 0x1001d853
// FUNCTION: MW2 0x10027acb
// FUNCTION: MW2MATROX 0x10035613
// FUNCTION: MW23DFX 0x100500cf
// poly_1687

// FUNCTION: MW2SHELL 0x1001e687
// FUNCTION: MW2 0x100288ff
// FUNCTION: MW2MATROX 0x10036447
// FUNCTION: MW23DFX 0x10050f03
// poly_1673

// FUNCTION: MW2SHELL 0x1001f491
// FUNCTION: MW2 0x10029709
// FUNCTION: MW2MATROX 0x10037251
// FUNCTION: MW23DFX 0x10051d0d
// poly_1713

// FUNCTION: MW2SHELL 0x1001ff8e
// FUNCTION: MW2 0x1002a206
// FUNCTION: MW2MATROX 0x10037d4e
// FUNCTION: MW23DFX 0x1005280a
// poly_1680

// FUNCTION: MW2SHELL 0x10020b5c
// FUNCTION: MW2 0x1002add4
// FUNCTION: MW2MATROX 0x1003891c
// FUNCTION: MW23DFX 0x100533d8
// poly_1536

// FUNCTION: MW2SHELL 0x100214f5
// FUNCTION: MW2 0x1002b76d
// FUNCTION: MW2MATROX 0x100392b5
// FUNCTION: MW23DFX 0x10053d71
// poly_1552

// FUNCTION: MW2SHELL 0x10021fc1
// FUNCTION: MW2 0x1002c239
// FUNCTION: MW2MATROX 0x10039d81
// FUNCTION: MW23DFX 0x1005483d
// poly_1616

// FUNCTION: MW2SHELL 0x10022ec2
// FUNCTION: MW2 0x1002d13a
// FUNCTION: MW2MATROX 0x1003ac82
// FUNCTION: MW23DFX 0x1005573e
// poly_1175

// FUNCTION: MW2SHELL 0x10023a55
// FUNCTION: MW2 0x1002dccd
// FUNCTION: MW2MATROX 0x1003b815
// FUNCTION: MW23DFX 0x100562d1
// poly_1168

// FUNCTION: MW2SHELL 0x10024386
// FUNCTION: MW2 0x1002e5fe
// FUNCTION: MW2MATROX 0x1003c146
// FUNCTION: MW23DFX 0x10056c02
// poly_1024

// FUNCTION: MW2SHELL 0x10024a82
// FUNCTION: MW2 0x1002ecfa
// FUNCTION: MW2MATROX 0x1003c842
// FUNCTION: MW23DFX 0x100572fe
// poly_1040

// FUNCTION: MW2SHELL 0x100252b1
// FUNCTION: MW2 0x1002f529
// FUNCTION: MW2MATROX 0x1003d071
// FUNCTION: MW23DFX 0x10057b2d
// poly_1032

// FUNCTION: MW2SHELL 0x10025ce7
// FUNCTION: MW2 0x1002ff5f
// FUNCTION: MW2MATROX 0x1003daa7
// FUNCTION: MW23DFX 0x10058563
// poly_1104

// FUNCTION: MW2SHELL 0x10026947
// FUNCTION: MW2 0x10030bbf
// FUNCTION: MW2MATROX 0x1003e707
// FUNCTION: MW23DFX 0x100591c3
// poly_128

// FUNCTION: MW2SHELL 0x10026cd6
// FUNCTION: MW2 0x10030f4e
// FUNCTION: MW2MATROX 0x1003ea96
// FUNCTION: MW23DFX 0x10059552
// poly_640

// FUNCTION: MW2SHELL 0x10026fc9
// FUNCTION: MW2 0x10031241
// FUNCTION: MW2MATROX 0x1003ed89
// FUNCTION: MW23DFX 0x10059845
// poly_64

// FUNCTION: MW2SHELL 0x1002763d
// FUNCTION: MW2 0x100318b5
// FUNCTION: MW2MATROX 0x1003f3fd
// FUNCTION: MW23DFX 0x10059eb9
// poly_768

// FUNCTION: MW2SHELL 0x10027aa6
// FUNCTION: MW2 0x10031d1e
// FUNCTION: MW2MATROX 0x1003f866
// FUNCTION: MW23DFX 0x1005a322
// poly_704

// FUNCTION: MW2SHELL 0x10028018
// FUNCTION: MW2 0x10032290
// FUNCTION: MW2MATROX 0x1003fdd8
// FUNCTION: MW23DFX 0x1005a894
// poly_576

// FUNCTION: MW2SHELL 0x100286a6
// FUNCTION: MW2 0x1003291e
// FUNCTION: MW2MATROX 0x10040466
// FUNCTION: MW23DFX 0x1005af22
// VFX_set_Gouraud_dither_level

// FUNCTION: MW2SHELL 0x100286c3
// FUNCTION: MW2 0x1003293b
// FUNCTION: MW2MATROX 0x10040483
// FUNCTION: MW23DFX 0x1005af3f
// GetCodeBlock

// FUNCTION: MW2SHELL 0x100286eb
// FUNCTION: MW2 0x10032963
// FUNCTION: MW2MATROX 0x100404ab
// VFX_polygon_render

// FUNCTION: MW2SHELL 0x1002875c
// FUNCTION: MW2 0x100329d4
// FUNCTION: MW2MATROX 0x1004051c
// FUNCTION: MW23DFX 0x1005afd8
// F16_div_to_F30

// FUNCTION: MW2SHELL 0x10028794
// FUNCTION: MW2 0x10032a0c
// FUNCTION: MW2MATROX 0x10040554
// FUNCTION: MW23DFX 0x1005b010
// F30_reciprocal

// FUNCTION: MW2SHELL 0x100287c4
// FUNCTION: MW2 0x10032a3c
// FUNCTION: MW2MATROX 0x10040584
// FUNCTION: MW23DFX 0x1005b040
// mul_F30

// FUNCTION: MW2SHELL 0x100287e0
// FUNCTION: MW2 0x10032a58
// FUNCTION: MW2MATROX 0x100405a0
// VFX_polygon_clip_XY_and_render

#endif // VFXREND_H
