/* The 3Dfx edition's A3D renderer (accel/a3d.h): the game's polygons drawn through Glide, one
   object of its own after accel/graphicsmenu.c's, compiled like the edition's other units. Its
   textures are VFX WINDOWs of 16-bit texels; it converts them to the Voodoo's ARGB 1555 (ARGB 4444
   with alpha) through Texus, or reads them converted from MW2.MIP, which the game writes on its
   first run (animation.c). Converted textures stay cached, most recently used first, and are
   downloaded to the texture memory when drawn; when it is full, a texture takes the memory of the
   least recently used one of the same layout, or the memory is cleared. The ground and the sky are
   subdivided into triangles small enough for their texture coordinates to stay accurate. Glide's
   frame buffer is written directly for the 2D drawing (FUN_10066930), and suspended while the
   polygons draw. */
#include "accel/a3d.h"

#include "accel/a3dvertex.h"
#include "decomp.h"
#include "palettecolor.h"
#include "pane.h"
#include "polydraw.h"
#include "types.h"
#include "window.h"

#include <glide.h>
#include <stdio.h>
#include <stdlib.h>
#include <texus.h>

// A converted texture: an item of the texture cache, a list from g_a3dTextureHead to
// g_a3dTextureTail, most recently used first.
// SIZE 0x438
typedef struct A3DCachedTexture A3DCachedTexture;
struct A3DCachedTexture {
	A3DCachedTexture* m_next; // 0x000
	A3DCachedTexture* m_prev; // 0x004
	MechS32 m_id;             // 0x008 — the WINDOW's m_buffer
	MechS32 m_clamp;          // 0x00c — the clamp mode it was last drawn with
	Gu3dfInfo m_info;         // 0x010
	GrMipMapId_t m_mipMap;    // 0x430 — in the texture memory; GR_NULL_MIPMAP_HANDLE if not
	MechS32 m_alpha;          // 0x434 — converted with alpha (ARGB 4444)
};

// A vertex of the polygons the ground and the sky are subdivided into, with the depth and the
// texture coordinates times it, which interpolate linearly on the screen.
// SIZE 0x4c
typedef struct A3DSubVertex {
	A3DVertex m_vertex;  // 0x00
	MechFloat m_depth;   // 0x3c — 1 / m_vertex.m_w
	MechFloat m_s;       // 0x40 — m_vertex.m_u times m_depth
	MechFloat m_t;       // 0x44
	MechFloat m_unk0x48; // 0x48
} A3DSubVertex;

DECOMP_SIZE_ASSERT(A3DCachedTexture, 0x438)
DECOMP_SIZE_ASSERT(A3DSubVertex, 0x4c)

MechS32 FUN_10066a30(Gu3dfInfo* p_info, MechS32* p_width, MechS32* p_height, void* p_data);
void FUN_10066e70(WINDOW* p_texture, MechS32 p_clamp);
MechS32 FUN_10067a90(A3DSubVertex* p_a, A3DSubVertex* p_b, A3DSubVertex* p_out);
MechS32 FUN_10067b60(A3DSubVertex* p_triangle);
MechS32 FUN_10067d00(A3DSubVertex* p_triangle, MechS32 p_level);
void FUN_10067e60(A3DVertex* p_vertices, MechS32 p_count);

// GLOBAL: MW23DFX 0x10265708
MechS32 g_a3dInitialized = FALSE;

// The color combine the drawing set last: 0 the vertex colors, 1 the texture times them.
// GLOBAL: MW23DFX 0x1026570c
MechS32 g_a3dColorCombine = 0;

// Textures convert with alpha (while MW2.MIP is written), or the polygon is translucent.
// GLOBAL: MW23DFX 0x10265710
MechS32 g_a3dTranslucent = FALSE;

// The texture drawn has alpha.
// GLOBAL: MW23DFX 0x10265714
MechS32 g_a3dTextureAlpha = FALSE;

// The texture selected when none is.
// GLOBAL: MW23DFX 0x10265718
A3DCachedTexture g_a3dNoTexture = {0};

// GLOBAL: MW23DFX 0x10265b50
A3DCachedTexture* g_a3dCurrentTexture = &g_a3dNoTexture;

// GLOBAL: MW23DFX 0x10265b58
A3DCachedTexture g_a3dTextureHead = {0};

// GLOBAL: MW23DFX 0x10265f90
A3DCachedTexture g_a3dTextureTail = {0};

// How often a texture was selected, and downloaded.
// GLOBAL: MW23DFX 0x102663c8
MechU32 g_a3dTextureSelects = 0;

// GLOBAL: MW23DFX 0x102663cc
MechU32 g_a3dTextureDownloads = 0;

// The converted textures' size.
// GLOBAL: MW23DFX 0x102663d0
MechU32 g_a3dTextureBytes = 0;

// Set when a texture couldn't be written to MW2.MIP.
// GLOBAL: MW23DFX 0x102663d4
MechS32 g_a3dMipError = 0;

// GLOBAL: MW23DFX 0x102663d8
FILE* g_a3dMip = NULL;

// GLOBAL: MW23DFX 0x102663dc
FILE* g_a3dLog = NULL;

