/* The Matrox edition's A3D renderer (0x1005a5f0 to 0x10067500): one object compiled with /Ox /G5 /Op,
   where the rest of the edition is /Od (CLAUDE.md, "The A3D layer"). */
#include "matrox/a3d.h"

#include "debugprint.h"
#include "decomp.h"
#include "msi95.h"
#include "mw2prj.h"
#include "palettecolor.h"
#include "polydraw.h"
#include "prjfile.h"
#include "simmain.h"
#include "types.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

typedef struct A3DHeapBlock A3DHeapBlock;

// The display msiInit returns.
typedef struct A3DDisplay {
	undefined m_unk0x00[0x50]; // 0x00
	MechU32 m_unk0x50;         // 0x50 — the texture cache's size
} A3DDisplay;

// An item of the texture caches, on two lists: its cache's (m_next, m_prev) and the heap cache's
// (m_heapNext, m_heapPrev).
typedef struct A3DCacheItem A3DCacheItem;
struct A3DCacheItem {
	A3DCacheItem* m_next;     // 0x00
	A3DCacheItem* m_prev;     // 0x04
	A3DCacheItem* m_heapNext; // 0x08
	A3DCacheItem* m_heapPrev; // 0x0c
	MechS32 m_id;             // 0x10
	A3DHeapBlock* m_unk0x14;  // 0x14 (the VRAM block)
	A3DHeapBlock* m_unk0x18;  // 0x18 (the heap block)
	MechS32 m_width;          // 0x1c
	MechS32 m_height;         // 0x20
	MechS32 m_format;         // 0x24
	undefined4 m_unk0x28;     // 0x28
	undefined4 m_unk0x2c;     // 0x2c
	MechU32 m_size;           // 0x30
	MechU32 m_levels;         // 0x34
	undefined4 m_unk0x38;     // 0x38
	MechU32 m_offsets[9];     // 0x3c
	MechU32 m_unk0x60[9];     // 0x60
};

// A block of the texture heap, on two lists: all blocks (m_unk0x00, m_unk0x04) and the used or the
// free blocks (m_unk0x08, m_unk0x0c; the free ones in address order).
struct A3DHeapBlock {
	A3DHeapBlock* m_unk0x00; // 0x00
	A3DHeapBlock* m_unk0x04; // 0x04
	A3DHeapBlock* m_unk0x08; // 0x08
	A3DHeapBlock* m_unk0x0c; // 0x0c
	A3DCacheItem* m_owner;   // 0x10
	MechS32 m_free;          // 0x14
	MechU32 m_size;          // 0x18
	MechU32 m_address;       // 0x1c
	undefined* m_unk0x20;    // 0x20 (heap blocks only: the data)
};

// GLOBAL: MW2MATROX 0x100ac900
MechS32 g_unk0x100ac900 = 0;

// GLOBAL: MW2MATROX 0x100ac904
MechS32 g_unk0x100ac904 = 0;

// GLOBAL: MW2MATROX 0x100ac908
MechS32 g_unk0x100ac908 = 0;

// GLOBAL: MW2MATROX 0x100ac90c
MechS32 g_unk0x100ac90c = 1;

// GLOBAL: MW2MATROX 0x100ac910
MechFloat g_unk0x100ac910 = 0.0f;

// GLOBAL: MW2MATROX 0x100ac914
MechFloat g_unk0x100ac914 = 0.0f;

// GLOBAL: MW2MATROX 0x100ac918
MechFloat g_unk0x100ac918 = 0.0f;

// GLOBAL: MW2MATROX 0x100ac91c
undefined4 g_unk0x100ac91c = 0x12c;

// GLOBAL: MW2MATROX 0x100ac920
undefined4 g_unk0x100ac920 = 0;

// GLOBAL: MW2MATROX 0x100ac924
MechU32 g_unk0x100ac924 = 1;

// GLOBAL: MW2MATROX 0x100ac928
MechU32 g_unk0x100ac928 = 1;

// GLOBAL: MW2MATROX 0x100ac92c
undefined4 g_unk0x100ac92c = 0x14;

// MSI95.DLL's display (msiInit's).
// GLOBAL: MW2MATROX 0x100ac930
A3DDisplay* g_unk0x100ac930 = NULL;

// GLOBAL: MW2MATROX 0x100ac934
undefined4 g_unk0x100ac934 = 0;

// The display's size in pixels.
// GLOBAL: MW2MATROX 0x100ac938
MechFloat g_unk0x100ac938 = 640.0f;

// GLOBAL: MW2MATROX 0x100ac93c
MechFloat g_unk0x100ac93c = 480.0f;

// GLOBAL: MW2MATROX 0x100ac940
MechFloat g_unk0x100ac940[0x24] = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
								   0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
								   0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f};

// GLOBAL: MW2MATROX 0x100ac9d0
MechS32 g_unk0x100ac9d0 = -1;

// GLOBAL: MW2MATROX 0x100ac9d4
MechS32 g_unk0x100ac9d4 = -1;

// GLOBAL: MW2MATROX 0x100ac9d8
MechS32 g_unk0x100ac9d8 = -1;

// GLOBAL: MW2MATROX 0x100ac9dc
MechS32 g_unk0x100ac9dc = -1;

// GLOBAL: MW2MATROX 0x100ac9e0
MechS32 g_unk0x100ac9e0 = 0;

// GLOBAL: MW2MATROX 0x100ac9e4
MechS32 g_unk0x100ac9e4 = 0;

// GLOBAL: MW2MATROX 0x100ac9e8
undefined4 g_unk0x100ac9e8[0x16] = {1, 0x40, 0x40, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0x80008000, 0, 0, 1, 0, 0, 0};

// GLOBAL: MW2MATROX 0x100aca40
undefined4 g_unk0x100aca40[0x16] = {0, 0, 0, 0xf, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0};

// The parameters FUN_100659f0 passes msiSetParameters.
// GLOBAL: MW2MATROX 0x100aca98
undefined4 g_unk0x100aca98[0xc] = {1, 0, 0, 0xf, 0, 0, 0, 0, 0, 0, 0, 0};

// GLOBAL: MW2MATROX 0x100acac8
undefined4 g_unk0x100acac8 = 0;

// GLOBAL: MW2MATROX 0x100acacc
undefined4 g_unk0x100acacc = 0;

// GLOBAL: MW2MATROX 0x100acad0
undefined4 g_unk0x100acad0 = 0;

// GLOBAL: MW2MATROX 0x100acad4
undefined2 g_unk0x100acad4 = 0;

// GLOBAL: MW2MATROX 0x100acad6
undefined2 g_unk0x100acad6 = 0;

// GLOBAL: MW2MATROX 0x100acad8
undefined2 g_unk0x100acad8 = 0;

// GLOBAL: MW2MATROX 0x100acada
undefined2 g_unk0x100acada = 0;

// GLOBAL: MW2MATROX 0x100acadc
undefined4 g_unk0x100acadc[5] = {0, 1, 0, 0, 0};

// GLOBAL: MW2MATROX 0x100acaf0
A3DHeapBlock* g_unk0x100acaf0 = NULL;

// GLOBAL: MW2MATROX 0x100acaf4
A3DHeapBlock* g_unk0x100acaf4 = NULL;

// GLOBAL: MW2MATROX 0x100acaf8
A3DHeapBlock* g_unk0x100acaf8 = NULL;

// GLOBAL: MW2MATROX 0x100acafc
MechU32 g_unk0x100acafc = 0;

// GLOBAL: MW2MATROX 0x100acb00
MechS32 g_unk0x100acb00 = 0;

// GLOBAL: MW2MATROX 0x100acb04
MechS32 g_unk0x100acb04 = 0;

// GLOBAL: MW2MATROX 0x100acb08
MechS32 g_unk0x100acb08 = 0;

// GLOBAL: MW2MATROX 0x100acb0c
A3DHeapBlock* g_unk0x100acb0c = NULL;

// GLOBAL: MW2MATROX 0x100acb10
A3DHeapBlock* g_unk0x100acb10 = NULL;

// GLOBAL: MW2MATROX 0x100acb14
A3DHeapBlock* g_unk0x100acb14 = NULL;

// GLOBAL: MW2MATROX 0x100acb18
MechU32 g_unk0x100acb18 = 0;

// GLOBAL: MW2MATROX 0x100acb1c
MechS32 g_unk0x100acb1c = 0;

// GLOBAL: MW2MATROX 0x100acb20
undefined* g_unk0x100acb20 = NULL;

// GLOBAL: MW2MATROX 0x100acb24
MechS32 g_unk0x100acb24 = 0;

// GLOBAL: MW2MATROX 0x100acb28
MechS32 g_unk0x100acb28 = 0;

// GLOBAL: MW2MATROX 0x100acb2c
undefined4 g_unk0x100acb2c = 0;

// GLOBAL: MW2MATROX 0x100acb30
A3DCacheItem* g_unk0x100acb30 = NULL;

// GLOBAL: MW2MATROX 0x100acb34
A3DCacheItem* g_unk0x100acb34 = NULL;

// GLOBAL: MW2MATROX 0x100acb38
MechS32 g_unk0x100acb38 = 0;

// GLOBAL: MW2MATROX 0x100acb3c
MechS32 g_unk0x100acb3c = 0;

// GLOBAL: MW2MATROX 0x100acb40
A3DCacheItem* g_unk0x100acb40 = NULL;

// GLOBAL: MW2MATROX 0x100acb44
A3DCacheItem* g_unk0x100acb44 = NULL;

// GLOBAL: MW2MATROX 0x100acb48
MechS32 g_unk0x100acb48 = 0;

// GLOBAL: MW2MATROX 0x100acb4c
MechS32 g_unk0x100acb4c = 0;

// GLOBAL: MW2MATROX 0x100acb50
MechS32 g_unk0x100acb50 = 0;

// GLOBAL: MW2MATROX 0x100acb54
undefined4 g_unk0x100acb54 = 0;

// GLOBAL: MW2MATROX 0x100acb58
MechS32 g_unk0x100acb58 = 0;

// GLOBAL: MW2MATROX 0x100acb5c
undefined4 g_unk0x100acb5c = 1;

// The cached items by ID.
// GLOBAL: MW2MATROX 0x100c2680
static A3DCacheItem* g_unk0x100c2680[0x4000];

// GLOBAL: MW2MATROX 0x100d2680
static A3DCacheItem* g_unk0x100d2680[0x4000];

// The buffer FUN_10066c40 fills rectangles from.
// GLOBAL: MW2MATROX 0x100e2680
static undefined g_unk0x100e2680[0x20];

// The MIP map thresholds (MYSTIQUE.PAR's, times its MIPmaxZ), ending in 1e8; room for eight before the
// texture at 0x100e26e0.
// GLOBAL: MW2MATROX 0x100e26a0
static MechDouble g_unk0x100e26a0[8];

// The 64x64 16-bit texture FUN_1005e2c0 loads for the ID 0x29b6.
// GLOBAL: MW2MATROX 0x100e26e0
undefined g_unk0x100e26e0[0x2000];

// The bounds of the polygon being drawn (maximum x and y, minimum x and y).
// GLOBAL: MW2MATROX 0x101246e0
MechFloat g_unk0x101246e0;

// GLOBAL: MW2MATROX 0x101246e4
MechFloat g_unk0x101246e4;

// GLOBAL: MW2MATROX 0x101246e8
MechFloat g_unk0x101246e8;

// GLOBAL: MW2MATROX 0x101246ec
MechFloat g_unk0x101246ec;

// Takes p_item off the list p_head to p_tail, through its members p_next and p_prev (p_nextItem holds
// p_item->p_next).
#define A3D_UNLINK(p_item, p_nextItem, p_next, p_prev, p_head, p_tail)                                                 \
	p_nextItem = (p_item)->p_next;                                                                                     \
	if (!p_nextItem && !(p_item)->p_prev) {                                                                            \
		if (p_head == (p_item)) {                                                                                      \
			p_head = NULL;                                                                                             \
		}                                                                                                              \
		if (p_tail == (p_item)) {                                                                                      \
			p_tail = NULL;                                                                                             \
		}                                                                                                              \
	}                                                                                                                  \
	else {                                                                                                             \
		if (p_nextItem) {                                                                                              \
			p_nextItem->p_prev = (p_item)->p_prev;                                                                     \
		}                                                                                                              \
		else {                                                                                                         \
			p_tail = (p_item)->p_prev;                                                                                 \
		}                                                                                                              \
		if ((p_item)->p_prev) {                                                                                        \
			(p_item)->p_prev->p_next = p_nextItem;                                                                     \
		}                                                                                                              \
		else {                                                                                                         \
			p_head = p_nextItem;                                                                                       \
		}                                                                                                              \
		(p_item)->p_next = NULL;                                                                                       \
		(p_item)->p_prev = NULL;                                                                                       \
	}

