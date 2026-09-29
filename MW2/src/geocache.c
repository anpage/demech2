#include "geocache.h"

#include "callbacks.h"
#include "decomp.h"
#include "error.h"
#include "hollowspire.h"
#include "loadres.h"
#include "object.h"
#include "quietmarsh.h"
#include "simmain.h"
#include "soundconfig.h"
#include "transform.h"
#include "twilightgrove.h"
#include "types.h"
#include "unk1001ce90.h"
#include "unk1003a530.h"
#include "unk100563d0.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(GeoClass, 0x08)
DECOMP_SIZE_ASSERT(HollowSpire0x7c, 0x7c)
DECOMP_SIZE_ASSERT(QuietMarsh0x7c, 0x7c)
DECOMP_SIZE_ASSERT(TwilightGrove0x24, 0x24)

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

// The block BeginBlock opens next.
// GLOBAL: MW2 0x100a387c
MechS32 g_unk0x100a387c = 0;

// The block BeginBlock opened last, -1: none.
// GLOBAL: MW2 0x100a3880
MechS32 g_currentBlock = -1;

// GLOBAL: MW2 0x100a3884
MechS32 g_blockDepth = 0;

// The transform the next block opens with (ApplyBlockXform).
// GLOBAL: MW2 0x100a3888
TwilightGrove0x24 g_unk0x100a3888 = {1, 1, 1, 0, 0, 0, 0, 0, 0};

// The transform BeginBlock resets g_unk0x100a3888 to.
// GLOBAL: MW2 0x100a38b0
TwilightGrove0x24 g_unk0x100a38b0 = {1, 1, 1, 0, 0, 0, 0, 0, 0};

// GLOBAL: MW2 0x100a38d4
MechS32 g_explosionChunks = 1;

// GLOBAL: MW2 0x1010b6b0
QuietMarsh0x7c g_unk0x1010b6b0[32];

// GLOBAL: MW2 0x1010c630
HollowSpire0x7c g_unk0x1010c630[0x402];

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

// Frees the class, thing and star tables.
// FUNCTION: MW2 0x1001f5cb
void FUN_1001f5cb(void)
{
	if (g_classes) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_classes);
		g_classes = NULL;
	}

	if (g_unk0x100a3860) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a3860);
		g_unk0x100a3860 = NULL;
	}

	if (g_unk0x100a3850) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a3850);
		g_unk0x100a3850 = NULL;
	}

	if (g_unk0x100a3864) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a3864);
		g_unk0x100a3864 = NULL;
	}

	if (g_unk0x100a3854) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_unk0x100a3854);
		g_unk0x100a3854 = NULL;
	}

	g_classCount = 0;
	g_classCapacity = 0;
	g_unk0x1012b7b4 = g_unk0x1010b6a0 = 0;
	g_unk0x1010b6a8 = g_unk0x1012b7b0 = 0;
}

// Returns the thing index of an ID (the last one listed), or -1.
// The loop test compares in the other operand order (the unit's symbol table).
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

// FUNCTION: MW2 0x1001f873
ScarletOrchid0x4c** FUN_1001f873(MechS32 p_index)
{
	return &g_unk0x1010c630[p_index].m_unk0x1c;
}

