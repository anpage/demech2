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

struct Player;

// The functions and globals of rendertarget.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_navCount;
	extern NavPoint g_navTable[128];

	MechS32 FUN_10060617(RenderTarget* p_target, MechS32 p_x, MechS32 p_y);
	MechS32 FUN_100606ed(
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
	void DrawShapeFrame(RenderTarget* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y);
	void FillRenderTargetRect(RenderTarget* p_target, MechS32 p_color);
	void FUN_10063755(
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
	MechS32 FUN_10064d4d(void* p_font);
	MechS32 FUN_10064d60(void* p_font, MechS32 p_char);
	MechS32 FUN_10064d80(
		RenderTarget* p_target,
		MechS32 p_x,
		MechS32 p_y,
		void* p_font,
		MechS32 p_char,
		void* p_unk0x14
	);
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
	MechS32 FUN_1005ec80(MechU32 p_owner, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	void FUN_1005ed4f(MechU32 p_owner, MechU32 p_nav);
	void FUN_1005ef5e(struct Player* p_player, MechS32 p_step, MechU32 p_flags);
	MechS32 FUN_1005fa22(struct Player* p_player);
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

#ifdef __cplusplus
}
#endif

#endif // RENDERTARGET_H
