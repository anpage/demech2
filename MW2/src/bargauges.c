#include "bargauges.h"

#include "clock.h"
#include "cockpitpanel.h"
#include "config.h"
#include "decomp.h"
#include "environment.h"
#include "fixedfloat.h"
#include "muldiv.h"
#include "palette.h"
#include "players.h"
#include "point.h"
#include "ramp.h"
#include "screenscale.h"
#include "simmain.h"
#include "targeting.h"
#include "types.h"

#ifdef MW2_ACCEL
#include "accel/vfx16.h"
#endif

// The cockpit's bar gauges: the heat, the rate the heat changes at, the throttle and the jump
// jet fuel. Each eases towards its value, and the rectangles are fractions of their panels
// until ScaleBarGauges scales them to pixels.

// GLOBAL: MW2 0x100a82f0
// GLOBAL: MW2MATROX 0x100bbd50
// GLOBAL: MW23DFX 0x1024c818
EasedValue g_heatBarLevel = {0, 0, 1};

// GLOBAL: MW2 0x100a82fc
// GLOBAL: MW23DFX 0x1024c824
undefined4 g_unk0x100a82fc = 0;

// GLOBAL: MW2 0x100a8300
// GLOBAL: MW2MATROX 0x100bbd60
// GLOBAL: MW23DFX 0x1024c828
EasedValue g_throttleBarLevel = {0, 0, 1};

// GLOBAL: MW2 0x100a830c
// GLOBAL: MW23DFX 0x1024c834
undefined4 g_unk0x100a830c = 0;

// GLOBAL: MW2 0x100a8310
// GLOBAL: MW2MATROX 0x100bbd70
// GLOBAL: MW23DFX 0x1024c838
EasedValue g_heatRateBarLevel = {0, 0, 4};

// GLOBAL: MW2 0x100a831c
// GLOBAL: MW23DFX 0x1024c844
undefined4 g_unk0x100a831c = 0;

// GLOBAL: MW2 0x100a8320
// GLOBAL: MW2MATROX 0x100bbd80
// GLOBAL: MW23DFX 0x1024c848
EasedValue g_jumpFuelBarLevel = {0, 0, 1};

// GLOBAL: MW2 0x100a832c
// GLOBAL: MW23DFX 0x1024c854
undefined4 g_unk0x100a832c = 0;

// The throttle gauge: its size, its frame (left, top, right and bottom), the left of its bar
// and the level of zero throttle. One block: the 3Dfx edition's ScaleBarGauges reads the panel
// again after each store to it, as it does only for an object whose address it passed on.
typedef struct ThrottleGauge {
	Point m_size;          // 0x00
	MechS32 m_frameLeft;   // 0x08
	MechS32 m_frameTop;    // 0x0c
	MechS32 m_frameRight;  // 0x10
	MechS32 m_frameBottom; // 0x14
	MechS32 m_barLeft;     // 0x18
	MechS32 m_zeroY;       // 0x1c
} ThrottleGauge;

DECOMP_SIZE_ASSERT(ThrottleGauge, 0x20)

// GLOBAL: MW2 0x100a8330
// GLOBAL: MW2MATROX 0x100bbd90
// GLOBAL: MW23DFX 0x1024c858
ThrottleGauge g_throttleGauge = {{0x1e7a, 0x8dc9}, 0, 0, 0, 0, 0, 0};

// The heat bar: its position and size.
// GLOBAL: MW2 0x100a8350
// GLOBAL: MW2MATROX 0x100bbdb0
// GLOBAL: MW23DFX 0x1024c878
Point g_heatBarPosition = {0, 0x3333};

// GLOBAL: MW2 0x100a8358
// GLOBAL: MW2MATROX 0x100bbdb8
// GLOBAL: MW23DFX 0x1024c880
Point g_heatBarSize = {0xea4e, 0x5555};

// The heat rate bar.
// GLOBAL: MW2 0x100a8360
// GLOBAL: MW2MATROX 0x100bbdc0
// GLOBAL: MW23DFX 0x1024c888
Point g_heatRateBarPosition = {0, 0x3333};

// GLOBAL: MW2 0x100a8368
// GLOBAL: MW2MATROX 0x100bbdc8
// GLOBAL: MW23DFX 0x1024c890
Point g_heatRateBarSize = {0xdf2e, 0x5555};

