#include "bargauges.h"

#include "clock.h"
#include "cockpitpanel.h"
#include "config.h"
#include "decomp.h"
#include "environment.h"
#include "muldiv.h"
#include "players.h"
#include "point.h"
#include "ramp.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "simmain.h"
#include "types.h"

// The cockpit's bar gauges: the heat, the rate the heat changes at, the throttle and the jump
// jet fuel. Each eases towards its value, and the rectangles are fractions of their panels
// until FUN_1004d020 scales them to pixels.

// GLOBAL: MW2 0x100a82f0
EasedValue g_unk0x100a82f0 = {0, 0, 1};

// GLOBAL: MW2 0x100a82fc
undefined4 g_unk0x100a82fc = 0;

// GLOBAL: MW2 0x100a8300
EasedValue g_unk0x100a8300 = {0, 0, 1};

// GLOBAL: MW2 0x100a830c
undefined4 g_unk0x100a830c = 0;

// GLOBAL: MW2 0x100a8310
EasedValue g_unk0x100a8310 = {0, 0, 4};

// GLOBAL: MW2 0x100a831c
undefined4 g_unk0x100a831c = 0;

// GLOBAL: MW2 0x100a8320
EasedValue g_unk0x100a8320 = {0, 0, 1};

// GLOBAL: MW2 0x100a832c
undefined4 g_unk0x100a832c = 0;

// The size of the throttle gauge.
// GLOBAL: MW2 0x100a8330
Point g_unk0x100a8330 = {0x1e7a, 0x8dc9};

// The throttle gauge's frame: left, top, right and bottom.
// GLOBAL: MW2 0x100a8338
MechS32 g_unk0x100a8338 = 0;

// GLOBAL: MW2 0x100a833c
MechS32 g_unk0x100a833c = 0;

// GLOBAL: MW2 0x100a8340
MechS32 g_unk0x100a8340 = 0;

// GLOBAL: MW2 0x100a8344
MechS32 g_unk0x100a8344 = 0;

// The left of the throttle bar and the level of zero throttle.
// GLOBAL: MW2 0x100a8348
MechS32 g_unk0x100a8348 = 0;

// GLOBAL: MW2 0x100a834c
MechS32 g_unk0x100a834c = 0;

// The heat bar: its position and size.
// GLOBAL: MW2 0x100a8350
Point g_unk0x100a8350 = {0, 0x3333};

// GLOBAL: MW2 0x100a8358
Point g_unk0x100a8358 = {0xea4e, 0x5555};

// The heat rate bar.
// GLOBAL: MW2 0x100a8360
Point g_unk0x100a8360 = {0, 0x3333};

// GLOBAL: MW2 0x100a8368
Point g_unk0x100a8368 = {0xdf2e, 0x5555};

// The jump jet fuel bar.
// GLOBAL: MW2 0x100a8370
Point g_unk0x100a8370 = {0, 0x3333};

// GLOBAL: MW2 0x100a8378
Point g_unk0x100a8378 = {0xdf2e, 0x5555};

// Scales the gauges' rectangles to their panels.
// FUNCTION: MW2 0x1004d020
void FUN_1004d020(void)
{
	CockpitPanel* panel;

	panel = g_unk0x100c3280[18];
	FUN_10056bc1(panel->m_target, &g_unk0x100a8330, &g_unk0x100a8330);
	g_unk0x100a8338 = panel->m_width - g_unk0x100a8330.m_x - 1;
	g_unk0x100a833c = panel->m_height - (g_unk0x100a8330.m_y + g_unk0x100a8330.m_y / 2) - 3;
	g_unk0x100a8340 = panel->m_width - 1;
	g_unk0x100a8344 = panel->m_height - 1;
	g_unk0x100a8348 = g_unk0x100a8338 + 1;
	g_unk0x100a834c = panel->m_height - g_unk0x100a8330.m_y / 2 - 2;

	panel = g_unk0x100c3280[20];
	FUN_10056bc1(panel->m_target, &g_unk0x100a8358, &g_unk0x100a8358);
	FUN_10056bc1(panel->m_target, &g_unk0x100a8350, &g_unk0x100a8350);

	panel = g_unk0x100c3280[21];
	FUN_10056bc1(panel->m_target, &g_unk0x100a8368, &g_unk0x100a8368);
	FUN_10056bc1(panel->m_target, &g_unk0x100a8360, &g_unk0x100a8360);

	panel = g_unk0x100c3280[22];
	FUN_10056bc1(panel->m_target, &g_unk0x100a8378, &g_unk0x100a8378);
	FUN_10056bc1(panel->m_target, &g_unk0x100a8370, &g_unk0x100a8370);
}

