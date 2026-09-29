#ifndef PALETTE_H
#define PALETTE_H

#include "decomp.h"
#include "types.h"

// The functions and globals of palette.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void ApplyPendingPalette(void);
	void UpdatePaletteFade(void);
	void StartPaletteFade(MechS32 p_palette, MechS32 p_duration, undefined4 p_unk0x08);
	void FUN_10002a24(MechS32 p_palette, MechS32 p_duration);
	void FUN_10002a5a(MechS32 p_palette);
	void StartPalettes(MechS32 p_unk0x00);

#ifdef __cplusplus
}
#endif

#endif // PALETTE_H
