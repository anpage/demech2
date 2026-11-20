/* The S3 ViRGE edition's A3D renderer (accel/a3d.h): the game's polygons drawn through S3's toolkit
   (S3DTKW.DLL) on DirectDraw surfaces, one object of its own after bwdnames.c's, compiled like the
   edition's other units. The toolkit draws into the frame buffer the display back end has locked
   (FUN_1000f850), a state at a time: each state the renderer sets is cached here, and set again
   only when it changes. A polygon is drawn as a fan of triangles, a list of pointers to its
   vertices. Its textures are VFX WINDOWs of 16-bit texels (RGB555), copied into DirectDraw
   surfaces in video memory that the toolkit textures from: up to 400, kept in a list per size (1
   to 128 texels square, by the log of the size), the most recently created first. A texture is
   looked up by its pixels; when its size has no free surface, it takes the least recently used
   one's, and every texture is copied again after a palette change. A mip-mapped texture's surface
   holds its levels one after the other, each averaged from the one before. The ATI edition's
   renderer, three months older, is built the same way over ATI's 3D CIF (ati/a3d.c). */
#include "accel/a3d.h"

#include "accel/a3dvertex.h"
#include "compat.h"
#include "debugprint.h"
#include "decomp.h"
#include "directdraw.h"
#include "palettecolor.h"
#include "pane.h"
#include "refreshmode.h"
#include "render.h"
#include "types.h"
#include "window.h"

#include <ddraw.h>
#include <s3dtk.h>
#include <string.h>

// A texture: a DirectDraw surface of its texels, and the toolkit's surface of it. Its levels are
// the surface's, from m_desc.lpSurface.
// SIZE 0xa4
typedef struct A3DCachedTexture A3DCachedTexture;
struct A3DCachedTexture {
	MechU16* m_pixels;             // 0x00 — the texels it holds, which identify it
	LPDIRECTDRAWSURFACE m_surface; // 0x04
	DDSURFACEDESC m_desc;          // 0x08 — as the last lock left it
	S3DTK_SURFACE m_s3dSurface;    // 0x74
	MechU32 m_lastUsed;            // 0x98 — g_a3dClock when it was last drawn
	MechU32 m_flags;               // 0x9c — 1 mip-mapped, 2 with its alpha bit
	A3DCachedTexture* m_next;      // 0xa0 — in its size's list
};

DECOMP_SIZE_ASSERT(A3DCachedTexture, 0xa4)

// The largest texture size, by its log, plus one.
#define A3D_SIZES 8

// The frame buffer the display back end has locked (FUN_1000f850).
// GLOBAL: MW2S3 0x10062210
undefined* g_a3dFrameBuffer = NULL;

// The textures that may be mip-mapped are (FUN_10010470).
// GLOBAL: MW2S3 0x10062214
MechS32 g_a3dMipmapped = FALSE;

// The renderer's states, as last set, and the texture filter to set once there is a renderer.
// GLOBAL: MW2S3 0x10062218
MechU32 g_a3dRenderingType = S3DTK_GOURAUD;

// GLOBAL: MW2S3 0x1006221c
MechU32 g_a3dFilter = S3DTK_TEX1TPP;

// GLOBAL: MW2S3 0x10062220
MechU32 g_a3dPendingFilter = S3DTK_TEX1TPP;

// GLOBAL: MW2S3 0x10062224
MechU32 g_a3dZBuffer = FALSE;

// GLOBAL: MW2S3 0x10062228
S3DTK_SURFACE* g_a3dTexture = NULL;

// GLOBAL: MW2S3 0x1006222c
MechU32 g_a3dAlphaBlending = S3DTK_ALPHAOFF;

// GLOBAL: MW2S3 0x10062230
MechU32 g_a3dTexBlending = 0;

// Set by FUN_1000f850, never read.
// GLOBAL: MW2S3 0x10062234
MechU32 g_unk0x10062234 = 0;

// The frame buffer the renderer was last pointed at.
// GLOBAL: MW2S3 0x10062238
undefined* g_a3dSurface = NULL;

// The toolkit is initialized (A3D_Init).
// GLOBAL: MW2S3 0x1006223c
MechS32 g_a3dInitialized = FALSE;

// Statistics: the textures found cached, the textures copied, and the bytes copied.
// GLOBAL: MW2S3 0x10062240
MechU32 g_a3dTextureHits = 0;

// GLOBAL: MW2S3 0x10062244
MechU32 g_a3dTextureFills = 0;

// GLOBAL: MW2S3 0x10062248
MechU32 g_a3dTextureBytes = 0;

// GLOBAL: MW2S3 0x1006224c
MechU32 g_unk0x1006224c = 0;

// The frame buffer's linear address (S3DTK_VIDEOMEMORYADDRESS), and its physical address.
// GLOBAL: MW2S3 0x10062250
MechU32 g_a3dVideoMemory = 0;

// GLOBAL: MW2S3 0x10062254
MechU32 g_a3dVideoMemoryPhysical = 0;

