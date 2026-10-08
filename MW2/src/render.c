#include "render.h"

#include "animation.h"
#include "cockpit.h"
#include "collision.h"
#include "decomp.h"
#include "depthsort.h"
#include "displaybackend.h"
#include "error.h"
#include "eyepoint.h"
#include "faceshade.h"
#include "fixedmul.h"
#include "geocache.h"
#include "hud.h"
#include "mss.h"
#include "muldiv14.h"
#include "object.h"
#include "objectanim.h"
#include "palette.h"
#include "palettecolor.h"
#include "palidentity.h"
#include "polydraw.h"
#include "recordstacks.h"
#include "refreshmode.h"
#include "screenscale.h"
#include "setres.h"
#include "shape.h"
#include "shapelists.h"
#include "simmain.h"
#include "targeting.h"
#include "types.h"
#include "vfxa.h"
#include "vfxrend.h"
#include "view.h"

#include <string.h>
#include <windows.h>

#ifdef MW2_MATROX
#include "matrox/vfx16.h"
#endif

// The draw mode the game starts in: none in 1.1, the first in the Matrox edition, which doesn't
// fall back to the others (InitRefreshMode's p_allowFallback).
#ifdef MW2_MATROX
#define INITIAL_DRAW_MODE 0
#define INITIAL_DRAW_MODE_FALLBACK 0
#else
#define INITIAL_DRAW_MODE -1
#define INITIAL_DRAW_MODE_FALLBACK 1
#endif

// GLOBAL: MW2 0x100a244c
// GLOBAL: MW2MATROX 0x100a49fc
MechS32 g_drawModeIndex = INITIAL_DRAW_MODE;

// GLOBAL: MW2 0x100a2450
// GLOBAL: MW2MATROX 0x100a4a00
MechS32 g_initDrawModeParam2 = INITIAL_DRAW_MODE_FALLBACK;

// GLOBAL: MW2 0x100a2454
// GLOBAL: MW2MATROX 0x100a4a04
MechS32 g_showBoundingSpheres = 0;

// The banner's file name, instead of sbannr (ShowBanner).
// GLOBAL: MW2 0x100a2458
// GLOBAL: MW2MATROX 0x100a4a08
MechChar* g_bannerName = NULL;

// GLOBAL: MW2 0x100a245c
// GLOBAL: MW2MATROX 0x100a4a0c
void* g_bannerBuffer = NULL;

// GLOBAL: MW2 0x100a2460
// GLOBAL: MW2MATROX 0x100a4a10
MechS32 g_projectionDirty = 1;

// GLOBAL: MW2 0x100a2464
// GLOBAL: MW2MATROX 0x100a4a14
MechS32 g_displayReady = 0;

// GLOBAL: MW2 0x100a2468
// GLOBAL: MW2MATROX 0x100a4a18
MechS32 g_framePane = 0;

// GLOBAL: MW2 0x100a246c
// GLOBAL: MW2MATROX 0x100a4a1c
MechS32 g_hasLightObject = 0;

// Cleared while an effect has the camera, set again when it gives it back.
// GLOBAL: MW2 0x100a2470
// GLOBAL: MW2MATROX 0x100a4a20
MechS32 g_lightFollowsObject = 1;

// GLOBAL: MW2 0x100a2474
// GLOBAL: MW2MATROX 0x100a4a24
MechS32 g_lightObject = -1;

// The object of the scene's shape of kind 0x90, made by SecondRender.
// GLOBAL: MW2 0x100a2478
// GLOBAL: MW2MATROX 0x100a4a28
SceneObject* g_skyObject = NULL;

// The object of the scene's shape of kind 0xa0.
// GLOBAL: MW2 0x100a247c
// GLOBAL: MW2MATROX 0x100a4a2c
SceneObject* g_cockpitObject = NULL;

// GLOBAL: MW2 0x100a2480
// GLOBAL: MW2MATROX 0x100a4a30
MechS32 g_drawnPolygonCount = 0;

// GLOBAL: MW2 0x100bdff8
// GLOBAL: MW2MATROX 0x100c1e28
PANE g_screenPane;

