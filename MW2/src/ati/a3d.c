/* The ATI 3D Rage edition's A3D renderer (accel/a3d.h): the game's polygons drawn through ATI's
   3D CIF (ati3dcif.dll), one object of its own after accel/graphicsmenu.c's, compiled like the
   edition's other units. The CIF draws into the main pixel buffer, a context state at a time: each
   state the renderer sets is cached here, and set again only when it changes. A polygon is drawn
   as a list of triangles fanned out of its first vertex. Its textures are VFX WINDOWs of 16-bit
   texels (RGB555), copied into DirectDraw surfaces that the CIF registers: up to 400, kept in a
   list per size (16 to 2048 texels square, by the log of the size), the most recently created
   first. A texture is looked up by its pixels; when its size has no free surface, it takes the
   least recently used one's, and every texture is copied again after a palette change. A
   mip-mapped texture's surface is 128 texels wide, its levels one under the other, each averaged
   from the one above. */
#include "accel/a3d.h"

#include "accel/a3dvertex.h"
#include "compat.h"
#include "debugprint.h"
#include "decomp.h"
#include "directdraw.h"
#include "palettecolor.h"
#include "pane.h"
#include "polydraw.h"
#include "render.h"
#include "types.h"
#include "window.h"

#include <ati3dcif.h>
#include <ddraw.h>
#include <string.h>

// A texture: a DirectDraw surface of its texels, registered with the CIF. Its levels are the
// surface's, from m_desc.lpSurface.
// SIZE 0xc4
typedef struct A3DCachedTexture A3DCachedTexture;
struct A3DCachedTexture {
	MechU16* m_pixels;             // 0x00 — the texels it holds, which identify it
	LPDIRECTDRAWSURFACE m_surface; // 0x04
	DDSURFACEDESC m_desc;          // 0x08 — as the last lock left it
	C3D_TMAP m_tmap;               // 0x74
	C3D_HTX m_handle;              // 0xb8
	MechU32 m_lastUsed;            // 0xbc — g_a3dClock when it was last drawn
	A3DCachedTexture* m_next;      // 0xc0 — in its size's list
};

DECOMP_SIZE_ASSERT(A3DCachedTexture, 0xc4)

// The largest texture size, by its log, plus one.
#define A3D_SIZES 12

// The CIF's context.
// GLOBAL: MW2ATI 0x101999e8
C3D_HRC g_a3dContext = 0;

// The CIF is rendering (ATI3DCIF_RenderBegin).
// GLOBAL: MW2ATI 0x101999ec
MechS32 g_a3dRendering = FALSE;

// The main pixel buffer the CIF was last pointed at.
// GLOBAL: MW2ATI 0x101999f0
undefined* g_a3dSurface = NULL;

// The CIF is initialized (A3D_Init).
// GLOBAL: MW2ATI 0x101999f4
MechS32 g_a3dInitialized = FALSE;

// The context's states, as last set.
// GLOBAL: MW2ATI 0x101999f8
MechU32 g_a3dPrimType = C3D_EPRIM_LINE;

// GLOBAL: MW2ATI 0x101999fc
MechU32 g_a3dShadeMode = C3D_ESH_FLAT;

// GLOBAL: MW2ATI 0x10199a00
MechU32 g_a3dPerspective = C3D_ETPC_NONE;

// GLOBAL: MW2ATI 0x10199a04
MechU32 g_a3dLight = C3D_ETL_NONE;

// GLOBAL: MW2ATI 0x10199a08
MechU32 g_a3dFilter = C3D_ETFILT_MIPLIN_MAG2BY2;

// GLOBAL: MW2ATI 0x10199a0c
MechU32 g_a3dTexOp = C3D_ETEXOP_ALPHA_MASK;

// GLOBAL: MW2ATI 0x10199a10
C3D_HTX g_a3dTexture = 0;

// GLOBAL: MW2ATI 0x10199a14
MechU32 g_a3dTextureEnable = FALSE;

// Statistics: the textures found cached, the textures copied, and the bytes copied.
// GLOBAL: MW2ATI 0x10199a18
MechU32 g_a3dTextureHits = 0;

// GLOBAL: MW2ATI 0x10199a1c
MechU32 g_a3dTextureFills = 0;

// GLOBAL: MW2ATI 0x10199a20
MechU32 g_a3dTextureBytes = 0;

// The "Filter" setting (FUN_100648a0).
// GLOBAL: MW2ATI 0x10199a24
MechS32 g_a3dFilterSetting = 0;

// The textures in use in g_a3dTextures.
// GLOBAL: MW2ATI 0x10199a28
MechU32 g_a3dTextureCount = 0;

// GLOBAL: MW2ATI 0x10199a2c
MechS32 g_unk0x10199a2c = 1;

// A clock that ticks at every texture drawn, and its time at the last palette change.
// GLOBAL: MW2ATI 0x10199a30
MechU32 g_a3dClock = 1;

// GLOBAL: MW2ATI 0x10199a34
MechU32 g_a3dPaletteClock = 0;

// The textures' chroma key.
// GLOBAL: MW2ATI 0x10199a38
C3D_COLOR g_a3dChromaKey = {0xff, 0xff, 0xff, 0};