// A palette component (0-63) to the 0-255 of the 800x600 display.
// GLOBAL: MW23DFX 0x102663e0
MechU16 g_a3dBrightness[0x100] = {
	0,   9,   14,  18,  22,  25,  28,  30,  33,  35,  37,  40,  42,  44,  46,  48,  50,  51,  53,  55,  57,  58,
	60,  61,  63,  65,  66,  68,  69,  70,  72,  73,  75,  76,  77,  79,  80,  81,  83,  84,  85,  87,  88,  89,
	90,  91,  93,  94,  95,  96,  97,  98,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 112, 113, 114,
	115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 127, 128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 139, 140, 141, 142, 143, 144, 145, 146, 147, 147, 148, 149, 150, 151, 152, 152, 153, 154,
	155, 156, 157, 157, 158, 159, 160, 161, 162, 162, 163, 164, 165, 166, 166, 167, 168, 169, 170, 170, 171, 172,
	173, 173, 174, 175, 176, 176, 177, 178, 179, 179, 180, 181, 182, 182, 183, 184, 185, 185, 186, 187, 188, 188,
	189, 190, 190, 191, 192, 193, 193, 194, 195, 195, 196, 197, 198, 198, 199, 200, 200, 201, 202, 202, 203, 204,
	205, 205, 206, 207, 207, 208, 209, 209, 210, 211, 211, 212, 213, 213, 214, 215, 215, 216, 217, 217, 218, 219,
	219, 220, 221, 221, 222, 222, 223, 224, 224, 225, 226, 226, 227, 228, 228, 229, 230, 230, 231, 231, 232, 233,
	233, 234, 235, 235, 236, 236, 237, 238, 238, 239, 239, 240, 241, 241, 242, 243, 243, 244, 244, 245, 246, 246,
	247, 247, 248, 249, 249, 250, 250, 251, 252, 252, 253, 253, 254, 255,
};

// The frame buffer is open for writing (FUN_10066930).
// GLOBAL: MW23DFX 0x102665e0
MechS32 g_a3dLfbOpen = FALSE;

// The 3D drawing closed it, to reopen it when done (FUN_100669d0).
// GLOBAL: MW23DFX 0x102665e4
MechS32 g_a3dLfbSuspended = TRUE;

// The polygons are drawing (FUN_100669a0).
// GLOBAL: MW23DFX 0x102665e8
MechS32 g_a3dDrawing = FALSE;

// GLOBAL: MW23DFX 0x102665ec
MechS32 g_a3dRes800x600 = FALSE;

// The ground's and the sky's subdivision stops at triangles this deep.
// GLOBAL: MW23DFX 0x1007dc78
static MechFloat g_a3dSubdivideLimit;

// MW2.MIP's end, where the next texture is written.
// GLOBAL: MW23DFX 0x1007dc7c
static MechU32 g_a3dMipEnd;

// The clamp mode set last.
// GLOBAL: MW23DFX 0x1007dc80
static MechS32 g_a3dClampMode;

// GLOBAL: MW23DFX 0x1007dc88
static A3DMipEntry g_a3dMipEntries[0x51c];

// The rendering state the translucent polygons restore.
// GLOBAL: MW23DFX 0x100819d8
static GrState g_a3dState;

// The polygon drawn, moved by the pane's position.
// GLOBAL: MW23DFX 0x10081b10
static A3DVertex g_a3dPolygon[20];

// The pane's position, plus 2^19: adding it rounds the coordinates to the Voodoo's 4 fractional
// bits.
// GLOBAL: MW23DFX 0x10081fc0
static MechFloat g_a3dOffsetX;

// GLOBAL: MW23DFX 0x10081fc4
static MechFloat g_a3dOffsetY;

// The frame buffer, for the 2D drawing.
// GLOBAL: MW23DFX 0x101528b8
undefined* g_a3dLfb;

// The counters of the loop and of the opened display are in each other's registers (register
// entropy), and the end of g_a3dMipEntries names the static after it in either build (2.2 lays
// statics out in an order of its own).
// FUNCTION: MW23DFX 0x10066570
MechS32 A3D_Init(void)
{
	GrHwConfiguration hardware;
	GrScreenResolution_t resolution;
	MechS32 opened;
	MechS32 tries;
	MechS32 i;

	g_a3dLog = fopen("mw2err.log", "w+");
	resolution = g_a3dRes800x600 ? GR_RESOLUTION_800x600 : GR_RESOLUTION_640x480;
	opened = FALSE;
	for (tries = 0; tries < 10; tries++) {
		grGlideInit();
		if (!grSstQueryHardware(&hardware)) {
			grGlideShutdown();
			if (g_a3dLog) {
				fprintf(g_a3dLog, "grQueryHardware failed\n");
			}
		}
		else {
			grSstSelect(0);
			if (grSstOpen(
					resolution,
					GR_REFRESH_60Hz,
					GR_COLORFORMAT_ARGB,
					GR_ORIGIN_UPPER_LEFT,
					GR_SMOOTHING_ENABLE,
					2
				)) {
				opened = TRUE;
				break;
			}

			if (g_a3dLog) {
				fprintf(g_a3dLog, "grOpen failed\n");
			}

			grGlideShutdown();
		}
	}

	if (!opened) {
		if (g_a3dLog) {
			fclose(g_a3dLog);
			g_a3dLog = NULL;
		}

		return 1;
	}

	g_a3dCurrentTexture = &g_a3dNoTexture;
	g_a3dTextureHead.m_next = &g_a3dTextureTail;
	g_a3dNoTexture.m_id = 0;
	g_a3dTextureHead.m_prev = NULL;
	g_a3dTextureTail.m_next = NULL;
	g_a3dTextureTail.m_prev = &g_a3dTextureHead;
	g_a3dInitialized = TRUE;
	grBufferClear(0, 0, 0xffff);
	guColorCombineFunction(GR_COLORCOMBINE_ITRGB);
	grTexCombineFunction(GR_TMU0, GR_TEXTURECOMBINE_ZERO);
	g_a3dLfb = grLfbGetWritePtr(GR_BUFFER_BACKBUFFER);
	grLfbWriteMode(GR_LFBWRITEMODE_555);
	grChromakeyMode(GR_CHROMAKEY_DISABLE);
	for (i = 0; i < 0x51c; i++) {
		g_a3dMipEntries[i].m_offset = -1;
	}

	g_a3dMip = fopen("mw2.mip", "rb");
	if (g_a3dMip && fread(g_a3dMipEntries, sizeof(g_a3dMipEntries), 1, g_a3dMip) != 1) {
		fclose(g_a3dMip);
	}

	return 0;
}