// Takes the heap block p_block off its used or free list, whose head is p_head.
#define A3D_UNLIST_BLOCK(p_block, p_head)                                                                              \
	if ((p_block)->m_unk0x0c) {                                                                                        \
		(p_block)->m_unk0x0c->m_unk0x08 = (p_block)->m_unk0x08;                                                        \
	}                                                                                                                  \
	else {                                                                                                             \
		p_head = (p_block)->m_unk0x08;                                                                                 \
	}                                                                                                                  \
	if ((p_block)->m_unk0x08) {                                                                                        \
		(p_block)->m_unk0x08->m_unk0x0c = (p_block)->m_unk0x0c;                                                        \
	}

// Takes the heap block p_block off the list of all blocks, whose head is p_head (p_prevBlock holds
// p_block->m_unk0x04).
#define A3D_UNCHAIN_BLOCK(p_block, p_prevBlock, p_head)                                                                \
	p_prevBlock = (p_block)->m_unk0x04;                                                                                \
	if (p_prevBlock) {                                                                                                 \
		p_prevBlock->m_unk0x00 = (p_block)->m_unk0x00;                                                                 \
	}                                                                                                                  \
	else {                                                                                                             \
		p_head = (p_block)->m_unk0x00;                                                                                 \
	}                                                                                                                  \
	if ((p_block)->m_unk0x00) {                                                                                        \
		(p_block)->m_unk0x00->m_unk0x04 = p_prevBlock;                                                                 \
	}

// p_a moved toward p_b by p_t, or p_a where they are (nearly) equal.
#define A3D_LERP(p_out, p_a, p_b, p_t)                                                                                 \
	if ((MechFloat) fabs((p_a) - (p_b)) < 1e-7f) {                                                                     \
		p_out = p_a;                                                                                                   \
	}                                                                                                                  \
	else {                                                                                                             \
		p_out = (MechFloat) (((p_b) - (p_a)) * (p_t) + (p_a));                                                         \
	}

// The screen position of the vertex p_vertex as msiDrawSingleLine takes it (y in the high word).
#define A3D_POINT(p_vertex) ((MechS32) (p_vertex)->m_y << 16 | (MechS32) (p_vertex)->m_x & 0xffff)

// Copies the p_count vertices p_from to p_to.
#ifdef PORTABLE_C
#define A3D_COPY_VERTICES(p_from, p_to, p_count) memcpy(p_to, p_from, (p_count) * sizeof(A3DVertex))
#else
#define A3D_COPY_VERTICES(p_from, p_to, p_count)                                                                       \
	__asm {                                                                                                            \
		__asm mov ebx, p_from                                                                                          \
		__asm mov edx, p_to                                                                                            \
		__asm mov ecx, p_count                                                                                         \
		__asm copyLoop:                                                                                                \
		__asm mov eax, [ebx + 0]                                                                                           \
		__asm mov [edx + 0], eax                                                                                           \
		__asm mov eax, [ebx + 4]                                                                                           \
		__asm mov [edx + 4], eax                                                                                           \
		__asm mov eax, [ebx + 8]                                                                                           \
		__asm mov [edx + 8], eax                                                                                           \
		__asm mov eax, [ebx + 12]                                                                                          \
		__asm mov [edx + 12], eax                                                                                          \
		__asm mov eax, [ebx + 16]                                                                                          \
		__asm mov [edx + 16], eax                                                                                          \
		__asm mov eax, [ebx + 20]                                                                                          \
		__asm mov [edx + 20], eax                                                                                          \
		__asm mov eax, [ebx + 24]                                                                                          \
		__asm mov [edx + 24], eax                                                                                          \
		__asm mov eax, [ebx + 28]                                                                                          \
		__asm mov [edx + 28], eax                                                                                          \
		__asm mov eax, [ebx + 32]                                                                                          \
		__asm mov [edx + 32], eax                                                                                          \
		__asm mov eax, [ebx + 36]                                                                                          \
		__asm mov [edx + 36], eax                                                                                          \
		__asm mov eax, [ebx + 40]                                                                                          \
		__asm mov [edx + 40], eax                                                                                          \
		__asm mov eax, [ebx + 44]                                                                                          \
		__asm mov [edx + 44], eax                                                                                          \
		__asm add ebx, 0x30                                                                                            \
		__asm add edx, 0x30                                                                                            \
		__asm dec ecx                                                                                                  \
		__asm jne copyLoop }
#endif

// Widens the bounds g_unk0x101246e0 to g_unk0x101246ec (set from the first vertex) by the p_count - 1
// vertices from p_vertex on.
#ifdef PORTABLE_C
#define A3D_COMPUTE_BOUNDS(p_vertex, p_count)                                                                          \
	{                                                                                                                  \
		A3DVertex* boundsVertex = p_vertex;                                                                            \
		MechS32 boundsCount = (p_count) - 1;                                                                           \
		do {                                                                                                           \
			if (boundsVertex->m_x < g_unk0x101246e8) {                                                                 \
				g_unk0x101246e8 = boundsVertex->m_x;                                                                   \
			}                                                                                                          \
			if (boundsVertex->m_x > g_unk0x101246e0) {                                                                 \
				g_unk0x101246e0 = boundsVertex->m_x;                                                                   \
			}                                                                                                          \
			if (boundsVertex->m_y < g_unk0x101246ec) {                                                                 \
				g_unk0x101246ec = boundsVertex->m_y;                                                                   \
			}                                                                                                          \
			if (boundsVertex->m_y > g_unk0x101246e4) {                                                                 \
				g_unk0x101246e4 = boundsVertex->m_y;                                                                   \
			}                                                                                                          \
			boundsVertex++;                                                                                            \
		} while (--boundsCount);                                                                                       \
	}
#else
#define A3D_COMPUTE_BOUNDS(p_vertex, p_count)                                                                          \
	__asm {                                                                                                            \
		__asm mov ebx, p_vertex                                                                                        \
		__asm mov ecx, p_count                                                                                         \
		__asm dec ecx                                                                                                  \
		__asm boundsLoop:                                                                                              \
		__asm fld dword ptr [ebx]                                                                                      \
		__asm fcomp g_unk0x101246e8                                                                                    \
		__asm fnstsw ax                                                                                                \
		__asm test eax, 0x100                                                                                          \
		__asm je boundsMaxX                                                                                            \
		__asm mov eax, [ebx]                                                                                           \
		__asm mov g_unk0x101246e8, eax                                                                                 \
		__asm boundsMaxX:                                                                                              \
		__asm fld dword ptr [ebx]                                                                                      \
		__asm fcomp g_unk0x101246e0                                                                                    \
		__asm fnstsw ax                                                                                                \
		__asm test eax, 0x4100                                                                                         \
		__asm jne boundsMinY                                                                                           \
		__asm mov eax, [ebx]                                                                                           \
		__asm mov g_unk0x101246e0, eax                                                                                 \
		__asm boundsMinY:                                                                                              \
		__asm fld dword ptr [ebx + 4]                                                                                  \
		__asm fcomp g_unk0x101246ec                                                                                    \
		__asm fnstsw ax                                                                                                \
		__asm test eax, 0x100                                                                                          \
		__asm je boundsMaxY                                                                                            \
		__asm mov eax, [ebx + 4]                                                                                       \
		__asm mov g_unk0x101246ec, eax                                                                                 \
		__asm boundsMaxY:                                                                                              \
		__asm fld dword ptr [ebx + 4]                                                                                  \
		__asm fcomp g_unk0x101246e4                                                                                    \
		__asm fnstsw ax                                                                                                \
		__asm test eax, 0x4100                                                                                         \
		__asm jne boundsNext                                                                                           \
		__asm mov eax, [ebx + 4]                                                                                       \
		__asm mov g_unk0x101246e4, eax                                                                                 \
		__asm boundsNext:                                                                                              \
		__asm add ebx, 0x30                                                                                            \
		__asm dec ecx                                                                                                  \
		__asm jne boundsLoop }
#endif

// A3D_LERP for doubles.
#define A3D_LERP_DOUBLE(p_out, p_a, p_b, p_t)                                                                          \
	if ((MechFloat) fabs((p_a) - (p_b)) < 1e-7f) {                                                                     \
		p_out = p_a;                                                                                                   \
	}                                                                                                                  \
	else {                                                                                                             \
		p_out = ((p_b) - (p_a)) * (p_t) + (p_a);                                                                       \
	}

void FUN_1005d0a0(void);
void FUN_1005d140(A3DHeapBlock* p_block);
void FUN_1005d440(A3DHeapBlock* p_block);
void FUN_1005e220(A3DCacheItem* p_item);
A3DHeapBlock* FUN_1005d990(A3DCacheItem* p_item, MechS32 p_defrag);
A3DHeapBlock* FUN_1005dcf0(MechU32 p_size);
A3DHeapBlock* FUN_1005e770(A3DCacheItem* p_item, MechU32 p_size, MechS32 p_defrag, MechS32 p_flush);
A3DHeapBlock* FUN_1005e990(MechU32 p_size);
A3DCacheItem* FUN_1005e2c0(MechS32 p_id, MechS32 p_paletted, MechU32 p_levels);
void FUN_1005ec40(A3DCacheItem* p_item, MechU16* p_pixels, MechU32 p_level, MechS32 p_paletted);
MechU32 FUN_1005f0c0(A3DCacheItem* p_item, MechU16* p_pixels, MechU32 p_level);
void FUN_1005ebb0(A3DCacheItem* p_item);
MechU32 FUN_1005fd70(A3DVertex* p_in, A3DVertex* p_out, MechU32 p_count, MechFloat* p_clip, MechS32 p_flags);
void FUN_10061ca0(void);
void FUN_10066fd0(void);
void FUN_10067080(void);
void FUN_10067170(void);
void FUN_100671a0(void);
void FUN_10066cf0(undefined* p_heap, undefined* p_buffer, MechS32 p_color);
void FUN_10067260(void);
void FUN_10067300(void);
void FUN_100673c0(void);

// Puts the block p_block on the used list p_head, in address order.
__inline static void InsertUsedBlock(A3DHeapBlock** p_head, A3DHeapBlock* p_block)
{
	A3DHeapBlock* cur;

	for (cur = *p_head; cur; cur = cur->m_unk0x08) {
		if (p_block->m_address < cur->m_address) {
			break;
		}
		if (!cur->m_unk0x08) {
			cur->m_unk0x08 = p_block;
			p_block->m_unk0x0c = cur;
			p_block->m_unk0x08 = NULL;
			return;
		}
	}

	if (cur) {
		p_block->m_unk0x08 = cur;
		p_block->m_unk0x0c = cur->m_unk0x0c;
		cur->m_unk0x0c = p_block;
		if (p_block->m_unk0x0c) {
			p_block->m_unk0x0c->m_unk0x08 = p_block;
			return;
		}
	}
	else {
		p_block->m_unk0x08 = NULL;
		p_block->m_unk0x0c = NULL;
	}
	*p_head = p_block;
}

// Puts the block p_block on the list of all blocks p_head, in address order.
__inline static void InsertChainBlock(A3DHeapBlock** p_head, A3DHeapBlock* p_block)
{
	A3DHeapBlock* cur;

	for (cur = *p_head; cur; cur = cur->m_unk0x00) {
		if (p_block->m_address < cur->m_address) {
			break;
		}
		if (!cur->m_unk0x00) {
			cur->m_unk0x00 = p_block;
			p_block->m_unk0x04 = cur;
			p_block->m_unk0x00 = NULL;
			return;
		}
	}

	if (cur) {
		p_block->m_unk0x00 = cur;
		p_block->m_unk0x04 = cur->m_unk0x04;
		cur->m_unk0x04 = p_block;
		if (p_block->m_unk0x04) {
			p_block->m_unk0x04->m_unk0x00 = p_block;
			return;
		}
	}
	else {
		p_block->m_unk0x04 = NULL;
		p_block->m_unk0x00 = NULL;
	}
	*p_head = p_block;
}

