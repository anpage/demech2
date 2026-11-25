#include "palette.h"

#include "animation.h"
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

#ifdef MW2_VFX16
#include "accel/vfx16.h"
#endif

// The S3 edition starts this object with the textured-polygon drawer, which the ATI edition has in
// bandpoly.c's.
#ifdef MW2_S3
#include "bandtexture.c"
#endif

// GLOBAL: MW2 0x100a00cc
// GLOBAL: MW2GBL 0x100a0488
// GLOBAL: MW2PREMERCS 0x100b3c40
// GLOBAL: MW2MATROX 0x100a61f8
// GLOBAL: MW23DFX 0x1024e450
// GLOBAL: MW2S3 0x10075df4
// GLOBAL: MW2POWERVR 0x101ad76c
MechS32 g_paneIndex = -1;

// GLOBAL: MW2 0x100a00d0
// GLOBAL: MW2GBL 0x100a048c
// GLOBAL: MW2PREMERCS 0x100b3c44
// GLOBAL: MW2MATROX 0x100a61fc
// GLOBAL: MW23DFX 0x1024e454
// GLOBAL: MW2ATI 0x10182764
// GLOBAL: MW2S3 0x10075df8
// GLOBAL: MW2POWERVR 0x101ad770
MechS32 g_currentPalette = 0x10;

// GLOBAL: MW2 0x100a00d4
// GLOBAL: MW2GBL 0x100a0490
// GLOBAL: MW2PREMERCS 0x100b3c48
// GLOBAL: MW2MATROX 0x100a6200
// GLOBAL: MW23DFX 0x1024e458
// GLOBAL: MW2ATI 0x10182768
// GLOBAL: MW2S3 0x10075dfc
// GLOBAL: MW2POWERVR 0x101ad774
MechS32 g_palettePending = 0;

// GLOBAL: MW2 0x100a00d8
// GLOBAL: MW2GBL 0x100a0494
// GLOBAL: MW2PREMERCS 0x100b3c4c
// GLOBAL: MW2MATROX 0x100a6204
// GLOBAL: MW23DFX 0x1024e45c
// GLOBAL: MW2ATI 0x1018276c
// GLOBAL: MW2S3 0x10075e00
// GLOBAL: MW2POWERVR 0x101ad778
MechS32 g_basePalette = 0;

// GLOBAL: MW2 0x100a00dc
// GLOBAL: MW2GBL 0x100a0498
// GLOBAL: MW2PREMERCS 0x100b3c50
// GLOBAL: MW2MATROX 0x100a6208
// GLOBAL: MW23DFX 0x1024e460
// GLOBAL: MW2ATI 0x10182770
// GLOBAL: MW2S3 0x10075e04
// GLOBAL: MW2POWERVR 0x101ad77c
MechS32 g_settledPalette = 0x10;

// GLOBAL: MW2 0x100a00e0
// GLOBAL: MW2GBL 0x100a049c
// GLOBAL: MW2PREMERCS 0x100b3c54
// GLOBAL: MW2MATROX 0x100a620c
// GLOBAL: MW23DFX 0x1024e464
// GLOBAL: MW2ATI 0x10182774
// GLOBAL: MW2S3 0x10075e08
// GLOBAL: MW2POWERVR 0x101ad780
MechS32 g_paletteFadeTarget = -1;

// GLOBAL: MW2 0x100a00e4
// GLOBAL: MW2GBL 0x100a04a0
// GLOBAL: MW2PREMERCS 0x100b3c58
// GLOBAL: MW2MATROX 0x100a6210
// GLOBAL: MW23DFX 0x1024e468
// GLOBAL: MW2ATI 0x10182778
// GLOBAL: MW2S3 0x10075e0c
// GLOBAL: MW2POWERVR 0x101ad784
MechS32 g_paletteFadeBack = -1;

// GLOBAL: MW2 0x100a00e8
// GLOBAL: MW2GBL 0x100a04a4
// GLOBAL: MW2PREMERCS 0x100b3c5c
// GLOBAL: MW2MATROX 0x100a6214
// GLOBAL: MW23DFX 0x1024e46c
// GLOBAL: MW2ATI 0x1018277c
// GLOBAL: MW2S3 0x10075e10
// GLOBAL: MW2POWERVR 0x101ad788
MechS32 g_paletteFadeBackSteps = 0;

