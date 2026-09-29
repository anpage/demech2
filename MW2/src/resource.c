// resource.h stays out of this unit: its declarations ahead of FUN_1005005e change that
// function's operand order (see there).
#include "decomp.h"
#include "error.h"
#include "players.h"
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

// GLOBAL: MW2 0x100a8628
MechS32 g_unk0x100a8628 = 0;

// The original loads p_id first; the operand order follows the symbol table.
// FUNCTION: MW2 0x100500c3
MechS32 MapResourceId(MechS32 p_id)
{
	return g_unk0x100a8628 + p_id;
}

// FUNCTION: MW2 0x100500dc
void SetMangleBase(MechS32 p_base)
{
	g_unk0x100a8628 = p_base;
}

// Returns the next free game thing index, or -1 when all 254 are taken.
// FUNCTION: MW2 0x1005072f
MechS32 FUN_1005072f(void)
{
	MechS32 index;

	index = -1;
	if (g_gameThingCount < 0xfe) {
		index = g_gameThingCount;
		g_gameThingCount++;
	}
	else {
		Error(0x2f, "Too many gamethings", 0);
	}

	return index;
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
