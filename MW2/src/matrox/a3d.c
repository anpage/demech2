/* The Matrox edition's A3D renderer (0x1005a5f0 to 0x10067500): one object compiled with /Ox /G5 /Op,
   where the rest of the edition is /Od (CLAUDE.md, "The A3D layer"). */
#include "matrox/a3d.h"

#include "debugprint.h"
#include "decomp.h"
#include "msi95.h"
#include "palettecolor.h"
#include "polydraw.h"
#include "simmain.h"
#include "types.h"

#include <string.h>
#include <windows.h>

typedef struct A3DHeapBlock A3DHeapBlock;

// An item of the texture caches, on two lists: its cache's (m_next, m_prev) and the heap cache's
// (m_heapNext, m_heapPrev).
typedef struct A3DCacheItem A3DCacheItem;
struct A3DCacheItem {
	A3DCacheItem* m_next;     // 0x00
	A3DCacheItem* m_prev;     // 0x04
	A3DCacheItem* m_heapNext; // 0x08
	A3DCacheItem* m_heapPrev; // 0x0c
	MechS32 m_id;             // 0x10
	A3DHeapBlock* m_unk0x14;  // 0x14
	undefined4* m_unk0x18;    // 0x18
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

// GLOBAL: MW2MATROX 0x100ac930
MechS32 g_unk0x100ac930 = 0;

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
undefined4 g_unk0x100acb08 = 0;

// GLOBAL: MW2MATROX 0x100acb0c
undefined4* g_unk0x100acb0c = NULL;

// GLOBAL: MW2MATROX 0x100acb10
A3DHeapBlock* g_unk0x100acb10 = NULL;

// GLOBAL: MW2MATROX 0x100acb14
A3DHeapBlock* g_unk0x100acb14 = NULL;

// GLOBAL: MW2MATROX 0x100acb18
undefined4 g_unk0x100acb18 = 0;

// GLOBAL: MW2MATROX 0x100acb1c
undefined4 g_unk0x100acb1c = 0;

// GLOBAL: MW2MATROX 0x100acb20
undefined* g_unk0x100acb20 = NULL;

// GLOBAL: MW2MATROX 0x100acb24
undefined4 g_unk0x100acb24 = 0;

// GLOBAL: MW2MATROX 0x100acb28
undefined4 g_unk0x100acb28 = 0;

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

void FUN_1005d0a0(void);
void FUN_1005d140(A3DHeapBlock* p_block);
void FUN_1005d440(undefined4* p_item);
void FUN_10061ca0(void);
void FUN_10066cf0(undefined* p_heap, undefined* p_buffer, MechS32 p_color);
void FUN_10067260(void);
void FUN_10067300(void);
void FUN_100673c0(void);

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

// Frees the heap block p_block: puts it on the free list, merged with a free neighbour.
// Not matched yet (43%): the logic follows the original, the register and stack allocation don't.
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
			if (cur == NULL) {
				if (p_block->m_address + size == last->m_address) {
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
			else if (cur->m_unk0x0c && cur->m_unk0x0c->m_address + cur->m_unk0x0c->m_size == p_block->m_address) {
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
		else {
			p_block->m_unk0x08 = NULL;
			p_block->m_unk0x0c = NULL;
			g_unk0x100acaf4 = p_block;
		}
		g_unk0x100acb00 = 0;
	}
}

// STUB: MW2MATROX 0x1005d440
void FUN_1005d440(undefined4* p_item)
{
	STUB(0x1005d440);
}

// STUB: MW2MATROX 0x1005d600
A3DTexture* FUN_1005d600(MechS32 p_id, MechS32 p_unk0x08, MechU32 p_mode)
{
	STUB(0x1005d600);
	return NULL;
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

// STUB: MW2MATROX 0x1005f8a0
void FUN_1005f8a0(PANE* p_pane, MechS32 p_color)
{
	STUB(0x1005f8a0);
}

// Draws the outline of a polygon.
// STUB: MW2MATROX 0x10061890
void FUN_10061890(PANE* p_pane, A3DVertex* p_vertices, MechS32 p_count)
{
	STUB(0x10061890);
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
// STUB: MW2MATROX 0x10061cb0
void FUN_10061cb0(PANE* p_pane, MechS32 p_count, A3DVertex* p_vertices)
{
	STUB(0x10061cb0);
}

// STUB: MW2MATROX 0x10062010
void FUN_10062010(PANE* p_pane, MechS32 p_count, A3DVertex* p_vertices, A3DTexture* p_texture, MechU32 p_flags)
{
	STUB(0x10062010);
}

// Draws a textured polygon.
// STUB: MW2MATROX 0x10062630
void FUN_10062630(
	PANE* p_pane,
	MechS32 p_count,
	A3DVertex* p_vertices,
	MechU32 p_flags,
	undefined4 p_unk0x10,
	A3DTexture* p_texture,
	undefined4 p_unk0x18,
	undefined4 p_unk0x1c
)
{
	STUB(0x10062630);
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

// STUB: MW2MATROX 0x10066d30
MechS32 FUN_10066d30(WNDPROC p_windowProc, MechS32 p_width, MechS32 p_height)
{
	STUB(0x10066d30);
	return -1;
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
			g_unk0x100ac930 = 0;
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
