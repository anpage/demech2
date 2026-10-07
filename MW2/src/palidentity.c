#include "palidentity.h"

#include "menu.h"
#include "palette.h"
#include "types.h"

// Fills g_textColors with the identity mapping of the 256 palette indices.
// FUNCTION: MW2 0x10065f10
// FUNCTION: MW2MATROX 0x1007f500
void ResetTextColors(void)
{
	MechS32 i;

	for (i = 0; i < 0x100; i++) {
		g_textColors[i] = PIXEL_COLOR(i);
	}
#ifdef MW2_MATROX

	g_textColors[0xff] = 0x8000;
#endif
}
