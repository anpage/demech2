#include "decomp.h"
#include "types.h"

// GLOBAL: MW2 0x100a3860
undefined4* g_unk0x100a3860 = NULL;

// GLOBAL: MW2 0x100a3864
undefined4* g_unk0x100a3864 = NULL;

// GLOBAL: MW2 0x100a3874
MechS32 g_unk0x100a3874 = 0;

// GLOBAL: MW2 0x100a3878
undefined4 g_unk0x100a3878 = 0;

// GLOBAL: MW2 0x1012b7b4
MechS32 g_unk0x1012b7b4;

// GLOBAL: MW2 0x1010b6a8
MechS32 g_unk0x1010b6a8;

void FUN_1001ffda(void);

// STUB: MW2 0x1001f74c
void UpdateGeoCache(void)
{
	STUB(0x1001f74c);
}

// FUNCTION: MW2 0x1001f7eb
MechS32 FindObjIdxById(undefined4 p_unk0x08)
{
	MechS32 result = -1;

	if (!g_unk0x100a3878) {
		FUN_1001ffda();
	}

	if (g_unk0x100a3874 < 0x402 && g_unk0x1012b7b4 > g_unk0x1010b6a8) {
		result = g_unk0x100a3874;
		g_unk0x100a3860[g_unk0x1010b6a8] = result;
		g_unk0x100a3864[g_unk0x1010b6a8] = p_unk0x08;
		g_unk0x100a3874++;
		g_unk0x1010b6a8++;
	}

	return result;
}

// STUB: MW2 0x1001ffda
void FUN_1001ffda(void)
{
	STUB(0x1001ffda);
}

// STUB: MW2 0x10020029
void FirstStaticCache(void)
{
	STUB(0x10020029);
}
