#include "palette.h"

#include "decomp.h"
#include "types.h"

// GLOBAL: MW2 0x100a00d8
MechS32 g_unk0x100a00d8 = 0;

// GLOBAL: MW2 0x100a00dc
MechS32 g_unk0x100a00dc = 0x10;

// STUB: MW2 0x10002546
void ApplyPendingPalette(void)
{
	STUB(0x10002546);
}

// STUB: MW2 0x10002600
void UpdatePaletteFade(void)
{
	STUB(0x10002600);
}

// STUB: MW2 0x10002687
void StartPaletteFade(MechS32 p_palette, MechS32 p_duration, undefined4 p_unk0x08)
{
	STUB(0x10002687);
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

// STUB: MW2 0x10002a75
void StartPalettes(MechS32 p_unk0x00)
{
	STUB(0x10002a75);
}