// The renderer.
// GLOBAL: MW2S3 0x10062258
S3DTK_LPFUNCTIONLIST g_a3dRenderer = NULL;

// A clock that ticks at every texture drawn, and its time at the last palette change.
// GLOBAL: MW2S3 0x1006225c
MechU32 g_a3dClock = 1;

// GLOBAL: MW2S3 0x10062260
MechU32 g_a3dPaletteClock = 0;

// The textures in use in g_a3dTextures.
// GLOBAL: MW2S3 0x10062264
MechU32 g_a3dTextureCount = 0;

// The rows of a mip-mapped texture's surface, by the log of its size: its levels', one after the
// other, at its width.
// GLOBAL: MW2S3 0x10062268
MechU32 g_a3dMipRows[9] = {1, 3, 6, 11, 22, 43, 86, 171, 342};

// The sizes, by their log, from which no surface can be created.
// GLOBAL: MW2S3 0x1006228c
MechU32 g_a3dSizeLimit = A3D_SIZES;

// The textures are created (FUN_10010110).
// GLOBAL: MW2S3 0x10062290
MechS32 g_a3dTexturesCreated = FALSE;

// A3D_DrawPolygon has yet to set its states the first time.
// GLOBAL: MW2S3 0x10062294
MechS32 g_a3dFirstPolygon = TRUE;

// The original has room for 8 lengths before the next global.
// GLOBAL: MW2S3 0x1014dc10
MechU32 g_a3dTextureListLengths[A3D_SIZES];

// The fans of the two vertex arrays (g_a3dVertices and g_a3dClipped), made by FUN_1000f940 at
// their first use in a frame: a pointer to each vertex.
// GLOBAL: MW2S3 0x1014dc30
A3DVertex* g_a3dFan[0x100];

// GLOBAL: MW2S3 0x1014e030
A3DVertex* g_a3dFan2[0x100];

// The surface drawn on: the frame buffer.
// GLOBAL: MW2S3 0x1014e430
S3DTK_SURFACE g_a3dDrawSurface;

// The textures of each size: at most 100.
// GLOBAL: MW2S3 0x1014e460
MechU32 g_a3dSizeTextures[A3D_SIZES];

// GLOBAL: MW2S3 0x1014e480
A3DPaletteColor g_a3dPalette[0x100];

// GLOBAL: MW2S3 0x1014e780
A3DCachedTexture g_a3dTextures[400];

// The lists of the textures of each size. The original has room for 7 before the next global.
// GLOBAL: MW2S3 0x1015e7c0
A3DCachedTexture* g_a3dTextureLists[A3D_SIZES - 1];

void FUN_1000f9a0(void);

// Returns the physical address of the linear address p_linear.
// FUNCTION: MW2S3 0x1000f600
MechU32 FUN_1000f600(MechU32 p_linear)
{
	MechU32 offset;

	offset = p_linear & 0xfff;
	return S3DTK_LinearToPhysical(p_linear & ~0xfff) + offset;
}

// Returns the offset of the linear address p_linear in video memory.
// FUNCTION: MW2S3 0x1000f620
MechU32 FUN_1000f620(MechU32 p_linear)
{
	return FUN_1000f600(p_linear) - g_a3dVideoMemoryPhysical;
}

// "A3D_Init()": initializes the toolkit, creates its renderer and sets its first states. Returns
// TRUE on success.
// FUNCTION: MW2S3 0x1000f630
MechS32 A3D_Init(void)
{
	S3DTK_RENDERER_INITSTRUCT init;
	ULONG result;

	DebugPrint("A3D: InitLib\n");
	result = S3DTK_InitLib(NULL);
	if (result != S3DTK_OK) {
		DebugPrint("Error: InitLib returned %d\n", result);
		return FALSE;
	}

	init.initUserID = 0;
	init.initAppID = 0;
	init.initFlags = S3DTK_VERIFY_UVRANGE;
	DebugPrint("A3D: CreateRenderer\n");
	result = S3DTK_CreateRenderer(&init, &g_a3dRenderer);
	if (result != S3DTK_OK) {
		DebugPrint("Error: CreateRenderer returned %d\n", result);
		return FALSE;
	}

	g_a3dRenderer->S3DTK_GetState(g_a3dRenderer, S3DTK_VIDEOMEMORYADDRESS, (ULONG) &g_a3dVideoMemory);
	g_a3dVideoMemoryPhysical = FUN_1000f600(g_a3dVideoMemory);
	g_a3dInitialized = TRUE;
	if (g_a3dRenderer && g_a3dPendingFilter != g_a3dFilter) {
		g_a3dFilter = g_a3dPendingFilter;
		g_a3dRenderer->S3DTK_SetState(g_a3dRenderer, S3DTK_TEXFILTERINGMODE, g_a3dPendingFilter);
	}

	g_a3dRenderer->S3DTK_SetState(g_a3dRenderer, S3DTK_FOGCOLOR, S3DTK_FOGOFF);
	if (g_a3dZBuffer != FALSE) {
		g_a3dZBuffer = FALSE;
		g_a3dRenderer->S3DTK_SetState(g_a3dRenderer, S3DTK_ZBUFFERENABLE, FALSE);
	}

	if (g_a3dAlphaBlending != S3DTK_ALPHAOFF) {
		g_a3dAlphaBlending = S3DTK_ALPHAOFF;
		g_a3dRenderer->S3DTK_SetState(g_a3dRenderer, S3DTK_ALPHABLENDING, S3DTK_ALPHAOFF);
	}

	g_a3dFan[0] = NULL;
	g_a3dFan2[0] = NULL;
	DebugPrint("A3D:\tInit() done.\n");
	return TRUE;
}

