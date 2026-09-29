/* Hand-written assembly: MemCopy and MemSet are C functions whose bodies are an __asm block
   (rep movsd/stosd). */
#include "loadres.h"

#include "decomp.h"
#include "simmain.h"
#include "types.h"

#include <windows.h>

// GLOBAL: MW2 0x101748d4
undefined4* g_unk0x101748d4;

// STUB: MW2 0x10019af0
void FUN_10019af0(undefined4* p_unk0x00)
{
	STUB(0x10019af0);
}

// STUB: MW2 0x10019dac
undefined4* FUN_10019dac(MechS32 p_id, const char* p_type)
{
	STUB(0x10019dac);
	return NULL;
}

// STUB: MW2 0x10019e53
void FUN_10019e53(undefined4* p_unk0x00)
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
	undefined4* entry;

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
	undefined4* entry;

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
	if (g_unk0x101748d4) {
		FUN_10019e53(g_unk0x101748d4);
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