// The sizes, by their log, from which no surface can be created.
// GLOBAL: MW2ATI 0x10199a3c
MechU32 g_a3dSizeLimit = A3D_SIZES;

// The textures are created (FUN_10064720).
// GLOBAL: MW2ATI 0x10199a40
MechS32 g_a3dTexturesCreated = FALSE;

// GLOBAL: MW2ATI 0x10072e00
A3DCachedTexture g_a3dTextures[400];

// The textures of each size: at most 100.
// GLOBAL: MW2ATI 0x10086040
MechU32 g_a3dSizeTextures[A3D_SIZES];

// The viewport, the pane drawn in.
// GLOBAL: MW2ATI 0x10086070
C3D_RECT g_a3dViewport;

// The triangle lists of the two vertex arrays (g_a3dVertices and g_a3dClipped), made by
// FUN_10063eb0 at their first use in a frame: three pointers to each triangle fanned out of the
// first vertex.
// GLOBAL: MW2ATI 0x10086080
A3DVertex* g_a3dTriangles[0x100];

// GLOBAL: MW2ATI 0x10086480
A3DVertex* g_a3dTriangles2[0x100];

// The lists of the textures of each size, and their lengths.
// GLOBAL: MW2ATI 0x10087480
A3DCachedTexture* g_a3dTextureLists[A3D_SIZES];

// The original has room for 11 lengths before the next global.
// GLOBAL: MW2ATI 0x100874b0
MechU32 g_a3dTextureListLengths[A3D_SIZES - 1];

void FUN_10063d10(void);
void FUN_10064710(void);

// "A3D_Init()": initializes the CIF, creates its context and sets its first states. Returns TRUE
// on success.
// FUNCTION: MW2ATI 0x10063af0
MechS32 A3D_Init(void)
{
	C3D_EC result;
	MechU32 vertexType;
	MechU32 alphaSource;
	MechU32 alphaDestination;

	alphaDestination = C3D_EADST_ZERO;
	alphaSource = C3D_EASRC_ONE;
	vertexType = C3D_EV_VTCF;
	result = ATI3DCIF_Init();
	if (result != C3D_EC_OK) {
		DebugPrint("A3D: Init returned %d\n", result);
		return FALSE;
	}

	g_a3dContext = ATI3DCIF_ContextCreate();
	if (!g_a3dContext) {
		DebugPrint("A3D: Create context returned %d\n", result);
		return FALSE;
	}

	g_a3dInitialized = TRUE;
	ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_ALPHA_DST, &alphaDestination);
	ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_ALPHA_SRC, &alphaSource);
	ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_VERTEX, &vertexType);
	if (g_a3dPrimType != C3D_EPRIM_TRI) {
		g_a3dPrimType = C3D_EPRIM_TRI;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_PRIM_TYPE, &g_a3dPrimType);
	}

	if (g_a3dFilter != C3D_ETFILT_MIPLIN_MAG2BY2) {
		g_a3dFilter = C3D_ETFILT_MIPLIN_MAG2BY2;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_FILTER, &g_a3dFilter);
	}

	if (g_a3dTextureEnable != TRUE) {
		g_a3dTextureEnable = TRUE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_EN, &g_a3dTextureEnable);
	}

	if (g_a3dTexOp != C3D_ETEXOP_ALPHA_MASK) {
		g_a3dTexOp = C3D_ETEXOP_ALPHA_MASK;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_TEXOP, &g_a3dTexOp);
	}

	FUN_100648a0(g_a3dFilterSetting);
	return TRUE;
}

// Points the CIF at the main pixel buffer (RGB555, the whole screen the viewport), initializing it
// first if it isn't, forgets the triangle lists and starts rendering. Returns FALSE if the CIF
// can't be initialized.
// FUNCTION: MW2ATI 0x10063c40
MechS32 FUN_10063c40(void)
{
	MechU32 pixelFormat;
	MechU32 pitch;

	pixelFormat = C3D_EPF_RGB1555;
	if (!g_a3dInitialized && !A3D_Init()) {
		return FALSE;
	}

	ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_SURF_DRAW_PTR, &g_mainPixelBuffer);
	pitch = (g_mainPixelBuffer.m_xMax + 1) >> 1;
	ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_SURF_DRAW_PITCH, &pitch);
	ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_SURF_DRAW_PF, &pixelFormat);
	g_a3dViewport.bottom = 0;
	g_a3dViewport.left = 0;
	g_a3dViewport.top = 479;
	g_a3dViewport.right = 639;
	if (ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_SURF_VPORT, &g_a3dViewport) != C3D_EC_OK) {
		DebugPrint("A3D: set clipping rect failed\n");
	}

	g_a3dTriangles[0] = NULL;
	g_a3dTriangles2[0] = NULL;
	if (!g_a3dRendering) {
		FUN_10063d10();
	}

	return TRUE;
}

// Starts the CIF's rendering.
// FUNCTION: MW2ATI 0x10063d10
void FUN_10063d10(void)
{
	if (ATI3DCIF_RenderBegin(g_a3dContext) == C3D_EC_OK) {
		g_a3dRendering = TRUE;
	}
}