// Points the renderer at the frame buffer (RGB555, half its pitch in pixels), initializing the
// toolkit first if it isn't. Returns FALSE if it can't be initialized.
// FUNCTION: MW2S3 0x1000f780
MechS32 FUN_1000f780(void)
{
	if (!g_a3dInitialized && !A3D_Init()) {
		return FALSE;
	}

	g_a3dDrawSurface.sfOffset = FUN_1000f620((MechU32) g_a3dFrameBuffer);
	g_a3dDrawSurface.sfFormat = S3DTK_VIDEORGB15;
	g_a3dDrawSurface.sfWidth = (g_mainPixelBuffer.m_xMax + 1) >> 1;
	g_a3dDrawSurface.sfHeight = g_mainPixelBuffer.m_yMax + 1;
	if (g_currentRefreshMode->m_index != 0 && g_currentRefreshMode->m_index != 2) {
		g_a3dDrawSurface.sfFormat = S3DTK_SYSTEM | S3DTK_VIDEORGB15;
	}

	g_a3dRenderer->S3DTK_SetState(g_a3dRenderer, S3DTK_DRAWSURFACE, (ULONG) &g_a3dDrawSurface);
	return TRUE;
}

// Waits until the toolkit's engine is idle.
// FUNCTION: MW2S3 0x1000f800
void FUN_1000f800(void)
{
	ULONG idle;

	idle = FALSE;
	if (g_a3dInitialized) {
		while (!g_a3dRenderer->S3DTK_GetState(g_a3dRenderer, S3DTK_GRAPHICS_ENGINE_IDLE, (ULONG) &idle)) {
		}
	}
}

// FUNCTION: MW2S3 0x1000f840
void FUN_1000f840(undefined* p_surface)
{
	FUN_1000f800();
}

// FUNCTION: MW2S3 0x1000f850
void FUN_1000f850(undefined* p_surface)
{
	FUN_1000f800();
	g_unk0x10062234 = 0;
	g_a3dFrameBuffer = p_surface;
}

// Sets g_a3dPalette, the palette's components times 4, and has every texture copied again (their
// texels are palette colors).
// FUNCTION: MW2S3 0x1000f870
void A3D_SetPalette(PaletteColor* p_palette)
{
	A3DPaletteColor* color;

	for (color = g_a3dPalette; color < &g_a3dPalette[0x100]; color++) {
		color->m_red = p_palette->m_red << 2;
		color->m_green = p_palette->m_green << 2;
		color->m_blue = p_palette->m_blue << 2;
		p_palette++;
	}

	FUN_1000f9a0();
}

// Prepares a polygon of p_count vertices: points the renderer at the frame buffer if it has moved.
// Returns FALSE if the renderer can't draw it.
// FUNCTION: MW2S3 0x1000f8a0
MechS32 FUN_1000f8a0(MechU32 p_count)
{
	if (g_a3dFrameBuffer != g_a3dSurface) {
		if (!FUN_1000f780()) {
			return FALSE;
		}

		g_a3dSurface = g_a3dFrameBuffer;
	}

	if (p_count > 0xff) {
		DebugPrint("A3D:  Not enough vertices\n");
		return FALSE;
	}

	return TRUE;
}

// Reports a toolkit error; returns TRUE for none.
// FUNCTION: MW2S3 0x1000f8f0
MechS32 FUN_1000f8f0(ULONG p_result)
{
	if (p_result != S3DTK_OK) {
		DebugPrint("A3D: Error %d\n", g_a3dRenderer->S3DTK_GetLastError(g_a3dRenderer));
		return FALSE;
	}

	return TRUE;
}

// Fills p_fan with pointers to the first 11 vertices of p_vertices.
// FUNCTION: MW2S3 0x1000f920
void FUN_1000f920(A3DVertex* p_vertices, A3DVertex** p_fan)
{
	MechS32 i;

	for (i = 11; i; i--) {
		*p_fan++ = p_vertices++;
	}
}

// Returns the fan of the vertex array p_vertices.
// FUNCTION: MW2S3 0x1000f940
A3DVertex** FUN_1000f940(A3DVertex* p_vertices)
{
	if (g_a3dFan[0] == p_vertices) {
		return g_a3dFan;
	}

	if (g_a3dFan2[0] == p_vertices) {
		return g_a3dFan2;
	}

	if (!g_a3dFan[0]) {
		FUN_1000f920(p_vertices, g_a3dFan);
		return g_a3dFan;
	}

	if (!g_a3dFan2[0]) {
		FUN_1000f920(p_vertices, g_a3dFan2);
		return g_a3dFan2;
	}

	return g_a3dFan2;
}