// "VRAM_copyfromheap()": copies the levels of p_item from its heap block into the VRAM block p_block.
__inline static void CopyLevelsFromHeap(A3DCacheItem* p_item, A3DHeapBlock* p_block)
{
	MechU32 level;

	g_unk0x100aca98[5] = (undefined4) g_unk0x100acb20;
	g_unk0x100aca98[3] = p_item->m_format;
	for (level = 0; level < p_item->m_levels; level++) {
		g_unk0x100aca98[1] = p_item->m_width / (1 << level);
		g_unk0x100aca98[2] = p_item->m_height / (1 << level);
		g_unk0x100aca98[4] = (undefined4) (p_item->m_unk0x18->m_unk0x20 + p_item->m_offsets[level]);
		g_unk0x100aca98[6] = p_item->m_offsets[level] + p_block->m_address;
		switch (p_item->m_format) {
		case 4:
		case 8:
			g_unk0x100aca98[7] = (undefined4) (p_item->m_unk0x18->m_unk0x20 + p_item->m_unk0x60[level]);
			g_unk0x100aca98[8] = (undefined4) g_unk0x100acb20;
			g_unk0x100aca98[9] = p_item->m_unk0x60[level] + p_block->m_address;
			break;
		case 0xf:
		case 0x10:
		case 0x18:
		case 0x20:
			g_unk0x100aca98[7] = 0;
			g_unk0x100aca98[8] = 0;
			g_unk0x100aca98[9] = 0;
			break;
		}
		if (g_unk0x100aca98[6] & 0x1f) {
			DebugPrint("VRAM_copyfromheap(): Unaligned VRAM raster  cache address!\n");
		}
		if (g_unk0x100aca98[9] & 0x1f) {
			DebugPrint("VRAM_copyfromheap(): Unaligned VRAM palette cache address!\n");
		}
		g_unk0x100ac9d0 = -1;
		msiSetParameters((int) g_unk0x100aca98);
	}
}

// "TEXTURECACHE_finditem": the texture cache's item of p_id (one with no VRAM block stops in the
// debugger).
__inline static A3DCacheItem* TextureCacheFindItem(MechS32 p_id)
{
	A3DCacheItem* item;

	if (p_id >= 0x4000) {
		DebugPrint("TEXTURECACHE_finditem: Out of range ID: %i\n", p_id);
		return NULL;
	}

	item = g_unk0x100c2680[p_id];
	if (item && !item->m_unk0x14) {
#if defined(_MSC_VER) && defined(_M_IX86)
		__asm int 3
#endif
	}

	return item;
}

// "HEAPCACHE_finditem": the heap cache's item of p_id.
__inline static A3DCacheItem* HeapCacheFindItem(MechS32 p_id)
{
	if (p_id >= 0x4000) {
		DebugPrint("HEAPCACHE_finditem: Out of range ID: %i\n", p_id);
		return NULL;
	}

	return g_unk0x100d2680[p_id];
}

// STUB: MW2MATROX 0x1005a5f0
MechU32 FUN_1005a5f0(ProjectedVertex* p_out, ProjectedVertex* p_in, MechU32 p_count, MechS32 p_unk0x10)
{
	STUB(0x1005a5f0);
	return 0;
}

// FUNCTION: MW2MATROX 0x1005d090
void FUN_1005d090(void)
{
	FUN_1005d0a0();
}

// Not matched yet: the original drops the unlink's test of the head (the item came from it).
// FUNCTION: MW2MATROX 0x1005d0a0
void FUN_1005d0a0(void)
{
	A3DCacheItem* item;
	A3DCacheItem* next;

	while ((item = g_unk0x100acb30) != NULL) {
		A3D_UNLINK(item, next, m_heapNext, m_heapPrev, g_unk0x100acb30, g_unk0x100acb34);
		g_unk0x100c2680[item->m_id] = NULL;
		FUN_1005d140(item->m_unk0x14);
		item->m_unk0x14 = NULL;
	}
	g_unk0x100acb38 = 0;
	g_unk0x100acb3c = 0;
}

// Frees the VRAM block p_block: puts it on the free list, merged with a free neighbour.
// Not matched yet: the logic follows the original, the register and stack allocation don't.
// FUNCTION: MW2MATROX 0x1005d140
void FUN_1005d140(A3DHeapBlock* p_block)
{
	A3DHeapBlock* cur;
	A3DHeapBlock* last;
	A3DHeapBlock* prev;
	MechU32 size;

	if (p_block) {
		g_unk0x100acb04--;
		A3D_UNLIST_BLOCK(p_block, g_unk0x100acaf0);
		size = p_block->m_size;
		p_block->m_free = TRUE;
		p_block->m_owner->m_unk0x14 = NULL;
		p_block->m_owner = NULL;
		g_unk0x100acafc += size;

		if (g_unk0x100acaf4) {
			for (cur = g_unk0x100acaf4; cur && cur->m_address <= p_block->m_address; cur = cur->m_unk0x08) {
				last = cur;
			}
			if (cur) {
				if (cur->m_unk0x0c && cur->m_unk0x0c->m_address + cur->m_unk0x0c->m_size == p_block->m_address) {
					cur->m_unk0x0c->m_size += size;
					A3D_UNCHAIN_BLOCK(p_block, prev, g_unk0x100acaf8);
					HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
				}
				else if (p_block->m_address + size == cur->m_address) {
					cur->m_address = p_block->m_address;
					cur->m_size += size;
					A3D_UNCHAIN_BLOCK(p_block, prev, g_unk0x100acaf8);
					HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
				}
				else if (cur) {
					p_block->m_unk0x0c = cur->m_unk0x0c;
					p_block->m_unk0x08 = cur;
					cur->m_unk0x0c = p_block;
					if (p_block->m_unk0x0c) {
						p_block->m_unk0x0c->m_unk0x08 = p_block;
					}
					else {
						g_unk0x100acaf4 = p_block;
					}
				}
				else {
					p_block->m_unk0x08 = NULL;
					p_block->m_unk0x0c = NULL;
					g_unk0x100acaf4 = p_block;
				}
			}
			else if (p_block->m_address + size == last->m_address) {
				last->m_address = p_block->m_address;
				last->m_size += size;
				A3D_UNCHAIN_BLOCK(p_block, prev, g_unk0x100acaf8);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
			}
			else {
				last->m_unk0x08 = p_block;
				p_block->m_unk0x0c = last;
				p_block->m_unk0x08 = NULL;
			}
		}
		else {
			p_block->m_unk0x08 = NULL;
			p_block->m_unk0x0c = NULL;
			g_unk0x100acaf4 = p_block;
		}
		g_unk0x100acb00 = 0;
	}
}

// Frees the heap block p_block, as FUN_1005d140 frees VRAM blocks.
// Not matched yet (as FUN_1005d140): the original keeps size in a register and updates the counters
// in memory; the logic follows it.
// FUNCTION: MW2MATROX 0x1005d440
void FUN_1005d440(A3DHeapBlock* p_block)
{
	A3DHeapBlock* cur;
	A3DHeapBlock* last;
	A3DHeapBlock* prev;
	MechU32 size;

	if (p_block) {
		g_unk0x100acb28--;
		A3D_UNLIST_BLOCK(p_block, g_unk0x100acb0c);
		size = p_block->m_size;
		p_block->m_free = TRUE;
		p_block->m_owner->m_unk0x18 = NULL;
		p_block->m_owner = NULL;
		g_unk0x100acb18 += size;

		if (g_unk0x100acb10) {
			for (cur = g_unk0x100acb10; cur && cur->m_address <= p_block->m_address; cur = cur->m_unk0x08) {
				last = cur;
			}
			if (cur) {
				if (cur->m_unk0x0c && cur->m_unk0x0c->m_address + cur->m_unk0x0c->m_size == p_block->m_address) {
					cur->m_unk0x0c->m_size += size;
					A3D_UNCHAIN_BLOCK(p_block, prev, g_unk0x100acb14);
					HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
				}
				else if (p_block->m_address + size == cur->m_address) {
					cur->m_address = p_block->m_address;
					cur->m_size += size;
					A3D_UNCHAIN_BLOCK(p_block, prev, g_unk0x100acb14);
					HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
				}
				else if (cur) {
					p_block->m_unk0x0c = cur->m_unk0x0c;
					p_block->m_unk0x08 = cur;
					cur->m_unk0x0c = p_block;
					if (p_block->m_unk0x0c) {
						p_block->m_unk0x0c->m_unk0x08 = p_block;
					}
					else {
						g_unk0x100acb10 = p_block;
					}
				}
				else {
					p_block->m_unk0x08 = NULL;
					p_block->m_unk0x0c = NULL;
					g_unk0x100acb10 = p_block;
				}
			}
			else if (p_block->m_address + size == last->m_address) {
				last->m_address = p_block->m_address;
				last->m_size += size;
				A3D_UNCHAIN_BLOCK(p_block, prev, g_unk0x100acb14);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_block);
			}
			else {
				last->m_unk0x08 = p_block;
				p_block->m_unk0x0c = last;
				p_block->m_unk0x08 = NULL;
			}
		}
		else {
			p_block->m_unk0x08 = NULL;
			p_block->m_unk0x0c = NULL;
			g_unk0x100acb10 = p_block;
		}
		g_unk0x100acb1c = 0;
	}
}

// "A3D_LoadTexture()": the texture of the CEL resource p_id, from the texture cache, else from the
// heap cache, else loaded (FUN_1005e2c0's p_paletted and p_levels); made the most recent in both.
// FUNCTION: MW2MATROX 0x1005d600
A3DTexture* FUN_1005d600(MechS32 p_id, MechS32 p_paletted, MechU32 p_levels)
{
	A3DCacheItem* item;
	A3DCacheItem* next;
	A3DCacheItem* oldest;
	A3DCacheItem* last;

	if (p_id >= 0x4000) {
		DebugPrint("A3D_LoadTexture(): Out of range ID: %i\n", p_id);
		return NULL;
	}

	item = TextureCacheFindItem(p_id);
	if (item) {
		g_unk0x100acb38++;
		g_unk0x100acb48++;
		A3D_UNLINK(item, next, m_next, m_prev, g_unk0x100acb40, g_unk0x100acb44);
		g_unk0x100d2680[item->m_id] = NULL;
		A3D_UNLINK(item, next, m_heapNext, m_heapPrev, g_unk0x100acb30, g_unk0x100acb34);
		g_unk0x100c2680[item->m_id] = NULL;
	}
	else {
		g_unk0x100acb3c++;
		item = HeapCacheFindItem(p_id);
		if (item) {
			g_unk0x100acb48++;
			A3D_UNLINK(item, next, m_next, m_prev, g_unk0x100acb40, g_unk0x100acb44);
			g_unk0x100d2680[item->m_id] = NULL;
		}
		else {
			g_unk0x100acb4c++;
			item = FUN_1005e2c0(p_id, p_paletted, p_levels);
			if (!item) {
				DebugPrint("A3D_LoadTexture(): Can't load texture ID: %i\n", p_id);
				return NULL;
			}
		}
		while (!FUN_1005d990(item, TRUE)) {
			oldest = g_unk0x100acb30;
			if (oldest) {
				FUN_1005e220(oldest);
				FUN_1005d140(oldest->m_unk0x14);
				oldest->m_unk0x14 = NULL;
			}
		}
	}

	last = g_unk0x100acb44;
	g_unk0x100acb44 = item;
	item->m_prev = last;
	item->m_next = NULL;
	if (last) {
		last->m_next = item;
	}
	else {
		g_unk0x100acb40 = item;
	}
	g_unk0x100d2680[item->m_id] = item;

	last = g_unk0x100acb34;
	g_unk0x100acb34 = item;
	item->m_heapPrev = last;
	item->m_heapNext = NULL;
	if (last) {
		last->m_heapNext = item;
	}
	else {
		g_unk0x100acb30 = item;
	}
	g_unk0x100c2680[item->m_id] = item;

	return (A3DTexture*) item;
}

// Allocates a VRAM block for p_item (freeing the one it has), defragmenting VRAM if needed
// (p_defrag), and copies the texture's levels into it from the heap.
// Register and scheduling entropy remains (the free block's size is updated in memory).
// FUNCTION: MW2MATROX 0x1005d990
A3DHeapBlock* FUN_1005d990(A3DCacheItem* p_item, MechS32 p_defrag)
{
	A3DHeapBlock* cur;
	A3DHeapBlock* found;
	A3DHeapBlock* block;
	A3DHeapBlock* prev;
	MechU32 size;
	MechU32 remaining;

	FUN_1005d140(p_item->m_unk0x14);
	size = (p_item->m_size + 0x1f) & ~0x1f;
	if (size > g_unk0x100acafc) {
		g_unk0x100acb00 = -2;
		return NULL;
	}

	found = NULL;
	for (cur = g_unk0x100acaf4; cur; cur = cur->m_unk0x08) {
		if (size <= cur->m_size) {
			found = cur;
			if (size == cur->m_size) {
				break;
			}
		}
	}
	if (!found) {
		if (p_defrag) {
			found = FUN_1005dcf0(size);
		}
		else {
			g_unk0x100acb00 = -1;
			return NULL;
		}
	}

	block = (A3DHeapBlock*) HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, 0x20);
	memset(block, 0, 0x20);
	remaining = found->m_size - size;
	block->m_free = FALSE;
	block->m_size = size;
	block->m_owner = p_item;
	found->m_size = remaining;
	block->m_address = found->m_address;
	found->m_address = block->m_address + size;
	if (!remaining) {
		A3D_UNLIST_BLOCK(found, g_unk0x100acaf4);
		A3D_UNCHAIN_BLOCK(found, prev, g_unk0x100acaf8);
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, found);
	}

	InsertUsedBlock(&g_unk0x100acaf0, block);
	InsertChainBlock(&g_unk0x100acaf8, block);

	CopyLevelsFromHeap(block->m_owner, block);
	g_unk0x100acafc -= block->m_size;
	g_unk0x100acb04++;
	p_item->m_unk0x14 = block;
	g_unk0x100acb00 = 0;
	return block;
}

