#include "unk1004b130.h"

#include "decomp.h"
#include "object.h"
#include "types.h"

// GLOBAL: MW2 0x100a7120
MechS32 g_unk0x100a7120 = 0;

// GLOBAL: MW2 0x100a7128
AmberWillow0x7c* g_unk0x100a7128 = NULL;

// FUNCTION: MW2 0x1004b539
void FUN_1004b539(MechS32 p_enable)
{
	if (g_unk0x100a7128) {
		if (p_enable) {
			FUN_10001926(g_unk0x100a7128);
		}
		else {
			FUN_100018ca(g_unk0x100a7128);
		}

		FUN_10001cf8(g_unk0x100a7128);
		g_unk0x100a7120 = p_enable;
	}
}