// Has every texture copied again before its next use.
// FUNCTION: MW2S3 0x1000f9a0
void FUN_1000f9a0(void)
{
	g_a3dPaletteClock = g_a3dClock;
}

// Returns the log of p_size, rounded down.
// FUNCTION: MW2S3 0x1000f9b0
MechU32 FUN_1000f9b0(MechU32 p_size)
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

// Returns the next free texture, cleared, or NULL if there is none (FUN_1000fa00 takes it).
// FUNCTION: MW2S3 0x1000f9d0
A3DCachedTexture* FUN_1000f9d0(void)
{
	if (g_a3dTextureCount < 400) {
		memset(&g_a3dTextures[g_a3dTextureCount], 0, sizeof(A3DCachedTexture));
		return &g_a3dTextures[g_a3dTextureCount];
	}

	return NULL;
}

// FUNCTION: MW2S3 0x1000fa00
void FUN_1000fa00(void)
{
	g_a3dTextureCount++;
}

// Creates p_texture's surface in video memory, 1 << p_log texels square, or with mip levels
// (p_flags 1) as high as its levels; returns FALSE on failure.
// FUNCTION: MW2S3 0x1000fa10
MechS32 FUN_1000fa10(A3DCachedTexture* p_texture, MechU32 p_log, MechU32 p_flags)
{
	LPDDSURFACEDESC desc;
	LPDIRECTDRAWSURFACE surface;
	HRESULT result;
	MechU32 height;
	MechU32 width;

	desc = &p_texture->m_desc;
	if (p_flags & 1) {
		width = 1 << p_log;
		height = g_a3dMipRows[p_log];
	}
	else {
		width = 1 << p_log;
		height = width;
	}

	memset(desc, 0, sizeof(DDSURFACEDESC));
	desc->dwSize = sizeof(DDSURFACEDESC);
	desc->dwWidth = width;
	desc->dwHeight = height;
	desc->dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
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
// FUNCTION: MW2S3 0x1000fb10
void FUN_1000fb10(A3DCachedTexture* p_texture)
{
	if (p_texture->m_surface) {
		IDirectDrawSurface_Release(p_texture->m_surface);
	}
}

// Sets p_texture's toolkit surface, 1 << p_log texels square, with its alpha bit when p_flags has
// 2; deselects the current texture. Returns TRUE.
// FUNCTION: MW2S3 0x1000fb20
MechS32 FUN_1000fb20(A3DCachedTexture* p_texture, MechU32 p_log, MechU32 p_flags)
{
	memset(&p_texture->m_s3dSurface, 0, sizeof(S3DTK_SURFACE));
	p_texture->m_s3dSurface.sfOffset = FUN_1000f620((MechU32) p_texture->m_desc.lpSurface);
	p_texture->m_s3dSurface.sfFormat = S3DTK_TEXARGB4444;
	p_texture->m_s3dSurface.sfWidth = 1 << p_log;
	p_texture->m_s3dSurface.sfHeight = 1 << p_log;
	if (!(p_flags & 2)) {
		p_texture->m_s3dSurface.sfFormat = S3DTK_TEXARGB1555;
	}

	g_a3dTexture = NULL;
	return TRUE;
}

// Clears p_texture's toolkit surface. Returns TRUE.
// FUNCTION: MW2S3 0x1000fb90
MechS32 FUN_1000fb90(A3DCachedTexture* p_texture)
{
	memset(&p_texture->m_s3dSurface, 0, sizeof(S3DTK_SURFACE));
	return TRUE;
}

// Creates a texture 1 << p_log texels square, as p_flags says (FUN_1000fa10), at the head of its
// size's list; returns NULL if no surface can be had.
// FUNCTION: MW2S3 0x1000fbb0
A3DCachedTexture* FUN_1000fbb0(MechU32 p_log, MechU32 p_flags)
{
	A3DCachedTexture* texture;

	if (g_a3dSizeLimit <= p_log) {
		return NULL;
	}

	if (g_a3dSizeTextures[p_log] >= 100) {
		return NULL;
	}

	texture = FUN_1000f9d0();
	if (!texture) {
		return NULL;
	}

	if (!FUN_1000fa10(texture, p_log, p_flags)) {
		g_a3dSizeLimit = p_log;
		DebugPrint("A3D: Create surface failed (%d: %s).\n", p_log, (p_flags & 1) ? "Mip" : "");
		return NULL;
	}

	if (!FUN_1000fb20(texture, p_log, p_flags)) {
		return NULL;
	}

	texture->m_flags = p_flags;
	if (g_a3dTextureLists[p_log]) {
		texture->m_next = g_a3dTextureLists[p_log];
	}

	g_a3dTextureLists[p_log] = texture;
	FUN_1000fa00();
	g_a3dTextureListLengths[p_log]++;
	g_a3dSizeTextures[p_log]++;
	return texture;
}

// Locks p_texture's surface, restoring it if it was lost; returns FALSE on failure.
// FUNCTION: MW2S3 0x1000fc80
MechS32 FUN_1000fc80(A3DCachedTexture* p_texture)
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
// FUNCTION: MW2S3 0x1000fcd0
MechS32 FUN_1000fcd0(A3DCachedTexture* p_texture)
{
	HRESULT result;

	result = DD_OK;
	if (p_texture->m_surface) {
		result = IDirectDrawSurface_Unlock(p_texture->m_surface, NULL);
	}

	return result == DD_OK;
}

// Frees the textures 1 << p_log texels square.
// FUNCTION: MW2S3 0x1000fcf0
void FUN_1000fcf0(MechU32 p_log)
{
	A3DCachedTexture* texture;

	while ((texture = g_a3dTextureLists[p_log]) != NULL) {
		FUN_1000fb90(texture);
		FUN_1000fcd0(texture);
		FUN_1000fb10(texture);
		g_a3dTextureLists[p_log] = texture->m_next;
		texture->m_next = NULL;
		g_a3dTextureListLengths[p_log]--;
		g_a3dSizeTextures[p_log]--;
		g_a3dTextureCount--;
	}
}

// Writes the level under p_pixels (2 << p_log texels square) to p_out, each texel the average of
// four, with the alpha bit; the 1-texel level is followed by three copies of its texel. Returns the
// end of the level written.
// FUNCTION: MW2S3 0x1000fd60
MechU16* FUN_1000fd60(MechU16* p_pixels, MechU16* p_out, MechU32 p_log)
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
			texel = (MechU8) ((red >> 2) | 0x20) << 10 | (MechU8) (green >> 2) << 5 | (MechU8) (blue >> 2);
			*p_out++ = texel;
		}

		p_pixels += pitch;
		below += pitch;
	}

	if (!p_log) {
		*p_out++ = texel;
		*p_out++ = texel;
		*p_out++ = texel;
	}

	return p_out;
}

