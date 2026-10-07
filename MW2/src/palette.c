#include "palette.h"

#include "clock.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "loadres.h"
#include "mw2prj.h"
#include "palcycle.h"
#include "palfade.h"
#include "palidentity.h"
#include "polydraw.h"
#include "refreshmode.h"
#include "render.h"
#include "simmain.h"
#include "targeting.h"
#include "ticks.h"
#include "types.h"

#include <string.h>
#include <windows.h>

// GLOBAL: MW2 0x100a00cc
// GLOBAL: MW2MATROX 0x100a61f8
MechS32 g_paneIndex = -1;

// GLOBAL: MW2 0x100a00d0
// GLOBAL: MW2MATROX 0x100a61fc
MechS32 g_currentPalette = 0x10;

// GLOBAL: MW2 0x100a00d4
// GLOBAL: MW2MATROX 0x100a6200
MechS32 g_palettePending = 0;

// GLOBAL: MW2 0x100a00d8
// GLOBAL: MW2MATROX 0x100a6204
MechS32 g_basePalette = 0;

// GLOBAL: MW2 0x100a00dc
// GLOBAL: MW2MATROX 0x100a6208
MechS32 g_settledPalette = 0x10;

// GLOBAL: MW2 0x100a00e0
// GLOBAL: MW2MATROX 0x100a620c
MechS32 g_paletteFadeTarget = -1;

// GLOBAL: MW2 0x100a00e4
// GLOBAL: MW2MATROX 0x100a6210
MechS32 g_paletteFadeBack = -1;

// GLOBAL: MW2 0x100a00e8
// GLOBAL: MW2MATROX 0x100a6214
MechS32 g_paletteFadeBackSteps = 0;

// GLOBAL: MW2 0x100a00ec
// GLOBAL: MW2MATROX 0x100a6218
MechS32 g_paletteFadeSteps = 0;

// GLOBAL: MW2 0x100a00f0
// GLOBAL: MW2MATROX 0x100a621c
MechS32 g_paletteCycling = 0;

// GLOBAL: MW2 0x100a00f4
// GLOBAL: MW2MATROX 0x100a6220
MechS32 g_paletteCycleResource = -1;

#ifdef MW2_MATROX
// The Matrox edition's tint for the palettes 0xc, 0x10 and 0x11 (StartPaletteFade): whether it is on, and
// its color.
// GLOBAL: MW2MATROX 0x100aa1d0
MechS32 g_unk0x100aa1d0 = 0;

// GLOBAL: MW2MATROX 0x10184700
MechFloat g_unk0x10184700[3];
#endif

// GLOBAL: MW2 0x100bcd20
// GLOBAL: MW2MATROX 0x100c1ff8
static PaletteFade g_paletteFade;

// GLOBAL: MW2 0x100bcd40
// GLOBAL: MW2MATROX 0x100c2018
static PaletteCycle g_paletteCycle;

// GLOBAL: MW2 0x10181a60
// GLOBAL: MW2MATROX 0x101d4af0
PANE g_panes[11];

// GLOBAL: MW2 0x10181b40
// GLOBAL: MW2MATROX 0x101d4aa0
MechS32 g_paletteResourceIds[20];

// FUNCTION: MW2 0x100023c0
// FUNCTION: MW2MATROX 0x1002d560
void InitPanes(PANE* p_target)
{
	MechS32 i;

	for (i = 0; i < 11; i++) {
		g_panes[i] = *p_target;
	}

	for (i = 0; i < 20; i++) {
		g_paletteResourceIds[i] = 0;
	}
}

// FUNCTION: MW2 0x1000242f
// FUNCTION: MW2MATROX 0x1002d5cf
void SelectPane(MechS32 p_index)
{
	PANE* target;

	if (p_index < 0 || p_index >= 11) {
		return;
	}

	if (p_index != g_paneIndex) {
		target = &g_panes[p_index];
		g_eyepoint->m_viewLeft = 0;
		g_eyepoint->m_viewTop = 0;
		g_eyepoint->m_viewRight = target->m_x1 - target->m_x0;
		g_eyepoint->m_viewBottom = target->m_y1 - target->m_y0;
#ifndef MW2_MATROX
		g_eyepoint->m_offsetX = 0;
		g_eyepoint->m_offsetY = 0;
#endif
		g_currentPane = *target;
		g_paneIndex = p_index;
		g_projectionDirty = 1;
	}
}

