#include "unk100696c0.h"

#include "decomp.h"
#include "types.h"

// Returns the sine of p_angle (16.16 degrees) from the table, as 16.16.
// STUB: MW2 0x100696c0
MechS32 FUN_100696c0(MechS32 p_angle)
{
	STUB(0x100696c0);
	return 0;
}

// Returns the cosine of p_angle (16.16 degrees): the sine 90 degrees on.
// FUNCTION: MW2 0x1006973a
MechS32 FUN_1006973a(MechS32 p_angle)
{
	return FUN_100696c0(p_angle + 0x5a0000);
}

// STUB: MW2 0x1006975b
MechS32 FUN_1006975b(MechS32 p_unk0x00)
{
	STUB(0x1006975b);
	return 0;
}

// STUB: MW2 0x100698de
MechS32 FUN_100698de(MechS32 p_x, MechS32 p_z)
{
	STUB(0x100698de);
	return 0;
}