// FUNCTION: MW23DFX 0x10066720
void A3D_shutdown(void)
{
	A3DCachedTexture* texture;
	A3DCachedTexture* next;

	if (!g_a3dInitialized) {
		return;
	}

	if (g_a3dLog) {
		fclose(g_a3dLog);
		g_a3dLog = NULL;
	}

	g_a3dInitialized = FALSE;
	if (g_a3dLfbOpen) {
		FUN_10066970();
	}

	grGlideShutdown();
	if (g_a3dMip) {
		fclose(g_a3dMip);
	}

	g_a3dMip = NULL;
	for (texture = g_a3dTextureHead.m_next; texture != &g_a3dTextureTail; texture = next) {
		next = texture->m_next;
		free(texture->m_info.data);
		free(texture);
	}

	g_a3dTextureHead.m_next = &g_a3dTextureTail;
	g_a3dTextureTail.m_prev = &g_a3dTextureHead;
	g_a3dTextureHead.m_prev = NULL;
	g_a3dTextureTail.m_next = NULL;
	g_a3dTextureBytes = 0;
}

// FUNCTION: MW23DFX 0x100667e0
void FUN_100667e0(MechS32 p_setting)
{
}

// FUNCTION: MW23DFX 0x100667f0
void FUN_100667f0(void)
{
	if (g_a3dDrawing) {
		FUN_100669d0();
	}
}

// FUNCTION: MW23DFX 0x10066800
MechFloat FUN_10066800(MechU8 p_component)
{
	if (p_component < 0x100) {
		return g_a3dBrightness[p_component];
	}

	return 255.0f;
}

// Sets g_paletteRgb: at 800x600 through g_a3dBrightness, at 640x480 times 4.
// One load is scheduled earlier, and the end of g_paletteRgb names the next global of either
// build.
// FUNCTION: MW23DFX 0x10066840
void A3D_SetPalette(PaletteColor* p_palette)
{
	MechFloat* color;

	if (g_a3dRes800x600) {
		for (color = g_paletteRgb[0]; color < g_paletteRgb[0x100]; color += 3) {
			color[0] = FUN_10066800(p_palette->m_red);
			color[1] = FUN_10066800(p_palette->m_green);
			color[2] = FUN_10066800(p_palette->m_blue);
			p_palette++;
		}

		return;
	}

	for (color = g_paletteRgb[0]; color < g_paletteRgb[0x100]; color += 3) {
		color[0] = p_palette->m_red * 4;
		if (color[0] > 255.0f) {
			color[0] = 255.0f;
		}

		color[1] = p_palette->m_green * 4;
		if (color[1] > 255.0f) {
			color[1] = 255.0f;
		}

		color[2] = p_palette->m_blue * 4;
		if (color[2] > 255.0f) {
			color[2] = 255.0f;
		}

		p_palette++;
	}
}

// FUNCTION: MW23DFX 0x10066910
void FUN_10066910(void)
{
}

// FUNCTION: MW23DFX 0x10066920
void FUN_10066920(MechS32 p_res800x600)
{
	g_a3dRes800x600 = p_res800x600;
}

// FUNCTION: MW23DFX 0x10066930
void FUN_10066930(void)
{
	MechS32 wait;

	g_a3dLfbOpen = TRUE;
	wait = 0xfffff;
	while (wait-- && grSstIsBusy()) {
	}

	grLfbBegin();
	grLfbBypassMode(GR_LFBBYPASS_ENABLE);
	g_a3dLfbSuspended = FALSE;
}

// FUNCTION: MW23DFX 0x10066970
void FUN_10066970(void)
{
	g_a3dLfbOpen = FALSE;
	grLfbBypassMode(GR_LFBBYPASS_DISABLE);
	grLfbEnd();
	g_a3dLfbSuspended = FALSE;
}

// FUNCTION: MW23DFX 0x100669a0
void FUN_100669a0(void)
{
	g_a3dDrawing = TRUE;
	if (g_a3dLfbOpen) {
		g_a3dLfbSuspended = TRUE;
		grLfbBypassMode(GR_LFBBYPASS_DISABLE);
		grLfbEnd();
	}
}

// FUNCTION: MW23DFX 0x100669d0
void FUN_100669d0(void)
{
	g_a3dDrawing = FALSE;
	if (g_a3dLfbSuspended) {
		FUN_10066930();
	}
}

// FUNCTION: MW23DFX 0x100669f0
A3DMipEntry* FUN_100669f0(MechS16 p_id)
{
	if (p_id < 1 || p_id >= 0x51c || !g_a3dMip || g_a3dMipEntries[p_id].m_offset == -1) {
		return NULL;
	}

	return &g_a3dMipEntries[p_id];
}

