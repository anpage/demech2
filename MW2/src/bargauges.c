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

#ifdef MW2_MFD_PANELS
#include "fixeddiv.h"
#include "fixedmul.h"
#include "fixedmul29.h"
#include "fixedtrig.h"
#endif

#ifdef MW2_VFX16
#include "accel/vfx16.h"
#endif

// The cockpit's bar gauges: the heat, the rate the heat changes at, the throttle and the jump
// jet fuel. Each eases towards its value, and the rectangles are fractions of their panels
// until ScaleBarGauges scales them to pixels.

// GLOBAL: MW2 0x100a82f0
// GLOBAL: MW2GBL 0x100b5910
// GLOBAL: MW2PREMERCS 0x100b20b0
// GLOBAL: MW2MATROX 0x100bbd50
// GLOBAL: MW23DFX 0x1024c818
// GLOBAL: MW2ATI 0x10180b98
// GLOBAL: MW2S3 0x10064120
// GLOBAL: MW2POWERVR 0x101aba70
EasedValue g_heatBarLevel = {0, 0, 1};

// GLOBAL: MW2 0x100a82fc
// GLOBAL: MW2GBL 0x100b591c
// GLOBAL: MW2PREMERCS 0x100b20bc
// GLOBAL: MW23DFX 0x1024c824
undefined4 g_unk0x100a82fc = 0;

// GLOBAL: MW2 0x100a8300
// GLOBAL: MW2GBL 0x100b5920
// GLOBAL: MW2PREMERCS 0x100b20c0
// GLOBAL: MW2MATROX 0x100bbd60
// GLOBAL: MW23DFX 0x1024c828
// GLOBAL: MW2ATI 0x10180ba8
// GLOBAL: MW2S3 0x10064130
// GLOBAL: MW2POWERVR 0x101aba80
EasedValue g_throttleBarLevel = {0, 0, 1};

// GLOBAL: MW2 0x100a830c
// GLOBAL: MW2GBL 0x100b592c
// GLOBAL: MW2PREMERCS 0x100b20cc
// GLOBAL: MW23DFX 0x1024c834
undefined4 g_unk0x100a830c = 0;

// GLOBAL: MW2 0x100a8310
// GLOBAL: MW2GBL 0x100b5930
// GLOBAL: MW2PREMERCS 0x100b20d0
// GLOBAL: MW2MATROX 0x100bbd70
// GLOBAL: MW23DFX 0x1024c838
// GLOBAL: MW2ATI 0x10180bb8
// GLOBAL: MW2S3 0x10064140
// GLOBAL: MW2POWERVR 0x101aba90
EasedValue g_heatRateBarLevel = {0, 0, 4};

// GLOBAL: MW2 0x100a831c
// GLOBAL: MW2GBL 0x100b593c
// GLOBAL: MW2PREMERCS 0x100b20dc
// GLOBAL: MW23DFX 0x1024c844
undefined4 g_unk0x100a831c = 0;

// GLOBAL: MW2 0x100a8320
// GLOBAL: MW2GBL 0x100b5940
// GLOBAL: MW2PREMERCS 0x100b20e0
// GLOBAL: MW2MATROX 0x100bbd80
// GLOBAL: MW23DFX 0x1024c848
// GLOBAL: MW2ATI 0x10180bc8
// GLOBAL: MW2S3 0x10064150
// GLOBAL: MW2POWERVR 0x101abaa0
EasedValue g_jumpFuelBarLevel = {0, 0, 1};

// GLOBAL: MW2 0x100a832c
// GLOBAL: MW2GBL 0x100b594c
// GLOBAL: MW23DFX 0x1024c854
undefined4 g_unk0x100a832c = 0;

