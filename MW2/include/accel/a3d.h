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
// optimized code compiled with /Ox /G5 /Op; the 3Dfx edition's is 3dfx/a3d.c, over Glide; the ATI
// edition's is ati/a3d.c, over ATI's 3D CIF; the S3 edition's is s3/a3d.c, over S3's toolkit; the
// PowerVR edition's is powervr/a3d.c, over SGL.

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
// The texture filter setting (MW2_TEXTURE_FILTER), passed to the renderer.
#define A3D_SET_TEXTURE_FILTER(p_filter) FUN_1005f810(p_filter)
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

#ifdef MW2_ATI
	// The ATI edition's renderer, over ATI's 3D CIF (ati/a3d.c). Its textures are VFX's: a WINDOW of
	// 16-bit texels, converted into a DirectDraw surface the CIF registers.

	// A texture as A3D_polygon_clip_XY_and_render takes it: a table of row pointers (only the first is set, the
	// pixels) and the size.
	// SIZE 0xc
	typedef struct A3DTextureRows {
		MechU16** m_rows; // 0x0
		MechS32 m_width;  // 0x4
		MechS32 m_height; // 0x8
	} A3DTextureRows;

	// Takes the "Filter" registry setting (SimMain's settings and the graphics page): 1 filters
	// magnified textures, 2 minified ones too, anything else neither.
	void FUN_100648a0(MechS32 p_filter);
	// Draws a textured polygon of p_count vertices on p_pane (DrawAnimatedPolygon's, with p_flags 1
	// for its shades and 2 for its translucency).
	void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, WINDOW* p_texture, MechU32 p_flags);
	// Draws the textured polygon p_vertices of p_count vertices on p_pane, its texture p_texture with
	// mip levels when p_mip: p_flags 1 is translucent, 0x400 or 0x600 perspective-corrected. The last
	// two arguments aren't read.
	void A3D_polygon_clip_XY_and_render(
		PANE* p_pane,
		MechU32 p_count,
		A3DVertex* p_vertices,
		MechU32 p_flags,
		MechS32 p_mip,
		A3DTextureRows* p_texture,
		undefined4 p_unk0x18,
		undefined4 p_unk0x1c
	);
	// Ends the CIF's rendering, if it is rendering.
	void FUN_10063d30(void);
	// Closes the textures, the CIF's context and the CIF.
	void A3D_shutdown(void);

// The texture filter setting (MW2_TEXTURE_FILTER), passed to the renderer.
#define A3D_SET_TEXTURE_FILTER(p_filter) FUN_100648a0(p_filter)

// The 3D view's pane wipe, with a 16-bit pixel: VFX's (the 16-bit port, accel/vfx16.h).
#define A3D_PANE_WIPE(p_pane, p_color) VFX_pane_wipe(p_pane, p_color)
#endif

#ifdef MW2_S3
	// The S3 edition's renderer, over S3's toolkit (s3/a3d.c). Its textures are VFX's: a WINDOW of
	// 16-bit texels, copied into a DirectDraw surface the toolkit textures from.

	// A texture as A3D_polygon_clip_XY_and_render takes it: a table of row pointers (only the first is set, the
	// pixels) and the size.
	// SIZE 0xc
	typedef struct A3DTextureRows {
		MechU16** m_rows; // 0x0
		MechS32 m_width;  // 0x4
		MechS32 m_height; // 0x8
	} A3DTextureRows;

	// A palette color as the renderer takes it, a byte per component (0-252), blue first like the
	// toolkit's vertices.
	// SIZE 0x3
	typedef struct A3DPaletteColor {
		MechU8 m_blue;  // 0x0
		MechU8 m_green; // 0x1
		MechU8 m_red;   // 0x2
	} A3DPaletteColor;

	// The palette's colors as the renderer takes them (A3D_SetPalette).
	extern A3DPaletteColor g_a3dPalette[0x100];

	// Waits until the toolkit's engine is idle (FUN_1000f800), done with p_surface: the display back
	// end calls it once it has unlocked the frame buffer.
	void FUN_1000f840(undefined* p_surface);
	// Waits until the engine is idle, then takes p_surface as the frame buffer to draw on (the display
	// back end's, once it has locked it).
	void FUN_1000f850(undefined* p_surface);
	// Takes the "Filter" registry setting (SimMain's settings and the graphics page): nonzero filters
	// the textures (4 texels per pixel), 0 doesn't.
	void FUN_100102b0(MechS32 p_filter);
	// Has the textures that may be mip-mapped mip-mapped (p_mipmapped).
	void FUN_10010470(MechS32 p_mipmapped);
	// Frees the textures.
	void FUN_10010150(void);
	// Returns the log of p_size, rounded down.
	MechU32 FUN_1000f9b0(MechU32 p_size);
	// Writes the mip level under p_pixels (2 << p_log texels square) to p_out; returns its end.
	MechU16* FUN_1000fd60(MechU16* p_pixels, MechU16* p_out, MechU32 p_log);
	// Draws a textured polygon of p_count vertices on p_pane (DrawAnimatedPolygon's, with p_flags 1
	// for its shades and 2 for its translucency).
	void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, WINDOW* p_texture, MechU32 p_flags);
	// Draws the textured polygon p_vertices of p_count vertices on p_pane, its texture p_texture with
	// mip levels when p_mip: p_flags 1 is translucent, 0x400 or 0x600 perspective-corrected. The last
	// two arguments aren't read.
	void A3D_polygon_clip_XY_and_render(
		PANE* p_pane,
		MechU32 p_count,
		A3DVertex* p_vertices,
		MechU32 p_flags,
		MechS32 p_mip,
		A3DTextureRows* p_texture,
		undefined4 p_unk0x18,
		undefined4 p_unk0x1c
	);
	// Destroys the renderer and closes the toolkit (DdrawStop).
	void A3D_shutdown(void);

