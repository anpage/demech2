#include "damagepanel.h"

#include "bargauges.h"
#include "cockpit.h"
#include "cockpitpanel.h"
#include "config.h"
#include "decomp.h"
#include "environment.h"
#include "loadres.h"
#include "mech.h"
#include "mechdamage.h"
#include "mechsection.h"
#include "menu.h"
#include "muldiv.h"
#include "mw2prj.h"
#include "players.h"
#include "point.h"
#include "render.h"
#include "screenscale.h"
#include "setres.h"
#include "simmain.h"
#include "targeting.h"
#include "types.h"
#include "vfxa.h"

// The damage panel (panel 2): the mech's outline, its sections shaded by damage, and bars of each
// section's front and rear armor.

// The section each of the outline's sixteen parts shows, plus one (0: none).
// GLOBAL: MW2 0x100a5c78
MechS32 g_unk0x100a5c78[16] = {1, 3, 3, 2, 2, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 0};

// The places of the armor bars' four labels, in 16.16 fractions of the panel until FUN_10040020
// scales them.

// GLOBAL: MW2 0x100a5cb8
Point g_unk0x100a5cb8 = {0x1e1e, 0x199a};

// GLOBAL: MW2 0x100a5cc0
Point g_unk0x100a5cc0 = {0x5050, 0x199a};

// GLOBAL: MW2 0x100a5cc8
Point g_unk0x100a5cc8 = {0x9697, 0x199a};

// GLOBAL: MW2 0x100a5cd0
Point g_unk0x100a5cd0 = {0xd2d3, 0x199a};

// The largest armor value of the local mech's sections.
// GLOBAL: MW2 0x100a5cd8
MechS32 g_unk0x100a5cd8 = 0;

// The outline's rectangle, centered in the panel.
// GLOBAL: MW2 0x100a5ce0
PANE g_unk0x100a5ce0 = {NULL, 0, 0, 0, 0};

// The outline's sixteen parts: rectangles in the outline shape's pixels (FUN_10070bda reads them)
// until FUN_10040020 places them on the screen.
// GLOBAL: MW2 0x100a5cf8
PANE g_unk0x100a5cf8[16] = {0};

// Where each part's shape is drawn from, relative to its rectangle.
// GLOBAL: MW2 0x100a5e38
Point g_unk0x100a5e38[16] = {0};

// Frames the outline's parts (OutlinePane) when set.
// GLOBAL: MW2 0x100a5eb8
MechS32 g_unk0x100a5eb8 = 0;

// The armor bars' labels.

// GLOBAL: MW2 0x100a5ebc
MechChar g_unk0x100a5ebc[4] = "H";

// GLOBAL: MW2 0x100a5ec0
MechChar g_unk0x100a5ec0[4] = "T";

// GLOBAL: MW2 0x100a5ec4
MechChar g_unk0x100a5ec4[4] = "A";

// GLOBAL: MW2 0x100a5ec8
MechChar g_unk0x100a5ec8[4] = "L";

// The armor bars' places, one per section (m_x the bar's left, m_y its bottom).
// GLOBAL: MW2 0x100be418
Point g_unk0x100be418[8];

// Each section's full front and rear armor.
// GLOBAL: MW2 0x100be458
Point g_unk0x100be458[8];

// The remap table that colors an outline part (entry 6) by its damage.
// GLOBAL: MW2 0x100be498
MechU8 g_unk0x100be498[0x100];

// The armor bars' width and full height.
// GLOBAL: MW2 0x100be598
Point g_unk0x100be598;