// FUNCTION: MW2ATI 0x10063d30
void FUN_10063d30(void)
{
	if (ATI3DCIF_RenderEnd() == C3D_EC_OK) {
		g_a3dRendering = FALSE;
	}
}

// Sets g_paletteRgb, the palette's components times 4, and has every texture copied again (their
// texels are palette colors).
// FUNCTION: MW2ATI 0x10063d50
void A3D_SetPalette(PaletteColor* p_palette)
{
	MechFloat* color;

	for (color = g_paletteRgb[0]; color < g_paletteRgb[0x100]; color += 3) {
		color[0] = p_palette->m_red * 4;
		color[1] = p_palette->m_green * 4;
		color[2] = p_palette->m_blue * 4;
		p_palette++;
	}

	FUN_10064710();
}

// Prepares a polygon of p_count vertices: points the CIF at the main pixel buffer if it has moved.
// Returns FALSE if the CIF can't draw it.
// FUNCTION: MW2ATI 0x10063db0
MechS32 FUN_10063db0(MechU32 p_count)
{
	if (g_mainPixelBuffer.m_buffer != g_a3dSurface) {
		if (!FUN_10063c40()) {
			return FALSE;
		}

		g_a3dSurface = g_mainPixelBuffer.m_buffer;
	}

	if (p_count > 0xff) {
		DebugPrint("A3D:  Not enough vertices\n");
		return FALSE;
	}

	return TRUE;
}

// Reports a CIF error; returns TRUE for none.
// FUNCTION: MW2ATI 0x10063e00
MechS32 FUN_10063e00(C3D_EC p_result)
{
	if (p_result != C3D_EC_OK) {
		DebugPrint("A3D: Error %d\n", p_result);
		return FALSE;
	}

	return TRUE;
}

// Sets the viewport to the rectangle p_left-p_bottom, clipped to the screen.
// FUNCTION: MW2ATI 0x10063e20
void FUN_10063e20(MechS32 p_left, MechS32 p_right, MechS32 p_top, MechS32 p_bottom)
{
	if (p_left < 0) {
		p_left = 0;
	}

	if (p_right > 639) {
		p_right = 639;
	}

	if (p_top < 0) {
		p_top = 0;
	}

	if (p_bottom > 479) {
		p_bottom = 479;
	}

	if (g_a3dViewport.left != p_left || g_a3dViewport.right != p_right || g_a3dViewport.top != p_top ||
		g_a3dViewport.bottom != p_bottom) {
		g_a3dViewport.left = p_left;
		g_a3dViewport.right = p_right;
		g_a3dViewport.top = p_top;
		g_a3dViewport.bottom = p_bottom;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_SURF_VPORT, &g_a3dViewport);
	}
}

// Returns the triangle list of the vertex array p_vertices: the first vertex, then each pair of
// the next ones, for nine triangles.
// FUNCTION: MW2ATI 0x10063eb0
A3DVertex** FUN_10063eb0(A3DVertex* p_vertices)
{
	MechS32 i;
	MechS32 n;

	if (g_a3dTriangles[0] == p_vertices) {
		return g_a3dTriangles;
	}

	if (g_a3dTriangles2[0] == p_vertices) {
		return g_a3dTriangles2;
	}

	if (!g_a3dTriangles[0]) {
		for (i = 0, n = 1; n < 10; n++) {
			g_a3dTriangles[i++] = p_vertices;
			g_a3dTriangles[i++] = &p_vertices[n];
			g_a3dTriangles[i++] = &p_vertices[n + 1];
		}

		return g_a3dTriangles;
	}

	if (!g_a3dTriangles2[0]) {
		for (i = 0, n = 1; n < 10; n++) {
			g_a3dTriangles2[i++] = p_vertices;
			g_a3dTriangles2[i++] = &p_vertices[n];
			g_a3dTriangles2[i++] = &p_vertices[n + 1];
		}

		return g_a3dTriangles2;
	}

	return g_a3dTriangles2;
}

// Returns the log of p_size, rounded down.
// FUNCTION: MW2ATI 0x10063f60
MechU32 FUN_10063f60(MechU32 p_size)
{
	MechU32 log;

	log = 0;
	if (p_size & ~0xf) {
		p_size >>= 4;
		log = 4;
	}

	while (p_size >>= 1) {
		log++;
	}

	return log;
}

// Returns the next free texture, cleared, or NULL if there is none (FUN_10063fc0 takes it).
// FUNCTION: MW2ATI 0x10063f80
A3DCachedTexture* FUN_10063f80(void)
{
	if (g_a3dTextureCount < 400) {
		memset(&g_a3dTextures[g_a3dTextureCount], 0, sizeof(A3DCachedTexture));
		return &g_a3dTextures[g_a3dTextureCount];
	}

	return NULL;
}

// FUNCTION: MW2ATI 0x10063fc0
void FUN_10063fc0(void)
{
	g_a3dTextureCount++;
}