// Converts the texture p_data of *p_width x *p_height texels into p_info, or reads it from MW2.MIP
// when p_data is the m_offset of its entry (DrawAnimatedPolygon's textures from the file). Returns
// nonzero on failure.
// The fseek arguments are scheduled in another order, and the end of g_a3dMipEntries names the
// static after it in either build.
// FUNCTION: MW23DFX 0x10066a30
MechS32 FUN_10066a30(Gu3dfInfo* p_info, MechS32* p_width, MechS32* p_height, void* p_data)
{
	A3DMipEntry* entry;

	p_info->data = NULL;
	entry = (A3DMipEntry*) ((MechU8*) p_data - 4);
	if (entry >= g_a3dMipEntries && entry < &g_a3dMipEntries[0x51c]) {
		fseek(g_a3dMip, g_a3dMipEntries[entry - g_a3dMipEntries].m_offset, SEEK_SET);
		if (fread(p_info, sizeof(Gu3dfInfo), 1, g_a3dMip) != 1) {
			return 1;
		}

		p_info->data = malloc(p_info->mem_required);
		if (!p_info->data) {
			return 1;
		}

		if (fread(p_info->data, p_info->mem_required, 1, g_a3dMip) != 1) {
			free(p_info->data);
			return 1;
		}

		return 0;
	}

	p_info->data = NULL;
	if (g_a3dTranslucent) {
		txInit3dfInfo(p_info, GR_TEXFMT_ARGB_4444, p_width, p_height, -1, TX_AUTORESIZE_GROW);
		txConvert(p_info, GR_TEXFMT_ARGB_4444, *p_width, *p_height, p_data, TX_DITHER_NONE, &p_info->table);
	}
	else {
		txInit3dfInfo(p_info, GR_TEXFMT_ARGB_1555, p_width, p_height, -1, TX_AUTORESIZE_GROW);
		txConvert(p_info, GR_TEXFMT_ARGB_1555, *p_width, *p_height, p_data, TX_DITHER_NONE, &p_info->table);
	}

	return p_info->data == NULL;
}

// The end of g_a3dMipEntries names the static after it in either build.
// FUNCTION: MW23DFX 0x10066b90
MechS32 FUN_10066b90(void)
{
	FILE* file;
	MechS32 i;

	file = fopen("mw2.mip", "rb");
	if (file) {
		fclose(file);
		return 1;
	}

	for (i = 0; i < 0x51c; i++) {
		g_a3dMipEntries[i].m_offset = -1;
		g_a3dMipEntries[i].m_size = 0;
		g_a3dMipEntries[i].m_height = 0;
		g_a3dMipEntries[i].m_width = 0;
	}

	g_a3dMip = fopen("mw2.mip", "wb");
	g_a3dMipEnd = sizeof(g_a3dMipEntries);
	fseek(g_a3dMip, sizeof(g_a3dMipEntries), SEEK_SET);
	g_a3dMipError = 0;
	g_a3dTranslucent = TRUE;
	return 0;
}

// FUNCTION: MW23DFX 0x10066c20
void FUN_10066c20(void)
{
	g_a3dTranslucent = FALSE;
	if (g_a3dMip) {
		g_a3dMipEnd = 0;
		fseek(g_a3dMip, 0, SEEK_SET);
		fwrite(g_a3dMipEntries, sizeof(g_a3dMipEntries), 1, g_a3dMip);
		fclose(g_a3dMip);
	}
}

// Only 128x128 textures go into the file; the others are converted when drawn.
// FUNCTION: MW23DFX 0x10066c70
MechS32 FUN_10066c70(void* p_pixels, MechS32 p_width, MechS32 p_height, MechS32 p_id)
{
	Gu3dfInfo info;

	if (p_id < 1 || p_id >= 0x51c) {
		return 1;
	}

	if (p_width != p_height || p_width != 0x80) {
		return 0;
	}

	info.data = NULL;
	g_a3dMipEntries[p_id].m_offset = g_a3dMipEnd;
	g_a3dMipEntries[p_id].m_width = p_width;
	g_a3dMipEntries[p_id].m_height = p_height;
	if (g_a3dTranslucent) {
		txInit3dfInfo(&info, GR_TEXFMT_ARGB_4444, &p_width, &p_height, -1, TX_AUTORESIZE_GROW);
		txConvert(&info, GR_TEXFMT_ARGB_4444, p_width, p_height, p_pixels, TX_DITHER_NONE, &info.table);
	}
	else {
		txInit3dfInfo(&info, GR_TEXFMT_ARGB_1555, &p_width, &p_height, -1, TX_AUTORESIZE_GROW);
		txConvert(&info, GR_TEXFMT_ARGB_1555, p_width, p_height, p_pixels, TX_DITHER_NONE, &info.table);
	}

	if (!info.data) {
		g_a3dMipError = 1;
		return 1;
	}

	if (fwrite(&info, sizeof(info), 1, g_a3dMip) != 1) {
		g_a3dMipError = 1;
		return 1;
	}

	if (fwrite(info.data, info.mem_required, 1, g_a3dMip) != 1) {
		g_a3dMipError = 1;
		return 1;
	}

	g_a3dMipEntries[p_id].m_size = info.mem_required + sizeof(info);
	g_a3dMipEnd += info.mem_required + sizeof(info);
	free(info.data);
	return 0;
}