// Sets up the damage panel: the identity remap table, the outline's rectangle and parts, the
// labels and armor bars' places, and the local mech's full armor.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10040020
void FUN_10040020(void)
{
	CockpitPanel* panel;
	MechS32 color;
	MechS32 height2;
	MechS32 width2;
	MechS32 height1;
	MechS32 width1;
	void* shape;
	Mech* mech;
	MechS32 i;
	Point max;
	Point min;
	MechSection* section;

	panel = g_unk0x100c3280[2];
	i = 0x100;
	while (i--) {
		g_unk0x100be498[i] = i;
	}

	shape = FUN_1001a19f(g_mw2PrjHandle, g_unk0x10109c30[0], g_resourceTypeTags[c_resTagShp], 0);
	if (shape) {
		height1 = VFX_shape_resolution(shape, 0);
		FUN_1001a163(g_unk0x10109c30[0], g_resourceTypeTags[c_resTagShp]);
		width1 = height1 >> 16;
		height1 &= 0xffff;
		shape = FUN_1001a19f(g_mw2PrjHandle, g_unk0x10109c30[0] + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp], 0);
	}

	if (shape) {
		height2 = VFX_shape_resolution(shape, 0);
		FUN_1001a163(g_unk0x10109c30[0] + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp]);
		width2 = height2 >> 16;
		height2 &= 0xffff;
		g_unk0x100a5ce0.m_window = &g_mainPixelBuffer;
		g_unk0x100a5ce0.m_x0 = (panel->m_width - width2) / 2 + panel->m_x;
		g_unk0x100a5ce0.m_y0 = (panel->m_height - height2) / 2 + panel->m_y;
		g_unk0x100a5ce0.m_x1 = g_unk0x100a5ce0.m_x0 + width2 - 1;
		g_unk0x100a5ce0.m_y1 = g_unk0x100a5ce0.m_y0 + height2 - 1;
		i = 16;
		while (i--) {
			g_unk0x100a5cf8[i].m_window = &g_mainPixelBuffer;
			min.m_x = (MechDouble) g_unk0x100a5cf8[i].m_x0 / width1 * 65536.0 + 0.5;
			min.m_y = (MechDouble) g_unk0x100a5cf8[i].m_y0 / height1 * 65536.0 + 0.5;
			max.m_x = (MechDouble) g_unk0x100a5cf8[i].m_x1 / width1 * 65536.0 + 0.5;
			max.m_y = (MechDouble) g_unk0x100a5cf8[i].m_y1 / height1 * 65536.0 + 0.5;
			max.m_x += min.m_x - 1;
			max.m_y += min.m_y - 1;
			ScalePointToFrame(&g_unk0x100a5ce0, &min, &min);
			ScalePointToFrame(&g_unk0x100a5ce0, &max, &max);
			g_unk0x100a5cf8[i].m_x0 = g_unk0x100a5ce0.m_x0 + min.m_x;
			g_unk0x100a5cf8[i].m_y0 = min.m_y + g_unk0x100a5ce0.m_y0;
			g_unk0x100a5cf8[i].m_x1 = g_unk0x100a5ce0.m_x0 + max.m_x;
			g_unk0x100a5cf8[i].m_y1 = max.m_y + g_unk0x100a5ce0.m_y0;
			g_unk0x100a5e38[i].m_x = -min.m_x;
			g_unk0x100a5e38[i].m_y = -min.m_y;
		}
	}

	g_unk0x100be418[0].m_x = 0x2323;
	g_unk0x100be418[1].m_x = 0x7373;
	g_unk0x100be418[2].m_x = 0x5a5a;
	g_unk0x100be418[3].m_x = 0x4141;
	g_unk0x100be418[4].m_x = 0xaaab;
	g_unk0x100be418[5].m_x = 0x9192;
	g_unk0x100be418[6].m_x = 0xe1e2;
	g_unk0x100be418[7].m_x = 0xc8c9;
	ScalePointToFrame(panel->m_target, &g_unk0x100a5cb8, &g_unk0x100a5cb8);
	ScalePointToFrame(panel->m_target, &g_unk0x100a5cc0, &g_unk0x100a5cc0);
	ScalePointToFrame(panel->m_target, &g_unk0x100a5cc8, &g_unk0x100a5cc8);
	ScalePointToFrame(panel->m_target, &g_unk0x100a5cd0, &g_unk0x100a5cd0);
	color = 0x5555;
	i = 8;
	while (i--) {
		g_unk0x100be418[i].m_y = color;
		ScalePointToFrame(panel->m_target, &g_unk0x100be418[i], &g_unk0x100be418[i]);
	}

	mech = g_players[g_localPlayerId]->m_mech;
	g_unk0x100be598.m_x = 0x1414;
	g_unk0x100be598.m_y = 0x8000;
	ScalePointToFrame(panel->m_target, &g_unk0x100be598, &g_unk0x100be598);
	for (i = 0; i < 8; i++) {
		section = &mech->m_sections[i];
		g_unk0x100be458[i].m_x = section->m_armor[0];
		g_unk0x100be458[i].m_y = section->m_armor[1];
		if (g_unk0x100be458[i].m_x > g_unk0x100a5cd8) {
			g_unk0x100a5cd8 = g_unk0x100be458[i].m_x;
		}

		if (g_unk0x100be458[i].m_x > g_unk0x100a5cd8) {
			g_unk0x100a5cd8 = g_unk0x100be458[i].m_x;
		}
	}
}

