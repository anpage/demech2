// resource.h stays out of this unit: its declarations ahead of FUN_1005005e change that
// function's operand order (see there).
#include "ai.h"
#include "config.h"
#include "decomp.h"
#include "error.h"
#include "loadres.h"
#include "players.h"
#include "prjfile.h"
#include "simmain.h"
#include "types.h"
#include "unk10044740.h"
#include "unk1006f480.h"

#include <mbstring.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

// GLOBAL: MW2 0x100a8608
void* g_unk0x100a8608 = NULL;

// GLOBAL: MW2 0x100a860c
void* g_unk0x100a860c = NULL;

// GLOBAL: MW2 0x100ea500
void* g_unk0x100ea500[16];

// Frees the mission tables' blocks.
// FUNCTION: MW2 0x1004fd55
void FUN_1004fd55(void)
{
	MechS32 i;

	if (g_unk0x100a8608) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a8608);
	}

	g_unk0x100a8608 = NULL;
	for (i = 0; i < 16; i++) {
		if (g_unk0x100ea500[i]) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100ea500[i]);
		}

		g_unk0x100ea500[i] = NULL;
	}

	if (g_unk0x100a860c) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a860c);
	}

	g_unk0x100a860c = NULL;
}

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

void* FUN_100508c0(MechU32 p_size);
void FUN_100508dc(void* p_block);

// FUNCTION: MW2 0x10050780
MechS32 FirstResource(void)
{
	MechS32 result;

	result = TRUE;
	SetPrjAllocator(FUN_100508c0, FUN_100508dc);
	FUN_10019d73();
	if (!g_unk0x100a8744) {
		g_unk0x100a8744 = (MechChar*) _mbsdup((unsigned char*) BuildGamePath("mw2.prj"));
	}

	g_unk0x100a8740 = OpenPrjFile(g_unk0x100a8744, 0);
	if (g_unk0x100a8740 != -1) {
		LoadPrjIndexes(g_unk0x100a8740);
	}
	else {
		Error(3, "\nCan't find file \"%s\"", g_unk0x100a8744, 0);
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2 0x1005082f
void CloseResourceFile(void)
{
	ClosePrjFile(g_unk0x100a8740);
}

// FUNCTION: MW2 0x10050848
void CachePreloads(void)
{
	FUN_10045a5b();
	FUN_1006f480();
	LoadAIScripts();
}

// FUNCTION: MW2 0x10050862
MechS32 FUN_10050862(MechS32 p_id, const char* p_type)
{
	MechS32 result;
	void* data;

	data = FUN_1001a19f(g_unk0x100a8740, p_id, p_type, 0);
	if (data) {
		result = TRUE;
		FUN_1001a163(p_id, p_type);
	}
	else {
		result = FALSE;
	}

	return result;
}

// FUNCTION: MW2 0x100508c0
void* FUN_100508c0(MechU32 p_size)
{
	return MemAlloc(p_size);
}

// FUNCTION: MW2 0x100508dc
void FUN_100508dc(void* p_block)
{
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
}