// "A3D_TEXTURE_fill()": copies the p_width x p_height texels p_pixels into p_texture's surface,
// and when it is mip-mapped (p_flags 1) its p_log levels after them.
// FUNCTION: MW2S3 0x1000ff30
void A3D_TEXTURE_fill(
	A3DCachedTexture* p_texture,
	MechU16* p_pixels,
	MechS32 p_width,
	MechS32 p_height,
	MechU32 p_flags,
	MechU32 p_log
)
{
	MechU16* texels;
	MechU16* level;
	MechU16* next;

	g_a3dTextureFills++;
	g_a3dTextureBytes += p_width * p_height * 2;
	if (!FUN_1000fc80(p_texture)) {
		DebugPrint("A3D_TEXTURE_fill(): Unable to lock buffer.");
	}

	p_texture->m_pixels = p_pixels;
	texels = (MechU16*) p_texture->m_desc.lpSurface;
	// The original copies the texels with an __asm block, which leaves texels pointing past them.
#ifdef PORTABLE_C
	memcpy(texels, p_pixels, ((MechU32) (p_width * p_height) >> 1) * sizeof(MechU32));
	texels += ((MechU32) (p_width * p_height) >> 1) * 2;
#else
	__asm {
		mov eax, p_width
		mov ecx, p_height
		imul ecx
		mov esi, p_pixels
		mov ecx, eax
		mov edi, texels
		shr ecx, 1
		rep movsd
		mov texels, edi
	}
#endif
	if (p_flags & 1) {
		level = (MechU16*) p_texture->m_desc.lpSurface;
		while (p_log) {
			p_log--;
			next = texels;
			texels = FUN_1000fd60(level, texels, p_log);
			level = next;
		}
	}

	if (!FUN_1000fcd0(p_texture)) {
		DebugPrint("A3D_TEXTURE_fill(): Unable to unlock buffer.");
	}
}

// Returns the texture 1 << p_log texels square that holds p_pixels, or NULL.
// FUNCTION: MW2S3 0x1000fff0
A3DCachedTexture* FUN_1000fff0(MechU32 p_log, MechU16* p_pixels)
{
	A3DCachedTexture* texture;

	for (texture = g_a3dTextureLists[p_log]; texture; texture = texture->m_next) {
		if (texture->m_pixels == p_pixels) {
			return texture;
		}
	}

	return NULL;
}

// Returns the least recently used texture 1 << p_log texels square, mip-mapped as p_flags says
// (the first of the size if none is).
// FUNCTION: MW2S3 0x10010010
A3DCachedTexture* FUN_10010010(MechU32 p_log, MechU32 p_flags)
{
	A3DCachedTexture* texture;
	A3DCachedTexture* oldest;
	MechU32 lastUsed;

	lastUsed = 0xffffffff;
	oldest = g_a3dTextureLists[p_log];
	if (p_flags & 1) {
		for (texture = oldest; texture; texture = texture->m_next) {
			if (lastUsed > texture->m_lastUsed && (texture->m_flags & 1)) {
				oldest = texture;
				lastUsed = texture->m_lastUsed;
			}
		}
	}
	else {
		for (texture = oldest; texture; texture = texture->m_next) {
			if (lastUsed > texture->m_lastUsed && !(texture->m_flags & 1)) {
				oldest = texture;
				lastUsed = texture->m_lastUsed;
			}
		}
	}

	return oldest;
}