// The jump jet fuel bar.
// GLOBAL: MW2 0x100a8370
// GLOBAL: MW2MATROX 0x100bbdd0
// GLOBAL: MW23DFX 0x1024c898
Point g_jumpFuelBarPosition = {0, 0x3333};

// GLOBAL: MW2 0x100a8378
// GLOBAL: MW2MATROX 0x100bbdd8
// GLOBAL: MW23DFX 0x1024c8a0
Point g_jumpFuelBarSize = {0xdf2e, 0x5555};

// Scales the gauges' rectangles to their panels.
// FUNCTION: MW2 0x1004d020
// FUNCTION: MW2MATROX 0x10089040
// FUNCTION: MW23DFX 0x1001c250
void ScaleBarGauges(void)
{
	CockpitPanel* panel;

	panel = g_cockpitPanels[c_panelSpeed];
	ScalePointToFrame(panel->m_target, &g_throttleGauge.m_size, &g_throttleGauge.m_size);
	g_throttleGauge.m_frameLeft = panel->m_width - g_throttleGauge.m_size.m_x - 1;
	g_throttleGauge.m_frameTop = panel->m_height - (g_throttleGauge.m_size.m_y + g_throttleGauge.m_size.m_y / 2) - 3;
	g_throttleGauge.m_frameRight = panel->m_width - 1;
	g_throttleGauge.m_frameBottom = panel->m_height - 1;
	g_throttleGauge.m_barLeft = g_throttleGauge.m_frameLeft + 1;
	g_throttleGauge.m_zeroY = panel->m_height - g_throttleGauge.m_size.m_y / 2 - 2;

	panel = g_cockpitPanels[c_panelHeat];
	ScalePointToFrame(panel->m_target, &g_heatBarSize, &g_heatBarSize);
	ScalePointToFrame(panel->m_target, &g_heatBarPosition, &g_heatBarPosition);

	panel = g_cockpitPanels[c_panelHeatRate];
	ScalePointToFrame(panel->m_target, &g_heatRateBarSize, &g_heatRateBarSize);
	ScalePointToFrame(panel->m_target, &g_heatRateBarPosition, &g_heatRateBarPosition);

	panel = g_cockpitPanels[c_panelJets];
	ScalePointToFrame(panel->m_target, &g_jumpFuelBarSize, &g_jumpFuelBarSize);
	ScalePointToFrame(panel->m_target, &g_jumpFuelBarPosition, &g_jumpFuelBarPosition);
}

// Draws the heat bar: a band from each end that meets in the middle as the heat rises.
// Stack-slot permutation of the locals (heat, width, middle, level, middleX, mech, rightX, x, y, edge,
// height, target and color).
// MW2MATROX: the heat converts as the Matrox edition does (fld, __ftol); the remaining diff is a
// stack-slot permutation, the operand order of `level >= width`, and DrawHorizontalBar, which
// the Matrox edition changed and has no MW2MATROX annotation yet.
// The 3Dfx edition's diff is register allocation, scheduling and comparison operand order only.
// FUNCTION: MW2 0x1004d175
// FUNCTION: MW2MATROX 0x10089195
// FUNCTION: MW23DFX 0x1001c350
void DrawHeatBar(PANE* p_target)
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
	x = g_heatBarPosition.m_x;
	y = g_heatBarPosition.m_y;
	width = g_heatBarSize.m_x;
	height = g_heatBarSize.m_y - 1;
	target = MulDiv64(heat, width, 100);
	g_heatBarLevel.m_target = target;
#ifdef MW2_ACCEL
	// The Matrox edition's heat is in plain units already.
	level = UpdateEasedValue(&g_heatBarLevel);
#else
	level = UpdateEasedValue(&g_heatBarLevel) >> 16;
#endif
	if (level >= width) {
		DrawHorizontalBar(p_target, g_heatBarPosition.m_x, g_heatBarPosition.m_y, g_heatBarSize.m_x, height, 0xb);
		return;
	}
	else if (level <= 0) {
		DrawHorizontalBar(p_target, g_heatBarPosition.m_x, g_heatBarPosition.m_y, g_heatBarSize.m_x, height, 7);
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
	DrawHorizontalBar(p_target, x, y, edge, height, 3);
	DrawHorizontalBar(p_target, middleX, y, middle, height, color);
	DrawHorizontalBar(p_target, rightX, y, edge, height, 3);
}

