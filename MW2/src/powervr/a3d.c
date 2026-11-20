/* The PowerVR edition's A3D renderer (accel/a3d.h): the game's shapes kept as VideoLogic's SGL
   (sgl.dll) scene graph, which SGL renders itself. Its functions start after GetFilterTextures and
   run up to the import thunks. Distances are in tenths: the game's coordinates are scaled by 0.1 on
   their way into SGL.

   SGL draws in DirectDraw mode (sgl_use_ddraw_mode) on a screen device the game window's size, with
   a callback that keeps the DirectDraw object and gives the locked frame to the 2D drawing. There
   are three views (A3DView): the 3D view, and two cockpit panels' inset views, each a viewport, a
   camera on a transform, lights and the list it renders. The scene's root list holds a list per
   kind of shape (terrain, things, pieces, sprites, and the detail levels the players' mechs load),
   the sky and ground (two convex planes: the ground, and the sky with a translucent white haze
   plane under it that fog shows or hides), and a list of camera-facing sprites. A shape is
   converted once (FUN_10061530, "Load3DObjects"): each model is a list with a mesh of its vertices
   and faces, the faces in materials by their color's draw mode (flat, Gouraud, textured), several
   models become an SGL detail levels object, and each scene object a list with a translation and a
   rotation. The textures are SGL cached textures of the animation frames, which SGL's texture
   callback loads when a frame needs them and unloads, the least recently used first, when the
   memory is full. A frame (FUN_10062b10) moves the camera with the eyepoint, renders the cockpit
   panels' views and the 3D view, and scrolls the sky.

   SGL reports errors only when asked (sgl_get_errors): after each call that can fail, the renderer
   prints the last one. */
#include "accel/a3d.h"

#include "accel/a3dvertex.h"
#include "animation.h"
#include "animframe.h"
#include "cockpit.h"
#include "cockpitpanel.h"
#include "config.h"
#include "debugprint.h"
#include "decomp.h"
#include "eyepoint.h"
#include "face.h"
#include "gridobject.h"
#include "logwindow.h"
#include "mech.h"
#include "model.h"
#include "object.h"
#include "palette.h"
#include "palettecolor.h"
#include "pane.h"
#include "players.h"
#include "polydraw.h"
#include "rendersettings.h"
#include "shape.h"
#include "shapelists.h"
#include "simmain.h"
#include "types.h"
#include "vertex.h"
#include "window.h"

#include <ddraw.h>
#include <math.h>
#include <sgl.h>
#include <string.h>

// SGL's errors the renderer handles.
enum {
	c_sglErrorFace = -100,             // sgl_add_face couldn't add the face
	c_sglErrorTextureMemoryFull = -243 // sgl_load_cached_texture: no room left
};

// The animations the sky's and the ground's textures are.
enum {
	c_a3dGroundAnimation = 0x108,
	c_a3dSkyAnimation = 0x18c
};

// The orders FUN_1005fbc0 rotates in.
enum {
	c_a3dRotateYXZ = 0,
	c_a3dRotateXYZ = 1,
	c_a3dRotateXZY = 2,
	c_a3dRotateZXY = 4,
	c_a3dRotateZYX = 5,
	c_a3dRotateYZX = 6
};

// What the renderer finds at a cockpit panel's 0x30 to 0x94 in the PowerVR edition: a panel that
// shows a 3D view asks for a render, and is called around it.
// SIZE 0x14
typedef struct A3DPanelRect {
	undefined4 m_unk0x00; // 0x00
	MechS32 m_left;       // 0x04
	MechS32 m_top;        // 0x08
	MechS32 m_right;      // 0x0c
	MechS32 m_bottom;     // 0x10
} A3DPanelRect;

typedef struct A3DCockpitPanel A3DCockpitPanel;

// SIZE 0x98
struct A3DCockpitPanel {
	undefined m_unk0x00[0x30];                                         // 0x00
	MechS32 m_redraw;                                                  // 0x30 — its view is to be rendered
	A3DView* m_view;                                                   // 0x34
	A3DPanelRect* m_rect;                                              // 0x38
	undefined m_unk0x3c[0x64 - 0x3c];                                  // 0x3c
	void (*m_setRect)(A3DCockpitPanel* p_panel, A3DPanelRect* p_rect); // 0x64
	undefined m_unk0x68[0x90 - 0x68];                                  // 0x68
	void (*m_beforeRender)(A3DCockpitPanel* p_panel);                  // 0x90
	void (*m_afterRender)(A3DCockpitPanel* p_panel);                   // 0x94
};

#define A3D_PANEL(p_index) ((A3DCockpitPanel*) g_cockpitPanels[p_index])

// The vertex's flags the renderer keeps in Vertex's padding.
#define A3D_VERTEX_UV_SCALED(p_vertex) ((p_vertex)->m_unk0x29[0]) // u and v were divided by the texture's size
#define A3D_VERTEX_LIT(p_vertex) ((p_vertex)->m_unk0x29[1])       // lit through its normal

// The scaling into SGL's units.
#define A3D_SCALE 0.1f

// Degrees to radians.
#define A3D_DEGREES (3.14159f / 180.0f)

// GLOBAL: MW2POWERVR 0x101c4d70
static sgl_vector g_a3dAxisX = {1.0f, 0.0f, 0.0f};

// GLOBAL: MW2POWERVR 0x101c4d80
static sgl_vector g_a3dAxisY = {0.0f, 1.0f, 0.0f};

// GLOBAL: MW2POWERVR 0x101c4d90
static sgl_vector g_a3dAxisZ = {0.0f, 0.0f, 1.0f};

// The renderer is open (FUN_10060f00).
// GLOBAL: MW2POWERVR 0x101c4d9c
static MechS32 g_a3dInitialized = FALSE;

// GLOBAL: MW2POWERVR 0x101c4db0
static MechS32 g_a3dDevice = -1;

// The sky: a list of the sky's plane, and the haze plane's under it.
// GLOBAL: MW2POWERVR 0x101c4db4
static MechS32 g_a3dSkyList = -1;

// GLOBAL: MW2POWERVR 0x101c4db8
static MechS32 g_a3dSkyConvex = -1;

// The list that turns the fog off for the sky.
// GLOBAL: MW2POWERVR 0x101c4dbc
static MechS32 g_a3dSkyFogList = -1;

// The haze plane's list, hidden while there is no fog.
// GLOBAL: MW2POWERVR 0x101c4dc0
static MechS32 g_a3dHazeList = -1;

// GLOBAL: MW2POWERVR 0x101c4dc4
static MechS32 g_a3dHazeMaterial = -1;

// GLOBAL: MW2POWERVR 0x101c4dc8
static MechS32 g_a3dGroundConvex = -1;

// The DirectDraw object SGL draws with, and the game's window (FUN_1005e880).
// GLOBAL: MW2POWERVR 0x101c4dcc
static LPDIRECTDRAW g_a3dDirectDraw = NULL;

// GLOBAL: MW2POWERVR 0x101c4dd0
static HWND g_a3dWindow = NULL;

// GLOBAL: MW2POWERVR 0x101c4dd8
static sgl_colour g_a3dWhite = {1.0f, 1.0f, 1.0f};

// GLOBAL: MW2POWERVR 0x101c4de8
static sgl_colour g_a3dBlack = {0.0f, 0.0f, 0.0f};

// The haze plane's texture: 32 by 32 white texels.
// GLOBAL: MW2POWERVR 0x101c4e04
static MechS32 g_a3dWhiteTexture = -1;

// GLOBAL: MW2POWERVR 0x1007cac0
static MechU32 g_a3dWhiteTexels[0x400];

// GLOBAL: MW2POWERVR 0x101c4e08
static sgl_intermediate_map g_a3dWhiteMap = {{'I', 'M', 'A', 'P'}, 0x20, 0x20, g_a3dWhiteTexels};

// The faces added, and the errors adding them.
// GLOBAL: MW2POWERVR 0x101c4e18
static MechS32 g_a3dFaceCount = 0;

// GLOBAL: MW2POWERVR 0x101c4e1c
static MechS32 g_a3dFaceErrors = 0;

// The glows of FUN_10061c60's infrared colors.
// GLOBAL: MW2POWERVR 0x101c51a8
static sgl_colour g_a3dGlowHot = {0.7f, 0.2f, 0.2f};

// GLOBAL: MW2POWERVR 0x101c51b8
static sgl_colour g_a3dGlowRed = {0.5f, 0.0f, 0.0f};

// GLOBAL: MW2POWERVR 0x101c51d8
static sgl_colour g_a3dGlowCold = {0.0f, 0.0f, 0.7f};

// GLOBAL: MW2POWERVR 0x101c51e8
static sgl_colour g_a3dGlowCool = {0.0f, 0.0f, 0.5f};

// GLOBAL: MW2POWERVR 0x101c51f8
static sgl_colour g_a3dGlowWarm = {0.6f, 0.5f, 0.0f};

// GLOBAL: MW2POWERVR 0x101c5218
static sgl_colour g_a3dGrey = {0.5f, 0.5f, 0.5f};

// The viewport of the whole screen, the 3D view's with panel 13's inset taken out.
// GLOBAL: MW2POWERVR 0x101c5234
static MechS32 g_a3dScreenViewport = -1;

// The camera-facing sprites' list and its rotation.
// GLOBAL: MW2POWERVR 0x101c5238
static MechS32 g_a3dFacingList = -1;

// GLOBAL: MW2POWERVR 0x101c523c
static MechS32 g_a3dFacingTransform = -1;

// The light's color (FUN_100607d0).
// GLOBAL: MW2POWERVR 0x101c5240
static sgl_colour g_a3dLightColour = {1.0f, 1.0f, 1.0f};

// The fog's density.
// GLOBAL: MW2POWERVR 0x101c524c
MechFloat g_a3dFogDensity = 0.0f;

// The next sgl_render recomputes the lights: set at the start of each frame (FUN_10062b10) and
// cleared by the frame's first render.
// GLOBAL: MW2POWERVR 0x101c5250
MechS32 g_a3dLightsChanged = 1;

// The frames rendered: when each texture was last used.
// GLOBAL: MW2POWERVR 0x101c5254
static MechS32 g_a3dFrame = 0;

// FUN_10060f30's lighting mode for every material.
// GLOBAL: MW2POWERVR 0x101c5258
static MechS32 g_a3dLightingMode = 0;

// The conversion prints what it makes.
// GLOBAL: MW2POWERVR 0x101c5264
static MechS32 g_a3dVerbose = 0;

// The objects and polygons converted, by kind.
// GLOBAL: MW2POWERVR 0x101c5268
static MechS32 g_a3dTerrainObjects = 0;

// GLOBAL: MW2POWERVR 0x101c526c
static MechS32 g_a3dTerrainPolys = 0;

// GLOBAL: MW2POWERVR 0x101c5270
static MechS32 g_a3dThingObjects = 0;

// GLOBAL: MW2POWERVR 0x101c5274
static MechS32 g_a3dThingPolys = 0;

// GLOBAL: MW2POWERVR 0x101c5278
static MechS32 g_a3dPieceObjects = 0;

// GLOBAL: MW2POWERVR 0x101c527c
static MechS32 g_a3dPiecePolys = 0;

// The list of the sky and the ground.
// GLOBAL: MW2POWERVR 0x10070e40
static MechS32 g_a3dSkyGroundList;

// The list of the shapes of kind 0x400.
// GLOBAL: MW2POWERVR 0x10070e44
static MechS32 g_a3dKind400List;

// Cockpit panel 13's view, lit from below.
// GLOBAL: MW2POWERVR 0x10070e50
static A3DView g_a3dPanel13View;

// The cached textures SGL's texture callback is given.
// GLOBAL: MW2POWERVR 0x10070e80
static sgl_texture_cache_entry g_a3dCacheEntries[2000];

// The lists by kind (FUN_10061910) and by detail level (FUN_100617b0).
// GLOBAL: MW2POWERVR 0x1007ca00
static MechS32 g_a3dThingList;

// The background's color.
// GLOBAL: MW2POWERVR 0x1007ca10
static sgl_colour g_a3dBackground;

// GLOBAL: MW2POWERVR 0x1007ca1c
static MechS32 g_a3dLevel3List;

// GLOBAL: MW2POWERVR 0x1007ca20
static MechS32 g_a3dLevel1List;

// GLOBAL: MW2POWERVR 0x1007ca24
static MechS32 g_a3dLevel4List;

// The 3D view.
// GLOBAL: MW2POWERVR 0x1007ca30
A3DView g_a3dMainView;

// GLOBAL: MW2POWERVR 0x1007ca5c
static MechS32 g_a3dLevel0List;

// GLOBAL: MW2POWERVR 0x1007ca60
static MechS32 g_a3dSpriteList;

// The fog's color.
// GLOBAL: MW2POWERVR 0x1007ca70
static sgl_colour g_a3dFogColour;

// GLOBAL: MW2POWERVR 0x1007ca7c
static MechS32 g_a3dTerrainList;

// Cockpit panel 2's view.
// GLOBAL: MW2POWERVR 0x1007ca80
static A3DView g_a3dPanel2View;

// The vertex, light direction or color being passed.
// GLOBAL: MW2POWERVR 0x1007cab0
static sgl_vector g_a3dVertex;

MechS32 __cdecl FUN_10060000(sgl_ddraw_frame* p_frame);
void __cdecl FUN_10061360(sgl_texture_cache_entry* p_entries, MechS32 p_count, MechU32 p_freeMemory);
MechS32 FUN_1005f200(MechS32 p_parent);
void FUN_1005ef90(MechS32 p_fog);
void FUN_1005f3f0(MechS32 p_list);
void FUN_1005f450(MechS32 p_list);
void FUN_1005f7f0(MechS32 p_count, MechS32* p_ids);
void FUN_10060fd0(MechS32 p_index, MechFloat* p_colour);
void FUN_10061050(MechS32 p_set);
void FUN_100610d0(MechS32 p_index);
MechS32 FUN_100617b0(MechS32 p_level);
MechS32 FUN_10061810(MechS32 p_index, MechS32* p_width, MechS32* p_height, MechS32 p_lit);
void FUN_100618b0(MechS32 p_material, MechS32 p_texture);
void FUN_10062160(Shape* p_shape, MechS32 p_material);
MechS32 FUN_10062960(Shape* p_shape);
void FUN_10062af0(SceneObject* p_obj, MechS32 p_parent);

