#ifndef SCREENSCALE_H
#define SCREENSCALE_H

#include "gaugequadrant.h"
#include "pixelbuffer.h"
#include "point.h"
#include "rect.h"
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
	Rect* FUN_10056a67(PixelBuffer* p_buffer, Rect* p_src, Rect* p_dst);
	Rect* FUN_10056b2b(RenderTarget* p_frame, Rect* p_src, Rect* p_dst);
	Rect* FUN_10056d64(Rect* p_src, Rect* p_dst);
	RenderTarget* FUN_10056fcf(RenderTarget* p_src, RenderTarget* p_dst, void* p_shape, MechS32 p_frame);
	RenderTarget* FUN_1005705e(RenderTarget* p_src, RenderTarget* p_dst, void* p_shape);
	void FUN_100570e9(RenderTarget* p_target, MechS32 p_color);
	void FUN_1005718d(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_color);
	void FUN_100571ea(RenderTarget* p_target, MechChar* p_text, Point p_pos, void* p_font, MechS32 p_color);
	void FUN_10057282(RenderTarget* p_target, MechChar* p_text, Point p_pos, void* p_font, MechS32 p_color);
	void FUN_10057396(RenderTarget* p_target, MechChar* p_text, void* p_font);
	RenderTarget* FUN_100575b9(MechChar* p_text, void* p_font, RenderTarget* p_rect);
	void FUN_100577be(RenderTarget* p_target);
	void FUN_10057896(RenderTarget* p_target, void* p_shape, MechS32 p_frame);
	MechS32 FUN_1005798d(MechS32 p_dx, MechS32 p_dy, MechS32* p_slope);
	Point* FUN_10057a03(RenderTarget* p_target, Point* p_point, Point* p_out);
	Point* FUN_10057ac4(RenderTarget* p_target, MechS32 p_angle, Point* p_out);
	Point* FUN_10057bf3(Point* p_half, GaugeQuadrant p_quadrant, MechS32 p_slope, Point* p_out);
	void FUN_10057e56(RenderTarget* p_target, MechS32 p_color);
	void FUN_10057edc(RenderTarget* p_target, Rect* p_rect, MechS32 p_color);
	MechS32 FUN_10057fbe(RenderTarget* p_target, MechS32 p_x, MechS32 p_y);
	Point* FUN_1005806a(RenderTarget* p_target, Point* p_point, Point* p_out);
	Point* FUN_1005816f(RenderTarget* p_target, MechS32 p_angle, Point* p_out);
	Point* FUN_100582c4(
		RenderTarget* p_target,
		Point* p_center,
		GaugeQuadrant p_quadrant,
		MechS32 p_angle,
		Point* p_out
	);
	RenderTarget* FUN_10056ec5(RenderTarget* p_src, RenderTarget* p_dst, Point p_scale);

#ifdef __cplusplus
}
#endif

#endif // SCREENSCALE_H
