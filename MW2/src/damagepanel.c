#include "damagepanel.h"

#include "bargauges.h"
#include "cockpit.h"
#include "cockpitpanel.h"
#include "config.h"
#include "decomp.h"
#include "environment.h"
#include "fixedfloat.h"
#include "loadres.h"
#include "mech.h"
#include "mechdamage.h"
#include "mechsection.h"
#include "menu.h"
#include "muldiv.h"
#include "mw2prj.h"
#include "palette.h"
#include "players.h"
#include "point.h"
#include "render.h"
#include "screenscale.h"
#include "setres.h"
#include "simmain.h"
#include "targeting.h"
#include "types.h"
#include "vfxa.h"

#ifdef MW2_VFX16
#include "accel/vfx16.h"
#endif
#ifdef MW2_KEY_COLOR
#include "matrox/colorkey.h"
#endif

// The damage panel (panel 2): the mech's outline, its sections shaded by damage, and bars of each
// section's front and rear armor.

// The section each of the outline's sixteen parts shows, plus one (0: none).
// GLOBAL: MW2 0x100a5c78
// GLOBAL: MW2GBL 0x100b2e58
// GLOBAL: MW2MATROX 0x100ac6a8
// GLOBAL: MW23DFX 0x1024c5c0
// GLOBAL: MW2ATI 0x10180940
MechS32 g_outlinePartSections[16] = {1, 3, 3, 2, 2, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 0};

// The Matrox edition keeps the full armor values as floats.
#ifdef MW2_FLOAT_SCALARS
typedef struct ArmorPoint {
	MechScalar m_x; // 0x00
	MechScalar m_y; // 0x04
} ArmorPoint;
#else
#define ArmorPoint Point
#endif

// The places of the armor bars' four labels, in 16.16 fractions of the panel until InitDamagePanel
// scales them.

// GLOBAL: MW2 0x100a5cb8
// GLOBAL: MW2GBL 0x100b2e98
// GLOBAL: MW2MATROX 0x100ac6e8
// GLOBAL: MW23DFX 0x1024c600
// GLOBAL: MW2ATI 0x10180980
// GLOBAL: MW2POWERVR 0x101ab828
Point g_htalLabelHPosition = {0x1e1e, 0x199a};

// GLOBAL: MW2 0x100a5cc0
// GLOBAL: MW2GBL 0x100b2ea0
// GLOBAL: MW2MATROX 0x100ac6f0
// GLOBAL: MW23DFX 0x1024c608
// GLOBAL: MW2ATI 0x10180988
// GLOBAL: MW2POWERVR 0x101ab830
Point g_htalLabelTPosition = {0x5050, 0x199a};

// GLOBAL: MW2 0x100a5cc8
// GLOBAL: MW2GBL 0x100b2ea8
// GLOBAL: MW2MATROX 0x100ac6f8
// GLOBAL: MW23DFX 0x1024c610
// GLOBAL: MW2ATI 0x10180990
// GLOBAL: MW2S3 0x10074040
// GLOBAL: MW2POWERVR 0x101ab838
Point g_htalLabelAPosition = {0x9697, 0x199a};

// GLOBAL: MW2 0x100a5cd0
// GLOBAL: MW2GBL 0x100b2eb0
// GLOBAL: MW2MATROX 0x100ac700
// GLOBAL: MW23DFX 0x1024c618
// GLOBAL: MW2ATI 0x10180998
// GLOBAL: MW2S3 0x10074048
// GLOBAL: MW2POWERVR 0x101ab840
Point g_htalLabelLPosition = {0xd2d3, 0x199a};

// The largest armor value of the local mech's sections.
// GLOBAL: MW2 0x100a5cd8
// GLOBAL: MW2GBL 0x100b2eb8
// GLOBAL: MW2MATROX 0x100ac708
// GLOBAL: MW23DFX 0x1024c620
// GLOBAL: MW2ATI 0x101809a0
// GLOBAL: MW2S3 0x10074050
// GLOBAL: MW2POWERVR 0x101ab848
MechScalar g_maxSectionArmor = 0;