// Prints SGL's first and last errors.
// FUNCTION: MW2POWERVR 0x1005e820
void FUN_1005e820(void)
{
	MechS32 first = 0;
	MechS32 last = 0;

	sgl_get_errors(&first, &last);
	if (first) {
		DebugPrintInternal("***PowerVR SGL first error %d\n", first);
	}
	if (last) {
		DebugPrintInternal("***PowerVR SGL last error %d\n", last);
	}
}

// Puts SGL in DirectDraw mode in the window p_window, its frames to FUN_10060000.
// FUNCTION: MW2POWERVR 0x1005e880
MechS32 FUN_1005e880(HWND p_window, MechS32 p_width, MechS32 p_height)
{
	MechS32 result;

	result = sgl_use_ddraw_mode(p_window, FUN_10060000);
	if (result) {
		DebugPrintInternal("sgl_use_ddraw_mode: %d", result);
		return FALSE;
	}

	g_a3dWindow = p_window;
	return TRUE;
}

// Creates the screen device.
// FUNCTION: MW2POWERVR 0x1005e8c0
MechS32 FUN_1005e8c0(MechS32 p_width, MechS32 p_height)
{
	MechS32 first;
	MechS32 last;
	MechS32 number;
	MechS32 width;
	MechS32 height;
	MechS32 mode;
	sgl_bool doubleBuffer;

	g_a3dDevice = sgl_create_screen_device(0, p_width - 1, p_height - 1, 0, TRUE);
	sgl_get_errors(&first, &last);
	if (last) {
		DebugPrintInternal("sgl_create_screen_device last error %d\n", last);
	}

	sgl_get_device(g_a3dDevice, &number, &width, &height, &mode, &doubleBuffer);
	DebugPrintInternal("device: %d (%d,%d), mode=%d, dbl_buf=%d\n", number, width, height, mode, doubleBuffer);
	return g_a3dDevice >= 0;
}

// Creates a viewport of p_width by p_height at p_x, p_y, its camera rectangle square, and returns
// the rectangle SGL gave it.
// FUNCTION: MW2POWERVR 0x1005e970
MechS32 FUN_1005e970(
	MechS32 p_x,
	MechS32 p_y,
	MechS32 p_width,
	MechS32 p_height,
	MechS32* p_left,
	MechS32* p_top,
	MechS32* p_right,
	MechS32* p_bottom
)
{
	MechS32 viewport;
	MechS32 first;
	MechS32 last;
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;
	MechFloat camLeft;
	MechFloat camTop;
	MechFloat camRight;
	MechFloat camBottom;
	MechS32 width;
	MechS32 height;

	viewport = sgl_create_viewport(
		g_a3dDevice,
		p_x,
		p_y,
		p_x + p_width + 1,
		p_y + p_height - 1,
		(MechFloat) p_x,
		(MechFloat) p_y,
		(MechFloat) (p_x + p_width - 1),
		(MechFloat) (p_y + p_height - 1)
	);
	sgl_get_errors(&first, &last);
	if (last) {
		DebugPrintInternal("sgl_create_viewport last error %d\n", last);
	}

	sgl_get_viewport(viewport, &left, &top, &right, &bottom, &camLeft, &camTop, &camRight, &camBottom);
	width = right - left + 1;
	height = bottom - top + 1;
	if (height < width) {
		camLeft = (MechFloat) (left + (width - height) / 2);
		camTop = (MechFloat) top;
		camRight = (MechFloat) (left + width + (height - width) / 2 - 1);
		camBottom = (MechFloat) (top + height - 1);
	}
	else {
		camLeft = (MechFloat) left;
		camTop = (MechFloat) top;
		camRight = (MechFloat) (left + width - 1);
		camBottom = (MechFloat) (top + height - 1);
	}

	sgl_set_viewport(viewport, left, top, right, bottom, camLeft, camTop, camRight, camBottom);
	sgl_get_errors(&first, &last);
	if (last) {
		DebugPrintInternal("sgl_create_viewport last error %d\n", last);
	}

	sgl_get_viewport(viewport, &left, &top, &right, &bottom, &camLeft, &camTop, &camRight, &camBottom);
	*p_left = left;
	*p_top = top;
	*p_right = right;
	*p_bottom = bottom;
	FUN_1005e820();

	return viewport;
}

// Creates a camera in the list p_list, on a transform of its own, with the background p_background,
// and the near and far distances p_near and p_far (none for 0).
// FUNCTION: MW2POWERVR 0x1005eba0
MechS32 FUN_1005eba0(MechS32 p_list, MechFloat* p_background, MechFloat p_near, MechFloat p_far, MechS32* p_transform)
{
	MechS32 transform;
	MechS32 camera;

	if (p_list < 0) {
		return -1;
	}

	sgl_modify_list(p_list, FALSE);
	FUN_1005e820();
	sgl_create_list(FALSE, TRUE, FALSE);
	transform = sgl_create_transform(TRUE);
	camera = sgl_create_camera(1.3333f, p_near * A3D_SCALE, p_far == 0.0f ? 0.0f : 1.0f / (p_far * A3D_SCALE));
	FUN_1005e820();
	if (camera < 0) {
		sgl_delete_list(p_list);
		return -1;
	}

	sgl_set_background_colour(camera, p_background);
	sgl_to_parent();
	FUN_1005e820();
	if (p_transform) {
		*p_transform = transform;
	}

	return camera;
}

// Creates the ground in the list p_list: a plane at the height p_height, its texture (the material
// p_material) repeating every p_tile of p_size.
// FUNCTION: MW2POWERVR 0x1005ec90
void FUN_1005ec90(MechS32 p_list, MechS32 p_material, MechFloat p_height, MechFloat p_size, MechFloat p_tile)
{
	sgl_2d_vec uv1;
	sgl_2d_vec uv2;
	sgl_2d_vec uv3;
	sgl_vector point1;
	sgl_vector point2;
	sgl_vector point3;
	MechFloat height;
	MechFloat size;

	sgl_modify_list(p_list, FALSE);
	if (p_material < 0) {
		return;
	}

	sgl_create_list(FALSE, TRUE, FALSE);
	height = p_height * A3D_SCALE;
	size = p_size / p_tile * A3D_SCALE;
	point1[0] = 0.0f;
	point1[1] = height;
	point1[2] = 0.0f;
	point2[0] = size;
	point2[1] = height;
	point2[2] = 0.0f;
	point3[0] = 0.0f;
	point3[1] = height;
	point3[2] = size;
	uv1[0] = 0.0f;
	uv1[1] = 0.0f;
	uv2[0] = 1.0f;
	uv2[1] = 0.0f;
	uv3[0] = 0.0f;
	uv3[1] = 1.0f;

	g_a3dGroundConvex = sgl_create_convex(TRUE);
	if (p_material >= 0) {
		sgl_use_material_instance(p_material, TRUE);
	}
	FUN_1005e820();
	sgl_add_plane(point1, point2, point3, FALSE, NULL, NULL, NULL, uv1, uv2, uv3);
	FUN_1005e820();
	sgl_to_parent();
}

// Creates the sky in the list p_list: a plane at the height p_height, its texture (the material
// p_material) repeating every p_tile of p_size, and under it, at p_hazeHeight, a translucent white
// plane that the fog shows.
// FUNCTION: MW2POWERVR 0x1005ed90
void FUN_1005ed90(
	MechS32 p_list,
	MechS32 p_material,
	MechFloat p_height,
	MechFloat p_size,
	MechFloat p_tile,
	MechFloat p_hazeHeight,
	MechFloat p_fogDensity
)
{
	sgl_2d_vec uv1;
	sgl_2d_vec uv2;
	sgl_2d_vec uv3;
	sgl_vector point1;
	sgl_vector point2;
	sgl_vector point3;
	MechFloat height;
	MechFloat size;

	height = p_height * A3D_SCALE;
	size = p_size / p_tile * A3D_SCALE;
	point1[0] = 0.0f;
	point1[1] = height;
	point1[2] = size;
	point2[0] = size;
	point2[1] = height;
	point2[2] = 0.0f;
	point3[0] = 0.0f;
	point3[1] = height;
	point3[2] = 0.0f;
	uv1[0] = 0.0f;
	uv1[1] = 1.0f;
	uv2[0] = 1.0f;
	uv2[1] = 0.0f;
	uv3[0] = 0.0f;
	uv3[1] = 0.0f;

	sgl_modify_list(p_list, FALSE);
	if (p_material >= 0) {
		g_a3dSkyList = sgl_create_list(TRUE, TRUE, FALSE);
		g_a3dSkyFogList = sgl_create_list(TRUE, FALSE, FALSE);
		sgl_qual_fog(FALSE);
		sgl_to_parent();
		g_a3dSkyConvex = sgl_create_convex(TRUE);
		sgl_use_material_instance(p_material, TRUE);
		FUN_1005e820();
		sgl_add_plane(point1, point2, point3, FALSE, NULL, NULL, NULL, uv1, uv2, uv3);
		FUN_1005e820();

		g_a3dHazeList = FUN_1005f200(g_a3dSkyList);
		sgl_qual_fog(TRUE);
		sgl_new_translucent();
		if (g_a3dWhiteTexture == -1) {
			memset(g_a3dWhiteTexels, 0xff, sizeof(g_a3dWhiteTexels));
			g_a3dWhiteTexture = sgl_create_texture(0, 0, FALSE, FALSE, &g_a3dWhiteMap, NULL);
			FUN_1005e820();
		}

		g_a3dHazeMaterial = sgl_create_material(TRUE, TRUE);
		sgl_set_texture_map(g_a3dWhiteTexture, FALSE, FALSE);
		sgl_set_opacity(0.4f);
		height = p_hazeHeight * A3D_SCALE;
		point1[1] = height;
		point2[1] = height;
		point3[1] = height;
		sgl_add_plane(point1, point2, point3, FALSE, NULL, NULL, NULL, uv1, uv2, uv3);
		FUN_1005e820();
	}
	else {
		g_a3dSkyList = -1;
	}

	FUN_1005ef90(FALSE);
}

// Shows the haze plane (p_fog), with the fog off for the sky, or hides it.
// FUNCTION: MW2POWERVR 0x1005ef90
void FUN_1005ef90(MechS32 p_fog)
{
	if (g_a3dSkyList == -1 || g_a3dHazeList == -1) {
		return;
	}

	if (p_fog) {
		sgl_modify_list(g_a3dSkyFogList, TRUE);
		sgl_qual_fog(FALSE);
		FUN_1005f450(g_a3dHazeList);
	}
	else {
		FUN_1005f3f0(g_a3dHazeList);
	}
}

// Moves the sky's plane: FUN_1005ed90's, its texture scrolled by p_scroll.
// FUNCTION: MW2POWERVR 0x1005efe0
void FUN_1005efe0(MechFloat p_height, MechFloat p_size, MechFloat p_tile, MechFloat p_scroll)
{
	sgl_2d_vec uv1;
	sgl_2d_vec uv2;
	sgl_2d_vec uv3;
	sgl_vector point1;
	sgl_vector point2;
	sgl_vector point3;
	MechFloat height;
	MechFloat size;

	height = p_height * A3D_SCALE;
	size = p_size / p_tile * A3D_SCALE;
	point1[0] = 0.0f;
	point1[1] = height;
	point1[2] = size;
	point2[0] = size;
	point2[1] = height;
	point2[2] = 0.0f;
	point3[0] = 0.0f;
	point3[1] = height;
	point3[2] = 0.0f;
	uv1[0] = p_scroll;
	uv1[1] = p_scroll + 1.0f;
	uv2[0] = p_scroll + 1.0f;
	uv2[1] = p_scroll;
	uv3[0] = p_scroll;
	uv3[1] = p_scroll;

	sgl_modify_convex(g_a3dSkyConvex, FALSE);
	sgl_set_plane(0, point1, point2, point3, FALSE, NULL, NULL, NULL, uv1, uv2, uv3);
	FUN_1005e820();
}

// Moves the ground's plane: to the height p_height, its texture repeating every p_tile of p_size.
// FUNCTION: MW2POWERVR 0x1005f0b0
void FUN_1005f0b0(MechFloat p_height, MechFloat p_size, MechFloat p_tile)
{
	sgl_2d_vec uv1;
	sgl_2d_vec uv2;
	sgl_2d_vec uv3;
	sgl_vector point1;
	sgl_vector point2;
	sgl_vector point3;
	MechFloat height;
	MechFloat size;

	height = p_height * A3D_SCALE;
	size = p_size / p_tile * A3D_SCALE;
	point1[0] = 0.0f;
	point1[1] = height;
	point1[2] = size;
	point2[0] = size;
	point2[1] = height;
	point2[2] = 0.0f;
	point3[0] = 0.0f;
	point3[1] = height;
	point3[2] = 0.0f;
	uv1[0] = 0.0f;
	uv1[1] = 1.0f;
	uv2[0] = 1.0f;
	uv2[1] = 0.0f;
	uv3[0] = 0.0f;
	uv3[1] = 0.0f;

	sgl_modify_convex(g_a3dGroundConvex, FALSE);
	sgl_set_plane(0, point1, point2, point3, FALSE, NULL, NULL, NULL, uv1, uv2, uv3);
	FUN_1005e820();
}

