/* A buffer split into two stacks that grow towards each other: FUN_1007d248 takes 0x20-byte
   records from the top, FUN_1007d296 0xc-byte records from the bottom, and both clear
   g_unk0x1010b5ac when the gap between them drops to 0xc8 bytes.

   Hand-written assembly: FUN_1007d248 and FUN_1007d296 are C functions whose bodies are
   mostly an __asm block (eax carries the new top across statements, which /Od never does). Their
   portable C (PORTABLE_C) is tested against the assembly by tests/asmequiv. */
#include "recordstacks.h"

#include "clock.h"
#include "compat.h"
#include "decomp.h"
#include "depthsort.h"
#include "error.h"
#include "loadres.h"
#include "types.h"

// GLOBAL: MW2 0x100ba5cc
MechS32 g_unk0x100ba5cc = 0x80;

// GLOBAL: MW2 0x100ba5d0
MechU8* g_unk0x100ba5d0 = NULL;

// GLOBAL: MW2 0x100c1a68
MechS32 g_unk0x100c1a68;

// GLOBAL: MW2 0x100c1a6c
MechU8* g_unk0x100c1a6c;

// GLOBAL: MW2 0x100c1a70
MechU8* g_unk0x100c1a70;

// GLOBAL: MW2 0x100c2280
AmberDune0x8* g_unk0x100c2280;

// GLOBAL: MW2 0x100c2698
MechU8* g_unk0x100c2698;

// GLOBAL: MW2 0x100c269c
AmberDune0x8* g_unk0x100c269c;

// GLOBAL: MW2 0x1010b5ac
MechS32 g_unk0x1010b5ac;

// FUNCTION: MW2 0x1007d120
void FUN_1007d120(void)
{
	if (g_unk0x100ba5d0 != NULL) {
		MemFree(g_unk0x100ba5d0);
		g_unk0x100ba5d0 = NULL;
	}
}

// Operand order: the original computes g_unk0x100c1a68 << 4 before p_unk0x00 << 10 in size
// (both front ends reorder commutative operands).
// FUNCTION: MW2 0x1007d150
void FUN_1007d150(MechS32 p_unk0x00, MechS32 p_unk0x04)
{
	MechU32 size;

	g_unk0x100c1a68 = p_unk0x04;
	g_unk0x100ba5cc = p_unk0x00 << 10;
	size = (g_unk0x100c1a68 << 4) + (p_unk0x00 << 10);
	g_unk0x100ba5d0 = MemAlloc(size);
	if (g_unk0x100ba5d0 == NULL) {
		Error(0x18, NULL);
	}

	MemSet(g_unk0x100ba5d0, 0, size);
	g_unk0x100c1a6c = g_unk0x100ba5d0;
	g_unk0x100c2280 = (AmberDune0x8*) (g_unk0x100ba5d0 + g_unk0x100ba5cc);
	g_unk0x100c269c = g_unk0x100c2280 + g_unk0x100c1a68;
	g_unk0x100c2698 = g_unk0x100c1a6c;
	g_unk0x100c1a70 = g_unk0x100c1a6c + (g_unk0x100ba5cc << 5) - 0x600;
	FUN_1007ca8e();
	FUN_1007c930();
	FUN_1007c9e3();
}

// FUNCTION: MW2 0x1007d220
void FUN_1007d220(void)
{
	g_unk0x100c2698 = g_unk0x100c1a6c;
	g_unk0x100c1a70 = g_unk0x100c1a6c + g_unk0x100ba5cc - 0x30;
}

// FUNCTION: MW2 0x1007d248
ProjectedVertex* FUN_1007d248(void)
{
	MechS32 recordSize;
	ProjectedVertex* record;

	recordSize = 0x20;
#ifdef PORTABLE_C
	/* The gap compares as the unsigned addresses do: the two stacks share one buffer. */
	g_unk0x100c1a70 -= recordSize;
	record = (ProjectedVertex*) g_unk0x100c1a70;
	record->m_projected = 0;
	if (g_unk0x100c1a70 - g_unk0x100c2698 <= 0xc8) {
		g_unk0x1010b5ac = 0;
	}
#else
	__asm {
		mov ecx, recordSize
		mov eax, g_unk0x100c1a70
		sub eax, ecx
		mov record, eax
		mov g_unk0x100c1a70, eax
		mov ebx, record
		mov byte ptr [ebx+0x1d], 0
		sub eax, 0xc8
		cmp eax, g_unk0x100c2698
		ja done
		xor eax, eax
		mov g_unk0x1010b5ac, eax
done:
	}
#endif

	return record;
}

// FUNCTION: MW2 0x1007d296
MechU8* FUN_1007d296(void)
{
	MechS32 recordSize;
	MechU8* record;

	recordSize = 0xc;
#ifdef PORTABLE_C
	record = g_unk0x100c2698;
	g_unk0x100c2698 += recordSize;
	if (g_unk0x100c1a70 - g_unk0x100c2698 <= 0xc8) {
		g_unk0x1010b5ac = 0;
	}
#else
	__asm {
		mov ecx, recordSize
		mov eax, g_unk0x100c2698
		mov record, eax
		add eax, ecx
		mov g_unk0x100c2698, eax
		add eax, 0xc8
		cmp eax, g_unk0x100c1a70
		jb done
		xor eax, eax
		mov g_unk0x1010b5ac, eax
done:
	}
#endif

	return record;
}
