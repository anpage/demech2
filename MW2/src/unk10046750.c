#include "unk10046750.h"

#include "decomp.h"
#include "types.h"

// GLOBAL: MW2 0x100a6d70
MechS32 g_unk0x100a6d70 = 0;

// STUB: MW2 0x10046750
void FUN_10046750(void)
{
	STUB(0x10046750);
}

// Scales p_value by mode p_mode: 1 by 1.5, 3 by 0.75, any other mode leaves it.
// FUNCTION: MW2 0x100472fe
MechS32 FUN_100472fe(MechS32 p_mode, MechS32 p_value)
{
	MechS32 result;

	switch (p_mode) {
	case 1:
		result = (p_value >> 1) + p_value;
		break;
	case 2:
		result = p_value;
		break;
	case 3:
		result = p_value - (p_value >> 2);
		break;
	default:
		result = p_value;
		break;
	}

	return result;
}

// FUNCTION: MW2 0x10047462
MechS32 FUN_10047462(void)
{
	return g_unk0x100a6d70;
}

// FUNCTION: MW2 0x10047477
MechS32 FUN_10047477(void)
{
	return 0x2c;
}