// Draws the heat bar: a band from each end that meets in the middle as the heat rises.
// Stack-slot permutation of the locals (heat, width, middle, level, middleX, mech, rightX, x, y, edge,
// height, target and color).
// FUNCTION: MW2 0x1004d175
void FUN_1004d175(PANE* p_target)
{
	MechS32 heat;
	MechS32 width;
	MechS32 middle;
	MechS32 level;
	MechS32 middleX;
	Mech* mech;
	MechS32 rightX;
	MechS32 x;
	MechS32 y;
	MechS32 edge;
	MechS32 height;
	MechS32 target;
	MechS32 color;

	mech = g_players[g_localPlayerId]->m_mech;
	heat = mech->m_heat;
	x = g_unk0x100a8350.m_x;
	y = g_unk0x100a8350.m_y;
	width = g_unk0x100a8358.m_x;
	height = g_unk0x100a8358.m_y - 1;
	target = MulDiv64(heat, width, 100);
	g_unk0x100a82f0.m_target = target;
	level = UpdateEasedValue(&g_unk0x100a82f0) >> 16;
	if (level >= width) {
		FUN_1004d8ae(p_target, g_unk0x100a8350.m_x, g_unk0x100a8350.m_y, g_unk0x100a8358.m_x, height, 0xb);
		return;
	}
	else if (level <= 0) {
		FUN_1004d8ae(p_target, g_unk0x100a8350.m_x, g_unk0x100a8350.m_y, g_unk0x100a8358.m_x, height, 7);
		return;
	}
	else if (width >> 1 > level) {
		edge = level;
		color = 7;
	}
	else {
		edge = width - level;
		color = 0xb;
	}

	middle = width - edge * 2;
	middleX = x + edge + 1;
	rightX = middleX + middle;
	FUN_1004d8ae(p_target, x, y, edge, height, 3);
	FUN_1004d8ae(p_target, middleX, y, middle, height, color);
	FUN_1004d8ae(p_target, rightX, y, edge, height, 3);
}

// Draws the heat rate bar.
// Stack-slot permutation: level, backColor, fill, fillColor and mech.
// FUNCTION: MW2 0x1004d310
void FUN_1004d310(PANE* p_target)
{
	MechS32 level;
	MechS32 backColor;
	MechS32 fill;
	MechS32 fillColor;
	Mech* mech;

	mech = g_players[g_localPlayerId]->m_mech;
	fill = 0;
	level = 0;
	g_unk0x100a8310.m_target = (mech->m_deltaHeat - mech->m_cooling * g_deltaTime) >> 6;
	level = UpdateEasedValue(&g_unk0x100a8310);
	if (level < 1) {
		fillColor = 7;
		backColor = 7;
		level = 0;
	}
	else if (level < 0x300) {
		fillColor = 3;
		backColor = 7;
	}
	else {
		fillColor = 0xb;
		backColor = 3;
		level -= 0x300;
	}

	if (level > 0) {
		fill = MulDiv64(g_unk0x100a8368.m_x, level, 0x300);
		if (fill < 0) {
			fill = 0;
		}
		else if (fill > g_unk0x100a8368.m_x) {
			fill = g_unk0x100a8368.m_x;
		}
	}

	if (fill > 0) {
		FUN_1004d8ae(p_target, g_unk0x100a8360.m_x, g_unk0x100a8360.m_y, fill, g_unk0x100a8368.m_y - 1, fillColor);
	}

	if (fill < g_unk0x100a8368.m_x) {
		FUN_1004d8ae(
			p_target,
			g_unk0x100a8360.m_x + fill,
			g_unk0x100a8360.m_y,
			g_unk0x100a8368.m_x - fill,
			g_unk0x100a8368.m_y - 1,
			backColor
		);
	}
}