// The outline's rectangle, centered in the panel.
// GLOBAL: MW2 0x100a5ce0
// GLOBAL: MW2GBL 0x100b2ec0
// GLOBAL: MW2MATROX 0x100ac710
// GLOBAL: MW23DFX 0x1024c628
// GLOBAL: MW2ATI 0x101809a8
// GLOBAL: MW2S3 0x10074058
// GLOBAL: MW2POWERVR 0x101ab850
PANE g_outlineRect = {NULL, 0, 0, 0, 0};

// The outline's sixteen parts: rectangles in the outline shape's pixels (LoadHudFile reads them)
// until InitDamagePanel places them on the screen.
// GLOBAL: MW2 0x100a5cf8
// GLOBAL: MW2GBL 0x100b2ed8
// GLOBAL: MW2MATROX 0x100ac728
// GLOBAL: MW23DFX 0x1024c640
// GLOBAL: MW2ATI 0x101809c0
// GLOBAL: MW2S3 0x10074070
// GLOBAL: MW2POWERVR 0x101ab868
PANE g_outlinePartRects[16] = {0};

// Where each part's shape is drawn from, relative to its rectangle.
// GLOBAL: MW2 0x100a5e38
// GLOBAL: MW2GBL 0x100b3018
// GLOBAL: MW2MATROX 0x100ac868
// GLOBAL: MW23DFX 0x1024c780
// GLOBAL: MW2ATI 0x10180b00
Point g_outlinePartOffsets[16] = {0};

// Frames the outline's parts (OutlinePane) when set.
// GLOBAL: MW2 0x100a5eb8
// GLOBAL: MW2GBL 0x100b3098
// GLOBAL: MW2PREMERCS 0x100b209c
// GLOBAL: MW2MATROX 0x100ac8e8
// GLOBAL: MW23DFX 0x1024c800
// GLOBAL: MW2ATI 0x10180b80
// GLOBAL: MW2S3 0x10074230
// GLOBAL: MW2POWERVR 0x101aba28
MechS32 g_frameOutlineParts = 0;

// The armor bars' labels.

// GLOBAL: MW2 0x100a5ebc
// GLOBAL: MW2GBL 0x100b309c
// GLOBAL: MW2MATROX 0x100ac8ec
// GLOBAL: MW23DFX 0x1024c808
// GLOBAL: MW2ATI 0x10180b88
// GLOBAL: MW2POWERVR 0x101aba30
MechChar g_htalLabelH[4] = "H";

// GLOBAL: MW2 0x100a5ec0
// GLOBAL: MW2GBL 0x100b30a0
// GLOBAL: MW2MATROX 0x100ac8f0
// GLOBAL: MW23DFX 0x1024c80c
// GLOBAL: MW2ATI 0x10180b8c
// GLOBAL: MW2POWERVR 0x101aba34
MechChar g_htalLabelT[4] = "T";

// GLOBAL: MW2 0x100a5ec4
// GLOBAL: MW2GBL 0x100b30a4
// GLOBAL: MW2MATROX 0x100ac8f4
// GLOBAL: MW23DFX 0x1024c810
// GLOBAL: MW2ATI 0x10180b90
// GLOBAL: MW2POWERVR 0x101aba38
MechChar g_htalLabelA[4] = "A";

// GLOBAL: MW2 0x100a5ec8
// GLOBAL: MW2GBL 0x100b30a8
// GLOBAL: MW2MATROX 0x100ac8f8
// GLOBAL: MW23DFX 0x1024c814
// GLOBAL: MW2ATI 0x10180b94
// GLOBAL: MW2POWERVR 0x101aba3c
MechChar g_htalLabelL[4] = "L";

// The armor bars' places, one per section (m_x the bar's left, m_y its bottom).
// GLOBAL: MW2 0x100be418
// GLOBAL: MW2GBL 0x100bb3c0
// GLOBAL: MW2MATROX 0x100c25f8
// GLOBAL: MW23DFX 0x1007c340
// GLOBAL: MW2ATI 0x10071340
// GLOBAL: MW2POWERVR 0x1006f340
Point g_armorBarPositions[8];

