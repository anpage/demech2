#include "palette.h"

#include "clock.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "loadres.h"
#include "palcycle.h"
#include "palfade.h"
#include "refreshmode.h"
#include "rendertarget.h"
#include "simmain.h"
#include "ticks.h"
#include "types.h"

#include <string.h>
#include <windows.h>

// GLOBAL: MW2 0x100a00cc
MechS32 g_renderTargetIndex = -1;

// GLOBAL: MW2 0x100a00d0
MechS32 g_currentPalette = 0x10;

// GLOBAL: MW2 0x100a00d4
MechS32 g_palettePending = 0;

// GLOBAL: MW2 0x100a00d8
MechS32 g_unk0x100a00d8 = 0;

// GLOBAL: MW2 0x100a00dc
MechS32 g_unk0x100a00dc = 0x10;

// GLOBAL: MW2 0x100a00e0
MechS32 g_paletteFadeTarget = -1;

// GLOBAL: MW2 0x100a00e4
MechS32 g_paletteFadeBack = -1;

// GLOBAL: MW2 0x100a00e8
MechS32 g_paletteFadeBackSteps = 0;

// GLOBAL: MW2 0x100a00ec
MechS32 g_paletteFadeSteps = 0;

// GLOBAL: MW2 0x100a00f0
MechS32 g_paletteCycling = 0;

// GLOBAL: MW2 0x100a00f4
MechS32 g_paletteCycleResource = -1;

// GLOBAL: MW2 0x100bcd20
static PaletteFade g_paletteFade;

// GLOBAL: MW2 0x100bcd40
static PaletteCycle g_paletteCycle;

// GLOBAL: MW2 0x10181a60
RenderTarget g_renderTargets[11];

// GLOBAL: MW2 0x10181b40
MechS32 g_paletteResourceIds[20];

// FUNCTION: MW2 0x100023c0
void InitRenderTargets(RenderTarget* p_target)
{
	MechS32 i;

	for (i = 0; i < 11; i++) {
		g_renderTargets[i] = *p_target;
	}

	for (i = 0; i < 20; i++) {
		g_paletteResourceIds[i] = 0;
	}
}

// FUNCTION: MW2 0x1000242f
void SelectRenderTarget(MechS32 p_index)
{
	RenderTarget* target;

	if (p_index < 0 || p_index >= 11) {
		return;
	}

	if (p_index != g_renderTargetIndex) {
		target = &g_renderTargets[p_index];
		g_eyepoint->m_unk0x2c = 0;
		g_eyepoint->m_unk0x34 = 0;
		g_eyepoint->m_unk0x30 = target->m_right - target->m_left;
		g_eyepoint->m_unk0x38 = target->m_bottom - target->m_top;
		g_eyepoint->m_unk0x4c = 0;
		g_eyepoint->m_unk0x50 = 0;
		g_currentRenderTarget = *target;
		g_renderTargetIndex = p_index;
		g_unk0x100a2460 = 1;
	}
}

// FUNCTION: MW2 0x100024f0
void FUN_100024f0(Eyepoint* p_eyepoint, MechS32* p_x, MechS32* p_y)
{
	MechS32 x;
	MechS32 y;

	x = p_eyepoint->m_unk0x4c + (p_eyepoint->m_unk0x2c + p_eyepoint->m_unk0x30) / 2;
	y = p_eyepoint->m_unk0x50 + (p_eyepoint->m_unk0x34 + p_eyepoint->m_unk0x38) / 2;
	*p_x = x;
	*p_y = y;
}

// FUNCTION: MW2 0x10002546
void ApplyPendingPalette(void)
{
	if (g_palettePending && g_paletteFadeSteps <= 0) {
		g_palettePending = 0;
		ApplyPaletteResource(g_unk0x100a00dc);
		g_currentPalette = g_unk0x100a00dc;
	}
}

// FUNCTION: MW2 0x1000258d
void ApplyPaletteResource(MechS32 p_slot)
{
	MechU8* palette;
	MechS32* id;

	id = &g_paletteResourceIds[p_slot];
	if (*id > 0) {
		palette = FUN_1001a19f(g_unk0x100a8740, *id, g_unk0x100a8694, 0);
		if (palette) {
			g_currentDisplayBackend->m_setPaletteWithBrightness((PaletteColor*) palette);
			FUN_1001a163(*id, g_unk0x100a8694);
		}
	}
}

