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

	Pane* ScaleRectToScreen(PixelBuffer* p_buffer, Pane* p_src, Pane* p_dst);
	Pane* FUN_1005699f(Pane* p_frame, Pane* p_src, Pane* p_dst);
	Point* ScalePointToScreen(PixelBuffer* p_buffer, Point* p_src, Point* p_dst);
	Point* FUN_10056bc1(Pane* p_frame, Point* p_src, Point* p_dst);
	Pane* FUN_10056c25(Pane* p_rect, MechS32 p_aspect);
	Pane* FUN_10056ce9(Pane* p_src, Pane* p_dst);
	Point* FUN_10056ddd(Point* p_src, Point* p_dst);
	Pane* CenterRectOnScreen(PixelBuffer* p_buffer, Pane* p_src, Pane* p_dst);
	Rect* FUN_10056a67(PixelBuffer* p_buffer, Rect* p_src, Rect* p_dst);
	Rect* FUN_10056b2b(Pane* p_frame, Rect* p_src, Rect* p_dst);
	Rect* FUN_10056d64(Rect* p_src, Rect* p_dst);
	Pane* FUN_10056fcf(Pane* p_src, Pane* p_dst, void* p_shape, MechS32 p_frame);
	Pane* FUN_1005705e(Pane* p_src, Pane* p_dst, void* p_shape);
	void FUN_100570e9(Pane* p_target, MechS32 p_color);
	void FUN_1005718d(Pane* p_target, Point p_pos, void* p_font, MechS32 p_color);
	void FUN_100571ea(Pane* p_target, MechChar* p_text, Point p_pos, void* p_font, MechS32 p_color);
	void FUN_10057282(Pane* p_target, MechChar* p_text, Point p_pos, void* p_font, MechS32 p_color);
	void FUN_10057396(Pane* p_target, MechChar* p_text, void* p_font);
	Pane* FUN_100575b9(MechChar* p_text, void* p_font, Pane* p_rect);
	void FUN_100577be(Pane* p_target);
	void FUN_10057896(Pane* p_target, void* p_shape, MechS32 p_frame);
	MechS32 FUN_1005798d(MechS32 p_dx, MechS32 p_dy, MechS32* p_slope);
	Point* FUN_10057a03(Pane* p_target, Point* p_point, Point* p_out);
	Point* FUN_10057ac4(Pane* p_target, MechS32 p_angle, Point* p_out);
	Point* FUN_10057bf3(Point* p_half, GaugeQuadrant p_quadrant, MechS32 p_slope, Point* p_out);
	void FUN_10057e56(Pane* p_target, MechS32 p_color);
	void FUN_10057edc(Pane* p_target, Rect* p_rect, MechS32 p_color);
	MechS32 FUN_10057fbe(Pane* p_target, MechS32 p_x, MechS32 p_y);
	Point* FUN_1005806a(Pane* p_target, Point* p_point, Point* p_out);
	Point* FUN_1005816f(Pane* p_target, MechS32 p_angle, Point* p_out);
	Point* FUN_100582c4(Pane* p_target, Point* p_center, GaugeQuadrant p_quadrant, MechS32 p_angle, Point* p_out);
	Pane* FUN_10056ec5(Pane* p_src, Pane* p_dst, Point p_scale);

#ifdef __cplusplus
}
#endif

#endif // SCREENSCALE_H