// Each section's full front and rear armor.
// GLOBAL: MW2 0x100be458
// GLOBAL: MW2GBL 0x100bb400
// GLOBAL: MW2MATROX 0x100c2640
// GLOBAL: MW23DFX 0x1007c388
// GLOBAL: MW2ATI 0x10071388
// GLOBAL: MW2POWERVR 0x1006f388
ArmorPoint g_fullSectionArmor[8];

// The remap table that colors an outline part (entry 6) by its damage.
// GLOBAL: MW2 0x100be498
// GLOBAL: MW2GBL 0x100bb440
MechU8 g_outlineRemap[0x100];

// The armor bars' width and full height.
// GLOBAL: MW2 0x100be598
// GLOBAL: MW2GBL 0x100bb540
// GLOBAL: MW2MATROX 0x100c2638
// GLOBAL: MW23DFX 0x1007c380
// GLOBAL: MW2ATI 0x10071380
// GLOBAL: MW2S3 0x1007db60
// GLOBAL: MW2POWERVR 0x1006f380
Point g_armorBarSize;

#ifdef MW2_MFD_PANELS
// The early Mercenaries build lays out and draws the damage outline for each of the MFDs' panes
// (DrawDamageOutline's p_slot: tables of eight outlines and of eight times sixteen parts).
// STUB: MW2PREMERCS 0x10046040
void InitDamagePanel(void)
{
	STUB(0x10046040);
}

// STUB: MW2PREMERCS 0x10046712
void DrawDamageOutline(Mech* p_mech, PANE* p_target, MechS32 p_slot)
{
	STUB(0x10046712);
}