// Draws the throttle gauge: its frame and a bar up from zero, or down in reverse.
// Stack-slot permutation: color, mech, width, height, x, y and value.
// FUNCTION: MW2 0x1004d48a
void FUN_1004d48a(PANE* p_target)
{
	MechS32 color;
	Mech* mech;
	MechS32 width;
	MechS32 height;
	MechS32 x;
	MechS32 y;
	MechS32 value;

	mech = g_players[g_localPlayerId]->m_mech;
	VFX_line_draw(p_target, g_unk0x100a8338, g_unk0x100a833c, g_unk0x100a8338, g_unk0x100a8344, 0, 10);
	VFX_line_draw(p_target, g_unk0x100a8340, g_unk0x100a833c, g_unk0x100a8340, g_unk0x100a8344, 0, 10);
	VFX_line_draw(p_target, g_unk0x100a8338, g_unk0x100a833c, g_unk0x100a8340, g_unk0x100a833c, 0, 10);
	VFX_line_draw(p_target, g_unk0x100a8338, g_unk0x100a8344, g_unk0x100a8340, g_unk0x100a8344, 0, 10);
	value = mech->m_player->m_steering->m_throttle << 16;
	value = MulDiv64(value, g_unk0x100a8330.m_y, 0x400);
	if (mech->m_player->m_steering->m_reverse) {
		value /= -2;
	}

	g_unk0x100a8300.m_target = value;
	value = UpdateEasedValue(&g_unk0x100a8300) >> 16;
	x = g_unk0x100a8348;
	y = g_unk0x100a834c;
	width = g_unk0x100a8330.m_x - 1;
	height = value;
	color = 0xf;
	if (mech->m_player->m_steering->m_reverse) {
		height = -height;
		if (height > g_unk0x100a8330.m_y / 2) {
			height = g_unk0x100a8330.m_y / 2;
		}

		color = 7;
		y += height;
	}
	else if (height > g_unk0x100a8330.m_y) {
		height = g_unk0x100a8330.m_y;
	}

	height++;
	FUN_1004d732(p_target, x, y, width, height, color);
}

// Draws the jump jet fuel bar.
// FUNCTION: MW2 0x1004d660
void FUN_1004d660(PANE* p_target)
{
	MechS32 fill;
	Mech* mech;

	mech = g_players[g_localPlayerId]->m_mech;
	if (mech->m_jumpFuel >= 0) {
		g_unk0x100a8320.m_target = mech->m_jumpFuel;
		fill = UpdateEasedValue(&g_unk0x100a8320);
		fill = MulDiv64(g_unk0x100a8378.m_x + 1, fill, 0x712);
		if (fill > g_unk0x100a8378.m_x) {
			fill = g_unk0x100a8378.m_x;
		}

		FUN_1004d8ae(p_target, g_unk0x100a8370.m_x, g_unk0x100a8370.m_y, fill, g_unk0x100a8378.m_y - 1, 0xf);
		FUN_1004d8ae(
			p_target,
			g_unk0x100a8370.m_x + fill,
			g_unk0x100a8370.m_y,
			g_unk0x100a8378.m_x - fill,
			g_unk0x100a8378.m_y - 1,
			0xb
		);
	}
}

// Draws a bar p_height up from (p_x, p_y), shaded darker towards its edges.
// Stack-slot permutation: half, dark, end, darker, top and i.
// FUNCTION: MW2 0x1004d732
void FUN_1004d732(PANE* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height, MechS32 p_color)
{
	MechS32 half;
	MechS32 dark;
	MechS32 end;
	MechS32 darker;
	MechS32 top;
	MechS32 i;

	half = p_width / 2;
	end = half / 2;
	top = p_y - p_height;
	dark = p_color - 1;
	darker = p_color - 2;
	if (p_height <= 0) {
		return;
	}

	for (i = 0; i < end; i++) {
		VFX_line_draw(p_target, i + p_x, p_y, i + p_x, top, 0, dark);
	}

	for (i = end; i < half; i++) {
		VFX_line_draw(p_target, i + p_x, p_y, i + p_x, top, 0, p_color);
	}

	end = half + (p_width - half) / 2;
	for (i = half; i < end; i++) {
		VFX_line_draw(p_target, i + p_x, p_y, i + p_x, top, 0, dark);
	}

	for (i = end; p_width > i; i++) {
		VFX_line_draw(p_target, i + p_x, p_y, i + p_x, top, 0, darker);
	}
}

// Draws a bar p_width across from (p_x, p_y), shaded darker towards its edges.
// Stack-slot permutation: half, dark, end, darker, right and i.
// FUNCTION: MW2 0x1004d8ae
void FUN_1004d8ae(PANE* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height, MechS32 p_color)
{
	MechS32 half;
	MechS32 dark;
	MechS32 end;
	MechS32 darker;
	MechS32 right;
	MechS32 i;

	half = p_height / 2;
	end = half / 2;
	right = p_width + p_x;
	dark = p_color - 1;
	darker = p_color - 2;
	if (p_width <= 0) {
		return;
	}

	for (i = 0; i < end; i++) {
		VFX_line_draw(p_target, p_x, i + p_y, right, i + p_y, 0, dark);
	}

	for (i = end; i < half; i++) {
		VFX_line_draw(p_target, p_x, i + p_y, right, i + p_y, 0, p_color);
	}

	end = half + (p_height - half) / 2;
	for (i = half; i < end; i++) {
		VFX_line_draw(p_target, p_x, i + p_y, right, i + p_y, 0, dark);
	}

	for (i = end; p_height > i; i++) {
		VFX_line_draw(p_target, p_x, i + p_y, right, i + p_y, 0, darker);
	}
}