// Creates p_texture's surface, 1 << p_log texels square, or with p_mip 128 texels wide and as high
// as its levels; returns FALSE on failure.
// FUNCTION: MW2ATI 0x10063fd0
MechS32 FUN_10063fd0(A3DCachedTexture* p_texture, MechU32 p_log, MechS32 p_mip)
{
	LPDDSURFACEDESC desc;
	LPDIRECTDRAWSURFACE surface;
	HRESULT result;
	MechU32 height;
	MechU32 width;
	MechU32 level;

	desc = &p_texture->m_desc;
	if (p_mip) {
		height = 0;
		for (level = 1 << p_log; level; level >>= 2) {
			height += level;
		}

		height++;
		width = 0x80;
	}
	else {
		height = 1 << p_log;
		width = height;
	}

	memset(desc, 0, sizeof(DDSURFACEDESC));
	desc->dwSize = sizeof(DDSURFACEDESC);
	desc->dwWidth = width;
	desc->dwHeight = height;
	desc->dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
	desc->ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_OFFSCREENPLAIN;
	if (IDirectDraw_CreateSurface(g_ddraw, desc, &p_texture->m_surface, NULL) != DD_OK) {
		return FALSE;
	}

	surface = p_texture->m_surface;
	memset(desc, 0, sizeof(DDSURFACEDESC));
	desc->dwSize = sizeof(DDSURFACEDESC);
	result = IDirectDrawSurface_Lock(surface, NULL, desc, DDLOCK_WAIT, NULL);
	if (result == DDERR_SURFACELOST) {
		IDirectDrawSurface_Restore(surface);
		result = IDirectDrawSurface_Lock(surface, NULL, desc, DDLOCK_WAIT, NULL);
	}

	IDirectDrawSurface_Unlock(surface, NULL);
	if (result != DD_OK) {
		DebugPrint("error: unlock surface\n");
		IDirectDrawSurface_Release(surface);
		return FALSE;
	}

	return TRUE;
}

// Releases p_texture's surface.
// FUNCTION: MW2ATI 0x100640d0
void FUN_100640d0(A3DCachedTexture* p_texture)
{
	IDirectDrawSurface_Release(p_texture->m_surface);
}

// Registers p_texture with the CIF, 1 << p_log texels square, with its levels when p_mip; returns
// FALSE on failure.
// FUNCTION: MW2ATI 0x100640e0
MechS32 FUN_100640e0(A3DCachedTexture* p_texture, MechU32 p_log, MechS32 p_mip)
{
	C3D_EC result;
	MechU16* pixels;
	MechU32 i;
	MechU32 level;

	memset(&p_texture->m_tmap, 0, sizeof(C3D_TMAP));
	p_texture->m_tmap.u32MaxMapXSizeLg2 = p_log;
	p_texture->m_tmap.u32Size = sizeof(C3D_TMAP);
	p_texture->m_tmap.u32MaxMapYSizeLg2 = p_log;
	p_texture->m_tmap.bMipMap = p_mip != 0;
	p_texture->m_tmap.clrTexChromaKey = g_a3dChromaKey;
	pixels = (MechU16*) p_texture->m_desc.lpSurface;
	p_texture->m_tmap.eTexFormat = C3D_ETF_RGB1555;
	if (p_mip) {
		for (level = p_log; (MechS32) level > 0; level--) {
			p_texture->m_tmap.apvLevels[p_log - level] = pixels;
			pixels += (1 << level) << level;
		}

		for (i = p_log; (MechS32) i < cu32MAX_TMAP_LEV; i++) {
			p_texture->m_tmap.apvLevels[i] = pixels;
		}
	}
	else {
		p_texture->m_tmap.apvLevels[0] = pixels;
	}

	result = ATI3DCIF_TextureReg(&p_texture->m_tmap, &p_texture->m_handle);
	if (result != C3D_EC_OK) {
		DebugPrint("Unable to register texture: %d\n", result);
		return FALSE;
	}

	return TRUE;
}

// Unregisters p_texture, deselecting the current texture; returns FALSE on failure.
// FUNCTION: MW2ATI 0x100641a0
MechS32 FUN_100641a0(A3DCachedTexture* p_texture)
{
	C3D_EC result;

	if (g_a3dTexture) {
		g_a3dTexture = 0;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_SELECT, &g_a3dTexture);
	}

	result = ATI3DCIF_TextureUnreg(p_texture->m_handle);
	if (result != C3D_EC_OK) {
		DebugPrint("Unable to unregister texture: %x %d\n", p_texture->m_handle, result);
		return FALSE;
	}

	return TRUE;
}

// Creates a texture 1 << p_log texels square, with levels when p_mip, at the head of its size's
// list; returns NULL if no surface can be had.
// FUNCTION: MW2ATI 0x10064200
A3DCachedTexture* FUN_10064200(MechU32 p_log, MechS32 p_mip)
{
	A3DCachedTexture* texture;

	if (g_a3dSizeLimit <= p_log) {
		return NULL;
	}

	if (g_a3dSizeTextures[p_log] >= 100) {
		return NULL;
	}

	texture = FUN_10063f80();
	if (!texture) {
		return NULL;
	}

	if (!FUN_10063fd0(texture, p_log, p_mip)) {
		g_a3dSizeLimit = p_log;
		DebugPrint("A3D: Create surface failed (%d: %s).\n", p_log, p_mip ? "Mip" : "");
		return NULL;
	}

	if (!FUN_100640e0(texture, p_log, p_mip)) {
		return NULL;
	}

	if (g_a3dTextureLists[p_log]) {
		texture->m_next = g_a3dTextureLists[p_log];
	}

	g_a3dTextureLists[p_log] = texture;
	FUN_10063fc0();
	g_a3dTextureListLengths[p_log]++;
	g_a3dSizeTextures[p_log]++;
	return texture;
}

