#ifndef PALETTE_H
#define PALETTE_H

#include "decomp.h"
#include "eyepoint.h"
#include "palettecolor.h"
#include "targeting.h"
#include "types.h"

// The functions and globals of palette.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_palettePending;
	extern MechS32 g_paletteResourceIds[20];
	extern MechS32 g_paneIndex;
	extern MechS32 g_currentPalette;
	// SelectPane copies one of the eleven into the current one (g_currentPane) and sizes the
	// eyepoint's view to it.
	extern PANE g_panes[11];

	void InitPanes(PANE* p_target);
	void SelectPane(MechS32 p_index);
	void GetViewCenter(Eyepoint* p_eyepoint, MechS32* p_x, MechS32* p_y);
	void ApplyPendingPalette(void);
	void ApplyPaletteResource(MechS32 p_slot);
	void UpdatePaletteFade(void);
	MechS32 StartPaletteFade(MechS32 p_palette, MechS32 p_duration, MechS32 p_mode);
	MechS32 StartPaletteFlash(MechS32 p_offset, MechS32 p_duration, MechS32 p_mode);
	void StartPaletteCycle(MechU8 p_first, MechS32 p_count);
	void StopPaletteCycle(void);
	MechS32 SetPaletteResourceId(MechS32 p_id, MechS32 p_slot);
	void FadeToBasePalette(MechS32 p_palette, MechS32 p_duration);
	void SetBasePalette(MechS32 p_palette);
	void StartPalettes(MechS32 p_dissolve);
	MechS32 GetPaletteFadeSteps(void);

#ifdef MW2_MATROX
	// The edition's: the 16-bit pixel of a palette color (a lookup in a word table). Its callers
	// use the whole of eax (they called it undeclared, as an int function).
	MechS32 FUN_100570aa(MechS32 p_color);
	// The edition's: sets a palette color's 16-bit pixel (FUN_100570aa's table).
	void FUN_1005708c(MechS32 p_index, MechS32 p_pixel);
	// The edition's: sets the display's palette (1.1 calls m_setPaletteWithBrightness).
	void FUN_1005f790(PaletteColor* p_palette);
	extern MechS32 g_unk0x100aa1d0;
	extern MechFloat g_unk0x10184700[3];

	// The edition's: marks a rectangle of the screen (right and bottom exclusive) for presenting.
	void FUN_10088246(MechS32 p_left, MechS32 p_top, MechS32 p_right, MechS32 p_bottom);
	// The edition's: marks a pane's rectangle of the screen for presenting.
	void FUN_10088280(PANE* p_pane);
#endif

// A color passed to a drawing primitive: the edition draws 16-bit pixels.
#ifdef MW2_MATROX
#define PIXEL_COLOR(c) FUN_100570aa(c)
#else
#define PIXEL_COLOR(c) (c)
#endif

#ifdef __cplusplus
}
#endif

#endif // PALETTE_H
