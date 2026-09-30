#include "cockpit.h"

#include "cobaltharbor.h"
#include "decomp.h"
#include "environment.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "gamething.h"
#include "geocache.h"
#include "loadres.h"
#include "mappoint.h"
#include "navpoint.h"
#include "object.h"
#include "palette.h"
#include "players.h"
#include "point.h"
#include "recttransition.h"
#include "render.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "simmain.h"
#include "team.h"
#include "types.h"
#include "unk10041fa0.h"
#include "unk100696c0.h"

#include <stdio.h>

// The three values of the HUD layout (FUN_10070bda).
// GLOBAL: MW2 0x10109c30
MechS32 g_unk0x10109c30[3];

// GLOBAL: MW2 0x10109c5c
MechS32 g_unk0x10109c5c;

// GLOBAL: MW2 0x10109c60
MechS32 g_unk0x10109c60;

// GLOBAL: MW2 0x10109c64
MechS32 g_cockpitLayoutIndex;

// GLOBAL: MW2 0x10109c68
MechS32 g_unk0x10109c68;

// The view g_cockpitLayoutIndex switches to; 3 and 5 enter and leave the satellite view (4).
// GLOBAL: MW2 0x10109c6c
MechS32 g_unk0x10109c6c;

// Places the cockpit view p_cockpit on the screen: its viewport (kept in the render-target table,
// and for the normal cockpit also in g_unk0x100adf58), its points and extra rectangles, and its
// gauge functions.
// Stack-slot permutation: index, rect, transition and viewport.
// FUNCTION: MW2 0x1003dab0
void LoadCockpitLayout(MechS32 p_cockpit, CockpitLayout* p_layout)
{
	MechS32 index;
	RenderTarget* rect;
	RectTransition* transition;
	RenderTarget* viewport;

	if (p_layout == NULL) {
		return;
	}

	viewport = p_layout->m_viewport;
	g_cockpitLayoutIndex = p_cockpit;
	viewport->m_buffer = &g_mainPixelBuffer;
	ScaleRectToScreen(&g_mainPixelBuffer, viewport, viewport);
	if (p_cockpit == 1) {
		g_unk0x100adf58 = *viewport;
	}

	g_renderTargets[p_layout->m_renderTargetSlot] = *viewport;
	FUN_10056bc1(viewport, &p_layout->m_unk0x58, &p_layout->m_unk0x58);
	FUN_10056bc1(viewport, &p_layout->m_unk0x60, &p_layout->m_unk0x60);
	FUN_10056bc1(viewport, &p_layout->m_unk0x68, &p_layout->m_unk0x68);
	transition = p_layout->m_transition;
	if (transition) {
		rect = transition->m_def->m_first;
		rect->m_buffer = &g_mainPixelBuffer;
		if (p_cockpit == 4) {
			FUN_10056c25(rect, g_eyepoint->m_pixelAspect);
			CenterRectOnScreen(&g_mainPixelBuffer, rect, rect);
		}
		else {
			FUN_1005699f(viewport, rect, rect);
		}

		rect = transition->m_def->m_second;
		rect->m_buffer = &g_mainPixelBuffer;
		if (p_cockpit == 4) {
			FUN_10056c25(rect, g_eyepoint->m_pixelAspect);
			CenterRectOnScreen(&g_mainPixelBuffer, rect, rect);
		}
		else {
			FUN_1005699f(viewport, rect, rect);
		}

		rect = transition->m_def->m_out;
		rect->m_buffer = &g_mainPixelBuffer;
	}

	index = (MechS32) p_layout->m_gauges[0];
	p_layout->m_gauges[0] = g_cockpitGauges[index];
	index = (MechS32) p_layout->m_gauges[1];
	p_layout->m_gauges[1] = g_cockpitGauges[index];
	index = (MechS32) p_layout->m_gauges[2];
	p_layout->m_gauges[2] = g_cockpitGauges[index];
	index = (MechS32) p_layout->m_gauges[3];
	p_layout->m_gauges[3] = g_cockpitGauges[index];
	FUN_1003ef07(0);
}

// FUNCTION: MW2 0x1003dd82
void FUN_1003dd82(void)
{
	g_unk0x10109c60 = g_unk0x10109c68;
	g_unk0x10109c68 = 4;
	if (g_cockpitLayoutIndex <= 2) {
		FUN_1003e03c();
	}
	else if (g_cockpitLayoutIndex == 4 && g_unk0x10109c6c <= 2) {
		FUN_1003ddd7();
	}
}