// Makes p_texture the texture drawn, with the clamp mode p_clamp.
// Register allocation: the original keeps reset in ebp, rereads p_texture->m_buffer where this
// keeps it in a register, and has a smaller frame.
// FUNCTION: MW23DFX 0x10066e70
void FUN_10066e70(WINDOW* p_texture, MechS32 p_clamp)
{
	A3DCachedTexture* texture;
	A3DCachedTexture* other;
	MechS32 reset;
	MechS32 width;
	MechS32 height;
	MechS32 wait;

	reset = FALSE;
	if (!p_texture->m_buffer) {
		return;
	}

	if (g_a3dCurrentTexture->m_id == (MechS32) p_texture->m_buffer) {
		g_a3dClampMode = p_clamp;
		grTexClampMode(GR_TMU0, p_clamp, p_clamp);
		return;
	}

	g_a3dTextureSelects++;
	for (texture = g_a3dTextureHead.m_next; texture != &g_a3dTextureTail; texture = texture->m_next) {
		if (texture->m_id == (MechS32) p_texture->m_buffer) {
			break;
		}
	}

	if (texture != &g_a3dTextureTail && texture->m_mipMap != GR_NULL_MIPMAP_HANDLE) {
		texture->m_prev->m_next = texture->m_next;
		texture->m_next->m_prev = texture->m_prev;
		g_a3dTextureHead.m_next->m_prev = texture;
		texture->m_prev = &g_a3dTextureHead;
		texture->m_next = g_a3dTextureHead.m_next;
		g_a3dTextureHead.m_next = texture;
	}
	else {
		if (texture == &g_a3dTextureTail) {
			texture = malloc(sizeof(A3DCachedTexture));
			if (!texture) {
				return;
			}

			texture->m_id = (MechS32) p_texture->m_buffer;
			width = (MechS16) p_texture->m_xMax + 1;
			height = (MechS16) p_texture->m_yMax + 1;
			if (FUN_10066a30(&texture->m_info, &width, &height, p_texture->m_buffer)) {
				free(texture);
				return;
			}

			texture->m_alpha = TRUE;
			if (!g_a3dTranslucent) {
				texture->m_alpha = FALSE;
			}

			g_a3dTextureBytes += texture->m_info.mem_required;
		}
		else {
			texture->m_prev->m_next = texture->m_next;
			texture->m_next->m_prev = texture->m_prev;
		}

		g_a3dTextureHead.m_next->m_prev = texture;
		texture->m_prev = &g_a3dTextureHead;
		texture->m_next = g_a3dTextureHead.m_next;
		g_a3dTextureHead.m_next = texture;
		for (;;) {
			texture->m_clamp = p_clamp;
			texture->m_mipMap = guTexAllocateMemory(
				GR_TMU0,
				GR_MIPMAPLEVELMASK_BOTH,
				texture->m_info.header.width,
				texture->m_info.header.height,
				texture->m_info.header.format,
				GR_MIPMAP_NEAREST_DITHER,
				texture->m_info.header.small_lod,
				texture->m_info.header.large_lod,
				texture->m_info.header.aspect_ratio,
				p_clamp,
				p_clamp,
				GR_TEXTUREFILTER_BILINEAR,
				GR_TEXTUREFILTER_BILINEAR,
				0,
				FXFALSE
			);
			if (texture->m_mipMap != GR_NULL_MIPMAP_HANDLE) {
				break;
			}

			if (reset == TRUE) {
				texture->m_prev->m_next = texture->m_next;
				texture->m_next->m_prev = texture->m_prev;
				free(texture->m_info.data);
				free(texture);
				return;
			}

			for (other = g_a3dTextureTail.m_prev; other != &g_a3dTextureHead; other = other->m_prev) {
				if (other != texture && other->m_mipMap != GR_NULL_MIPMAP_HANDLE &&
					other->m_info.mem_required == texture->m_info.mem_required &&
					other->m_info.header.width == texture->m_info.header.width &&
					other->m_info.header.height == texture->m_info.header.height &&
					other->m_info.header.format == texture->m_info.header.format &&
					other->m_info.header.small_lod == texture->m_info.header.small_lod &&
					other->m_info.header.large_lod == texture->m_info.header.large_lod &&
					other->m_info.header.aspect_ratio == texture->m_info.header.aspect_ratio) {
					break;
				}
			}

			if (other != &g_a3dTextureHead) {
				texture->m_mipMap = other->m_mipMap;
				other->m_mipMap = GR_NULL_MIPMAP_HANDLE;
				texture->m_clamp = other->m_clamp;
				break;
			}

			for (other = g_a3dTextureHead.m_next; other != &g_a3dTextureTail; other = other->m_next) {
				other->m_mipMap = GR_NULL_MIPMAP_HANDLE;
			}

			reset = TRUE;
			guTexMemReset();
		}

		g_a3dTextureDownloads++;
		wait = 0xfffff;
		while (wait-- && grSstIsBusy()) {
		}

		guTexDownloadMipMap(texture->m_mipMap, texture->m_info.data, &texture->m_info.table.nccTable);
	}

	guTexSource(texture->m_mipMap);
	g_a3dCurrentTexture = texture;
	g_a3dTextureAlpha = texture->m_alpha;
	g_a3dClampMode = texture->m_clamp;
	if (g_a3dClampMode != p_clamp) {
		g_a3dClampMode = p_clamp;
		grTexClampMode(GR_TMU0, p_clamp, p_clamp);
	}
}

// FUNCTION: MW23DFX 0x10067180
void A3D_DrawPolygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices)
{
	MechU32 i;

	if (p_count <= 20) {
		if (!g_a3dDrawing) {
			FUN_100669a0();
		}

		if (g_a3dColorCombine != 0) {
			guColorCombineFunction(GR_COLORCOMBINE_ITRGB);
			g_a3dColorCombine = 0;
		}

		g_a3dOffsetX = p_pane->m_x0 + 524288.0f;
		g_a3dOffsetY = p_pane->m_y0 + 524288.0f;
		for (i = 0; i < p_count; i++) {
			g_a3dPolygon[i] = p_vertices[i];
			g_a3dPolygon[i].m_x += g_a3dOffsetX;
			g_a3dPolygon[i].m_y += g_a3dOffsetY;
		}

		grDrawPolygonVertexList(p_count, (GrVertex*) g_a3dPolygon);
	}
}

