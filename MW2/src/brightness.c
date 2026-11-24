#include "brightness.h"

#include "decomp.h"
#include "displaybackend.h"
#include "palettecolor.h"
#include "refreshmode.h"
#include "types.h"

#include <math.h>

// Display brightness: a gamma table with one row per brightness level, applied to the palette.

// GLOBAL: MW2 0x100a9468
// GLOBAL: MW2GBL 0x100b5a84
// GLOBAL: MW2PREMERCS 0x100b00e0
// GLOBAL: MW2MATROX 0x100bcbe4
// GLOBAL: MW23DFX 0x1024a848
// GLOBAL: MW2ATI 0x1017eca0
// GLOBAL: MW2S3 0x100652f8
// GLOBAL: MW2POWERVR 0x101a99c0
MechS32 g_displayBrightness = 9;

// GLOBAL: MW2 0x100a946c
// GLOBAL: MW2GBL 0x100b5a88
// GLOBAL: MW2PREMERCS 0x100b00e4
// GLOBAL: MW2MATROX 0x100bcbe8
// GLOBAL: MW23DFX 0x1024a84c
// GLOBAL: MW2ATI 0x1017eca4
// GLOBAL: MW2S3 0x100652fc
// GLOBAL: MW2POWERVR 0x101a99c4
MechS32 g_brightnessSetting = 9;

// GLOBAL: MW2 0x100e96a0
// GLOBAL: MW2GBL 0x100bc250
// GLOBAL: MW2PREMERCS 0x10212f40
// GLOBAL: MW2MATROX 0x1012bdf0
// GLOBAL: MW23DFX 0x102447b0
// GLOBAL: MW2ATI 0x10178470
// GLOBAL: MW2S3 0x100b1e20
// GLOBAL: MW2POWERVR 0x101a3130
PaletteColor g_paletteColorsPreBrightness[0x100];

// GLOBAL: MW2 0x100e99a0
// GLOBAL: MW2GBL 0x100bc550
// GLOBAL: MW2PREMERCS 0x10213240
// GLOBAL: MW2MATROX 0x1012b9f0
// GLOBAL: MW23DFX 0x102443b0
// GLOBAL: MW2ATI 0x10178070
// GLOBAL: MW2S3 0x100b2120
// GLOBAL: MW2POWERVR 0x101a2d30
MechU8 g_gammaTable[16][64];

// Row i maps a 6-bit color component c to 63 * (c / 63) ^ (1 / (0.5 + i / 16)).
// FUNCTION: MW2 0x10058400
// FUNCTION: MW2GBL 0x10073300
// FUNCTION: MW2PREMERCS 0x10025a80
// FUNCTION: MW2MATROX 0x1008da50
// FUNCTION: MW23DFX 0x100072a0
// FUNCTION: MW2ATI 0x10007c20
// FUNCTION: MW2S3 0x10020c60
// FUNCTION: MW2POWERVR 0x10006960
void InitGammaTable(void)
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

// FUNCTION: MW2 0x100584a7
// FUNCTION: MW2GBL 0x100733b7
// FUNCTION: MW2PREMERCS 0x10025b37
// FUNCTION: MW2MATROX 0x1008db2f
// FUNCTION: MW23DFX 0x10007330
void SavePreBrightnessPalette(void)
{
	GetPaletteColors(0, 0x100, g_paletteColorsPreBrightness);
}

// Sets the palette at brightness p_brightness without changing g_displayBrightness.
// FUNCTION: MW2 0x100584c6
// FUNCTION: MW2GBL 0x100733d6
// FUNCTION: MW2PREMERCS 0x10025b56
// FUNCTION: MW2MATROX 0x1008db4e
// FUNCTION: MW23DFX 0x10007350
// FUNCTION: MW2ATI 0x10007cb0
// FUNCTION: MW2S3 0x10020cd0
// FUNCTION: MW2POWERVR 0x100069f0
void PreviewBrightness(MechS32 p_brightness)
{
	MechS32 brightness;

	brightness = g_displayBrightness;
	g_displayBrightness = p_brightness;
	g_currentDisplayBackend->m_setPaletteWithBrightness(g_paletteColorsPreBrightness);
	g_displayBrightness = brightness;
}

// FUNCTION: MW2 0x100584fc
// FUNCTION: MW2GBL 0x1007340c
// FUNCTION: MW2PREMERCS 0x10025b8c
// FUNCTION: MW2MATROX 0x1008db84
// FUNCTION: MW23DFX 0x10007380
// FUNCTION: MW2ATI 0x10007ce0
// FUNCTION: MW2S3 0x10020d00
// FUNCTION: MW2POWERVR 0x10006a20
void CopyPaletteColorWithBrightness(PaletteColor* p_src, PaletteColor* p_dst)
{
	MechU8* row;

	row = g_gammaTable[g_displayBrightness];
	p_dst->m_red = row[p_src->m_red];
	p_dst->m_green = row[p_src->m_green];
	p_dst->m_blue = row[p_src->m_blue];
}
