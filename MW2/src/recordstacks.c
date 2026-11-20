/* A buffer split into two stacks that grow towards each other: AllocProjectedVertex takes 0x20-byte
   records from the top, AllocQueuedPolygon 0xc-byte records from the bottom, and both clear
   g_queueHasRoom when the gap between them drops to 0xc8 bytes.

   Hand-written assembly: AllocProjectedVertex and AllocQueuedPolygon are C functions whose bodies are
   mostly an __asm block (eax carries the new top across statements, which /Od never does). Their
   portable C (PORTABLE_C) is tested against the assembly by tests/asmequiv. */
#include "recordstacks.h"

#include "clock.h"
#include "compat.h"
#include "debugprint.h"
#include "decomp.h"
#include "depthsort.h"
#include "error.h"
#include "loadres.h"
#include "types.h"

#ifdef MW2_ACCEL
// The Matrox edition defines the draw buffer's globals in another order, all initialized.
// GLOBAL: MW2MATROX 0x100a4fac
// GLOBAL: MW23DFX 0x1024db90
// GLOBAL: MW2ATI 0x10181ea0
// GLOBAL: MW2POWERVR 0x101acf60
DepthEntry* g_drawList = NULL;

// GLOBAL: MW2MATROX 0x100a4fb0
// GLOBAL: MW23DFX 0x1024db94
// GLOBAL: MW2ATI 0x10181ea4
// GLOBAL: MW2S3 0x10060ef0
// GLOBAL: MW2POWERVR 0x101acf64
DepthEntry* g_depthQueue = NULL;

// GLOBAL: MW2MATROX 0x100a4fb4
// GLOBAL: MW23DFX 0x1024db98
// GLOBAL: MW2ATI 0x10181ea8
// GLOBAL: MW2S3 0x10060ef4
// GLOBAL: MW2POWERVR 0x101acf68
MechU8* g_drawBuffer = NULL;

// GLOBAL: MW2MATROX 0x100a4fb8
// GLOBAL: MW23DFX 0x1024db9c
// GLOBAL: MW2ATI 0x10181eac
// GLOBAL: MW2S3 0x10060ef8
// GLOBAL: MW2POWERVR 0x101acf6c
MechU8* g_drawBufferTop = NULL;

// GLOBAL: MW2MATROX 0x100a4fbc
// GLOBAL: MW23DFX 0x1024dba0
// GLOBAL: MW2ATI 0x10181eb0
// GLOBAL: MW2S3 0x10060efc
// GLOBAL: MW2POWERVR 0x101acf70
MechU8* g_drawBufferBottom = NULL;

// GLOBAL: MW2MATROX 0x100a4fc0
// GLOBAL: MW23DFX 0x1024dba4
// GLOBAL: MW2ATI 0x10181eb4
// GLOBAL: MW2POWERVR 0x101acf74
MechS32 g_depthListCapacity = 0;

// GLOBAL: MW2MATROX 0x100a4fc4
// GLOBAL: MW23DFX 0x1024dba8
// GLOBAL: MW2ATI 0x10181eb8
// GLOBAL: MW2S3 0x10060f04
// GLOBAL: MW2POWERVR 0x101acf78
MechS32 g_drawBufferSize = 0x140;

// GLOBAL: MW2MATROX 0x100a4fc8
// GLOBAL: MW23DFX 0x1024dbac
// GLOBAL: MW2ATI 0x10181ebc
// GLOBAL: MW2S3 0x10060f08
// GLOBAL: MW2POWERVR 0x101acf7c
MechU8* g_drawBufferMemory = NULL;
#else
// GLOBAL: MW2 0x100ba5cc
MechS32 g_drawBufferSize = 0x80;

// GLOBAL: MW2 0x100ba5d0
MechU8* g_drawBufferMemory = NULL;

// GLOBAL: MW2 0x100c1a68
MechS32 g_depthListCapacity;

// GLOBAL: MW2 0x100c1a6c
MechU8* g_drawBuffer;

// GLOBAL: MW2 0x100c1a70
MechU8* g_drawBufferTop;

// GLOBAL: MW2 0x100c2280
DepthEntry* g_drawList;

// GLOBAL: MW2 0x100c2698
MechU8* g_drawBufferBottom;

// GLOBAL: MW2 0x100c269c
DepthEntry* g_depthQueue;
#endif

// GLOBAL: MW2 0x1010b5ac
// GLOBAL: MW2MATROX 0x101d6968
// GLOBAL: MW23DFX 0x101b0ec8
// GLOBAL: MW2ATI 0x100e4ea8
// GLOBAL: MW2S3 0x100b1d6c
// GLOBAL: MW2POWERVR 0x1010c3c8
MechS32 g_queueHasRoom;

// FUNCTION: MW2 0x1007d120
// FUNCTION: MW2MATROX 0x100216a0
// FUNCTION: MW23DFX 0x10025d60
// FUNCTION: MW2ATI 0x10025ad0
// FUNCTION: MW2S3 0x10003210
// FUNCTION: MW2POWERVR 0x10024d60
void ShutdownDrawBuffer(void)
{
	if (g_drawBufferMemory != NULL) {
		MemFree(g_drawBufferMemory);
		g_drawBufferMemory = NULL;
	}
}