// Translucent polygons blend with the texture's alpha, or with 230.4 (90%) for textures
// without.
// FUNCTION: MW23DFX 0x10067240
void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, WINDOW* p_texture, MechU32 p_flags)
{
	MechU32 i;

	if (p_count <= 20) {
		if (!g_a3dDrawing) {
			FUN_100669a0();
		}

		g_a3dTranslucent = (p_flags & 2) >> 1;
		if (g_a3dColorCombine != 1) {
			guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
			grTexCombineFunction(GR_TMU0, GR_TEXTURECOMBINE_DECAL);
			g_a3dColorCombine = 1;
		}

		if (g_a3dCurrentTexture->m_id != (MechS32) p_texture->m_buffer || g_a3dClampMode != GR_TEXTURECLAMP_CLAMP) {
			FUN_10066e70(p_texture, GR_TEXTURECLAMP_CLAMP);
		}

		g_a3dOffsetX = p_pane->m_x0 + 524288.0f;
		g_a3dOffsetY = p_pane->m_y0 + 524288.0f;
		if (g_a3dTranslucent) {
			if (g_a3dTextureAlpha) {
				grGlideGetState(&g_a3dState);
				grAlphaCombine(
					GR_COMBINE_FUNCTION_SCALE_OTHER,
					GR_COMBINE_FACTOR_TEXTURE_ALPHA,
					GR_COMBINE_LOCAL_CONSTANT,
					GR_COMBINE_OTHER_TEXTURE,
					FXFALSE
				);
				grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ZERO, GR_BLEND_ZERO);
			}
			else {
				grGlideGetState(&g_a3dState);
				guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
				grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ONE, GR_BLEND_ZERO);
				grAlphaTestFunction(GR_CMP_ALWAYS);
			}
		}

		if (!g_a3dTranslucent || g_a3dTextureAlpha) {
			for (i = 0; i < p_count; i++) {
				g_a3dPolygon[i] = p_vertices[i];
				g_a3dPolygon[i].m_x += g_a3dOffsetX;
				g_a3dPolygon[i].m_y += g_a3dOffsetY;
			}
		}
		else {
			for (i = 0; i < p_count; i++) {
				g_a3dPolygon[i] = p_vertices[i];
				g_a3dPolygon[i].m_x += g_a3dOffsetX;
				g_a3dPolygon[i].m_y += g_a3dOffsetY;
				g_a3dPolygon[i].m_alpha = 230.4f;
			}
		}

		grDrawPolygonVertexList(p_count, (GrVertex*) g_a3dPolygon);
		if (g_a3dTranslucent) {
			grGlideSetState(&g_a3dState);
			g_a3dTranslucent = FALSE;
		}
	}
}

// Draws the polygon's edges but the closing one.
// FUNCTION: MW23DFX 0x10067410
void A3D_DrawOutline(PANE* p_pane, A3DVertex* p_vertices, MechU32 p_count)
{
	MechU32 i;

	if (p_count <= 20) {
		g_a3dOffsetX = p_pane->m_x0 + 524288.0f;
		g_a3dOffsetY = p_pane->m_y0 + 524288.0f;
		for (i = 0; i < p_count; i++) {
			g_a3dPolygon[i] = p_vertices[i];
			g_a3dPolygon[i].m_x += g_a3dOffsetX;
			g_a3dPolygon[i].m_y += g_a3dOffsetY;
		}

		if (!g_a3dDrawing) {
			FUN_100669a0();
		}

		for (i = 0; i < p_count - 1; i++) {
			grDrawLine((GrVertex*) &g_a3dPolygon[i], (GrVertex*) &g_a3dPolygon[i + 1]);
		}
	}
}

