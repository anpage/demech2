#include "screenscale.h"

#include "clock.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "gaugequadrant.h"
#include "pixelbuffer.h"
#include "point.h"
#include "rect.h"
#include "render.h"
#include "rendertarget.h"
#include "simmain.h"
#include "types.h"
#include "unk100696c0.h"

#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(GaugeQuadrant, 0x04)
DECOMP_SIZE_ASSERT(Rect, 0x10)

// Rectangles and points in 16.16 fractions of the screen (or of a frame rectangle), and in the
// 320x200 coordinates the HUD tables were authored in.

// The margins FUN_100575b9 leaves around a text block, in 16.16 fractions.
// GLOBAL: MW2 0x100a9450
Point g_unk0x100a9450 = {0x28f, 0x28f};

// GLOBAL: MW2 0x100a9458
MechS32 g_unk0x100a9458 = 0;

// GLOBAL: MW2 0x100a945c
MechS32 g_unk0x100a945c = 0xf0;

// GLOBAL: MW2 0x100a9460
MechS32 g_unk0x100a9460 = 0;

// GLOBAL: MW2 0x100a9464
MechS32 g_unk0x100a9464 = 0;

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

// FUNCTION: MW2 0x10056a67
Rect* FUN_10056a67(PixelBuffer* p_buffer, Rect* p_src, Rect* p_dst)
{
	p_dst->m_left = FixedMul16(g_screenWidthMinus1, p_src->m_left);
	p_dst->m_top = FixedMul16(g_screenHeightMinus1, p_src->m_top);
	p_dst->m_right = FixedMul16(g_screenWidthMinus1, p_src->m_right);
	p_dst->m_bottom = FixedMul16(g_screenHeightMinus1, p_src->m_bottom);
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
// FUNCTION: MW2 0x10056b2b
Rect* FUN_10056b2b(RenderTarget* p_frame, Rect* p_src, Rect* p_dst)
{
	MechS32 width;
	MechS32 height;

	width = p_frame->m_right - p_frame->m_left;
	height = p_frame->m_bottom - p_frame->m_top;
	p_dst->m_left = FixedMul16(width, p_src->m_left);
	p_dst->m_top = FixedMul16(height, p_src->m_top);
	p_dst->m_right = FixedMul16(width, p_src->m_right);
	p_dst->m_bottom = FixedMul16(height, p_src->m_bottom);
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

// FUNCTION: MW2 0x10056d64
Rect* FUN_10056d64(Rect* p_src, Rect* p_dst)
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

// Scales p_src about its center by p_scale (16.16).
// Stack-slot permutation: centerX and centerY.
// FUNCTION: MW2 0x10056ec5
RenderTarget* FUN_10056ec5(RenderTarget* p_src, RenderTarget* p_dst, Point p_scale)
{
	MechS32 centerX;
	MechS32 centerY;

	centerX = ((p_src->m_right - p_src->m_left + 1) >> 1) + p_src->m_left;
	centerY = ((p_src->m_bottom - p_src->m_top + 1) >> 1) + p_src->m_top;
	p_dst->m_left = p_src->m_left - centerX;
	p_dst->m_top = p_src->m_top - centerY;
	p_dst->m_right = p_src->m_right - centerX;
	p_dst->m_bottom = p_src->m_bottom - centerY;
	p_dst->m_left = FixedMul16(p_dst->m_left, p_scale.m_x);
	p_dst->m_top = FixedMul16(p_dst->m_top, p_scale.m_y);
	p_dst->m_right = FixedMul16(p_dst->m_right, p_scale.m_x);
	p_dst->m_bottom = FixedMul16(p_dst->m_bottom, p_scale.m_y);
	p_dst->m_left += centerX;
	p_dst->m_top += centerY;
	p_dst->m_right += centerX;
	p_dst->m_bottom += centerY;
	return p_dst;
}

// Scales p_src about its center to the size of a shape frame.
// Stack-slot permutation: size and scale.
// FUNCTION: MW2 0x10056fcf
RenderTarget* FUN_10056fcf(RenderTarget* p_src, RenderTarget* p_dst, void* p_shape, MechS32 p_frame)
{
	MechS32 size;
	Point scale;

	size = FUN_100657a8(p_shape, p_frame);
	scale.m_x = size >> 16;
	scale.m_y = size & 0xffff;
	scale.m_x = FixedDiv16(scale.m_x, p_src->m_right - p_src->m_left + 1);
	scale.m_y = FixedDiv16(scale.m_y, p_src->m_bottom - p_src->m_top + 1);
	FUN_10056ec5(p_src, p_dst, scale);
	return p_dst;
}

// Scales p_src about its center to the size of a shape.
// Stack-slot permutation: size and scale.
// FUNCTION: MW2 0x1005705e
RenderTarget* FUN_1005705e(RenderTarget* p_src, RenderTarget* p_dst, void* p_shape)
{
	MechS32 size;
	Point scale;

	size = FUN_10065770(p_shape);
	scale.m_x = size >> 16;
	scale.m_y = size & 0xffff;
	scale.m_x = FixedDiv16(scale.m_x, p_src->m_right - p_src->m_left + 1);
	scale.m_y = FixedDiv16(scale.m_y, p_src->m_bottom - p_src->m_top + 1);
	FUN_10056ec5(p_src, p_dst, scale);
	return p_dst;
}

// Outlines a render target.
// Stack-slot permutation: width and height.
// FUNCTION: MW2 0x100570e9
void FUN_100570e9(RenderTarget* p_target, MechS32 p_color)
{
	MechS32 width;
	MechS32 height;

	width = p_target->m_right - p_target->m_left;
	height = p_target->m_bottom - p_target->m_top;
	FUN_100606ed(p_target, 0, 0, width, 0, 0, p_color);
	FUN_100606ed(p_target, width, 0, width, height, 0, p_color);
	FUN_100606ed(p_target, width, height, 0, height, 0, p_color);
	FUN_100606ed(p_target, 0, height, 0, 0, 0, p_color);
}

// Draws a line across the render target under a line of text at p_y.
// Stack-slot permutation: height and width.
// FUNCTION: MW2 0x1005718d
void FUN_1005718d(RenderTarget* p_target, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_color)
{
	MechS32 height;
	MechS32 width;

	height = FUN_10064d4d(p_font);
	p_x = 0;
	p_y += height;
	width = p_target->m_right - p_target->m_left + 1;
	FUN_100606ed(p_target, p_x, p_y, width - 1, p_y, 0, p_color);
}

// Underlines text drawn at p_x, p_y.
// Stack-slot permutation: width, height and i.
// FUNCTION: MW2 0x100571ea
void FUN_100571ea(RenderTarget* p_target, MechChar* p_text, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_color)
{
	MechS32 width;
	MechS32 height;
	MechU32 i;

	width = 0;
	height = FUN_10064d4d(p_font);
	p_y += height;
	for (i = 0; i < strlen(p_text); i++) {
		width += FUN_10064d60(p_font, p_text[i]);
	}

	FUN_100606ed(p_target, p_x, p_y, p_x + width - 1, p_y, 0, p_color);
}

// Draws a box around text drawn at p_x, p_y.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x10057282
void FUN_10057282(RenderTarget* p_target, MechChar* p_text, MechS32 p_x, MechS32 p_y, void* p_font, MechS32 p_color)
{
	MechS32 left;
	MechS32 top;
	MechS32 bottom;
	MechS32 right;
	MechS32 height;
	MechS32 width;
	MechU32 i;

	width = 0;
	height = FUN_10064d4d(p_font);
	for (i = 0; i < strlen(p_text); i++) {
		width += FUN_10064d60(p_font, p_text[i]);
	}

	left = p_x;
	right = p_x + width;
	bottom = p_y + height + 1;
	top = p_y - 1;
	FUN_100606ed(p_target, left, top, right, top, 0, p_color);
	FUN_100606ed(p_target, left, bottom, right, bottom, 0, p_color);
	FUN_100606ed(p_target, left, top, left, bottom, 0, p_color);
	FUN_100606ed(p_target, right, top, right, bottom, 0, p_color);
}

// Draws text word-wrapped into a render target, inside the margins, until it runs out of lines.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x10057396
void FUN_10057396(RenderTarget* p_target, MechChar* p_text, void* p_font)
{
	MechS32 x;
	MechChar* line;
	Point origin;
	MechChar* end;
	MechS32 length;
	MechChar* breakAt;
	MechS32 lineHeight;
	MechChar* buffer;
	MechChar* p;
	MechS32 width;
	MechS32 height;

	if (p_target == NULL || p_text == NULL || p_font == NULL) {
		return;
	}

	length = strlen(p_text);
	if (length == 0) {
		return;
	}

	buffer = p = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, length + 1);
	end = buffer + length;
	strcpy(buffer, p_text);
	FUN_10056bc1(p_target, &g_unk0x100a9450, &origin);
	width = p_target->m_right - p_target->m_left;
	height = p_target->m_bottom - p_target->m_top;
	lineHeight = FUN_10064d4d(p_font);

	while (p <= end && origin.m_y < height) {
		line = p;
		x = origin.m_x;
		while (p <= end) {
			if (*p == '\n') {
				break;
			}

			x += FUN_10064d60(p_font, *p);
			if (x > width) {
				break;
			}

			p++;
		}

		if (p <= end) {
			if (*p == '\n') {
				*p = '\0';
			}
			else {
				breakAt = p;
				while (p >= line && *p != ' ' && *p != '\t') {
					p--;
				}

				if (p < line) {
					p = breakAt;
					*p = '\0';
				}
				else {
					*p = '\0';
				}
			}
		}

		if (*line != '\0') {
			FUN_10064f0b(p_target, origin.m_x, origin.m_y, p_font, line, g_unk0x100e9350);
		}

		origin.m_y += lineHeight;
		p++;
	}

	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, buffer);
}

