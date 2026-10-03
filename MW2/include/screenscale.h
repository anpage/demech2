#ifndef SCREENSCALE_H
#define SCREENSCALE_H

#include "gaugequadrant.h"
#include "point.h"
#include "rect.h"
#include "rendertarget.h"
#include "types.h"
#include "window.h"

// The functions and globals of screenscale.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	PANE* ScaleRectToScreen(WINDOW* p_buffer, PANE* p_src, PANE* p_dst);
	PANE* FUN_1005699f(PANE* p_frame, PANE* p_src, PANE* p_dst);
	Point* ScalePointToScreen(WINDOW* p_buffer, Point* p_src, Point* p_dst);
	Point* FUN_10056bc1(PANE* p_frame, Point* p_src, Point* p_dst);
	PANE* FUN_10056c25(PANE* p_rect, MechS32 p_aspect);
	PANE* FUN_10056ce9(PANE* p_src, PANE* p_dst);
	Point* FUN_10056ddd(Point* p_src, Point* p_dst);
	PANE* CenterRectOnScreen(WINDOW* p_buffer, PANE* p_src, PANE* p_dst);
	Rect* FUN_10056a67(WINDOW* p_buffer, Rect* p_src, Rect* p_dst);
	Rect* FUN_10056b2b(PANE* p_frame, Rect* p_src, Rect* p_dst);
	Rect* FUN_10056d64(Rect* p_src, Rect* p_dst);
	PANE* FUN_10056fcf(PANE* p_src, PANE* p_dst, void* p_shape, MechS32 p_frame);
	PANE* FUN_1005705e(PANE* p_src, PANE* p_dst, void* p_shape);
	void FUN_100570e9(PANE* p_target, MechS32 p_color);
	void FUN_1005718d(PANE* p_target, Point p_pos, void* p_font, MechS32 p_color);
	void FUN_100571ea(PANE* p_target, MechChar* p_text, Point p_pos, void* p_font, MechS32 p_color);
	void FUN_10057282(PANE* p_target, MechChar* p_text, Point p_pos, void* p_font, MechS32 p_color);
	void FUN_10057396(PANE* p_target, MechChar* p_text, void* p_font);
	PANE* FUN_100575b9(MechChar* p_text, void* p_font, PANE* p_rect);
	void FUN_100577be(PANE* p_target);
	void FUN_10057896(PANE* p_target, void* p_shape, MechS32 p_frame);
	MechS32 FUN_1005798d(MechS32 p_dx, MechS32 p_dy, MechS32* p_slope);
	Point* FUN_10057a03(PANE* p_target, Point* p_point, Point* p_out);
	Point* FUN_10057ac4(PANE* p_target, MechS32 p_angle, Point* p_out);
	Point* FUN_10057bf3(Point* p_half, GaugeQuadrant p_quadrant, MechS32 p_slope, Point* p_out);
	void FUN_10057e56(PANE* p_target, MechS32 p_color);
	void FUN_10057edc(PANE* p_target, Rect* p_rect, MechS32 p_color);
	MechS32 FUN_10057fbe(PANE* p_target, MechS32 p_x, MechS32 p_y);
	Point* FUN_1005806a(PANE* p_target, Point* p_point, Point* p_out);
	Point* FUN_1005816f(PANE* p_target, MechS32 p_angle, Point* p_out);
	Point* FUN_100582c4(PANE* p_target, Point* p_center, GaugeQuadrant p_quadrant, MechS32 p_angle, Point* p_out);
	PANE* FUN_10056ec5(PANE* p_src, PANE* p_dst, Point p_scale);

#ifdef __cplusplus
}
#endif

#endif // SCREENSCALE_H