// Closes SGL: the sky, the haze's texture and the scene, DirectDraw's cooperative level back to
// normal, and the device.
// FUNCTION: MW2POWERVR 0x1005f180
void FUN_1005f180(void)
{
	if (!g_a3dInitialized) {
		return;
	}

	if (g_a3dSkyList > 0) {
		sgl_delete_list(g_a3dSkyList);
	}
	g_a3dSkyList = -1;
	if (g_a3dWhiteTexture > 0) {
		sgl_delete_texture(g_a3dWhiteTexture);
	}
	g_a3dWhiteTexture = -1;

	sgl_modify_list(-1, TRUE);
	if (g_a3dDirectDraw) {
		IDirectDraw_SetCooperativeLevel(g_a3dDirectDraw, g_a3dWindow, DDSCL_NORMAL);
	}
	sgl_delete_device(g_a3dDevice);
	g_a3dInitialized = FALSE;
}

// Creates a list in p_parent, or a separate one when there is none.
// FUNCTION: MW2POWERVR 0x1005f200
MechS32 FUN_1005f200(MechS32 p_parent)
{
	MechS32 first = 0;
	MechS32 last = 0;
	MechS32 list;

	if (!g_a3dInitialized) {
		return -1;
	}

	if (p_parent > 0) {
		sgl_modify_list(p_parent, FALSE);
		list = sgl_create_list(TRUE, TRUE, FALSE);
	}
	else {
		list = sgl_create_list(TRUE, TRUE, TRUE);
	}

	sgl_get_errors(&first, &last);
	if (last) {
		DebugPrintInternal("sgl_create_list last error %d\n", last);
	}

	return list;
}

// Deletes the list p_list.
// FUNCTION: MW2POWERVR 0x1005f280
void FUN_1005f280(MechS32 p_list)
{
	if (g_a3dInitialized && p_list > 0) {
		sgl_delete_list(p_list);
	}
}

// Empties the list p_list.
// FUNCTION: MW2POWERVR 0x1005f2a0
void FUN_1005f2a0(MechS32 p_list)
{
	if (g_a3dInitialized && p_list > 0) {
		sgl_modify_list(p_list, TRUE);
	}
}

// Adds an instance of the list p_instance to the list p_list.
// FUNCTION: MW2POWERVR 0x1005f2c0
void FUN_1005f2c0(MechS32 p_list, MechS32 p_instance)
{
	MechS32 first;
	MechS32 last;

	if (g_a3dInitialized && p_list > 0 && p_instance > 0) {
		sgl_modify_list(p_list, FALSE);
		sgl_use_instance(p_instance);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_use_instance last error %d\n", last);
		}
	}
}

// Attaches the list p_list to p_parent, or to the root when there is none.
// FUNCTION: MW2POWERVR 0x1005f320
void FUN_1005f320(MechS32 p_list, MechS32 p_parent)
{
	MechS32 first = 0;
	MechS32 last = 0;

	if (g_a3dInitialized && p_list > 0) {
		if (p_parent <= 0) {
			sgl_modify_list(-1, FALSE);
		}
		else {
			sgl_modify_list(p_parent, FALSE);
		}
		FUN_1005e820();
		sgl_attach_list(p_list);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_attach_list last error %d\n", last);
		}
	}
}

// Detaches the list p_list from its parent.
// FUNCTION: MW2POWERVR 0x1005f390
void FUN_1005f390(MechS32 p_list)
{
	MechS32 first = 0;
	MechS32 last = 0;

	if (g_a3dInitialized && p_list > 0) {
		sgl_detach_list(p_list);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_detach_list last error %d\n", last);
		}
	}
}

// Hides the list p_list.
// FUNCTION: MW2POWERVR 0x1005f3f0
void FUN_1005f3f0(MechS32 p_list)
{
	MechS32 first = 0;
	MechS32 last = 0;

	if (g_a3dInitialized && p_list > 0) {
		sgl_set_ignore_list(p_list, TRUE);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_set_ignore_list (hide) last error %d\n", last);
		}
	}
}

// Shows the list p_list.
// FUNCTION: MW2POWERVR 0x1005f450
void FUN_1005f450(MechS32 p_list)
{
	MechS32 first = 0;
	MechS32 last = 0;

	if (g_a3dInitialized && p_list > 0) {
		sgl_set_ignore_list(p_list, FALSE);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_set_ignore_list (unhide) last error %d\n", last);
		}
	}
}

// Creates a cached texture of the pixels p_map, which the texture callback knows by p_id.
// FUNCTION: MW2POWERVR 0x1005f4b0
MechS32 FUN_1005f4b0(MechU32 p_id, sgl_intermediate_map* p_map)
{
	MechS32 first;
	MechS32 last;
	MechS32 size;
	MechS32 texture;

	if (p_map) {
		size = sgl_texture_size(p_map);
		if (size > 0) {
			p_map->pixels = p_map + 1;
			texture = sgl_create_cached_texture(p_id);
			if (texture < 0) {
				texture = -1;
			}
			sgl_get_errors(&first, &last);
			if (last) {
				DebugPrintInternal("sgl_create_cached_texture last error %d\n", last);
			}

			return texture;
		}

		DebugPrintInternal("sgl_texture_size() error: %d \n", size);
		DebugPrintInternal("type %c%c%d%d \n", p_map->id[0], p_map->id[1], p_map->id[2], p_map->id[3]);
	}

	return -1;
}

// Deletes the texture p_texture.
// FUNCTION: MW2POWERVR 0x1005f560
void FUN_1005f560(MechS32 p_texture)
{
	if (p_texture >= 0) {
		sgl_delete_texture(p_texture);
	}
}

// Creates a material.
// FUNCTION: MW2POWERVR 0x1005f570
MechS32 FUN_1005f570(void)
{
	MechS32 first;
	MechS32 last;
	MechS32 material;

	material = sgl_create_material(TRUE, FALSE);
	sgl_get_errors(&first, &last);
	if (last) {
		DebugPrintInternal("sgl_create_material last error %d\n", last);
	}

	return material;
}

// Starts a mesh in the list p_list, its faces drawn from both sides.
// FUNCTION: MW2POWERVR 0x1005f5c0
void FUN_1005f5c0(MechS32 p_list)
{
	if (g_a3dInitialized && p_list > 0) {
		sgl_modify_list(p_list, FALSE);
		FUN_1005e820();
		sgl_create_mesh(FALSE);
		FUN_1005e820();
		sgl_set_cull_mode(2);
		FUN_1005e820();
	}
}

// Adds a vertex to the mesh, with a normal and a texture coordinate.
// FUNCTION: MW2POWERVR 0x1005f600
void FUN_1005f600(
	MechFloat p_x,
	MechFloat p_y,
	MechFloat p_z,
	MechFloat p_normalX,
	MechFloat p_normalY,
	MechFloat p_normalZ,
	MechFloat p_u,
	MechFloat p_v
)
{
	sgl_2d_vec uv;
	sgl_vector normal;

	if (g_a3dInitialized) {
		g_a3dVertex[0] = p_x * A3D_SCALE;
		g_a3dVertex[1] = p_y * A3D_SCALE;
		g_a3dVertex[2] = p_z * A3D_SCALE;
		normal[0] = -p_normalX;
		normal[1] = -p_normalY;
		normal[2] = -p_normalZ;
		uv[0] = p_u;
		uv[1] = p_v;
		sgl_add_vertices(1, &g_a3dVertex, &normal, &uv);
		FUN_1005e820();
	}
}

// Adds a vertex to the mesh, with a texture coordinate.
// FUNCTION: MW2POWERVR 0x1005f6a0
void FUN_1005f6a0(MechFloat p_x, MechFloat p_y, MechFloat p_z, MechFloat p_u, MechFloat p_v)
{
	sgl_2d_vec uv;

	if (g_a3dInitialized) {
		g_a3dVertex[0] = p_x * A3D_SCALE;
		g_a3dVertex[1] = p_y * A3D_SCALE;
		g_a3dVertex[2] = p_z * A3D_SCALE;
		uv[0] = p_u;
		uv[1] = p_v;
		sgl_add_vertices(1, &g_a3dVertex, NULL, &uv);
		FUN_1005e820();
	}
}

// Adds a vertex to the mesh, with a normal.
// FUNCTION: MW2POWERVR 0x1005f710
void FUN_1005f710(
	MechFloat p_x,
	MechFloat p_y,
	MechFloat p_z,
	MechFloat p_normalX,
	MechFloat p_normalY,
	MechFloat p_normalZ
)
{
	sgl_vector normal;

	if (g_a3dInitialized) {
		g_a3dVertex[0] = p_x * A3D_SCALE;
		g_a3dVertex[1] = p_y * A3D_SCALE;
		g_a3dVertex[2] = p_z * A3D_SCALE;
		normal[0] = -p_normalX;
		normal[1] = -p_normalY;
		normal[2] = -p_normalZ;
		sgl_add_vertices(1, &g_a3dVertex, &normal, NULL);
		FUN_1005e820();
	}
}

// Adds a vertex to the mesh.
// FUNCTION: MW2POWERVR 0x1005f790
void FUN_1005f790(MechFloat p_x, MechFloat p_y, MechFloat p_z)
{
	if (g_a3dInitialized) {
		g_a3dVertex[0] = p_x * A3D_SCALE;
		g_a3dVertex[1] = p_y * A3D_SCALE;
		g_a3dVertex[2] = p_z * A3D_SCALE;
		sgl_add_vertices(1, &g_a3dVertex, NULL, NULL);
		FUN_1005e820();
	}
}

// Adds the face of the p_count vertices p_ids to the mesh, as a fan of faces of up to four (SGL's
// limit): each takes the first vertex and the last one of the one before. A face SGL refuses is
// deleted again.
// FUNCTION: MW2POWERVR 0x1005f7f0
void FUN_1005f7f0(MechS32 p_count, MechS32* p_ids)
{
	MechS32 first;
	MechS32 last;
	MechS32 count;
	MechS32 i;
	MechS32 j;

	count = p_count >= 4 ? 4 : p_count;
	while (count) {
		sgl_add_face(count, p_ids);
		g_a3dFaceCount++;
		sgl_get_errors(&first, &last);
		if (last) {
			g_a3dFaceErrors++;
			if (last == c_sglErrorFace) {
				DebugPrintInternal("add face last error=%d, deleting\n", last);
				sgl_delete_face(sgl_num_faces() - 1);
			}
		}

		if (p_count <= count) {
			count = 0;
		}
		else {
			j = 1;
			for (i = count - 1; i < p_count; i++) {
				p_ids[j++] = p_ids[i];
			}
			p_count = j;
			count = p_count >= 4 ? 4 : p_count;
		}
	}
}

// Adds a textured face, in the material p_material.
// FUNCTION: MW2POWERVR 0x1005f8c0
void FUN_1005f8c0(MechS32 p_count, MechS32* p_ids, MechS32 p_material)
{
	MechS32 first;
	MechS32 last;

	if (g_a3dInitialized && p_count > 2) {
		sgl_use_material_instance(p_material, TRUE);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("textured poly material last error %d\n", last);
		}
		FUN_1005f7f0(p_count, p_ids);
	}
}

// Adds a Gouraud-shaded face, lit in the color p_red, p_green, p_blue.
// FUNCTION: MW2POWERVR 0x1005f920
void FUN_1005f920(MechS32 p_count, MechS32* p_ids, MechFloat p_red, MechFloat p_green, MechFloat p_blue)
{
	MechS32 first;
	MechS32 last;
	sgl_colour colour;

	if (g_a3dInitialized && p_count > 2) {
		sgl_create_material(FALSE, TRUE);
		colour[0] = p_red;
		colour[1] = p_green;
		colour[2] = p_blue;
		sgl_set_ambient(colour);
		sgl_set_diffuse(colour);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("gouraud poly material last error %d\n", last);
		}
		FUN_1005f7f0(p_count, p_ids);
	}
}

// Adds a flat face of the color p_red, p_green, p_blue, which glows it rather than lights it.
// FUNCTION: MW2POWERVR 0x1005f9b0
void FUN_1005f9b0(MechS32 p_count, MechS32* p_ids, MechFloat p_red, MechFloat p_green, MechFloat p_blue)
{
	MechS32 first;
	MechS32 last;
	sgl_colour colour;

	if (g_a3dInitialized && p_count > 2) {
		sgl_create_material(FALSE, TRUE);
		colour[0] = p_red;
		colour[1] = p_green;
		colour[2] = p_blue;
		sgl_set_ambient(g_a3dBlack);
		sgl_set_diffuse(g_a3dBlack);
		sgl_set_glow(colour);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("flat poly material last error %d\n", last);
		}
		FUN_1005f7f0(p_count, p_ids);
	}
}

// Makes the models p_models (three, the last -2 for none) the detail levels of an object in the
// list p_list, inside the box p_x0..p_z1, changing at the sizes p_changePoints.
// FUNCTION: MW2POWERVR 0x1005fa50
void FUN_1005fa50(
	MechS32 p_list,
	MechS32* p_models,
	MechFloat p_x0,
	MechFloat p_y0,
	MechFloat p_z0,
	MechFloat p_x1,
	MechFloat p_y1,
	MechFloat p_z1,
	MechS32* p_changePoints
)
{
	MechS32 first = 0;
	MechS32 last = 0;
	sgl_vector corner1;
	sgl_vector corner2;

	if (g_a3dInitialized && p_list >= 0) {
		corner1[0] = p_x0 * A3D_SCALE;
		corner1[1] = p_y0 * A3D_SCALE;
		corner1[2] = p_z0 * A3D_SCALE;
		corner2[0] = p_x1 * A3D_SCALE;
		corner2[1] = p_y1 * A3D_SCALE;
		corner2[2] = p_z1 * A3D_SCALE;
		sgl_modify_list(p_list, FALSE);
		sgl_create_detail_levels(FALSE, corner1, corner2, p_models, p_changePoints);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_create_detail_levels last error %d\n", last);
		}
	}
}