// The Matrox edition adds m_viewLeft and m_viewRight in the other operand order.
// FUNCTION: MW2 0x100024f0
// FUNCTION: MW2MATROX 0x1002d678
void GetViewCenter(Eyepoint* p_eyepoint, MechS32* p_x, MechS32* p_y)
{
	MechS32 x;
	MechS32 y;

#ifdef MW2_MATROX
	x = (p_eyepoint->m_viewLeft + p_eyepoint->m_viewRight) / 2;
	y = (p_eyepoint->m_viewTop + p_eyepoint->m_viewBottom) / 2;
#else
	x = p_eyepoint->m_offsetX + (p_eyepoint->m_viewLeft + p_eyepoint->m_viewRight) / 2;
	y = p_eyepoint->m_offsetY + (p_eyepoint->m_viewTop + p_eyepoint->m_viewBottom) / 2;
#endif
	*p_x = x;
	*p_y = y;
}

// FUNCTION: MW2 0x10002546
// FUNCTION: MW2MATROX 0x1002d6be
void ApplyPendingPalette(void)
{
	if (g_palettePending && g_paletteFadeSteps <= 0) {
		g_palettePending = 0;
		ApplyPaletteResource(g_settledPalette);
		g_currentPalette = g_settledPalette;
	}
}

// The Matrox edition ors the pixel's green and blue parts in the other order.
// FUNCTION: MW2 0x1000258d
// FUNCTION: MW2MATROX 0x1002d705
void ApplyPaletteResource(MechS32 p_slot)
{
	MechU8* palette;
	MechS32* id;
#ifdef MW2_MATROX
	MechS32 i;
	PaletteColor color;
#endif

	id = &g_paletteResourceIds[p_slot];
	if (*id > 0) {
		palette = LoadCachedResource(g_mw2PrjHandle, *id, g_resourceTypeTags[c_resTagPal], 0);
		if (palette) {
#ifdef MW2_MATROX
			// The Matrox edition's 16-bit pixels: RGB565 from the 6-bit components.
			for (i = 0; i < 0x100; i++) {
				color = ((PaletteColor*) palette)[i];
				FUN_1005708c(i, (color.m_red & ~1) << 10 | color.m_green << 5 | color.m_blue >> 1);
			}

			ResetTextColors();
			FUN_1005f790((PaletteColor*) palette);
#else
			g_currentDisplayBackend->m_setPaletteWithBrightness((PaletteColor*) palette);
#endif
			UnlockCachedResource(*id, g_resourceTypeTags[c_resTagPal]);
		}
	}
}

// FUNCTION: MW2 0x10002600
// FUNCTION: MW2MATROX 0x1002d7de
void UpdatePaletteFade(void)
{
	if (g_paletteFadeSteps) {
		StepPaletteFade(&g_paletteFade);
		if (--g_paletteFadeSteps == 0) {
			g_currentPalette = g_paletteFadeTarget;
			if (g_paletteFadeBackSteps) {
				StartPaletteFade(g_paletteFadeBack, g_paletteFadeBackSteps, 0);
			}
			else {
				g_palettePending = 1;
			}
		}
	}

	if (g_paletteCycling) {
		RotatePaletteCycle(&g_paletteCycle);
	}
}

