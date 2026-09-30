#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "decomp.h"
#include "navpoint.h"
#include "pixelbuffer.h"
#include "types.h"

// A rectangle of a pixel buffer. SelectRenderTarget copies one of the eleven in
// g_renderTargets into the current one (0x10176ed0) and sizes the eyepoint's view to it.
// SIZE 0x14
typedef struct RenderTarget {
	PixelBuffer* m_buffer; // 0x00
	MechS32 m_left;        // 0x04
	MechS32 m_top;         // 0x08
	MechS32 m_right;       // 0x0c
	MechS32 m_bottom;      // 0x10
} RenderTarget;

struct AmberWillow0x7c;
struct Player;
struct ScarletOrchid0x4c;

// The functions and globals of rendertarget.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_navCount;
	extern NavPoint g_navTable[128];

	MechS32 PutViewPixel(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, MechU32 p_color);
	MechS32 GetViewPixel(RenderTarget* p_target, MechS32 p_x, MechS32 p_y);
	MechS32 BlitLine(
		RenderTarget* p_target,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_x2,
		MechS32 p_y2,
		MechS32 p_mode,
		MechS32 p_color
	);
	void FUN_100610ef(
		RenderTarget* p_target,
		MechS32 p_left,
		MechS32 p_top,
		MechS32 p_right,
		MechS32 p_bottom,
		MechU8 p_color
	);
	void BlitShpFrame(RenderTarget* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y);
	void FillView(RenderTarget* p_target, MechS32 p_color);
	void DrawEllipse(
		RenderTarget* p_target,
		MechS32 p_centerX,
		MechS32 p_centerY,
		MechS32 p_radiusX,
		MechS32 p_radiusY,
		MechS32 p_color
	);
	void FUN_10063a96(
		RenderTarget* p_target,
		MechS32 p_centerX,
		MechS32 p_centerY,
		MechS32 p_radiusX,
		MechS32 p_radiusY,
		MechS32 p_color
	);
	void GetCosSin(MechS32 p_angle, MechS32* p_cos, MechS32* p_sin);
	void RotateScalePoint(
		MechS32* p_point,
		MechS32* p_result,
		MechS32* p_origin,
		MechS32 p_angle,
		MechS32 p_scaleX,
		MechS32 p_scaleY
	);
	MechS32 FUN_10064d4d(void* p_font);
	MechS32 FUN_10064d60(void* p_font, MechS32 p_char);
	MechS32 BlitChar(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_char, void* p_unk0x14);
	void FUN_10064f0b(
		RenderTarget* p_target,
		MechS32 p_x,
		MechS32 p_y,
		void* p_font,
		MechChar* p_text,
		void* p_unk0x14
	);
	MechS32 FUN_10065770(void* p_shape);
	MechS32 FUN_100657a8(void* p_shape, MechS32 p_frame);
	MechS32 FUN_100657ca(void* p_shape, MechS32 p_frame);
	MechS32 FUN_100657ed(void* p_shape, MechS32 p_frame);
	MechS32 FUN_10065821(void* p_shape, MechS32 p_frame);
	MechS32 GetShapeFrameCount(void* p_shape);
	MechS32 FUN_10065a7b(RenderTarget* p_dst, RenderTarget* p_src, MechS32 p_unk0x08, MechS32 p_unk0x0c);
	MechChar* GetDisplayDriverName(MechChar* (**p_driver)(void) );
	void SetDisplayDriver(MechU32* p_src);
	void BlitShpFrameUnclipped(RenderTarget* p_target, void* p_frame, MechS32 p_x, MechS32 p_y, undefined4 p_unk0x10);
	void SetRemapTable(MechU8* p_map);
	MechS32 BlitShpFrameRemappedUnclipped(
		RenderTarget* p_target,
		void* p_frame,
		MechS32 p_x,
		MechS32 p_y,
		undefined4 p_unk0x10
	);
	MechS32 BlitShpFrameRemapped(RenderTarget* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y);
	MechS32 FUN_100628c6(
		void* p_shape,
		MechS32 p_frame,
		MechS32 p_x,
		MechS32 p_y,
		undefined4 p_unk0x10,
		undefined4 p_unk0x14
	);
	MechS32 RemapShpFrame(void* p_shape, MechS32 p_frame);
	MechS32 BlitView(
		RenderTarget* p_target,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_width,
		MechS32 p_height,
		MechS32 p_unk0x14,
		MechS32 p_unk0x18
	);
	MechS32 ScrollView(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height);
	void FUN_10064c60(MechS32 p_a, MechS32 p_b, MechS32* p_result);
	MechS32 WriteViewRow(RenderTarget* p_target, MechS32 p_row, MechU8* p_src, MechS32 p_width);
	MechU8* FindIffChunk(MechChar* p_tag, MechU8* p_iff);
	MechS32 BlitIff(RenderTarget* p_target, MechU8* p_iff);
	void ReadIffPalette(MechU8* p_iff, MechU8* p_palette);
	MechS32 GetIffSize(MechU8* p_iff);
	MechS32 BlitPicture(RenderTarget* p_target, MechU8* p_pcx);
	void ReadPicturePalette(MechU8* p_pcx, MechS32 p_size, MechU8* p_palette);
	MechS32 GetPictureSize(MechU8* p_pcx);
	void GifInitCodes(void);
	void GifReadByte(void);
	void GifReadCode(void);
	void GifAddCode(void);
	void GifPutPixel(void);
	MechS32 BlitGif(RenderTarget* p_target, MechU8* p_gif, MechU8* p_state);
	void ReadGifPalette(MechU8* p_gif, MechU8* p_palette);
	void FUN_1006584b(void* p_shape, MechS32 p_frame, MechU8* p_palette);
	MechS32 FUN_10065896(void* p_shape, MechS32 p_frame, MechU32* p_out);
	MechS32 FUN_100658de(void* p_shape, MechS32 p_frame, MechU32* p_in);
	MechS32 CountShpUniqueFrames(void* p_shape, MechS32* p_out);
	MechS32 FUN_1006599d(void* p_shape, MechS32* p_out);
	MechS32 CountViewColors(RenderTarget* p_target, MechU32* p_out);
	MechS32 FUN_1005ec80(MechU32 p_owner, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void FUN_1005ed4f(MechU32 p_owner, MechU32 p_nav);
	void FUN_1005ef5e(struct Player* p_player, MechS32 p_step, MechU32 p_flags);
	void FUN_1005f284(void);
	MechS32 FUN_1005fa22(struct Player* p_player);
	MechS32 FUN_1005fe63(void);
	MechS32 FUN_1005febe(void);
	struct ScarletOrchid0x4c* FUN_1005ff19(void);
	struct AmberWillow0x7c* FUN_1005ff56(void);
	void FUN_10060197(
		MechS32 p_dx,
		MechS32 p_dy,
		MechS32 p_dz,
		MechS32* p_unk0x0c,
		MechS32* p_unk0x10,
		MechU32* p_distance,
		MechS32* p_unk0x18
	);
	void FUN_100602b2(struct Player* p_player, MechS32 p_step, MechS32 p_unk0x08);
	void FUN_100602ec(MechS32 p_step);
	void FUN_1006031b(MechS32 p_step);
	void FUN_1006034a(MechS32 p_step);
	void FUN_1006037c(MechS32 p_step);
	void FUN_100603ae(void);

#ifdef __cplusplus
}
#endif

#endif // RENDERTARGET_H