// "VRAM_defrag()": merges free VRAM blocks into the largest one, moving the textures between them,
// until it holds p_size bytes; returns it.
// Not matched yet: register allocation and the order of the level copy's loads differ.
// FUNCTION: MW2MATROX 0x1005dcf0
A3DHeapBlock* FUN_1005dcf0(MechU32 p_size)
{
	A3DHeapBlock* largest;
	A3DHeapBlock* cur;
	A3DHeapBlock* block;
	A3DHeapBlock* prev;
	MechU32 size;
	MechU8 pass;

	g_unk0x100acb08++;
	largest = NULL;
	DebugPrint("VRAM Defragmentation Occured!\n");
	size = 0;
	for (cur = g_unk0x100acaf4; cur; cur = cur->m_unk0x08) {
		if (cur->m_size > size) {
			size = cur->m_size;
			largest = cur;
		}
	}

	for (pass = 0; largest->m_size < p_size; pass++) {
		if (pass & 1) {
			for (cur = largest->m_unk0x00; cur && !cur->m_free; cur = cur->m_unk0x00) {
			}
			if (cur) {
				for (block = cur->m_unk0x04; block != largest; block = block->m_unk0x04) {
					block->m_address += cur->m_size;
					CopyLevelsFromHeap(block->m_owner, block);
				}
				largest->m_size += cur->m_size;
				A3D_UNLIST_BLOCK(cur, g_unk0x100acaf4);
				A3D_UNCHAIN_BLOCK(cur, prev, g_unk0x100acaf8);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, cur);
			}
		}
		else {
			for (cur = largest->m_unk0x04; cur && !cur->m_free; cur = cur->m_unk0x04) {
			}
			if (cur) {
				for (block = cur->m_unk0x00; block != largest; block = block->m_unk0x00) {
					block->m_address -= cur->m_size;
					CopyLevelsFromHeap(block->m_owner, block);
				}
				largest->m_size += cur->m_size;
				largest->m_address -= cur->m_size;
				A3D_UNLIST_BLOCK(cur, g_unk0x100acaf4);
				A3D_UNCHAIN_BLOCK(cur, prev, g_unk0x100acaf8);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, cur);
			}
		}
		if (!largest->m_unk0x00 && !largest->m_unk0x04) {
			DebugPrint("VRAM_defrag() error!\n");
			return NULL;
		}
	}

	return largest;
}

// Takes p_item off the texture cache (the items in VRAM; their list is m_heapNext and m_heapPrev).
// Register entropy: next and the address of p_item->m_heapPrev take each other's registers.
// FUNCTION: MW2MATROX 0x1005e220
void FUN_1005e220(A3DCacheItem* p_item)
{
	A3DCacheItem* prev;
	A3DCacheItem* next;

	next = p_item->m_heapNext;
	if (!next && !p_item->m_heapPrev) {
		if (g_unk0x100acb30 == p_item) {
			g_unk0x100acb30 = NULL;
		}
		if (g_unk0x100acb34 == p_item) {
			g_unk0x100acb34 = NULL;
		}
		g_unk0x100c2680[p_item->m_id] = NULL;
		return;
	}

	if (next) {
		next->m_heapPrev = p_item->m_heapPrev;
	}
	else {
		g_unk0x100acb34 = p_item->m_heapPrev;
	}
	prev = p_item->m_heapPrev;
	if (prev) {
		prev->m_heapNext = next;
	}
	else {
		g_unk0x100acb30 = next;
	}
	p_item->m_heapNext = NULL;
	g_unk0x100c2680[p_item->m_id] = NULL;
	p_item->m_heapPrev = NULL;
}

// "HEAPCACHE_loaditem()": loads the CEL resource p_id into a new item's heap block, with p_levels
// levels (p_paletted: 8-bit with a palette per level, else 16-bit).
// Not matched yet: register allocation and block order differ; the calls and stores follow the
// original.
// FUNCTION: MW2MATROX 0x1005e2c0
A3DCacheItem* FUN_1005e2c0(MechS32 p_id, MechS32 p_paletted, MechU32 p_levels)
{
	A3DCacheItem* item;
	A3DCacheItem* oldest;
	A3DCacheItem* next;
	MechS16* data;
	MechS32 resourceSize;
	MechU32 size;
	MechU32 bytes;
	MechU32 level;

	if (p_id == 0x29b6) {
		p_levels = 1;
		size = 0x2000;
	}
	else {
		resourceSize = GetPrjResourceSize(g_mw2PrjHandle, g_resourceTypeTags[c_resTagCel], p_id);
		if (resourceSize < 0) {
			DebugPrint("HEAPCACHE_loaditem(): GetIndexedItemSize() failed on ID #%i\n", p_id);
			return NULL;
		}
		if (p_paletted) {
			size = 0;
			bytes = (resourceSize - 4) >> 1;
			for (level = p_levels; level; level--) {
				size = ((((size + 0x1f) & ~0x1f) + bytes + 0x1f) & ~0x1f) + 0x200;
				bytes >>= 2;
				if (!bytes) {
					break;
				}
			}
		}
		else {
			size = 0;
			bytes = (resourceSize - 4) & ~1;
			for (level = p_levels; level; level--) {
				size = ((size + 0x1f) & ~0x1f) + bytes;
				bytes >>= 2;
				if (!bytes) {
					break;
				}
			}
			size += 4;
		}
	}

	item = (A3DCacheItem*) HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(A3DCacheItem));
	if (item) {
		memset(item, 0, sizeof(A3DCacheItem));
	}
	item->m_unk0x18 = FUN_1005e770(item, size, TRUE, TRUE);
	if (!item->m_unk0x18) {
		g_unk0x100ac9d0 = -1;
		g_unk0x100acb2c++;
		msiSetParameters(0);
		msiSetParameters(-1);
		while (!item->m_unk0x18) {
			oldest = g_unk0x100acb40;
			if (oldest) {
				FUN_1005e220(oldest);
				FUN_1005d140(oldest->m_unk0x14);
				FUN_1005d440(oldest->m_unk0x18);
				FUN_1005ebb0(oldest);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, oldest);
				g_unk0x100acb50--;
			}
			item->m_unk0x18 = FUN_1005e770(item, size, TRUE, FALSE);
		}
	}

	if (p_id == 0x29b6) {
		data = (MechS16*) item->m_unk0x18->m_address;
		memcpy(data, g_unk0x100e26e0, size);
		item->m_format = 0xf;
		item->m_id = 0x29b6;
		item->m_unk0x18->m_unk0x20 = (undefined*) data;
		item->m_width = 0x40;
		item->m_height = 0x40;
		g_unk0x100acb50++;
		return item;
	}

	if (p_paletted) {
		data = (MechS16*) HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, resourceSize);
		if (ReadPrjResource(g_mw2PrjHandle, g_resourceTypeTags[c_resTagCel], p_id, data) >= 0) {
			item->m_id = p_id;
			item->m_width = data[0];
			item->m_height = data[1];
			item->m_levels = p_levels;
			item->m_unk0x38 = 0;
			item->m_unk0x18->m_unk0x20 = (undefined*) item->m_unk0x18->m_address;
			FUN_1005ec40(item, (MechU16*) (data + 2), 0, p_paletted);
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
			g_unk0x100acb50++;
			return item;
		}

		DebugPrint("HEAPCACHE_loaditem(): RetrieveByIndex() failed on ID #%i\n", p_id);
		if (item) {
			A3D_UNLINK(item, next, m_heapNext, m_heapPrev, g_unk0x100acb30, g_unk0x100acb34);
			g_unk0x100c2680[item->m_id] = NULL;
			FUN_1005d140(item->m_unk0x14);
			FUN_1005d440(item->m_unk0x18);
			FUN_1005ebb0(item);
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, item);
			g_unk0x100acb50--;
		}
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
		return NULL;
	}

	data = (MechS16*) item->m_unk0x18->m_address;
	if (ReadPrjResource(g_mw2PrjHandle, g_resourceTypeTags[c_resTagCel], p_id, data) >= 0) {
		item->m_format = 0xf;
		item->m_id = p_id;
		item->m_width = data[0];
		item->m_height = data[1];
		item->m_levels = p_levels;
		item->m_unk0x28 = 0;
		item->m_unk0x2c = 1;
		item->m_unk0x38 = 0;
		item->m_unk0x18->m_unk0x20 = (undefined*) (item->m_unk0x18->m_address + 4);
		FUN_1005ec40(item, (MechU16*) (data + 2), 0, 0);
		g_unk0x100acb50++;
		return item;
	}

	DebugPrint("HEAPCACHE_loaditem(): RetrieveByIndex() failed on ID #%i\n", p_id);
	if (item) {
		A3D_UNLINK(item, next, m_heapNext, m_heapPrev, g_unk0x100acb30, g_unk0x100acb34);
		g_unk0x100c2680[item->m_id] = NULL;
		FUN_1005d140(item->m_unk0x14);
		FUN_1005d440(item->m_unk0x18);
		FUN_1005ebb0(item);
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, item);
		g_unk0x100acb50--;
	}
	return NULL;
}

// Allocates a heap block of p_size bytes for p_item, defragmenting the heap if needed (p_defrag;
// p_flush flushes the card first).
// Register entropy: the search loop's and the insertions' registers swap, and the counters
// update through registers.
// FUNCTION: MW2MATROX 0x1005e770
A3DHeapBlock* FUN_1005e770(A3DCacheItem* p_item, MechU32 p_size, MechS32 p_defrag, MechS32 p_flush)
{
	A3DHeapBlock* cur;
	A3DHeapBlock* found;
	A3DHeapBlock* block;
	A3DHeapBlock* prev;
	MechU32 size;
	MechU32 remaining;

	size = (p_size + 0xff) & ~0xff;
	if (size > g_unk0x100acb18) {
		g_unk0x100acb1c = -2;
		return NULL;
	}

	found = NULL;
	for (cur = g_unk0x100acb10; cur; cur = cur->m_unk0x08) {
		if (size <= cur->m_size) {
			found = cur;
			if (size == cur->m_size) {
				break;
			}
		}
	}
	if (!found) {
		if (p_defrag) {
			if (p_flush) {
				g_unk0x100ac9d0 = -1;
				g_unk0x100acb2c++;
				msiSetParameters(0);
				msiSetParameters(-1);
			}
			found = FUN_1005e990(size);
		}
		else {
			g_unk0x100acb1c = -1;
			return NULL;
		}
	}

	block = (A3DHeapBlock*) HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(A3DHeapBlock));
	memset(block, 0, sizeof(A3DHeapBlock));
	remaining = found->m_size - size;
	block->m_free = FALSE;
	block->m_owner = p_item;
	block->m_size = size;
	block->m_address = found->m_address;
	found->m_size = remaining;
	found->m_address = block->m_address + size;
	if (!remaining) {
		A3D_UNLIST_BLOCK(found, g_unk0x100acb10);
		A3D_UNCHAIN_BLOCK(found, prev, g_unk0x100acb14);
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, found);
	}

	InsertUsedBlock(&g_unk0x100acb0c, block);
	InsertChainBlock(&g_unk0x100acb14, block);
	g_unk0x100acb18 -= block->m_size;
	g_unk0x100acb28++;
	p_item->m_unk0x18 = block;
	g_unk0x100acb1c = 0;
	return block;
}

// "HEAP_defrag()": merges free heap blocks into the largest one, moving the data between them, until
// it holds p_size bytes; returns it.
// Not matched yet: register allocation of the defragmentation loop differs.
// FUNCTION: MW2MATROX 0x1005e990
A3DHeapBlock* FUN_1005e990(MechU32 p_size)
{
	A3DHeapBlock* largest;
	A3DHeapBlock* cur;
	A3DHeapBlock* block;
	A3DHeapBlock* prev;
	MechU32 size;
	MechU8 pass;

	g_unk0x100acb24++;
	largest = NULL;
	DebugPrint("HEAP Defragmentation Occured!\n");
	size = 0;
	for (cur = g_unk0x100acb10; cur; cur = cur->m_unk0x08) {
		if (cur->m_size > size) {
			size = cur->m_size;
			largest = cur;
		}
	}

	for (pass = 0; largest->m_size < p_size; pass++) {
		if (pass & 1) {
			for (cur = largest->m_unk0x00; cur && !cur->m_free; cur = cur->m_unk0x00) {
			}
			if (cur) {
				for (block = cur->m_unk0x04; block != largest; block = block->m_unk0x04) {
					memmove(
						(undefined*) (cur->m_size + block->m_address),
						(undefined*) block->m_address,
						block->m_size
					);
					block->m_address += cur->m_size;
					block->m_unk0x20 += cur->m_size;
				}
				largest->m_size += cur->m_size;
				A3D_UNLIST_BLOCK(cur, g_unk0x100acb10);
				A3D_UNCHAIN_BLOCK(cur, prev, g_unk0x100acb14);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, cur);
			}
		}
		else {
			for (cur = largest->m_unk0x04; cur && !cur->m_free; cur = cur->m_unk0x04) {
			}
			if (cur) {
				for (block = cur->m_unk0x00; block != largest; block = block->m_unk0x00) {
					memmove(
						(undefined*) (block->m_address - cur->m_size),
						(undefined*) block->m_address,
						block->m_size
					);
					block->m_address -= cur->m_size;
					block->m_unk0x20 -= cur->m_size;
				}
				largest->m_size += cur->m_size;
				largest->m_address -= cur->m_size;
				A3D_UNLIST_BLOCK(cur, g_unk0x100acb10);
				A3D_UNCHAIN_BLOCK(cur, prev, g_unk0x100acb14);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, cur);
			}
		}
		if (!largest->m_unk0x00 && !largest->m_unk0x04) {
			DebugPrint("HEAP_defrag() error!\n");
			return NULL;
		}
	}

	return largest;
}