// GLOBAL: MW2 0x100a00ec
// GLOBAL: MW2GBL 0x100a04a8
// GLOBAL: MW2PREMERCS 0x100b3c60
// GLOBAL: MW2MATROX 0x100a6218
// GLOBAL: MW23DFX 0x1024e470
// GLOBAL: MW2ATI 0x10182780
// GLOBAL: MW2S3 0x10075e14
// GLOBAL: MW2POWERVR 0x101ad78c
MechS32 g_paletteFadeSteps = 0;

// GLOBAL: MW2 0x100a00f0
// GLOBAL: MW2GBL 0x100a04ac
// GLOBAL: MW2PREMERCS 0x100b3c64
// GLOBAL: MW2MATROX 0x100a621c
// GLOBAL: MW23DFX 0x1024e474
// GLOBAL: MW2ATI 0x10182784
// GLOBAL: MW2S3 0x10075e18
// GLOBAL: MW2POWERVR 0x101ad790
MechS32 g_paletteCycling = 0;

// GLOBAL: MW2 0x100a00f4
// GLOBAL: MW2GBL 0x100a04b0
// GLOBAL: MW2PREMERCS 0x100b3c68
// GLOBAL: MW2MATROX 0x100a6220
// GLOBAL: MW23DFX 0x1024e478
MechS32 g_paletteCycleResource = -1;

// GLOBAL: MW2 0x100bcd20
// GLOBAL: MW2GBL 0x100b90e8
// GLOBAL: MW2PREMERCS 0x100cb398
// GLOBAL: MW2MATROX 0x100c1ff8
// GLOBAL: MW23DFX 0x1007c6b8
// GLOBAL: MW2ATI 0x100716b8
// GLOBAL: MW2S3 0x1007efd0
// GLOBAL: MW2POWERVR 0x1006f700
static PaletteFade g_paletteFade;

// GLOBAL: MW2 0x100bcd40
// GLOBAL: MW2GBL 0x100b9108
// GLOBAL: MW2PREMERCS 0x100cb3b8
// GLOBAL: MW2MATROX 0x100c2018
// GLOBAL: MW23DFX 0x1007c6d8
// GLOBAL: MW2ATI 0x100716d8
// GLOBAL: MW2S3 0x1007eff0
// GLOBAL: MW2POWERVR 0x1006f720
static PaletteCycle g_paletteCycle;

// GLOBAL: MW2 0x10181a60
// GLOBAL: MW2GBL 0x10137af0
// GLOBAL: MW2PREMERCS 0x1017b8d0
// GLOBAL: MW2MATROX 0x101d4af0
// GLOBAL: MW23DFX 0x101b0d70
// GLOBAL: MW2ATI 0x100e4d60
// GLOBAL: MW2S3 0x100846f0
// GLOBAL: MW2POWERVR 0x1010c280
PANE g_panes[11];

// GLOBAL: MW2 0x10181b40
// GLOBAL: MW2GBL 0x10137bd0
// GLOBAL: MW2PREMERCS 0x1017b9b0
// GLOBAL: MW2MATROX 0x101d4aa0
// GLOBAL: MW23DFX 0x101b0d20
// GLOBAL: MW2ATI 0x100e4d10
// GLOBAL: MW2S3 0x100847d0
// GLOBAL: MW2POWERVR 0x1010c230
MechS32 g_paletteResourceIds[20];

// FUNCTION: MW2 0x100023c0
// FUNCTION: MW2GBL 0x1001e180
// FUNCTION: MW2PREMERCS 0x1005ef20
// FUNCTION: MW2MATROX 0x1002d560
// FUNCTION: MW23DFX 0x1002ab20
// FUNCTION: MW2ATI 0x1002a770
// FUNCTION: MW2S3 0x1003eba0
// FUNCTION: MW2POWERVR 0x10026a50
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
// FUNCTION: MW2GBL 0x1001e1ef
// FUNCTION: MW2PREMERCS 0x1005ef8f
// FUNCTION: MW2MATROX 0x1002d5cf
// FUNCTION: MW23DFX 0x1002ab50
// FUNCTION: MW2ATI 0x1002a7a0
// FUNCTION: MW2S3 0x1003ebd0
// FUNCTION: MW2POWERVR 0x10026a80
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
#ifndef MW2_NO_PROJECTION_OFFSET
		g_eyepoint->m_offsetX = 0;
		g_eyepoint->m_offsetY = 0;
#endif
		g_currentPane = *target;
		g_paneIndex = p_index;
		g_projectionDirty = 1;
	}
}