// GLOBAL: MW2 0x10176eb4
// GLOBAL: MW2MATROX 0x10212da8
GameWindowGeometry* g_gameWindowGeometry;

// GLOBAL: MW2 0x10176eb8
// GLOBAL: MW2MATROX 0x10212dac
MechS32 g_screenHeight;

// GLOBAL: MW2 0x10176eb0
// GLOBAL: MW2MATROX 0x10212d9c
undefined4 g_unk0x10176eb0;

// Set when the next Blit should stretch the current pane over the window.
// GLOBAL: MW2 0x10176ebc
// GLOBAL: MW2MATROX 0x10212da4
MechS32 g_stretchPending;

// GLOBAL: MW2 0x10176ec0
// GLOBAL: MW2MATROX 0x10212da0
MechS32 g_screenHeightMinus1;

// GLOBAL: MW2 0x10176ec4
// GLOBAL: MW2MATROX 0x10212d50
MechS32 g_screenPixelCount;

// GLOBAL: MW2 0x10176ec8
// GLOBAL: MW2MATROX 0x10212d74
MechS32 g_screenWidth;

// GLOBAL: MW2 0x10176ed0
// GLOBAL: MW2MATROX 0x10212d80
PANE g_currentPane;

// GLOBAL: MW2 0x10176ee4
// GLOBAL: MW2MATROX 0x10212d54
MechS32 g_screenWidthMinus1;

// GLOBAL: MW2 0x10176ee8
// GLOBAL: MW2MATROX 0x10212d94
MechS32 g_screenHalfWidth;

// GLOBAL: MW2 0x10176eec
// GLOBAL: MW2MATROX 0x10212d98
MechS32 g_screenHalfHeight;

// GLOBAL: MW2 0x10176ef0
// GLOBAL: MW2MATROX 0x10212d60
WINDOW g_mainPixelBuffer;

// FUNCTION: MW2 0x10012720
// FUNCTION: MW2MATROX 0x10017370
MechS32 InitGameWindowGeometry(void)
{
	g_gameWindowGeometry = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(GameWindowGeometry));
	if (g_gameWindowGeometry == NULL) {
		return 0;
	}

	g_gameWindowGeometry->m_width = g_gameWindowWidth;
	g_gameWindowGeometry->m_height = g_gameWindowHeight;
	g_gameWindowGeometry->m_unk0x08 = 1;
	g_gameWindowGeometry->m_numColors = 0x100;
	g_gameWindowGeometry->m_unk0x10 = 1;
	g_gameWindowGeometry->m_unk0x14 = 0;
	g_screenPixelCount = g_gameWindowHeight * g_gameWindowWidth;
	g_screenWidth = g_gameWindowWidth;
	g_screenHeight = g_gameWindowHeight;
	g_screenWidthMinus1 = g_gameWindowWidth - 1;
	g_screenHeightMinus1 = g_gameWindowHeight - 1;
	g_screenHalfWidth = g_screenWidth / 2;
	g_screenHalfHeight = g_screenHeight / 2;
	return 1;
}

// FUNCTION: MW2 0x10012802
// FUNCTION: MW2MATROX 0x10017452
MechS32 InitDisplayGeometry(void)
{
	MechS32 result;

	result = 0;
	if (InitGameWindowGeometry()) {
		ChooseArtResolution(g_gameWindowGeometry);
		SetPixelAspect(g_gameWindowGeometry);
		g_currentPane.m_window = &g_mainPixelBuffer;
		g_currentPane.m_x0 = 0;
		g_currentPane.m_y0 = 0;
		g_currentPane.m_x1 = g_gameWindowGeometry->m_width - 1;
		g_currentPane.m_y1 = g_gameWindowGeometry->m_height - 1;
		g_screenPane = g_currentPane;
		result = 1;
		InitPanes(&g_currentPane);
		ResetTextColors();
		g_displayReady = 1;
	}

	return result;
}