// Restores p_texture's surface if it was lost; returns FALSE if it was (its texels are gone).
// FUNCTION: MW2S3 0x10010090
MechS32 FUN_10010090(A3DCachedTexture* p_texture)
{
	if (IDirectDrawSurface_IsLost(p_texture->m_surface) == DDERR_SURFACELOST) {
		DebugPrint("A3D: texture surface lost");
		IDirectDrawSurface_Restore(p_texture->m_surface);
		if (IDirectDrawSurface_IsLost(p_texture->m_surface) == DDERR_SURFACELOST) {
			IDirectDrawSurface_Restore(p_texture->m_surface);
		}

		if (!FUN_1000fc80(p_texture)) {
			DebugPrint("A3D: Lock texture failed.");
		}

		if (!FUN_1000fcd0(p_texture)) {
			DebugPrint("A3D: Unlock texture failed.");
		}

		return FALSE;
	}

	return TRUE;
}

// Creates the textures: four of each size from 16 to 128 texels square, two of the largest
// mip-mapped. Returns TRUE.
// FUNCTION: MW2S3 0x10010110
MechS32 FUN_10010110(void)
{
	MechU32 i;
	MechU32 log;

	for (i = 0; i < 4; i++) {
		for (log = 7; log >= 4; log--) {
			FUN_1000fbb0(log, i < 2 && log == 7);
		}
	}

	return TRUE;
}

// FUNCTION: MW2S3 0x10010150
void FUN_10010150(void)
{
	MechS32 log;

	FUN_1000fcf0(7);
	for (log = 4; log <= 7; log++) {
		FUN_1000fcf0(log);
	}
}

// Returns the toolkit surface of a texture holding the p_width x p_height texels p_pixels, as
// p_flags says (FUN_1000fa10), copying them into one if no texture holds them or they changed;
// NULL if there is none to take.
// FUNCTION: MW2S3 0x10010170
S3DTK_SURFACE* FUN_10010170(MechU16* p_pixels, MechU32 p_width, MechU32 p_height, MechU32 p_flags)
{
	A3DCachedTexture* texture;
	MechU32 size;
	MechU32 log;

	if (!g_a3dTexturesCreated) {
		g_a3dTexturesCreated = FUN_10010110();
	}

	size = p_height;
	if (p_height <= p_width) {
		size = p_width;
	}

	log = FUN_1000f9b0(size);
	if ((MechS32) (1 << log) < (MechS32) size) {
		log++;
	}

	texture = FUN_1000fff0(log, p_pixels);
	if (!texture) {
		if (!(p_flags & 1)) {
			texture = FUN_1000fbb0(log, p_flags);
		}

		if (!texture) {
			texture = FUN_10010010(log, p_flags);
		}

		if (texture) {
			A3D_TEXTURE_fill(texture, p_pixels, p_width, p_height, p_flags, log);
			texture->m_s3dSurface.sfFormat = S3DTK_TEXARGB4444;
			if (!(p_flags & 2)) {
				texture->m_s3dSurface.sfFormat = S3DTK_TEXARGB1555;
			}

			g_a3dTexture = NULL;
		}
	}
	else {
		g_a3dTextureHits++;
		if (!FUN_10010090(texture) || texture->m_lastUsed < g_a3dPaletteClock) {
			A3D_TEXTURE_fill(texture, p_pixels, p_width, p_height, p_flags, log);
			FUN_1000fb90(texture);
			FUN_1000fb20(texture, log, p_flags);
		}
	}

	if (texture) {
		texture->m_lastUsed = g_a3dClock++;
		return &texture->m_s3dSurface;
	}

	return NULL;
}

// Sets the texture filter state p_filter, or keeps it for when there is a renderer.
#define A3D_SET_FILTER(p_filter)                                                                                       \
	if (g_a3dRenderer) {                                                                                               \
		if (g_a3dFilter != (p_filter)) {                                                                               \
			g_a3dFilter = (p_filter);                                                                                  \
			g_a3dRenderer->S3DTK_SetState(g_a3dRenderer, S3DTK_TEXFILTERINGMODE, (p_filter));                          \
		}                                                                                                              \
	}                                                                                                                  \
	else {                                                                                                             \
		g_a3dPendingFilter = (p_filter);                                                                               \
	}

