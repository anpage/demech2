#include "decomp.h"
#include "drawmodeextension.h"
#include "types.h"

#include <math.h>

// Display brightness: a gamma table with one row per brightness level, applied to the palette.

extern DrawModeExtension* g_currentDrawModeExtension;

MechS32 FUN_10011450(MechS32 p_first, MechS32 p_count, MechU8* p_palette);

// GLOBAL: MW2SHELL 0x100717a4
MechS32 g_displayBrightness = 9;

// GLOBAL: MW2SHELL 0x10095ed0
MechU8 g_paletteColorsPreBrightness[0x300];

// GLOBAL: MW2SHELL 0x100961d0
MechU8 g_gammaTable[16][64];

// Row i maps a 6-bit color component c to 63 * (c / 63) ^ (1 / (0.5 + i / 16)).
// Stack-slot permutation: i, j, row, x and exponent.
// FUNCTION: MW2SHELL 0x100460a0
void FUN_100460a0()
{
	double x;
	MechS32 j;
	MechS32 i;
	MechU8* row;
	double exponent;

	for (i = 0; i < 16; i++) {
		exponent = i * 0.0625 + 0.5;
		exponent = 1.0 / exponent;
		row = g_gammaTable[i];
		for (j = 0; j < 64; j++) {
			x = j / 63.0;
			row[j] = (MechU8) (pow(x, exponent) * 63.0);
		}
	}
}

// FUNCTION: MW2SHELL 0x10046147
void FUN_10046147()
{
	FUN_10011450(0, 0x100, g_paletteColorsPreBrightness);
}

// Sets the palette at brightness p_brightness without changing g_displayBrightness.
// FUNCTION: MW2SHELL 0x10046166
void FUN_10046166(MechS32 p_brightness)
{
	MechS32 brightness;

	brightness = g_displayBrightness;
	g_displayBrightness = p_brightness;
	g_currentDrawModeExtension->m_setPaletteWithBrightness(g_paletteColorsPreBrightness);
	g_displayBrightness = brightness;
}

// FUNCTION: MW2SHELL 0x1004619c
void CopyPaletteColorWithBrightness(MechU8* p_src, MechU8* p_dst)
{
	MechU8* row;

	row = g_gammaTable[g_displayBrightness];
	p_dst[0] = row[p_src[0]];
	p_dst[1] = row[p_src[1]];
	p_dst[2] = row[p_src[2]];
}