// Makes the code block writable (the drawing routines patch themselves), sets up the vertex
// buffers and the scene, and installs the normal render hooks.
// Stack-slot permutation: size, start, segment and oldProtect.
// FUNCTION: MW2 0x100128b5
// FUNCTION: MW2MATROX 0x10017505
void FirstRender(void)
{
	MechS32 segment;
	MechS32 size;
	DWORD oldProtect;
	undefined4 start;

	size = 0;
	start = 0;
	segment = 0;
	oldProtect = 0;
	size = GetCodeBlock(&start, &segment);
	if (!VirtualProtect(
			(void*) start,
			size,
			(GetVersion() & 0x80000000) ? PAGE_READWRITE : PAGE_EXECUTE_READWRITE,
			&oldProtect
		)) {
		Error(0x4d, NULL);
	}

#ifdef MW2_MATROX
	InitializeDrawBuffer(0x140, 0xbb8);
	g_maxPolygons = 0xaf0;
#else
	InitializeDrawBuffer(0x80, 0x5dc);
	g_maxPolygons = 0x578;
#endif
	InitShapeLists();
	g_renderSettings.m_frameDrawCallback = DrawScene;
	g_renderSettings.m_shapeFilter = CullSceneShape;
	g_renderSettings.m_projectVertex = ProjectVertex;
	g_renderSettings.m_drawFace = (MechS32 (*)()) GetFaceColor;
	g_renderSettings.m_drawPolygon = DrawScenePolygon;
	g_unk0x100a5558 = 0xff;
	if (g_renderSettings.m_drawSky || g_renderSettings.m_drawGround) {
		g_renderSettings.m_clearFrame = 0;
	}

	g_renderSettings.m_flags |= 8;
}

// Makes the objects of the scene's shapes of kinds 0x90 and 0xa0, and sets up its shapes of kind
// 0x70 and type 4.
// FUNCTION: MW2 0x100129b7
// FUNCTION: MW2MATROX 0x10017607
void SecondRender(void)
{
	Shape* root;
	Shape* shape;
	Shape* next;

	root = g_sceneShapes;
	if (!root) {
		return;
	}

	for (shape = root->m_next; shape; shape = shape->m_next) {
		if ((shape->m_kind & 0xf0) == 0x90) {
			g_skyObject = GetShapeObject(shape);
			DetachObjTreeShapes(g_skyObject);
			break;
		}
	}

	for (shape = root->m_next; shape; shape = shape->m_next) {
		if ((shape->m_kind & 0xf0) == 0xa0) {
			g_cockpitObject = GetShapeObject(shape);
			break;
		}
	}

	for (shape = root->m_next; shape; shape = next) {
		next = shape->m_next;
		if ((shape->m_kind & 0xf0) == 0x70) {
			HideShape(shape);
			DisableShapeCollision(shape);
		}

		if (shape->m_collisionType == 4) {
			DisableShapeCollision(shape);
		}
	}
}

// The 3D view's pane wipe: the Matrox edition's goes through its A3D layer, with a 16-bit pixel.
#ifdef MW2_MATROX
#define SCENE_PANE_WIPE(p_pane, p_color) FUN_1005f8a0(p_pane, PIXEL_COLOR(p_color))
#else
#define SCENE_PANE_WIPE(p_pane, p_color) VFX_pane_wipe(p_pane, p_color)
#endif