// STUB: MW2 0x1003ddd7
MechS32 FUN_1003ddd7(void)
{
	STUB(0x1003ddd7);
	return 0;
}

// FUNCTION: MW2 0x1003e03c
void FUN_1003e03c(void)
{
	FUN_1003ddd7();
	if (g_unk0x100c3280[0]->m_unk0x06) {
		FUN_1003f8d1();
	}
	else {
		FUN_1003e06c();
	}
}

// STUB: MW2 0x1003e06c
void FUN_1003e06c(void)
{
	STUB(0x1003e06c);
}

// Draws the map view's contents: the nav points, the center icon and the other players and
// things, and the target.
// The only diff is a stack-slot permutation of the locals. The original has an unused local (unused).
// FUNCTION: MW2 0x1003e32c
void FUN_1003e32c(CockpitLayout* p_layout)
{
	MechS32 id;
	RenderTarget* viewport;
	MechS32 unused;
	MechS32 y;
	MechS32 x;
	Player* player;
	void* shape;

	viewport = p_layout->m_viewport;
	player = g_players[g_localPlayerId];
	FUN_1003e974(p_layout);
	if (g_cockpitLayoutIndex != 4) {
		x = (viewport->m_right - viewport->m_left + 1) >> 1;
		y = (viewport->m_bottom - viewport->m_top + 1) >> 1;
		id = p_layout->m_icons[0][0] + g_unk0x100e9614;
		shape = FUN_1001a19f(g_unk0x100a8740, id, g_unk0x100a8680, 0);
		if (shape) {
			BlitShpFrame(viewport, shape, 0, x, y);
			FUN_1001a163(id, g_unk0x100a8680);
			FUN_1003e4cd(p_layout);
		}
	}

	FUN_1003e689(p_layout);
}

// Draws icon p_icon at the world position p_pos, if the map view shows it.
// FUNCTION: MW2 0x1003e40a
void FUN_1003e40a(CockpitLayout* p_layout, MapPoint p_pos, MechS32 p_icon)
{
	RenderTarget* viewport;
	void* shape;
	MechS32 visible;

	viewport = p_layout->m_viewport;
	visible = FUN_1004251e(&p_pos);
	if (visible && p_layout->m_gauges[1]) {
		visible = p_layout->m_gauges[1](viewport, p_pos.m_xy);
	}

	if (visible) {
		shape = FUN_1001a19f(g_unk0x100a8740, p_icon + g_unk0x100e9614, g_unk0x100a8680, 0);
		if (shape) {
			BlitShpFrame(viewport, shape, 0, p_pos.m_xy.m_x, p_pos.m_xy.m_y);
			FUN_1001a163(p_icon + g_unk0x100e9614, g_unk0x100a8680);
		}
	}
}

// Draws the other live players and the live game things, by side.
// Stack-slot permutation; i < g_gameThingCount compares in the other operand order.
// FUNCTION: MW2 0x1003e4cd
void FUN_1003e4cd(CockpitLayout* p_layout)
{
	Player* player;
	MechS32 icon;
	GameThing* thing;
	MechS32 i;
	MapPoint pos;

	for (i = 0; i < g_playerCount; i++) {
		if (i != g_localPlayerId) {
			player = g_players[i];
			if ((player->m_flags & 0x1400) && !(player->m_flags & 0x800) && !(player->m_flags & 0x4000) &&
				!(player->m_flags & 0x16)) {
				pos.m_xy.m_x = player->m_position.m_x;
				pos.m_xy.m_y = player->m_position.m_y;
				pos.m_z = player->m_position.m_z;
				icon = p_layout->m_icons[1][GetPlayerSide(i)];
				FUN_1003e40a(p_layout, pos, icon);
			}
		}
	}

	for (i = 0; i < g_gameThingCount; i++) {
		thing = &g_gameThings[i];
		if ((thing->m_unk0x00 & 0x1400) && !(thing->m_unk0x00 & 0x1e)) {
			FUN_10020c6f(thing->m_unk0x04, &pos.m_xy.m_x, &pos.m_xy.m_y, &pos.m_z);
			icon = p_layout->m_icons[1][FUN_1003c30e(i)];
			FUN_1003e40a(p_layout, pos, icon);
		}
	}
}