// Takes the "Filter" setting: nonzero reads 4 texels per pixel, 0 one, in the current filter's
// kind (mip-mapped or not). Turning the filter on from the mip-mapped point filter sets the
// mip-mapped 4-texel one and then the plain one.
// FUNCTION: MW2S3 0x100102b0
void FUN_100102b0(MechS32 p_filter)
{
	if (p_filter) {
		switch (g_a3dFilter) {
		case S3DTK_TEXM1TPP:
			A3D_SET_FILTER(S3DTK_TEXM4TPP);
		case S3DTK_TEX1TPP:
			A3D_SET_FILTER(S3DTK_TEX4TPP);
			break;
		}
	}
	else {
		switch (g_a3dFilter) {
		case S3DTK_TEXM2TPP:
			A3D_SET_FILTER(S3DTK_TEXM1TPP);
			break;
		case S3DTK_TEXM4TPP:
			A3D_SET_FILTER(S3DTK_TEXM1TPP);
			break;
		case S3DTK_TEXM8TPP:
			A3D_SET_FILTER(S3DTK_TEXM1TPP);
			break;
		case S3DTK_TEXV2TPP:
			A3D_SET_FILTER(S3DTK_TEX1TPP);
			break;
		case S3DTK_TEX4TPP:
			A3D_SET_FILTER(S3DTK_TEX1TPP);
			break;
		}
	}
}

// FUNCTION: MW2S3 0x10010470
void FUN_10010470(MechS32 p_mipmapped)
{
	g_a3dMipmapped = p_mipmapped;
}

// Sets the mip-mapped kind of the current filter when p_mip, the plain kind otherwise. The plain
// filters all turn into the plain 4-texel one.
// FUNCTION: MW2S3 0x10010480
void FUN_10010480(MechS32 p_mip)
{
	if (p_mip) {
		switch (g_a3dFilter) {
		case S3DTK_TEX1TPP:
			A3D_SET_FILTER(S3DTK_TEXM1TPP);
			break;
		case S3DTK_TEXV2TPP:
			A3D_SET_FILTER(S3DTK_TEXM2TPP);
			break;
		case S3DTK_TEX4TPP:
			A3D_SET_FILTER(S3DTK_TEXM4TPP);
			break;
		}
	}
	else {
		switch (g_a3dFilter) {
		case S3DTK_TEXM1TPP:
			A3D_SET_FILTER(S3DTK_TEX1TPP);
			break;
		case S3DTK_TEXM2TPP:
			A3D_SET_FILTER(S3DTK_TEX4TPP);
			break;
		case S3DTK_TEXM4TPP:
			A3D_SET_FILTER(S3DTK_TEX4TPP);
			break;
		case S3DTK_TEXM8TPP:
			A3D_SET_FILTER(S3DTK_TEX4TPP);
			break;
		}
	}
}

// Sets the state p_state to p_value, as cached in p_cache, if it isn't.
#define A3D_SET_STATE(p_cache, p_state, p_value)                                                                       \
	if ((p_cache) != (p_value)) {                                                                                      \
		(p_cache) = (p_value);                                                                                         \
		g_a3dRenderer->S3DTK_SetState(g_a3dRenderer, (p_state), (ULONG) (p_value));                                    \
	}

// Draws the shaded polygon p_vertices of p_count vertices. The first one also sets the states of
// a lit texture, once.
// FUNCTION: MW2S3 0x10010650
void A3D_DrawPolygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices)
{
	A3DVertex** fan;

	if (!FUN_1000f8a0(p_count)) {
		return;
	}

	fan = FUN_1000f940(p_vertices);
	if (g_a3dFirstPolygon) {
		A3D_SET_STATE(g_a3dTexture, S3DTK_TEXTUREACTIVE, NULL);
		A3D_SET_STATE(g_a3dRenderingType, S3DTK_RENDERINGTYPE, S3DTK_LITTEXTURE);
		A3D_SET_STATE(g_a3dTexBlending, S3DTK_TEXBLENDINGMODE, S3DTK_TEXMODULATE);
		A3D_SET_STATE(g_a3dAlphaBlending, S3DTK_ALPHABLENDING, S3DTK_ALPHATEXTURE);
		g_a3dFirstPolygon = FALSE;
	}

	A3D_SET_STATE(g_a3dRenderingType, S3DTK_RENDERINGTYPE, S3DTK_GOURAUD);
	A3D_SET_STATE(g_a3dAlphaBlending, S3DTK_ALPHABLENDING, S3DTK_ALPHAOFF);
	A3D_SET_STATE(g_a3dTexBlending, S3DTK_TEXBLENDINGMODE, S3DTK_TEXDECAL);
	g_a3dTexture = NULL;
	FUN_1000f8f0(g_a3dRenderer->S3DTK_TriangleSet(g_a3dRenderer, (ULONG*) fan, p_count, S3DTK_TRIFAN));
}

