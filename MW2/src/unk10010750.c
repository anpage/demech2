#include "unk10010750.h"

#include "decomp.h"
#include "palette.h"
#include "simmain.h"
#include "types.h"

// Draws a polygon of p_count points in the mode in p_flags' low four bits, in the color in bits
// 4-11: modes 0 and 3 draw it filled (FUN_100107de) when the render settings allow.
// FUNCTION: MW2 0x10010750
void FUN_10010750(MechU32 p_flags, MechS32 p_count, MechU32* p_points, MechS32 p_unk0x0c)
{
	MechU32 color;
	MechU32 mode;

	mode = p_flags & 0xf;
	color = (p_flags & 0xff0) >> 4;
	switch (mode) {
	case 0:
	case 3:
		if (g_unk0x100a6cc8.m_unk0x10 & 1) {
			FUN_100107de(color, p_count, p_points, p_unk0x0c, 0);
		}
		break;
	case 1:
		break;
	default:
		break;
	}
}

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