// Draws the damage panel's outline, each part shaded by its section's damage: yellow, then red
// as its armor goes, black once the section is destroyed.
// The original loads m_sections before scaling index, and the locals are a stack-slot permutation.
// FUNCTION: MW2 0x10040511
void FUN_10040511(Mech* p_mech, PANE* p_target)
{
	MechS32 rear;
	MechS32 color;
	MechS32 front;
	MechS32 remap;
	void* shape;
	MechS32 scale;
	MechS32 level;
	MechS32 index;
	MechS32 i;
	MechSection* section;

	rear = 0;
	front = 0;
	shape = FUN_1001a19f(g_mw2PrjHandle, g_unk0x10109c30[0] + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp], 0);
	if (!shape) {
		return;
	}

	VFX_shape_draw(&g_unk0x100a5ce0, shape, 0, 0, 0);
	for (i = 0; i < 16; i++) {
		rear = front = 0;
		if (g_unk0x100a5cf8[i].m_x1 - g_unk0x100a5cf8[i].m_x0 + 1 <= 0) {
			continue;
		}

		if (g_unk0x100a5eb8) {
			OutlinePane(&g_unk0x100a5cf8[i], 0xe);
		}

		color = 6;
		index = g_unk0x100a5c78[i] - 1;
		section = &p_mech->m_sections[index];
		scale = (section->m_unk0x26 & 0xf0U) >> 4;
		if (scale) {
			front = 15 - (section->m_unk0x08 + section->m_armor[1] / g_unk0x100a1598) * 3 / (scale << 16);
		}

		if (front < 1) {
			front = 0;
		}
		else if (front > 15) {
			front = 15;
		}

		scale = section->m_unk0x26 & 0xf;
		if (scale) {
			rear = 15 - (section->m_unk0x08 + section->m_armor[0] / g_unk0x100a1598) * 3 / (scale << 16);
		}

		if (rear < 1) {
			rear = 0;
		}
		else if (rear > 15) {
			rear = 15;
		}

		level = front > rear ? front : rear;
		if (section->m_unk0x26 & 0x2000) {
			color = 0;
		}
		else if (level > 11) {
			color = 0xb;
		}
		else if (level > 0) {
			color = 3;
		}

		if (color != 6) {
			remap = color;
			g_unk0x100be498[6] = remap;
			VFX_shape_lookaside(g_unk0x100be498);
			VFX_shape_translate_draw(&g_unk0x100a5cf8[i], shape, 0, g_unk0x100a5e38[i].m_x, g_unk0x100a5e38[i].m_y);
		}
	}

	FUN_1001a163(g_unk0x10109c30[0] + g_unk0x100e9614, g_resourceTypeTags[c_resTagShp]);
}

