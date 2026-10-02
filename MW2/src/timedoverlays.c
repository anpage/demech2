#include "timedoverlays.h"

#include "clock.h"
#include "decomp.h"
#include "loadres.h"
#include "menu.h"
#include "mw2prj.h"
#include "point.h"
#include "render.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "setres.h"
#include "timedoverlay.h"
#include "types.h"

#include <string.h>

DECOMP_SIZE_ASSERT(TimedOverlay, 0x24)

// GLOBAL: MW2 0x100adee0
Pane g_unk0x100adee0 = {&g_mainPixelBuffer, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100adef4
undefined4 g_unk0x100adef4 = 0;

// GLOBAL: MW2 0x100adef8
Pane g_unk0x100adef8 = {&g_mainPixelBuffer, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100adf0c
undefined4 g_unk0x100adf0c = 0;

// GLOBAL: MW2 0x100c3360
static MechChar g_unk0x100c3360[0x100];

// GLOBAL: MW2 0x100c3460
static MechChar g_unk0x100c3460[0x100];

// GLOBAL: MW2 0x100adf10
TimedOverlay g_timedOverlays[2] = {
	{g_unk0x100c3460, {0x28f, 0}, 0, 0, 1, 0x4c, 0, &g_unk0x100adee0},
	{g_unk0x100c3360, {0x28f, 0}, 0, 0, 1, 0x4c, 0, &g_unk0x100adef8}
};

// Lays out the message boxes: scales their rectangles to the screen and to their background
// shapes, moves them to the top (the first) and bottom (the second) edge, and centers the
// text vertically.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x1006ee60
void FUN_1006ee60(void)
{
	MechS32 fontHeight;
	MechS32 sign;
	MechS32 dy;
	MechS32 dx;
	Pane* target;
	void* font;
	MechS32 i;
	TimedOverlay* overlay;
	void* shape;
	MechS32 height;

	for (i = 0; i < 2; i++) {
		overlay = &g_timedOverlays[i];
		target = overlay->m_target;
		ScaleRectToScreen(&g_mainPixelBuffer, target, target);
		shape =
			FUN_1001a19f(g_mw2PrjHandle, overlay->m_background + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp], 0);
		if (shape != NULL) {
			FUN_10056fcf(target, target, shape, 0);
		}

		if (i == 0) {
			sign = -1;
		}
		else {
			sign = 1;
		}

		dx = -target->m_left;
		dy = target->m_top * sign;
		target->m_left += dx;
		target->m_top += dy;
		target->m_right += dx;
		target->m_bottom += dy;
		FUN_10056bc1(target, &overlay->m_textPos, &overlay->m_textPos);

		font = FUN_1001a19f(g_mw2PrjHandle, overlay->m_font + g_unk0x100e9614, g_resourceTypeTags[c_resTagFont], 0);
		if (font != NULL) {
			height = target->m_bottom - target->m_top + 1;
			fontHeight = FontGetHeight(font);
			overlay->m_textPos.m_y = (height - fontHeight) / 2;
		}
	}
}

// Shows p_text in a free message box for p_duration ticks, or replaces the one of the lowest
// priority not above p_priority (of those, the one that expires last). Returns 1 if shown.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x1006efd4
MechS32 ShowInGameMessage(MechChar* p_text, MechS32 p_font, MechS32 p_duration, MechS32 p_priority)
{
	MechS32 result;
	MechS32 slot;
	MechS32 latest;
	MechS32 i;
	MechS32 done;
	MechS32 lowestPriority;
	TimedOverlay* overlay;

	done = FALSE;
	result = 0;
	slot = -1;
	lowestPriority = 0x29a;
	i = 0;
	while (!done) {
		if (!g_timedOverlays[i].m_active) {
			slot = i;
			done = TRUE;
		}
		else {
			if (g_timedOverlays[i].m_priority <= p_priority) {
				if (g_timedOverlays[i].m_priority < lowestPriority) {
					slot = i;
					lowestPriority = g_timedOverlays[i].m_priority;
					latest = g_timedOverlays[i].m_expireTime;
				}
				else if (g_timedOverlays[i].m_priority == lowestPriority && g_timedOverlays[i].m_expireTime > latest) {
					slot = i;
					latest = g_timedOverlays[i].m_expireTime;
				}
			}

			i++;
			if (i == 2) {
				done = TRUE;
			}
		}
	}

	if (slot != -1) {
		result = 1;
		overlay = &g_timedOverlays[slot];
		strncpy(overlay->m_text, p_text, 0xff);
		overlay->m_text[0xff] = '\0';
		overlay->m_active = 1;
		overlay->m_priority = p_priority;
		overlay->m_expireTime = p_duration + FUN_1007d05d();
		if (p_font < 1) {
			overlay->m_font = 1;
		}
		else {
			overlay->m_font = p_font;
		}
	}

	return result;
}

// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x1006f179
void DrawTimedOverlays(void)
{
	MechS32 i;
	TimedOverlay* overlay;
	void* font;
	void* background;

	for (i = 0; i < 2; i++) {
		overlay = &g_timedOverlays[i];
		if (overlay->m_active) {
			if (FUN_1007d05d() < overlay->m_expireTime) {
				font = FUN_1001a19f(
					g_mw2PrjHandle,
					overlay->m_font + g_unk0x100e9614,
					g_resourceTypeTags[c_resTagFont],
					0
				);
				if (font != NULL) {
					background = FUN_1001a19f(
						g_mw2PrjHandle,
						overlay->m_background + g_unk0x100e9614,
						g_resourceTypeTags[c_resTagShp],
						0
					);
					if (background != NULL) {
						BlitShpFrame(overlay->m_target, background, 0, 0, 0);
						BlitString(
							overlay->m_target,
							overlay->m_textPos.m_x,
							overlay->m_textPos.m_y,
							font,
							overlay->m_text,
							g_unk0x100e9350
						);
					}
				}
			}
			else {
				g_timedOverlays[i].m_active = 0;
			}
		}
	}
}

// Draws text in a box sized to it, on a tiled background shape (p_background, -1 for none),
// at p_x, p_y or centered on the screen along an axis where that is negative.
// Stack-slot permutation: the locals.
// FUNCTION: MW2 0x1006f28f
void FUN_1006f28f(MechS32 p_background, MechS32 p_font, MechChar* p_text, MechS32 p_x, MechS32 p_y)
{
	Pane centered;
	void* background;
	Pane rect;
	void* font;

	background = NULL;
	if (p_text == NULL) {
		return;
	}

	if (p_background != -1) {
		background = FUN_1001a19f(g_mw2PrjHandle, p_background + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp], 0);
	}

	font = FUN_1001a19f(g_mw2PrjHandle, p_font + g_unk0x100e9614, g_resourceTypeTags[c_resTagFont], 0);
	if (font != NULL) {
		rect.m_buffer = &g_mainPixelBuffer;
		FUN_100575b9(p_text, font, &rect);
		if (p_x >= 0) {
			rect.m_left += p_x;
			rect.m_right += p_x;
		}
		else {
			centered = rect;
			CenterRectOnScreen(&g_mainPixelBuffer, &centered, &centered);
			rect.m_left = centered.m_left;
			rect.m_right = centered.m_right;
		}

		if (p_y >= 0) {
			rect.m_top += p_y;
			rect.m_bottom += p_y;
		}
		else {
			centered = rect;
			CenterRectOnScreen(&g_mainPixelBuffer, &centered, &centered);
			rect.m_top = centered.m_top;
			rect.m_bottom = centered.m_bottom;
		}

		if (background != NULL) {
			FUN_10057896(&rect, background, 0);
		}

		FUN_10057396(&rect, p_text, font);
	}
}

// Draws a TEXT resource, stored with each character negated (mod 256), as FUN_1006f28f does.
// FUNCTION: MW2 0x1006f3ea
void FUN_1006f3ea(MechS32 p_background, MechS32 p_font, MechS32 p_id, MechS32 p_x, MechS32 p_y)
{
	MechChar* text;
	MechChar* c;
	MechS32 ch;

	text = FUN_1001a19f(g_mw2PrjHandle, p_id, g_resourceTypeTags[c_resTagText], 0);
	if (text != NULL) {
		c = text;
		while (*c != '\0') {
			ch = *c;
			ch = 0x100 - ch;
			*c = ch;
			c++;
		}

		FUN_1006f28f(p_background, p_font, text, p_x, p_y);
	}
}