// Draws the 3D view, the normal frame draw callback: clears the frame first when drawing to
// another pane, updates the eyepoint, draws the scene (without the extra pass on the
// DirectDraw backend), then the objects of the shapes of kinds 0x90 and 0xa0 with their own clip
// distances, the scene's objects, and the animations.
// MW2MATROX: wipes with the background's 16-bit pixel, keeps the clip distance it restores as a
// float, and doesn't draw the bounding spheres.
// FUNCTION: MW2 0x10012afe
// FUNCTION: MW2MATROX 0x1001774e
void DrawScene(void)
{
	MechScalar saved;
	MechS32 pass;

	if (g_framePane) {
		memset(g_mainPixelBuffer.m_buffer, g_backgroundColor, g_refreshModePixelCount);
		SelectPane(g_framePane);
	}

	if (g_projectionDirty) {
		UpdateProjection(g_eyepoint);
		g_projectionDirty = 0;
	}

	if (g_renderSettings.m_blankScene) {
		SCENE_PANE_WIPE(&g_currentPane, g_backgroundColor);
		return;
	}

	UpdateViewMatrix(g_eyepoint);
	SelectEyepoint(g_eyepoint);
	if (g_renderSettings.m_clearFrame || g_renderSettings.m_wireframe) {
		SCENE_PANE_WIPE(&g_currentPane, g_backgroundColor);
	}
	else if (g_renderSettings.m_drawSky || g_renderSettings.m_drawGround) {
		if (g_currentDisplayBackend->m_id == c_displayBackendDirectDraw) {
			pass = g_renderSettings.m_drawGround;
			g_renderSettings.m_drawGround = 0;
			DrawSkyAndGround(g_eyepoint);
			g_renderSettings.m_drawGround = pass;
		}
		else {
			DrawSkyAndGround(g_eyepoint);
		}
	}

	if (g_hasLightObject && g_lightFollowsObject && g_lightObject != -1) {
		GetStaticObjectPosition(g_lightObject, &g_eyepoint->m_lightX, &g_eyepoint->m_lightY, &g_eyepoint->m_lightZ);
	}

	g_drawnPolygonCount = 0;
	if (g_skyObject) {
		saved = g_eyepoint->m_farPlane;
		SetFarPlane(g_eyepoint, 0x7fffffff);
		g_renderSettings.m_shapeFilter = CullShapeToFrustum;
		DrawObjTreeShapes(g_skyObject);
		g_drawnPolygonCount += g_depthEntryCount;
		SetFarPlane(g_eyepoint, saved);
		g_renderSettings.m_shapeFilter = CullSceneShape;
	}

	DrawShapeList(g_sceneShapes);
	g_drawnPolygonCount += g_depthEntryCount;
	if (g_inCockpitView && g_cockpitObject) {
		saved = g_eyepoint->m_nearPlane;
		SetNearPlane(g_eyepoint, 8);
		g_renderSettings.m_shapeFilter = CullHiddenShape;
		DrawObjTreeShapes(g_cockpitObject);
		g_drawnPolygonCount += g_depthEntryCount;
		SetNearPlane(g_eyepoint, saved);
		g_renderSettings.m_shapeFilter = CullSceneShape;
	}

#ifndef MW2_MATROX
	if (g_showBoundingSpheres) {
		DrawBoundingSpheres(g_sceneShapes);
	}
#endif

	FUN_10069591();
	SelectPane(0);
}

// FUNCTION: MW2 0x10012dca
// FUNCTION: MW2MATROX 0x10017a14
void SetFramePane(MechS32 p_value)
{
	if (p_value >= 0 && p_value < 11) {
		g_framePane = p_value;
	}
	else {
		g_framePane = 0;
	}
}

// FUNCTION: MW2 0x10012e00
// FUNCTION: MW2MATROX 0x10017a4a
void ResetPane(void)
{
	SelectPane(0);
}

// Presents the frame, or stretches the current pane over the window when a stretch is
// pending, restoring the pane afterwards.
// FUNCTION: MW2 0x10012e15
// FUNCTION: MW2MATROX 0x10017a5f
void Blit(void)
{
	if (g_stretchPending) {
		g_currentRefreshMode
			->m_stretchBlit(g_currentPane.m_x0 + 1, g_currentPane.m_y0 + 1, g_currentPane.m_x1, g_currentPane.m_y1);
		g_currentPane = g_screenPane;
		g_showHud = g_savedShowHud;
		g_stretchPending = 0;
	}
	else if (g_windowActive) {
		g_currentRefreshMode->m_flip();
	}
}

// MW2MATROX: wipes with a 16-bit pixel, through the Matrox edition's own (16-bit) VFX_pane_wipe.
// FUNCTION: MW2 0x10012e91
// FUNCTION: MW2MATROX 0x10017adb
void ShutdownRender(void)
{
	FreeSceneShapes();
	ShutdownDrawBuffer();
	if (g_bannerBuffer && g_currentPane.m_window) {
		VFX_pane_wipe(&g_currentPane, PIXEL_COLOR(0));
		if (g_windowActive) {
			g_currentRefreshMode->m_flip();
		}
	}

	if (g_bannerBuffer) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_bannerBuffer);
	}

	g_displayReady = 0;
	ShutdownRefreshMode();
}