// Creates a transform in the list p_list.
// FUNCTION: MW2POWERVR 0x1005fb30
MechS32 FUN_1005fb30(MechS32 p_list)
{
	if (g_a3dInitialized && p_list > 0) {
		sgl_modify_list(p_list, FALSE);
		return sgl_create_transform(TRUE);
	}

	return -1;
}

// Translates the transform p_transform by p_x, p_y, p_z, cleared first (p_clear).
// FUNCTION: MW2POWERVR 0x1005fb60
void FUN_1005fb60(MechS32 p_transform, MechS32 p_clear, MechFloat p_x, MechFloat p_y, MechFloat p_z)
{
	if (g_a3dInitialized && p_transform > 0) {
		sgl_modify_transform(p_transform, p_clear);
		sgl_translate(p_x * A3D_SCALE, p_y * A3D_SCALE, p_z * A3D_SCALE);
		FUN_1005e820();
	}
}

// Rotates the transform p_transform by the angles p_x, p_y, p_z (degrees) in the order p_order
// (c_a3dRotate...), cleared first (p_clear).
// FUNCTION: MW2POWERVR 0x1005fbc0
void FUN_1005fbc0(MechS32 p_transform, MechS32 p_order, MechFloat p_x, MechFloat p_y, MechFloat p_z, MechS32 p_clear)
{
	if (!g_a3dInitialized || p_transform <= 0) {
		return;
	}

	sgl_modify_transform(p_transform, p_clear);
	switch (p_order) {
	case c_a3dRotateYXZ:
		sgl_rotate(g_a3dAxisY, p_y * A3D_DEGREES);
		sgl_rotate(g_a3dAxisX, p_x * A3D_DEGREES);
		sgl_rotate(g_a3dAxisZ, p_z * A3D_DEGREES);
		break;
	case c_a3dRotateXYZ:
		sgl_rotate(g_a3dAxisX, p_x * A3D_DEGREES);
		sgl_rotate(g_a3dAxisY, p_y * A3D_DEGREES);
		sgl_rotate(g_a3dAxisZ, p_z * A3D_DEGREES);
		break;
	case c_a3dRotateXZY:
		sgl_rotate(g_a3dAxisX, p_x * A3D_DEGREES);
		sgl_rotate(g_a3dAxisZ, p_z * A3D_DEGREES);
		sgl_rotate(g_a3dAxisY, p_y * A3D_DEGREES);
		break;
	case c_a3dRotateZXY:
		sgl_rotate(g_a3dAxisZ, p_z * A3D_DEGREES);
		sgl_rotate(g_a3dAxisX, p_x * A3D_DEGREES);
		sgl_rotate(g_a3dAxisY, p_y * A3D_DEGREES);
		break;
	case c_a3dRotateZYX:
		sgl_rotate(g_a3dAxisZ, p_z * A3D_DEGREES);
		sgl_rotate(g_a3dAxisY, p_y * A3D_DEGREES);
		sgl_rotate(g_a3dAxisX, p_x * A3D_DEGREES);
		break;
	case c_a3dRotateYZX:
		sgl_rotate(g_a3dAxisY, p_y * A3D_DEGREES);
		sgl_rotate(g_a3dAxisZ, p_z * A3D_DEGREES);
		sgl_rotate(g_a3dAxisX, p_x * A3D_DEGREES);
		break;
	}

	FUN_1005e820();
}

// Creates a white parallel light in the list p_list, shining along p_x, p_y, p_z.
// FUNCTION: MW2POWERVR 0x1005fe60
MechS32 FUN_1005fe60(MechS32 p_list, MechFloat p_x, MechFloat p_y, MechFloat p_z, MechFloat p_unk0x10)
{
	MechS32 first;
	MechS32 last;
	MechS32 light;

	if (g_a3dInitialized && p_list >= 0) {
		sgl_modify_list(p_list, FALSE);
		g_a3dVertex[0] = p_x;
		g_a3dVertex[1] = p_y;
		g_a3dVertex[2] = p_z;
		light = sgl_create_parallel_light(TRUE, g_a3dWhite, g_a3dVertex, FALSE, TRUE);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_create_parallel_light last error %d\n", last);
		}

		return light;
	}

	return -1;
}

// Creates a grey ambient light of the level p_level in the list p_list.
// FUNCTION: MW2POWERVR 0x1005ff00
MechS32 FUN_1005ff00(MechS32 p_list, MechFloat p_level)
{
	if (g_a3dInitialized && p_list >= 0) {
		sgl_modify_list(p_list, FALSE);
		g_a3dVertex[0] = p_level;
		g_a3dVertex[1] = p_level;
		g_a3dVertex[2] = p_level;
		return sgl_create_ambient_light(TRUE, g_a3dVertex, FALSE);
	}

	return -1;
}

// Sets the camera p_camera's zoom and its near and far distances (none for 0).
// FUNCTION: MW2POWERVR 0x1005ff50
void FUN_1005ff50(MechS32 p_camera, MechFloat p_zoom, MechFloat p_near, MechFloat p_far)
{
	if (p_camera > 0) {
		if (p_far == 0.0f) {
			sgl_set_camera(p_camera, p_zoom * 1.3333f, p_near * A3D_SCALE, 0.0f);
		}
		else {
			sgl_set_camera(p_camera, p_zoom * 1.3333f, p_near * A3D_SCALE, 1.0f / (p_far * A3D_SCALE));
		}
	}
}

// sgl_use_ddraw_mode's callback: keeps the DirectDraw object, and draws the 2D over the frame.
// FUNCTION: MW2POWERVR 0x10060000
MechS32 __cdecl FUN_10060000(sgl_ddraw_frame* p_frame)
{
	if (!g_a3dDirectDraw) {
		g_a3dDirectDraw = (LPDIRECTDRAW) p_frame->ddraw;
	}
	if (p_frame->pixels) {
		FUN_10007db0(p_frame->pixels, p_frame->pitch);
	}

	return 0;
}

// Creates the viewports: the screen's, the 3D view's, and the insets of cockpit panels 2 and 13,
// which the screen's leaves out; and centers panel 14's rectangle on panel 13's.
// FUNCTION: MW2POWERVR 0x10060030
MechS32 FUN_10060030(void)
{
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;
	MechS32 width;
	MechS32 halfHeight;
	A3DCockpitPanel* panel;
	A3DPanelRect* rect;

	g_a3dScreenViewport = FUN_1005e970(0, 0, g_gameWindowWidth, g_gameWindowHeight, &left, &top, &right, &bottom);
	if (g_a3dMainView.m_viewport < 0) {
		return FALSE;
	}

	g_a3dMainView.m_viewport = FUN_1005e970(0, 0, g_gameWindowWidth, g_gameWindowHeight, &left, &top, &right, &bottom);
	if (g_a3dMainView.m_viewport < 0) {
		return FALSE;
	}

	panel = A3D_PANEL(2);
	g_a3dPanel2View.m_viewport = FUN_1005e970(0x200, 0x160, 0x60, 0x60, &left, &top, &right, &bottom);
	if (g_a3dPanel2View.m_viewport < 0) {
		return FALSE;
	}

	panel->m_rect->m_left = left;
	panel->m_rect->m_top = top;
	panel->m_rect->m_right = right;
	panel->m_rect->m_bottom = bottom;
	panel->m_setRect(panel, panel->m_rect);
	panel->m_view = &g_a3dPanel2View;

	panel = A3D_PANEL(13);
	g_a3dPanel13View.m_viewport = FUN_1005e970(0x20, 0x160, 0x60, 0x60, &left, &top, &right, &bottom);
	if (g_a3dPanel13View.m_viewport < 0) {
		return FALSE;
	}

	panel->m_rect->m_left = left;
	panel->m_rect->m_top = top;
	panel->m_rect->m_right = right;
	panel->m_rect->m_bottom = bottom;
	panel->m_setRect(panel, panel->m_rect);
	panel->m_view = &g_a3dPanel13View;

	panel = A3D_PANEL(14);
	rect = panel->m_rect;
	width = rect->m_right - rect->m_left + 1;
	rect->m_left = left;
	rect->m_right = left + width - 1;
	halfHeight = (rect->m_bottom - rect->m_top + 1) >> 1;
	rect->m_top = top - halfHeight;
	rect->m_bottom = bottom + halfHeight - 1;
	panel->m_setRect(panel, rect);

	sgl_subtract_viewport(g_a3dScreenViewport, g_a3dPanel13View.m_viewport);
	return TRUE;
}

// Creates the sky and the ground, textured as the "Sky" and "Ground" settings say.
// FUNCTION: MW2POWERVR 0x10060200
void FUN_10060200(void)
{
	MechS32 width;
	MechS32 height;
	MechS32 material;

	if (g_renderSettings.m_texturedSky && g_skyTileU > 1.0f) {
		material = FUN_10061810(c_a3dSkyAnimation, &width, &height, FALSE);
	}
	else {
		material = -1;
	}
	FUN_1005ed90(g_a3dSkyGroundList, material, 2000000.0f, 64000000.0f, g_skyTileU, 55555.555f, g_a3dFogDensity);

	if (g_renderSettings.m_texturedGround && g_groundTile > 1.0f) {
		material = FUN_10061810(c_a3dGroundAnimation, &width, &height, TRUE);
	}
	else {
		material = -1;
	}
	FUN_1005ec90(g_a3dSkyGroundList, material, -10.0f, 16000000.0f, g_groundTile);
}

// Turns the ground's material to its texture (p_textured) or to its average color.
// FUNCTION: MW2POWERVR 0x100602c0
void FUN_100602c0(MechS32 p_textured)
{
	if (p_textured) {
		FUN_100610d0(c_a3dGroundAnimation);
	}
	else {
		FUN_10060fd0(c_a3dGroundAnimation, NULL);
	}
}

// Scales a fog density read as a distance down to SGL's.
// FUNCTION: MW2POWERVR 0x100602e0
void FUN_100602e0(void)
{
	if (g_a3dFogDensity > 0.7f) {
		g_a3dFogDensity *= 0.000244140625f;
	}

	DebugPrintInternal("fog_density: %f\n", g_a3dFogDensity);
}

// Computes the fog's and the background's colors from the ground's and the sky's textures and the
// eyepoint's ambient light.
// FUNCTION: MW2POWERVR 0x10060320
void FUN_10060320(void)
{
	MechFloat ground[3];
	MechFloat sky[3];
	MechFloat ambient;

	ambient = g_eyepoint->m_ambientLight * 0.01171875f;
	if (g_groundTile < 0.0f) {
		FUN_10027620(c_a3dGroundAnimation, &ground[0], &ground[1], &ground[2]);
		g_a3dFogColour[0] = g_a3dBackground[0] = ground[0];
		g_a3dFogColour[1] = g_a3dBackground[1] = ground[1];
		g_a3dFogColour[2] = g_a3dBackground[2] = ground[2];
		return;
	}

	if (g_renderSettings.m_texturedGround && (MechFloat) fabs(g_groundTile - 1.0f) >= 1e-07f) {
		FUN_10027620(c_a3dGroundAnimation, &ground[0], &ground[1], &ground[2]);
		if (g_renderSettings.m_drawSky && (MechFloat) fabs(g_skyTileU - 1.0f) >= 1e-07f) {
			FUN_10027620(c_a3dSkyAnimation, &sky[0], &sky[1], &sky[2]);
			g_a3dFogColour[0] = (ground[0] + ambient + sky[0]) / 3.0f;
			g_a3dFogColour[1] = (ground[1] + ambient + sky[1]) / 3.0f;
			g_a3dFogColour[2] = (ground[2] + ambient + sky[2]) / 3.0f;
		}
		else {
			g_a3dFogColour[0] = (ground[0] + ambient) * 0.5f;
			g_a3dFogColour[1] = (ground[1] + ambient) * 0.5f;
			g_a3dFogColour[2] = (ground[2] + ambient) * 0.5f;
		}
		g_a3dBackground[0] = g_a3dFogColour[0];
		g_a3dBackground[1] = g_a3dFogColour[1];
		g_a3dBackground[2] = g_a3dFogColour[2];
	}
	else if (g_renderSettings.m_texturedSky && (MechFloat) fabs(g_skyTileU - 1.0f) >= 1e-07f) {
		FUN_10027620(c_a3dSkyAnimation, &sky[0], &sky[1], &sky[2]);
		g_a3dFogColour[0] = (sky[0] + ambient) * 0.5f;
		g_a3dFogColour[1] = (sky[1] + ambient) * 0.5f;
		g_a3dFogColour[2] = (sky[2] + ambient) * 0.5f;
		g_a3dBackground[0] = sky[0];
		g_a3dBackground[1] = sky[1];
		g_a3dBackground[2] = sky[2];
	}
	else {
		g_a3dFogColour[0] = g_a3dFogColour[1] = g_a3dFogColour[2] = ambient;
		g_a3dBackground[0] = g_a3dBackground[1] = g_a3dBackground[2] = ambient;
	}

	DebugPrintInternal(
		"ground avg = (%.2f,%.2f,%.2f) sky avg = (%.2f,%.2f,%.2f) fog = (%.2f,%.2f,%.2f)\n",
		ground[0],
		ground[1],
		ground[2],
		sky[0],
		sky[1],
		sky[2],
		g_a3dBackground[0],
		g_a3dBackground[1],
		g_a3dBackground[2]
	);
}

// Sets the view p_view's qualities again, in its list cleared.
// FUNCTION: MW2POWERVR 0x10060640
void FUN_10060640(A3DView* p_view)
{
	sgl_modify_list(p_view->m_list, TRUE);
	sgl_qual_fog(p_view->m_fog);
	sgl_qual_texturing(p_view->m_texturing);
	sgl_qual_smooth_shading(p_view->m_smoothShading);
	sgl_qual_generate_shadows(p_view->m_shadows);
}

