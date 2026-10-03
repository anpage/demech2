#include "palidentity.h"

#include "menu.h"
#include "types.h"

// Fills g_unk0x100e9350 with the identity mapping of the 256 palette indices.
// FUNCTION: MW2 0x10065f10
void FUN_10065f10(void)
{
	MechS32 i;

	for (i = 0; i < 0x100; i++) {
		g_unk0x100e9350[i] = i;
	}
}