// Draws the heat rate bar.
// Stack-slot permutation: level, backColor, fill, fillColor and mech.
// The 3Dfx edition's diff is register allocation and instruction scheduling only.
// FUNCTION: MW2 0x1004d310
// FUNCTION: MW2MATROX 0x10089332
// FUNCTION: MW23DFX 0x1001c4a0
void DrawHeatRateBar(PANE* p_target)
{
	MechS32 level;
	MechS32 backColor;
	MechS32 fill;
	MechS32 fillColor;
	Mech* mech;

	mech = g_players[g_localPlayerId]->m_mech;
	fill = 0;
	level = 0;
#ifdef MW2_ACCEL
	g_heatRateBarLevel.m_target = (MechS32) (mech->m_deltaHeat - mech->m_cooling * g_deltaTime) << 10;
#else
	g_heatRateBarLevel.m_target = FIXED_SHR(mech->m_deltaHeat - mech->m_cooling * g_deltaTime, 6);
#endif
	level = UpdateEasedValue(&g_heatRateBarLevel);
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
		fill = MulDiv64(g_heatRateBarSize.m_x, level, 0x300);
		if (fill < 0) {
			fill = 0;
		}
		else if (fill > g_heatRateBarSize.m_x) {
			fill = g_heatRateBarSize.m_x;
		}
	}

	if (fill > 0) {
		DrawHorizontalBar(
			p_target,
			g_heatRateBarPosition.m_x,
			g_heatRateBarPosition.m_y,
			fill,
			g_heatRateBarSize.m_y - 1,
			fillColor
		);
	}

	if (fill < g_heatRateBarSize.m_x) {
		DrawHorizontalBar(
			p_target,
			g_heatRateBarPosition.m_x + fill,
			g_heatRateBarPosition.m_y,
			g_heatRateBarSize.m_x - fill,
			g_heatRateBarSize.m_y - 1,
			backColor
		);
	}
}

// Draws the throttle gauge: its frame and a bar up from zero, or down in reverse.
// Stack-slot permutation: color, mech, width, height, x, y and value.
// The 3Dfx edition's diff is register allocation and instruction scheduling only.
// FUNCTION: MW2 0x1004d48a
// FUNCTION: MW2MATROX 0x100894b4
// FUNCTION: MW23DFX 0x1001c5b0
void DrawThrottleGauge(PANE* p_target)
{
	MechS32 color;
	Mech* mech;
	MechS32 width;
	MechS32 height;
	MechS32 x;
	MechS32 y;
	MechS32 value;

	mech = g_players[g_localPlayerId]->m_mech;
	VFX_line_draw(
		p_target,
		g_throttleGauge.m_frameLeft,
		g_throttleGauge.m_frameTop,
		g_throttleGauge.m_frameLeft,
		g_throttleGauge.m_frameBottom,
		0,
		PIXEL_COLOR(10)
	);
	VFX_line_draw(
		p_target,
		g_throttleGauge.m_frameRight,
		g_throttleGauge.m_frameTop,
		g_throttleGauge.m_frameRight,
		g_throttleGauge.m_frameBottom,
		0,
		PIXEL_COLOR(10)
	);
	VFX_line_draw(
		p_target,
		g_throttleGauge.m_frameLeft,
		g_throttleGauge.m_frameTop,
		g_throttleGauge.m_frameRight,
		g_throttleGauge.m_frameTop,
		0,
		PIXEL_COLOR(10)
	);
	VFX_line_draw(
		p_target,
		g_throttleGauge.m_frameLeft,
		g_throttleGauge.m_frameBottom,
		g_throttleGauge.m_frameRight,
		g_throttleGauge.m_frameBottom,
		0,
		PIXEL_COLOR(10)
	);
#ifdef MW2_ACCEL
	value = (MechS32) (mech->m_player->m_steering->m_throttle * 65536.0f + 0.5f) << 16;
#else
	value = mech->m_player->m_steering->m_throttle << 16;
#endif
	value = MulDiv64(value, g_throttleGauge.m_size.m_y, 0x400);
	if (mech->m_player->m_steering->m_reverse) {
		value /= -2;
	}

	g_throttleBarLevel.m_target = value;
	value = UpdateEasedValue(&g_throttleBarLevel) >> 16;
	x = g_throttleGauge.m_barLeft;
	y = g_throttleGauge.m_zeroY;
	width = g_throttleGauge.m_size.m_x - 1;
	height = value;
	color = 0xf;
	if (mech->m_player->m_steering->m_reverse) {
		height = -height;
		if (height > g_throttleGauge.m_size.m_y / 2) {
			height = g_throttleGauge.m_size.m_y / 2;
		}

		color = 7;
		y += height;
	}
	else if (height > g_throttleGauge.m_size.m_y) {
		height = g_throttleGauge.m_size.m_y;
	}

	height++;
	DrawVerticalBar(p_target, x, y, width, height, color);
}