// FUNCTION: MW2 0x10002600
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
		from = FUN_1001a19f(g_unk0x100a8740, g_paletteResourceIds[fromSlot], g_unk0x100a8694, 0);
	}

	if (from) {
		to = FUN_1001a19f(g_unk0x100a8740, g_paletteResourceIds[p_palette], g_unk0x100a8694, 0);
		if (to) {
			g_paletteFadeTarget = p_palette;
			g_paletteFadeBack = fromSlot;
			if (g_deltaTime) {
				if (p_mode == 1) {
					p_duration >>= 1;
				}

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

			if (p_mode == 1 || p_mode == 2) {
				g_paletteFadeBackSteps = steps;
			}
			else {
				g_paletteFadeBackSteps = 0;
			}

			InitPaletteFade(&g_paletteFade, from, to, 0, 0x100, steps);
			result = 1;
			FUN_1001a163(g_paletteResourceIds[p_palette], g_unk0x100a8694);
		}

		if (fromSlot == -1) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, from);
		}
		else {
			FUN_1001a163(g_paletteResourceIds[fromSlot], g_unk0x100a8694);
		}
	}

	return result;
}

// FUNCTION: MW2 0x1000288e
MechS32 FUN_1000288e(MechS32 p_offset, MechS32 p_duration, MechS32 p_mode)
{
	return StartPaletteFade(p_offset + g_unk0x100a00d8, p_duration, p_mode);
}

// FUNCTION: MW2 0x100028b7
void StartPaletteCycle(MechU8 p_first, MechS32 p_count)
{
	MechU8* palette;

	if (g_paletteFadeSteps > 0 || g_paletteCycling) {
		return;
	}

	g_paletteCycling = 1;
	g_paletteCycleResource = g_paletteResourceIds[g_currentPalette];
	palette = FUN_1001a19f(g_unk0x100a8740, g_paletteCycleResource, g_unk0x100a8694, 0);
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
void StopPaletteCycle(void)
{
	if (!g_paletteCycling) {
		return;
	}

	FreePaletteCycle(&g_paletteCycle);
	g_paletteCycling = 0;
	FUN_1001a163(g_paletteCycleResource, g_unk0x100a8694);
	g_paletteCycleResource = -1;
	ApplyPendingPalette();
}

// FUNCTION: MW2 0x100029c8
MechS32 SetPaletteResourceId(MechS32 p_id, MechS32 p_slot)
{
	MechS32 old;

	old = g_paletteResourceIds[p_slot];
	g_paletteResourceIds[p_slot] = p_id;
	FUN_1001a19f(g_unk0x100a8740, p_id, g_unk0x100a8694, 0);
	FUN_1001a163(p_id, g_unk0x100a8694);
	return old;
}

// FUNCTION: MW2 0x10002a24
void FUN_10002a24(MechS32 p_palette, MechS32 p_duration)
{
	MechS32 palette;

	palette = p_palette;
	StartPaletteFade(palette, p_duration, 0);
	g_unk0x100a00d8 = p_palette;
	g_unk0x100a00dc = palette;
}

// FUNCTION: MW2 0x10002a5a
void FUN_10002a5a(MechS32 p_palette)
{
	g_unk0x100a00d8 = p_palette;
	g_unk0x100a00dc = p_palette;
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
	PixelBuffer buffer;
	MechS32 hasPalette;
	MechS32 seed;
	RenderTarget* src;
	RenderTarget target;
	MechS32 count;
	void* pixels;
	MechS32 ticks;

	seed = 0;
	last = -1;
	ticks = 0;
	hasPalette = g_paletteResourceIds[0x10] != -1;
	if (hasPalette) {
		palette = FUN_1001a19f(g_unk0x100a8740, hasPalette, g_unk0x100a8694, 0);
		if (palette) {
			if (p_dissolve == 0) {
				g_currentDisplayBackend->m_blendPalettes((PaletteColor*) palette, 30);
				FUN_1001a163(hasPalette, g_unk0x100a8694);
				g_currentDisplayBackend->m_setPaletteWithBrightness((PaletteColor*) palette);
			}
			else {
				pixels = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, g_refreshModePixelCount);
				if (pixels) {
					target = g_currentRenderTarget;
					target.m_buffer = &buffer;
					buffer = g_mainPixelBuffer;
					buffer.m_pixels = pixels;
					src = &g_currentRenderTarget;
					count = (g_refreshModePixelCount * 4) / 181;
					handle = AllocTicks(0x100);
					ResetTicks(handle);
					while (ticks < 0x10f) {
						ticks = GetTicks(handle);
						if (ticks > last) {
							last = ticks + 1;
							seed = DissolveView(&target, src, count, seed);
							if (g_windowActive) {
								g_currentRefreshMode->m_flip();
							}
						}
					}

					FillView(src, 0);
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
	g_unk0x100a00dc = 0x10;
}

// FUNCTION: MW2 0x10002c76
MechS32 FUN_10002c76(void)
{
	return g_paletteFadeSteps;
}