// p_mode 0 draws the polygon's outline, 1 the polygon with 1 / w scaled to its largest, and 2 the
// polygon subdivided where its depth varies much.
// The registers differ, and the original keeps subdivided in no register or slot: its paths jump
// straight to the draw or past it (with the draw in each case instead, the frame is a slot short).
// FUNCTION: MW23DFX 0x100674c0
void FUN_100674c0(
	PANE* p_pane,
	MechS32 p_count,
	A3DVertex* p_vertices,
	WINDOW* p_texture,
	MechU32 p_flags,
	MechS32 p_mode
)
{
	MechFloat minDepth;
	MechFloat minY;
	MechFloat maxY;
	MechFloat maxRatio;
	MechS32 nearest;
	A3DSubVertex triangle[3];
	A3DVertex polygon[20];
	A3DSubVertex vertices[20];
	MechS32 i;
	MechS32 j;
	MechS32 k;
	MechFloat w;
	MechS32 span;
	MechS32 subdivided;

	subdivided = FALSE;
	g_a3dOffsetX = p_pane->m_x0 + 524288.0f;
	g_a3dOffsetY = p_pane->m_y0 + 524288.0f;
	if (!g_a3dDrawing) {
		FUN_100669a0();
	}

	g_a3dTranslucent = (p_flags & 2) >> 1;
	if (g_a3dColorCombine != 1) {
		guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
		grTexCombineFunction(GR_TMU0, GR_TEXTURECOMBINE_DECAL);
		g_a3dColorCombine = 1;
	}

	if (g_a3dCurrentTexture->m_id != (MechS32) p_texture->m_buffer || g_a3dClampMode != GR_TEXTURECLAMP_WRAP) {
		FUN_10066e70(p_texture, GR_TEXTURECLAMP_WRAP);
	}

	if (!g_a3dTextureAlpha) {
		g_a3dTranslucent = FALSE;
	}

	if (g_a3dTranslucent) {
		if (g_a3dTextureAlpha) {
			grGlideGetState(&g_a3dState);
			grAlphaCombine(
				GR_COMBINE_FUNCTION_SCALE_OTHER,
				GR_COMBINE_FACTOR_TEXTURE_ALPHA,
				GR_COMBINE_LOCAL_CONSTANT,
				GR_COMBINE_OTHER_TEXTURE,
				FXFALSE
			);
			grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ZERO, GR_BLEND_ZERO);
		}
		else {
			grGlideGetState(&g_a3dState);
			guAlphaSource(GR_ALPHASOURCE_ITERATED_ALPHA);
			grAlphaBlendFunction(GR_BLEND_SRC_ALPHA, GR_BLEND_ONE_MINUS_SRC_ALPHA, GR_BLEND_ONE, GR_BLEND_ZERO);
			grAlphaTestFunction(GR_CMP_ALWAYS);
		}
	}

	switch (p_mode) {
	case 0:
		for (i = 0; i < p_count; i++) {
			p_vertices[i].m_x += g_a3dOffsetX;
			p_vertices[i].m_y += g_a3dOffsetY;
		}

		for (i = 0; i < p_count - 1; i++) {
			grDrawLine((GrVertex*) &p_vertices[i], (GrVertex*) &p_vertices[i + 1]);
		}

		grDrawLine((GrVertex*) &p_vertices[p_count - 1], (GrVertex*) p_vertices);
		break;

	case 1:
		minDepth = 1.0f / p_vertices[0].m_w;
		for (i = 1; i < p_count; i++) {
			w = 1.0f / p_vertices[i].m_w;
			if (w < minDepth) {
				minDepth = w;
			}
		}

		for (i = 0; i < p_count; i++) {
			w = p_vertices[i].m_w;
			p_vertices[i].m_w = w * minDepth;
			p_vertices[i].m_tmuW = w * minDepth;
			p_vertices[i].m_u *= minDepth;
			p_vertices[i].m_v *= minDepth;
			p_vertices[i].m_x += g_a3dOffsetX;
			p_vertices[i].m_y += g_a3dOffsetY;
		}
		break;

	default:
		nearest = 0;
		for (i = 0; i < p_count; i++) {
			vertices[i].m_vertex = p_vertices[i];
			vertices[i].m_depth = 1.0f / vertices[i].m_vertex.m_w;
			vertices[i].m_s = vertices[i].m_depth * vertices[i].m_vertex.m_u;
			vertices[i].m_t = vertices[i].m_vertex.m_v * vertices[i].m_depth;
			if (i == 0) {
				minDepth = vertices[0].m_depth;
				maxY = minY = vertices[0].m_vertex.m_y;
			}
			else {
				if (vertices[i].m_depth < minDepth) {
					minDepth = vertices[i].m_depth;
					nearest = i;
				}

				if (vertices[i].m_vertex.m_y < minY) {
					minY = vertices[i].m_vertex.m_y;
				}

				if (maxY < vertices[i].m_vertex.m_y) {
					maxY = vertices[i].m_vertex.m_y;
				}
			}
		}

		maxRatio = 0;
		for (i = 0; i < p_count; i++) {
			vertices[i].m_depth /= minDepth;
			if (vertices[i].m_depth > maxRatio) {
				maxRatio = vertices[i].m_depth;
			}

			vertices[i].m_vertex.m_w = 1.0f / vertices[i].m_depth;
			vertices[i].m_vertex.m_u = vertices[i].m_s * vertices[i].m_vertex.m_w;
			vertices[i].m_vertex.m_v = vertices[i].m_t * vertices[i].m_vertex.m_w;
		}

		if (maxRatio > 2.0f) {
			span = (MechS32) (maxY - minY);
			if (span < 100) {
				g_a3dSubdivideLimit = 2.0f;
			}
			else if (span < 200) {
				g_a3dSubdivideLimit = 6.0f;
			}
			else {
				g_a3dSubdivideLimit = 8.0f;
				if (span >= 300) {
					g_a3dSubdivideLimit = 10.0f;
				}
			}

			FUN_10067e60(p_vertices, p_count);
			triangle[0] = vertices[nearest];
			for (i = nearest + 1; i < p_count - 1 + nearest; i++) {
				j = i < p_count ? i : i - p_count;
				k = i + 1 < p_count ? i + 1 : i + 1 - p_count;
				if (vertices[j].m_depth <= vertices[k].m_depth) {
					triangle[1] = vertices[j];
					triangle[2] = vertices[k];
				}
				else {
					triangle[1] = vertices[k];
					triangle[2] = vertices[j];
				}

				FUN_10067d00(triangle, 0);
			}

			subdivided = TRUE;
		}
		else {
			for (i = 0; i < p_count; i++) {
				polygon[i] = vertices[i].m_vertex;
				polygon[i].m_u *= 255.0f;
				polygon[i].m_v *= 255.0f;
				polygon[i].m_x += g_a3dOffsetX;
				polygon[i].m_y += g_a3dOffsetY;
			}

			p_vertices = polygon;
		}
		break;
	}

	if (!subdivided) {
		grDrawPolygonVertexList(p_count, (GrVertex*) p_vertices);
	}

	if (g_a3dTranslucent) {
		grGlideSetState(&g_a3dState);
		g_a3dTranslucent = FALSE;
	}
}

// Puts the midpoint of the edge from p_a to p_b in p_out; returns nonzero, and nothing, when the
// edge is less than 4 pixels high.
// FUNCTION: MW23DFX 0x10067a90
MechS32 FUN_10067a90(A3DSubVertex* p_a, A3DSubVertex* p_b, A3DSubVertex* p_out)
{
	MechS32 dy;

	dy = (MechS32) (p_a->m_vertex.m_y - p_b->m_vertex.m_y);
	if (dy < 0) {
		dy = -dy;
	}

	if (dy < 4) {
		return 1;
	}

	p_out->m_vertex.m_red = p_a->m_vertex.m_red;
	p_out->m_vertex.m_green = p_a->m_vertex.m_green;
	p_out->m_vertex.m_blue = p_a->m_vertex.m_blue;
	p_out->m_vertex.m_x = (p_b->m_vertex.m_x + p_a->m_vertex.m_x) * 0.5f;
	p_out->m_vertex.m_y = (p_b->m_vertex.m_y + p_a->m_vertex.m_y) * 0.5f;
	p_out->m_vertex.m_u = (p_b->m_vertex.m_u + p_a->m_vertex.m_u) * 0.5f;
	p_out->m_vertex.m_v = (p_b->m_vertex.m_v + p_a->m_vertex.m_v) * 0.5f;
	p_out->m_vertex.m_w = (p_b->m_vertex.m_w + p_a->m_vertex.m_w) * 0.5f;
	p_out->m_depth = 1.0f / p_out->m_vertex.m_w;
	p_out->m_s = p_out->m_depth * p_out->m_vertex.m_u;
	p_out->m_t = p_out->m_vertex.m_v * p_out->m_depth;
	return 0;
}