// Draws the damage panel's armor bars: each section's front armor, and the rear armor of the
// torso sections (1 to 3) beside it, full height for the section's full armor.
// The two full > armor tests take their operands in the other order, and the locals are a
// stack-slot permutation.
// FUNCTION: MW2 0x100407b6
void FUN_100407b6(Mech* p_mech, PANE* p_target)
{
	MechS32 full;
	MechS32 i;
	MechS32 color;
	void* font;
	MechS32 armor;
	MechS32 width;
	MechSection* section;

	g_unk0x100e9350[0xe] = 6;
	font = FUN_1001a19f(g_mw2PrjHandle, g_unk0x100e9614 + 1, g_resourceTypeTags[c_resTagFont], 0);
	if (font) {
		VFX_string_draw(p_target, g_unk0x100a5cb8.m_x, g_unk0x100a5cb8.m_y, font, g_unk0x100a5ebc, g_unk0x100e9350);
		VFX_string_draw(p_target, g_unk0x100a5cc0.m_x, g_unk0x100a5cc0.m_y, font, g_unk0x100a5ec0, g_unk0x100e9350);
		VFX_string_draw(p_target, g_unk0x100a5cc8.m_x, g_unk0x100a5cc8.m_y, font, g_unk0x100a5ec4, g_unk0x100e9350);
		VFX_string_draw(p_target, g_unk0x100a5cd0.m_x, g_unk0x100a5cc0.m_y, font, g_unk0x100a5ec8, g_unk0x100e9350);
		g_unk0x100e9350[0xe] = 0xe;
		FUN_1001a163(g_unk0x100e9614 + 1, g_resourceTypeTags[c_resTagFont]);
	}

	for (i = 0; i < 8; i++) {
		section = &p_mech->m_sections[i];
		width = g_unk0x100be598.m_x;
		if (i > 0 && i < 4) {
			width /= 2;
		}

		if (section->m_unk0x26 & 0x2000) {
			armor = 0;
			color = 0xf3;
		}
		else {
			armor = section->m_armor[0];
			if (g_unk0x100be458[i].m_x >> 2 >= armor) {
				color = 0xb;
			}
			else {
				color = 3;
			}

			armor = MulDiv64(armor, g_unk0x100be598.m_y, g_unk0x100a5cd8);
		}

		full = MulDiv64(g_unk0x100be458[i].m_x, g_unk0x100be598.m_y, g_unk0x100a5cd8);
		if (armor) {
			FUN_1004d732(p_target, g_unk0x100be418[i].m_x, g_unk0x100be418[i].m_y + armor, width, armor, 0xf);
		}

		if (full > armor) {
			FUN_1004d732(p_target, g_unk0x100be418[i].m_x, g_unk0x100be418[i].m_y + full, width, full - armor, color);
		}

		if (i > 0 && i < 4) {
			if (section->m_unk0x26 & 0x2000) {
				armor = 0;
				color = 0xf3;
			}
			else {
				armor = section->m_armor[1];
				if (g_unk0x100be458[i].m_y >> 2 >= armor) {
					color = 0xb;
				}
				else {
					color = 3;
				}

				armor = MulDiv64(armor, g_unk0x100be598.m_y, g_unk0x100a5cd8);
			}

			full = MulDiv64(g_unk0x100be458[i].m_y, g_unk0x100be598.m_y, g_unk0x100a5cd8);
			if (armor) {
				FUN_1004d732(
					p_target,
					g_unk0x100be418[i].m_x + width,
					g_unk0x100be418[i].m_y + armor,
					width,
					armor,
					0xf
				);
			}

			if (full > armor) {
				FUN_1004d732(
					p_target,
					g_unk0x100be418[i].m_x + width,
					g_unk0x100be418[i].m_y + full,
					width,
					full - armor,
					color
				);
			}
		}
	}
}