// FUNCTION: MW2 0x10012f14
// FUNCTION: MW2MATROX 0x10017b67
undefined4 FUN_10012f14(void)
{
	return g_unk0x10176eb0;
}

// FUNCTION: MW2 0x10012f29
// FUNCTION: MW2MATROX 0x10017b7c
void FUN_10012f29(undefined4 p_unk0x00, undefined4 p_value)
{
	g_unk0x10176eb0 = p_value;
}

// Shows the banner GIF (sbannr, or g_bannerName's name, with the art resolution's suffix) and
// fades its palette in. Nothing calls it.
// Stack-slot permutation of the locals. The Matrox edition's takes the suffix from the art
// resolution before its 512x384 override (g_unk0x1012c1e4).
// FUNCTION: MW2 0x10012f3c
// FUNCTION: MW2MATROX 0x10017b8f
void ShowBanner(void)
{
	void* gif;
	PANE target;
	MechU8* state;
	MechChar path[256];
	PaletteColor* palette;

	if (g_bannerName == NULL || *g_bannerName == '\0') {
		strcpy(path, "sbannr");
#ifdef MW2_MATROX
		strcat(path, g_artResolutionSuffixes[g_unk0x1012c1e4]);
#else
		strcat(path, g_artResolutionSuffixes[g_artResolution]);
#endif
		strcat(path, ".");
		strcat(path, "gif");
	}
	else {
		strcpy(path, g_bannerName);
#ifdef MW2_MATROX
		strcat(path, g_artResolutionSuffixes[g_unk0x1012c1e4]);
#else
		strcat(path, g_artResolutionSuffixes[g_artResolution]);
#endif
		strcat(path, ".");
		strcat(path, "gif");
	}

	gif = FILE_read(path, NULL);
	if (gif) {
		state = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, 0x502e);
		if (state) {
			palette = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 0x100 * sizeof(PaletteColor));
			if (palette) {
				g_currentDisplayBackend->m_setPalette(0, 0x100, palette, 1);
				target = g_currentPane;
				FitRectToGif(&target, &target, gif);
				if ((g_windowActive ? g_currentDisplayBackend->m_acquireFramebuffer() : -1) == 0) {
					VFX_GIF_draw(&target, gif, state);
					if (g_windowActive) {
						g_currentRefreshMode->m_flip();
					}
				}

				VFX_GIF_palette(gif, (MechU8*) palette);
				g_currentDisplayBackend->m_blendPalettes(palette, 30);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, palette);
			}

			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, state);
		}

		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, gif);
	}
}

// Draws the bounding spheres (the "michelin" cheat) of the list p_root's colliding mech shapes
// (kind 0x100 without flag 0x800) and of kind 0x50, as circles of their radius in color 0xf.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100131f1
void DrawBoundingSpheres(Shape* p_root)
{
	MechS32 color;
	Shape* shape;
	MechS32 x;
	MechS32 y;
	MechS32 z;
	MechS32 radius;
	MechS32 radiusY;

	if (!p_root || !p_root->m_next || p_root->m_prev == p_root->m_next) {
		return;
	}

	for (shape = p_root->m_next; shape; shape = shape->m_next) {
		if ((((shape->m_kind & 0x100) && !(shape->m_flags & 0x800)) || (shape->m_kind & 0xf0) == 0x50) &&
			!g_renderSettings.m_shapeFilter(shape)) {
			x = shape->m_centerX;
			y = shape->m_centerY;
			z = shape->m_centerZ;
			radius = shape->m_radius;
			color = 0xf;
			if (ProjectWorldPoint(&x, &y, &z)) {
				radius = ProjectRadius(g_eyepoint->m_projectScaleX, radius, z);
				radiusY = FixedMul16(radius, g_eyepoint->m_pixelAspect);
				VFX_ellipse_draw(&g_currentPane, x, y, radius, radiusY, color);
			}
		}
	}
}

#ifdef MW2_MATROX
// STUB: MW2MATROX 0x1005f8a0
void FUN_1005f8a0(PANE* p_pane, MechS32 p_color)
{
	STUB(0x1005f8a0);
}
#endif