// FUNCTION: MW2 0x1001f894
ScarletOrchid0x4c* FUN_1001f894(MechS32 p_index)
{
	return g_unk0x1010c630[p_index].m_unk0x1c;
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

// Sets the transform the next block opens with.
// FUNCTION: MW2 0x1001fe8c
void ApplyBlockXform(TwilightGrove0x24 p_xform)
{
	g_unk0x100a3888 = p_xform;
}

// Transforms a point by the current block's matrix.
// FUNCTION: MW2 0x1001fea6
void FUN_1001fea6(MechS32* p_point)
{
	if (g_currentBlock != -1) {
		FUN_1000d650(&g_unk0x1010b6b0[g_currentBlock].m_unk0x48, p_point, p_point + 1, p_point + 2);
	}
}

// Resets a static object cache entry.
// FUNCTION: MW2 0x1001feef
void FUN_1001feef(MechS32 p_index)
{
	HollowSpire0x7c* entry;

	entry = &g_unk0x1010c630[p_index];
	entry->m_unk0x00 = -1;
	entry->m_unk0x04 = -1;
	entry->m_unk0x08 = -1;
	entry->m_unk0x0c = 0;
	entry->m_unk0x10 = 0;
	entry->m_unk0x14 = -1;
	entry->m_unk0x18 = -1;
	entry->m_unk0x1c = NULL;
	entry->m_unk0x20 = NULL;
	entry->m_unk0x24 = entry->m_unk0x28 = entry->m_unk0x2c = 1;
	entry->m_unk0x30 = entry->m_unk0x34 = entry->m_unk0x38 = 0;
	entry->m_unk0x3c = entry->m_unk0x40 = entry->m_unk0x44 = 0;
	entry->m_unk0x78 = NULL;
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

// The loop test compares in the other operand order (the unit's symbol table), and i and entry
// sit in permuted stack slots.
// FUNCTION: MW2 0x10020029
void FirstStaticCache(void)
{
	MechS32 i;
	HollowSpire0x7c* entry;

	for (i = 0; i < g_unk0x100a3874; i++) {
		entry = &g_unk0x1010c630[i];
		FUN_10020704(i, entry->m_unk0x04);
	}
}

// FUNCTION: MW2 0x10020080
void AttachTaskToObj(MechS32 p_index, TimedCallbackFn p_fn, MechS32 p_period, undefined4 p_data)
{
	HollowSpire0x7c* entry;

	entry = &g_unk0x1010c630[p_index];
	CreateDetachedTask(&entry->m_unk0x78, p_fn, p_period, p_data);
}

// Runs the timed callbacks of the cache entries.
// The only diff is a stack-slot permutation of i and entry.
// FUNCTION: MW2 0x100200bd
void FUN_100200bd(void)
{
	MechS32 i;
	HollowSpire0x7c* entry;

	for (i = 0; i < g_unk0x100a3874; i++) {
		entry = &g_unk0x1010c630[i];
		if (!(entry->m_unk0x0c & 0x800) && entry->m_unk0x78) {
			RunTimedCallbacks(&entry->m_unk0x78);
		}
	}
}

// FUNCTION: MW2 0x1002012a
void FUN_1002012a(MechS32 p_index, TimedCallback* p_callback)
{
	HollowSpire0x7c* entry;

	entry = &g_unk0x1010c630[p_index];
	FUN_1007d3bf(&entry->m_unk0x78, p_callback);
}

// FUNCTION: MW2 0x1002015f
void FUN_1002015f(MechS32 p_index)
{
	HollowSpire0x7c* entry;

	entry = &g_unk0x1010c630[p_index];
	FUN_1007d475(&entry->m_unk0x78);
}

// FUNCTION: MW2 0x10020190
void FUN_10020190(MechS32 p_index)
{
	HollowSpire0x7c* entry;

	entry = &g_unk0x1010c630[p_index];
	FUN_1007d4dc(&entry->m_unk0x78);
}

// FUNCTION: MW2 0x100201c1
void FUN_100201c1(MechS32 p_index, MechU32 p_unk0x0c)
{
	HollowSpire0x7c* entry;

	entry = &g_unk0x1010c630[p_index];
	entry->m_unk0x0c &= ~0xf000;
	entry->m_unk0x0c |= (p_unk0x0c << 12) & 0xf000;
}

// STUB: MW2 0x100204e8
void FUN_100204e8(void)
{
	STUB(0x100204e8);
}

// STUB: MW2 0x10020704
undefined4 FUN_10020704(MechS32 p_index, MechS32 p_block)
{
	STUB(0x10020704);
	return 0;
}

// Frees a cache entry's shape.
// FUNCTION: MW2 0x10020b95
void FUN_10020b95(MechS32 p_index)
{
	HollowSpire0x7c* entry;

	entry = &g_unk0x1010c630[p_index];
	if (entry->m_unk0x1c) {
		FUN_1003b78b(entry->m_unk0x1c);
		entry->m_unk0x1c = NULL;
	}
}

// FUNCTION: MW2 0x10020bdd
struct AmberWillow0x7c* FUN_10020bdd(MechS32 p_index)
{
	struct AmberWillow0x7c* result;

	result = NULL;
	if (p_index < g_unk0x100a3874 && p_index >= 0) {
		result = g_unk0x1010c630[p_index].m_unk0x20;
	}

	return result;
}

// FUNCTION: MW2 0x10020c26
ScarletOrchid0x4c* FUN_10020c26(MechS32 p_index)
{
	ScarletOrchid0x4c* result;

	result = NULL;
	if (p_index < g_unk0x100a3874 && p_index >= 0) {
		result = g_unk0x1010c630[p_index].m_unk0x1c;
	}

	return result;
}

// STUB: MW2 0x10020c6f
void FUN_10020c6f(MechS32 p_id, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	STUB(0x10020c6f);
}

// Frees a scene object tree, and the shapes on it when the object has one.
// FUNCTION: MW2 0x100213cf
void FUN_100213cf(struct AmberWillow0x7c* p_obj)
{
	ShapeCallback callback;
	ScarletOrchid0x4c* shape;

	callback = NULL;
	if (!p_obj) {
		return;
	}

	shape = FUN_1000154d(p_obj);
	if (shape) {
		callback = FUN_1003b78b;
	}

	FUN_10001f82(p_obj, callback);
}

// FUNCTION: MW2 0x10021423
MechS32 FUN_10021423(void)
{
	return g_explosionChunks;
}

// FUNCTION: MW2 0x10021438
void SetExplosionChunks(undefined4 p_unk0x00, MechS32 p_explosionChunks)
{
	g_explosionChunks = p_explosionChunks;
	g_mw2SndCfgData->m_explosionChunks = p_explosionChunks;
}