// Takes p_item off the heap cache (the items in the heap).
// Register entropy: next and the address of p_item->m_prev take each other's registers.
// FUNCTION: MW2MATROX 0x1005ebb0
void FUN_1005ebb0(A3DCacheItem* p_item)
{
	A3DCacheItem* prev;
	A3DCacheItem* next;

	next = p_item->m_next;
	if (!next && !p_item->m_prev) {
		if (g_unk0x100acb40 == p_item) {
			g_unk0x100acb40 = NULL;
		}
		if (g_unk0x100acb44 == p_item) {
			g_unk0x100acb44 = NULL;
		}
		g_unk0x100d2680[p_item->m_id] = NULL;
		return;
	}

	if (next) {
		next->m_prev = p_item->m_prev;
	}
	else {
		g_unk0x100acb44 = p_item->m_prev;
	}
	prev = p_item->m_prev;
	if (prev) {
		prev->m_next = next;
	}
	else {
		g_unk0x100acb40 = next;
	}
	p_item->m_next = NULL;
	g_unk0x100d2680[p_item->m_id] = NULL;
	p_item->m_prev = NULL;
}

// Averages the 2x2 blocks of the p_width x p_width 16-bit picture p_pixels into p_out (p_width / 2
// pixels square), with the top bit set.
#define A3D_SHRINK(p_pixels, p_out, p_width, p_half, p_row0, p_row1, p_dest, p_y, p_x, p_i, p_a, p_b, p_c, p_d)        \
	p_row0 = p_pixels;                                                                                                 \
	p_row1 = p_pixels + p_width;                                                                                       \
	for (p_y = p_half; p_y; p_y--) {                                                                                   \
		p_dest = p_out;                                                                                                \
		p_i = 0;                                                                                                       \
		for (p_x = p_half; p_x; p_x--) {                                                                               \
			p_a = p_row0[p_i];                                                                                         \
			p_b = p_row0[p_i + 1];                                                                                     \
			p_c = p_row1[p_i];                                                                                         \
			p_d = p_row1[p_i + 1];                                                                                     \
			*p_dest++ =                                                                                                \
				0x8000 |                                                                                               \
				((((p_a & 0x7c00) >> 10) + ((p_b & 0x7c00) >> 10) + ((p_c & 0x7c00) >> 10) + ((p_d & 0x7c00) >> 10)) / \
				 4) << 10 |                                                                                            \
				((((p_a & 0x3e0) >> 5) + ((p_b & 0x3e0) >> 5) + ((p_c & 0x3e0) >> 5) + ((p_d & 0x3e0) >> 5)) / 4)      \
					<< 5 |                                                                                             \
				((p_a & 0x1f) + (p_b & 0x1f) + (p_c & 0x1f) + (p_d & 0x1f)) / 4;                                       \
			p_i += 2;                                                                                                  \
		}                                                                                                              \
		p_row0 += p_width * 2;                                                                                         \
		p_row1 += p_width * 2;                                                                                         \
		p_out += p_half;                                                                                               \
	}

// Builds the levels of p_item from its level p_level, p_pixels: 16-bit levels in the heap block,
// each after the last, or (p_paletted) each level quantized by FUN_1005f0c0.
// Not matched yet (38%): the original keeps the 2x2 filter's index in esi over row pointers it
// reloads; this build strength-reduces them into walking pointers.
// FUNCTION: MW2MATROX 0x1005ec40
void FUN_1005ec40(A3DCacheItem* p_item, MechU16* p_pixels, MechU32 p_level, MechS32 p_paletted)
{
	MechU32 width;
	MechU32 half;
	MechU32 bytes;
	MechU32 y;
	MechU32 x;
	MechU32 i;
	MechU16* row0;
	MechU16* row1;
	MechU16* dest;
	MechU16* destRow;
	MechU16* next;
	MechU16 a;
	MechU16 b;
	MechU16 c;
	MechU16 d;

	if (!p_level) {
		p_item->m_offsets[p_level] = 0;
		p_item->m_size = 0;
	}
	width = p_item->m_width / (MechU32) (1 << p_level);

	if (p_paletted) {
		FUN_1005f0c0(p_item, p_pixels, p_level);
		p_level++;
		if (width == 1) {
			p_item->m_levels = p_level;
		}
		if (p_level < p_item->m_levels) {
			half = width >> 1;
			next = (MechU16*) HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, (width & ~1) * half);
			destRow = next;
			A3D_SHRINK(p_pixels, destRow, width, half, row0, row1, dest, y, x, i, a, b, c, d);
			FUN_1005ec40(p_item, next, p_level, p_paletted);
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, next);
		}
	}
	else {
		bytes = (width * width * 2 + 0x1f) & ~0x1f;
		p_level++;
		p_item->m_size += bytes;
		if (width == 1) {
			p_item->m_levels = p_level;
		}
		if (p_level < p_item->m_levels) {
			next = (MechU16*) ((undefined*) p_pixels + bytes);
			p_item->m_offsets[p_level] = bytes;
			half = width >> 1;
			destRow = next;
			A3D_SHRINK(p_pixels, destRow, width, half, row0, row1, dest, y, x, i, a, b, c, d);
			FUN_1005ec40(p_item, next, p_level, p_paletted);
		}
	}
}

// Quantizes the level p_level of p_item, p_pixels, to a palette of 16 or 256 colors; returns the
// color count.
// STUB: MW2MATROX 0x1005f0c0
MechU32 FUN_1005f0c0(A3DCacheItem* p_item, MechU16* p_pixels, MechU32 p_level)
{
	STUB(0x1005f0c0);
	return 0;
}

// FUNCTION: MW2MATROX 0x1005f6e0
void FUN_1005f6e0(void)
{
	g_unk0x100acb58++;
	if (g_unk0x100ac904) {
		g_unk0x100ac9d0 = -1;
		msiSetParameters(-1);
		g_unk0x100ac904 = 0;
	}
}

// FUNCTION: MW2MATROX 0x1005f710
void FUN_1005f710(void)
{
	if (!g_unk0x100ac900) {
		g_unk0x100ac900 = 1;
		FUN_1005f6e0();
		msiStartFrame(g_unk0x100ac90c, g_unk0x100ac910, g_unk0x100ac914, g_unk0x100ac918, 0, 0);
		g_unk0x100ac9d0 = -1;
	}
}

// FUNCTION: MW2MATROX 0x1005f760
void FUN_1005f760(void)
{
	if (g_unk0x100ac900) {
		g_unk0x100ac900 = 0;
		FUN_1005f6e0();
		msiEndFrame(0, 0, 0);
	}
}

// Register entropy: ecx and edx swapped (the probe that defines the table in this unit matches).
// FUNCTION: MW2MATROX 0x1005f790
void FUN_1005f790(PaletteColor* p_palette)
{
	MechS32 i;

	for (i = 0; i < 0x100; i++) {
		g_paletteRgb[i][0] = (MechFloat) (p_palette->m_red << 2);
		g_paletteRgb[i][1] = (MechFloat) (p_palette->m_green << 2);
		g_paletteRgb[i][2] = (MechFloat) (p_palette->m_blue << 2);
		p_palette++;
	}
}

// FUNCTION: MW2MATROX 0x1005f810
void FUN_1005f810(MechS32 p_filter)
{
}

// FUNCTION: MW2MATROX 0x1005f820
void FUN_1005f820(MechS32 p_clear, MechU32 p_color)
{
	g_unk0x100ac90c = p_clear;
	g_unk0x100ac910 = (MechFloat) ((p_color & 0xf800) >> 11);
	g_unk0x100ac914 = (MechFloat) ((p_color & 0x5e0) >> 5);
	g_unk0x100ac918 = (MechFloat) (p_color & 0x1f);
}

// Clips the polygon p_vertices of p_count vertices to the rectangle p_clip (left, right, top,
// bottom) into p_out; returns the clipped polygon's vertex count.
__inline MechS32 A3DClipPolygon(
	A3DVertex* p_vertices,
	A3DVertex* p_out,
	MechS32 p_count,
	MechFloat* p_clip,
	MechS32 p_flags
)
{
	A3DVertex* vertex;

	vertex = p_vertices;
	g_unk0x101246e0 = g_unk0x101246e8 = vertex->m_x;
	g_unk0x101246e4 = g_unk0x101246ec = vertex->m_y;
	vertex++;
	A3D_COMPUTE_BOUNDS(vertex, p_count);

	if (g_unk0x101246e8 < p_clip[0] || g_unk0x101246e0 > p_clip[1] || g_unk0x101246ec < p_clip[2] ||
		g_unk0x101246e4 > p_clip[3]) {
		if (g_unk0x101246e8 > p_clip[1] || g_unk0x101246e0 < p_clip[0] || g_unk0x101246ec > p_clip[3] ||
			g_unk0x101246e4 < p_clip[2]) {
			return 0;
		}
		return FUN_1005fd70(p_vertices, p_out, p_count, p_clip, p_flags);
	}

	A3D_COPY_VERTICES(p_vertices, p_out, p_count);
	return p_count;
}

// Register allocation and stack slots differ, and the clip call reloads its first argument from
// memory in the original.
// FUNCTION: MW2MATROX 0x1005f8a0
void FUN_1005f8a0(PANE* p_pane, MechS32 p_color)
{
	MechFloat red;
	MechFloat green;
	MechFloat blue;
	MechFloat x0;
	MechFloat y0;
	MechFloat x1;
	MechFloat y1;
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;
	MechS32 width;
	MechS32 count;
	MechS32 i;
	A3DVertex* vertex;
	MechFloat clip[4];
	A3DVertex corners[4];
	A3DVertex polygon[0x40];

	red = (MechFloat) ((p_color & 0xf800) >> 8);
	green = (MechFloat) ((p_color & 0x7e0) >> 3);
	blue = (MechFloat) ((p_color & 0x1f) << 3);

	x0 = (MechFloat) p_pane->m_x0;
	y0 = (MechFloat) p_pane->m_y0;
	corners[0].m_x = x0;
	corners[0].m_y = y0;
	corners[0].m_z = 0.0f;
	corners[0].m_red = red;
	corners[0].m_green = green;
	corners[0].m_blue = blue;
	x1 = (MechFloat) p_pane->m_x1 + 1.0f;
	corners[1].m_x = x1;
	corners[1].m_y = y0;
	corners[1].m_z = 0.0f;
	corners[1].m_red = red;
	corners[1].m_green = green;
	corners[1].m_blue = blue;
	y1 = (MechFloat) p_pane->m_y1 + 1.0f;
	corners[2].m_x = x1;
	corners[2].m_y = y1;
	corners[2].m_z = 0.0f;
	corners[2].m_red = red;
	corners[2].m_green = green;
	corners[2].m_blue = blue;
	corners[3].m_x = x0;
	corners[3].m_y = y1;
	corners[3].m_z = 0.0f;
	corners[3].m_red = red;
	corners[3].m_green = green;
	corners[3].m_blue = blue;

	if (g_unk0x100ac9d0 != 1) {
		g_unk0x100ac9d0 = 1;
		msiSetParameters((int) g_unk0x100aca40);
		g_unk0x100ac904 = 0;
	}

	top = p_pane->m_y0;
	bottom = p_pane->m_y1 + 1;
	right = p_pane->m_x1 + 1;
	left = p_pane->m_x0;
	if (left < 0) {
		left = 0;
	}
	width = (MechS32) g_unk0x100ac938;
	if (width < right) {
		right = width;
	}
	if (top < 0) {
		top = 0;
	}
	if ((MechS32) g_unk0x100ac93c < bottom) {
		bottom = (MechS32) g_unk0x100ac93c;
	}
	clip[0] = (MechFloat) left;
	clip[1] = (MechFloat) right;
	clip[2] = (MechFloat) top;
	clip[3] = (MechFloat) bottom;

	count = A3DClipPolygon(corners, polygon, 4, clip, 0);
	if (count >= 3) {
		vertex = &polygon[1];
		for (i = count - 2; i; i--) {
			msiRenderTriangle(polygon, vertex, vertex + 1, 100);
			vertex++;
		}
	}
}