// Locks p_texture's surface, restoring it if it was lost; returns FALSE on failure.
// FUNCTION: MW2ATI 0x100642d0
MechS32 FUN_100642d0(A3DCachedTexture* p_texture)
{
	HRESULT result;

	result = IDirectDrawSurface_Lock(p_texture->m_surface, NULL, &p_texture->m_desc, DDLOCK_WAIT, NULL);
	if (result == DDERR_SURFACELOST) {
		IDirectDrawSurface_Restore(p_texture->m_surface);
		result = IDirectDrawSurface_Lock(p_texture->m_surface, NULL, &p_texture->m_desc, DDLOCK_WAIT, NULL);
	}

	return result == DD_OK;
}

// Unlocks p_texture's surface; returns FALSE on failure.
// FUNCTION: MW2ATI 0x10064320
MechS32 FUN_10064320(A3DCachedTexture* p_texture)
{
	return IDirectDrawSurface_Unlock(p_texture->m_surface, NULL) == DD_OK;
}

// Frees the textures 1 << p_log texels square.
// FUNCTION: MW2ATI 0x10064340
void FUN_10064340(MechU32 p_log)
{
	A3DCachedTexture* texture;

	while ((texture = g_a3dTextureLists[p_log]) != NULL) {
		FUN_100641a0(texture);
		FUN_10064320(texture);
		FUN_100640d0(texture);
		g_a3dTextureLists[p_log] = texture->m_next;
		texture->m_next = NULL;
		g_a3dTextureListLengths[p_log]--;
		g_a3dSizeTextures[p_log]--;
		g_a3dTextureCount--;
	}
}

// Writes the level under p_pixels (2 << p_log texels square) to p_out, each texel the average of
// four, with the alpha bit; returns the end of the level written.
// FUNCTION: MW2ATI 0x100643b0
MechU16* FUN_100643b0(MechU16* p_pixels, MechU16* p_out, MechU32 p_log)
{
	MechU16* below;
	MechU16 texel;
	MechU8 red;
	MechU8 green;
	MechU8 blue;
	MechS32 pitch;
	MechS32 size;
	MechS32 x;
	MechS32 y;

	pitch = 1 << (p_log + 1);
	size = 1 << p_log;
	below = p_pixels + pitch;
	for (y = size; y; y--) {
		for (x = size; x; x--) {
			texel = *p_pixels++;
			blue = texel & 0x1f;
			texel >>= 5;
			green = texel & 0x1f;
			texel >>= 5;
			red = texel & 0x1f;
			texel = *p_pixels++;
			blue += texel & 0x1f;
			texel >>= 5;
			green += texel & 0x1f;
			texel >>= 5;
			red += texel & 0x1f;
			texel = *below++;
			blue += texel & 0x1f;
			texel >>= 5;
			green += texel & 0x1f;
			texel >>= 5;
			red += texel & 0x1f;
			texel = *below++;
			blue += texel & 0x1f;
			texel >>= 5;
			green += texel & 0x1f;
			texel >>= 5;
			red += texel & 0x1f;
			*p_out++ = (MechU8) ((red >> 2) | 0x20) << 10 | (MechU8) (green >> 2) << 5 | (MechU8) (blue >> 2);
		}

		p_pixels += pitch;
		below += pitch;
	}

	return p_out;
}

// "A3D_TEXTURE_fill()": copies the p_width x p_height texels p_pixels into p_texture's surface,
// and with p_mip its p_log levels under them.
// FUNCTION: MW2ATI 0x10064570
void A3D_TEXTURE_fill(
	A3DCachedTexture* p_texture,
	MechU16* p_pixels,
	MechS32 p_width,
	MechS32 p_height,
	MechS32 p_mip,
	MechU32 p_log
)
{
	MechU16* out;
	MechU16* level;
	MechU16* next;

	g_a3dTextureFills++;
	g_a3dTextureBytes += p_width * p_height * 2;
	if (!FUN_100642d0(p_texture)) {
		DebugPrint("A3D_TEXTURE_fill(): Unable to lock buffer.");
	}

	p_texture->m_pixels = p_pixels;
	out = (MechU16*) p_texture->m_desc.lpSurface;
	// The original copies the texels with an __asm block, leaving out after them.
#ifdef PORTABLE_C
	memcpy(out, p_pixels, ((MechU32) (p_width * p_height) >> 1) * sizeof(MechU32));
	out += ((MechU32) (p_width * p_height) >> 1) * 2;
#else
	__asm {
		mov eax, p_width
		mov ecx, p_height
		imul ecx
		mov esi, p_pixels
		mov ecx, eax
		mov edi, out
		shr ecx, 1
		rep movsd
		mov out, edi
	}
#endif
	if (p_mip) {
		level = (MechU16*) p_texture->m_desc.lpSurface;
		while (p_log) {
			p_log--;
			next = out;
			out = FUN_100643b0(level, out, p_log);
			level = next;
		}
	}

	if (!FUN_10064320(p_texture)) {
		DebugPrint("A3D_TEXTURE_fill(): Unable to unlock buffer.");
	}
}

