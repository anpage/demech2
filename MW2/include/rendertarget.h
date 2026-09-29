#ifndef RENDERTARGET_H
#define RENDERTARGET_H

#include "decomp.h"
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

// The functions and globals of rendertarget.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void DrawShapeFrame(RenderTarget* p_target, void* p_shape, MechS32 p_frame, MechS32 p_x, MechS32 p_y);
	void FillRenderTargetRect(RenderTarget* p_target, MechS32 p_color);
	MechS32 FUN_10064d60(undefined4 p_font, MechS32 p_char);
	void FUN_10064f0b(
		RenderTarget* p_target,
		MechS32 p_x,
		MechS32 p_y,
		void* p_font,
		MechChar* p_text,
		void* p_unk0x14
	);
	MechS32 FUN_10065a7b(RenderTarget* p_dst, RenderTarget* p_src, MechS32 p_unk0x08, MechS32 p_unk0x0c);

#ifdef __cplusplus
}
#endif

#endif // RENDERTARGET_H