// Sizes p_rect to fit a block of text, lines separated by newlines, plus the margins.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x100575b9
RenderTarget* FUN_100575b9(MechChar* p_text, void* p_font, RenderTarget* p_rect)
{
	MechS32 lineWidth;
	MechS32 lineHeight;
	MechChar* c;
	MechS32 maxWidth;
	MechS32 height;
	MechChar ch;

	maxWidth = 0;
	height = 0;
	lineWidth = 0;
	if (p_rect == NULL || p_text == NULL || p_font == NULL) {
		return NULL;
	}

	c = p_text;
	height = lineHeight = FUN_10064d4d(p_font);
	while ((ch = *c++) != '\0') {
		if (ch == '\n') {
			if (lineWidth > maxWidth) {
				maxWidth = lineWidth;
			}

			lineWidth = 0;
			height += lineHeight;
		}
		else {
			lineWidth += FUN_10064d60(p_font, ch);
		}
	}

	if (lineWidth > maxWidth) {
		maxWidth = lineWidth;
	}

	p_rect->m_left = p_rect->m_top = 0;
	p_rect->m_right = FixedDiv16(maxWidth, 0x10000 - g_unk0x100a9450.m_x);
	p_rect->m_bottom = FixedDiv16(height, 0x10000 - g_unk0x100a9450.m_y);
	return p_rect;
}

