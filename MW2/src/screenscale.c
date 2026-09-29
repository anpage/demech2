#include "screenscale.h"

#include "decomp.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "pixelbuffer.h"
#include "point.h"
#include "render.h"
#include "rendertarget.h"
#include "types.h"

// Rectangles and points in 16.16 fractions of the screen (or of a frame rectangle), and in the
// 320x200 coordinates the HUD tables were authored in.

// Maps 16.16 fractions of the screen onto pixels.
// FUNCTION: MW2 0x10056920
RenderTarget* ScaleRectToScreen(PixelBuffer* p_buffer, RenderTarget* p_src, RenderTarget* p_dst)
{
	p_dst->m_left = FixedMul16(g_screenWidthMinus1, p_src->m_left);
	p_dst->m_top = FixedMul16(g_screenHeightMinus1, p_src->m_top);
	p_dst->m_right = FixedMul16(g_screenWidthMinus1, p_src->m_right);
	p_dst->m_bottom = FixedMul16(g_screenHeightMinus1, p_src->m_bottom);
	return p_dst;
}

// Maps 16.16 fractions of p_frame onto pixels.
// Stack-slot permutation: width and height.
// FUNCTION: MW2 0x1005699f
RenderTarget* FUN_1005699f(RenderTarget* p_frame, RenderTarget* p_src, RenderTarget* p_dst)
{
	MechS32 width;
	MechS32 height;

	width = p_frame->m_right - p_frame->m_left;
	height = p_frame->m_bottom - p_frame->m_top;
	p_dst->m_left = FixedMul16(p_src->m_left, width);
	p_dst->m_top = FixedMul16(p_src->m_top, height);
	p_dst->m_right = FixedMul16(p_src->m_right, width);
	p_dst->m_bottom = FixedMul16(p_src->m_bottom, height);
	p_dst->m_left += p_frame->m_left;
	p_dst->m_top += p_frame->m_top;
	p_dst->m_right += p_frame->m_left;
	p_dst->m_bottom += p_frame->m_top;
	return p_dst;
}

// FUNCTION: MW2 0x10056ae4
Point* ScalePointToScreen(PixelBuffer* p_buffer, Point* p_src, Point* p_dst)
{
	p_dst->m_x = FixedMul16(g_screenWidthMinus1, p_src->m_x);
	p_dst->m_y = FixedMul16(g_screenHeightMinus1, p_src->m_y);
	return p_dst;
}

// Stack-slot permutation: width and height.
// FUNCTION: MW2 0x10056bc1
Point* FUN_10056bc1(RenderTarget* p_frame, Point* p_src, Point* p_dst)
{
	MechS32 width;
	MechS32 height;

	width = p_frame->m_right - p_frame->m_left;
	height = p_frame->m_bottom - p_frame->m_top;
	p_dst->m_x = FixedMul16(width, p_src->m_x);
	p_dst->m_y = FixedMul16(height, p_src->m_y);
	return p_dst;
}

// Scales 16.16 fractions to 320x200 coordinates, correcting the height for the aspect ratio
// p_aspect (16.16, 0xd555 = 5:6 for none).
// FUNCTION: MW2 0x10056c25
RenderTarget* FUN_10056c25(RenderTarget* p_rect, MechS32 p_aspect)
{
	MechS32 scale;

	scale = FixedDiv16(p_aspect, 0xd555);
	p_rect->m_left = FixedMul16(p_rect->m_left, 319);
	p_rect->m_top = FixedMul16(p_rect->m_top, 199);
	p_rect->m_top = FixedMul16(p_rect->m_top, scale);
	p_rect->m_right = FixedMul16(p_rect->m_right, 319);
	p_rect->m_bottom = FixedMul16(p_rect->m_bottom, 199);
	p_rect->m_bottom = FixedMul16(p_rect->m_bottom, scale);
	return p_rect;
}

