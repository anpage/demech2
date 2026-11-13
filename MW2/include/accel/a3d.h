#ifndef ACCEL_A3D_H
#define ACCEL_A3D_H

#include "accel/a3dtexture.h"
#include "accel/a3dvertex.h"
#include "decomp.h"
#include "palettecolor.h"
#include "pane.h"
#include "projectedvertex.h"
#include "types.h"
#include "window.h"

#include <windows.h>

// The A3D renderer, which the accelerated editions draw their polygons through in place of VFX's
// (CLAUDE.md, "The A3D layer"): the game's interface to one back end per card. The Matrox edition's
// is matrox/a3d.c (0x1005a5f0-0x10067500, between damagepanel.c's and keyboard.c's objects),
// optimized code compiled with /Ox /G5 /Op; the 3Dfx edition's is 3dfx/a3d.c, over Glide.

// A vertex of the textured polygons A3D_GroundSkyPolyPlot draws, in doubles: the screen position, the depth
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

	// What both editions' renderers do.

	// Sets the display's palette (1.1 calls m_setPaletteWithBrightness).
	void A3D_SetPalette(PaletteColor* p_palette);
	// Draws the outline of the polygon p_vertices of p_count vertices on p_pane, in the color of its
	// first vertex.
	void A3D_DrawOutline(PANE* p_pane, A3DVertex* p_vertices, MechU32 p_count);
	// Draws the shaded polygon p_vertices of p_count vertices on p_pane.
	void A3D_DrawPolygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices);
	// Puts in p_out the point where the edge from p_a to p_b crosses p_edge on the axis p_axis (1 x,
	// 2 y), interpolating the colors (p_flags 1), the texture coordinates (4) and m_w (8).
	void A3D_EdgeCrossing(
		MechS32 p_axis,
		MechFloat p_edge,
		A3DVertex* p_a,
		A3DVertex* p_b,
		A3DVertex* p_out,
		MechU32 p_flags
	);

