#include "compat.h"
#include "decomp.h"
#include "types.h"

#include <stdlib.h>
#include <string.h>

typedef struct DrawCacheEntry {
	MechS16 m_unk0x00;
	MechS16 m_unk0x02;
	undefined4 m_unk0x04;
	struct DrawCacheEntry* m_unk0x08;
	struct DrawCacheEntry* m_unk0x0c;
	struct DrawCacheEntry* m_unk0x10;
} DrawCacheEntry;

void FUN_100139e7(DrawCacheEntry* p_entry);

// GLOBAL: MW2SHELL 0x10063a54
DrawCacheEntry** g_unk0x10063a54 = NULL;

// GLOBAL: MW2SHELL 0x10096860
undefined4 g_unk0x10096860;

// GLOBAL: MW2SHELL 0x10096864
DrawCacheEntry* g_unk0x10096864;

// GLOBAL: MW2SHELL 0x10096868
DrawCacheEntry* g_unk0x10096868;

// FUNCTION: MW2SHELL 0x10013690
void FUN_10013690(DrawCacheEntry* p_entry)
{
	if (p_entry->m_unk0x02 == 0) {
		return;
	}

	p_entry->m_unk0x02 = 0;
	if (g_unk0x10096868 != NULL) {
		g_unk0x10096868->m_unk0x0c = p_entry;
	}
	p_entry->m_unk0x10 = g_unk0x10096868;
	g_unk0x10096868 = p_entry;
	p_entry->m_unk0x0c = NULL;
	if (g_unk0x10096864 == NULL) {
		g_unk0x10096864 = p_entry;
	}
}

// FUNCTION: MW2SHELL 0x10013703
void FUN_10013703(DrawCacheEntry* p_entry)
{
	if (p_entry->m_unk0x02 == 1) {
		return;
	}

	p_entry->m_unk0x02 = 1;
	if (p_entry->m_unk0x0c != NULL) {
		p_entry->m_unk0x0c->m_unk0x10 = p_entry->m_unk0x10;
	}
	if (p_entry->m_unk0x10 != NULL) {
		p_entry->m_unk0x10->m_unk0x0c = p_entry->m_unk0x0c;
	}
	if (p_entry == g_unk0x10096864) {
		g_unk0x10096864 = p_entry->m_unk0x0c;
	}
	if (p_entry == g_unk0x10096868) {
		g_unk0x10096868 = p_entry->m_unk0x10;
	}
	p_entry->m_unk0x0c = NULL;
	p_entry->m_unk0x10 = NULL;
}

// FUNCTION: MW2SHELL 0x100137aa
void FUN_100137aa(void)
{
	g_unk0x10063a54 = (DrawCacheEntry**) calloc(0x3f1, 4);
}

// FUNCTION: MW2SHELL 0x100137c9
void FUN_100137c9(void)
{
	MechS32 i;
	DrawCacheEntry* entry;

	g_unk0x10096864 = NULL;
	g_unk0x10096868 = NULL;
	for (i = 0; i < 0x3f1; i++) {
		for (entry = g_unk0x10063a54[i]; entry != NULL; entry = entry->m_unk0x08) {
			if (entry->m_unk0x02 == 0) {
				entry->m_unk0x02 = 1;
				FUN_10013690(entry);
			}
		}
	}
}

// FUNCTION: MW2SHELL 0x1001385c
void FUN_1001385c(void)
{
	MechS32 i;
	DrawCacheEntry* entry;
	DrawCacheEntry* next;

	if (g_unk0x10063a54 == NULL) {
		return;
	}

	for (i = 0; i < 0x3f1; i++) {
		for (entry = g_unk0x10063a54[i]; entry != NULL; entry = next) {
			next = entry->m_unk0x08;
			FUN_100139e7(entry);
		}
	}
	g_unk0x10096860 = 0;
	g_unk0x10096864 = NULL;
	g_unk0x10096868 = NULL;
	free(g_unk0x10063a54);
}

// FUNCTION: MW2SHELL 0x10013907
void FUN_10013907(void)
{
	g_unk0x10096860 = 0;
	g_unk0x10096864 = 0;
	g_unk0x10096868 = 0;
	FUN_100137aa();
}

// FUNCTION: MW2SHELL 0x10013935
void FUN_10013935(void)
{
}