// FUNCTION: MW2POWERVR 0x10060690
MechS32 A3D_SetQuality(A3DView* p_view, MechS32 p_quality, MechS32 p_value)
{
	if (!p_view) {
		return 0;
	}

	sgl_modify_list(p_view->m_list, FALSE);
	switch (p_quality) {
	case c_a3dQualityFog:
		p_view->m_fog = p_value;
		sgl_qual_fog(p_value);
		if (p_view == &g_a3dMainView) {
			if ((MechFloat) fabs(g_a3dFogDensity) < 1e-07f) {
				p_value = FALSE;
			}
			FUN_1005ef90(p_value);
		}
		break;
	case c_a3dQualityTexturing:
		p_view->m_texturing = p_value;
		sgl_qual_texturing(p_value);
		break;
	case c_a3dQualitySmoothShading:
		p_view->m_smoothShading = p_value;
		sgl_qual_smooth_shading(p_value);
		break;
	case c_a3dQualityShadows:
		p_view->m_shadows = p_value;
		sgl_qual_generate_shadows(p_value);
		break;
	default:
		return 0;
	}

	return 1;
}

// Sets all of the view p_view's qualities, in its list cleared.
// FUNCTION: MW2POWERVR 0x10060770
void FUN_10060770(A3DView* p_view, MechS32 p_fog, MechS32 p_texturing, MechS32 p_smoothShading, MechS32 p_shadows)
{
	sgl_modify_list(p_view->m_list, TRUE);
	p_view->m_fog = p_fog;
	sgl_qual_fog(p_fog);
	p_view->m_texturing = p_texturing;
	sgl_qual_texturing(p_texturing);
	p_view->m_smoothShading = p_smoothShading;
	sgl_qual_smooth_shading(p_smoothShading);
	p_view->m_shadows = p_shadows;
	sgl_qual_generate_shadows(p_shadows);
}

// Colors the light p_red, p_green, p_blue (an effect's flash): the sky's and the ground's materials,
// the 3D view's lights, the fog and the background.
// FUNCTION: MW2POWERVR 0x100607d0
void FUN_100607d0(MechFloat p_red, MechFloat p_green, MechFloat p_blue)
{
	sgl_colour colour;
	sgl_vector direction;

	direction[0] = g_mainEyepoint.m_lightX;
	direction[1] = g_mainEyepoint.m_lightY;
	direction[2] = g_mainEyepoint.m_lightZ;
	g_a3dLightColour[0] = p_red;
	g_a3dLightColour[1] = p_green;
	g_a3dLightColour[2] = p_blue;
	FUN_100610d0(c_a3dSkyAnimation);
	FUN_100610d0(c_a3dGroundAnimation);
	sgl_set_parallel_light(g_a3dMainView.m_light, g_a3dLightColour, direction, FALSE, TRUE);

	if (g_a3dLightColour[0] == 1.0f && g_a3dLightColour[1] == 1.0f && g_a3dLightColour[2] == 1.0f) {
		colour[0] = g_eyepoint->m_ambientLight * g_a3dLightColour[0] * 0.01171875f;
		colour[1] = g_eyepoint->m_ambientLight * g_a3dLightColour[1] * 0.01171875f;
		colour[2] = g_eyepoint->m_ambientLight * g_a3dLightColour[2] * 0.01171875f;
	}
	else {
		colour[0] = g_a3dLightColour[0] * 0.1f;
		colour[1] = g_a3dLightColour[1] * 0.1f;
		colour[2] = g_a3dLightColour[2] * 0.1f;
	}
	sgl_set_ambient_light(g_a3dMainView.m_ambientLight, colour, FALSE);

	colour[0] = g_a3dLightColour[0] * g_a3dFogColour[0];
	colour[1] = g_a3dLightColour[1] * g_a3dFogColour[1];
	colour[2] = g_a3dLightColour[2] * g_a3dFogColour[2];
	sgl_set_fog(g_a3dMainView.m_camera, colour, g_a3dFogDensity);

	colour[0] = g_a3dLightColour[0] * g_a3dBackground[0];
	colour[1] = g_a3dLightColour[1] * g_a3dBackground[1];
	colour[2] = g_a3dLightColour[2] * g_a3dBackground[2];
	sgl_set_background_colour(g_a3dMainView.m_camera, colour);
}

// Creates the scene: its root and the lists in it, the views' qualities, lights and cameras, and
// the camera-facing sprites' list. The light points down, at least 0.3 of the way.
// FUNCTION: MW2POWERVR 0x100609a0
MechS32 FUN_100609a0(void)
{
	MechFloat length;
	MechFloat y;

	FUN_10060320();
	FUN_100602e0();

	g_a3dMainView.m_root = FUN_1005f200(0);
	if (g_a3dMainView.m_root < 0) {
		return FALSE;
	}

	g_a3dMainView.m_list = sgl_create_list(TRUE, FALSE, FALSE);
	if (g_a3dMainView.m_list < 0) {
		return FALSE;
	}

	FUN_10060770(&g_a3dMainView, g_a3dMainView.m_fog, TRUE, g_a3dMainView.m_smoothShading, FALSE);
	DebugPrintInternal(
		"light pos: (%6.3f, %6.3f, %6.3f)\n",
		g_mainEyepoint.m_lightX,
		g_mainEyepoint.m_lightY,
		g_mainEyepoint.m_lightZ
	);

	y = (MechFloat) fabs(g_mainEyepoint.m_lightY);
	length = (MechFloat) sqrt(
		g_mainEyepoint.m_lightX * g_mainEyepoint.m_lightX + y * y + g_mainEyepoint.m_lightZ * g_mainEyepoint.m_lightZ
	);
	g_mainEyepoint.m_lightX = -(g_mainEyepoint.m_lightX / length);
	g_mainEyepoint.m_lightZ = -(g_mainEyepoint.m_lightZ / length);
	g_mainEyepoint.m_lightY = -(y / length);
	if (g_mainEyepoint.m_lightY > -0.3f) {
		g_mainEyepoint.m_lightY = -0.3f;
	}
	DebugPrintInternal(
		"normalized light dir: (%6.3f, %6.3f, %6.3f)\n",
		g_mainEyepoint.m_lightX,
		g_mainEyepoint.m_lightY,
		g_mainEyepoint.m_lightZ
	);

	g_a3dMainView.m_light = FUN_1005fe60(
		g_a3dMainView.m_root,
		g_mainEyepoint.m_lightX,
		g_mainEyepoint.m_lightY,
		g_mainEyepoint.m_lightZ,
		1.0f
	);
	g_a3dMainView.m_ambientLight = FUN_1005ff00(g_a3dMainView.m_root, g_eyepoint->m_ambientLight * 0.01171875f);
	if (g_a3dMainView.m_light < 0 || g_a3dMainView.m_ambientLight < 0) {
		return FALSE;
	}

	g_a3dMainView.m_camera =
		FUN_1005eba0(g_a3dMainView.m_root, g_a3dBackground, 50.0f, g_eyepoint->m_farPlane, &g_a3dMainView.m_transform);
	if (g_a3dMainView.m_camera < 0) {
		return FALSE;
	}

	sgl_set_fog(g_a3dMainView.m_camera, g_a3dFogColour, g_a3dFogDensity);
	g_a3dPanel2View.m_camera = FUN_1005eba0(
		g_a3dMainView.m_root,
		g_a3dBackground,
		50.0f,
		g_eyepoint->m_farPlane * 0.5f,
		&g_a3dPanel2View.m_transform
	);
	if (g_a3dPanel2View.m_camera < 0) {
		return FALSE;
	}

	g_a3dPanel2View.m_root = g_a3dMainView.m_root;
	g_a3dPanel2View.m_list = g_a3dMainView.m_list;
	g_a3dPanel2View.m_light = g_a3dMainView.m_light;
	g_a3dPanel2View.m_ambientLight = g_a3dMainView.m_ambientLight;
	g_a3dLevel4List = FUN_1005f200(g_a3dMainView.m_root);
	g_a3dSkyGroundList = FUN_1005f200(g_a3dMainView.m_root);
	g_a3dSpriteList = FUN_1005f200(g_a3dMainView.m_root);
	g_a3dLevel0List = FUN_1005f200(g_a3dMainView.m_root);
	g_a3dTerrainList = FUN_1005f200(g_a3dMainView.m_root);
	g_a3dThingList = FUN_1005f200(g_a3dMainView.m_root);
	g_a3dLevel1List = FUN_1005f200(g_a3dMainView.m_root);
	g_a3dKind400List = FUN_1005f200(g_a3dMainView.m_root);
	g_a3dLevel3List = FUN_1005f200(g_a3dMainView.m_root);

	g_a3dPanel13View.m_root = FUN_1005f200(0);
	g_a3dPanel13View.m_camera = FUN_1005eba0(
		g_a3dPanel13View.m_root,
		g_a3dBlack,
		50.0f,
		g_eyepoint->m_farPlane * 0.5f,
		&g_a3dPanel13View.m_transform
	);
	if (g_a3dPanel13View.m_root < 0 || g_a3dPanel13View.m_camera < 0) {
		return FALSE;
	}

	g_a3dPanel13View.m_list = sgl_create_list(TRUE, FALSE, FALSE);
	FUN_10060770(&g_a3dPanel13View, FALSE, TRUE, FALSE, FALSE);
	g_a3dPanel13View.m_light = FUN_1005fe60(
		g_a3dPanel13View.m_root,
		g_mainEyepoint.m_lightX,
		-g_mainEyepoint.m_lightY,
		g_mainEyepoint.m_lightZ,
		1.0f
	);
	g_a3dPanel13View.m_ambientLight = FUN_1005ff00(g_a3dPanel13View.m_root, g_eyepoint->m_ambientLight * 0.00390625f);
	if (g_a3dPanel13View.m_light < 0 || g_a3dPanel13View.m_ambientLight < 0) {
		return FALSE;
	}

	g_a3dFacingList = sgl_create_list(TRUE, FALSE, TRUE);
	if (g_a3dFacingList < 0) {
		return FALSE;
	}

	g_a3dFacingTransform = FUN_1005fb30(g_a3dFacingList);
	return TRUE;
}

// Registers the texture callback.
// FUNCTION: MW2POWERVR 0x10060e00
MechS32 FUN_10060e00(void)
{
	MechS32 first;
	MechS32 last;

	sgl_register_texture_callback(FUN_10061360, g_a3dCacheEntries, 2000);
	sgl_get_errors(&first, &last);
	if (last) {
		DebugPrintInternal("sgl_register_texture_callback error: %d\n", last);
		return FALSE;
	}

	return TRUE;
}

// FUNCTION: MW2POWERVR 0x10060e60
MechS32 FUN_10060e60(HWND p_window, MechS32 p_width, MechS32 p_height)
{
	MechS32 first;
	MechS32 last;

	if (!FUN_1005e880(p_window, p_width, p_height)) {
		return FALSE;
	}
	if (!FUN_1005e8c0(g_gameWindowWidth, g_gameWindowHeight)) {
		return FALSE;
	}
	if (!FUN_10060030()) {
		return FALSE;
	}

	sgl_render(g_a3dMainView.m_viewport, g_a3dMainView.m_camera, TRUE);
	sgl_get_errors(&first, &last);
	if (last) {
		DebugPrintInternal("sgl_render error: %d\n", last);
	}

	return TRUE;
}

// Opens the renderer: the texture callback and the scene.
// FUNCTION: MW2POWERVR 0x10060f00
MechS32 FUN_10060f00(void)
{
	g_a3dInitialized = TRUE;
	if (!FUN_10060e00()) {
		return FALSE;
	}

	return FUN_100609a0() != 0;
}

// Lights the material being modified: dim grey (p_mode), lit white (the animation p_animation is
// lit), or glowing its texture.
// FUNCTION: MW2POWERVR 0x10060f30
void FUN_10060f30(Animation* p_animation, MechFloat* p_colour, MechS32 p_mode)
{
	if (p_mode) {
		sgl_set_ambient(g_a3dBlack);
		sgl_set_diffuse(g_a3dGrey);
	}
	else if (!p_animation->m_lit) {
		sgl_set_ambient(g_a3dBlack);
		sgl_set_diffuse(g_a3dBlack);
		sgl_set_glow(g_a3dWhite);
		sgl_set_texture_effect(FALSE, FALSE, FALSE, TRUE);
	}
	else {
		sgl_set_ambient(g_a3dWhite);
		sgl_set_diffuse(g_a3dWhite);
		sgl_set_texture_effect(TRUE, TRUE, FALSE, FALSE);
	}

	sgl_set_opacity(1.0f);
}

// Colors the animation p_index's material p_colour, or its texture's average color.
// FUNCTION: MW2POWERVR 0x10060fd0
void FUN_10060fd0(MechS32 p_index, MechFloat* p_colour)
{
	Animation* animation;
	sgl_colour colour;

	animation = &g_animations[p_index];
	if (!p_colour) {
		FUN_10027620(p_index, &colour[0], &colour[1], &colour[2]);
	}
	else {
		colour[0] = p_colour[0];
		colour[1] = p_colour[1];
		colour[2] = p_colour[2];
	}

	if (animation->m_material >= 0) {
		sgl_modify_material(animation->m_material, TRUE);
		sgl_set_ambient(colour);
		sgl_set_diffuse(colour);
	}
}

// Colors the materials of the animations of the set p_set their first frame's average color (their
// texture unloaded).
// FUNCTION: MW2POWERVR 0x10061050
void FUN_10061050(MechS32 p_set)
{
	Animation* animation;
	sgl_colour colour;

	colour[0] = g_animFrames[p_set][0].m_red;
	colour[1] = g_animFrames[p_set][0].m_green;
	colour[2] = g_animFrames[p_set][0].m_blue;
	for (animation = g_animations; animation < g_animations + 0x200; animation++) {
		if (animation->m_set == p_set && animation->m_material >= 0) {
			sgl_modify_material(animation->m_material, TRUE);
			sgl_set_ambient(colour);
			sgl_set_diffuse(colour);
		}
	}
}

