#ifndef LOADRES_H
#define LOADRES_H

#include "decomp.h"
#include "types.h"

// The header of a cached resource: its ID, type and lock count (the cache log prints
// "ID %5d Type %4s Lock %d Size %7d"), its hash chain in g_cacheTable and its place in the
// purge list of unlocked items. The resource data follows.
// SIZE 0x14
typedef struct CacheItem {
	MechS16 m_id;                  // 0x00
	MechS16 m_lock;                // 0x02
	MechS32 m_type;                // 0x04 — four characters
	struct CacheItem* m_next;      // 0x08
	struct CacheItem* m_purgeNext; // 0x0c
	struct CacheItem* m_purgePrev; // 0x10
} CacheItem;

// The functions and globals of loadres.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10019af0(CacheItem* p_item);
	void FUN_10019b63(CacheItem* p_item);
	void FUN_10019c0c(void);
	void FUN_10019c2f(void);
	void FUN_10019cc2(void);
	void FUN_10019d73(void);
	void FUN_10019da1(void);
	CacheItem* FUN_10019dac(MechS32 p_id, const char* p_type);
	void FUN_10019e53(CacheItem* p_item);
	void FUN_1001a158(void);
	void FUN_1001a163(MechS32 p_id, const char* p_type);
	void* FUN_1001a19f(undefined4 p_unk0x00, MechS32 p_unk0x04, const char* p_unk0x08, undefined4 p_unk0x0c);
	void FUN_1001a4e5(MechS32 p_id, const char* p_type);
	void FUN_1001a521(void);
	undefined4 FUN_1001a52c(undefined4 p_unk0x00);
	void* FUN_1001a53f(MechS32 p_id, const char* p_type);
	MechS32 FUN_1001a563(void);
	void* MemAlloc(MechU32 p_size);
	void* MemCopy(void* p_dst, const void* p_src, MechU32 p_size);
	void* MemSet(void* p_dst, MechS32 p_value, MechU32 p_size);
	void MemFree(void* p_block);

#ifdef __cplusplus
}
#endif

#endif // LOADRES_H