// Stack-slot permutation: bucket and entry exchange [ebp-8] and [ebp-4]. The hash
// loads type[3] before type[2] in the recompilation; reversing their source order
// does not change VC++ 4.1's load order.
// FUNCTION: MW2SHELL 0x10013940
DrawCacheEntry* FUN_10013940(MechS32 p_id, char* p_type)
{
	MechS32 bucket;
	DrawCacheEntry* entry;

	if (p_id < 0) {
		return NULL;
	}

	bucket = ((MechS8) p_type[3] + (MechS8) p_type[2] + (MechS8) p_type[0] + (MechS8) p_type[1] + p_id) % 0x3f1;
	for (entry = g_unk0x10063a54[bucket]; entry != NULL; entry = entry->m_unk0x08) {
		if (entry->m_unk0x00 == p_id && entry->m_unk0x04 == *(undefined4*) p_type) {
			break;
		}
	}

	return entry;
}

// Stack-slot permutation only: entry uses [ebp-10] instead of [ebp-4], the
// four type bytes use [ebp-8..-5] instead of [ebp-c..-9], the terminator uses
// [ebp-4] instead of [ebp-8], and bucket uses [ebp-c] instead of [ebp-10].
// FUNCTION: MW2SHELL 0x100139e7
void FUN_100139e7(DrawCacheEntry* p_entry)
{
	MechS32 bucket;
	DrawCacheEntry* entry;
	MechChar type[5];

	entry = NULL;
	if (p_entry == NULL) {
		return;
	}

	p_entry->m_unk0x02 = 0;
	FUN_10013703(p_entry);
	type[4] = '\0';
	*(undefined4*) type = p_entry->m_unk0x04;
	bucket = ((MechS8) type[2] + (MechS8) type[3] + (MechS8) type[0] + (MechS8) type[1] + p_entry->m_unk0x00) % 0x3f1;
	if (g_unk0x10063a54[bucket] == p_entry) {
		g_unk0x10063a54[bucket] = p_entry->m_unk0x08;
	}
	else {
		for (entry = g_unk0x10063a54[bucket]; entry != NULL && entry->m_unk0x08 != NULL; entry = entry->m_unk0x08) {
			if (entry->m_unk0x08 == p_entry) {
				break;
			}
		}
		if (entry != NULL && entry->m_unk0x08 != NULL) {
			entry->m_unk0x08 = entry->m_unk0x08->m_unk0x08;
		}
		else {
			return;
		}
	}

	free(p_entry);
	g_unk0x10096860--;
}

// FUNCTION: MW2SHELL 0x10013c6e
void FUN_10013c6e(void)
{
}

// FUNCTION: MW2SHELL 0x10013c79
void FUN_10013c79(MechS32 p_id, char* p_type)
{
	DrawCacheEntry* entry = FUN_10013940(p_id, p_type);
	if (entry == NULL) {
		return;
	}

	FUN_10013690(entry);
}

// STUB: MW2SHELL 0x10013cb5
void* FUN_10013cb5(MechS32 p_unk0x00, MechS32 p_id, char* p_type, MechS32 p_unk0x0c)
{
	STUB(0x10013cb5);
	return NULL;
}

// FUNCTION: MW2SHELL 0x10013ef4
void FUN_10013ef4(MechS32 p_id, char* p_type)
{
	DrawCacheEntry* entry = FUN_10013940(p_id, p_type);
	if (entry == NULL) {
		return;
	}

	FUN_100139e7(entry);
}

// FUNCTION: MW2SHELL 0x10013f30
void FUN_10013f30(void)
{
}

// FUNCTION: MW2SHELL 0x10013f3b
undefined4 FUN_10013f3b(undefined4 p_value)
{
	return p_value;
}

// FUNCTION: MW2SHELL 0x10013f4e
void* FUN_10013f4e(MechS32 p_id, char* p_type)
{
	return FUN_10013cb5(0, p_id, p_type, 0);
}

// FUNCTION: MW2SHELL 0x10013f72
MechS32 FUN_10013f72(void)
{
	if (g_unk0x10096864 != NULL) {
		FUN_100139e7(g_unk0x10096864);
		return 1;
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x10013fa9
void* FUN_10013fa9(undefined4 p_size)
{
	return calloc(p_size, 1);
}

// FUN_10013fc7 and FUN_10013ff1 are memcpy and memset written as inline __asm: rep movsd/stosd
// for the dwords, then rep movsb/stosb for the rest. Modern compilers (COMPAT_MODE) call the CRT.

// FUNCTION: MW2SHELL 0x10013fc7
void* FUN_10013fc7(void* p_destination, void* p_source, MechU32 p_size)
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

// FUNCTION: MW2SHELL 0x10013ff1
void* FUN_10013ff1(void* p_destination, MechS32 p_value, MechU32 p_size)
{
#ifdef COMPAT_MODE
	memset(p_destination, p_value, p_size);
#else
	__asm {
		mov edx, p_size
		mov eax, p_value
		mov cl, al
		mov ch, cl
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

// FUNCTION: MW2SHELL 0x10014029
void FUN_10014029(void* p_buffer)
{
	free(p_buffer);
}
