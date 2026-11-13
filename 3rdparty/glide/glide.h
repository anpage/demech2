#ifndef __GLIDE_H__
#define __GLIDE_H__

// GLIDE.DLL, 3Dfx Interactive's API for the Voodoo Graphics, in the Glide 1.x version the 3Dfx
// edition imports (glide.def): our own declarations of the functions it calls and of the types and
// constants they and Texus (3rdparty/texus) take, not the SDK's header. Names follow Glide's;
// values are the Voodoo Graphics' (Glide 2's header has the same ones). The layouts the game only
// passes around (GrHwConfiguration, GrState) are sized from its buffers.
#include "3dfx.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef FxU32 GrColor_t;
	typedef FxU8 GrAlpha_t;
	typedef FxU32 GrMipMapId_t;

	typedef FxI32 GrChipID_t;
#define GR_TMU0 0x0

#define GLIDE_NUM_TMU 2

#define GR_NULL_MIPMAP_HANDLE ((GrMipMapId_t) - 1)

#define GR_MIPMAPLEVELMASK_EVEN FXBIT(0)
#define GR_MIPMAPLEVELMASK_ODD FXBIT(1)
#define GR_MIPMAPLEVELMASK_BOTH (GR_MIPMAPLEVELMASK_EVEN | GR_MIPMAPLEVELMASK_ODD)

	typedef FxI32 GrCombineFunction_t;
#define GR_COMBINE_FUNCTION_SCALE_OTHER 0x3

	typedef FxI32 GrCombineFactor_t;
#define GR_COMBINE_FACTOR_TEXTURE_ALPHA 0x4

	typedef FxI32 GrCombineLocal_t;
#define GR_COMBINE_LOCAL_CONSTANT 0x1

	typedef FxI32 GrCombineOther_t;
#define GR_COMBINE_OTHER_TEXTURE 0x1

	typedef FxI32 GrAlphaSource_t;
#define GR_ALPHASOURCE_ITERATED_ALPHA 0x1

	typedef FxI32 GrColorCombineFnc_t;
#define GR_COLORCOMBINE_ITRGB 0x2
#define GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB 0x6

	typedef FxI32 GrAlphaBlendFnc_t;
#define GR_BLEND_ZERO 0x0
#define GR_BLEND_SRC_ALPHA 0x1
#define GR_BLEND_ONE 0x4
#define GR_BLEND_ONE_MINUS_SRC_ALPHA 0x5

	typedef FxI32 GrAspectRatio_t;
#define GR_ASPECT_8x1 0x0
#define GR_ASPECT_4x1 0x1
#define GR_ASPECT_2x1 0x2
#define GR_ASPECT_1x1 0x3
#define GR_ASPECT_1x2 0x4
#define GR_ASPECT_1x4 0x5
#define GR_ASPECT_1x8 0x6

	typedef FxI32 GrBuffer_t;
#define GR_BUFFER_FRONTBUFFER 0x0
#define GR_BUFFER_BACKBUFFER 0x1

	typedef FxI32 GrChromakeyMode_t;
#define GR_CHROMAKEY_DISABLE 0x0
#define GR_CHROMAKEY_ENABLE 0x1

	typedef FxI32 GrCmpFnc_t;
#define GR_CMP_ALWAYS 0x7

	typedef FxI32 GrColorFormat_t;
#define GR_COLORFORMAT_ARGB 0x0

	typedef FxI32 GrLfbBypassMode_t;
#define GR_LFBBYPASS_DISABLE 0x0
#define GR_LFBBYPASS_ENABLE 0x1

	typedef FxI32 GrLfbWriteMode_t;
#define GR_LFBWRITEMODE_565 0x0
#define GR_LFBWRITEMODE_555 0x1

	typedef FxI32 GrOriginLocation_t;
#define GR_ORIGIN_UPPER_LEFT 0x0

	typedef FxI32 GrLOD_t;
#define GR_LOD_256 0x0
#define GR_LOD_128 0x1
#define GR_LOD_64 0x2
#define GR_LOD_32 0x3
#define GR_LOD_16 0x4
#define GR_LOD_8 0x5
#define GR_LOD_4 0x6
#define GR_LOD_2 0x7
#define GR_LOD_1 0x8

	typedef FxI32 GrMipMapMode_t;