#ifdef MW2_MFD_PANELS
// The early Mercenaries build keeps each MFD's gauges in the panel (CockpitPanel::m_throttleSize
// on), draws the heat as a dial of ticks and the jump fuel as a vertical gauge, and draws into the
// panel's pane.
// FUNCTION: MW2PREMERCS 0x10047590
void ScaleHeatBar(CockpitPanel* p_panel)
{
	p_panel->m_heatPosition.m_x = 0;
	p_panel->m_heatPosition.m_y = 0x3333;
	p_panel->m_heatSize.m_x = 0xea4e;
	p_panel->m_heatSize.m_y = 0x5555;
	ScalePointToFrame(p_panel->m_target, &p_panel->m_heatSize, &p_panel->m_heatSize);
	ScalePointToFrame(p_panel->m_target, &p_panel->m_heatPosition, &p_panel->m_heatPosition);
}

// FUNCTION: MW2PREMERCS 0x10047606
void ScaleHeatRateBar(CockpitPanel* p_panel)
{
	p_panel->m_heatRatePosition.m_x = 0;
	p_panel->m_heatRatePosition.m_y = 0x8000;
	p_panel->m_heatRateSize.m_x = 0x8000;
	p_panel->m_heatRateSize.m_y = 0x2db7;
	ScalePointToFrame(p_panel->m_target, &p_panel->m_heatRateSize, &p_panel->m_heatRateSize);
	ScalePointToFrame(p_panel->m_target, &p_panel->m_heatRatePosition, &p_panel->m_heatRatePosition);
}

// FUNCTION: MW2PREMERCS 0x1004768d
void ScaleThrottleGauge(CockpitPanel* p_panel)
{
	p_panel->m_throttleSize.m_x = 0x1e7a;
	p_panel->m_throttleSize.m_y = 0x8dc9;
	ScalePointToFrame(p_panel->m_target, &p_panel->m_throttleSize, &p_panel->m_throttleSize);
	p_panel->m_throttleFrameLeft = p_panel->m_width - p_panel->m_throttleSize.m_x - 1;
	p_panel->m_throttleFrameTop =
		p_panel->m_height - (p_panel->m_throttleSize.m_y + p_panel->m_throttleSize.m_y / 2) - 3;
	p_panel->m_throttleFrameRight = p_panel->m_width - 1;
	p_panel->m_throttleFrameBottom = p_panel->m_height - 1;
	p_panel->m_throttleBarLeft = p_panel->m_throttleFrameLeft + 1;
	p_panel->m_throttleZeroY = p_panel->m_height - p_panel->m_throttleSize.m_y / 2 - 2;
}

// FUNCTION: MW2PREMERCS 0x1004775a
void ScaleJumpFuelBar(CockpitPanel* p_panel)
{
	p_panel->m_jumpFuelSize.m_x = 0x1e7a;
	p_panel->m_jumpFuelSize.m_y = 0x8dc9;
	p_panel->m_jumpFuelPosition.m_x = 0;
	p_panel->m_jumpFuelPosition.m_y = 0;
	ScalePointToFrame(p_panel->m_target, &p_panel->m_jumpFuelSize, &p_panel->m_jumpFuelSize);
	ScalePointToFrame(p_panel->m_target, &p_panel->m_jumpFuelPosition, &p_panel->m_jumpFuelPosition);
	p_panel->m_jumpFuelFrameLeft = 0;
	p_panel->m_jumpFuelFrameTop =
		p_panel->m_height - (p_panel->m_jumpFuelSize.m_y + p_panel->m_jumpFuelSize.m_y / 2) - 3;
	p_panel->m_jumpFuelFrameRight = p_panel->m_jumpFuelSize.m_x;
	p_panel->m_jumpFuelFrameBottom = p_panel->m_height - 1;
	p_panel->m_jumpFuelPosition.m_x = p_panel->m_jumpFuelFrameLeft + 1;
	p_panel->m_jumpFuelPosition.m_y = p_panel->m_height - 2;
	p_panel->m_jumpFuelSize.m_y = p_panel->m_jumpFuelFrameBottom - p_panel->m_jumpFuelFrameTop - 2;
}