// Returns whether team p_team has reached the nav point. The team test is an | where an & was
// meant: any reached nav counts.
// FUNCTION: MW2 0x1003e645
MechS32 FUN_1003e645(NavPoint* p_nav, MechS32 p_team)
{
	return (p_nav->m_flags & 0x20) && (p_nav->m_unk0x26 | (1 << p_team));
}

// Draws the local player's target: in the view, or clamped to its edge when outside it.
// Stack-slot permutation; m_icons[row][side] loads the table before the row (index order).
// FUNCTION: MW2 0x1003e689
void FUN_1003e689(CockpitLayout* p_layout)
{
	MechS32 index;
	Player* player;
	MechS32 visible;
	MechS32 icon;
	MechS32 side;
	MechS32 kind;
	RenderTarget* viewport;
	MapPoint pos;
	MechS32 row;
	MechU32 target;
	void* shape;

	viewport = p_layout->m_viewport;
	icon = -1;
	player = g_players[g_localPlayerId];
	pos.m_xy.m_x = player->m_targetInfo.m_position.m_x;
	pos.m_xy.m_y = player->m_targetInfo.m_position.m_y;
	pos.m_z = player->m_targetInfo.m_position.m_z;
	visible = FUN_1004251e(&pos);
	if (visible && p_layout->m_gauges[1]) {
		visible = p_layout->m_gauges[1](viewport, pos.m_xy);
	}

	target = player->m_targetInfo.m_target;
	if (!target || (target & 0x1000)) {
		return;
	}

	kind = target & 0xf00;
	index = target & 0xff;
	switch (kind) {
	case 0x200:
		side = GetPlayerSide(index);
		if ((g_players[index]->m_flags & 0x1400) && !(g_players[index]->m_flags & 0x800)) {
			row = 2;
		}
		else {
			row = 3;
		}
		break;
	case 0x400:
		side = FUN_1003c30e(index);
		if (g_gameThings[index].m_unk0x00 & 0x1400) {
			row = 2;
		}
		else {
			row = 3;
		}
		break;
	case 0x100:
		if (visible) {
			if (FUN_1003e645(&g_navTable[index], g_unk0x100a5918)) {
				side = 1;
			}
			else {
				side = 0;
			}
		}
		else {
			side = 0;
		}

		row = 5;
		break;
	default:
		return;
		break;
	}

	if (!visible) {
		row = 6;
	}

	icon = p_layout->m_icons[row][side];
	if (!icon) {
		return;
	}

	shape = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + icon, g_unk0x100a8680, 0);
	if (shape) {
		if (visible) {
			BlitShpFrame(viewport, shape, 0, pos.m_xy.m_x, pos.m_xy.m_y);
		}
		else if (p_layout->m_gauges[2]) {
			p_layout->m_gauges[2](viewport, &pos, &pos);
			BlitShpFrame(viewport, shape, 0, pos.m_xy.m_x, pos.m_xy.m_y);
		}

		FUN_1001a163(g_unk0x100e9614 + icon, g_unk0x100a8680);
	}
}

// Draws the local team's nav points, marking the ones reached.
// Stack-slot permutation; i < g_navCount compares in the other operand order.
// FUNCTION: MW2 0x1003e974
void FUN_1003e974(CockpitLayout* p_layout)
{
	MapPoint pos;
	MechS32 i;
	RenderTarget* viewport;
	MechS32 icon;
	MechS32 visible;
	NavPoint* nav;
	void* shape;

	viewport = p_layout->m_viewport;
	for (i = 0; i < g_navCount; i++) {
		nav = &g_navTable[i];
		if (nav->m_team == g_unk0x100a5918 && nav->m_unk0x00 && !(nav->m_flags & 1)) {
			if (nav->m_obj) {
				GetObjPosition(nav->m_obj, &pos.m_xy.m_x, &pos.m_xy.m_y, &pos.m_z);
			}
			else {
				pos.m_xy.m_x = nav->m_position[0];
				pos.m_xy.m_y = nav->m_position[1];
				pos.m_z = nav->m_position[2];
			}

			visible = FUN_1004251e(&pos);
			if (visible && p_layout->m_gauges[1]) {
				visible = p_layout->m_gauges[1](viewport, pos.m_xy);
			}

			if (visible) {
				if (FUN_1003e645(nav, g_unk0x100a5918)) {
					icon = p_layout->m_icons[4][1];
				}
				else {
					icon = p_layout->m_icons[4][0];
				}

				if (icon != -1) {
					shape = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + icon, g_unk0x100a8680, 0);
					if (shape) {
						BlitShpFrame(viewport, shape, 0, pos.m_xy.m_x, pos.m_xy.m_y);
						FUN_1001a163(g_unk0x100e9614 + icon, g_unk0x100a8680);
					}
				}
			}
		}
	}
}