// FUNCTION: MW2 0x100024f0
// FUNCTION: MW2GBL 0x1001e2b0
// FUNCTION: MW2PREMERCS 0x1005f051
// FUNCTION: MW2MATROX 0x1002d678
// FUNCTION: MW23DFX 0x1002abd0
void GetViewCenter(Eyepoint* p_eyepoint, MechS32* p_x, MechS32* p_y)
{
	MechS32 x;
	MechS32 y;

#ifdef MW2_NO_PROJECTION_OFFSET
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
// FUNCTION: MW2GBL 0x1001e306
// FUNCTION: MW2MATROX 0x1002d6be
// FUNCTION: MW23DFX 0x1002ac00
// FUNCTION: MW2ATI 0x1002a820
// FUNCTION: MW2S3 0x1003ec50
// FUNCTION: MW2POWERVR 0x10026b00
void ApplyPendingPalette(void)
{
	if (g_palettePending && g_paletteFadeSteps <= 0) {
		g_palettePending = 0;
		ApplyPaletteResource(g_settledPalette);
		g_currentPalette = g_settledPalette;
	}
}

// FUNCTION: MW2 0x1000258d
// FUNCTION: MW2GBL 0x1001e34d
// FUNCTION: MW2PREMERCS 0x1005f0f3
// FUNCTION: MW2MATROX 0x1002d705
// FUNCTION: MW23DFX 0x1002ac40
// FUNCTION: MW2ATI 0x1002a860
// FUNCTION: MW2S3 0x1003ec90
// FUNCTION: MW2POWERVR 0x10026b40
void ApplyPaletteResource(MechS32 p_slot)
{
	MechU8* palette;
	MechS32* id;
#ifdef MW2_VFX16
	MechS32 i;
	PaletteColor color;
#endif

	id = &g_paletteResourceIds[p_slot];
	if (*id > 0) {
		palette = LoadCachedResource(g_mw2PrjHandle, *id, g_resTagPal, 0);
		if (palette) {
#ifdef MW2_VFX16
			for (i = 0; i < 0x100; i++) {
				color = ((PaletteColor*) palette)[i];
#if MW2_PIXEL_FORMAT == 555 && !defined(MW2_LOOKASIDE_RED_SHIFT_10)
				// The 3Dfx edition's 16-bit pixels: RGB555 from the 6-bit components.
				VFX_lookaside_write16(i, (color.m_red & ~1) << 9 | (color.m_green & ~1) << 4 | color.m_blue >> 1);
#else
				// The Matrox edition's 16-bit pixels: RGB565 from the 6-bit components (the PowerVR
				// edition's too, though its pixels are RGB555).
				VFX_lookaside_write16(i, (color.m_red & ~1) << 10 | color.m_green << 5 | color.m_blue >> 1);
#endif
			}
#endif

#ifdef MW2_A3D
			ResetTextColors();
#ifndef MW2_A3D_NO_PALETTE
			A3D_SetPalette((PaletteColor*) palette);
#endif
#else
			g_currentDisplayBackend->m_setPaletteWithBrightness((PaletteColor*) palette);
#endif
			UnlockCachedResource(*id, g_resTagPal);
		}
	}
}

// FUNCTION: MW2 0x10002600
// FUNCTION: MW2GBL 0x1001e3c0
// FUNCTION: MW2PREMERCS 0x1005f166
// FUNCTION: MW2MATROX 0x1002d7de
// FUNCTION: MW23DFX 0x1002acf0
// FUNCTION: MW2ATI 0x1002a910
// FUNCTION: MW2S3 0x1003ed40
// FUNCTION: MW2POWERVR 0x10026be0
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

#ifdef MW2_S3
// The S3 edition's fade light is a byte per component, blue first (animation.c).
#define FADE_LIGHT_RED 2
#define FADE_LIGHT_BLUE 0
#else
#define FADE_LIGHT_RED 0
#define FADE_LIGHT_BLUE 2
#endif

// FUNCTION: MW2 0x10002687
// FUNCTION: MW2GBL 0x1001e447
// FUNCTION: MW2PREMERCS 0x1005f1ed
// FUNCTION: MW2MATROX 0x1002d865
// FUNCTION: MW23DFX 0x1002ad50
// FUNCTION: MW2ATI 0x1002a970
// FUNCTION: MW2S3 0x1003eda0
// FUNCTION: MW2POWERVR 0x10026c40
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
		from = LoadCachedResource(g_mw2PrjHandle, g_paletteResourceIds[fromSlot], g_resTagPal, 0);
	}

	if (from) {
		to = LoadCachedResource(g_mw2PrjHandle, g_paletteResourceIds[p_palette], g_resTagPal, 0);
		if (to) {
			g_paletteFadeTarget = p_palette;
			g_paletteFadeBack = fromSlot;
			if (g_deltaTime) {
				if (p_mode == 1) {
					p_duration >>= 1;
				}

#ifdef MW2_FLOAT_SCALARS
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

#ifdef MW2_FADE_BACK_DURATION
			if (p_mode == 1 || p_mode == 2) {
				g_paletteFadeBackSteps = p_duration;
			}
			else {
				g_paletteFadeBackSteps = 0;
			}
#else
			if (p_mode == 1 || p_mode == 2) {
				g_paletteFadeBackSteps = steps;
			}
			else {
				g_paletteFadeBackSteps = 0;
			}
#endif

#ifdef MW2_A3D
			switch (p_palette) {
			case 0xc:
				g_unk0x100aa1d0 = 1;
				g_unk0x10184700[FADE_LIGHT_RED] = 0.0f;
				g_unk0x10184700[1] = 255.0f;
				g_unk0x10184700[FADE_LIGHT_BLUE] = 8.0f;
				break;
			case 0x11:
				g_unk0x100aa1d0 = 1;
				g_unk0x10184700[FADE_LIGHT_RED] = 255.0f;
				g_unk0x10184700[1] = 8.0f;
				g_unk0x10184700[FADE_LIGHT_BLUE] = 8.0f;
				break;
			case 0x10:
				g_unk0x100aa1d0 = 1;
				g_unk0x10184700[FADE_LIGHT_RED] = 0.0f;
				g_unk0x10184700[1] = 0.0f;
				g_unk0x10184700[FADE_LIGHT_BLUE] = 0.0f;
				break;
			default:
				g_unk0x100aa1d0 = 0;
#ifdef MW2_FADE_LIGHT_COLOUR
				g_unk0x10184700[FADE_LIGHT_RED] = 32.0f;
				g_unk0x10184700[1] = 32.0f;
				g_unk0x10184700[FADE_LIGHT_BLUE] = 32.0f;
#endif
				break;
			}
#endif
#ifdef MW2_FADE_LIGHT_COLOUR
			FUN_100607d0(g_unk0x10184700[0] / 32.0f, g_unk0x10184700[1] / 32.0f, g_unk0x10184700[2] / 32.0f);
#endif

			InitPaletteFade(&g_paletteFade, from, to, 0, 0x100, steps);
			result = 1;
			UnlockCachedResource(g_paletteResourceIds[p_palette], g_resTagPal);
		}

		if (fromSlot == -1) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, from);
		}
		else {
			UnlockCachedResource(g_paletteResourceIds[fromSlot], g_resTagPal);
		}
	}

	return result;
}