// Scales every MFD's gauges.
// FUNCTION: MW2PREMERCS 0x10047851
void ScaleBarGauges(void)
{
	ScaleThrottleGauge(g_cockpitPanels[22]);
	ScaleThrottleGauge(g_cockpitPanels[2]);
	ScaleThrottleGauge(g_cockpitPanels[17]);
	ScaleThrottleGauge(g_cockpitPanels[26]);
	ScaleThrottleGauge(g_cockpitPanels[31]);
	ScaleThrottleGauge(g_cockpitPanels[32]);
	ScaleThrottleGauge(g_cockpitPanels[33]);
	ScaleHeatBar(g_cockpitPanels[22]);
	ScaleHeatBar(g_cockpitPanels[2]);
	ScaleHeatBar(g_cockpitPanels[17]);
	ScaleHeatBar(g_cockpitPanels[26]);
	ScaleHeatBar(g_cockpitPanels[31]);
	ScaleHeatBar(g_cockpitPanels[32]);
	ScaleHeatBar(g_cockpitPanels[33]);
	ScaleHeatRateBar(g_cockpitPanels[22]);
	ScaleHeatRateBar(g_cockpitPanels[2]);
	ScaleHeatRateBar(g_cockpitPanels[17]);
	ScaleHeatRateBar(g_cockpitPanels[26]);
	ScaleHeatRateBar(g_cockpitPanels[31]);
	ScaleHeatRateBar(g_cockpitPanels[32]);
	ScaleHeatRateBar(g_cockpitPanels[33]);
	ScaleJumpFuelBar(g_cockpitPanels[26]);
	ScaleJumpFuelBar(g_cockpitPanels[2]);
	ScaleJumpFuelBar(g_cockpitPanels[17]);
	ScaleJumpFuelBar(g_cockpitPanels[22]);
	ScaleJumpFuelBar(g_cockpitPanels[31]);
	ScaleJumpFuelBar(g_cockpitPanels[32]);
	ScaleJumpFuelBar(g_cockpitPanels[33]);
}

// Draws a tick of the heat dial at p_angle: from the pane's ellipse in to p_inner's rectangle,
// centered on the pane, or to the center without p_inner.
// FUNCTION: MW2PREMERCS 0x100479e4
void DrawDialTick(PANE* p_pane, MechScalar p_angle, MechS32 p_color, PANE* p_inner)
{
	Point end;
	Point point;
	Point inner;
	MechScalar sin;
	MechScalar cos;
	MechS32 scale;

	scale = (p_pane->m_y1 - p_pane->m_y0 + 1) << 16;
	sin = FixedSin(p_angle);
	cos = FixedCos(p_angle);
	point.m_x = ((p_pane->m_x1 - p_pane->m_x0 + 1) >> 1) + (FixedMul29(scale, cos) >> 16);
	point.m_y = ((p_pane->m_y1 - p_pane->m_y0 + 1) >> 1) + (FixedMul29(scale, sin) >> 16);
	GetEllipseNeedleToward(p_pane, &point, &end);
	if (p_inner) {
		scale = (p_inner->m_x1 - p_inner->m_x0 + 1) << 16;
		point.m_x = ((p_inner->m_x1 - p_inner->m_x0 + 1) >> 1) + (FixedMul29(scale, cos) >> 16);
		point.m_y = ((p_inner->m_y1 - p_inner->m_y0 + 1) >> 1) + (FixedMul29(scale, sin) >> 16);
		GetRectNeedleToward(p_inner, &point, &inner);
		inner.m_x += ((p_pane->m_x1 - p_pane->m_x0 + 1) >> 1) - ((p_inner->m_x1 - p_inner->m_x0 + 1) >> 1);
		inner.m_y += ((p_pane->m_y1 - p_pane->m_y0 + 1) >> 1) - ((p_inner->m_y1 - p_inner->m_y0 + 1) >> 1);
		VFX_line_draw(p_pane, end.m_x, end.m_y, inner.m_x, inner.m_y, 0, p_color);
	}
	else {
		VFX_line_draw(
			p_pane,
			end.m_x,
			end.m_y,
			(p_pane->m_x1 - p_pane->m_x0 + 1) >> 1,
			(p_pane->m_y1 - p_pane->m_y0 + 1) >> 1,
			0,
			p_color
		);
	}
}

