#ifndef PALETTE_H
#define PALETTE_H

#include "decomp.h"
#include "eyepoint.h"
#include "rendertarget.h"
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
	void FUN_100024f0(Eyepoint* p_eyepoint, MechS32* p_x, MechS32* p_y);
	void ApplyPendingPalette(void);
	void ApplyPaletteResource(MechS32 p_slot);
	void UpdatePaletteFade(void);
	MechS32 StartPaletteFade(MechS32 p_palette, MechS32 p_duration, MechS32 p_mode);
	MechS32 FUN_1000288e(MechS32 p_offset, MechS32 p_duration, MechS32 p_mode);
	void StartPaletteCycle(MechU8 p_first, MechS32 p_count);
	void StopPaletteCycle(void);
	MechS32 SetPaletteResourceId(MechS32 p_id, MechS32 p_slot);
	void FUN_10002a24(MechS32 p_palette, MechS32 p_duration);
	void FUN_10002a5a(MechS32 p_palette);
	void StartPalettes(MechS32 p_dissolve);
	MechS32 FUN_10002c76(void);

#ifdef __cplusplus
}
#endif

#endif // PALETTE_H