// Draws the two lines of the view's field of view from the center, around heading p_heading.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1003eb22
void FUN_1003eb22(CockpitLayout* p_layout, MechS32 p_heading)
{
	MechS32 halfFov;
	RenderTarget* viewport;
	Point end;
	MechS32 y;
	MechS32 x;
	MechS32* colors;

	viewport = p_layout->m_viewport;
	colors = p_layout->m_colors;
	p_heading = (p_heading % 0x1680000 + 0x1680000) % 0x1680000;
	x = (viewport->m_right - viewport->m_left + 1) >> 1;
	y = (viewport->m_bottom - viewport->m_top + 1) >> 1;
	halfFov = FUN_100698de(0x10000, g_eyepoint->m_fovX);
	p_heading = 0x5a0000 - p_heading;
	if (p_layout->m_gauges[3]) {
		p_layout->m_gauges[3](viewport, p_heading - halfFov, &end);
		BlitLine(viewport, x, y, end.m_x, end.m_y, 0, colors[11]);
		p_layout->m_gauges[3](viewport, halfFov + p_heading, &end);
		BlitLine(viewport, x, y, end.m_x, end.m_y, 0, colors[11]);
	}
}

// Draws the view's range readout and, in the satellite view, the heading readout, formatting them
// again only when they change.
// Stack-slot permutation: every local.
// FUNCTION: MW2 0x1003ec35
void DrawMapViewText(CockpitLayout* p_layout)
{
	MechDouble value;
	MechChar* units;
	MechS32 range;
	void* font;
	RenderTarget* viewport;
	MechS32 heading;
	Player* player;
	MechDouble degrees;

	viewport = p_layout->m_viewport;
	player = g_players[g_localPlayerId];
	font = FUN_1001a19f(g_unk0x100a8740, p_layout->m_unk0x30 + g_unk0x100e9614, g_unk0x100a8684, 0);
	if (font) {
		if (p_layout->m_unk0x18 != p_layout->m_unk0x1c) {
			range = p_layout->m_unk0x18 / 2;
			if (range >= 100000) {
				range = FixedDiv16(range, 100000);
				units = p_layout->m_unk0x54;
			}
			else {
				range = FixedDiv16(range, 100);
				units = p_layout->m_unk0x50;
			}

			value = range / 65536.0;
			sprintf(p_layout->m_unk0x40, "%s%3.1lf%s", p_layout->m_unk0x3c, value, units);
			p_layout->m_unk0x1c = p_layout->m_unk0x18;
		}

		BlitString(
			viewport,
			p_layout->m_unk0x60.m_x,
			p_layout->m_unk0x60.m_y,
			font,
			p_layout->m_unk0x40,
			g_unk0x100e9350
		);
		if (g_cockpitLayoutIndex == 4) {
			heading = player->m_heading;
			heading = (heading % 0x1680000 + 0x1680000) % 0x1680000;
			if (p_layout->m_unk0x4c != heading) {
				degrees = heading / 65536.0;
				sprintf(p_layout->m_unk0x48, "%s%3.1lf", p_layout->m_unk0x44, degrees);
				p_layout->m_unk0x4c = heading;
			}

			BlitString(
				viewport,
				p_layout->m_unk0x68.m_x,
				p_layout->m_unk0x68.m_y,
				font,
				p_layout->m_unk0x48,
				g_unk0x100e9350
			);
		}

		FUN_1001a163(p_layout->m_unk0x30 + g_unk0x100e9614, g_unk0x100a8684);
	}
}

// Cycles the three cockpit views.
// FUNCTION: MW2 0x1003ee26
void FUN_1003ee26(void)
{
	if (g_cockpitLayoutIndex <= 2 && g_unk0x10109c6c <= 2) {
		g_unk0x10109c6c = g_cockpitLayoutIndex + 1;
		g_unk0x10109c6c %= 3;
	}
}

