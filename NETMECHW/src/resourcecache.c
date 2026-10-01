#include "resourcecache.h"

#include "compat.h"
#include "decomp.h"
#include "prjfile.h"
#include "types.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NETMECHW.DLL's copy of the shell's resourcecache.c, the same source but where noted.

// A cached resource, followed by its data. DumpResourceCache prints the fields as "ID", "Type"
// and "Lock". Unlocked entries sit on the purge list, oldest first.
// SIZE 0x14
typedef struct ResourceCacheEntry {
	MechS16 m_id;                           // 0x00
	MechS16 m_lock;                         // 0x02
	undefined4 m_type;                      // 0x04 — the four-character type tag
	struct ResourceCacheEntry* m_next;      // 0x08 — in the g_cacheTable bucket
	struct ResourceCacheEntry* m_purgeNext; // 0x0c — toward g_purgeListTail
	struct ResourceCacheEntry* m_purgePrev; // 0x10
} ResourceCacheEntry;

void FreeCacheEntry(ResourceCacheEntry* p_entry);

// GLOBAL: NETMECHW 0x10023bb4
ResourceCacheEntry** g_cacheTable = NULL;

// The number of the next dump DumpResourceCache writes.
// GLOBAL: NETMECHW 0x10023bb8
MechS32 g_cacheDumpNumber = 0;

// The number of entries.
// GLOBAL: NETMECHW 0x1001f514
MechS32 g_cacheEntryCount;

// GLOBAL: NETMECHW 0x1001f50c
ResourceCacheEntry* g_purgeListHead;

// GLOBAL: NETMECHW 0x1001f510
ResourceCacheEntry* g_purgeListTail;

// FUNCTION: NETMECHW 0x10012a80
void UnlockCacheEntry(ResourceCacheEntry* p_entry)
{
	if (p_entry->m_lock == 0) {
		return;
	}

	p_entry->m_lock = 0;
	if (g_purgeListTail != NULL) {
		g_purgeListTail->m_purgeNext = p_entry;
	}
	p_entry->m_purgePrev = g_purgeListTail;
	g_purgeListTail = p_entry;
	p_entry->m_purgeNext = NULL;
	if (g_purgeListHead == NULL) {
		g_purgeListHead = p_entry;
	}
}

// Operand order: the original compares p_entry against g_purgeListHead and g_purgeListTail with
// the globals loaded first; it follows the unit's symbol table.
// FUNCTION: NETMECHW 0x10012af3
void LockCacheEntry(ResourceCacheEntry* p_entry)
{
	if (p_entry->m_lock == 1) {
		return;
	}

	p_entry->m_lock = 1;
	if (p_entry->m_purgeNext != NULL) {
		p_entry->m_purgeNext->m_purgePrev = p_entry->m_purgePrev;
	}
	if (p_entry->m_purgePrev != NULL) {
		p_entry->m_purgePrev->m_purgeNext = p_entry->m_purgeNext;
	}
	if (p_entry == g_purgeListHead) {
		g_purgeListHead = p_entry->m_purgeNext;
	}
	if (p_entry == g_purgeListTail) {
		g_purgeListTail = p_entry->m_purgePrev;
	}
	p_entry->m_purgeNext = NULL;
	p_entry->m_purgePrev = NULL;
}

// FUNCTION: NETMECHW 0x10012b9a
void AllocateCacheTable(void)
{
	g_cacheTable = (ResourceCacheEntry**) calloc(0x3f1, 4);
}

// Stack-slot permutation (VC++ 2.2): i and entry.
// FUNCTION: NETMECHW 0x10012bb9
void RebuildPurgeList(void)
{
	MechS32 i;
	ResourceCacheEntry* entry;

	g_purgeListHead = NULL;
	g_purgeListTail = NULL;
	for (i = 0; i < 0x3f1; i++) {
		for (entry = g_cacheTable[i]; entry != NULL; entry = entry->m_next) {
			if (entry->m_lock == 0) {
				entry->m_lock = 1;
				UnlockCacheEntry(entry);
			}
		}
	}
}

// Stack-slot permutation (VC++ 2.2): i, entry and next.
// FUNCTION: NETMECHW 0x10012c4c
void ShutdownResourceCache(void)
{
	MechS32 i;
	ResourceCacheEntry* entry;
	ResourceCacheEntry* next;

	if (g_cacheTable == NULL) {
		return;
	}

	for (i = 0; i < 0x3f1; i++) {
		for (entry = g_cacheTable[i]; entry != NULL; entry = next) {
			next = entry->m_next;
			FreeCacheEntry(entry);
		}
	}
	g_cacheEntryCount = 0;
	g_purgeListHead = NULL;
	g_purgeListTail = NULL;
	free(g_cacheTable);
}

// FUNCTION: NETMECHW 0x10012cf7
void InitializeResourceCache(void)
{
	g_cacheEntryCount = 0;
	g_purgeListHead = 0;
	g_purgeListTail = 0;
	AllocateCacheTable();
}