// Matches except for the stack slots of fromSlot, steps and to (a consistent permutation).
// FUNCTION: MW2 0x10002687
// FUNCTION: MW2MATROX 0x1002d865
MechS32 StartPaletteFade(MechS32 p_palette, MechS32 p_duration, MechS32 p_mode)
{
	MechU8* to;
	MechU8* from;
	MechS32 steps;
	MechS32 fromSlot;
	MechS32 result;

	from = NULL;
	fromSlot = -1;
	result = 0;
	if (g_paletteFadeSteps > 0 && p_mode == 0) {
		from = g_paletteFade.m_palette;
	}
	else if (g_paletteFadeSteps <= 0) {
		fromSlot = g_currentPalette;
		from = LoadCachedResource(g_mw2PrjHandle, g_paletteResourceIds[fromSlot], g_resourceTypeTags[c_resTagPal], 0);
	}

	if (from) {
		to = LoadCachedResource(g_mw2PrjHandle, g_paletteResourceIds[p_palette], g_resourceTypeTags[c_resTagPal], 0);
		if (to) {
			g_paletteFadeTarget = p_palette;
			g_paletteFadeBack = fromSlot;
			if (g_deltaTime) {
				if (p_mode == 1) {
					p_duration >>= 1;
				}

#ifdef MW2_MATROX
				if (g_deltaTime > 0) {
					steps = (MechFloat) p_duration / g_deltaTime + 0.5f;
				}
				else {
					steps = 10;
				}

				if (steps == 0) {
					steps = 1;
				}
#else
				if (g_deltaTime > 0) {
					steps = FixedDiv16(p_duration, g_deltaTime);
				}
				else {
					steps = 10;
				}

				if (steps >= 0x10000) {
					steps >>= 16;
				}
				else {
					steps = 1;
				}
#endif
			}
			else {
				steps = 20;
			}

			if (p_mode == 2) {
				g_paletteFadeSteps = 1;
			}
			else {
				g_paletteFadeSteps = steps;
			}

#ifdef MW2_MATROX
			if (p_mode == 1 || p_mode == 2) {
				g_paletteFadeBackSteps = p_duration;
			}
			else {
				g_paletteFadeBackSteps = 0;
			}

			switch (p_palette) {
			case 0xc:
				g_unk0x100aa1d0 = 1;
				g_unk0x10184700[0] = 0.0f;
				g_unk0x10184700[1] = 255.0f;
				g_unk0x10184700[2] = 8.0f;
				break;
			case 0x11:
				g_unk0x100aa1d0 = 1;
				g_unk0x10184700[0] = 255.0f;
				g_unk0x10184700[1] = 8.0f;
				g_unk0x10184700[2] = 8.0f;
				break;
			case 0x10:
				g_unk0x100aa1d0 = 1;
				g_unk0x10184700[0] = 0.0f;
				g_unk0x10184700[1] = 0.0f;
				g_unk0x10184700[2] = 0.0f;
				break;
			default:
				g_unk0x100aa1d0 = 0;
				break;
			}
#else
			if (p_mode == 1 || p_mode == 2) {
				g_paletteFadeBackSteps = steps;
			}
			else {
				g_paletteFadeBackSteps = 0;
			}
#endif

			InitPaletteFade(&g_paletteFade, from, to, 0, 0x100, steps);
			result = 1;
			UnlockCachedResource(g_paletteResourceIds[p_palette], g_resourceTypeTags[c_resTagPal]);
		}

		if (fromSlot == -1) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, from);
		}
		else {
			UnlockCachedResource(g_paletteResourceIds[fromSlot], g_resourceTypeTags[c_resTagPal]);
		}
	}

	return result;
}

// FUNCTION: MW2 0x1000288e
// FUNCTION: MW2MATROX 0x1002db4c
MechS32 StartPaletteFlash(MechS32 p_offset, MechS32 p_duration, MechS32 p_mode)
{
	return StartPaletteFade(p_offset + g_basePalette, p_duration, p_mode);
}

// FUNCTION: MW2 0x100028b7
// FUNCTION: MW2MATROX 0x1002db75
void StartPaletteCycle(MechU8 p_first, MechS32 p_count)
{
	MechU8* palette;

	if (g_paletteFadeSteps > 0 || g_paletteCycling) {
		return;
	}

	g_paletteCycling = 1;
	g_paletteCycleResource = g_paletteResourceIds[g_currentPalette];
	palette = LoadCachedResource(g_mw2PrjHandle, g_paletteCycleResource, g_resourceTypeTags[c_resTagPal], 0);
	if (palette == NULL) {
		return;
	}

	InitPaletteCycle(&g_paletteCycle, palette, p_first, p_count);
	memcpy(g_paletteCycle.m_working, palette, 0x300);
	g_paletteCycle.m_first = 0x80;
	g_paletteCycle.m_count = 0x10;
	g_paletteCycling = 1;
}

// FUNCTION: MW2 0x10002971
// FUNCTION: MW2MATROX 0x1002dc2f
void StopPaletteCycle(void)
{
	if (!g_paletteCycling) {
		return;
	}

	FreePaletteCycle(&g_paletteCycle);
	g_paletteCycling = 0;
	UnlockCachedResource(g_paletteCycleResource, g_resourceTypeTags[c_resTagPal]);
	g_paletteCycleResource = -1;
	ApplyPendingPalette();
}