// FUNCTION: MW2 0x1003ee69
MechS32 FUN_1003ee69(void)
{
	return g_cockpitLayoutIndex == 4;
}

// FUNCTION: MW2 0x1003ee92
void FUN_1003ee92(void)
{
	if (g_cockpitLayoutIndex == 4) {
		FUN_1003eeaf();
	}
}

// Leaves the satellite view, or else enters it from a cockpit view.
// FUNCTION: MW2 0x1003eeaf
void FUN_1003eeaf(void)
{
	if (g_cockpitLayoutIndex == 4 && g_unk0x10109c6c == 4) {
		g_unk0x10109c6c = 5;
	}
	else if (g_cockpitLayoutIndex <= 2 && g_unk0x10109c6c <= 2) {
		g_unk0x10109c6c = 3;
	}
}

// STUB: MW2 0x1003ef07
void FUN_1003ef07(MechS32 p_unk0x00)
{
	STUB(0x1003ef07);
}

// The map view's shape filter (SlateHeron0x68::m_unk0x58): skips dead players' shapes and
// shapes of types 0x30 and 0x70, then culls through FUN_10042206.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1003f00d
MechS32 FUN_1003f00d(ScarletOrchid0x4c* p_shape)
{
	MechS32 type;
	MechS32 flags;
	MechS32 skip;
	MechS32 kind;

	skip = FALSE;
	kind = p_shape->m_unk0x02 & 0xf00;
	if (kind == 0x100) {
		flags = g_players[p_shape->m_unk0x14]->m_flags;
		if (flags & 0x16) {
			skip = TRUE;
		}
		else {
			skip = FALSE;
		}
	}
	else {
		type = p_shape->m_unk0x02 & 0xf0;
		switch (type) {
		case 0x30:
		case 0x70:
			skip = TRUE;
			break;
		default:
			break;
		}
	}

	if (!skip) {
		skip = FUN_10042206(p_shape);
	}

	return skip;
}

// Returns the shade (0-15) of a height p_unk0x04 in the map view.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1003f513
MechS32 FUN_1003f513(CockpitLayout* p_layout, MechS32 p_unk0x04)
{
	MechS32 shade;
	MechS32 height;
	MechS32 fraction;

	height = p_layout->m_unk0x18 - (p_unk0x04 >> 2);
	height -= g_unk0x100a5a30;
	fraction = FixedDiv16(height, g_unk0x100a5a38);
	shade = FixedMul16(fraction, 0x10);
	if (shade < 0) {
		shade = 0;
	}
	else if (shade > 15) {
		shade = 15;
	}

	return shade;
}

// Draws the layout's view into the rectangle p_transition has reached, restoring the viewport
// afterwards. Returns FALSE once the transition is over; then, with p_unk0x0c, it draws into the
// transition's final rectangle and has it stretched to the screen.
// FUNCTION: MW2 0x1003f594
MechS32 FUN_1003f594(MechS32 p_reverse, CockpitLayout* p_layout, RectTransition* p_transition, MechS32 p_unk0x0c)
{
	MechS32 result;
	RenderTarget* rect;
	RenderTarget* viewport;

	viewport = p_layout->m_viewport;
	result = TRUE;
	rect = UpdateRectTransition(p_reverse, p_transition);
	if (!rect) {
		result = FALSE;
		if (p_unk0x0c) {
			if (!p_reverse) {
				rect = p_transition->m_def->m_second;
			}
			else {
				rect = p_transition->m_def->m_first;
			}
		}
	}

	if (rect) {
		*p_layout->m_unk0x04 = *viewport;
		*viewport = *rect;
		FUN_1003e06c();
		*viewport = *p_layout->m_unk0x04;
		if (p_unk0x0c) {
			g_currentRenderTarget = *rect;
			g_unk0x10176ebc = 1;
		}
	}

	return result;
}

// STUB: MW2 0x1003f8d1
void FUN_1003f8d1(void)
{
	STUB(0x1003f8d1);
}

// FUNCTION: MW2 0x1003fad9
void FUN_1003fad9(void)
{
	g_unk0x10109c60 = 0;
	g_unk0x10109c68 = 0;
}