#define GR_MIPMAP_DISABLE 0x0
#define GR_MIPMAP_NEAREST 0x1
#define GR_MIPMAP_NEAREST_DITHER 0x2

	typedef FxI32 GrSmoothingMode_t;
#define GR_SMOOTHING_DISABLE 0x0
#define GR_SMOOTHING_ENABLE 0x1

	typedef FxI32 GrScreenResolution_t;
#define GR_RESOLUTION_640x480 0x7
#define GR_RESOLUTION_800x600 0x8

	typedef FxI32 GrScreenRefresh_t;
#define GR_REFRESH_60Hz 0x0

	typedef FxI32 GrTextureClampMode_t;
#define GR_TEXTURECLAMP_WRAP 0x0
#define GR_TEXTURECLAMP_CLAMP 0x1

	typedef FxI32 GrTextureCombineFnc_t;
#define GR_TEXTURECOMBINE_ZERO 0x0
#define GR_TEXTURECOMBINE_DECAL 0x1

	typedef FxI32 GrTextureFilterMode_t;
#define GR_TEXTUREFILTER_POINT_SAMPLED 0x0
#define GR_TEXTUREFILTER_BILINEAR 0x1

	typedef FxI32 GrTextureFormat_t;
#define GR_TEXFMT_8BIT 0x0
#define GR_TEXFMT_RGB_332 GR_TEXFMT_8BIT
#define GR_TEXFMT_YIQ_422 0x1
#define GR_TEXFMT_ALPHA_8 0x2
#define GR_TEXFMT_INTENSITY_8 0x3
#define GR_TEXFMT_ALPHA_INTENSITY_44 0x4
#define GR_TEXFMT_P_8 0x5
#define GR_TEXFMT_RSVD0 0x6
#define GR_TEXFMT_RSVD1 0x7
#define GR_TEXFMT_16BIT 0x8
#define GR_TEXFMT_ARGB_8332 GR_TEXFMT_16BIT
#define GR_TEXFMT_AYIQ_8422 0x9
#define GR_TEXFMT_RGB_565 0xa
#define GR_TEXFMT_ARGB_1555 0xb
#define GR_TEXFMT_ARGB_4444 0xc
#define GR_TEXFMT_ALPHA_INTENSITY_88 0xd
#define GR_TEXFMT_AP_88 0xe
#define GR_TEXFMT_RSVD2 0xf

	// The hardware grSstQueryHardware describes (0x124 bytes in the game's buffer).
	typedef struct {
		int num_sst;
		FxU8 m_unk0x04[0x124 - 0x04];
	} GrHwConfiguration;

	// The rendering state grGlideGetState saves (0x138 bytes in the game's).
	typedef struct {
		FxU32 m_unk0x00[0x138 / 4];
	} GrState;

	typedef struct {
		float sow;
		float tow;
		float oow;
	} GrTmuVertex;

	// SIZE 0x3c
	typedef struct {
		float x, y, z;
		float r, g, b;
		float ooz;
		float a;
		float oow;
		GrTmuVertex tmuvtx[GLIDE_NUM_TMU];
	} GrVertex;

	// The texture file structures (.3df), which Texus converts the game's textures into.
	typedef struct {
		FxU32 width, height;
		int small_lod, large_lod;
		GrAspectRatio_t aspect_ratio;
		GrTextureFormat_t format;
	} Gu3dfHeader;

	typedef struct {
		FxU8 yRGB[16];
		FxI16 iRGB[4][3];
		FxI16 qRGB[4][3];
		FxU32 packed_data[12];
	} GuNccTable;

	typedef struct {
		FxU32 data[256];
	} GuTexPalette;

	typedef union {
		GuNccTable nccTable;
		GuTexPalette palette;
	} GuTexTable;

	// SIZE 0x420
	typedef struct {
		Gu3dfHeader header;
		GuTexTable table;
		void* data;
		FxU32 mem_required;
	} Gu3dfInfo;

	void FX_CALL grAlphaBlendFunction(
		GrAlphaBlendFnc_t p_rgbSf,
		GrAlphaBlendFnc_t p_rgbDf,
		GrAlphaBlendFnc_t p_alphaSf,
		GrAlphaBlendFnc_t p_alphaDf
	);
	void FX_CALL grAlphaCombine(
		GrCombineFunction_t p_function,
		GrCombineFactor_t p_factor,
		GrCombineLocal_t p_local,
		GrCombineOther_t p_other,
		FxBool p_invert
	);
	void FX_CALL grAlphaTestFunction(GrCmpFnc_t p_function);
	void FX_CALL grBufferClear(GrColor_t p_color, GrAlpha_t p_alpha, FxU16 p_depth);
	int FX_CALL grBufferNumPending(void);
	void FX_CALL grBufferSwap(int p_swapInterval);
	void FX_CALL grChromakeyMode(GrChromakeyMode_t p_mode);
	void FX_CALL grDrawLine(const GrVertex* p_a, const GrVertex* p_b);
	void FX_CALL grDrawPolygonVertexList(int p_count, const GrVertex* p_vertices);
	void FX_CALL grDrawTriangle(const GrVertex* p_a, const GrVertex* p_b, const GrVertex* p_c);
	void FX_CALL grGlideGetState(GrState* p_state);
	void FX_CALL grGlideInit(void);
	void FX_CALL grGlideSetState(const GrState* p_state);
	void FX_CALL grGlideShutdown(void);
	void FX_CALL grLfbBegin(void);
	void FX_CALL grLfbBypassMode(GrLfbBypassMode_t p_mode);
	void FX_CALL grLfbEnd(void);
	void* FX_CALL grLfbGetWritePtr(GrBuffer_t p_buffer);
	void FX_CALL grLfbWriteMode(GrLfbWriteMode_t p_mode);
	FxBool FX_CALL grSstIsBusy(void);
	FxBool FX_CALL grSstOpen(
		GrScreenResolution_t p_resolution,
		GrScreenRefresh_t p_refresh,
		GrColorFormat_t p_format,
		GrOriginLocation_t p_origin,
		GrSmoothingMode_t p_smoothing,
		int p_buffers
	);
	FxBool FX_CALL grSstQueryHardware(GrHwConfiguration* p_config);
	void FX_CALL grSstSelect(int p_sst);
	FxU32 FX_CALL grTexCalcMemRequired(
		GrLOD_t p_smallLod,
		GrLOD_t p_largeLod,
		GrAspectRatio_t p_aspect,
		GrTextureFormat_t p_format
	);
	void FX_CALL grTexClampMode(GrChipID_t p_tmu, GrTextureClampMode_t p_sClamp, GrTextureClampMode_t p_tClamp);
	void FX_CALL grTexCombineFunction(GrChipID_t p_tmu, GrTextureCombineFnc_t p_function);
	void FX_CALL guAlphaSource(GrAlphaSource_t p_mode);
	void FX_CALL guColorCombineFunction(GrColorCombineFnc_t p_function);
	GrMipMapId_t FX_CALL guTexAllocateMemory(
		GrChipID_t p_tmu,
		FxU8 p_oddEvenMask,
		int p_width,
		int p_height,
		GrTextureFormat_t p_format,
		GrMipMapMode_t p_mipMapMode,
		GrLOD_t p_smallLod,
		GrLOD_t p_largeLod,
		GrAspectRatio_t p_aspect,
		GrTextureClampMode_t p_sClamp,
		GrTextureClampMode_t p_tClamp,
		GrTextureFilterMode_t p_minFilter,
		GrTextureFilterMode_t p_magFilter,
		float p_lodBias,
		FxBool p_trilinear
	);
	void FX_CALL guTexDownloadMipMap(GrMipMapId_t p_mipMap, const void* p_source, const GuNccTable* p_table);
	void FX_CALL guTexMemReset(void);
	void FX_CALL guTexSource(GrMipMapId_t p_mipMap);

#ifdef __cplusplus
}
#endif

#endif // __GLIDE_H__
