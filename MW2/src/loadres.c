/* Hand-written assembly: MemCopy and MemSet are C functions whose bodies are an __asm block
   (rep movsd/stosd). */
#include "loadres.h"

#include "decomp.h"
#include "simmain.h"
#include "types.h"

#include <windows.h>

// STUB: MW2 0x1001a19f
void* FUN_1001a19f(undefined4 p_unk0x00, MechS32 p_unk0x04, const char* p_unk0x08, undefined4 p_unk0x0c)
{
	STUB(0x1001a19f);
	return NULL;
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