// Draws a pulsing frame just inside a render target.
// FUNCTION: MW2 0x100577be
void FUN_100577be(RenderTarget* p_target)
{
	Point corner;

	if (g_currentClock - g_unk0x100a9460 >= 0x5a) {
		g_unk0x100a9460 = g_currentClock;
		g_unk0x100a945c += g_unk0x100a9464;
		if (g_unk0x100a945c == 0xf0) {
			g_unk0x100a9464 = 1;
		}
		else if (g_unk0x100a945c == 0xf7) {
			g_unk0x100a9464 = g_unk0x100a9464 * -1;
		}
	}

	g_unk0x100a9458 %= 2;
	g_unk0x100a9458++;
	corner.m_x = p_target->m_right - g_unk0x100a9458;
	corner.m_y = p_target->m_bottom - g_unk0x100a9458;
	FUN_100610ef(p_target, g_unk0x100a9458, g_unk0x100a9458, corner.m_x, corner.m_y, g_unk0x100a945c);
}

// Tiles a render target with a shape frame.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x10057896
void FUN_10057896(RenderTarget* p_target, void* p_shape, MechS32 p_frame)
{
	MechS32 size;
	MechS32 tileWidth;
	MechS32 tileHeight;
	MechS32 columns;
	MechS32 rows;
	MechS32 row;
	MechS32 column;
	MechS32 x;
	MechS32 y;

	x = 0;
	y = 0;
	if (p_target == NULL || p_shape == NULL) {
		return;
	}

	size = FUN_100657a8(p_shape, p_frame);
	tileWidth = size >> 16;
	tileHeight = size & 0xffff;
	columns = (p_target->m_right - p_target->m_left + tileWidth) / tileWidth;
	rows = (p_target->m_bottom - p_target->m_top + tileHeight) / tileHeight;
	for (row = 0; row < rows; row++) {
		for (column = 0; column < columns; column++) {
			DrawShapeFrame(p_target, p_shape, p_frame, x, y);
			x += tileWidth;
		}

		x = 0;
		y += tileHeight;
	}
}