// Returns the texture 1 << p_log texels square that holds p_pixels, or NULL.
// FUNCTION: MW2ATI 0x10064630
A3DCachedTexture* FUN_10064630(MechU32 p_log, MechU16* p_pixels)
{
	A3DCachedTexture* texture;

	for (texture = g_a3dTextureLists[p_log]; texture; texture = texture->m_next) {
		if (texture->m_pixels == p_pixels) {
			return texture;
		}
	}

	return NULL;
}

// Returns the least recently used texture 1 << p_log texels square, with levels as p_mip says
// (the first of the size if none has).
// FUNCTION: MW2ATI 0x10064650
A3DCachedTexture* FUN_10064650(MechU32 p_log, MechS32 p_mip)
{
	A3DCachedTexture* texture;
	A3DCachedTexture* oldest;
	MechU32 lastUsed;

	lastUsed = 0xffffffff;
	oldest = g_a3dTextureLists[p_log];
	for (texture = oldest; texture; texture = texture->m_next) {
		if (lastUsed > texture->m_lastUsed && texture->m_tmap.bMipMap == p_mip) {
			oldest = texture;
			lastUsed = texture->m_lastUsed;
		}
	}

	return oldest;
}

// Restores p_texture's surface if it was lost; returns FALSE if it was (its texels are gone).
// FUNCTION: MW2ATI 0x10064690
MechS32 FUN_10064690(A3DCachedTexture* p_texture)
{
	if (IDirectDrawSurface_IsLost(p_texture->m_surface) == DDERR_SURFACELOST) {
		DebugPrint("A3D: texture surface lost");
		IDirectDrawSurface_Restore(p_texture->m_surface);
		if (IDirectDrawSurface_IsLost(p_texture->m_surface) == DDERR_SURFACELOST) {
			IDirectDrawSurface_Restore(p_texture->m_surface);
		}

		if (!FUN_100642d0(p_texture)) {
			DebugPrint("A3D: Lock texture failed.");
		}

		if (!FUN_10064320(p_texture)) {
			DebugPrint("A3D: Unlock texture failed.");
		}

		return FALSE;
	}

	return TRUE;
}

// Has every texture copied again before its next use.
// FUNCTION: MW2ATI 0x10064710
void FUN_10064710(void)
{
	g_a3dPaletteClock = g_a3dClock;
}

// Creates the textures: four of each size from 16 to 128 texels square, two of the largest with
// levels. Returns TRUE.
// FUNCTION: MW2ATI 0x10064720
MechS32 FUN_10064720(void)
{
	MechU32 i;
	MechU32 log;

	for (i = 0; i < 4; i++) {
		for (log = 7; log >= 4; log--) {
			FUN_10064200(log, i < 2 && log == 7);
		}
	}

	return TRUE;
}

// Frees the textures.
// FUNCTION: MW2ATI 0x10064760
void FUN_10064760(void)
{
	MechS32 log;

	FUN_10064340(7);
	for (log = 4; log <= 7; log++) {
		FUN_10064340(log);
	}
}

// Returns the CIF's handle of a texture holding the p_width x p_height texels p_pixels (with
// levels when p_mip), copying them into one if no texture holds them or they changed; 0 if there
// is none to take.
// FUNCTION: MW2ATI 0x10064780
C3D_HTX FUN_10064780(MechU16* p_pixels, MechU32 p_width, MechU32 p_height, MechS32 p_mip)
{
	A3DCachedTexture* texture;
	MechU32 size;
	MechU32 log;

	if (!g_a3dTexturesCreated) {
		g_a3dTexturesCreated = FUN_10064720();
	}

	size = p_height;
	if (p_height <= p_width) {
		size = p_width;
	}

	log = FUN_10063f60(size);
	if ((MechS32) (1 << log) < (MechS32) size) {
		log++;
	}

	texture = FUN_10064630(log, p_pixels);
	if (!texture) {
		if (!p_mip) {
			texture = FUN_10064200(log, p_mip);
		}

		if (!texture && (texture = FUN_10064650(log, p_mip)) == NULL) {
			return 0;
		}

		A3D_TEXTURE_fill(texture, p_pixels, p_width, p_height, p_mip, log);
	}
	else {
		g_a3dTextureHits++;
		if (!FUN_10064690(texture) || texture->m_lastUsed < g_a3dPaletteClock) {
			A3D_TEXTURE_fill(texture, p_pixels, p_width, p_height, p_mip, log);
			FUN_100641a0(texture);
			if (!FUN_100640e0(texture, log, p_mip)) {
				return 0;
			}
		}
	}

	if (texture) {
		texture->m_lastUsed = g_a3dClock++;
		return texture->m_handle;
	}

	return 0;
}