// Clips the polygon p_in of p_count vertices to the rectangle p_clip (left, right, top, bottom) into
// p_out; returns the clipped polygon's vertex count.
// STUB: MW2MATROX 0x1005fd70
MechU32 FUN_1005fd70(A3DVertex* p_in, A3DVertex* p_out, MechU32 p_count, MechFloat* p_clip, MechS32 p_flags)
{
	STUB(0x1005fd70);
	return 0;
}

// Interpolates the vertices p_a and p_b at p_t into p_out, past a clip edge on x: the colors and
// the texture coordinates, and with p_perspective, m_w and the depth from it.
// FUNCTION: MW2MATROX 0x10061510
void FUN_10061510(A3DVertex* p_out, A3DVertex* p_a, A3DVertex* p_b, MechDouble p_t, MechS32 p_perspective)
{
	if (!p_perspective) {
		A3D_LERP(p_out->m_red, p_a->m_red, p_b->m_red, p_t)
		A3D_LERP(p_out->m_green, p_a->m_green, p_b->m_green, p_t)
		A3D_LERP(p_out->m_blue, p_a->m_blue, p_b->m_blue, p_t)
		A3D_LERP(p_out->m_u, p_a->m_u, p_b->m_u, p_t)
		A3D_LERP(p_out->m_v, p_a->m_v, p_b->m_v, p_t)
		p_out->m_z = 0.0f;
	}
	else {
		A3D_LERP(p_out->m_u, p_a->m_u, p_b->m_u, p_t)
		A3D_LERP(p_out->m_v, p_a->m_v, p_b->m_v, p_t)
		A3D_LERP(p_out->m_red, p_a->m_red, p_b->m_red, p_t)
		A3D_LERP(p_out->m_green, p_a->m_green, p_b->m_green, p_t)
		A3D_LERP(p_out->m_blue, p_a->m_blue, p_b->m_blue, p_t)
		A3D_LERP(p_out->m_w, p_a->m_w, p_b->m_w, p_t)
		p_out->m_z = g_eyepoint->m_projectScaleX / p_out->m_w;
	}
}

// Draws the outline of a polygon in the color of its first vertex.
// Register allocation and stack slots differ.
// FUNCTION: MW2MATROX 0x10061890
void FUN_10061890(PANE* p_pane, A3DVertex* p_vertices, MechU32 p_count)
{
	MechU32 color;
	MechFloat dx;
	MechFloat dy;
	A3DVertex* vertex;
	A3DVertex* last;
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;
	MechFloat clip[4];
	A3DVertex polygon[0x40];

	if (p_count >= 3) {
		color = ((((MechU32) p_vertices->m_red & ~7) << 6 | ((MechU32) p_vertices->m_green & ~7)) << 2 |
				 (MechU32) p_vertices->m_blue >> 3) &
				0x7fff;

		left = p_pane->m_x0;
		if (left || p_pane->m_y0) {
			dx = (MechFloat) left;
			dy = (MechFloat) p_pane->m_y0;
			for (vertex = p_vertices; vertex < p_vertices + p_count; vertex++) {
				vertex->m_x += dx;
				vertex->m_y += dy;
			}
		}

		bottom = p_pane->m_y1;
		top = p_pane->m_y0;
		right = p_pane->m_x1;
		if (left < 0) {
			left = 0;
		}
		if ((MechS32) g_unk0x100ac938 < right) {
			right = (MechS32) g_unk0x100ac938;
		}
		if (top < 0) {
			top = 0;
		}
		if ((MechS32) g_unk0x100ac93c < bottom) {
			bottom = (MechS32) g_unk0x100ac93c;
		}
		clip[0] = (MechFloat) left;
		clip[1] = (MechFloat) right;
		clip[2] = (MechFloat) top;
		clip[3] = (MechFloat) bottom;

		p_count = A3DClipPolygon(p_vertices, polygon, p_count, clip, 0);
		if (p_count >= 3) {
			last = &polygon[p_count - 1];
			for (vertex = polygon; vertex < last; vertex++) {
				msiDrawSingleLine(color, A3D_POINT(vertex), A3D_POINT(vertex + 1), -1);
			}
			msiDrawSingleLine(color, A3D_POINT(&polygon[p_count - 1]), A3D_POINT(polygon), -1);
		}
	}
}

// FUNCTION: MW2MATROX 0x10061c90
void FUN_10061c90(void)
{
}

// FUNCTION: MW2MATROX 0x10061ca0
void FUN_10061ca0(void)
{
}

// Draws a shaded polygon.
// Register allocation, the offset loop's scheduling and the clip test's operand order differ.
// FUNCTION: MW2MATROX 0x10061cb0
void FUN_10061cb0(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices)
{
	MechFloat dx;
	MechFloat dy;
	A3DVertex* vertex;
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;
	MechS32 i;
	MechFloat clip[4];
	A3DVertex polygon[0x40];

	if (p_count >= 3) {
		if (g_unk0x100ac9d0 != 1) {
			g_unk0x100ac9d0 = 1;
			msiSetParameters((int) g_unk0x100aca40);
			g_unk0x100ac904 = 0;
		}

		left = p_pane->m_x0;
		if (left || p_pane->m_y0) {
			dx = (MechFloat) left;
			dy = (MechFloat) p_pane->m_y0;
			for (vertex = p_vertices; vertex < p_vertices + p_count; vertex++) {
				vertex->m_x += dx;
				vertex->m_y += dy;
			}
		}

		top = p_pane->m_y0;
		bottom = p_pane->m_y1 + 1;
		right = p_pane->m_x1 + 1;
		if (left < 0) {
			left = 0;
		}
		if ((MechS32) g_unk0x100ac938 < right) {
			right = (MechS32) g_unk0x100ac938;
		}
		if (top < 0) {
			top = 0;
		}
		if ((MechS32) g_unk0x100ac93c < bottom) {
			bottom = (MechS32) g_unk0x100ac93c;
		}
		clip[0] = (MechFloat) left;
		clip[1] = (MechFloat) right;
		clip[2] = (MechFloat) top;
		clip[3] = (MechFloat) bottom;

		p_count = A3DClipPolygon(p_vertices, polygon, p_count, clip, 0);
		if (p_count >= 3) {
			vertex = &polygon[1];
			for (i = p_count - 2; i; i--) {
				msiRenderTriangle(polygon, vertex, vertex + 1, 100);
				vertex++;
			}
		}
	}
}

// Register allocation and store scheduling differ (the parameter block's setup).
// FUNCTION: MW2MATROX 0x10062010
void FUN_10062010(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, A3DTexture* p_texture, MechU32 p_flags)
{
	MechS32 shade;
	MechS32 transparent;
	MechU32 scale;
	MechFloat dx;
	MechFloat dy;
	A3DVertex* vertex;
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;
	MechS32 minU;
	MechS32 minV;
	MechU32 i;
	MechFloat clip[4];
	A3DVertex polygon[0x40];

	if (p_count < 3) {
		return;
	}
	if (!p_texture) {
		DebugPrint("A3D_map_polygon(): NULL textureptr passed\n");
		return;
	}

	transparent = (p_flags & 2) >> 1;
	shade = p_flags & 1;
	if (g_unk0x100ac9d0 != 2 || p_texture->m_unk0x10 != g_unk0x100ac9d4 ||
		(MechS32) p_texture->m_unk0x14->m_address != g_unk0x100ac9dc || transparent != g_unk0x100ac9e4 ||
		shade != g_unk0x100ac9e0 || p_texture->m_unk0x38 != g_unk0x100ac9d8) {
		g_unk0x100ac9d4 = p_texture->m_unk0x10;
		g_unk0x100ac9e0 = shade;
		g_unk0x100ac9e4 = transparent;
		g_unk0x100ac9dc = p_texture->m_unk0x14->m_address;
		g_unk0x100ac9d8 = p_texture->m_unk0x38;
		scale = 1 << p_texture->m_unk0x38;
		g_unk0x100ac9d0 = 2;
		if (transparent) {
			g_unk0x100acad0 = 1;
			switch (p_texture->m_unk0x24) {
			case 4:
				if (p_texture->m_unk0x2c) {
					g_unk0x100acad4 = 0;
					g_unk0x100acad6 = 0xf;
				}
				else {
					g_unk0x100acad4 = 0;
					g_unk0x100acad0 = 0;
					g_unk0x100acad6 = 0;
				}
				break;
			case 8:
				if (p_texture->m_unk0x2c) {
					g_unk0x100acad4 = 0;
					g_unk0x100acad6 = 0xff;
				}
				else {
					g_unk0x100acad4 = 0;
					g_unk0x100acad0 = 0;
					g_unk0x100acad6 = 0;
				}
				break;
			case 0xf:
				g_unk0x100acad4 = 0;
				g_unk0x100acad6 = 0x8000;
				break;
			case 0x10:
				g_unk0x100acad4 = 0;
				g_unk0x100acad0 = 0;
				g_unk0x100acad6 = 0;
				break;
			}
		}
		else {
			g_unk0x100acad4 = 0;
			g_unk0x100acad0 = 0;
			g_unk0x100acad6 = 0;
		}

		g_unk0x100acac8 = shade;
		g_unk0x100aca98[1] = p_texture->m_unk0x1c / scale;
		g_unk0x100aca98[2] = p_texture->m_unk0x20 / scale;
		g_unk0x100aca98[3] = p_texture->m_unk0x24;
		g_unk0x100aca98[4] = 0;
		g_unk0x100aca98[5] = 0;
		g_unk0x100aca98[6] = p_texture->m_unk0x3c[p_texture->m_unk0x38] + p_texture->m_unk0x14->m_address;
		if (p_texture->m_unk0x24 >= 0xf) {
			g_unk0x100aca98[7] = 0;
			g_unk0x100aca98[8] = 0;
			g_unk0x100aca98[9] = 0;
		}
		else {
			g_unk0x100aca98[7] = 0;
			g_unk0x100aca98[8] = 0;
			g_unk0x100aca98[9] = p_texture->m_unk0x60[p_texture->m_unk0x38] + p_texture->m_unk0x14->m_address;
		}
		msiSetParameters((int) g_unk0x100aca98);
		g_unk0x100ac904 = 0;
	}

	left = p_pane->m_x0;
	if (left || p_pane->m_y0) {
		dx = (MechFloat) left;
		dy = (MechFloat) p_pane->m_y0;
		for (vertex = p_vertices; vertex < p_vertices + p_count; vertex++) {
			vertex->m_x += dx;
			vertex->m_y += dy;
		}
	}

	top = p_pane->m_y0;
	bottom = p_pane->m_y1 + 1;
	right = p_pane->m_x1 + 1;
	if (left < 0) {
		left = 0;
	}
	if ((MechS32) g_unk0x100ac938 < right) {
		right = (MechS32) g_unk0x100ac938;
	}
	if (top < 0) {
		top = 0;
	}
	if ((MechS32) g_unk0x100ac93c < bottom) {
		bottom = (MechS32) g_unk0x100ac93c;
	}
	clip[0] = (MechFloat) left;
	clip[1] = (MechFloat) right;
	clip[2] = (MechFloat) top;
	clip[3] = (MechFloat) bottom;

	p_count = A3DClipPolygon(p_vertices, polygon, p_count, clip, 0);
	if (p_count >= 3) {
		for (i = 0; i < p_count; i++) {
			polygon[i].m_z = 0.0f;
			polygon[i].m_w = 1.0f;
			polygon[i].m_unk0x1c = polygon[i].m_red;
			polygon[i].m_unk0x20 = polygon[i].m_green;
			polygon[i].m_unk0x24 = polygon[i].m_blue;
		}

		minU = (MechS32) polygon[0].m_u;
		minV = (MechS32) polygon[0].m_v;
		for (i = 1; i < p_count; i++) {
			if ((MechS32) polygon[i].m_u < minU) {
				minU = (MechS32) polygon[i].m_u;
			}
			if ((MechS32) polygon[i].m_v < minV) {
				minV = (MechS32) polygon[i].m_v;
			}
		}
		for (i = 0; i < p_count; i++) {
			polygon[i].m_u -= (MechFloat) minU;
			polygon[i].m_v -= (MechFloat) minV;
		}

		vertex = &polygon[1];
		for (i = p_count - 2; i; i--) {
			msiRenderTriangle(polygon, vertex, vertex + 1, 100);
			vertex++;
		}
	}
}