// Draws the heat dial: a tick every 15 degrees up to the heat, from 270 degrees on, yellow past
// a third and red past three quarters. The heat bar's place and height go unused.
// FUNCTION: MW2PREMERCS 0x10047bab
void DrawHeatBar(CockpitPanel* p_panel)
{
	PANE* pane;
	Mech* mech;
	MechS32 heat;
	MechS32 i;
	MechS32 color;
	MechS32 x;
	MechS32 y;
	MechS32 width;
	MechS32 height;
	MechS32 target;
	MechS32 level;
	MechScalar fraction;
	MechS32 angle;
	MechScalar tick;

	pane = p_panel->m_target;
	mech = g_players[g_localPlayerId]->m_mech;
	heat = mech->m_heat;
	i = 0;
	color = 0xe;
	x = p_panel->m_heatPosition.m_x;
	y = p_panel->m_heatPosition.m_y;
	width = p_panel->m_heatSize.m_x;
	height = p_panel->m_heatSize.m_y - 1;
	target = MulDiv64(heat, width, 100);
	g_heatBarLevel.m_target = target;
	level = UpdateEasedValue(&g_heatBarLevel) >> 16;
	VFX_line_draw(
		pane,
		(pane->m_x1 - pane->m_x0 + 1) >> 1,
		0,
		(pane->m_x1 - pane->m_x0 + 1) >> 1,
		(pane->m_y1 - pane->m_y0 + 1) >> 1,
		0,
		2
	);
	if (width > level) {
		fraction = FixedDiv16(level << 16, width << 16);
		angle = FixedMul16(fraction, FIXED_CONST(360)) >> 16;
	}
	else {
		angle = 0x163;
	}

	if (angle) {
		for (i = 0; i < angle; i += 15) {
			if (i > 0x78 && i < 0x10e) {
				color = 3;
			}
			else if (i >= 0x10e) {
				color = 10;
			}

			tick = (i + 0x10e) % 0x168 << 16;
			DrawDialTick(pane, tick, color, g_cockpitPanels[30]->m_target);
		}

		if (angle > 0x78 && angle < 0x10e) {
			color = 3;
		}
		else if (angle >= 0x10e) {
			color = 10;
		}

		tick = (angle + 0x10e) % 0x168;
		tick <<= 16;
		DrawDialTick(pane, tick, color, g_cockpitPanels[30]->m_target);
	}
}

// FUNCTION: MW2PREMERCS 0x10047ddb
void DrawHeatRateBar(CockpitPanel* p_panel)
{
	PANE* target;
	Mech* mech;
	MechS32 level;
	MechS32 fill;
	MechS32 fillColor;
	MechS32 backColor;

	target = p_panel->m_target;
	mech = g_players[g_localPlayerId]->m_mech;
	fill = 0;
	level = 0;
	g_heatRateBarLevel.m_target = FIXED_SHR(mech->m_deltaHeat - mech->m_cooling * g_deltaTime, 6);
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
		fill = MulDiv64(p_panel->m_heatRateSize.m_x, level, 0x300);
		if (fill < 0) {
			fill = 0;
		}
		else if (fill > p_panel->m_heatRateSize.m_x) {
			fill = p_panel->m_heatRateSize.m_x;
		}
	}

	if (fill > 0) {
		DrawHorizontalBar(
			target,
			p_panel->m_heatRatePosition.m_x,
			p_panel->m_heatRatePosition.m_y,
			fill,
			p_panel->m_heatRateSize.m_y - 1,
			fillColor
		);
	}

	if (fill < p_panel->m_heatRateSize.m_x) {
		DrawHorizontalBar(
			target,
			p_panel->m_heatRatePosition.m_x + fill,
			p_panel->m_heatRatePosition.m_y,
			p_panel->m_heatRateSize.m_x - fill,
			p_panel->m_heatRateSize.m_y - 1,
			backColor
		);
	}
}