// Maps 320x200 coordinates to 16.16 fractions of the screen.
// FUNCTION: MW2 0x10056ce9
RenderTarget* FUN_10056ce9(RenderTarget* p_src, RenderTarget* p_dst)
{
	p_dst->m_left = FixedDiv16(p_src->m_left, 319);
	p_dst->m_top = FixedDiv16(p_src->m_top, 199);
	p_dst->m_right = FixedDiv16(p_src->m_right, 319);
	p_dst->m_bottom = FixedDiv16(p_src->m_bottom, 199);
	return p_dst;
}

// FUNCTION: MW2 0x10056ddd
Point* FUN_10056ddd(Point* p_src, Point* p_dst)
{
	p_dst->m_x = FixedDiv16(p_src->m_x, 319);
	p_dst->m_y = FixedDiv16(p_src->m_y, 199);
	return p_dst;
}

// Centers a rectangle of p_src's size on the screen.
// FUNCTION: MW2 0x10056e22
RenderTarget* CenterRectOnScreen(PixelBuffer* p_buffer, RenderTarget* p_src, RenderTarget* p_dst)
{
	RenderTarget rect;
	MechS32 left;
	MechS32 top;

	rect = *p_src;
	rect.m_right -= rect.m_left;
	rect.m_left = 0;
	rect.m_bottom -= rect.m_top;
	rect.m_top = 0;
	left = (g_screenWidthMinus1 - (rect.m_right - rect.m_left + 1) - 1) / 2;
	top = (g_screenHeightMinus1 - (rect.m_bottom - rect.m_top + 1) - 1) / 2;
	p_dst->m_left = rect.m_left + left;
	p_dst->m_right = rect.m_right + left;
	p_dst->m_top = top + rect.m_top;
	p_dst->m_bottom = top + rect.m_bottom;
	return p_dst;
}

// Scales p_src about its center by p_scaleX and p_scaleY (16.16).
// Stack-slot permutation: centerX and centerY.
// FUNCTION: MW2 0x10056ec5
RenderTarget* FUN_10056ec5(RenderTarget* p_src, RenderTarget* p_dst, MechS32 p_scaleX, MechS32 p_scaleY)
{
	MechS32 centerX;
	MechS32 centerY;

	centerX = ((p_src->m_right - p_src->m_left + 1) >> 1) + p_src->m_left;
	centerY = ((p_src->m_bottom - p_src->m_top + 1) >> 1) + p_src->m_top;
	p_dst->m_left = p_src->m_left - centerX;
	p_dst->m_top = p_src->m_top - centerY;
	p_dst->m_right = p_src->m_right - centerX;
	p_dst->m_bottom = p_src->m_bottom - centerY;
	p_dst->m_left = FixedMul16(p_dst->m_left, p_scaleX);
	p_dst->m_top = FixedMul16(p_dst->m_top, p_scaleY);
	p_dst->m_right = FixedMul16(p_dst->m_right, p_scaleX);
	p_dst->m_bottom = FixedMul16(p_dst->m_bottom, p_scaleY);
	p_dst->m_left += centerX;
	p_dst->m_top += centerY;
	p_dst->m_right += centerX;
	p_dst->m_bottom += centerY;
	return p_dst;
}

// STUB: MW2 0x100570e9
void FUN_100570e9(void)
{
	STUB(0x100570e9);
}

// STUB: MW2 0x10057a03
void FUN_10057a03(void)
{
	STUB(0x10057a03);
}

// STUB: MW2 0x10057ac4
void FUN_10057ac4(void)
{
	STUB(0x10057ac4);
}

// STUB: MW2 0x10057e56
void FUN_10057e56(void)
{
	STUB(0x10057e56);
}

// STUB: MW2 0x10057fbe
void FUN_10057fbe(void)
{
	STUB(0x10057fbe);
}

// STUB: MW2 0x1005806a
void FUN_1005806a(void)
{
	STUB(0x1005806a);
}

// STUB: MW2 0x1005816f
void FUN_1005816f(void)
{
	STUB(0x1005816f);
}