// Empty and never called: there is nothing to name it after.
// FUNCTION: NETMECHW 0x10012d25
void FUN_10013935(void)
{
}

// Stack-slot permutation (VC++ 2.2): bucket and entry.
// FUNCTION: NETMECHW 0x10012d30
ResourceCacheEntry* FindCacheEntry(MechS32 p_id, char* p_type)
{
	MechS32 bucket;
	ResourceCacheEntry* entry;

	if (p_id < 0) {
		return NULL;
	}

	bucket = ((MechS8) p_type[3] + (MechS8) p_type[2] + (MechS8) p_type[0] + (MechS8) p_type[1] + p_id) % 0x3f1;
	for (entry = g_cacheTable[bucket]; entry != NULL; entry = entry->m_next) {
		if (entry->m_id == p_id && entry->m_type == *(undefined4*) p_type) {
			break;
		}
	}

	return entry;
}

// Stack-slot permutation (VC++ 2.2): bucket, entry and type.
// FUNCTION: NETMECHW 0x10012dd7
void FreeCacheEntry(ResourceCacheEntry* p_entry)
{
	MechS32 bucket;
	ResourceCacheEntry* entry;
	MechChar type[5];

	entry = NULL;
	if (p_entry == NULL) {
		return;
	}

	p_entry->m_lock = 0;
	LockCacheEntry(p_entry);
	type[4] = '\0';
	*(undefined4*) type = p_entry->m_type;
	bucket = ((MechS8) type[2] + (MechS8) type[3] + (MechS8) type[0] + (MechS8) type[1] + p_entry->m_id) % 0x3f1;
	if (g_cacheTable[bucket] == p_entry) {
		g_cacheTable[bucket] = p_entry->m_next;
	}
	else {
		for (entry = g_cacheTable[bucket]; entry != NULL && entry->m_next != NULL; entry = entry->m_next) {
			if (entry->m_next == p_entry) {
				break;
			}
		}
		if (entry != NULL && entry->m_next != NULL) {
			entry->m_next = entry->m_next->m_next;
		}
		else {
			return;
		}
	}

	free(p_entry);
	g_cacheEntryCount--;
}

// Writes the cache to dbugcch<n>.log: every entry by bucket, then the purge list.
// Stack-slot permutation (VC++ 2.2): entry and file.
// FUNCTION: NETMECHW 0x10012f01
void DumpResourceCache(void)
{
	MechChar name[100];
	ResourceCacheEntry* entry;
	MechChar type[5];
	MechS32 i;
	FILE* file;

	sprintf(name, "dbugcch%d.log", g_cacheDumpNumber++);
	file = fopen(name, "w");
	type[4] = '\0';
	fprintf(file, "Cache table\n-----------------------\n");
	for (i = 0; i < 0x3f1; i++) {
		for (entry = g_cacheTable[i]; entry != NULL; entry = entry->m_next) {
			*(undefined4*) type = entry->m_type;
			fprintf(file, "ID=%5d  Type=%4s  Lock=%d  Size=%7d\n", entry->m_id, type, entry->m_lock, _msize(entry));
		}
	}

	fprintf(file, "\nPurge list\n-----------------------\n");
	for (entry = g_purgeListHead; entry != NULL; entry = entry->m_purgeNext) {
		*(undefined4*) type = entry->m_type;
		fprintf(file, "ID=%5d  Type=%4s  Lock=%d  Size=%7d\n", entry->m_id, type, entry->m_lock, _msize(entry));
	}

	fclose(file);
}

// Empty and never called: there is nothing to name it after.
// FUNCTION: NETMECHW 0x1001305e
void FUN_10013c6e(void)
{
}

// FUNCTION: NETMECHW 0x10013069
void UnlockCachedResource(MechS32 p_id, char* p_type)
{
	ResourceCacheEntry* entry = FindCacheEntry(p_id, p_type);
	if (entry == NULL) {
		return;
	}

	UnlockCacheEntry(entry);
}