// FUNCTION: MW2ATI 0x100648a0
void FUN_100648a0(MechS32 p_filter)
{
	g_a3dFilterSetting = p_filter;
	switch (p_filter) {
	case 1:
		if (g_a3dContext && g_a3dFilter != C3D_ETFILT_MINPNT_MAG2BY2) {
			g_a3dFilter = C3D_ETFILT_MINPNT_MAG2BY2;
			ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_FILTER, &g_a3dFilter);
		}
		break;
	case 2:
		if (g_a3dContext && g_a3dFilter != C3D_ETFILT_MIN2BY2_MAG2BY2) {
			g_a3dFilter = C3D_ETFILT_MIN2BY2_MAG2BY2;
			ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_FILTER, &g_a3dFilter);
		}
		break;
	default:
		if (g_a3dContext && g_a3dFilter != C3D_ETFILT_MINPNT_MAGPNT) {
			g_a3dFilter = C3D_ETFILT_MINPNT_MAGPNT;
			ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_FILTER, &g_a3dFilter);
		}
		break;
	}
}

// FUNCTION: MW2ATI 0x10064930
void A3D_DrawPolygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices)
{
	A3DVertex** triangles;

	if (!FUN_10063db0(p_count)) {
		return;
	}

	FUN_10063e20(p_pane->m_x0, p_pane->m_x1, p_pane->m_y0, p_pane->m_y1);
	triangles = FUN_10063eb0(p_vertices);
	if (!g_a3dRendering) {
		FUN_10063d10();
	}

	if (g_a3dTextureEnable != FALSE) {
		g_a3dTextureEnable = FALSE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_EN, &g_a3dTextureEnable);
	}

	if (g_a3dShadeMode != C3D_ESH_SMOOTH) {
		g_a3dShadeMode = C3D_ESH_SMOOTH;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_SHADE_MODE, &g_a3dShadeMode);
	}

	FUN_10063e00(ATI3DCIF_RenderPrimList((C3D_VLIST) triangles, p_count * 3 - 6));
}

// FUNCTION: MW2ATI 0x100649e0
void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, WINDOW* p_texture, MechU32 p_flags)
{
	A3DVertex** triangles;
	C3D_HTX texture;

	if (!FUN_10063db0(p_count)) {
		return;
	}

	FUN_10063e20(p_pane->m_x0, p_pane->m_x1, p_pane->m_y0, p_pane->m_y1);
	texture = FUN_10064780((MechU16*) p_texture->m_buffer, p_texture->m_xMax + 1, p_texture->m_yMax + 1, 0);
	if (!texture) {
		DebugPrint("A3D: No texture\n");
		return;
	}

	triangles = FUN_10063eb0(p_vertices);
	if (g_a3dPerspective != C3D_ETPC_NONE) {
		g_a3dPerspective = C3D_ETPC_NONE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_PERSP_COR, &g_a3dPerspective);
	}

	if (texture != g_a3dTexture) {
		g_a3dTexture = texture;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_SELECT, &g_a3dTexture);
	}

	if (g_a3dTextureEnable != TRUE) {
		g_a3dTextureEnable = TRUE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_EN, &g_a3dTextureEnable);
	}

	if (p_flags & 2) {
		if (g_a3dTexOp != C3D_ETEXOP_ALPHA_MASK) {
			g_a3dTexOp = C3D_ETEXOP_ALPHA_MASK;
			ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_TEXOP, &g_a3dTexOp);
		}
	}
	else if (g_a3dTexOp != C3D_ETEXOP_NONE) {
		g_a3dTexOp = C3D_ETEXOP_NONE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_TEXOP, &g_a3dTexOp);
	}

	if (p_flags & 1) {
		if (g_a3dLight != C3D_ETL_MODULATE) {
			g_a3dLight = C3D_ETL_MODULATE;
			ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_LIGHT, &g_a3dLight);
		}
	}
	else if (g_a3dLight != C3D_ETL_NONE) {
		g_a3dLight = C3D_ETL_NONE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_LIGHT, &g_a3dLight);
	}

	if (!g_a3dRendering) {
		FUN_10063d10();
	}

	FUN_10063e00(ATI3DCIF_RenderPrimList((C3D_VLIST) triangles, p_count * 3 - 6));
}

// FUNCTION: MW2ATI 0x10064b60
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
	A3DVertex** triangles;
	C3D_HTX texture;

	if (!FUN_10063db0(p_count)) {
		return;
	}

	FUN_10063e20(p_pane->m_x0, p_pane->m_x1, p_pane->m_y0, p_pane->m_y1);
	texture = FUN_10064780(*p_texture->m_rows, p_texture->m_width, p_texture->m_height, p_mip);
	if (!texture) {
		DebugPrint("No texture");
		return;
	}

	triangles = FUN_10063eb0(p_vertices);
	if (!g_a3dRendering) {
		FUN_10063d10();
	}

	if (p_flags & 1) {
		if (g_a3dTexOp != C3D_ETEXOP_ALPHA_MASK) {
			g_a3dTexOp = C3D_ETEXOP_ALPHA_MASK;
			ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_TEXOP, &g_a3dTexOp);
		}
	}
	else if (g_a3dTexOp != C3D_ETEXOP_NONE) {
		g_a3dTexOp = C3D_ETEXOP_NONE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_TEXOP, &g_a3dTexOp);
	}

	if (p_flags & 0x600) {
		if (g_a3dPerspective != C3D_ETPC_THREE) {
			g_a3dPerspective = C3D_ETPC_THREE;
			ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_PERSP_COR, &g_a3dPerspective);
		}
	}
	else if (g_a3dPerspective != C3D_ETPC_NONE) {
		g_a3dPerspective = C3D_ETPC_NONE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_PERSP_COR, &g_a3dPerspective);
	}

	if (g_a3dTexture != texture) {
		g_a3dTexture = texture;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_SELECT, &g_a3dTexture);
	}

	if (g_a3dLight != C3D_ETL_MODULATE) {
		g_a3dLight = C3D_ETL_MODULATE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_LIGHT, &g_a3dLight);
	}

	if (g_a3dTextureEnable != TRUE) {
		g_a3dTextureEnable = TRUE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_EN, &g_a3dTextureEnable);
	}

	FUN_10063e00(ATI3DCIF_RenderPrimList((C3D_VLIST) triangles, p_count * 3 - 6));
}

