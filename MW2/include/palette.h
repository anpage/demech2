#ifndef PALETTE_H
#define PALETTE_H

#include "decomp.h"
#include "eyepoint.h"
#include "palettecolor.h"
#include "targeting.h"
#include "types.h"

#ifdef MW2_ACCEL
// A3D_SetPalette (the A3D renderer) and FUN_10088246/FUN_10088280 (the key color fills), for
// palette.h's users.
#include "accel/a3d.h"
#ifdef MW2_MATROX
#include "matrox/colorkey.h"
#endif
#endif

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

#ifdef MW2_ACCEL
	// The Matrox edition's 16-bit 2D object (accel/vfx16.asm, annotated in accel/vfx16.h), declared
	// here for PIXEL_COLOR's users. The 16-bit pixel of a palette color, from the object's lookaside
	// table; its callers use the whole of eax (they called it undeclared, as an int function).
	MechS32 __cdecl VFX_lookaside_read16(MechS32 p_color);
	// Sets a palette color's 16-bit pixel in the lookaside table.
	void __cdecl VFX_lookaside_write16(MechS32 p_index, MechS32 p_pixel);
#endif

// A color passed to a drawing primitive: the Matrox edition draws 16-bit pixels.
#ifdef MW2_ACCEL
#define PIXEL_COLOR(c) VFX_lookaside_read16(c)
#else
#define PIXEL_COLOR(c) (c)
#endif

#ifdef __cplusplus
}
#endif

#endif // PALETTE_H
