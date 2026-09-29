#include "unk10010750.h"

#include "decomp.h"
#include "palette.h"
#include "types.h"

// STUB: MW2 0x100107de
MechS32 FUN_100107de(
	undefined4 p_unk0x00,
	MechS32 p_unk0x04,
	undefined4* p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10
)
{
	STUB(0x100107de);
	return 0;
}

// FUNCTION: MW2 0x10010a7f
MechS32 FUN_10010a7f(undefined4 p_unk0x00, MechS32 p_unk0x04, undefined4* p_unk0x08)
{
	MechS32 result;

	result = FUN_100107de(p_unk0x00, p_unk0x04, p_unk0x08, 15, 0);
	if (result > 60) {
		StartPaletteFade(17, 181, 1);
	}

	return result;
}
