#include "geocache.h"

#include "audio.h"
#include "bwdblockrecord.h"
#include "callbacks.h"
#include "classtable.h"
#include "collision.h"
#include "debris.h"
#include "decomp.h"
#include "error.h"
#include "gamething.h"
#include "loadres.h"
#include "mw2prj.h"
#include "object.h"
#include "players.h"
#include "poolsizes.h"
#include "prjfile.h"
#include "quadtree.h"
#include "resourcename.h"
#include "resourceref.h"
#include "shape.h"
#include "shapegeom.h"
#include "shapelists.h"
#include "simmain.h"
#include "soundconfig.h"
#include "staticblock.h"
#include "staticobject.h"
#include "transform.h"
#include "types.h"
#include "wtbshapes.h"
#include "xform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(GeoClass, 0x08)
DECOMP_SIZE_ASSERT(StaticObject, 0x7c)
DECOMP_SIZE_ASSERT(StaticBlock, 0x7c)
DECOMP_SIZE_ASSERT(Xform, 0x24)

// GLOBAL: MW2 0x100a3850
MechS32* g_unk0x100a3850 = NULL;

// GLOBAL: MW2 0x100a3854
MechS32* g_unk0x100a3854 = NULL;

// Set inside a world stream's repeated section (REPR to ENDR); g_unk0x100a385c counts its passes.
// GLOBAL: MW2 0x100a3858
MechS32 g_unk0x100a3858 = 0;

// GLOBAL: MW2 0x100a385c
MechS32 g_unk0x100a385c = 0;

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
MechS32 g_nextBlock = 0;

// The block BeginBlock opened last, -1: none.
// GLOBAL: MW2 0x100a3880
MechS32 g_currentBlock = -1;

// GLOBAL: MW2 0x100a3884
MechS32 g_blockDepth = 0;

// The transform the next block opens with (ApplyBlockXform).
// GLOBAL: MW2 0x100a3888
Xform g_pendingXform = {1, 1, 1, 0, 0, 0, 0, 0, 0};

// The transform BeginBlock resets g_pendingXform to.
// GLOBAL: MW2 0x100a38b0
Xform g_defaultXform = {1, 1, 1, 0, 0, 0, 0, 0, 0};

// GLOBAL: MW2 0x100a38d4
MechS32 g_explosionChunks = 1;

// GLOBAL: MW2 0x1010b6b0
StaticBlock g_staticBlocks[32];

// GLOBAL: MW2 0x1010c630
StaticObject g_staticObjects[0x402];

// GLOBAL: MW2 0x1012b7b4
MechS32 g_unk0x1012b7b4;

// GLOBAL: MW2 0x1012b7b0
MechS32 g_unk0x1012b7b0;

// The blocks BeginBlock opened.
// GLOBAL: MW2 0x1012b730
MechS32 g_blockStack[32];

// GLOBAL: MW2 0x1010b610
MechS32 g_unk0x1010b610;

// The shapes FUN_10020d51 shows for the blocks' boxes.
// GLOBAL: MW2 0x1010b620
Shape* g_unk0x1010b620[32];

// GLOBAL: MW2 0x1010b6a0
MechS32 g_unk0x1010b6a0;

// Whether FUN_10020d51 shows the blocks' boxes.
// GLOBAL: MW2 0x1010b6a4
MechS32 g_unk0x1010b6a4;

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
MechS32 FUN_1001f504(MechS32 p_id, Shape* p_class)
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