#ifdef MW2_MATROX
	// The texture modes the textured polygons take their textures in (1 for both).
	extern MechU32 g_unk0x100ac924;
	extern MechU32 g_unk0x100ac928;

	// Clips the polygon p_in of p_count projected vertices (ProjectPolygon's) against the view's
	// planes p_flags selects into p_out; returns the clipped polygon's vertex count.
	MechS32 FUN_1005a5f0(ProjectedVertex* p_out, ProjectedVertex* p_in, MechS32 p_count, MechU32 p_flags);
	// Releases the texture cache (a jump to FUN_1005d0a0).
	void FUN_1005d090(void);
	// Returns the cached texture of the CEL resource p_id (loading it with p_levels levels, 8-bit when
	// p_paletted, if it isn't cached).
	A3DTexture* A3D_LoadTexture(MechS32 p_id, MechS32 p_paletted, MechU32 p_levels);
	// Starts a frame (msiStartFrame), unless one is started.
	void FUN_1005f710(void);
	// Ends the frame (msiEndFrame), if one is started.
	void FUN_1005f760(void);
	// Takes the "Filter" registry setting (SimMain's settings, simmain.c).
	void FUN_1005f810(MechS32 p_filter);
	// Sets whether and in which 16-bit color the frame is cleared (red, green and blue as floats).
	void FUN_1005f820(MechS32 p_clear, MechU32 p_color);
	// Fills p_pane with a 16-bit color (DrawScene's pane wipe).
	void FUN_1005f8a0(PANE* p_pane, MechS32 p_color);
	// Draws a textured polygon of p_count vertices on p_pane (DrawAnimatedPolygon's, with p_flags 1
	// for its shades and 2 for its fourth argument).
	void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, A3DTexture* p_texture, MechU32 p_flags);
	// Draws a textured polygon, perspective-corrected with p_flags 0x200 or 0x400.
	void A3D_polygon_clip_XY_and_render(
		PANE* p_pane,
		MechU32 p_count,
		A3DVertex* p_vertices,
		MechU32 p_flags,
		undefined4 p_unk0x10,
		A3DTexture* p_texture,
		undefined4 p_unk0x18,
		undefined4 p_unk0x1c
	);
	// "A3D_GroundSkyPolyPlot()": draws the textured polygon p_vertices of p_count vertices on p_pane
	// (the ground's and the sky's), clipped to the depth p_depth or, negative, put at -p_depth.
	void A3D_GroundSkyPolyPlot(
		PANE* p_pane,
		MechU32 p_count,
		A3DPolyVertex* p_vertices,
		A3DTexture* p_texture,
		MechS32 p_unk0x10,
		MechDouble p_depth,
		MechS32 p_mip
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
	MechS32 A3D_Init(WNDPROC p_windowProc, MechS32 p_width, MechS32 p_height);
	// "A3D_shutdown()": closes the display (msiExit).
	void A3D_shutdown(void);

// The 3D view's pane wipe, with a 16-bit pixel: the Matrox edition's goes through the renderer.
#define A3D_PANE_WIPE(p_pane, p_color) FUN_1005f8a0(p_pane, p_color)
#endif

#ifdef MW2_3DFX
	// The 3Dfx edition's renderer, over Glide (3dfx/a3d.c). Its textures are VFX's: a WINDOW of
	// 16-bit texels, or the m_buffer of one in MW2.MIP, the file of textures it converts them into.

	// A texture of MW2.MIP: its size, and where its data is in the file. Texture IDs are CEL
	// resource IDs (1-0x51b); MW2.MIP starts with the 0x51c entries.
	// SIZE 0xc
	typedef struct A3DMipEntry {
		MechS16 m_width;  // 0x00
		MechS16 m_height; // 0x02
		MechS32 m_offset; // 0x04 — -1 for a texture the file doesn't have
		MechU32 m_size;   // 0x08
	} A3DMipEntry;

	// Whether A3D_Init opens the display at 800x600 (FUN_10066920).
	extern MechS32 g_a3dRes800x600;
	// The frame buffer is open for writing (FUN_10066930), at g_a3dLfb.
	extern MechS32 g_a3dLfbOpen;
	extern undefined* g_a3dLfb;

	// Opens Glide (at 800x600 with FUN_10066920's setting, 640x480 otherwise), its log file and
	// MW2.MIP; returns 0 on success.
	MechS32 A3D_Init(void);
	// Closes Glide, the log file and MW2.MIP, and frees the cached textures.
	void A3D_shutdown(void);
	// The graphics page's resolution setting (accel/graphicsmenu.c): nothing to do.
	void FUN_100667e0(MechS32 p_setting);
	// Resumes the frame buffer access FUN_100669a0 suspended, if it is suspended.
	void FUN_100667f0(void);
	void FUN_10066910(void);
	// Sets whether A3D_Init opens the display at 800x600 (SimMain's "Res800x600" setting).
	void FUN_10066920(MechS32 p_res800x600);
	// Starts writing to the frame buffer directly (the 2D panels).
	void FUN_10066930(void);
	// Ends FUN_10066930's frame buffer access.
	void FUN_10066970(void);
	// Suspends the frame buffer access for the 3D drawing.
	void FUN_100669a0(void);
	// Ends the 3D drawing, and resumes the frame buffer access it suspended.
	void FUN_100669d0(void);
	// Returns texture p_id's entry in MW2.MIP, or NULL if the file doesn't have it.
	A3DMipEntry* FUN_100669f0(MechS16 p_id);
	// Creates MW2.MIP, unless it exists; returns 0 when it is to be filled (FUN_10066c70).
	MechS32 FUN_10066b90(void);
	// Writes MW2.MIP's entries and closes it.
	void FUN_10066c20(void);
	// Converts texture p_id (16-bit texels p_pixels, p_width x p_height) and appends it to MW2.MIP;
	// returns nonzero on failure.
	MechS32 FUN_10066c70(void* p_pixels, MechS32 p_width, MechS32 p_height, MechS32 p_id);
	// Draws a textured polygon of p_count vertices on p_pane (DrawAnimatedPolygon's, with p_flags 1
	// for its shades and 2 for its translucency).
	void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, WINDOW* p_texture, MechU32 p_flags);
	// Draws the ground's and the sky's textured polygons on p_pane, subdivided to correct the
	// perspective when p_mode is 2.
	void FUN_100674c0(
		PANE* p_pane,
		MechS32 p_count,
		A3DVertex* p_vertices,
		WINDOW* p_texture,
		MechU32 p_flags,
		MechS32 p_mode
	);

// The 3D view's pane wipe, with a 16-bit pixel: VFX's (the 16-bit port, accel/vfx16.h).
#define A3D_PANE_WIPE(p_pane, p_color) VFX_pane_wipe(p_pane, p_color)
#endif

#ifdef __cplusplus
}
#endif

#endif // ACCEL_A3D_H