// FUNCTION: MW2 0x100029c8
// FUNCTION: MW2MATROX 0x1002dc86
MechS32 SetPaletteResourceId(MechS32 p_id, MechS32 p_slot)
{
	MechS32 old;

	old = g_paletteResourceIds[p_slot];
	g_paletteResourceIds[p_slot] = p_id;
	LoadCachedResource(g_mw2PrjHandle, p_id, g_resourceTypeTags[c_resTagPal], 0);
	UnlockCachedResource(p_id, g_resourceTypeTags[c_resTagPal]);
	return old;
}

// FUNCTION: MW2 0x10002a24
// FUNCTION: MW2MATROX 0x1002dce2
void FadeToBasePalette(MechS32 p_palette, MechS32 p_duration)
{
	MechS32 palette;

	palette = p_palette;
	StartPaletteFade(palette, p_duration, 0);
	g_basePalette = p_palette;
	g_settledPalette = palette;
}

// FUNCTION: MW2 0x10002a5a
// FUNCTION: MW2MATROX 0x1002dd18
void SetBasePalette(MechS32 p_palette)
{
	g_basePalette = p_palette;
	g_settledPalette = p_palette;
}

// Brings up the start palette (slot 0x10): with p_dissolve, it dissolves the screen to black
// over 0x10f ticks first. It passes the result of the slot test, not the slot's resource id,
// to the loader, so it always loads PAL resource 1 (SimMain has the same test).
// Matches except for the stack slots of all its locals (a consistent permutation).
// FUNCTION: MW2 0x10002a75
void StartPalettes(MechS32 p_dissolve)
{
	MechU8* palette;
	MechS32 last;
	MechS32 handle;
	WINDOW buffer;
	MechS32 hasPalette;
	MechS32 seed;
	PANE* src;
	PANE target;
	MechS32 count;
	void* pixels;
	MechS32 ticks;

	seed = 0;
	last = -1;
	ticks = 0;
	hasPalette = g_paletteResourceIds[0x10] != -1;
	if (hasPalette) {
		palette = LoadCachedResource(g_mw2PrjHandle, hasPalette, g_resourceTypeTags[c_resTagPal], 0);
		if (palette) {
			if (p_dissolve == 0) {
				g_currentDisplayBackend->m_blendPalettes((PaletteColor*) palette, 30);
				UnlockCachedResource(hasPalette, g_resourceTypeTags[c_resTagPal]);
				g_currentDisplayBackend->m_setPaletteWithBrightness((PaletteColor*) palette);
			}
			else {
				pixels = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, g_refreshModePixelCount);
				if (pixels) {
					target = g_currentPane;
					target.m_window = &buffer;
					buffer = g_mainPixelBuffer;
					buffer.m_buffer = pixels;
					src = &g_currentPane;
					count = (g_refreshModePixelCount * 4) / 181;
					handle = AllocTicks(0x100);
					ResetTicks(handle);
					while (ticks < 0x10f) {
						ticks = GetTicks(handle);
						if (ticks > last) {
							last = ticks + 1;
							seed = VFX_pixel_fade(&target, src, count, seed);
							if (g_windowActive) {
								g_currentRefreshMode->m_flip();
							}
						}
					}

					VFX_pane_wipe(src, 0);
					if (g_windowActive) {
						g_currentRefreshMode->m_flip();
					}

					g_currentDisplayBackend->m_setPaletteWithBrightness((PaletteColor*) palette);
					HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, pixels);
					FreeTicks(handle);
				}
			}
		}
	}

	g_currentPalette = 0x10;
	g_settledPalette = 0x10;
}

// FUNCTION: MW2 0x10002c76
MechS32 GetPaletteFadeSteps(void)
{
	return g_paletteFadeSteps;
}

#ifdef MW2_MATROX
// STUB: MW2MATROX 0x1005708c
void FUN_1005708c(MechS32 p_index, MechS32 p_pixel)
{
	STUB(0x1005708c);
}

// STUB: MW2MATROX 0x100570aa
MechS32 FUN_100570aa(MechS32 p_color)
{
	STUB(0x100570aa);
	return 0;
}

// STUB: MW2MATROX 0x1005f790
void FUN_1005f790(PaletteColor* p_palette)
{
	STUB(0x1005f790);
}

// STUB: MW2MATROX 0x10088246
void FUN_10088246(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom)
{
	STUB(0x10088246);
}

// STUB: MW2MATROX 0x10088280
void FUN_10088280(PANE* p_pane)
{
	STUB(0x10088280);
}
#endif
