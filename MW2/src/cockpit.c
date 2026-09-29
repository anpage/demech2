#include "cockpit.h"

#include "cobaltharbor.h"
#include "decomp.h"
#include "environment.h"
#include "fixeddiv.h"
#include "loadres.h"
#include "palette.h"
#include "players.h"
#include "point.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "simmain.h"
#include "types.h"

#include <stdio.h>

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
// Stack-slot permutation: index, rect, extra and viewport.
// FUNCTION: MW2 0x1003dab0
void LoadCockpitLayout(MechS32 p_cockpit, CockpitLayout* p_layout)
{
	MechS32 index;
	RenderTarget* rect;
	CockpitLayoutExtra* extra;
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
	extra = p_layout->m_extra;
	if (extra) {
		rect = extra->m_rects->m_unk0x04;
		rect->m_buffer = &g_mainPixelBuffer;
		if (p_cockpit == 4) {
			FUN_10056c25(rect, g_eyepoint->m_pixelAspect);
			CenterRectOnScreen(&g_mainPixelBuffer, rect, rect);
		}
		else {
			FUN_1005699f(viewport, rect, rect);
		}

		rect = extra->m_rects->m_unk0x08;
		rect->m_buffer = &g_mainPixelBuffer;
		if (p_cockpit == 4) {
			FUN_10056c25(rect, g_eyepoint->m_pixelAspect);
			CenterRectOnScreen(&g_mainPixelBuffer, rect, rect);
		}
		else {
			FUN_1005699f(viewport, rect, rect);
		}

		rect = extra->m_rects->m_unk0x0c;
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
	if (g_unk0x100c3280->m_unk0x06) {
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

		FUN_10064f0b(
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

			FUN_10064f0b(
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

// STUB: MW2 0x1003f8d1
void FUN_1003f8d1(void)
{
	STUB(0x1003f8d1);
}
