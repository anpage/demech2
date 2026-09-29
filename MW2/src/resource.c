// resource.h stays out of this unit: its declarations ahead of FUN_1005005e change that
// function's operand order (see there).
#include "decomp.h"
#include "types.h"

#include <stdarg.h>
#include <stdio.h>

// STUB: MW2 0x1004fe85
void FUN_1004fe85(
	MechFloat p_unk0x00,
	MechFloat p_unk0x04,
	MechFloat p_unk0x08,
	MechFloat p_unk0x0c,
	undefined4* p_unk0x10,
	undefined4* p_unk0x14,
	undefined4* p_unk0x18,
	undefined4* p_unk0x1c
)
{
	STUB(0x1004fe85);
}

/* The only diff is the order of the three products in d (and of each product's operands), which
   follows the symbol table, not the source. It matches with seven more symbols declared ahead of
   the function (placeholder prototypes do it); stubbing the object's other functions does not. */
// FUNCTION: MW2 0x1005005e
void FUN_1005005e(
	MechFloat p_x,
	MechFloat p_y,
	MechFloat p_z,
	MechFloat p_nx,
	MechFloat p_ny,
	MechFloat p_nz,
	undefined4* p_unk0x18,
	undefined4* p_unk0x1c,
	undefined4* p_unk0x20,
	undefined4* p_unk0x24
)
{
	MechFloat a;
	MechFloat b;
	MechFloat c;
	MechFloat d;

	a = p_nx;
	b = -p_ny;
	c = p_nz;
	d = -(p_x * a + p_y * b + p_z * c);
	FUN_1004fe85(a, b, c, d, p_unk0x18, p_unk0x1c, p_unk0x20, p_unk0x24);
}

// STUB: MW2 0x10050780
void FirstResource(void)
{
	STUB(0x10050780);
}

// STUB: MW2 0x1005082f
void CloseResourceFile(void)
{
	STUB(0x1005082f);
}

// STUB: MW2 0x10050848
void CachePreloads(void)
{
	STUB(0x10050848);
}