// Returns 1 if the line through (0, 0) and (p_dx, p_dy) is too steep for a 16.16 slope,
// otherwise 0 with the slope in p_slope.
// FUNCTION: MW2 0x1005798d
MechS32 FUN_1005798d(MechS32 p_dx, MechS32 p_dy, MechS32* p_slope)
{
	MechS32 steep;

	if (p_dx == 0 || p_dy / p_dx > 0x7fff || p_dy / p_dx < -0x8000) {
		steep = 1;
	}
	else {
		steep = 0;
	}

	if (!steep) {
		*p_slope = FixedDiv16(p_dy, p_dx);
	}

	return steep;
}

// Where the needle from the center of a gauge rectangle towards p_point leaves the rectangle.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x10057a03
Point* FUN_10057a03(RenderTarget* p_target, Point* p_point, Point* p_out)
{
	GaugeQuadrant quadrant;
	MechS32 slope;
	Point half;
	MechS32 dx;
	MechS32 dy;

	half.m_x = (p_target->m_right - p_target->m_left + 1) >> 1;
	half.m_y = (p_target->m_bottom - p_target->m_top + 1) >> 1;
	quadrant.m_value = 0;

	dx = half.m_x - p_point->m_x;
	if (dx >= 0) {
		quadrant.m_bits.m_left = 1;
	}

	dy = half.m_y - p_point->m_y;
	if (dy >= 0) {
		quadrant.m_bits.m_up = 1;
	}

	quadrant.m_bits.m_steep = FUN_1005798d(dx, dy, &slope);
	FUN_10057bf3(&half, quadrant, slope, p_out);
	return p_out;
}

// Where the needle from the center of a gauge rectangle at the heading p_angle (16.16 degrees)
// leaves the rectangle.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x10057ac4
Point* FUN_10057ac4(RenderTarget* p_target, MechS32 p_angle, Point* p_out)
{
	GaugeQuadrant quadrant;
	MechS32 slope;
	Point half;
	MechS32 dx;
	MechS32 dy;

	p_angle = (p_angle % 0x1680000 + 0x1680000) % 0x1680000;
	half.m_x = (p_target->m_right - p_target->m_left + 1) >> 1;
	half.m_y = (p_target->m_bottom - p_target->m_top + 1) >> 1;
	quadrant.m_value = 0;

	if (p_angle >= 0x5a0000 && p_angle <= 0x10e0000) {
		quadrant.m_bits.m_left = 1;
	}
	else {
		quadrant.m_bits.m_left = 0;
	}

	if (p_angle <= 0xb40000) {
		quadrant.m_bits.m_up = 1;
	}
	else {
		quadrant.m_bits.m_up = 0;
	}

	dx = FUN_1006973a(p_angle) >> 13;
	dy = FUN_100696c0(p_angle) >> 13;
	dy = FixedMul16(dy, g_eyepoint->m_pixelAspect);
	quadrant.m_bits.m_steep = FUN_1005798d(dx, dy, &slope);
	slope = -slope;
	FUN_10057bf3(&half, quadrant, slope, p_out);
	return p_out;
}