// Sets the animation p_index's material to its frame's texture, lit as FUN_10060f30 says (the sky
// and the ground lit only when the flag at 0x101ad7e4 is set).
// FUNCTION: MW2POWERVR 0x100610d0
void FUN_100610d0(MechS32 p_index)
{
	Animation* animation;
	AnimFrame* frame;

	frame = &g_animFrames[g_animations[p_index].m_set][g_animations[p_index].m_frame];
	animation = &g_animations[p_index];
	if (animation->m_material > 0 && frame->m_texture > 0) {
		if (p_index == c_a3dSkyAnimation || p_index == c_a3dGroundAnimation) {
			animation->m_lit = TRUE;
			if (!g_unk0x101ad7e4) {
				animation->m_lit = FALSE;
			}
		}

		sgl_modify_material(animation->m_material, TRUE);
		FUN_100618b0(animation->m_material, frame->m_texture);
		FUN_10060f30(animation, g_a3dLightColour, g_a3dLightingMode);
	}
}

// Sets every animation's material to its texture (FUN_100610d0) in the lighting mode p_mode, but the
// ground's in cockpit layout 4.
// FUNCTION: MW2POWERVR 0x10061160
void FUN_10061160(MechFloat* p_colour, MechS32 p_mode)
{
	MechS32 i;

	g_a3dLightingMode = p_mode;
	for (i = 0; i < 0x200; i++) {
		if (g_cockpitLayoutIndex != 4 || i != c_a3dGroundAnimation) {
			FUN_100610d0(i);
		}
	}
}

// Unloads a texture to make room for the entry p_index's: the least recently used one of the pixels
// p_data that this frame doesn't need, or else the needed one of the lowest priority, whose
// animations then take their average color.
// FUNCTION: MW2POWERVR 0x100611a0
MechS32 FUN_100611a0(sgl_texture_cache_entry* p_entries, MechS32 p_count, MechS32 p_index, MechU16* p_data)
{
	sgl_texture_cache_entry* entry;
	AnimFrame* frame;
	MechS32 first;
	MechS32 last;
	MechS32 oldest;
	MechS32 oldestEntry;
	MechS32 priority;
	MechS32 priorityEntry;
	MechS32 set;
	MechS32 number;
	MechS32 i;

	oldest = g_a3dFrame;
	oldestEntry = -1;
	priorityEntry = -1;
	priority = p_entries[p_index].priority;
	for (i = 0, entry = p_entries; i < p_count; i++, entry++) {
		if (i == p_index) {
			continue;
		}

		set = entry->user_id & 0xffff;
		number = entry->user_id >> 16;
		if (set < 0 || set >= 0x200 || number < 0 || number >= 0x20) {
			DebugPrintInternal("invalid user id\n");
			continue;
		}

		frame = &g_animFrames[set][number];
		if (frame->m_data == p_data && entry->loaded && !entry->unk10) {
			if (!entry->needed) {
				if (oldest > frame->m_lastUsed) {
					oldestEntry = i;
					oldest = frame->m_lastUsed;
				}
			}
			else if (priority > entry->priority) {
				priority = entry->priority;
				priorityEntry = i;
			}
		}
	}

	if (oldestEntry >= 0) {
		entry = &p_entries[oldestEntry];
		sgl_unload_cached_texture(entry->texture);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_unload_cached_texture error: %d\n", last);
			return FALSE;
		}

		entry->loaded = FALSE;
		return TRUE;
	}

	if (priorityEntry >= 0) {
		entry = &p_entries[priorityEntry];
		sgl_unload_cached_texture(entry->texture);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_unload_cached_texture error: %d\n", last);
			return FALSE;
		}

		FUN_10061050(entry->user_id & 0xffff);
		entry->loaded = FALSE;
		return TRUE;
	}

	return FALSE;
}

// SGL's texture callback: loads each needed texture that isn't, unloading others while the memory
// is full, and marks the loaded ones used this frame. An animation set whose texture can't be loaded
// takes its average color.
// FUNCTION: MW2POWERVR 0x10061360
void __cdecl FUN_10061360(sgl_texture_cache_entry* p_entries, MechS32 p_count, MechU32 p_freeMemory)
{
	sgl_texture_cache_entry* entry;
	AnimFrame* frame;
	sgl_intermediate_map* map;
	MechU16* data;
	MechS32 unk;
	MechS32 first;
	MechS32 result;
	MechS32 size;
	MechS32 failed;
	MechS32 set;
	MechS32 number;
	MechS32 i;

	for (i = 0, entry = p_entries; i < p_count; i++, entry++) {
		if (!entry->needed) {
			continue;
		}

		set = entry->user_id & 0xffff;
		number = entry->user_id >> 16;
		if (set < 0 || set >= 0x200 || number < 0 || number >= 0x20) {
			DebugPrintInternal("invalid user id: 0x%08x\n", entry->user_id);
			continue;
		}

		frame = &g_animFrames[set][number];
		if (frame->m_resourceId <= 0) {
			DebugPrintInternal("invalid cache id\n");
			continue;
		}
		if (entry->loaded) {
			frame->m_lastUsed = g_a3dFrame;
			continue;
		}

		map = FUN_10027350(set, number, &data, &unk);
		if (!map) {
			DebugPrintInternal("can't get texture bits\n");
			continue;
		}

		failed = FALSE;
		result = 0;
		size = sgl_texture_size(map);
		while (!entry->loaded) {
			if (failed) {
				break;
			}

			if (size < (MechS32) p_freeMemory) {
				sgl_load_cached_texture(entry->texture, 0, 0, 0, 0, map, NULL, NULL);
				sgl_get_errors(&first, &result);
			}
			else {
				result = c_sglErrorTextureMemoryFull;
			}

			if (result == c_sglErrorTextureMemoryFull) {
				if (!FUN_100611a0(p_entries, p_count, i, data)) {
					DebugPrintInternal("can't unload anything else\n");
					failed = TRUE;
				}
				else {
					p_freeMemory = sgl_get_free_texture_mem();
				}
			}
			else if (!result) {
				entry->loaded = TRUE;
				frame->m_lastUsed = g_a3dFrame;
			}
			else {
				failed = TRUE;
			}
		}

		if (failed) {
			FUN_10061050(set);
		}
	}
}

// Converts the scene ("Load3DObjects"): the sky and the ground, the players' mechs at their detail
// levels, the shapes of the scene and the hidden ones, and the sky object; then prints the counts.
// FUNCTION: MW2POWERVR 0x10061530
void FUN_10061530(void)
{
	Player* player;
	Shape* shape;
	MechS32 i;
	MechS32 j;

	DebugPrintInternal("Load3DObjects\n");
	FUN_10060200();
	for (i = 0; i < g_playerCount; i++) {
		player = g_players[i];
		if (player->m_detailLevel > -1) {
			FUN_10062af0(player->m_obj, FUN_100617b0(player->m_detailLevel));
		}
	}

	FUN_100259c0();
	for (i = 0; i < g_playerCount; i++) {
		player = g_players[i];
		for (j = 0; j <= player->m_baseLevel; j++) {
			FUN_100252a0(player->m_index, j);
		}
	}

	for (shape = g_sceneShapes->m_next; shape; shape = shape->m_next) {
		if (!FUN_10062960(shape)) {
			if (shape->m_object) {
				FUN_10062af0(shape->m_object, -1);
			}
			else {
				FUN_10062160(shape, -1);
			}
		}
	}

	g_a3dVerbose = FALSE;
	for (shape = g_hiddenShapes->m_next; shape; shape = shape->m_next) {
		if (!FUN_10062960(shape)) {
			if (shape->m_object) {
				FUN_10062af0(shape->m_object, -1);
			}
			else {
				FUN_10062160(shape, -1);
			}
		}
	}

	if (g_skyObject) {
		FUN_10062af0(g_skyObject, -1);
	}

	FUN_10061160(g_a3dLightColour, 0);
	if (g_a3dMainView.m_fog && (MechFloat) fabs(g_a3dFogDensity) >= 1e-07f) {
		FUN_1005ef90(TRUE);
	}

	DebugPrintInternal(
		"terrain: %d objects, %d polys, %d avg polys\n",
		g_a3dTerrainObjects,
		g_a3dTerrainPolys,
		g_a3dTerrainObjects ? g_a3dTerrainPolys / g_a3dTerrainObjects : 0
	);
	DebugPrintInternal(
		"gamethings: %d objects, %d polys, %d avg polys\n",
		g_a3dThingObjects,
		g_a3dThingPolys,
		g_a3dThingObjects ? g_a3dThingPolys / g_a3dThingObjects : 0
	);
	DebugPrintInternal(
		"gamepieces: %d objects, %d polys, %d avg polys\n",
		g_a3dPieceObjects,
		g_a3dPiecePolys,
		g_a3dPieceObjects ? g_a3dPiecePolys / g_a3dPieceObjects : 0
	);
	DebugPrintInternal(
		"total: %d objs, %d polys\n",
		g_a3dTerrainObjects + g_a3dThingObjects + g_a3dPieceObjects,
		g_a3dTerrainPolys + g_a3dThingPolys + g_a3dPiecePolys
	);
	DebugPrintInternal("done Load3DObjects\n");
}

// Frees the shape p_shape's models, now that SGL has them, but its first (which collision uses),
// or all of them for a shape that collides without them.
// FUNCTION: MW2POWERVR 0x10061750
void FUN_10061750(Shape* p_shape)
{
	Model* model;
	Model* next;

	if (p_shape->m_collisionType == 4 || p_shape->m_collisionType == 6 || (p_shape->m_kind & 0x100)) {
		p_shape->m_model = NULL;
		model = p_shape->m_models;
		p_shape->m_models = NULL;
	}
	else {
		p_shape->m_model = p_shape->m_models;
		model = p_shape->m_models->m_next;
		p_shape->m_models->m_next = NULL;
	}

	while (model) {
		next = model->m_next;
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, model);
		model = next;
	}
}

// The list of the detail level p_level's shapes.
// FUNCTION: MW2POWERVR 0x100617b0
MechS32 FUN_100617b0(MechS32 p_level)
{
	switch (p_level) {
	case 0:
		return g_a3dLevel0List;
	case 1:
	case 2:
		return g_a3dLevel1List;
	case 3:
		return g_a3dLevel3List;
	case 4:
		return g_a3dLevel4List;
	default:
		return g_a3dMainView.m_root;
	}
}

// Moves the player p_player's mech into the list of the detail level p_level.
// FUNCTION: MW2POWERVR 0x100617f0
void FUN_100617f0(Player* p_player, MechS32 p_level)
{
	FUN_1005f320(p_player->m_obj->m_list, FUN_100617b0(p_level));
}

// The animation p_index's material, its texture loaded (FUN_100273e0, which gives its size), and
// lit (p_lit); -1 when there is none.
// FUNCTION: MW2POWERVR 0x10061810
MechS32 FUN_10061810(MechS32 p_index, MechS32* p_width, MechS32* p_height, MechS32 p_lit)
{
	Animation* animation;

	if (p_index < 0 || p_index >= 0x200) {
		return -1;
	}
	if (!FUN_100273e0(p_index, p_width, p_height)) {
		return -1;
	}

	animation = &g_animations[p_index];
	if (p_lit) {
		animation->m_lit = TRUE;
	}

	return animation->m_material;
}

// The size of the texture p_map, by its type.
// FUNCTION: MW2POWERVR 0x10061870
MechS32 FUN_10061870(sgl_intermediate_map* p_map)
{
	switch (p_map->id[3]) {
	case 0:
		return 0x20;
	case 1:
		return 0x40;
	case 2:
		return 0x80;
	case 3:
		return 0x100;
	default:
		return 0;
	}
}

// Sets the material p_material's texture to p_texture.
// FUNCTION: MW2POWERVR 0x100618b0
void FUN_100618b0(MechS32 p_material, MechS32 p_texture)
{
	MechS32 first;
	MechS32 last;

	if (p_material >= 0 && p_texture >= 0) {
		sgl_modify_material(p_material, FALSE);
		sgl_set_texture_map(p_texture, FALSE, FALSE);
		sgl_get_errors(&first, &last);
		if (last) {
			DebugPrintInternal("sgl_set_texture_map error %d\n", last);
		}
	}
}

// The list a shape of p_shape's kind goes in.
// FUNCTION: MW2POWERVR 0x10061910
MechS32 FUN_10061910(Shape* p_shape)
{
	MechU16 kind;
	MechU16 type;

	if (!p_shape) {
		return g_a3dMainView.m_root;
	}

	kind = p_shape->m_kind;
	type = kind & 0xf0;
	if (type == 0x20 || type == 0x10 || type == 0x60) {
		return g_a3dSpriteList;
	}
	if (type == 0xa0) {
		return g_a3dLevel4List;
	}
	if (kind & 0x100) {
		return g_a3dLevel0List;
	}
	if (kind & 0x200) {
		return g_a3dThingList;
	}
	if (kind & 0x400) {
		return g_a3dKind400List;
	}
	if (kind & 0x800) {
		return g_a3dTerrainList;
	}
	if (p_shape->m_models &&
		(((Face*) ((MechU8*) p_shape->m_models + p_shape->m_models->m_faceOffset))->m_color & 0xf000) == 0x3000) {
		return g_a3dSpriteList;
	}

	return g_a3dMainView.m_root;
}

