#include "geocache.h"

#include "decomp.h"
#include "error.h"
#include "loadres.h"
#include "simmain.h"
#include "soundconfig.h"
#include "types.h"
#include "unk1001ce90.h"
#include "unk100563d0.h"

DECOMP_SIZE_ASSERT(GeoClass, 0x08)

// GLOBAL: MW2 0x100a3850
MechS32* g_unk0x100a3850 = NULL;

// GLOBAL: MW2 0x100a3854
MechS32* g_unk0x100a3854 = NULL;

// GLOBAL: MW2 0x100a3860
MechS32* g_unk0x100a3860 = NULL;

// GLOBAL: MW2 0x100a3864
MechS32* g_unk0x100a3864 = NULL;

// GLOBAL: MW2 0x100a3868
MechS32 g_classCount = 0;

// GLOBAL: MW2 0x100a386c
MechS32 g_classCapacity = 0;

// GLOBAL: MW2 0x100a3870
GeoClass* g_classes = NULL;

// GLOBAL: MW2 0x100a3874
MechS32 g_unk0x100a3874 = 0;

// GLOBAL: MW2 0x100a3878
undefined4 g_unk0x100a3878 = 0;

// The block BeginBlock opened last, -1: none.
// GLOBAL: MW2 0x100a3880
MechS32 g_currentBlock = -1;

// GLOBAL: MW2 0x100a3884
MechS32 g_blockDepth = 0;

// GLOBAL: MW2 0x100a38d4
MechS32 g_explosionChunks = 1;

// GLOBAL: MW2 0x1012b7b4
MechS32 g_unk0x1012b7b4;

// GLOBAL: MW2 0x1012b7b0
MechS32 g_unk0x1012b7b0;

// The blocks BeginBlock opened.
// GLOBAL: MW2 0x1012b730
MechS32 g_blockStack[32];

// GLOBAL: MW2 0x1010b610
MechS32 g_unk0x1010b610;

// GLOBAL: MW2 0x1010b6a0
MechS32 g_unk0x1010b6a0;

// GLOBAL: MW2 0x1010b6a8
MechS32 g_unk0x1010b6a8;

void FUN_1001f5cb(void);
void FUN_1001feef(MechS32 p_index);
void FUN_1001ffda(void);
void FUN_100200bd(void);
void FUN_100204e8(void);

// Allocates the class, thing and star tables to the sizes in the mission's static memory table.
// Returns whether it could.
// FUNCTION: MW2 0x1001f3e0
MechS32 FUN_1001f3e0(void)
{
	MechU32 size;
	MechS32 result;

	result = 0;
	FUN_1001f5cb();
	size = GetStaticPoolSize(7);
	if (size) {
		g_classCapacity = size >> 3;
		g_classes = MemAlloc(size);
		if (g_classes) {
			size = GetStaticPoolSize(8);
			if (size) {
				g_unk0x1010b6a0 = size >> 2;
				g_unk0x1012b7b4 = g_unk0x1010b6a0;
				g_unk0x100a3860 = MemAlloc(size);
				if (g_unk0x100a3860) {
					g_unk0x100a3850 = MemAlloc(size);
					if (g_unk0x100a3850) {
						size = GetStaticPoolSize(9);
						if (size) {
							g_unk0x100a3864 = MemAlloc(size);
							if (g_unk0x100a3864) {
								g_unk0x100a3854 = MemAlloc(size);
								if (g_unk0x100a3854) {
									result = 1;
								}
							}
						}
					}
				}
			}
		}
	}

	return result;
}

// Adds a class to the class table. Returns whether there was room.
// FUNCTION: MW2 0x1001f504
MechS32 FUN_1001f504(MechS32 p_id, undefined4 p_class)
{
	MechS32 result;

	result = 0;
	if (g_classCount < g_classCapacity) {
		g_classes[g_classCount].m_id = p_id;
		g_classes[g_classCount].m_class = p_class;
		g_classCount++;
		result = 1;
	}

	return result;
}

// Returns the class of an ID (the last one added), or 0.
// FUNCTION: MW2 0x1001f564
undefined4 FindClassById(MechS32 p_id)
{
	undefined4 result;
	MechS32 i;

	result = 0;
	for (i = g_classCount - 1; i >= 0; i--) {
		if (g_classes[i].m_id == p_id) {
			result = g_classes[i].m_class;
			break;
		}
	}

	return result;
}

// STUB: MW2 0x1001f5cb
void FUN_1001f5cb(void)
{
	STUB(0x1001f5cb);
}

// Returns the thing index of an ID (the last one listed), or -1.
// FUNCTION: MW2 0x1001f6e9
MechS32 FindThingIdxById(MechS32 p_id)
{
	MechS32 i;
	MechS32 result;

	result = -1;
	for (i = 0; i < g_unk0x1012b7b0; i++) {
		if (g_unk0x100a3854[i] == p_id) {
			result = g_unk0x100a3850[i];
		}
	}

	return result;
}

// FUNCTION: MW2 0x1001f74c
void UpdateGeoCache(void)
{
	if (g_unk0x1010b610) {
		FUN_100204e8();
		g_unk0x1010b610 = 0;
	}

	FUN_1001da44();
	FUN_100200bd();
}

// Returns the star index of an ID (the last one listed), or -1.
// The loop test compares in the other operand order (the unit's symbol table).
// FUNCTION: MW2 0x1001f77d
MechS32 FindStarIdxById(MechS32 p_id)
{
	MechS32 i;
	MechS32 result;

	result = -1;
	if (p_id >= 0) {
		for (i = 0; i < g_unk0x1010b6a8; i++) {
			if (g_unk0x100a3864[i] == p_id) {
				result = g_unk0x100a3860[i];
			}
		}
	}

	return result;
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

// FUNCTION: MW2 0x1001fe41
void HandleElseBlock(void)
{
}

// Closes the block BeginBlock opened last.
// FUNCTION: MW2 0x1001fe4c
void EndBlock(void)
{
	g_blockDepth--;
	if (g_blockDepth <= -1) {
		Error(0x22, NULL);
	}
	else {
		g_currentBlock = g_blockStack[g_blockDepth];
	}
}

// STUB: MW2 0x1001feef
void FUN_1001feef(MechS32 p_index)
{
	STUB(0x1001feef);
}

// FUNCTION: MW2 0x1001ffda
void FUN_1001ffda(void)
{
	MechS32 i;

	for (i = 0; i < 0x402; i++) {
		FUN_1001feef(i);
	}

	g_unk0x100a3874 = 0;
	g_unk0x100a3878 = 1;
}

// STUB: MW2 0x10020029
void FirstStaticCache(void)
{
	STUB(0x10020029);
}

// STUB: MW2 0x100200bd
void FUN_100200bd(void)
{
	STUB(0x100200bd);
}

// STUB: MW2 0x100204e8
void FUN_100204e8(void)
{
	STUB(0x100204e8);
}

// STUB: MW2 0x10020c6f
void FUN_10020c6f(MechS32 p_id, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	STUB(0x10020c6f);
}

// FUNCTION: MW2 0x10021438
void SetExplosionChunks(undefined4 p_unk0x00, MechS32 p_explosionChunks)
{
	g_explosionChunks = p_explosionChunks;
	g_mw2SndCfgData->m_explosionChunks = p_explosionChunks;
}