// Stack-slot permutation: width, height, ratio and result; the comparisons with p_slope load
// the other operand first.
// Where a line from the center of a rectangle (p_half: its half size) leaves it, for the
// slope p_slope and the direction bits of p_quadrant (FUN_10057a03).
// FUNCTION: MW2 0x10057bf3
Point* FUN_10057bf3(Point* p_half, GaugeQuadrant p_quadrant, MechS32 p_slope, Point* p_out)
{
	MechS32 height;
	MechS32 width;
	MechS32 ratio;
	MechS32 negRatio;
	Point result;

	width = p_half->m_x * 2;
	height = p_half->m_y * 2;
	if (p_half->m_x == 0) {
		ratio = 0;
	}
	else {
		ratio = FixedDiv16(p_half->m_y, p_half->m_x);
	}
	negRatio = -ratio;

	switch (p_quadrant.m_value) {
	case 3:
		if (ratio < p_slope) {
			result.m_y = 0;
			result.m_x = p_half->m_x - FixedDiv16(p_half->m_y, p_slope);
		}
		else {
			result.m_x = 0;
			result.m_y = p_half->m_y - FixedMul16(p_half->m_x, p_slope);
		}
		break;
	case 2:
		if (p_slope < negRatio) {
			result.m_y = 0;
			result.m_x = p_half->m_x - FixedDiv16(p_half->m_y, p_slope);
		}
		else {
			result.m_x = width;
			result.m_y = p_half->m_y + FixedMul16(p_half->m_x, p_slope);
		}
		break;
	case 0:
		if (ratio < p_slope) {
			result.m_y = height;
			result.m_x = p_half->m_x + FixedDiv16(p_half->m_y, p_slope);
		}
		else {
			result.m_x = width;
			result.m_y = p_half->m_y + FixedMul16(p_half->m_x, p_slope);
		}
		break;
	case 1:
		if (p_slope < negRatio) {
			result.m_y = height;
			result.m_x = p_half->m_x + FixedDiv16(p_half->m_y, p_slope);
		}
		else {
			result.m_x = 0;
			result.m_y = p_half->m_y - FixedMul16(p_half->m_x, p_slope);
		}
		break;
	case 7:
		result.m_y = 0;
		result.m_x = p_half->m_x;
		break;
	case 5:
		result.m_y = height;
		result.m_x = p_half->m_x;
		break;
	default:
		result.m_y = 0;
		result.m_x = 0;
		break;
	}

	*p_out = result;
	return p_out;
}

// Draws the ellipse inscribed in a render target, corrected for the pixel aspect.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x10057e56
void FUN_10057e56(RenderTarget* p_target, MechS32 p_color)
{
	MechS32 centerX;
	MechS32 centerY;
	MechS32 radiusY;
	MechS32 radius;
	MechS32 aspect;

	aspect = g_eyepoint->m_pixelAspect;
	centerX = (p_target->m_right - p_target->m_left + 1) >> 1;
	centerY = (p_target->m_bottom - p_target->m_top + 1) >> 1;
	radius = (p_target->m_right - p_target->m_left + 1) / 2 - 1;
	radiusY = FixedMul16(radius, aspect);
	FUN_10063755(p_target, centerX, centerY, radius, radiusY, p_color);
}

// Fills the part of the ellipse inscribed in a render target that lies in p_rect.
// Stack-slot permutation: the locals; clip.m_top loads p_rect->m_top first in the original.
// FUNCTION: MW2 0x10057edc
void FUN_10057edc(RenderTarget* p_target, Rect* p_rect, MechS32 p_color)
{
	RenderTarget clip;
	MechS32 centerX;
	MechS32 centerY;
	MechS32 radiusY;
	MechS32 radius;
	MechS32 aspect;

	aspect = g_eyepoint->m_pixelAspect;
	centerX = (p_target->m_right - p_target->m_left + 1) >> 1;
	centerY = (p_target->m_bottom - p_target->m_top + 1) >> 1;
	radius = (p_target->m_right - p_target->m_left + 1) / 2 - 1;
	radiusY = FixedMul16(radius, aspect);
	clip.m_buffer = p_target->m_buffer;
	clip.m_left = p_target->m_left + p_rect->m_left;
	clip.m_top = p_target->m_top + p_rect->m_top;
	clip.m_right = p_target->m_left + p_rect->m_right;
	clip.m_bottom = p_target->m_top + p_rect->m_bottom;
	centerX -= p_rect->m_left;
	centerY -= p_rect->m_top;
	FUN_10063a96(&clip, centerX, centerY, radius, radiusY, p_color);
}

// Tests whether a point lies inside the ellipse inscribed in a render target.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x10057fbe
MechS32 FUN_10057fbe(RenderTarget* p_target, MechS32 p_x, MechS32 p_y)
{
	MechS32 centerX;
	MechS32 centerY;
	MechS32 radiusSquared;
	MechS32 dx;
	MechS32 dy;
	MechS32 distanceSquared;
	MechS32 aspect;

	aspect = g_eyepoint->m_pixelAspect;
	centerX = (p_target->m_right - p_target->m_left + 1) >> 1;
	centerY = (p_target->m_bottom - p_target->m_top + 1) >> 1;
	radiusSquared = (p_target->m_right - p_target->m_left + 1) / 2 - 1;
	radiusSquared = radiusSquared * radiusSquared;
	dx = p_x - centerX;
	dy = FixedDiv16(p_y - centerY, aspect);
	distanceSquared = dy * dy + dx * dx;
	return distanceSquared <= radiusSquared;
}