// Draws the jump jet fuel bar.
// The 3Dfx edition's diff is register allocation, scheduling and comparison operand order only.
// FUNCTION: MW2 0x1004d660
// FUNCTION: MW2MATROX 0x100896bf
// FUNCTION: MW23DFX 0x1001c750
void DrawJumpFuelBar(PANE* p_target)
{
	MechS32 fill;
	Mech* mech;

	mech = g_players[g_localPlayerId]->m_mech;
	if (mech->m_jumpFuel >= 0) {
		g_jumpFuelBarLevel.m_target = mech->m_jumpFuel;
		fill = UpdateEasedValue(&g_jumpFuelBarLevel);
		fill = MulDiv64(g_jumpFuelBarSize.m_x + 1, fill, 0x712);
		if (fill > g_jumpFuelBarSize.m_x) {
			fill = g_jumpFuelBarSize.m_x;
		}

		DrawHorizontalBar(
			p_target,
			g_jumpFuelBarPosition.m_x,
			g_jumpFuelBarPosition.m_y,
			fill,
			g_jumpFuelBarSize.m_y - 1,
			0xf
		);
		DrawHorizontalBar(
			p_target,
			g_jumpFuelBarPosition.m_x + fill,
			g_jumpFuelBarPosition.m_y,
			g_jumpFuelBarSize.m_x - fill,
			g_jumpFuelBarSize.m_y - 1,
			0xb
		);
	}
}

// Draws a bar p_height up from (p_x, p_y), shaded darker towards its edges.
// Stack-slot permutation: half, dark, end, darker, top and i.
// The 3Dfx edition's diff is register allocation and instruction scheduling only.
// FUNCTION: MW2 0x1004d732
// FUNCTION: MW2MATROX 0x10089791
// FUNCTION: MW23DFX 0x1001c810
void DrawVerticalBar(PANE* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height, MechS32 p_color)
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
		VFX_line_draw(p_target, i + p_x, p_y, i + p_x, top, 0, PIXEL_COLOR(dark));
	}

	for (i = end; i < half; i++) {
		VFX_line_draw(p_target, i + p_x, p_y, i + p_x, top, 0, PIXEL_COLOR(p_color));
	}

	end = half + (p_width - half) / 2;
	for (i = half; i < end; i++) {
		VFX_line_draw(p_target, i + p_x, p_y, i + p_x, top, 0, PIXEL_COLOR(dark));
	}

	for (i = end; p_width > i; i++) {
		VFX_line_draw(p_target, i + p_x, p_y, i + p_x, top, 0, PIXEL_COLOR(darker));
	}
}

// Draws a bar p_width across from (p_x, p_y), shaded darker towards its edges.
// Stack-slot permutation: half, dark, end, darker, right and i.
// The 3Dfx edition's diff is register allocation, scheduling and comparison operand order only.
// FUNCTION: MW2 0x1004d8ae
// FUNCTION: MW2MATROX 0x10089931
// FUNCTION: MW23DFX 0x1001c960
void DrawHorizontalBar(PANE* p_target, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height, MechS32 p_color)
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
		VFX_line_draw(p_target, p_x, i + p_y, right, i + p_y, 0, PIXEL_COLOR(dark));
	}

	for (i = end; i < half; i++) {
		VFX_line_draw(p_target, p_x, i + p_y, right, i + p_y, 0, PIXEL_COLOR(p_color));
	}

	end = half + (p_height - half) / 2;
	for (i = half; i < end; i++) {
		VFX_line_draw(p_target, p_x, i + p_y, right, i + p_y, 0, PIXEL_COLOR(dark));
	}

	for (i = end; p_height > i; i++) {
		VFX_line_draw(p_target, p_x, i + p_y, right, i + p_y, 0, PIXEL_COLOR(darker));
	}
}