// STUB: MW2PREMERCS 0x10046a01
void DrawArmorBars(Mech* p_mech, PANE* p_target)
{
	STUB(0x10046a01);
}
#else
// Sets up the damage panel: the identity remap table, the outline's rectangle and parts, the
// labels and armor bars' places, and the local mech's full armor.
// FUNCTION: MW2 0x10040020
// FUNCTION: MW2GBL 0x10053e30
// FUNCTION: MW2MATROX 0x10059940
// FUNCTION: MW23DFX 0x1001b230
// FUNCTION: MW2ATI 0x1001b5c0
// FUNCTION: MW2S3 0x100396f0
// FUNCTION: MW2POWERVR 0x1001a4e0
void InitDamagePanel(void)
{
	MechS32 color;
	Point size2;
	Point size1;
	Point min;
	void* shape;
	Mech* mech;
	MechS32 i;
	CockpitPanel* panel;
	Point max;
	MechSection* section;

	panel = g_cockpitPanels[c_panelMechView];
#ifndef MW2_VFX16
	i = 0x100;
	while (i--) {
		g_outlineRemap[i] = i;
	}
#endif

	shape = LoadCachedResource(g_mw2PrjHandle, g_hudLayoutValues[0], g_resTagShp, 0);
	if (shape) {
		size1.m_y = VFX_shape_resolution(shape, 0);
		UnlockCachedResource(g_hudLayoutValues[0], g_resTagShp);
		size1.m_x = size1.m_y >> 16;
		size1.m_y &= 0xffff;
		shape = LoadCachedResource(g_mw2PrjHandle, g_hudLayoutValues[0] + g_artResolution, g_resTagShp, 0);
	}

	if (shape) {
		size2.m_y = VFX_shape_resolution(shape, 0);
		UnlockCachedResource(g_hudLayoutValues[0] + g_artResolution, g_resTagShp);
		size2.m_x = size2.m_y >> 16;
		size2.m_y &= 0xffff;
		g_outlineRect.m_window = &g_mainPixelBuffer;
		g_outlineRect.m_x0 = (panel->m_width - size2.m_x) / 2 + panel->m_x;
		g_outlineRect.m_y0 = (panel->m_height - size2.m_y) / 2 + panel->m_y;
		g_outlineRect.m_x1 = g_outlineRect.m_x0 + size2.m_x - 1;
		g_outlineRect.m_y1 = size2.m_y + g_outlineRect.m_y0 - 1;
		i = 16;
		while (i--) {
			g_outlinePartRects[i].m_window = &g_mainPixelBuffer;
			min.m_x = (MechDouble) g_outlinePartRects[i].m_x0 / size1.m_x * 65536.0 + 0.5;
			min.m_y = (MechDouble) g_outlinePartRects[i].m_y0 / size1.m_y * 65536.0 + 0.5;
			max.m_x = (MechDouble) g_outlinePartRects[i].m_x1 / size1.m_x * 65536.0 + 0.5;
			max.m_y = (MechDouble) g_outlinePartRects[i].m_y1 / size1.m_y * 65536.0 + 0.5;
			max.m_x += min.m_x - 1;
			max.m_y += min.m_y - 1;
			ScalePointToFrame(&g_outlineRect, &min, &min);
			ScalePointToFrame(&g_outlineRect, &max, &max);
			g_outlinePartRects[i].m_x0 = g_outlineRect.m_x0 + min.m_x;
			g_outlinePartRects[i].m_y0 = min.m_y + g_outlineRect.m_y0;
			g_outlinePartRects[i].m_x1 = g_outlineRect.m_x0 + max.m_x;
			g_outlinePartRects[i].m_y1 = max.m_y + g_outlineRect.m_y0;
			g_outlinePartOffsets[i].m_x = -min.m_x;
			g_outlinePartOffsets[i].m_y = -min.m_y;
		}
	}

	g_armorBarPositions[0].m_x = 0x2323;
	g_armorBarPositions[1].m_x = 0x7373;
	g_armorBarPositions[2].m_x = 0x5a5a;
	g_armorBarPositions[3].m_x = 0x4141;
	g_armorBarPositions[4].m_x = 0xaaab;
	g_armorBarPositions[5].m_x = 0x9192;
	g_armorBarPositions[6].m_x = 0xe1e2;
	g_armorBarPositions[7].m_x = 0xc8c9;
	ScalePointToFrame(panel->m_target, &g_htalLabelHPosition, &g_htalLabelHPosition);
	ScalePointToFrame(panel->m_target, &g_htalLabelTPosition, &g_htalLabelTPosition);
	ScalePointToFrame(panel->m_target, &g_htalLabelAPosition, &g_htalLabelAPosition);
	ScalePointToFrame(panel->m_target, &g_htalLabelLPosition, &g_htalLabelLPosition);
	color = 0x5555;
	i = 8;
	while (i--) {
		g_armorBarPositions[i].m_y = color;
		ScalePointToFrame(panel->m_target, &g_armorBarPositions[i], &g_armorBarPositions[i]);
	}

	mech = g_players[g_localPlayerId]->m_mech;
	g_armorBarSize.m_x = 0x1414;
	g_armorBarSize.m_y = 0x8000;
	ScalePointToFrame(panel->m_target, &g_armorBarSize, &g_armorBarSize);
	for (i = 0; i < 8; i++) {
		section = &mech->m_sections[i];
		g_fullSectionArmor[i].m_x = section->m_armor[0];
		g_fullSectionArmor[i].m_y = section->m_armor[1];
		if (g_fullSectionArmor[i].m_x > g_maxSectionArmor) {
			g_maxSectionArmor = g_fullSectionArmor[i].m_x;
		}

		if (g_fullSectionArmor[i].m_x > g_maxSectionArmor) {
			g_maxSectionArmor = g_fullSectionArmor[i].m_x;
		}
	}
}

// The mask of the front armor's scale (the section flags' bits 4-7): the ATI, S3, PowerVR and 3Dfx
// editions' is signed, which their optimized code computes in a word.
#if defined(MW2_ATI) || defined(MW2_S3) || defined(MW2_POWERVR) || defined(MW2_3DFX)
#define SCALE_MASK 0xf0
#else
#define SCALE_MASK 0xf0U
#endif