// FUNCTION: MW2PREMERCS 0x10047f8b
void DrawThrottleGauge(CockpitPanel* p_panel)
{
	MechS32 color;
	Mech* mech;
	MechS32 width;
	MechS32 height;
	MechS32 x;
	MechS32 y;
	MechS32 value;
	PANE* target;

	target = p_panel->m_target;
	mech = g_players[g_localPlayerId]->m_mech;
	VFX_line_draw(
		target,
		p_panel->m_throttleFrameLeft,
		p_panel->m_throttleFrameTop,
		p_panel->m_throttleFrameLeft,
		p_panel->m_throttleFrameBottom,
		0,
		PIXEL_COLOR(10)
	);
	VFX_line_draw(
		target,
		p_panel->m_throttleFrameRight,
		p_panel->m_throttleFrameTop,
		p_panel->m_throttleFrameRight,
		p_panel->m_throttleFrameBottom,
		0,
		PIXEL_COLOR(10)
	);
	VFX_line_draw(
		target,
		p_panel->m_throttleFrameLeft,
		p_panel->m_throttleFrameTop,
		p_panel->m_throttleFrameRight,
		p_panel->m_throttleFrameTop,
		0,
		PIXEL_COLOR(10)
	);
	VFX_line_draw(
		target,
		p_panel->m_throttleFrameLeft,
		p_panel->m_throttleFrameBottom,
		p_panel->m_throttleFrameRight,
		p_panel->m_throttleFrameBottom,
		0,
		PIXEL_COLOR(10)
	);
	value = mech->m_player->m_steering->m_throttle << 16;
	value = MulDiv64(value, p_panel->m_throttleSize.m_y, 0x400);
	if (mech->m_player->m_steering->m_reverse) {
		value /= -2;
	}

	g_throttleBarLevel.m_target = value;
	value = UpdateEasedValue(&g_throttleBarLevel) >> 16;
	x = p_panel->m_throttleBarLeft;
	y = p_panel->m_throttleZeroY;
	width = p_panel->m_throttleSize.m_x - 1;
	height = value;
	color = 0xf;
	if (mech->m_player->m_steering->m_reverse) {
		height = -height;
		if (height > p_panel->m_throttleSize.m_y / 2) {
			height = p_panel->m_throttleSize.m_y / 2;
		}

		color = 7;
		y += height;
	}
	else if (height > p_panel->m_throttleSize.m_y) {
		height = p_panel->m_throttleSize.m_y;
	}

	height++;
	DrawVerticalBar(target, x, y, width, height, color);
}

// FUNCTION: MW2PREMERCS 0x10048184
void DrawJumpFuelBar(CockpitPanel* p_panel)
{
	PANE* target;
	Mech* mech;
	MechS32 fill;

	target = p_panel->m_target;
	mech = g_players[g_localPlayerId]->m_mech;
	VFX_line_draw(
		target,
		p_panel->m_jumpFuelFrameLeft,
		p_panel->m_jumpFuelFrameTop,
		p_panel->m_jumpFuelFrameLeft,
		p_panel->m_jumpFuelFrameBottom,
		0,
		10
	);
	VFX_line_draw(
		target,
		p_panel->m_jumpFuelFrameRight,
		p_panel->m_jumpFuelFrameTop,
		p_panel->m_jumpFuelFrameRight,
		p_panel->m_jumpFuelFrameBottom,
		0,
		10
	);
	VFX_line_draw(
		target,
		p_panel->m_jumpFuelFrameLeft,
		p_panel->m_jumpFuelFrameTop,
		p_panel->m_jumpFuelFrameRight,
		p_panel->m_jumpFuelFrameTop,
		0,
		10
	);
	VFX_line_draw(
		target,
		p_panel->m_jumpFuelFrameLeft,
		p_panel->m_jumpFuelFrameBottom,
		p_panel->m_jumpFuelFrameRight,
		p_panel->m_jumpFuelFrameBottom,
		0,
		10
	);
	if (mech->m_jumpFuel >= 0) {
		g_jumpFuelBarLevel.m_target = mech->m_jumpFuel;
		fill = UpdateEasedValue(&g_jumpFuelBarLevel);
		fill = MulDiv64(p_panel->m_jumpFuelSize.m_y + 1, fill, 0x712);
		if (fill > p_panel->m_jumpFuelSize.m_y) {
			fill = p_panel->m_jumpFuelSize.m_y;
		}

		DrawVerticalBar(
			target,
			p_panel->m_jumpFuelPosition.m_x,
			p_panel->m_jumpFuelPosition.m_y,
			p_panel->m_jumpFuelSize.m_x - 1,
			fill,
			0xb
		);
	}
}

