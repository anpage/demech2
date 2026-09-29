#ifndef SCREENSCALE_H
#define SCREENSCALE_H

#include "pixelbuffer.h"
#include "point.h"
#include "rendertarget.h"
#include "types.h"

// The functions and globals of screenscale.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	RenderTarget* ScaleRectToScreen(PixelBuffer* p_buffer, RenderTarget* p_src, RenderTarget* p_dst);
	RenderTarget* FUN_1005699f(RenderTarget* p_frame, RenderTarget* p_src, RenderTarget* p_dst);
	Point* ScalePointToScreen(PixelBuffer* p_buffer, Point* p_src, Point* p_dst);
	Point* FUN_10056bc1(RenderTarget* p_frame, Point* p_src, Point* p_dst);
	RenderTarget* FUN_10056c25(RenderTarget* p_rect, MechS32 p_aspect);
	RenderTarget* FUN_10056ce9(RenderTarget* p_src, RenderTarget* p_dst);
	Point* FUN_10056ddd(Point* p_src, Point* p_dst);
	RenderTarget* CenterRectOnScreen(PixelBuffer* p_buffer, RenderTarget* p_src, RenderTarget* p_dst);
	void FUN_100570e9(RenderTarget* p_target, MechS32 p_unk0x04);
	void FUN_10057a03(void);
	void FUN_10057ac4(void);
	void FUN_10057e56(void);
	void FUN_10057fbe(void);
	void FUN_1005806a(void);
	void FUN_1005816f(void);
	RenderTarget* FUN_10056ec5(RenderTarget* p_src, RenderTarget* p_dst, MechS32 p_scaleX, MechS32 p_scaleY);

#ifdef __cplusplus
}
#endif

#endif // SCREENSCALE_H