// Draws the triangle p_triangle unless it is too deep or its texture coordinates span more than
// 128 texels; returns 0 when it isn't drawn for its texture coordinates.
// The loops' pointer is in another register, and the x87 stores are scheduled differently.
// FUNCTION: MW23DFX 0x10067b60
MechS32 FUN_10067b60(A3DSubVertex* p_triangle)
{
	A3DVertex vertices[3];
	MechFloat minS;
	MechFloat minT;
	MechFloat w;
	MechS32 i;

	if (p_triangle[0].m_depth > g_a3dSubdivideLimit) {
		return 1;
	}

	for (i = 0; i < 3; i++) {
		vertices[i] = p_triangle[i].m_vertex;
	}

	minS = p_triangle[0].m_s;
	minT = p_triangle[0].m_t;
	for (i = 1; i < 3; i++) {
		if (p_triangle[i].m_s < minS) {
			minS = p_triangle[i].m_s;
		}

		if (p_triangle[i].m_t < minT) {
			minT = p_triangle[i].m_t;
		}
	}

	minS = (MechS32) minS;
	minT = (MechS32) minT;
	for (i = 0; i < 3; i++) {
		vertices[i].m_u = p_triangle[i].m_s - minS;
		if (vertices[i].m_u > 128.0f) {
			return 0;
		}

		vertices[i].m_v = p_triangle[i].m_t - minT;
		if (vertices[i].m_v > 128.0f) {
			return 0;
		}
	}

	for (i = 0; i < 3; i++) {
		vertices[i].m_x += g_a3dOffsetX;
		vertices[i].m_y += g_a3dOffsetY;
		vertices[i].m_tmuW = vertices[i].m_w;
		w = vertices[i].m_w * 255.0f;
		vertices[i].m_u *= w;
		vertices[i].m_v = w * vertices[i].m_v;
	}

	grDrawTriangle((GrVertex*) &vertices[0], (GrVertex*) &vertices[1], (GrVertex*) &vertices[2]);
	return 1;
}

// Draws the triangle p_triangle, split in two through the midpoint of its long edge until it can
// be drawn (FUN_10067b60), at most 8 levels deep.
// The comparison is made before the first copy in the original.
// FUNCTION: MW23DFX 0x10067d00
MechS32 FUN_10067d00(A3DSubVertex* p_triangle, MechS32 p_level)
{
	A3DSubVertex middle;
	A3DSubVertex first[3];
	A3DSubVertex second[3];
	MechS32 drawnFirst;
	MechS32 drawnSecond;

	if (FUN_10067b60(p_triangle)) {
		return TRUE;
	}

	if (p_level == 8) {
		return FALSE;
	}

	if (FUN_10067a90(&p_triangle[0], &p_triangle[2], &middle)) {
		return FALSE;
	}

	first[0] = p_triangle[0];
	if (p_triangle[1].m_depth <= middle.m_depth) {
		first[1] = p_triangle[1];
		first[2] = middle;
		second[0] = p_triangle[1];
		second[1] = middle;
	}
	else {
		first[1] = middle;
		first[2] = p_triangle[1];
		second[0] = middle;
		second[1] = p_triangle[1];
	}

	second[2] = p_triangle[2];
	drawnFirst = FUN_10067d00(first, p_level + 1);
	drawnSecond = FUN_10067d00(second, p_level + 1);
	if (!drawnFirst || !drawnSecond) {
		return drawnFirst != 0;
	}

	return TRUE;
}

// Draws the polygon under the subdivided ground or sky, with a texture of 70 texels per pixel
// at a constant depth.
// FUNCTION: MW23DFX 0x10067e60
void FUN_10067e60(A3DVertex* p_vertices, MechS32 p_count)
{
	A3DVertex polygon[20];
	MechS32 i;

	for (i = 0; i < p_count; i++) {
		polygon[i] = p_vertices[i];
		polygon[i].m_tmuW = 1.0f;
		polygon[i].m_w = 1.0f;
		polygon[i].m_u = polygon[i].m_x * 70.0f;
		polygon[i].m_v = polygon[i].m_y * 70.0f;
		polygon[i].m_x += g_a3dOffsetX;
		polygon[i].m_y += g_a3dOffsetY;
	}

	grDrawPolygonVertexList(p_count, (GrVertex*) polygon);
}

// The x87 interpolations are scheduled differently.
// FUNCTION: MW23DFX 0x10067ef0
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
		else {
			p_out->m_w = p_a->m_w;
		}
		break;
	case 2:
		t = (p_edge - p_a->m_y) / (p_b->m_y - p_a->m_y);
		p_out->m_y = p_edge;
		p_out->m_x = (p_b->m_x - p_a->m_x) * t + p_a->m_x;
		if (p_flags & 8) {
			p_out->m_w = (p_b->m_w - p_a->m_w) * t + p_a->m_w;
		}
		else {
			p_out->m_w = p_a->m_w;
		}
		break;
	}

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

	if (p_flags & 4) {
		p_out->m_u = (p_b->m_u - p_a->m_u) * t + p_a->m_u;
		p_out->m_v = (p_b->m_v - p_a->m_v) * t + p_a->m_v;
	}
	else {
		p_out->m_u = p_a->m_u;
		p_out->m_v = p_a->m_v;
	}

	p_out->m_tmuW = p_out->m_w;
}