// What a face of the color p_color is drawn with: translucent (p_translucent, set only), lit
// through its normals (p_lit), textured (p_textured) by the animation p_index.
// FUNCTION: MW2POWERVR 0x100619a0
void FUN_100619a0(MechU16 p_color, MechS32* p_translucent, MechS32* p_lit, MechS32* p_textured, MechS32* p_index)
{
	switch (p_color & 0xf000) {
	case 0x1000:
	case 0x4000:
		*p_textured = FALSE;
		*p_lit = TRUE;
		break;
	case 0x3000:
		*p_textured = TRUE;
		*p_lit = FALSE;
		*p_index = (p_color & 0xff0) >> 4;
		if (*p_index >= 0x100) {
			*p_index -= 0x100;
		}
		break;
	case 0x5000:
	case 0x6000:
	case 0x7000:
		*p_textured = TRUE;
		*p_lit = TRUE;
		*p_index = (p_color & 0xff) + 0x100;
		break;
	case 0x8000:
	case 0xb000:
		*p_translucent = TRUE;
		*p_textured = TRUE;
		*p_lit = FALSE;
		*p_index = (p_color & 0xff) + 0x100;
		break;
	case 0x9000:
	case 0xa000:
		*p_translucent = TRUE;
		*p_textured = TRUE;
		*p_lit = TRUE;
		*p_index = (p_color & 0xff) + 0x100;
		break;
	default:
		*p_lit = FALSE;
		*p_textured = FALSE;
		break;
	}
}

// The RGB565 color p_color's components, 0 to 1.
// FUNCTION: MW2POWERVR 0x10061b20
void FUN_10061b20(MechU32 p_color, MechFloat* p_red, MechFloat* p_green, MechFloat* p_blue)
{
	*p_red = ((p_color & 0xf800) >> 11) * 0.03125f;
	*p_green = ((p_color & 0x7e0) >> 5) * 0.015625f;
	*p_blue = (p_color & 0x1f) * 0.03125f;
}

// A flat face's color: the palette's at the color's bits 4 to 11.
// FUNCTION: MW2POWERVR 0x10061ba0
void FUN_10061ba0(MechU16 p_color, MechFloat* p_red, MechFloat* p_green, MechFloat* p_blue)
{
	MechU32 pixel;

	pixel = VFX_lookaside_read16((p_color & 0xff0) >> 4);
	FUN_10061b20(pixel, p_red, p_green, p_blue);
}

// FUNCTION: MW2POWERVR 0x10061be0
void FUN_10061be0(MechU16 p_color, MechFloat* p_red, MechFloat* p_green, MechFloat* p_blue)
{
	MechU32 pixel;

	pixel = VFX_lookaside_read16((p_color & 0xff0) >> 4);
	FUN_10061b20(pixel, p_red, p_green, p_blue);
}

// A Gouraud face's color: the palette's at the middle of its shade range.
// FUNCTION: MW2POWERVR 0x10061c20
void FUN_10061c20(MechU16 p_color, MechFloat* p_red, MechFloat* p_green, MechFloat* p_blue)
{
	MechU32 pixel;

	pixel = VFX_lookaside_read16(((p_color & 0xf00) | 0x80) >> 4);
	FUN_10061b20(pixel, p_red, p_green, p_blue);
}

// Makes the materials of the object tree p_obj glow by their shapes' kinds and damage (p_flags bit
// 0), or not.
// FUNCTION: MW2POWERVR 0x10061c60
void FUN_10061c60(SceneObject* p_obj, MechU32 p_flags)
{
	Shape* shape;
	MechU32 damage;

	if (p_obj && (shape = p_obj->m_shape) != NULL) {
		if (p_obj->m_material != -1) {
			sgl_modify_material(p_obj->m_material, TRUE);
			if (p_flags & 1) {
				if (shape->m_kind & 0x200) {
					sgl_set_glow(g_a3dGlowCold);
				}
				else if (shape->m_kind & 0x400) {
					sgl_set_glow(g_a3dGlowHot);
				}
				else if (shape->m_kind & 0x100) {
					damage = (shape->m_flags & 0xf0) >> 4;
					if (damage < 1) {
						sgl_set_glow(g_a3dGlowCool);
					}
					else if (damage < 0xc) {
						sgl_set_glow(g_a3dGlowWarm);
					}
					else {
						sgl_set_glow(g_a3dGlowRed);
					}
				}
				else {
					sgl_set_glow(g_a3dGlowCold);
				}
			}
		}
		else {
			DebugPrint("ERROR! Segment w/o damage material %08x:%08x(%d)\n", p_obj, shape, shape->m_partId);
		}
	}

	for (p_obj = p_obj->m_firstChild; p_obj; p_obj = p_obj->m_nextSibling) {
		FUN_10061c60(p_obj, p_flags);
	}
}

// The size on the screen, in pixels and at least one, of p_size at the distance p_distance.
// FUNCTION: MW2POWERVR 0x10061d60
MechS32 FUN_10061d60(MechFloat p_distance, MechFloat p_size)
{
	MechS32 pixels;

	pixels = (MechS32) (g_gameWindowWidth * 0.5f * p_size / (g_eyepoint->m_detailScale * p_distance));
	if (!pixels) {
		pixels = 1;
	}

	return pixels;
}

// Converts the model p_model of a sprite shape p_shape: each of its triangles is a textured quad
// between the triangle's two corners at u 0, kept above the ground; a camera-facing sprite (type
// 0x30) is a mesh of its own, translated and in an instance of the camera-facing list.
// FUNCTION: MW2POWERVR 0x10061db0
void FUN_10061db0(Shape* p_shape, Model* p_model)
{
	Vertex* vertices;
	Face* face;
	Vertex* vertex;
	MechU8* indices;
	MechS32 ids[4];
	MechS32 foundTop;
	MechS32 foundBottom;
	MechFloat topX;
	MechFloat topY;
	MechFloat topZ;
	MechFloat bottomX;
	MechFloat bottomY;
	MechFloat bottomZ;
	MechFloat halfX;
	MechFloat halfY;
	MechS32 translucent;
	MechS32 lit;
	MechS32 textured;
	MechS32 index;
	MechS32 size;
	MechS32 material;
	MechS32 base;
	MechU16 lastColor;
	MechS32 i;
	MechS32 k;

	foundBottom = FALSE;
	foundTop = FALSE;
	lastColor = 0xfffe;
	if (!p_model) {
		return;
	}

	p_model->m_list = FUN_1005f200(0);
	if ((p_shape->m_kind & 0xf0) != 0x30) {
		FUN_1005f5c0(p_model->m_list);
	}

	vertices = (Vertex*) (p_model + 1);
	base = 0;
	for (i = 0; i < p_model->m_faceCount; i++) {
		face = (Face*) ((MechU8*) p_model + p_model->m_faceOffset) + i;
		if (face->m_indexCount != 3) {
			continue;
		}

		indices = (MechU8*) face + face->m_indexOffset;
		for (k = 0; k < 3; k++) {
			vertex = &vertices[indices[k]];
			if ((MechFloat) fabs(vertex->m_u) < 1e-07f) {
				if ((MechFloat) fabs(vertex->m_v) < 1e-07f) {
					foundBottom = TRUE;
					bottomX = vertex->m_worldX;
					bottomY = vertex->m_worldY;
					bottomZ = vertex->m_worldZ;
				}
				else {
					foundTop = TRUE;
					topX = vertex->m_worldX;
					topY = vertex->m_worldY;
					topZ = vertex->m_worldZ;
				}
			}
		}

		if (!foundBottom || !foundTop) {
			continue;
		}

		halfY = (topY - bottomY) * 0.5f;
		halfX = (topX - bottomX) * 0.5f;
		if (topY < -10.0f) {
			bottomY = -10.0f - topY + bottomY;
			topY = -10.0f;
		}

		if ((p_shape->m_kind & 0xf0) == 0x30) {
			sgl_modify_list(p_model->m_list, FALSE);
			sgl_create_list(FALSE, TRUE, FALSE);
			sgl_create_transform(FALSE);
			sgl_translate(topX * 0.1f, topY * 0.1f, topZ * 0.1f);
			sgl_use_instance(g_a3dFacingList);
			bottomX -= topX;
			bottomY -= topY;
			bottomZ -= topZ;
			topX = 0.0f;
			topY = 0.0f;
			topZ = 0.0f;
			base = 0;
			sgl_create_mesh(FALSE);
		}

		FUN_1005f6a0(bottomX - halfY, bottomY + halfX, bottomZ, 1.0f, 0.0f);
		FUN_1005f6a0(bottomX + halfY, bottomY - halfX, bottomZ, 0.0f, 0.0f);
		FUN_1005f6a0(topX + halfY, topY - halfX, topZ, 0.0f, 1.0f);
		FUN_1005f6a0(topX - halfY, topY + halfX, topZ, 1.0f, 1.0f);
		for (k = 0; k < 4; k++) {
			ids[k] = base++;
		}

		if (face->m_color == lastColor && (p_shape->m_kind & 0xf0) != 0x30) {
			FUN_1005f7f0(4, ids);
		}
		else {
			FUN_100619a0(face->m_color, &translucent, &lit, &textured, &index);
			material = FUN_10061810(index, &size, &size, lit);
			if (material >= 0) {
				FUN_1005f8c0(4, ids, material);
				lastColor = face->m_color;
			}
		}
	}
}