// FUNCTION: MW2PREMERCS 0x100482ea
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

#else
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
// GLOBAL: MW2GBL 0x100b5950
// GLOBAL: MW2MATROX 0x100bbd90
// GLOBAL: MW23DFX 0x1024c858
// GLOBAL: MW2ATI 0x10180bd8
// GLOBAL: MW2S3 0x10064160
// GLOBAL: MW2POWERVR 0x101abab0
ThrottleGauge g_throttleGauge = {{0x1e7a, 0x8dc9}, 0, 0, 0, 0, 0, 0};

// The heat bar: its position and size.
// GLOBAL: MW2 0x100a8350
// GLOBAL: MW2GBL 0x100b5970
// GLOBAL: MW2MATROX 0x100bbdb0
// GLOBAL: MW23DFX 0x1024c878
// GLOBAL: MW2ATI 0x10180bf8
// GLOBAL: MW2S3 0x10064180
// GLOBAL: MW2POWERVR 0x101abad0
Point g_heatBarPosition = {0, 0x3333};

// GLOBAL: MW2 0x100a8358
// GLOBAL: MW2GBL 0x100b5978
// GLOBAL: MW2MATROX 0x100bbdb8
// GLOBAL: MW23DFX 0x1024c880
// GLOBAL: MW2ATI 0x10180c00
// GLOBAL: MW2S3 0x10064188
// GLOBAL: MW2POWERVR 0x101abad8
Point g_heatBarSize = {0xea4e, 0x5555};

// The heat rate bar.
// GLOBAL: MW2 0x100a8360
// GLOBAL: MW2GBL 0x100b5980
// GLOBAL: MW2MATROX 0x100bbdc0
// GLOBAL: MW23DFX 0x1024c888
// GLOBAL: MW2ATI 0x10180c08
// GLOBAL: MW2S3 0x10064190
// GLOBAL: MW2POWERVR 0x101abae0
Point g_heatRateBarPosition = {0, 0x3333};

// GLOBAL: MW2 0x100a8368
// GLOBAL: MW2GBL 0x100b5988
// GLOBAL: MW2MATROX 0x100bbdc8
// GLOBAL: MW23DFX 0x1024c890
// GLOBAL: MW2ATI 0x10180c10
// GLOBAL: MW2S3 0x10064198
// GLOBAL: MW2POWERVR 0x101abae8
Point g_heatRateBarSize = {0xdf2e, 0x5555};

// The jump jet fuel bar.
// GLOBAL: MW2 0x100a8370
// GLOBAL: MW2GBL 0x100b5990
// GLOBAL: MW2MATROX 0x100bbdd0
// GLOBAL: MW23DFX 0x1024c898
// GLOBAL: MW2ATI 0x10180c18
// GLOBAL: MW2S3 0x100641a0
// GLOBAL: MW2POWERVR 0x101abaf0
Point g_jumpFuelBarPosition = {0, 0x3333};

// GLOBAL: MW2 0x100a8378
// GLOBAL: MW2GBL 0x100b5998
// GLOBAL: MW2MATROX 0x100bbdd8
// GLOBAL: MW23DFX 0x1024c8a0
// GLOBAL: MW2ATI 0x10180c20
// GLOBAL: MW2S3 0x100641a8
// GLOBAL: MW2POWERVR 0x101abaf8
Point g_jumpFuelBarSize = {0xdf2e, 0x5555};