// Returns the data of p_type item p_id of p_handle, loading it into the cache when needed and
// purging the least recently used entries to make room. p_unk0x0c is ignored (the name tables
// pass 1), so nothing gives it a name.
// Stack-slot permutation (VC++ 2.2): entry, bucket, size, log and file. Operand order: the
// original adds the type bytes in source order (p_type[2] first); VC++ 2.2 loads them from
// p_type[0] up, where the shell's copy keeps the source order under VC++ 4.1.
// FUNCTION: NETMECHW 0x100130a5
void* LoadCachedResource(MechS32 p_handle, MechS32 p_id, char* p_type, MechS32 p_unk0x0c)
{
	ResourceCacheEntry* entry;
	ResourceCacheEntry* block;
	MechS32 bucket;
	MechS32 size;
	FILE* log;
	FILE* file;

	if (p_id < 0) {
		return NULL;
	}

	entry = FindCacheEntry(p_id, p_type);
	if (entry != NULL) {
		LockCacheEntry(entry);
		return entry + 1;
	}

	if (g_cacheEntryCount >= 1000) {
		if (g_purgeListHead != NULL) {
			FreeCacheEntry(g_purgeListHead);
		}
		else {
			return NULL;
		}
	}

	size = GetArchiveItemSize(p_handle, p_type, p_id);
	if (size <= 0) {
		log = fopen("symlog.txt", "a");
		if (log != NULL) {
			fprintf(log, "Couldn't load ID=%d Type=%s\n", p_id, p_type);
		}
		fclose(log);
		return NULL;
	}

	while ((block = (ResourceCacheEntry*) malloc(size + sizeof(ResourceCacheEntry))) == NULL) {
		if (g_purgeListHead != NULL) {
			FreeCacheEntry(g_purgeListHead);
		}
		else {
			return NULL;
		}
	}

	if (ReadArchiveItem(p_handle, p_type, p_id, block + 1) == -1) {
		file = fopen("symlog.txt", "a");
		if (file != NULL) {
			fprintf(file, "Couldn't load ID=%d Type=%s\n", p_id, p_type);
		}
		fclose(file);
		return NULL;
	}

	bucket = ((MechS8) p_type[2] + (MechS8) p_type[3] + (MechS8) p_type[0] + (MechS8) p_type[1] + p_id) % 0x3f1;
	block->m_next = g_cacheTable[bucket];
	g_cacheTable[bucket] = block;
	block->m_lock = 1;
	block->m_id = p_id;
	block->m_type = *(undefined4*) p_type;
	block->m_purgeNext = NULL;
	block->m_purgePrev = NULL;
	g_cacheEntryCount++;
	return block + 1;
}

// FUNCTION: NETMECHW 0x100132e4
void FreeCachedResource(MechS32 p_id, char* p_type)
{
	ResourceCacheEntry* entry = FindCacheEntry(p_id, p_type);
	if (entry == NULL) {
		return;
	}

	FreeCacheEntry(entry);
}

// Empty and never called: there is nothing to name it after.
// FUNCTION: NETMECHW 0x10013320
void FUN_10013f30(void)
{
}

// Returns its argument. Never called, and the body gives no name.
// FUNCTION: NETMECHW 0x1001332b
undefined4 FUN_10013f3b(undefined4 p_value)
{
	return p_value;
}

// LoadCachedResource from archive handle 0. Never called; it keeps its placeholder because what
// handle 0 stands for here isn't known.
// FUNCTION: NETMECHW 0x1001333e
void* FUN_10013f4e(MechS32 p_id, char* p_type)
{
	return LoadCachedResource(0, p_id, p_type, 0);
}

// FUNCTION: NETMECHW 0x10013362
MechS32 PurgeOldestCacheEntry(void)
{
	if (g_purgeListHead != NULL) {
		FreeCacheEntry(g_purgeListHead);
		return 1;
	}

	return 0;
}

// Zeroed, with calloc.
// FUNCTION: NETMECHW 0x10013399
void* AllocateMemory(undefined4 p_size)
{
	return calloc(p_size, 1);
}

// CopyMemoryFast and FillMemoryFast (both unused) are memcpy and memset written as inline __asm: rep movsd/stosd
// for the dwords, then rep movsb/stosb for the rest. Modern compilers (COMPAT_MODE) call the CRT.

// FUNCTION: NETMECHW 0x100133b7
void* CopyMemoryFast(void* p_destination, void* p_source, MechU32 p_size)
{
#ifdef COMPAT_MODE
	memcpy(p_destination, p_source, p_size);
#else
	__asm {
		mov eax, p_size
		mov edi, p_destination
		mov esi, p_source
		mov ecx, eax
		shr ecx, 2
		rep movsd
		mov ecx, eax
		and ecx, 3
		rep movsb
	}
#endif

	return p_destination;
}

// The shell's copy builds the fill word with one more instruction (mov cl, al; mov ch, cl).
// FUNCTION: NETMECHW 0x100133e1
void* FillMemoryFast(void* p_destination, MechS32 p_value, MechU32 p_size)
{
#ifdef COMPAT_MODE
	memset(p_destination, p_value, p_size);
#else
	__asm {
		mov edx, p_size
		mov eax, p_value
		mov ch, al
		mov cl, al
		mov eax, ecx
		shl eax, 16
		mov ax, cx
		mov edi, p_destination
		mov ecx, edx
		shr ecx, 2
		rep stosd
		mov ecx, edx
		and ecx, 3
		rep stosb
	}
#endif

	return p_destination;
}

// Unused.
// FUNCTION: NETMECHW 0x10013417
void FreeMemory(void* p_buffer)
{
	free(p_buffer);
}