// FUNCTION: MW2 0x1007d150
// FUNCTION: MW2MATROX 0x100216d0
// FUNCTION: MW23DFX 0x10025d80
// FUNCTION: MW2ATI 0x10025af0
// FUNCTION: MW2POWERVR 0x10024d80
void InitializeDrawBuffer(MechS32 p_kilobytes, MechS32 p_entries)
{
	MechU32 size;

	g_depthListCapacity = p_entries;
	g_drawBufferSize = p_kilobytes << 10;
#ifdef MW2_A3D
	size = g_depthListCapacity * (2 * sizeof(DepthEntry)) + (p_kilobytes << 10);
#else
	size = (g_depthListCapacity << 4) + (p_kilobytes << 10);
#endif
	g_drawBufferMemory = MemAlloc(size);
	if (g_drawBufferMemory == NULL) {
		Error(0x18, NULL);
	}

	MemSet(g_drawBufferMemory, 0, size);
	g_drawBuffer = g_drawBufferMemory;
	g_drawList = (DepthEntry*) (g_drawBufferMemory + g_drawBufferSize);
	g_depthQueue = g_drawList + g_depthListCapacity;
	g_drawBufferBottom = g_drawBuffer;
#ifdef MW2_A3D
	g_drawBufferTop = g_drawBuffer + g_drawBufferSize - 0x30;
#else
	g_drawBufferTop = g_drawBuffer + (g_drawBufferSize << 5) - 0x600;
#endif
#ifdef MW2_EXTRA_DEBUG_PRINTS
	DebugPrint("setup_render: npalloc=%x nvalloc=%x\n", g_drawBufferBottom, g_drawBufferTop);
#endif
#ifndef MW2_FLOAT_SCALARS
	InitSqrtTable();
#endif
	InitSinAtanTables();
	InitSlopeTables();
}

// FUNCTION: MW2 0x1007d220
// FUNCTION: MW2MATROX 0x100217b3
// FUNCTION: MW23DFX 0x10025e20
// FUNCTION: MW2ATI 0x10025b90
// FUNCTION: MW2S3 0x100032d0
void ResetDrawBuffer(void)
{
	g_drawBufferBottom = g_drawBuffer;
	g_drawBufferTop = g_drawBuffer + g_drawBufferSize - 0x30;
}

// The Matrox edition's is C, for its 0x3c-byte records.
// FUNCTION: MW2 0x1007d248
// FUNCTION: MW2MATROX 0x100217db
// FUNCTION: MW23DFX 0x10025e40
// FUNCTION: MW2ATI 0x10025bb0
// FUNCTION: MW2S3 0x100032f0
ProjectedVertex* AllocProjectedVertex(void)
{
#ifdef MW2_A3D
	ProjectedVertex* record;

	g_drawBufferTop -= 0x3c;
	record = (ProjectedVertex*) g_drawBufferTop;
	record->m_projected = 0;
	if (g_drawBufferTop - 0xc8 <= g_drawBufferBottom) {
		g_queueHasRoom = 0;
	}
#else
	MechS32 recordSize;
	ProjectedVertex* record;

	recordSize = 0x20;
#ifdef PORTABLE_C
	/* The gap compares as the unsigned addresses do: the two stacks share one buffer. */
	g_drawBufferTop -= recordSize;
	record = (ProjectedVertex*) g_drawBufferTop;
	record->m_projected = 0;
	if (g_drawBufferTop - g_drawBufferBottom <= 0xc8) {
		g_queueHasRoom = 0;
	}
#else
	__asm {
		mov ecx, recordSize
		mov eax, g_drawBufferTop
		sub eax, ecx
		mov record, eax
		mov g_drawBufferTop, eax
		mov ebx, record
		mov byte ptr [ebx+0x1d], 0
		sub eax, 0xc8
		cmp eax, g_drawBufferBottom
		ja done
		xor eax, eax
		mov g_queueHasRoom, eax
done:
	}
#endif
#endif

	return record;
}

// The Matrox edition's takes 0x10-byte records.
// FUNCTION: MW2 0x1007d296
// FUNCTION: MW2MATROX 0x10021827
// FUNCTION: MW23DFX 0x10025e70
// FUNCTION: MW2ATI 0x10025be0
// FUNCTION: MW2S3 0x10003320
MechU8* AllocQueuedPolygon(void)
{
	MechS32 recordSize;
	MechU8* record;

#ifdef MW2_A3D
	recordSize = 0x10;
#else
	recordSize = 0xc;
#endif
#ifdef PORTABLE_C
	record = g_drawBufferBottom;
	g_drawBufferBottom += recordSize;
	if (g_drawBufferTop - g_drawBufferBottom <= 0xc8) {
		g_queueHasRoom = 0;
	}
#else
	__asm {
		mov ecx, recordSize
		mov eax, g_drawBufferBottom
		mov record, eax
		add eax, ecx
		mov g_drawBufferBottom, eax
		add eax, 0xc8
		cmp eax, g_drawBufferTop
		jb done
		xor eax, eax
		mov g_queueHasRoom, eax
done:
	}
#endif

	return record;
}
