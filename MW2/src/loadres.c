/* Hand-written assembly: MemCopy and MemSet are C functions whose bodies are an __asm block
   (rep movsd/stosd). */
#include "loadres.h"

#include "decomp.h"
#include "simmain.h"
#include "types.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(CacheItem, 0x14)

// The resource cache's hash chains, 0x3f1 of them.
// GLOBAL: MW2 0x100a2c5c
CacheItem** g_cacheTable = NULL;

// GLOBAL: MW2 0x101748d0
MechS32 g_cacheItemCount;

// The purge list: unlocked items, oldest first.
// GLOBAL: MW2 0x101748d4
CacheItem* g_purgeHead;

// GLOBAL: MW2 0x101748d8
CacheItem* g_purgeTail;
// Unlocks an item and appends it to the purge list.
// FUNCTION: MW2 0x10019af0
void FUN_10019af0(CacheItem* p_item)
{
	if (p_item->m_lock == 0) {
		return;
	}

	p_item->m_lock = 0;
	if (g_purgeTail) {
		g_purgeTail->m_purgeNext = p_item;
	}

	p_item->m_purgePrev = g_purgeTail;
	g_purgeTail = p_item;
	p_item->m_purgeNext = NULL;
	if (!g_purgeHead) {
		g_purgeHead = p_item;
	}
}

// Locks an item and takes it off the purge list.
// The two list-end comparisons load their operands in the other order (the unit's symbol table).
// FUNCTION: MW2 0x10019b63
void FUN_10019b63(CacheItem* p_item)
{
	if (p_item->m_lock == 1) {
		return;
	}

	p_item->m_lock = 1;
	if (p_item->m_purgeNext) {
		p_item->m_purgeNext->m_purgePrev = p_item->m_purgePrev;
	}

	if (p_item->m_purgePrev) {
		p_item->m_purgePrev->m_purgeNext = p_item->m_purgeNext;
	}

	if (g_purgeHead == p_item) {
		g_purgeHead = p_item->m_purgeNext;
	}

	if (g_purgeTail == p_item) {
		g_purgeTail = p_item->m_purgePrev;
	}

	p_item->m_purgeNext = NULL;
	p_item->m_purgePrev = NULL;
}

// FUNCTION: MW2 0x10019c0c
void FUN_10019c0c(void)
{
	g_cacheTable = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 0x3f1 * sizeof(CacheItem*));
}

// Rebuilds the purge list from the unlocked items.
// FUNCTION: MW2 0x10019c2f
void FUN_10019c2f(void)
{
	CacheItem* item;
	MechS32 i;

	g_purgeHead = NULL;
	g_purgeTail = NULL;
	for (i = 0; i < 0x3f1; i++) {
		for (item = g_cacheTable[i]; item; item = item->m_next) {
			if (item->m_lock == 0) {
				item->m_lock = 1;
				FUN_10019af0(item);
			}
		}
	}
}

// Frees every cached item and the hash table.
// FUNCTION: MW2 0x10019cc2
void FUN_10019cc2(void)
{
	CacheItem* item;
	MechS32 i;
	CacheItem* next;

	if (!g_cacheTable) {
		return;
	}

	for (i = 0; i < 0x3f1; i++) {
		for (item = g_cacheTable[i]; item; item = next) {
			next = item->m_next;
			FUN_10019e53(item);
		}
	}

	g_cacheItemCount = 0;
	g_purgeHead = NULL;
	g_purgeTail = NULL;
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_cacheTable);
}

// FUNCTION: MW2 0x10019d73
void FUN_10019d73(void)
{
	g_cacheItemCount = 0;
	g_purgeHead = NULL;
	g_purgeTail = NULL;
	FUN_10019c0c();
}

// FUNCTION: MW2 0x10019da1
void FUN_10019da1(void)
{
}

// Returns the cached item of an ID and type, or NULL.
// The only diff is a stack-slot permutation of item and slot.
// FUNCTION: MW2 0x10019dac
CacheItem* FUN_10019dac(MechS32 p_id, const char* p_type)
{
	CacheItem* item;
	MechS32 slot;

	if (p_id < 0) {
		return NULL;
	}

	slot = (p_type[2] + p_type[3] + p_type[0] + p_type[1] + p_id) % 0x3f1;
	for (item = g_cacheTable[slot]; item; item = item->m_next) {
		if (item->m_id == p_id && item->m_type == *(MechS32*) p_type) {
			break;
		}
	}

	return item;
}

// STUB: MW2 0x10019e53
void FUN_10019e53(CacheItem* p_item)
{
	STUB(0x10019e53);
}

// FUNCTION: MW2 0x1001a158
void FUN_1001a158(void)
{
}

// FUNCTION: MW2 0x1001a163
void FUN_1001a163(MechS32 p_id, const char* p_type)
{
	CacheItem* entry;

	entry = FUN_10019dac(p_id, p_type);
	if (entry == NULL) {
		return;
	}

	FUN_10019af0(entry);
}

// STUB: MW2 0x1001a19f
void* FUN_1001a19f(undefined4 p_unk0x00, MechS32 p_unk0x04, const char* p_unk0x08, undefined4 p_unk0x0c)
{
	STUB(0x1001a19f);
	return NULL;
}

// FUNCTION: MW2 0x1001a4e5
void FUN_1001a4e5(MechS32 p_id, const char* p_type)
{
	CacheItem* entry;

	entry = FUN_10019dac(p_id, p_type);
	if (entry == NULL) {
		return;
	}

	FUN_10019e53(entry);
}

// FUNCTION: MW2 0x1001a521
void FUN_1001a521(void)
{
}

// FUNCTION: MW2 0x1001a52c
undefined4 FUN_1001a52c(undefined4 p_unk0x00)
{
	return p_unk0x00;
}

// FUNCTION: MW2 0x1001a53f
void* FUN_1001a53f(MechS32 p_id, const char* p_type)
{
	return FUN_1001a19f(0, p_id, p_type, 0);
}

// FUNCTION: MW2 0x1001a563
MechS32 FUN_1001a563(void)
{
	if (g_purgeHead) {
		FUN_10019e53(g_purgeHead);
		return 1;
	}

	return 0;
}

// FUNCTION: MW2 0x1001a59a
void* MemAlloc(MechU32 p_size)
{
	return HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, p_size);
}

// FUNCTION: MW2 0x1001a5bc
void* MemCopy(void* p_dst, const void* p_src, MechU32 p_size)
{
	__asm {
		mov eax, p_size
		mov edi, p_dst
		mov esi, p_src
		mov ecx, eax
		shr ecx, 2
		rep movsd
		mov ecx, eax
		and ecx, 3
		rep movsb
	}

	return p_dst;
}

// FUNCTION: MW2 0x1001a5e6
void* MemSet(void* p_dst, MechS32 p_value, MechU32 p_size)
{
	__asm {
		mov edx, p_size
		mov eax, p_value
		mov cl, al
		mov ch, cl
		mov cl, al
		mov eax, ecx
		shl eax, 16
		mov ax, cx
		mov edi, p_dst
		mov ecx, edx
		shr ecx, 2
		rep stosd
		mov ecx, edx
		and ecx, 3
		rep stosb
	}

	return p_dst;
}

// FUNCTION: MW2 0x1001a61e
void MemFree(void* p_block)
{
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
}
