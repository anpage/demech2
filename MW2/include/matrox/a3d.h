#ifndef MATROX_A3D_H
#define MATROX_A3D_H

#include "decomp.h"
#include "matrox/a3dtexture.h"
#include "matrox/a3dvertex.h"
#include "palettecolor.h"
#include "pane.h"
#include "projectedvertex.h"
#include "types.h"

#include <windows.h>

// The Matrox edition's A3D renderer (0x1005a5f0-0x100673c0, between damagepanel.c's and
// keyboard.c's objects): optimized code the edition draws its polygons through, in place of VFX's
// (CLAUDE.md, "The A3D layer"). It isn't written yet: matrox/a3d.c stubs the entry points the game
// code calls.

// A vertex of the textured polygons FUN_10062cd0 draws, in doubles: the screen position, the depth
// and the projection scale over it, the color (0-255 per component) and the texture coordinates
// (times m_w).
// SIZE 0x60
typedef struct A3DPolyVertex {
	MechDouble m_x;                   // 0x00
	MechDouble m_y;                   // 0x08
	MechDouble m_z;                   // 0x10
	MechDouble m_w;                   // 0x18
	MechDouble m_red;                 // 0x20
	MechDouble m_green;               // 0x28
	MechDouble m_blue;                // 0x30
	undefined m_unk0x38[0x50 - 0x38]; // 0x38
	MechDouble m_u;                   // 0x50
	MechDouble m_v;                   // 0x58
} A3DPolyVertex;

#ifdef __cplusplus
extern "C"
{
#endif

	// The texture modes the textured polygons take their textures in (1 for both).
	extern MechU32 g_unk0x100ac924;
	extern MechU32 g_unk0x100ac928;

	// Clips the polygon p_in of p_count projected vertices (ProjectPolygon's) into p_out; returns
	// the clipped polygon's vertex count.
	MechU32 FUN_1005a5f0(ProjectedVertex* p_out, ProjectedVertex* p_in, MechU32 p_count, MechS32 p_unk0x10);
	// Releases the texture cache (a jump to FUN_1005d0a0).
	void FUN_1005d090(void);
	// Returns the cached texture of the CEL resource p_id.
	A3DTexture* FUN_1005d600(MechS32 p_id, MechS32 p_unk0x08, MechU32 p_mode);
	// Starts a frame (msiStartFrame), unless one is started.
	void FUN_1005f710(void);
	// Ends the frame (msiEndFrame), if one is started.
	void FUN_1005f760(void);
	// Sets the display's palette (1.1 calls m_setPaletteWithBrightness).
	void FUN_1005f790(PaletteColor* p_palette);
	// Takes the "Filter" registry setting (SimMain's settings, simmain.c).
	void FUN_1005f810(MechS32 p_filter);
	// Sets whether and in which 16-bit color the frame is cleared (red, green and blue as floats).
	void FUN_1005f820(MechS32 p_clear, MechU32 p_color);
	void FUN_10061890(PANE* p_pane, A3DVertex* p_vertices, MechS32 p_count);
	void FUN_10061cb0(PANE* p_pane, MechS32 p_count, A3DVertex* p_vertices);
	void FUN_10062630(
		PANE* p_pane,
		MechS32 p_count,
		A3DVertex* p_vertices,
		MechU32 p_flags,
		undefined4 p_unk0x10,
		A3DTexture* p_texture,
		undefined4 p_unk0x18,
		undefined4 p_unk0x1c
	);
	// Draws a textured polygon of p_count vertices on p_pane.
	MechS32 FUN_10062cd0(
		PANE* p_pane,
		MechS32 p_count,
		A3DPolyVertex* p_vertices,
		A3DTexture* p_texture,
		MechS32 p_unk0x10,
		MechDouble p_depth,
		MechS32 p_unk0x1c
	);
	void FUN_10066a50(
		MechS32 p_axis,
		MechFloat p_edge,
		A3DVertex* p_a,
		A3DVertex* p_b,
		A3DVertex* p_out,
		MechS32 p_unk0x14
	);
	// Frees the texture heap FUN_10066c10 allocated (msiFreeTextureHeap).
	void FUN_10066c00(undefined* p_heap);
	// Allocates the texture heap (msiAllocTextureHeap), which holds the frame buffers.
	undefined* FUN_10066c10(void);
	// Fills a rectangle of the frame buffer p_buffer of the heap p_heap with a 16-bit color.
	void FUN_10066c40(
		undefined* p_heap,
		undefined* p_buffer,
		MechS32 p_left,
		MechS32 p_top,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 p_color
	);
	// Opens MSI95.DLL's display at p_width x p_height (msiInit), with p_windowProc as its window's
	// procedure; returns 0 on success.
	MechS32 FUN_10066d30(WNDPROC p_windowProc, MechS32 p_width, MechS32 p_height);
	// "A3D_shutdown()": closes the display (msiExit).
	void FUN_100671e0(void);

#ifdef __cplusplus
}
#endif

#endif // MATROX_A3D_H