// The texture filter setting (MW2_TEXTURE_FILTER), passed to the renderer.
#define A3D_SET_TEXTURE_FILTER(p_filter) FUN_100102b0(p_filter)

// The 3D view's pane wipe, with a 16-bit pixel: VFX's (the 16-bit port, accel/vfx16.h).
#define A3D_PANE_WIPE(p_pane, p_color) VFX_pane_wipe(p_pane, p_color)
#endif

#ifdef MW2_POWERVR
	// The PowerVR edition's renderer, over VideoLogic's SGL (powervr/a3d.c): the game's objects are
	// SGL lists and meshes of a scene graph, which a view (a viewport, a camera and a list) renders.
	// The client code compiles the ATI edition's calls for it where its own aren't known yet.

	// A view of the scene: an SGL viewport and camera, the transform that places the camera, the
	// lists it renders, its lights, and the rendering qualities A3D_SetQuality sets.
	// SIZE 0x2c
	typedef struct A3DView {
		MechS32 m_viewport;      // 0x00
		MechS32 m_camera;        // 0x04
		MechS32 m_transform;     // 0x08 — the camera's
		MechS32 m_root;          // 0x0c — the list of the scene
		MechS32 m_list;          // 0x10 — the list the qualities are set in
		MechS32 m_light;         // 0x14 — the parallel light
		MechS32 m_ambientLight;  // 0x18
		MechS32 m_fog;           // 0x1c
		MechS32 m_texturing;     // 0x20
		MechS32 m_smoothShading; // 0x24
		MechS32 m_shadows;       // 0x28
	} A3DView;

	// The qualities of A3D_SetQuality.
	enum {
		c_a3dQualityFog = 1,
		c_a3dQualityTexturing = 2,
		c_a3dQualitySmoothShading = 3,
		c_a3dQualityShadows = 4
	};

	// The 3D view.
	extern A3DView g_a3dMainView;

	// Sets one of the view p_view's rendering qualities (sgl_qual_fog, sgl_qual_texturing,
	// sgl_qual_smooth_shading, sgl_qual_generate_shadows) to p_value; returns 0 for an unknown one.
	MechS32 A3D_SetQuality(A3DView* p_view, MechS32 p_quality, MechS32 p_value);

	// Opens SGL's display in the window p_window: DirectDraw mode, with the 2D drawing in the frame's
	// callback, the screen device at the game window's size and the views. Nonzero on success.
	MechS32 FUN_10060e60(HWND p_window, MechS32 p_width, MechS32 p_height);

	// The 2D drawing into the locked frame (simmain.c), which the renderer's frame callback calls.
	void FUN_10007db0(void* p_pixels, MechS32 p_pitch);

	// The next sgl_render recomputes the lights: FUN_10062b10 sets it at the start of each frame and
	// clears it after the first render, so FUN_10007db0 draws the 2D part once a frame.
	extern MechS32 g_a3dLightsChanged;

	// The renderer's entry points the other units call.

	// Moves the sky's plane (its height, size and tile), its texture scrolled by p_scroll.
	void FUN_1005efe0(MechFloat p_height, MechFloat p_size, MechFloat p_tile, MechFloat p_scroll);
	// Moves the ground's plane to the height p_height, its texture repeating every p_tile of p_size.
	void FUN_1005f0b0(MechFloat p_height, MechFloat p_size, MechFloat p_tile);
	// Creates a list in p_parent (a separate one for none); -1 before the renderer is open.
	MechS32 FUN_1005f200(MechS32 p_parent);
	// Deletes, empties, or adds an instance of p_instance to, the list p_list.
	void FUN_1005f280(MechS32 p_list);
	void FUN_1005f2a0(MechS32 p_list);
	void FUN_1005f2c0(MechS32 p_list, MechS32 p_instance);
	// Attaches the list p_list to p_parent (the root for none), detaches it, hides it, shows it.
	void FUN_1005f320(MechS32 p_list, MechS32 p_parent);
	void FUN_1005f390(MechS32 p_list);
	void FUN_1005f3f0(MechS32 p_list);
	void FUN_1005f450(MechS32 p_list);
	// Creates a cached texture of p_map  known to the texture callback by
	// p_id (an animation set, and its frame in the high word); -1 on failure.
	MechS32 FUN_1005f4b0(MechU32 p_id, struct sgl_intermediate_map* p_map);
	// Creates a material.
	MechS32 FUN_1005f570(void);
	// Translates (p_x, p_y, p_z) and rotates (degrees, in the order p_order) a transform, cleared
	// first when p_clear.
	void FUN_1005fb60(MechS32 p_transform, MechS32 p_clear, MechFloat p_x, MechFloat p_y, MechFloat p_z);
	void FUN_1005fbc0(
		MechS32 p_transform,
		MechS32 p_order,
		MechFloat p_x,
		MechFloat p_y,
		MechFloat p_z,
		MechS32 p_clear
	);
	// Sets a camera's zoom and its near and far distances (none for 0).
	void FUN_1005ff50(MechS32 p_camera, MechFloat p_zoom, MechFloat p_near, MechFloat p_far);
	// Turns the ground's material to its texture (p_textured) or its average color.
	void FUN_100602c0(MechS32 p_textured);
	// Colors the light (an effect's flash): materials, lights, fog and background.
	void FUN_100607d0(MechFloat p_red, MechFloat p_green, MechFloat p_blue);
	// Opens the renderer (the texture callback and the scene); nonzero on success.
	MechS32 FUN_10060f00(void);
	// Sets every animation's material to its texture, in the lighting mode p_mode (nonzero: dim
	// grey); p_colour is unused.
	void FUN_10061160(MechFloat* p_colour, MechS32 p_mode);
	// Converts the mission's scene to SGL ("Load3DObjects").
	void FUN_10061530(void);
	// Frees a converted shape's models but the one collision keeps.
	void FUN_10061750(struct Shape* p_shape);
	// Moves a player's mech (struct Player*) into the list of the detail level p_level.
	void FUN_100617f0(struct Player* p_player, MechS32 p_level);
	// An animation's SGL material, its texture loaded, lit when p_lit; -1 for none.
	MechS32 FUN_10061810(MechS32 p_index, MechS32* p_width, MechS32* p_height, MechS32 p_lit);
	// The size of a texture (an sgl_intermediate_map) by its type.
	MechS32 FUN_10061870(struct sgl_intermediate_map* p_map);
	// Sets a material's texture.
	void FUN_100618b0(MechS32 p_material, MechS32 p_texture);
	// Makes the object tree's materials glow by kind and damage (p_flags bit 0), or not.
	void FUN_10061c60(struct SceneObject* p_obj, MechU32 p_flags);
	// Converts a shape (p_material is unused).
	void FUN_10062160(struct Shape* p_shape, MechS32 p_material);
	// The shape is converted already.
	MechS32 FUN_10062960(struct Shape* p_shape);
	// Converts the object tree of p_obj, from its root, into p_parent (-1: by the shapes' kinds).
	void FUN_10062af0(struct SceneObject* p_obj, MechS32 p_parent);
	// Renders a frame.
	void FUN_10062b10(void);
	// Closes the renderer.
	void FUN_10062df0(void);

	// The fog's density.
	extern MechFloat g_a3dFogDensity;

	// Stand-ins a3d.c defines until their units do: an animation's average color, a frame's texture
	// bits (p_data gets its pixels), an animation's texture loaded (its
	// size), and two of the players' level loaders; the flag that lets the sky and the ground be lit.
	void FUN_10027620(MechS32 p_index, MechFloat* p_red, MechFloat* p_green, MechFloat* p_blue);
	struct sgl_intermediate_map* FUN_10027350(MechS32 p_set, MechS32 p_number, MechU16** p_data, MechS32* p_unk0x0c);
	MechS32 FUN_100273e0(MechS32 p_index, MechS32* p_width, MechS32* p_height);
	void FUN_100259c0(void);
	void FUN_100252a0(MechS32 p_player, MechS32 p_level);
	extern MechS32 g_unk0x101ad7e4;

	// polydraw.c's sky settings, which polydraw.h doesn't declare.
	extern MechFloat g_skyTileU;
	extern MechFloat g_groundTile;
	extern MechFloat g_skyScroll;
	extern MechFloat g_skyScrollSpeed;
	// render.c's sky object.
	extern struct SceneObject* g_skyObject;

	// A texture as A3D_polygon_clip_XY_and_render takes it (the ATI edition's A3DTextureRows).
	// SIZE 0xc
	typedef struct A3DTextureRows {
		MechU16** m_rows; // 0x0
		MechS32 m_width;  // 0x4
		MechS32 m_height; // 0x8
	} A3DTextureRows;

	void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, WINDOW* p_texture, MechU32 p_flags);
	void A3D_polygon_clip_XY_and_render(
		PANE* p_pane,
		MechU32 p_count,
		A3DVertex* p_vertices,
		MechU32 p_flags,
		MechS32 p_mip,
		A3DTextureRows* p_texture,
		undefined4 p_unk0x18,
		undefined4 p_unk0x1c
	);
	// Closes the renderer.
	void A3D_shutdown(void);

// The 3D view's pane wipe, with a 16-bit pixel: VFX's (the 16-bit port, accel/vfx16.h).
#define A3D_PANE_WIPE(p_pane, p_color) VFX_pane_wipe(p_pane, p_color)
#endif

#ifdef __cplusplus
}
#endif

#endif // ACCEL_A3D_H