// Where the needle of the gauge ellipse towards p_point ends.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x1005806a
Point* FUN_1005806a(RenderTarget* p_target, Point* p_point, Point* p_out)
{
	MechS32 angle;
	GaugeQuadrant quadrant;
	Point half;
	MechS32 dx;
	MechS32 dy;

	half.m_x = (p_target->m_right - p_target->m_left + 1) >> 1;
	half.m_y = (p_target->m_bottom - p_target->m_top + 1) >> 1;
	quadrant.m_value = 0;

	dx = half.m_x - p_point->m_x;
	if (dx >= 0) {
		quadrant.m_bits.m_left = 1;
	}

	dy = half.m_y - p_point->m_y;
	if (dy >= 0) {
		quadrant.m_bits.m_up = 1;
	}

	if (dx == 0 || dy / dx > 0x7fff || dy / dx < -0x8000) {
		quadrant.m_bits.m_steep = 1;
	}
	else {
		quadrant.m_bits.m_steep = 0;
	}

	dy = FixedDiv16(dy, g_eyepoint->m_pixelAspect);
	angle = FUN_100698de(dy, dx);
	FUN_100582c4(p_target, &half, quadrant, angle, p_out);
	return p_out;
}

// Where the needle of the gauge ellipse at the heading p_angle (16.16 degrees) ends.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x1005816f
Point* FUN_1005816f(RenderTarget* p_target, MechS32 p_angle, Point* p_out)
{
	MechS32 angle;
	GaugeQuadrant quadrant;
	Point half;
	MechS32 dx;
	MechS32 dy;

	p_angle = (p_angle % 0x1680000 + 0x1680000) % 0x1680000;
	half.m_x = (p_target->m_right - p_target->m_left + 1) >> 1;
	half.m_y = (p_target->m_bottom - p_target->m_top + 1) >> 1;
	quadrant.m_value = 0;

	if (p_angle >= 0x5a0000 && p_angle <= 0x10e0000) {
		quadrant.m_bits.m_left = 1;
	}
	else {
		quadrant.m_bits.m_left = 0;
	}

	if (p_angle <= 0xb40000) {
		quadrant.m_bits.m_up = 1;
	}
	else {
		quadrant.m_bits.m_up = 0;
	}

	dx = FUN_1006973a(p_angle) >> 13;
	dy = FUN_100696c0(p_angle) >> 13;
	dy = FixedMul16(dy, g_eyepoint->m_pixelAspect);
	if (dx == 0 || dy / dx > 0x7fff || dy / dx < -0x8000) {
		quadrant.m_bits.m_steep = 1;
	}
	else {
		quadrant.m_bits.m_steep = 0;
	}

	angle = FUN_100698de(dy, -dx);
	FUN_100582c4(p_target, &half, quadrant, angle, p_out);
	return p_out;
}

// Where a needle of the gauge ellipse at p_angle ends, from p_center.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x100582c4
Point* FUN_100582c4(RenderTarget* p_target, Point* p_center, GaugeQuadrant p_quadrant, MechS32 p_angle, Point* p_out)
{
	MechS32 radius;
	MechS32 radiusY;
	Point result;
	Point center;

	center = *p_center;
	radius = (p_target->m_right - p_target->m_left + 1) / 2 - 1;
	radiusY = FixedMul16(radius, g_eyepoint->m_pixelAspect);
	radius--;
	radiusY--;

	switch (p_quadrant.m_value) {
	case 0:
	case 1:
	case 2:
	case 3:
		result.m_x = center.m_x - (FixedMul16(radius, FUN_1006973a(p_angle)) >> 13);
		result.m_y = center.m_y - (FixedMul16(radiusY, FUN_100696c0(p_angle)) >> 13);
		break;
	case 7:
		result.m_y = center.m_y - radiusY;
		result.m_x = center.m_x;
		break;
	case 5:
		result.m_y = center.m_y + radiusY;
		result.m_x = center.m_x;
		break;
	default:
		result.m_y = 0;
		result.m_x = 0;
		break;
	}

	*p_out = result;
	return p_out;
}