// "A3D_shutdown()".
// FUNCTION: MW2ATI 0x10064d00
void A3D_shutdown(void)
{
	FUN_10064760();
	ATI3DCIF_RenderEnd();
	if (g_a3dContext) {
		ATI3DCIF_ContextDestroy(g_a3dContext);
	}

	if (g_a3dInitialized) {
		ATI3DCIF_Term();
	}
}

// Draws the outline of p_vertices as a line strip, in the color of the first vertex.
// FUNCTION: MW2ATI 0x10064d30
void A3D_DrawOutline(PANE* p_pane, A3DVertex* p_vertices, MechU32 p_count)
{
	C3D_COLOR color;

	color.r = (C3D_UINT32) p_vertices->m_red;
	color.g = (C3D_UINT32) p_vertices->m_green;
	color.b = (C3D_UINT32) p_vertices->m_blue;
	if (!FUN_10063db0(p_count)) {
		return;
	}

	FUN_10063e20(p_pane->m_x0, p_pane->m_x1, p_pane->m_y0, p_pane->m_y1);
	if (!g_a3dRendering) {
		FUN_10063d10();
	}

	if (g_a3dTextureEnable != FALSE) {
		g_a3dTextureEnable = FALSE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_TMAP_EN, &g_a3dTextureEnable);
	}

	if (g_a3dPrimType != C3D_EPRIM_LINE) {
		g_a3dPrimType = C3D_EPRIM_LINE;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_PRIM_TYPE, &g_a3dPrimType);
	}

	if (g_a3dShadeMode != C3D_ESH_SOLID) {
		g_a3dShadeMode = C3D_ESH_SOLID;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_SHADE_MODE, &g_a3dShadeMode);
	}

	ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_SOLID_CLR, &color);
	FUN_10063e00(ATI3DCIF_RenderPrimStrip((C3D_VSTRIP) p_vertices, p_count));
	if (g_a3dPrimType != C3D_EPRIM_TRI) {
		g_a3dPrimType = C3D_EPRIM_TRI;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_PRIM_TYPE, &g_a3dPrimType);
	}

	if (g_a3dShadeMode != C3D_ESH_SMOOTH) {
		g_a3dShadeMode = C3D_ESH_SMOOTH;
		ATI3DCIF_ContextSetState(g_a3dContext, C3D_ERS_SHADE_MODE, &g_a3dShadeMode);
	}
}

// FUNCTION: MW2ATI 0x10064eb0
void A3D_EdgeCrossing(
	MechS32 p_axis,
	MechFloat p_edge,
	A3DVertex* p_a,
	A3DVertex* p_b,
	A3DVertex* p_out,
	MechU32 p_flags
)
{
	MechFloat t;

	switch (p_axis) {
	case 1:
		t = (p_edge - p_a->m_x) / (p_b->m_x - p_a->m_x);
		p_out->m_x = p_edge;
		p_out->m_y = (p_b->m_y - p_a->m_y) * t + p_a->m_y;
		if (p_flags & 8) {
			p_out->m_w = (p_b->m_w - p_a->m_w) * t + p_a->m_w;
		}
		break;
	case 2:
		t = (p_edge - p_a->m_y) / (p_b->m_y - p_a->m_y);
		p_out->m_y = p_edge;
		p_out->m_x = (p_b->m_x - p_a->m_x) * t + p_a->m_x;
		if (p_flags & 8) {
			p_out->m_w = (p_b->m_w - p_a->m_w) * t + p_a->m_w;
		}
		break;
	}

	p_out->m_z = p_a->m_z;
	if (p_flags & 1) {
		p_out->m_red = (p_b->m_red - p_a->m_red) * t + p_a->m_red;
		p_out->m_green = (p_b->m_green - p_a->m_green) * t + p_a->m_green;
		p_out->m_blue = (p_b->m_blue - p_a->m_blue) * t + p_a->m_blue;
	}
	else {
		p_out->m_red = p_a->m_red;
		p_out->m_green = p_a->m_green;
		p_out->m_blue = p_a->m_blue;
	}

	if (p_flags & 2) {
		p_out->m_alpha = (p_b->m_alpha - p_a->m_alpha) * t + p_a->m_alpha;
	}

	if (p_flags & 4) {
		p_out->m_u = (p_b->m_u - p_a->m_u) * t + p_a->m_u;
		p_out->m_v = (p_b->m_v - p_a->m_v) * t + p_a->m_v;
	}
}