// Returns the class of an ID (the last one added), or NULL.
// FUNCTION: MW2 0x1001f564
Shape* FindClassById(MechS32 p_id)
{
	Shape* result;
	MechS32 i;

	result = NULL;
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
		ResetStaticCache();
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
Shape** GetStaticShapeSlot(MechS32 p_index)
{
	return &g_staticObjects[p_index].m_shape;
}

// FUNCTION: MW2 0x1001f894
Shape* GetStaticShape(MechS32 p_index)
{
	return g_staticObjects[p_index].m_shape;
}

// Places static object p_id: its resource, transform, block and parent, and the object it hangs
// from (none for a shape-only entry, -2). Returns its index, or -1 if p_id isn't cached.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1001f8b5
MechS32 FUN_1001f8b5(
	MechS32 p_id,
	MechS32 p_resource,
	Xform p_xform,
	MechS32 p_block,
	MechS32 p_parent,
	MechS32 p_unk0x3c,
	MechU32 p_flags,
	MechU32 p_kind,
	undefined4 p_unk0x48
)
{
	MechS32 index;
	StaticObject* entry;
	Matrix matrix;

	index = FindStarIdxById(p_id);
	if (index == -1) {
		index = FindObjIdxById(p_id);
	}

	if (index != -1) {
		entry = &g_staticObjects[index];
		BuildMatrix(
			&matrix,
			p_xform.m_angleX,
			p_xform.m_angleY,
			p_xform.m_angleZ,
			p_xform.m_x,
			p_xform.m_y,
			p_xform.m_z
		);
		CopyMatrix(&matrix, &entry->m_matrix);
		entry->m_xform = p_xform;
		entry->m_resource = p_resource;
		entry->m_parent = p_parent;
		entry->m_block = p_block;
		entry->m_shape = NULL;
		if (p_parent == -2) {
			entry->m_object = NULL;
		}
		else if (p_parent == -1) {
			entry->m_object = FUN_100012d0(NULL, 10);
		}
		else {
			entry->m_object = FUN_100012d0(g_staticObjects[p_parent].m_object, 10);
		}

		entry->m_flags |= p_flags & 0x1ff;
		entry->m_flags |= (p_kind << 12) & 0xf000;
		entry->m_unk0x10 = p_unk0x48;
		FUN_100204e8();
	}

	return index;
}

// Opens the next block (g_nextBlock) inside the current one: its box, from a corner and a
// size, its center, and the matrix that places its objects, from the pending transform
// (g_pendingXform, reset afterwards) and every enclosing block's matrix. The outermost block
// also clears the placed-object list.
// Stack-slot permutation; i < g_unk0x1012b7b4 compares in the other operand order, and the second
// center sum adds its operands in the other order.
// FUNCTION: MW2 0x1001fa05
void BeginBlock(BwdBlockRecord* p_record)
{
	StaticBlock* block;
	Matrix* matrix;
	Matrix scale;
	MechS32 i;
	StaticBlock* parent;
	Matrix translate;
	Matrix rotation;

	if (g_blockDepth == 0) {
		g_unk0x1010b6a8 = 0;
		for (i = 0; i < g_unk0x1012b7b4; i++) {
			g_unk0x100a3860[i] = -1;
		}

		g_blockStack[g_blockDepth] = -1;
	}

	if (g_nextBlock < 32 && g_nextBlock >= 0) {
		g_blockDepth++;
		g_blockStack[g_blockDepth] = g_nextBlock;
		block = &g_staticBlocks[g_nextBlock];
		block->m_minX = p_record->m_origin[0];
		block->m_minY = p_record->m_origin[1];
		block->m_minZ = p_record->m_origin[2];
		block->m_maxX = p_record->m_size[0];
		block->m_maxY = p_record->m_size[1];
		block->m_maxZ = p_record->m_size[2];
		block->m_parent = g_blockStack[g_blockDepth - 1];
		block->m_xform = g_pendingXform;
		if (block->m_maxX < 0) {
			block->m_minX += block->m_maxX;
			block->m_maxX = -block->m_maxX;
		}

		block->m_maxX += block->m_minX;
		if (block->m_maxY < 0) {
			block->m_minY += block->m_maxY;
			block->m_maxY = -block->m_maxY;
		}

		block->m_maxY += block->m_minY;
		if (block->m_maxZ < 0) {
			block->m_minZ += block->m_maxZ;
			block->m_maxZ = -block->m_maxZ;
		}

		block->m_maxZ += block->m_minZ;
		block->m_centerX = (block->m_maxX + block->m_minX) / 2;
		block->m_centerY = (block->m_maxY + block->m_minY) / 2;
		block->m_centerZ = (block->m_maxZ + block->m_minZ) / 2;
		matrix = &block->m_matrix;
		SetIdentityMatrix(&rotation);
		rotation.m_rows[0][0] = block->m_xform.m_scaleX << 29;
		rotation.m_rows[1][1] = block->m_xform.m_scaleY << 29;
		rotation.m_rows[2][2] = block->m_xform.m_scaleZ << 29;
		BuildMatrix(&translate, 0, 0, 0, -block->m_centerX, -block->m_centerY, -block->m_centerZ);
		BuildMatrix(&scale, block->m_xform.m_angleX, block->m_xform.m_angleY, block->m_xform.m_angleZ, 0, 0, 0);
		MultiplyMatrix(&scale, &translate, matrix);
		BuildMatrix(&translate, 0, 0, 0, block->m_xform.m_x, block->m_xform.m_y, block->m_xform.m_z);
		MultiplyMatrix(&translate, matrix, matrix);
		BuildMatrix(&translate, 0, 0, 0, block->m_centerX, block->m_centerY, block->m_centerZ);
		MultiplyMatrix(&translate, matrix, matrix);
		parent = block;
		while (parent->m_parent != -1) {
			parent = &g_staticBlocks[parent->m_parent];
			MultiplyMatrix(&parent->m_matrix, matrix, matrix);
		}

		g_pendingXform = g_defaultXform;
		g_currentBlock = g_nextBlock;
		g_nextBlock++;
	}
}

// FUNCTION: MW2 0x1001fe41
void HandleElseBlock(void)
{
}

// Closes the block BeginBlock opened last.
// FUNCTION: MW2 0x1001fe4c
void EndBlock(struct BwdStream* p_stream)
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
void ApplyBlockXform(Xform p_xform)
{
	g_pendingXform = p_xform;
}

// Transforms a point by the current block's matrix.
// FUNCTION: MW2 0x1001fea6
void TransformBlockPoint(MechS32* p_point)
{
	if (g_currentBlock != -1) {
		TransformPoint(&g_staticBlocks[g_currentBlock].m_matrix, p_point, p_point + 1, p_point + 2);
	}
}

// Resets a static object cache entry.
// FUNCTION: MW2 0x1001feef
void ResetStaticObject(MechS32 p_index)
{
	StaticObject* entry;

	entry = &g_staticObjects[p_index];
	entry->m_resource = -1;
	entry->m_block = -1;
	entry->m_parent = -1;
	entry->m_flags = 0;
	entry->m_unk0x10 = 0;
	entry->m_replacement = -1;
	entry->m_thing = -1;
	entry->m_shape = NULL;
	entry->m_object = NULL;
	entry->m_xform.m_scaleX = entry->m_xform.m_scaleY = entry->m_xform.m_scaleZ = 1;
	entry->m_xform.m_angleX = entry->m_xform.m_angleY = entry->m_xform.m_angleZ = 0;
	entry->m_xform.m_x = entry->m_xform.m_y = entry->m_xform.m_z = 0;
	entry->m_callback = NULL;
}

// FUNCTION: MW2 0x1001ffda
void ResetStaticCache(void)
{
	MechS32 i;

	for (i = 0; i < 0x402; i++) {
		ResetStaticObject(i);
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
	StaticObject* entry;

	for (i = 0; i < g_unk0x100a3874; i++) {
		entry = &g_staticObjects[i];
		FUN_10020704(i, entry->m_block);
	}
}

// FUNCTION: MW2 0x10020080
void AttachTaskToObj(MechS32 p_index, TimedCallbackFn p_fn, MechS32 p_period, MechChar* p_data)
{
	StaticObject* entry;

	entry = &g_staticObjects[p_index];
	CreateDetachedTask(&entry->m_callback, p_fn, p_period, p_data);
}

// Runs the timed callbacks of the cache entries.
// The only diff is a stack-slot permutation of i and entry.
// FUNCTION: MW2 0x100200bd
void FUN_100200bd(void)
{
	MechS32 i;
	StaticObject* entry;

	for (i = 0; i < g_unk0x100a3874; i++) {
		entry = &g_staticObjects[i];
		if (!(entry->m_flags & 0x800) && entry->m_callback) {
			RunTimedCallbacks(&entry->m_callback);
		}
	}
}

// FUNCTION: MW2 0x1002012a
void FUN_1002012a(MechS32 p_index, TimedCallback* p_callback)
{
	StaticObject* entry;

	entry = &g_staticObjects[p_index];
	FUN_1007d3bf(&entry->m_callback, p_callback);
}

// FUNCTION: MW2 0x1002015f
void FUN_1002015f(MechS32 p_index)
{
	StaticObject* entry;

	entry = &g_staticObjects[p_index];
	FUN_1007d475(&entry->m_callback);
}

// FUNCTION: MW2 0x10020190
void FUN_10020190(MechS32 p_index)
{
	StaticObject* entry;

	entry = &g_staticObjects[p_index];
	FUN_1007d4dc(&entry->m_callback);
}

// FUNCTION: MW2 0x100201c1
void FUN_100201c1(MechS32 p_index, MechU32 p_unk0x0c)
{
	StaticObject* entry;

	entry = &g_staticObjects[p_index];
	entry->m_flags &= ~0xf000;
	entry->m_flags |= (p_unk0x0c << 12) & 0xf000;
}

// Links cache entry p_index to its game thing and to the entry that replaces it when the thing is
// destroyed (FUN_10020292), which stays unloaded (0x800) until then.
// Stack-slot permutation; p_index >= g_unk0x100a3874 compares in the other operand order.
// FUNCTION: MW2 0x100201fe
void FUN_100201fe(MechS32 p_index, MechS32 p_replacement, MechS32 p_thing)
{
	StaticObject* replacement;
	StaticObject* entry;

	if (p_index < 0 || p_index >= g_unk0x100a3874) {
		return;
	}

	entry = &g_staticObjects[p_index];
	entry->m_flags |= 0x400;
	entry->m_unk0x10 |= 0x200;
	entry->m_replacement = (MechS16) p_replacement;
	entry->m_thing = (MechS16) p_thing;
	if (p_replacement > -1) {
		replacement = &g_staticObjects[p_replacement];
		replacement->m_flags |= 0x800;
		FUN_100204e8();
	}
}

// Destroys the game thing of cache entry p_index: marks both (0x200 and 0x800, 4), frees its shape
// and loads the entry that replaces it (m_replacement) where the old object was. Returns the
// replacement's index, or -1.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10020292
MechS32 FUN_10020292(MechS32 p_index)
{
	Matrix matrix;
	MechS32 replacement;
	struct SceneObject* obj;
	StaticObject* entry;
	Shape* shape;
	GameThing* thing;

	obj = NULL;
	entry = &g_staticObjects[p_index];
	if (entry->m_thing == -1) {
		return -1;
	}

	thing = &g_gameThings[entry->m_thing];
	entry->m_flags |= 0x200;
	entry->m_flags |= 0x800;
	thing->m_unk0x00 |= 4;
	thing->m_unk0x04 = -1;
	FUN_1002015f(p_index);
	shape = entry->m_shape;
	if (shape) {
		obj = entry->m_object;
		if (obj) {
			matrix = *FUN_10001e01(obj);
		}

		FUN_1003b78b(shape);
		entry->m_shape = NULL;
		entry->m_object = NULL;
	}

	replacement = entry->m_replacement;
	if (replacement != -1) {
		entry = &g_staticObjects[replacement];
		entry->m_flags &= ~0x800;
		if (FUN_10020704(replacement, entry->m_block)) {
			obj = entry->m_object;
			if (obj) {
				FUN_10001de6(obj, &matrix);
			}

			FUN_10020190(replacement);
			if (obj) {
				FUN_10001c3f(obj);
			}
		}
		else {
			entry->m_flags &= 0x800;
			replacement = -1;
		}
	}

	return replacement;
}

// Marks p_thing's cache entry (flag 0x200) and has FUN_10020292 replace it. When the replacement's
// game thing has flag 0x8000, its object tree is handed to FUN_100044f3 and FUN_10021314.
// Stack-slot permutation: index, current and entry.
// FUNCTION: MW2 0x10020429
void FUN_10020429(GameThing* p_thing)
{
	MechS32 index;
	MechS32 current;
	GameThing* thing;
	StaticObject* entry;

	index = -1;
	entry = &g_staticObjects[p_thing->m_unk0x04];
	entry->m_flags |= 0x200;
	current = p_thing->m_unk0x04;
	if (current != -1) {
		index = FUN_10020292(current);
	}

	if (index != -1) {
		thing = &g_gameThings[g_staticObjects[index].m_thing];
		if (thing->m_unk0x00 & 0x8000) {
			FUN_100044f3(g_staticObjects[index].m_object, FUN_100213cf, 0);
			FUN_10021314(index);
		}
	}

	FUN_100204e8();
}

// Propagates the parents' states to the cache entries hanging from them: a hidden parent (0x200)
// hides the entry (FUN_10020292); a parent flagged 0x800 flags the entry and unloads it
// (FUN_10020b95), and an entry whose parent no longer is is loaded again (FUN_10020704), its game
// thing's object marked when the thing has bit 0x8000.
// Stack-slot permutation of the locals. Operand order: the loop test (i < g_unk0x100a3874)
// loads g_unk0x100a3874 first in the original.
// FUNCTION: MW2 0x100204e8
void FUN_100204e8(void)
{
	GameThing* thing;
	MechS32 i;
	MechS32 index;
	StaticObject* entry;

	for (i = 0; i < g_unk0x100a3874; i++) {
		entry = &g_staticObjects[i];
		if (entry->m_parent > -1) {
			if (g_staticObjects[entry->m_parent].m_flags & 0x200 && !(entry->m_flags & 0x200)) {
				g_unk0x1010b610 = 1;
				FUN_10020292(i);
			}

			if (g_staticObjects[entry->m_parent].m_flags & 0x800) {
				if (!(entry->m_flags & 0x800)) {
					g_unk0x1010b610 = 1;
					entry->m_flags |= 0x800;
					FUN_10020b95(i);
				}
			}
			else if (entry->m_flags & 0x800 && !(entry->m_flags & 0x200)) {
				g_unk0x1010b610 = 1;
				entry->m_flags &= ~0x800;
				if (FUN_10020704(i, entry->m_block)) {
					index = g_staticObjects[i].m_thing;
					if (index != -1) {
						thing = &g_gameThings[index];
						if (thing->m_unk0x00 & 0x8000) {
							FUN_100044f3(g_staticObjects[i].m_object, FUN_100213cf, 0);
							FUN_10021314(i);
						}
					}
				}
				else {
					entry->m_flags &= 0x800;
				}
			}
		}
	}
}

// Returns whether the cache is full: no entry is free (resource -1).
// FUNCTION: MW2 0x10020684
MechS32 FUN_10020684(void)
{
	MechS32 i;
	MechS32 found;

	found = FALSE;
	for (i = 0; i < g_unk0x100a3874 && !found; i++) {
		if (g_staticObjects[i].m_resource == -1) {
			found = TRUE;
		}
	}

	if (found) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

// Loads cache entry p_index's shape and places it: a shape-only entry (-2) in block p_block's
// frame, otherwise on its object. Returns TRUE while the shape is loaded, FALSE if it can't be or
// shouldn't be: a hidden entry (0x800), or a destroyed game thing's (without explosion chunks).
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10020704
MechS32 FUN_10020704(MechS32 p_index, MechS32 p_block)
{
	MechS32 size;
	MechS32 offset;
	MechS32 thing;
	MechU8* data;
	struct SceneObject* parent;
	MechU32 kind;
	StaticObject* entry;
	Matrix matrix;

	size = 0;
	offset = 0;
	entry = &g_staticObjects[p_index];
	if (entry->m_flags & 0x800) {
		return FALSE;
	}

	if (entry->m_resource == -1) {
		return FALSE;
	}

	thing = entry->m_thing;
	if (thing != -1 && (g_gameThings[thing].m_unk0x00 & 0x8000) && (!g_explosionChunks || FUN_100040b0())) {
		return FALSE;
	}

	if (entry->m_shape && !(FUN_1003acf7(entry->m_shape) & 0x400)) {
		return TRUE;
	}

	data = FUN_1001a19f(g_mw2PrjHandle, entry->m_resource, g_resourceTypeTags[c_resTagPoly], 0);
	if (data) {
		size = GetPrjResourceSize(g_mw2PrjHandle, g_resourceTypeTags[c_resTagPoly], entry->m_resource);
	}
	else {
		return FALSE;
	}

	SetShapeScale(entry->m_xform.m_scaleX, entry->m_xform.m_scaleY, entry->m_xform.m_scaleZ);
	SetShapeFlags(entry->m_flags & 0x1ff);
	parent = NULL;
	if (entry->m_parent > -1) {
		parent = g_staticObjects[entry->m_parent].m_object;
	}

	if (entry->m_thing != -1) {
		g_unk0x100bfd40 = 1;
		g_unk0x100bfd44 = 0x200;
		g_unk0x100bfd48 = entry->m_thing;
	}

	entry->m_shape = LoadShapes(data, &offset, size, parent);
	g_unk0x100bfd40 = 0;
	FUN_1001a163(entry->m_resource, g_resourceTypeTags[c_resTagPoly]);
	if (!entry->m_shape) {
		return FALSE;
	}

	kind = (entry->m_flags & 0xf000) >> 12;
	if (entry->m_parent == -2) {
		MultiplyMatrix(&g_staticBlocks[p_block].m_matrix, &entry->m_matrix, &matrix);
		TransformShape(entry->m_shape, &matrix);
		entry->m_object = NULL;
		if (entry->m_flags & 0x400) {
			FUN_1003ad2d(entry->m_shape, entry->m_unk0x10);
			FUN_1003ad62(entry->m_shape, entry->m_thing);
		}
		else {
			FUN_1003ad2d(entry->m_shape, entry->m_unk0x10);
			FUN_1003ad62(entry->m_shape, p_index);
		}

		FUN_10034a40(entry->m_shape, kind);
		FUN_1006d732(entry->m_shape);
	}
	else if (entry->m_object) {
		FUN_10001532(entry->m_object, entry->m_shape);
		SetShapeObject(entry->m_shape, entry->m_object);
		FUN_1006d732(entry->m_shape);
		if (entry->m_parent == -1) {
			MultiplyMatrix(&g_staticBlocks[p_block].m_matrix, &entry->m_matrix, &matrix);
			FUN_10001694(entry->m_object, &matrix);
			UpdateObj(entry->m_object);
		}
		else {
			SetObjRotation(
				entry->m_object,
				entry->m_xform.m_angleX,
				entry->m_xform.m_angleY,
				entry->m_xform.m_angleZ,
				0
			);
			SetObjPosition(entry->m_object, entry->m_xform.m_x, entry->m_xform.m_y, entry->m_xform.m_z);
			UpdateObj(entry->m_object);
		}

		if (entry->m_flags & 0x400) {
			SetObjTreeFlag(entry->m_object, entry->m_unk0x10);
			FUN_10001b0c(entry->m_object, entry->m_thing);
		}
		else {
			FUN_1003ad2d(entry->m_shape, entry->m_unk0x10);
			FUN_1003ad62(entry->m_shape, p_index);
		}

		FUN_10001b6a(entry->m_object, kind);
	}
	else {
		return FALSE;
	}

	if (kind == 5) {
		FUN_1001df00(entry->m_shape);
	}

	return TRUE;
}

// Frees a cache entry's shape.
// FUNCTION: MW2 0x10020b95
void FUN_10020b95(MechS32 p_index)
{
	StaticObject* entry;

	entry = &g_staticObjects[p_index];
	if (entry->m_shape) {
		FUN_1003b78b(entry->m_shape);
		entry->m_shape = NULL;
	}
}

// FUNCTION: MW2 0x10020bdd
struct SceneObject* FUN_10020bdd(MechS32 p_index)
{
	struct SceneObject* result;

	result = NULL;
	if (p_index < g_unk0x100a3874 && p_index >= 0) {
		result = g_staticObjects[p_index].m_object;
	}

	return result;
}

// FUNCTION: MW2 0x10020c26
Shape* FUN_10020c26(MechS32 p_index)
{
	Shape* result;

	result = NULL;
	if (p_index < g_unk0x100a3874 && p_index >= 0) {
		result = g_staticObjects[p_index].m_shape;
	}

	return result;
}

// Returns a cache entry's position: its shape's, or its object's. A bad index (the test lets
// g_unk0x100a3874 itself through) or an empty entry gives 0, 0, 0.
// Stack-slot permutation: obj and entry. Operand order: p_index > g_unk0x100a3874 loads
// g_unk0x100a3874 first in the original.
// FUNCTION: MW2 0x10020c6f
void FUN_10020c6f(MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	struct SceneObject* obj;
	StaticObject* entry;
	Shape* shape;

	if (p_index > g_unk0x100a3874 || p_index < 0) {
		*p_x = *p_y = *p_z = 0;
	}
	else {
		entry = &g_staticObjects[p_index];
		shape = entry->m_shape;
		if (shape) {
			*p_x = shape->m_centerX;
			*p_y = shape->m_centerY;
			*p_z = shape->m_centerZ;
		}
		else {
			obj = entry->m_object;
			if (obj) {
				GetObjPosition(obj, p_x, p_y, p_z);
			}
			else {
				*p_x = *p_y = *p_z = 0;
			}
		}
	}
}

// Shows the static blocks' boxes (the "unitbox" model scaled to each), or frees them when they are
// shown. Returns whether any is shown.
// Stack-slot permutation; the loop tests compare in the other operand order.
// FUNCTION: MW2 0x10020d51
MechS32 FUN_10020d51(void)
{
	MechS32 result;
	MechS32 size;
	ResourceRef* ref;
	MechS32 dz;
	MechS32 dy;
	MechS32 dx;
	MechS32 scaleZ;
	MechS32 scaleY;
	MechS32 scaleX;
	MechS32 offset;
	Shape* shape;
	StaticBlock* block;
	MechS32 i;
	ResourceRef local;
	Matrix matrix;
	MechU8* data;
	FILE* file;

	result = FALSE;
	if (!g_unk0x1010b6a4) {
		g_unk0x1010b6a4 = TRUE;
		ref = &local;
		ref->m_id = -1;
		strncpy(ref->m_name, "unitbox", 12);
		ref->m_name[12] = '\0';
		data =
			FUN_10073922(ref, g_resourceTypeTags[c_resTagPoly], g_resourceTypeExtensions[c_resExtWtb], 1, &size, NULL);
		if (data) {
			for (i = 0; i < g_nextBlock; i++) {
				block = &g_staticBlocks[i];
				if (block->m_maxX == block->m_minX) {
					continue;
				}

				dx = block->m_maxX - block->m_minX;
				dy = block->m_maxY - block->m_minY;
				dz = block->m_maxZ - block->m_minZ;
				dx = abs(dx);
				dy = abs(dy);
				dz = abs(dz);
				scaleX = (dx + 9) / 10;
				scaleY = (dy + 9) / 10;
				scaleZ = (dz + 9) / 10;
				SetShapeScale(scaleX, scaleY, scaleZ);
				SetShapeFlags(0);
				offset = 0;
				g_unk0x1010b620[i] = LoadShapes(data, &offset, size, NULL);
				shape = g_unk0x1010b620[i];
				if (shape) {
					result = TRUE;
					FUN_10034a40(shape, 4);
					BuildMatrix(&matrix, 0, 0, 0, block->m_centerX, block->m_centerY, block->m_centerZ);
					MultiplyMatrix(&block->m_matrix, &matrix, &matrix);
					TransformShape(shape, &matrix);
					FUN_1006d732(shape);
				}
			}

			if (ref->m_id == -1) {
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
			}
			else {
				FUN_1001a163(ref->m_id, g_resourceTypeTags[c_resTagPoly]);
			}
		}
		else {
			file = fopen("symlog.txt", "a");
			if (file) {
				fprintf(file, "Couldn't load ID=%s Type=%s\n", ref->m_name, g_resourceTypeTags[c_resTagPoly]);
			}

			fclose(file);
		}
	}
	else {
		g_unk0x1010b6a4 = FALSE;
		result = TRUE;
		for (i = 0; i < g_nextBlock; i++) {
			if (g_unk0x1010b620[i]) {
				FUN_1003b78b(g_unk0x1010b620[i]);
				g_unk0x1010b620[i] = NULL;
			}
		}
	}

	return result;
}

// Loads the "unitbox" model into every box of the tree p_root.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10021067
void FUN_10021067(QuadtreeNode* p_root)
{
	MechS32 size;
	ResourceRef* ref;
	ResourceRef local;
	MechU8* data;
	FILE* file;

	ref = &local;
	ref->m_id = -1;
	strncpy(ref->m_name, "unitbox", 12);
	ref->m_name[12] = '\0';
	data = FUN_10073922(ref, g_resourceTypeTags[c_resTagPoly], g_resourceTypeExtensions[c_resExtWtb], 1, &size, NULL);
	if (data) {
		FUN_1002116a(p_root, data, size);
		if (ref->m_id == -1) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
		}
		else {
			FUN_1001a163(ref->m_id, g_resourceTypeTags[c_resTagPoly]);
		}
	}
	else {
		file = fopen("symlog.txt", "a");
		if (file) {
			fprintf(file, "Couldn't load ID=%s Type=%s\n", ref->m_name, g_resourceTypeTags[c_resTagPoly]);
		}

		fclose(file);
	}
}

// Loads model p_data scaled to the box of p_node at its center, then to its children's.
// Stack-slot permutation; the original loads m_unk0x14 first in z's sum (commutative operand order).
// FUNCTION: MW2 0x1002116a
void FUN_1002116a(QuadtreeNode* p_node, MechU8* p_data, MechS32 p_size)
{
	MechS32 result;
	MechS32 dz;
	MechS32 dy;
	MechS32 dx;
	MechS32 scaleZ;
	MechS32 scaleY;
	MechS32 scaleX;
	MechS32 offset;
	MechS32 x;
	Shape* shape;
	MechS32 z;
	MechS32 i;
	MechS32 y;
	Matrix matrix;

	result = FALSE;
	if (!p_node) {
		return;
	}

	dx = p_node->m_unk0x04 - p_node->m_unk0x00;
	dy = p_node->m_unk0x0c - p_node->m_unk0x08;
	dz = p_node->m_unk0x14 - p_node->m_unk0x10;
	dx = abs(dx);
	dy = abs(dy);
	dz = abs(dz);
	scaleX = (dx + 9) / 10;
	scaleY = (dy * 2 + 18) / 10;
	scaleZ = (dz + 9) / 10;
	SetShapeScale(scaleX, scaleY, scaleZ);
	SetShapeFlags(4);
	offset = 0;
	shape = LoadShapes(p_data, &offset, p_size, NULL);
	if (shape) {
		result = TRUE;
		FUN_10034a40(shape, 4);
		x = (p_node->m_unk0x04 + p_node->m_unk0x00) >> 1;
		y = (p_node->m_unk0x08 + p_node->m_unk0x0c) >> 1;
		z = (p_node->m_unk0x14 + p_node->m_unk0x10) >> 1;
		BuildMatrix(&matrix, 0, 0, 0, x, y, z);
		TransformShape(shape, &matrix);
		FUN_1006d732(shape);
	}

	for (i = 0; i < 4; i++) {
		FUN_1002116a(p_node->m_children[i], p_data, p_size);
	}
}

// Frees cache entry p_index and, first, every entry hanging from it, zeroing its game thing.
// Returns whether p_index is an entry.
// Stack-slot permutation; the loop test compares in the other operand order.
// FUNCTION: MW2 0x10021314
MechS32 FUN_10021314(MechU32 p_index)
{
	MechS32 result;
	MechU32 thing;
	MechS32 i;
	StaticObject* entry;

	result = FALSE;
	for (i = 0; i < g_unk0x100a3874; i++) {
		if (g_staticObjects[i].m_parent == p_index) {
			FUN_10021314(i);
		}
	}

	if (p_index < 0x402) {
		entry = &g_staticObjects[p_index];
		thing = entry->m_thing;
		if (thing < 0xfe) {
			ZeroGameThing(thing);
		}

		ResetStaticObject(p_index);
		result = TRUE;
	}

	return result;
}

// Frees a scene object tree, and the shapes on it when the object has one.
// FUNCTION: MW2 0x100213cf
void FUN_100213cf(struct SceneObject* p_obj)
{
	ShapeCallback callback;
	Shape* shape;

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
MechS32 FUN_10021423(MechS32 p_arg)
{
	return g_explosionChunks;
}

// FUNCTION: MW2 0x10021438
void SetExplosionChunks(MechS32 p_arg, MechS32 p_explosionChunks)
{
	g_explosionChunks = p_explosionChunks;
	g_mw2SndCfgData->m_explosionChunks = p_explosionChunks;
}