// Converts the shape p_shape: each model a list with a mesh of its vertices and faces (a sprite's
// through FUN_10061db0), the models an object of detail levels or the first model attached to the
// shape's list; then frees the models SGL has.
// FUNCTION: MW2POWERVR 0x10062160
void FUN_10062160(Shape* p_shape, MechS32 p_material)
{
	Model* model;
	Model* level;
	Face* faces;
	Face* face;
	Vertex* vertex;
	MechU8* indices;
	MechS32 ids[16];
	MechS32 models[3];
	MechS32 changePoints[2];
	MechS32 translucent;
	MechS32 levels;
	MechS32 lit;
	MechS32 textured;
	MechS32 index;
	MechS32 width;
	MechS32 height;
	MechFloat red;
	MechFloat green;
	MechFloat blue;
	MechFloat minX;
	MechFloat minY;
	MechFloat minZ;
	MechFloat maxX;
	MechFloat maxY;
	MechFloat maxZ;
	MechFloat extent;
	MechFloat delta;
	MechS32 parent;
	MechU16 lastColor;
	MechS32 i;
	MechS32 k;

	levels = 0;
	if (!g_a3dInitialized || FUN_10062960(p_shape)) {
		return;
	}

	if (p_shape->m_kind & 0x800) {
		g_a3dTerrainObjects++;
	}
	if (p_shape->m_kind & 0x200) {
		g_a3dThingObjects++;
	}
	if (p_shape->m_kind & 0x100) {
		g_a3dPieceObjects++;
	}

	translucent = FALSE;
	p_shape->m_flags |= 0x2000;
	for (model = p_shape->m_models; model; model = model->m_next) {
		if (model->m_vertexCount < 3 || model->m_faceCount < 1) {
			continue;
		}

		faces = (Face*) ((MechU8*) model + model->m_faceOffset);
		if ((faces->m_color & 0xf000) == 0x3000) {
			if (!(p_shape->m_kind & 0xf0)) {
				p_shape->m_kind = (p_shape->m_kind & ~0xf0) | 0x20;
			}
			FUN_10061db0(p_shape, model);
			translucent = TRUE;
			continue;
		}

		model->m_list = FUN_1005f200(0);
		if (g_a3dVerbose) {
			DebugPrint("Rep:%d\n", model->m_list);
		}
		FUN_1005f5c0(model->m_list);
		if (g_a3dVerbose) {
			DebugPrint("Mesh\n");
		}

		lastColor = 0xfffe;
		if (g_a3dVerbose) {
			DebugPrint("Polys:%d\n", model->m_faceCount);
		}

		for (i = 0; i < model->m_faceCount; i++) {
			face = &faces[i];
			if (face->m_indexCount < 2) {
				continue;
			}

			FUN_100619a0(face->m_color, &translucent, &lit, &textured, &index);
			if (textured && FUN_10061810(index, &width, &height, lit) < 0) {
				textured = FALSE;
			}

			indices = (MechU8*) face + face->m_indexOffset;
			for (k = 0; k < face->m_indexCount; k++) {
				vertex = (Vertex*) (model + 1) + indices[k];
				if (textured && A3D_VERTEX_UV_SCALED(vertex) != 1) {
					vertex->m_u = vertex->m_u / width;
					A3D_VERTEX_UV_SCALED(vertex) = 1;
					vertex->m_v = vertex->m_v / height;
				}
				if (lit) {
					A3D_VERTEX_LIT(vertex) = 1;
				}
			}
		}

		if (g_a3dVerbose) {
			DebugPrint("Verts:%d\n", model->m_vertexCount);
		}

		vertex = (Vertex*) (model + 1);
		for (i = 0; i < model->m_vertexCount; i++, vertex++) {
			if (p_shape->m_models == model) {
				if (!i) {
					maxX = minX = vertex->m_worldX;
					maxY = minY = vertex->m_worldY;
					maxZ = minZ = vertex->m_worldZ;
				}
				else {
					if (vertex->m_worldX < minX) {
						minX = vertex->m_worldX;
					}
					if (vertex->m_worldY < minY) {
						minY = vertex->m_worldY;
					}
					if (vertex->m_worldZ < minZ) {
						minZ = vertex->m_worldZ;
					}
					if (vertex->m_worldX > maxX) {
						maxX = vertex->m_worldX;
					}
					if (vertex->m_worldY > maxY) {
						maxY = vertex->m_worldY;
					}
					if (vertex->m_worldZ > maxZ) {
						maxZ = vertex->m_worldZ;
					}
				}
			}

			if (A3D_VERTEX_UV_SCALED(vertex) == 1) {
				if (A3D_VERTEX_LIT(vertex) == 1) {
					FUN_1005f600(
						vertex->m_worldX,
						vertex->m_worldY,
						vertex->m_worldZ,
						vertex->m_modelNormalX,
						vertex->m_modelNormalY,
						vertex->m_modelNormalZ,
						vertex->m_u,
						vertex->m_v
					);
				}
				else {
					FUN_1005f6a0(vertex->m_worldX, vertex->m_worldY, vertex->m_worldZ, vertex->m_u, vertex->m_v);
				}
			}
			else if (A3D_VERTEX_LIT(vertex) == 1) {
				FUN_1005f710(
					vertex->m_worldX,
					vertex->m_worldY,
					vertex->m_worldZ,
					vertex->m_modelNormalX,
					vertex->m_modelNormalY,
					vertex->m_modelNormalZ
				);
			}
			else {
				FUN_1005f790(vertex->m_worldX, vertex->m_worldY, vertex->m_worldZ);
			}
		}

		for (i = 0; i < model->m_faceCount; i++) {
			if (p_shape->m_kind & 0x800) {
				g_a3dTerrainPolys++;
			}
			if (p_shape->m_kind & 0x200) {
				g_a3dThingPolys++;
			}
			if (p_shape->m_kind & 0x100) {
				g_a3dPiecePolys++;
			}

			face = &faces[i];
			if (face->m_indexCount <= 2 || face->m_indexCount >= 0x10) {
				continue;
			}

			indices = (MechU8*) face + face->m_indexOffset;
			for (k = 0; k < face->m_indexCount; k++) {
				ids[k] = indices[k];
			}

			if (lastColor == face->m_color) {
				FUN_1005f7f0(face->m_indexCount, ids);
				continue;
			}

			switch (face->m_color & 0xf000) {
			case 0:
				FUN_10061ba0(face->m_color, &red, &green, &blue);
				FUN_1005f9b0(face->m_indexCount, ids, red, green, blue);
				break;
			case 0x1000:
				FUN_10061c20(face->m_color, &red, &green, &blue);
				FUN_1005f920(face->m_indexCount, ids, red, green, blue);
				break;
			case 0x2000:
				FUN_10061be0(face->m_color, &red, &green, &blue);
				FUN_1005f9b0(face->m_indexCount, ids, red, green, blue);
				break;
			case 0x4000:
				vertex = (Vertex*) (model + 1) + indices[0];
				red = vertex->m_red * 0.00390625f;
				green = vertex->m_green * 0.00390625f;
				blue = vertex->m_blue * 0.00390625f;
				FUN_1005f920(face->m_indexCount, ids, red, green, blue);
				break;
			case 0xf000:
				DebugPrintInternal("MAPSOLID poly\n");
				index = (face->m_color & 0xff) + 0x100;
				FUN_10027620(index, &red, &green, &blue);
				FUN_1005f920(face->m_indexCount, ids, red, green, blue);
				break;
			default:
				FUN_100619a0(face->m_color, &translucent, &lit, &textured, &index);
				index = FUN_10061810(index, &width, &height, lit);
				if (index < 0) {
					continue;
				}
				FUN_1005f8c0(face->m_indexCount, ids, index);
				break;
			}

			lastColor = face->m_color;
		}

		levels++;
	}

	model = p_shape->m_models;
	if (model->m_list <= 0) {
		return;
	}

	if (p_shape->m_object) {
		parent = p_shape->m_object->m_list;
	}
	else {
		parent = FUN_10061910(p_shape);
	}
	p_shape->m_list = FUN_1005f200(parent);
	if (g_a3dVerbose) {
		DebugPrint("Object:%d\n", p_shape->m_list);
	}
	if (translucent) {
		sgl_new_translucent();
	}

	if (levels > 1 && (MechFloat) fabs(model->m_key) >= 1e-07f) {
		extent = maxX - minX;
		delta = maxY - minY;
		if (delta > extent) {
			extent = delta;
		}
		delta = maxZ - minZ;
		if (delta > extent) {
			extent = delta;
		}

		models[0] = model->m_list;
		level = model;
		for (i = levels / 2; i > 0; i--) {
			level = level->m_next;
		}
		models[1] = level->m_list;
		changePoints[0] = FUN_10061d60(level->m_key, extent);
		if (!level->m_next) {
			models[2] = -2;
			changePoints[1] = 1;
		}
		else {
			do {
				level = level->m_next;
			} while (level->m_next);
			models[2] = level->m_list;
			changePoints[1] = FUN_10061d60(level->m_key, extent);
		}

		FUN_1005fa50(p_shape->m_list, models, minX, minY, minZ, maxX, maxY, maxZ, changePoints);
		if (g_a3dVerbose) {
			DebugPrint("Set LODs\n");
		}
	}
	else {
		FUN_1005f320(model->m_list, p_shape->m_list);
		if (g_a3dVerbose) {
			DebugPrint("Attatch Rep:%d to Object:%d\n", model->m_list, p_shape->m_list);
		}
	}

	if (p_shape->m_flags & 0x1000) {
		FUN_1005f3f0(p_shape->m_list);
	}

	FUN_10061750(p_shape);
}

// The shape p_shape is converted already.
// FUNCTION: MW2POWERVR 0x10062960
MechS32 FUN_10062960(Shape* p_shape)
{
	if (p_shape && (p_shape->m_flags & 0x2000)) {
		return TRUE;
	}

	return FALSE;
}

// Converts the object tree p_obj: each object a list in its parent's (or in p_parent, or in its
// shape's kind's list), with a translation and a rotation from its local transform, a material, and
// its shape.
// FUNCTION: MW2POWERVR 0x10062980
void FUN_10062980(SceneObject* p_obj, MechS32 p_parent)
{
	MechScalar angleX;
	MechScalar angleY;
	MechScalar angleZ;

	if (!g_a3dInitialized || !p_obj) {
		return;
	}

	if (p_obj->m_list <= 0) {
		if (p_obj->m_parent) {
			p_obj->m_list = FUN_1005f200(p_obj->m_parent->m_list);
			if (g_a3dVerbose) {
				DebugPrint("Segment:%d  Parent:%d\n", p_obj->m_list, p_obj->m_parent->m_list);
			}
		}
		else if (p_parent >= 0) {
			p_obj->m_list = FUN_1005f200(p_parent);
			if (g_a3dVerbose) {
				DebugPrint("Segment:%d  Parent:%d\n", p_obj->m_list, p_parent);
			}
		}
		else {
			p_obj->m_list = FUN_1005f200(FUN_10061910(p_obj->m_shape));
			if (g_a3dVerbose) {
				DebugPrint("Segment:%d  Parent:%d\n", p_obj->m_list, FUN_10061910(p_obj->m_shape));
			}
		}

		p_obj->m_translation = FUN_1005fb30(p_obj->m_list);
		FUN_1005fb60(
			p_obj->m_translation,
			TRUE,
			p_obj->m_local.m_rows[3][0],
			p_obj->m_local.m_rows[3][1],
			p_obj->m_local.m_rows[3][2]
		);
		p_obj->m_rotation = FUN_1005fb30(p_obj->m_list);
		GetObjAngles(p_obj, &angleX, &angleY, &angleZ);
		FUN_1005fbc0(p_obj->m_rotation, c_a3dRotateYXZ, angleX, angleY, angleZ, TRUE);
		p_obj->m_material = FUN_1005f570();
		if (g_a3dVerbose) {
			DebugPrint("Material:%d\n", p_obj->m_material);
		}
	}

	if (p_obj->m_shape) {
		FUN_10062160(p_obj->m_shape, p_obj->m_material);
	}

	for (p_obj = p_obj->m_firstChild; p_obj; p_obj = p_obj->m_nextSibling) {
		FUN_10062980(p_obj, p_parent);
	}
}

// Converts the object tree of p_obj, from its root.
// FUNCTION: MW2POWERVR 0x10062af0
void FUN_10062af0(SceneObject* p_obj, MechS32 p_parent)
{
	if (!g_a3dInitialized) {
		return;
	}

	while (p_obj->m_parent) {
		p_obj = p_obj->m_parent;
	}
	FUN_10062980(p_obj, p_parent);
}

// Renders a frame: turns the camera-facing sprites to the camera, moves the 3D view's camera to the
// eyepoint, renders cockpit panel 13's view, the 3D view and (the local mech hidden) the other
// panels' views, and scrolls the sky.
// FUNCTION: MW2POWERVR 0x10062b10
void FUN_10062b10(void)
{
	MechS32 first;
	MechS32 last;
	A3DCockpitPanel* panel;
	Mech* mech;
	MechS32 i;

	if (!g_a3dInitialized) {
		return;
	}

	g_a3dFrame++;
	if (g_gridObjectShown) {
		if (IsSatelliteView()) {
			FUN_1005fbc0(g_a3dFacingTransform, c_a3dRotateYXZ, -45.0f, 180.0f, 0.0f, TRUE);
		}
		else {
			FUN_1005fbc0(g_a3dFacingTransform, c_a3dRotateYXZ, 0.0f, g_eyepoint->m_heading, 0.0f, TRUE);
		}
	}

	FUN_1005fb60(
		g_a3dMainView.m_transform,
		TRUE,
		g_eyepoint->m_x,
		g_cockpitLayoutIndex == 4 ? g_eyepoint->m_y * 0.5f : g_eyepoint->m_y,
		g_eyepoint->m_z
	);
	FUN_1005fbc0(
		g_a3dMainView.m_transform,
		c_a3dRotateYXZ,
		g_eyepoint->m_pitch,
		g_eyepoint->m_heading,
		g_eyepoint->m_roll,
		FALSE
	);

	panel = A3D_PANEL(13);
	g_a3dLightsChanged = TRUE;
	mech = g_players[g_localPlayerId]->m_mech;
	if (panel->m_beforeRender) {
		panel->m_beforeRender(panel);
		if (panel->m_redraw) {
			sgl_render(panel->m_view->m_viewport, panel->m_view->m_camera, g_a3dLightsChanged);
			sgl_get_errors(&first, &last);
			if (last) {
				DebugPrintInternal("sgl_render error %d\n", last);
			}

			panel->m_redraw = FALSE;
			g_a3dLightsChanged = FALSE;
			if (panel->m_afterRender) {
				panel->m_afterRender(panel);
			}
		}
	}

	FUN_10060640(&g_a3dMainView);
	if (g_a3dLightsChanged) {
		sgl_render(g_a3dMainView.m_viewport, g_a3dMainView.m_camera, g_a3dLightsChanged);
		g_a3dLightsChanged = FALSE;
	}
	else {
		sgl_render(g_a3dScreenViewport, g_a3dMainView.m_camera, g_a3dLightsChanged);
	}
	sgl_get_errors(&first, &last);
	if (last) {
		DebugPrintInternal("sgl_render error %d\n", last);
	}

	FUN_1005f3f0(mech->m_player->m_obj->m_list);
	for (i = 0; i < c_panelCount; i++) {
		if (i == 13) {
			continue;
		}

		panel = A3D_PANEL(i);
		if (panel->m_beforeRender) {
			panel->m_beforeRender(panel);
			if (panel->m_redraw) {
				sgl_render(panel->m_view->m_viewport, panel->m_view->m_camera, g_a3dLightsChanged);
				sgl_get_errors(&first, &last);
				if (last) {
					DebugPrintInternal("sgl_render error %d\n", last);
				}

				A3D_PANEL(i)->m_redraw = FALSE;
				if (panel->m_afterRender) {
					panel->m_afterRender(panel);
				}
			}
		}
	}
	FUN_1005f450(mech->m_player->m_obj->m_list);

	if (!g_simPaused && g_skyScrollSpeed > 1e-07f && !IsSatelliteView()) {
		g_skyScroll += g_skyScrollSpeed;
		FUN_1005efe0(2000000.0f, 64000000.0f, g_skyTileU, g_skyScroll);
		if ((MechFloat) fabs(fmod(g_skyScroll, g_skyTileU)) < 1e-07f) {
			g_skyScroll = 0.0f;
		}
	}
}

// Closes the renderer: the animations' materials and textures, the scene and SGL.
// FUNCTION: MW2POWERVR 0x10062df0
void FUN_10062df0(void)
{
	Animation* animation;
	AnimFrame* frame;
	MechS32 set;
	MechS32 number;

	for (animation = g_animations; animation < g_animations + 0x200; animation++) {
		if (animation->m_material > 0) {
			sgl_modify_material(animation->m_material, TRUE);
		}
		animation->m_material = -1;
	}

	frame = &g_animFrames[0][0];
	for (set = 0; set < 0x200; set++) {
		for (number = 0; number < 0x20; number++, frame++) {
			if (frame->m_texture > 0) {
				FUN_1005f560(frame->m_texture);
			}
			frame->m_texture = -1;
		}
	}

	if (g_a3dPanel13View.m_root > 0) {
		FUN_1005f280(g_a3dPanel13View.m_root);
	}
	if (g_a3dMainView.m_root > 0) {
		FUN_1005f280(g_a3dMainView.m_root);
	}
	g_a3dPanel13View.m_root = -1;
	g_a3dMainView.m_root = -1;
	FUN_1005f180();
}

// Stand-ins for the entry points the shared client code calls under the ATI edition's names, which
// the PowerVR edition's renderer doesn't have.

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

// Stand-ins for the other units' functions and global the renderer uses, which no unit defines yet
// (the parent's to place): remove each when its unit defines it.

MechS32 g_unk0x101ad7e4 = 1;

void FUN_10027620(MechS32 p_index, MechFloat* p_red, MechFloat* p_green, MechFloat* p_blue)
{
	*p_red = *p_green = *p_blue = 0.0f;
}

sgl_intermediate_map* FUN_10027350(MechS32 p_set, MechS32 p_number, MechU16** p_data, MechS32* p_unk0x0c)
{
	return NULL;
}

MechS32 FUN_100273e0(MechS32 p_index, MechS32* p_width, MechS32* p_height)
{
	return FALSE;
}

void FUN_100259c0(void)
{
}

void FUN_100252a0(MechS32 p_player, MechS32 p_level)
{
}