// Draws a textured polygon, perspective-corrected with p_flags 0x200 or 0x400.
// Register allocation and store scheduling differ (the parameter block's setup).
// FUNCTION: MW2MATROX 0x10062630
void FUN_10062630(
	PANE* p_pane,
	MechU32 p_count,
	A3DVertex* p_vertices,
	MechU32 p_flags,
	undefined4 p_unk0x10,
	A3DTexture* p_texture,
	undefined4 p_unk0x18,
	undefined4 p_unk0x1c
)
{
	MechS32 transparent;
	MechU32 perspective;
	MechU32 scale;
	MechFloat dx;
	MechFloat dy;
	A3DVertex* vertex;
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;
	MechS32 minU;
	MechS32 minV;
	MechU32 i;
	MechFloat clip[4];
	A3DVertex polygon[0x40];

	if (p_count < 3) {
		return;
	}
	if (!p_texture) {
		DebugPrint("A3D_polygon_clip_XY_and_render(): NULL textureptr passed\n");
		return;
	}

	perspective = (p_flags & 0x600) != 0;
	transparent = p_flags & 1;
	if (g_unk0x100ac9d0 != 3 || p_texture->m_unk0x10 != g_unk0x100ac9d4 ||
		(MechS32) p_texture->m_unk0x14->m_address != g_unk0x100ac9dc || transparent != g_unk0x100ac9e4 ||
		p_texture->m_unk0x38 != g_unk0x100ac9d8) {
		g_unk0x100ac9d4 = p_texture->m_unk0x10;
		g_unk0x100ac9e4 = transparent;
		g_unk0x100ac9dc = p_texture->m_unk0x14->m_address;
		g_unk0x100ac9d8 = p_texture->m_unk0x38;
		scale = 1 << p_texture->m_unk0x38;
		g_unk0x100ac9d0 = 3;
		if (transparent) {
			g_unk0x100acad0 = 1;
			switch (p_texture->m_unk0x24) {
			case 4:
				if (p_texture->m_unk0x2c) {
					g_unk0x100acad4 = 0;
					g_unk0x100acad6 = 0xf;
				}
				else {
					g_unk0x100acad4 = 0;
					g_unk0x100acad0 = 0;
					g_unk0x100acad6 = 0;
				}
				break;
			case 8:
				if (p_texture->m_unk0x2c) {
					g_unk0x100acad4 = 0;
					g_unk0x100acad6 = 0xff;
				}
				else {
					g_unk0x100acad4 = 0;
					g_unk0x100acad0 = 0;
					g_unk0x100acad6 = 0;
				}
				break;
			case 0xf:
				g_unk0x100acad4 = 0;
				g_unk0x100acad6 = 0x8000;
				break;
			case 0x10:
				g_unk0x100acad4 = 0;
				g_unk0x100acad0 = 0;
				g_unk0x100acad6 = 0;
				break;
			}
		}
		else {
			g_unk0x100acad4 = 0;
			g_unk0x100acad0 = 0;
			g_unk0x100acad6 = 0;
		}

		g_unk0x100acac8 = 1;
		g_unk0x100aca98[1] = p_texture->m_unk0x1c / scale;
		g_unk0x100aca98[2] = p_texture->m_unk0x20 / scale;
		g_unk0x100aca98[3] = p_texture->m_unk0x24;
		g_unk0x100aca98[4] = 0;
		g_unk0x100aca98[5] = 0;
		g_unk0x100aca98[6] = p_texture->m_unk0x3c[p_texture->m_unk0x38] + p_texture->m_unk0x14->m_address;
		if (p_texture->m_unk0x24 >= 0xf) {
			g_unk0x100aca98[7] = 0;
			g_unk0x100aca98[8] = 0;
			g_unk0x100aca98[9] = 0;
		}
		else {
			g_unk0x100aca98[7] = 0;
			g_unk0x100aca98[8] = 0;
			g_unk0x100aca98[9] = p_texture->m_unk0x60[p_texture->m_unk0x38] + p_texture->m_unk0x14->m_address;
		}
		msiSetParameters((int) g_unk0x100aca98);
		g_unk0x100ac904 = 0;
	}

	left = p_pane->m_x0;
	if (left || p_pane->m_y0) {
		dx = (MechFloat) left;
		dy = (MechFloat) p_pane->m_y0;
		for (vertex = p_vertices; vertex < p_vertices + p_count; vertex++) {
			vertex->m_x += dx;
			vertex->m_y += dy;
		}
	}

	top = p_pane->m_y0;
	bottom = p_pane->m_y1 + 1;
	right = p_pane->m_x1 + 1;
	if (left < 0) {
		left = 0;
	}
	if ((MechS32) g_unk0x100ac938 < right) {
		right = (MechS32) g_unk0x100ac938;
	}
	if (top < 0) {
		top = 0;
	}
	if ((MechS32) g_unk0x100ac93c < bottom) {
		bottom = (MechS32) g_unk0x100ac93c;
	}
	clip[0] = (MechFloat) left;
	clip[1] = (MechFloat) right;
	clip[2] = (MechFloat) top;
	clip[3] = (MechFloat) bottom;

	p_count = A3DClipPolygon(p_vertices, polygon, p_count, clip, perspective);
	if (p_count >= 3) {
		if (!perspective) {
			for (i = 0; i < p_count; i++) {
				polygon[i].m_z = 0.0f;
				polygon[i].m_w = 1.0f;
				polygon[i].m_unk0x1c = polygon[i].m_red;
				polygon[i].m_unk0x20 = polygon[i].m_green;
				polygon[i].m_unk0x24 = polygon[i].m_blue;
			}
		}
		else {
			for (i = 0; i < p_count; i++) {
				polygon[i].m_u = polygon[i].m_u / polygon[i].m_w;
				polygon[i].m_unk0x1c = polygon[i].m_red;
				polygon[i].m_unk0x20 = polygon[i].m_green;
				polygon[i].m_unk0x24 = polygon[i].m_blue;
				polygon[i].m_v = polygon[i].m_v / polygon[i].m_w;
			}
		}

		minU = (MechS32) polygon[0].m_u;
		minV = (MechS32) polygon[0].m_v;
		for (i = 1; i < p_count; i++) {
			if ((MechS32) polygon[i].m_u < minU) {
				minU = (MechS32) polygon[i].m_u;
			}
			if ((MechS32) polygon[i].m_v < minV) {
				minV = (MechS32) polygon[i].m_v;
			}
		}
		for (i = 0; i < p_count; i++) {
			polygon[i].m_u -= (MechFloat) minU;
			polygon[i].m_v -= (MechFloat) minV;
		}

		vertex = &polygon[1];
		for (i = p_count - 2; i; i--) {
			msiRenderTriangle(polygon, vertex, vertex + 1, 100);
			vertex++;
		}
	}
}

// STUB: MW2MATROX 0x10062cd0
MechS32 FUN_10062cd0(
	PANE* p_pane,
	MechS32 p_count,
	A3DPolyVertex* p_vertices,
	A3DTexture* p_texture,
	MechS32 p_unk0x10,
	MechDouble p_depth,
	MechS32 p_unk0x1c
)
{
	STUB(0x10062cd0);
	return 0;
}

// Clips the polygon p_vertices of p_count vertices to the depth p_depth (by m_w, the projection
// scale over the depth), in place; returns the clipped polygon's vertex count.
// The original copies the polygon with rep movsd alone (no byte tail); register allocation differs.
// FUNCTION: MW2MATROX 0x10065710
MechU32 FUN_10065710(A3DPolyVertex* p_vertices, MechU32 p_count, MechDouble p_depth)
{
	MechDouble w;
	MechDouble t;
	A3DPolyVertex* a;
	A3DPolyVertex* b;
	MechU32 i;
	MechU32 count;
	A3DPolyVertex polygon[0x40];

	w = g_eyepoint->m_projectScaleX / p_depth;
	memcpy(polygon, p_vertices, p_count * sizeof(A3DPolyVertex));
	a = polygon;
	b = &polygon[1];
	count = 0;
	for (i = 0; i < p_count; i++) {
		if (p_count - i == 1) {
			b = polygon;
		}
		if (a->m_w >= w) {
			*p_vertices = *a;
			if (b->m_w < w) {
				count++;
				p_vertices++;
				*p_vertices = *b;
				t = (w - a->m_w) / (b->m_w - a->m_w);
				A3D_LERP_DOUBLE(p_vertices->m_u, a->m_u, b->m_u, t)
				A3D_LERP_DOUBLE(p_vertices->m_v, a->m_v, b->m_v, t)
				p_vertices->m_w = w;
			}
			count++;
			p_vertices++;
		}
		else if (b->m_w >= w) {
			*p_vertices = *a;
			t = (w - a->m_w) / (b->m_w - a->m_w);
			A3D_LERP_DOUBLE(p_vertices->m_u, a->m_u, b->m_u, t)
			A3D_LERP_DOUBLE(p_vertices->m_v, a->m_v, b->m_v, t)
			p_vertices->m_w = w;
			count++;
			p_vertices++;
		}
		a++;
		b++;
	}

	return count;
}

// Draws the textured triangle p_a, p_b, p_c, choosing its MIP map level by its mean depth with
// p_mip.
// Register allocation differs (the original biases the triangle pointer by 0x10).
// FUNCTION: MW2MATROX 0x100659f0
void FUN_100659f0(A3DPolyVertex* p_a, A3DPolyVertex* p_b, A3DPolyVertex* p_c, A3DTexture* p_texture, MechS32 p_mip)
{
	A3DPolyVertex* vertices[3];
	MechDouble depth;
	A3DPolyVertex** from;
	A3DPolyVertex* vertex;
	A3DVertex* to;
	MechU32 level;
	MechU32 scale;
	MechS32 minU;
	MechS32 minV;
	A3DVertex triangle[3];

	depth = 0.0;
	vertices[0] = p_a;
	vertices[1] = p_b;
	vertices[2] = p_c;
	for (from = vertices, to = triangle; from < vertices + 3; from++, to++) {
		vertex = *from;
		to->m_x = (MechFloat) vertex->m_x;
		to->m_y = (MechFloat) vertex->m_y;
		to->m_z = (MechFloat) vertex->m_z;
		to->m_w = (MechFloat) vertex->m_w;
		to->m_red = (MechFloat) vertex->m_red;
		to->m_green = (MechFloat) vertex->m_green;
		to->m_blue = (MechFloat) vertex->m_blue;
		to->m_unk0x1c = to->m_red;
		to->m_unk0x20 = to->m_green;
		to->m_unk0x24 = to->m_blue;
		to->m_u = (MechFloat) (vertex->m_u / vertex->m_w);
		to->m_v = (MechFloat) (vertex->m_v / vertex->m_w);
		if (p_mip) {
			depth += g_eyepoint->m_projectScaleX / vertex->m_w;
		}
	}

	if (p_mip) {
		depth /= 3.0;
		for (level = 0; level < g_unk0x100ac920; level++) {
			if (depth < g_unk0x100e26a0[level]) {
				break;
			}
		}
		if (level >= g_unk0x100ac920) {
			level = g_unk0x100ac920 - 1;
		}
		if (level >= p_texture->m_unk0x34) {
			level = p_texture->m_unk0x34 - 1;
		}
		if (level != (MechU32) g_unk0x100ac9d8) {
			scale = 1 << level;
			g_unk0x100aca98[1] = p_texture->m_unk0x1c / scale;
			g_unk0x100ac9d8 = level;
			p_texture->m_unk0x38 = level;
			g_unk0x100aca98[2] = p_texture->m_unk0x20 / scale;
			g_unk0x100aca98[6] = p_texture->m_unk0x3c[p_texture->m_unk0x38] + p_texture->m_unk0x14->m_address;
			g_unk0x100aca98[9] = p_texture->m_unk0x60[p_texture->m_unk0x38] + p_texture->m_unk0x14->m_address;
			msiSetParameters((int) g_unk0x100aca98);
		}
	}

	minU = (MechS32) triangle[0].m_u;
	minV = (MechS32) triangle[0].m_v;
	for (to = &triangle[1]; to < triangle + 3; to++) {
		if ((MechS32) to->m_u < minU) {
			minU = (MechS32) to->m_u;
		}
		if ((MechS32) to->m_v < minV) {
			minV = (MechS32) to->m_v;
		}
	}
	for (to = triangle; to < triangle + 3; to++) {
		to->m_u -= (MechFloat) minU;
		to->m_v -= (MechFloat) minV;
	}

	msiRenderTriangle(&triangle[0], &triangle[1], &triangle[2], 100);
}