// Draws the damage panel's outline, each part shaded by its section's damage: yellow, then red
// as its armor goes, black once the section is destroyed.
// FUNCTION: MW2 0x10040511
// FUNCTION: MW2GBL 0x10054321
// FUNCTION: MW2MATROX 0x10059e60
// FUNCTION: MW23DFX 0x1001b620
// FUNCTION: MW2ATI 0x1001b9b0
// FUNCTION: MW2S3 0x10039a80
// FUNCTION: MW2POWERVR 0x1001a8d0
void DrawDamageOutline(Mech* p_mech, PANE* p_target)
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
	shape = LoadCachedResource(g_mw2PrjHandle, g_hudLayoutValues[0] + g_artResolution, g_resTagShp, 0);
	if (!shape) {
		return;
	}

	VFX_shape_draw(&g_outlineRect, shape, 0, 0, 0);
	for (i = 0; i < 16; i++) {
		rear = front = 0;
		if (g_outlinePartRects[i].m_x1 - g_outlinePartRects[i].m_x0 + 1 <= 0) {
			continue;
		}

		if (g_frameOutlineParts) {
			OutlinePane(&g_outlinePartRects[i], 0xe);
		}

		color = 6;
		index = g_outlinePartSections[i] - 1;
		section = &p_mech->m_sections[index];
		scale = (section->m_flags & SCALE_MASK) >> 4;
		if (scale) {
#ifdef MW2_FLOAT_SCALARS
			front = 15 - (MechS32) (((section->m_internal + section->m_armor[1] / g_localArmorPerLevel) * 3) / scale);
#else
			front = 15 - (section->m_internal + section->m_armor[1] / g_localArmorPerLevel) * 3 / (scale << 16);
#endif
		}

		if (front < 1) {
			front = 0;
		}
		else if (front > 15) {
			front = 15;
		}

		scale = section->m_flags & 0xf;
		if (scale) {
#ifdef MW2_FLOAT_SCALARS
			rear = 15 - (MechS32) (((section->m_internal + section->m_armor[0] / g_localArmorPerLevel) * 3) / scale);
#else
			rear = 15 - (section->m_internal + section->m_armor[0] / g_localArmorPerLevel) * 3 / (scale << 16);
#endif
		}

		if (rear < 1) {
			rear = 0;
		}
		else if (rear > 15) {
			rear = 15;
		}

		level = front > rear ? front : rear;
		if (section->m_flags & 0x2000) {
			color = 0;
		}
		else if (level > 11) {
			color = 0xb;
		}
		else if (level > 0) {
			color = 3;
		}

		if (color != 6) {
#ifdef MW2_VFX16
			// The Matrox edition recolors the lookaside table's entry 6 for the part, then restores it.
			remap = VFX_lookaside_read16(6);
			VFX_lookaside_write16(6, VFX_lookaside_read16(color));
			VFX_shape_draw(&g_outlinePartRects[i], shape, 0, g_outlinePartOffsets[i].m_x, g_outlinePartOffsets[i].m_y);
			VFX_lookaside_write16(6, remap);
#else
			remap = color;
			g_outlineRemap[6] = remap;
			VFX_shape_lookaside(g_outlineRemap);
			VFX_shape_translate_draw(
				&g_outlinePartRects[i],
				shape,
				0,
				g_outlinePartOffsets[i].m_x,
				g_outlinePartOffsets[i].m_y
			);
#endif
		}
	}

	UnlockCachedResource(g_hudLayoutValues[0] + g_artResolution, g_resTagShp);
#ifdef MW2_KEY_COLOR
	FUN_10088280(&g_outlineRect);
#endif
}