// FUNCTION: MW2 0x1000288e
// FUNCTION: MW2GBL 0x1001e64e
// FUNCTION: MW2PREMERCS 0x1005f3f4
// FUNCTION: MW2MATROX 0x1002db4c
// FUNCTION: MW23DFX 0x1002af60
// FUNCTION: MW2ATI 0x1002ab80
// FUNCTION: MW2S3 0x1003ef90
// FUNCTION: MW2POWERVR 0x10026ea0
MechS32 StartPaletteFlash(MechS32 p_offset, MechS32 p_duration, MechS32 p_mode)
{
	return StartPaletteFade(p_offset + g_basePalette, p_duration, p_mode);
}

// FUNCTION: MW2 0x100028b7
// FUNCTION: MW2GBL 0x1001e677
// FUNCTION: MW2PREMERCS 0x1005f41d
// FUNCTION: MW2MATROX 0x1002db75
// FUNCTION: MW23DFX 0x1002af80
void StartPaletteCycle(undefined4 p_first, MechS32 p_count)
{
	MechU8* palette;

	if (g_paletteFadeSteps > 0 || g_paletteCycling) {
		return;
	}

	g_paletteCycling = 1;
	g_paletteCycleResource = g_paletteResourceIds[g_currentPalette];
	palette = LoadCachedResource(g_mw2PrjHandle, g_paletteCycleResource, g_resTagPal, 0);
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
// FUNCTION: MW2GBL 0x1001e738
// FUNCTION: MW2PREMERCS 0x1005f4de
// FUNCTION: MW2MATROX 0x1002dc2f
// FUNCTION: MW23DFX 0x1002b010
void StopPaletteCycle(void)
{
	if (!g_paletteCycling) {
		return;
	}

	FreePaletteCycle(&g_paletteCycle);
	g_paletteCycling = 0;
	UnlockCachedResource(g_paletteCycleResource, g_resTagPal);
	g_paletteCycleResource = -1;
	ApplyPendingPalette();
}

// FUNCTION: MW2 0x100029c8
// FUNCTION: MW2GBL 0x1001e78f
// FUNCTION: MW2PREMERCS 0x1005f535
// FUNCTION: MW2MATROX 0x1002dc86
// FUNCTION: MW23DFX 0x1002b050
// FUNCTION: MW2ATI 0x1002aba0
// FUNCTION: MW2S3 0x1003efb0
// FUNCTION: MW2POWERVR 0x10026ec0
MechS32 SetPaletteResourceId(MechS32 p_id, MechS32 p_slot)
{
	MechS32 old;

	old = g_paletteResourceIds[p_slot];
	g_paletteResourceIds[p_slot] = p_id;
	LoadCachedResource(g_mw2PrjHandle, p_id, g_resTagPal, 0);
	UnlockCachedResource(p_id, g_resTagPal);
	return old;
}

// FUNCTION: MW2 0x10002a24
// FUNCTION: MW2GBL 0x1001e7eb
// FUNCTION: MW2PREMERCS 0x1005f591
// FUNCTION: MW2MATROX 0x1002dce2
// FUNCTION: MW23DFX 0x1002b090
// FUNCTION: MW2ATI 0x1002abe0
// FUNCTION: MW2S3 0x1003eff0
// FUNCTION: MW2POWERVR 0x10026f00
void FadeToBasePalette(MechS32 p_palette, MechS32 p_duration)
{
	MechS32 palette;

	palette = p_palette;
	StartPaletteFade(palette, p_duration, 0);
	g_basePalette = p_palette;
	g_settledPalette = palette;
}

// FUNCTION: MW2 0x10002a5a
// FUNCTION: MW2GBL 0x1001e821
// FUNCTION: MW2PREMERCS 0x1005f5c7
// FUNCTION: MW2MATROX 0x1002dd18
// FUNCTION: MW23DFX 0x1002b0b0
void SetBasePalette(MechS32 p_palette)
{
	g_basePalette = p_palette;
	g_settledPalette = p_palette;
}

// Brings up the start palette (slot 0x10): with p_dissolve, it dissolves the screen to black
// over 0x10f ticks first. It passes the result of the slot test, not the slot's resource id,
// to the loader, so it always loads PAL resource 1 (SimMain has the same test).
// FUNCTION: MW2 0x10002a75
// FUNCTION: MW2GBL 0x1001e83c
// FUNCTION: MW2MATROX 0x1002dd33
// FUNCTION: MW23DFX 0x1002b0c0
// FUNCTION: MW2ATI 0x1002ac00
// FUNCTION: MW2S3 0x1003f010
void StartPalettes(MechS32 p_dissolve)
{
	MechU8* palette;
#ifdef MW2_NO_DISSOLVE
	MechS32 hasPalette;

	// The Matrox edition has no dissolve.
	hasPalette = g_paletteResourceIds[0x10] != -1;
	if (hasPalette) {
		palette = LoadCachedResource(g_mw2PrjHandle, hasPalette, g_resTagPal, 0);
		if (palette) {
			g_currentDisplayBackend->m_blendPalettes((PaletteColor*) palette, 30);
			UnlockCachedResource(hasPalette, g_resTagPal);
			g_currentDisplayBackend->m_setPaletteWithBrightness((PaletteColor*) palette);
		}
	}
#else
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
		palette = LoadCachedResource(g_mw2PrjHandle, hasPalette, g_resTagPal, 0);
		if (palette) {
			if (p_dissolve == 0) {
				g_currentDisplayBackend->m_blendPalettes((PaletteColor*) palette, 30);
				UnlockCachedResource(hasPalette, g_resTagPal);
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
#endif

	g_currentPalette = 0x10;
	g_settledPalette = 0x10;
}

// FUNCTION: MW2 0x10002c76
// FUNCTION: MW2GBL 0x1001ea3d
// FUNCTION: MW2PREMERCS 0x1005f7e3
// FUNCTION: MW2MATROX 0x1002ddd8
// FUNCTION: MW23DFX 0x1002b130
// FUNCTION: MW2ATI 0x1002ac70
// FUNCTION: MW2S3 0x1003f080
// FUNCTION: MW2POWERVR 0x10026f20
MechS32 GetPaletteFadeSteps(void)
{
	return g_paletteFadeSteps;
}