// The original interleaves the color and texture interpolations on the FPU stack, which neither
// direct stores nor locals (stored under /Op) reproduce, and /Oa (which does) breaks the others.
// FUNCTION: MW2MATROX 0x10066a50
void FUN_10066a50(MechS32 p_axis, MechFloat p_edge, A3DVertex* p_a, A3DVertex* p_b, A3DVertex* p_out, MechU32 p_flags)
{
	MechFloat t;

	switch (p_axis) {
	case 1:
		t = (p_edge - p_a->m_x) / (p_b->m_x - p_a->m_x);
		p_out->m_x = p_edge;
		p_out->m_y = (p_b->m_y - p_a->m_y) * t + p_a->m_y;
		if (p_flags & 8) {
			p_out->m_w = (p_b->m_w - p_a->m_w) * t + p_a->m_w;
		}
		break;
	case 2:
		t = (p_edge - p_a->m_y) / (p_b->m_y - p_a->m_y);
		p_out->m_y = p_edge;
		p_out->m_x = (p_b->m_x - p_a->m_x) * t + p_a->m_x;
		if (p_flags & 8) {
			p_out->m_w = (p_b->m_w - p_a->m_w) * t + p_a->m_w;
		}
		break;
	}

	p_out->m_z = p_a->m_z;
	if (p_flags & 1) {
		p_out->m_red = (p_b->m_red - p_a->m_red) * t + p_a->m_red;
		p_out->m_green = (p_b->m_green - p_a->m_green) * t + p_a->m_green;
		p_out->m_blue = (p_b->m_blue - p_a->m_blue) * t + p_a->m_blue;
	}
	else {
		p_out->m_red = p_a->m_red;
		p_out->m_green = p_a->m_green;
		p_out->m_blue = p_a->m_blue;
	}
	if (p_flags & 4) {
		p_out->m_u = (p_b->m_u - p_a->m_u) * t + p_a->m_u;
		p_out->m_v = (p_b->m_v - p_a->m_v) * t + p_a->m_v;
	}
}

// FUNCTION: MW2MATROX 0x10066c00
void FUN_10066c00(undefined* p_heap)
{
	msiFreeTextureHeap(p_heap);
}

// FUNCTION: MW2MATROX 0x10066c10
undefined* FUN_10066c10(void)
{
	return msiAllocTextureHeap(((MechU32) (MechS32) (g_unk0x100ac93c * g_unk0x100ac938 * 2.0f) * 2 + 0xfff) >> 12);
}

// FUNCTION: MW2MATROX 0x10066c40
void FUN_10066c40(
	undefined* p_heap,
	undefined* p_buffer,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_width,
	MechS32 p_height,
	MechS32 p_color
)
{
	MechS32 offset;

	if (p_width > 0 && p_height > 0) {
		offset = ((MechS32) g_unk0x100ac938 * p_top + p_left) * 2;
		if (p_color < 0) {
			msiBlitRect(
				p_heap,
				p_buffer + offset,
				g_unk0x100e2680,
				(MechS32) (g_unk0x100ac938 * 2.0f),
				0x10,
				p_width,
				p_height,
				offset,
				-1,
				0
			);
		}
		else {
			msiBlitRect(
				p_heap,
				p_buffer + offset,
				g_unk0x100e2680,
				(MechS32) (g_unk0x100ac938 * 2.0f),
				0x10,
				p_width,
				p_height,
				offset,
				p_color,
				0xffff
			);
		}
	}
}

// FUNCTION: MW2MATROX 0x10066cf0
void FUN_10066cf0(undefined* p_heap, undefined* p_buffer, MechS32 p_color)
{
	FUN_10066c40(p_heap, p_buffer, 0, 0, (MechS32) g_unk0x100ac938, (MechS32) g_unk0x100ac93c, p_color);
}

// Opens MSI95.DLL's display; reads the MIP map settings from MYSTIQUE.PAR first.
// Store scheduling differs.
// FUNCTION: MW2MATROX 0x10066d30
MechS32 A3D_Init(WNDPROC p_windowProc, MechS32 p_width, MechS32 p_height)
{
	FILE* file;
	MechDouble* thresholds;
	MechU32 i;
	MechFloat threshold;
	MechFloat maxZ;

	g_unk0x100ac934 = (undefined4) p_windowProc;
	DebugPrint("A3D_Init() Called...");
	if (g_unk0x100ac908) {
		return 0;
	}

	file = fopen("MYSTIQUE.PAR", "rt");
	if (file) {
		MechChar line[0x100];

		thresholds = g_unk0x100e26a0;
		while (fgets(line, sizeof(line), file)) {
			if (line[0] != '/' && !sscanf(line, "groundMIPlevels = %i", &g_unk0x100ac924) &&
				!sscanf(line, "skyMIPlevels = %i", &g_unk0x100ac928) &&
				!sscanf(line, "NoMIPThresholds = %i", &g_unk0x100ac920)) {
				if (sscanf(line, "MIPThreshold = %f", &threshold)) {
					*thresholds++ = threshold;
				}
				else if (
					!sscanf(line, "MIPmaxZ = %f", &maxZ) && !sscanf(line, "MIPtessheight = %i", &g_unk0x100ac92c)
				) {
					sscanf(line, "NoHeapPages = %i", &g_unk0x100ac91c);
				}
			}
		}
		fclose(file);
	}

	for (i = 0; i < g_unk0x100ac920; i++) {
		g_unk0x100e26a0[i] *= maxZ;
	}
	g_unk0x100e26a0[i] = 100000000.0;

	g_unk0x100ac904 = 0;
	g_unk0x100ac900 = 0;
	g_unk0x100ac908 = 1;
	if (g_unk0x100ac930) {
		msiExit();
		g_unk0x100ac930 = NULL;
	}

	g_unk0x100ac938 = (MechFloat) p_width;
	g_unk0x100ac93c = (MechFloat) p_height;
	g_unk0x100ac930 =
		msiInit((MechS32) g_unk0x100ac938, (MechS32) g_unk0x100ac93c, 0x10, 0, 0, (void*) g_unk0x100ac934);
	if (!g_unk0x100ac930) {
		DebugPrint("A3D_Init(): msiInit() returned error!\n");
		return 1;
	}

	FUN_10066fd0();
	FUN_10067080();
	FUN_100671a0();
	FUN_10067170();
	FUN_10061c90();
	DebugPrint("A3D_Init() Completed!\n");
	return 0;
}

// Starts the texture cache's heap: one free block of the card's texture memory.
// Store scheduling differs.
// FUNCTION: MW2MATROX 0x10066fd0
void FUN_10066fd0(void)
{
	g_unk0x100acaf8 = NULL;
	g_unk0x100acaf0 = NULL;
	g_unk0x100acaf4 = NULL;
	g_unk0x100acafc = g_unk0x100ac930->m_unk0x50;
	g_unk0x100acb04 = 0;
	DebugPrint("Texture Cache Size = %i\n", g_unk0x100acafc);

	g_unk0x100acaf4 = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(A3DHeapBlock));
	memset(g_unk0x100acaf4, 0, sizeof(A3DHeapBlock));
	g_unk0x100acaf4->m_unk0x08 = NULL;
	g_unk0x100acaf4->m_unk0x0c = NULL;
	g_unk0x100acaf4->m_unk0x00 = NULL;
	g_unk0x100acaf4->m_unk0x04 = NULL;
	g_unk0x100acaf4->m_size = g_unk0x100acafc;
	g_unk0x100acaf4->m_address = 0;
	g_unk0x100acaf4->m_free = TRUE;
	g_unk0x100acaf8 = g_unk0x100acaf4;
}

// HEAP_init (its log string): allocates the texture heap (g_unk0x100ac91c pages) and starts its
// list with one free block.
// Store scheduling differs.
// FUNCTION: MW2MATROX 0x10067080
void FUN_10067080(void)
{
	g_unk0x100acb14 = NULL;
	g_unk0x100acb0c = NULL;
	g_unk0x100acb10 = NULL;
	g_unk0x100acb28 = 0;
	g_unk0x100acb20 = msiAllocTextureHeap(g_unk0x100ac91c);
	if (!g_unk0x100acb20) {
		g_unk0x100acb18 = 0;
		DebugPrint("HEAP_init(): Unable to allocate texture heap!\n");
		return;
	}

	g_unk0x100acb18 = g_unk0x100ac91c << 12;
	DebugPrint("Texture Heap Size = %i\n", g_unk0x100acb18);
	g_unk0x100acb10 = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, 0x24);
	memset(g_unk0x100acb10, 0, 0x24);
	g_unk0x100acb10->m_unk0x08 = NULL;
	g_unk0x100acb10->m_unk0x0c = NULL;
	g_unk0x100acb10->m_unk0x00 = NULL;
	g_unk0x100acb10->m_unk0x04 = NULL;
	g_unk0x100acb10->m_size = g_unk0x100acb18;
	g_unk0x100acb10->m_address = (MechU32) g_unk0x100acb20;
	g_unk0x100acb10->m_free = TRUE;
	g_unk0x100acb14 = g_unk0x100acb10;
}

// FUNCTION: MW2MATROX 0x10067170
void FUN_10067170(void)
{
	g_unk0x100acb30 = NULL;
	g_unk0x100acb34 = NULL;
	memset(g_unk0x100c2680, 0, sizeof(g_unk0x100c2680));
	g_unk0x100acb38 = 0;
	g_unk0x100acb3c = 0;
}

// FUNCTION: MW2MATROX 0x100671a0
void FUN_100671a0(void)
{
	g_unk0x100acb40 = NULL;
	g_unk0x100acb44 = NULL;
	memset(g_unk0x100d2680, 0, sizeof(g_unk0x100d2680));
	g_unk0x100acb48 = 0;
	g_unk0x100acb4c = 0;
	g_unk0x100acb50 = 0;
}

// FUNCTION: MW2MATROX 0x100671e0
void A3D_shutdown(void)
{
	DebugPrint("\nA3D_shutdown() Called...");
	if (g_unk0x100ac908) {
		g_unk0x100ac908 = 0;
		FUN_100673c0();
		FUN_10067300();
		FUN_10067260();
		if (g_unk0x100ac900) {
			FUN_1005f6e0();
			msiEndFrame(0, 0, 1);
		}
		FUN_10061ca0();
		if (g_unk0x100ac930) {
			msiExit();
			g_unk0x100ac930 = NULL;
		}
		DebugPrint("\nA3D_shutdown() Completed!\n");
	}
}

// The original reloads g_primaryHeap at the end of the loop, ahead of the next iteration.
// FUNCTION: MW2MATROX 0x10067260
void FUN_10067260(void)
{
	HANDLE heap;
	A3DHeapBlock* block;
	A3DHeapBlock* prev;

	while (g_unk0x100acaf0) {
		FUN_1005d140(g_unk0x100acaf0);
	}
	while (g_unk0x100acaf4) {
		heap = g_primaryHeap;
		block = g_unk0x100acaf4;
		A3D_UNLIST_BLOCK(block, g_unk0x100acaf4);
		A3D_UNCHAIN_BLOCK(block, prev, g_unk0x100acaf8);
		HeapFree(heap, HEAP_NO_SERIALIZE, block);
	}
}

// The original stores g_unk0x100acb0c's NULL elsewhere and calls HeapFree through esi.
// FUNCTION: MW2MATROX 0x10067300
void FUN_10067300(void)
{
	HANDLE heap;
	A3DHeapBlock* block;
	A3DHeapBlock* prev;

	while (g_unk0x100acb0c) {
		FUN_1005d440(g_unk0x100acb0c);
	}
	g_unk0x100acb0c = NULL;
	while (g_unk0x100acb10) {
		heap = g_primaryHeap;
		block = g_unk0x100acb10;
		A3D_UNLIST_BLOCK(block, g_unk0x100acb10);
		A3D_UNCHAIN_BLOCK(block, prev, g_unk0x100acb14);
		HeapFree(heap, HEAP_NO_SERIALIZE, block);
	}
	msiFreeTextureHeap(g_unk0x100acb20);
	g_unk0x100acb20 = NULL;
}

// Register allocation differs (the original holds both texture list ends in registers).
// FUNCTION: MW2MATROX 0x100673c0
void FUN_100673c0(void)
{
	A3DCacheItem* item;
	A3DCacheItem* next;
	HANDLE heap;

	item = g_unk0x100acb40;
	if (item) {
		while (item) {
			A3D_UNLINK(item, next, m_heapNext, m_heapPrev, g_unk0x100acb30, g_unk0x100acb34);
			g_unk0x100c2680[item->m_id] = NULL;
			FUN_1005d140(item->m_unk0x14);
			FUN_1005d440(item->m_unk0x18);
			heap = g_primaryHeap;
			A3D_UNLINK(item, next, m_next, m_prev, g_unk0x100acb40, g_unk0x100acb44);
			g_unk0x100d2680[item->m_id] = NULL;
			HeapFree(heap, HEAP_NO_SERIALIZE, item);
			g_unk0x100acb50--;
			item = g_unk0x100acb40;
		}
	}
	g_unk0x100acb48 = 0;
	g_unk0x100acb4c = 0;
	g_unk0x100acb50 = 0;
}