// Scales the gauges' rectangles to their panels.
// FUNCTION: MW2 0x1004d020
// FUNCTION: MW2GBL 0x1006ad70
// FUNCTION: MW2MATROX 0x10089040
// FUNCTION: MW23DFX 0x1001c250
// FUNCTION: MW2ATI 0x1001c580
// FUNCTION: MW2S3 0x10014bc0
// FUNCTION: MW2POWERVR 0x1001b370
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
// MW2MATROX: the heat converts as the Matrox edition does (fld, __ftol).
// FUNCTION: MW2 0x1004d175
// FUNCTION: MW2GBL 0x1006aec5
// FUNCTION: MW2MATROX 0x10089195
// FUNCTION: MW23DFX 0x1001c350
// FUNCTION: MW2ATI 0x1001c680
// FUNCTION: MW2S3 0x10014cc0
// FUNCTION: MW2POWERVR 0x1001b470
void DrawHeatBar(PANE* p_target)
{
	MechS32 heat;
	MechS32 width;
	MechS32 edge;
	MechS32 middle;
	MechS32 level;
	MechS32 middleX;
	Mech* mech;
	MechS32 rightX;
	MechS32 x;
	MechS32 y;
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
#ifdef MW2_FLOAT_SCALARS
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
// FUNCTION: MW2 0x1004d310
// FUNCTION: MW2GBL 0x1006b060
// FUNCTION: MW2MATROX 0x10089332
// FUNCTION: MW23DFX 0x1001c4a0
// FUNCTION: MW2ATI 0x1001c7d0
// FUNCTION: MW2S3 0x10014e10
// FUNCTION: MW2POWERVR 0x1001b5c0
void DrawHeatRateBar(PANE* p_target)
{
	Mech* mech;
	MechS32 level;
	MechS32 fill;
	MechS32 fillColor;
	MechS32 backColor;

	mech = g_players[g_localPlayerId]->m_mech;
	fill = 0;
	level = 0;
#ifdef MW2_FLOAT_SCALARS
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
// FUNCTION: MW2 0x1004d48a
// FUNCTION: MW2GBL 0x1006b1da
// FUNCTION: MW2MATROX 0x100894b4
// FUNCTION: MW23DFX 0x1001c5b0
// FUNCTION: MW2ATI 0x1001c8e0
// FUNCTION: MW2S3 0x10014f20
// FUNCTION: MW2POWERVR 0x1001b6d0
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
#ifdef MW2_FLOAT_SCALARS
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
// FUNCTION: MW2 0x1004d660
// FUNCTION: MW2GBL 0x1006b3b0
// FUNCTION: MW2MATROX 0x100896bf
// FUNCTION: MW23DFX 0x1001c750
// FUNCTION: MW2ATI 0x1001ca80
// FUNCTION: MW2S3 0x100150c0
// FUNCTION: MW2POWERVR 0x1001b870
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
// FUNCTION: MW2 0x1004d732
// FUNCTION: MW2GBL 0x1006b482
// FUNCTION: MW2MATROX 0x10089791
// FUNCTION: MW23DFX 0x1001c810
// FUNCTION: MW2ATI 0x1001cb40
// FUNCTION: MW2S3 0x10015180
// FUNCTION: MW2POWERVR 0x1001b930
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

#endif

// Draws a bar p_width across from (p_x, p_y), shaded darker towards its edges.
// FUNCTION: MW2 0x1004d8ae
// FUNCTION: MW2GBL 0x1006b5fe
// FUNCTION: MW2PREMERCS 0x10048466
// FUNCTION: MW2MATROX 0x10089931
// FUNCTION: MW23DFX 0x1001c960
// FUNCTION: MW2ATI 0x1001cc90
// FUNCTION: MW2S3 0x100152d0
// FUNCTION: MW2POWERVR 0x1001ba80
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