// FUNCTION: MW2S3 0x100107a0
void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, WINDOW* p_texture, MechU32 p_flags)
{
	A3DVertex** fan;
	S3DTK_SURFACE* texture;
	MechU32 translucent;
	MechS32 mip;

	if (!FUN_1000f8a0(p_count)) {
		return;
	}

	translucent = p_flags & 2;
	mip = !translucent && g_a3dMipmapped;
	texture = FUN_10010170(
		(MechU16*) p_texture->m_buffer,
		p_texture->m_xMax + 1,
		p_texture->m_yMax + 1,
		(translucent ? 2 : 0) | (mip ? 1 : 0)
	);
	if (!texture) {
		DebugPrint("A3D_map_polygon(): No texture\n");
		return;
	}

	fan = FUN_1000f940(p_vertices);
	FUN_10010480(mip);
	if (p_flags & 1) {
		A3D_SET_STATE(g_a3dRenderingType, S3DTK_RENDERINGTYPE, S3DTK_LITTEXTURE);
	}
	else {
		A3D_SET_STATE(g_a3dRenderingType, S3DTK_RENDERINGTYPE, S3DTK_UNLITTEXTURE);
	}

	A3D_SET_STATE(g_a3dTexture, S3DTK_TEXTUREACTIVE, texture);
	A3D_SET_STATE(g_a3dTexBlending, S3DTK_TEXBLENDINGMODE, S3DTK_TEXMODULATE);
	if (translucent) {
		A3D_SET_STATE(g_a3dAlphaBlending, S3DTK_ALPHABLENDING, S3DTK_ALPHATEXTURE);
	}
	else {
		A3D_SET_STATE(g_a3dAlphaBlending, S3DTK_ALPHABLENDING, S3DTK_ALPHAOFF);
	}

	FUN_1000f8f0(g_a3dRenderer->S3DTK_TriangleSet(g_a3dRenderer, (ULONG*) fan, p_count, S3DTK_TRIFAN));
}

// FUNCTION: MW2S3 0x10010930
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
	A3DVertex** fan;
	S3DTK_SURFACE* texture;
	MechU32 translucent;
	MechS32 mip;

	if (!FUN_1000f8a0(p_count)) {
		return;
	}

	translucent = p_flags & 1;
	mip = !translucent && g_a3dMipmapped;
	texture = FUN_10010170(
		*p_texture->m_rows,
		p_texture->m_width,
		p_texture->m_height,
		(translucent ? 2 : 0) | (mip ? 1 : 0)
	);
	if (!texture) {
		DebugPrint("No texture\n");
		return;
	}

	fan = FUN_1000f940(p_vertices);
	FUN_10010480(mip);
	if (p_flags & 0x600) {
		A3D_SET_STATE(g_a3dRenderingType, S3DTK_RENDERINGTYPE, S3DTK_LITTEXTUREPERSPECT);
	}
	else {
		A3D_SET_STATE(g_a3dRenderingType, S3DTK_RENDERINGTYPE, S3DTK_LITTEXTURE);
	}

	A3D_SET_STATE(g_a3dTexBlending, S3DTK_TEXBLENDINGMODE, S3DTK_TEXMODULATE);
	A3D_SET_STATE(g_a3dTexture, S3DTK_TEXTUREACTIVE, texture);
	if (translucent) {
		A3D_SET_STATE(g_a3dAlphaBlending, S3DTK_ALPHABLENDING, S3DTK_ALPHATEXTURE);
	}
	else {
		A3D_SET_STATE(g_a3dAlphaBlending, S3DTK_ALPHABLENDING, S3DTK_ALPHAOFF);
	}

	FUN_1000f8f0(g_a3dRenderer->S3DTK_TriangleSet(g_a3dRenderer, (ULONG*) fan, p_count, S3DTK_TRIFAN));
}

// "A3D_shutdown()": destroys the renderer and closes the toolkit.
// FUNCTION: MW2S3 0x10010ac0
void A3D_shutdown(void)
{
	if (g_a3dRenderer) {
		DebugPrint("A3D: DestroyRenderer returned %d\n", S3DTK_DestroyRenderer(&g_a3dRenderer));
		g_a3dRenderer = NULL;
	}

	if (g_a3dInitialized) {
		DebugPrint("A3D: DestroyRenderer returned %d\n", S3DTK_ExitLib());
		g_a3dInitialized = FALSE;
	}
}

// Puts in p_out the point where the edge from p_a to p_b crosses p_edge on the axis p_axis (1 x,
// 2 y), interpolating the colors (p_flags 1), the alpha (2), w (8) and the texture coordinates
// over w (4).
// FUNCTION: MW2S3 0x10010b20
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
	MechFloat w;

	w = 1.0f;
	switch (p_axis) {
	case 1:
		t = (p_edge - p_a->m_x) / (p_b->m_x - p_a->m_x);
		p_out->m_x = p_edge;
		p_out->m_y = (p_b->m_y - p_a->m_y) * t + p_a->m_y;
		break;
	case 2:
		t = (p_edge - p_a->m_y) / (p_b->m_y - p_a->m_y);
		p_out->m_y = p_edge;
		p_out->m_x = (p_b->m_x - p_a->m_x) * t + p_a->m_x;
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

	if (p_flags & 8) {
		w = (1.0f / p_b->m_w - 1.0f / p_a->m_w) * t + 1.0f / p_a->m_w;
		p_out->m_w = 1.0f / w;
	}

	if (p_flags & 4) {
		p_out->m_u = ((p_b->m_u / p_b->m_w - p_a->m_u / p_a->m_w) * t + p_a->m_u / p_a->m_w) / w;
		p_out->m_v = (p_a->m_v / p_a->m_w + (p_b->m_v / p_b->m_w - p_a->m_v / p_a->m_w) * t) / w;
	}
}