// Draws the damage panel's armor bars: each section's front armor, and the rear armor of the
// torso sections (1 to 3) beside it, full height for the section's full armor.
// FUNCTION: MW2 0x100407b6
// FUNCTION: MW2GBL 0x100545c6
// FUNCTION: MW2MATROX 0x1005a18b
// FUNCTION: MW23DFX 0x1001b890
// FUNCTION: MW2ATI 0x1001bc20
// FUNCTION: MW2S3 0x10039c90
// FUNCTION: MW2POWERVR 0x1001ab40
void DrawArmorBars(Mech* p_mech, PANE* p_target)
{
	MechScalar full;
	MechS32 i;
	MechS32 color;
	void* font;
	MechScalar armor;
	MechS32 width;
	MechSection* section;

	g_textColors[0xe] = PIXEL_COLOR(6);
	font = LoadCachedResource(g_mw2PrjHandle, HUD_ART_RESOLUTION + 1, g_resTagFont, 0);
	if (font) {
		VFX_string_draw(p_target, g_htalLabelHPosition.m_x, g_htalLabelHPosition.m_y, font, g_htalLabelH, g_textColors);
		VFX_string_draw(p_target, g_htalLabelTPosition.m_x, g_htalLabelTPosition.m_y, font, g_htalLabelT, g_textColors);
		VFX_string_draw(p_target, g_htalLabelAPosition.m_x, g_htalLabelAPosition.m_y, font, g_htalLabelA, g_textColors);
		VFX_string_draw(p_target, g_htalLabelLPosition.m_x, g_htalLabelTPosition.m_y, font, g_htalLabelL, g_textColors);
		g_textColors[0xe] = PIXEL_COLOR(0xe);
		UnlockCachedResource(HUD_ART_RESOLUTION + 1, g_resTagFont);
	}

	for (i = 0; i < 8; i++) {
		section = &p_mech->m_sections[i];
		width = g_armorBarSize.m_x;
		if (i > 0 && i < 4) {
			width /= 2;
		}

		if (section->m_flags & 0x2000) {
			armor = 0;
			color = 0xf3;
		}
		else {
			armor = section->m_armor[0];
			if (FIXED_SHR(g_fullSectionArmor[i].m_x, 2) >= armor) {
				color = 0xb;
			}
			else {
				color = 3;
			}

#ifdef MW2_FLOAT_SCALARS
			armor = g_armorBarSize.m_y * armor / g_maxSectionArmor;
#else
			armor = MulDiv64(armor, g_armorBarSize.m_y, g_maxSectionArmor);
#endif
		}

#ifdef MW2_FLOAT_SCALARS
		full = g_armorBarSize.m_y * g_fullSectionArmor[i].m_x / g_maxSectionArmor;
#else
		full = MulDiv64(g_fullSectionArmor[i].m_x, g_armorBarSize.m_y, g_maxSectionArmor);
#endif
		if (FIXED_IS_NONZERO(armor)) {
			DrawVerticalBar(
				p_target,
				g_armorBarPositions[i].m_x,
				g_armorBarPositions[i].m_y + SCALAR_TO_INT(armor),
				width,
				SCALAR_TO_INT(armor),
				0xf
			);
		}

		if (full > armor) {
			DrawVerticalBar(
				p_target,
				g_armorBarPositions[i].m_x,
				g_armorBarPositions[i].m_y + SCALAR_TO_INT(full),
				width,
				SCALAR_TO_INT(full - armor),
				color
			);
		}

		if (i > 0 && i < 4) {
			if (section->m_flags & 0x2000) {
				armor = 0;
				color = 0xf3;
			}
			else {
				armor = section->m_armor[1];
				if (FIXED_SHR(g_fullSectionArmor[i].m_y, 2) >= armor) {
					color = 0xb;
				}
				else {
					color = 3;
				}

#ifdef MW2_FLOAT_SCALARS
				armor = g_armorBarSize.m_y * armor / g_maxSectionArmor;
#else
				armor = MulDiv64(armor, g_armorBarSize.m_y, g_maxSectionArmor);
#endif
			}

#ifdef MW2_FLOAT_SCALARS
			full = g_armorBarSize.m_y * g_fullSectionArmor[i].m_y / g_maxSectionArmor;
#else
			full = MulDiv64(g_fullSectionArmor[i].m_y, g_armorBarSize.m_y, g_maxSectionArmor);
#endif
			if (armor) {
				DrawVerticalBar(
					p_target,
					g_armorBarPositions[i].m_x + width,
					g_armorBarPositions[i].m_y + SCALAR_TO_INT(armor),
					width,
					SCALAR_TO_INT(armor),
					0xf
				);
			}

			if (full > armor) {
				DrawVerticalBar(
					p_target,
					g_armorBarPositions[i].m_x + width,
					g_armorBarPositions[i].m_y + SCALAR_TO_INT(full),
					width,
					SCALAR_TO_INT(full - armor),
					color
				);
			}
		}
	}
}
#endif
