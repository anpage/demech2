/* The Matrox edition's A3D renderer (0x1005a5f0 to 0x10067500): one object compiled with /Ox /G5 /Op,
   where the rest of the Matrox edition is /Od (CLAUDE.md, "The A3D layer"). Only VC++ 4.x builds the
   Matrox edition, so its inline assembly (A3D_COPY_VERTICES, A3D_COMPUTE_BOUNDS) has no portable C. */
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
#include "view.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

typedef struct A3DHeapBlock A3DHeapBlock;

// A color of the palette FUN_1005f0c0 builds: its index, its pixel count, the sums of its pixels'
// components and its components (5 bits each).
// SIZE 0x20
typedef struct A3DPaletteEntry {
	MechU32 m_index;    // 0x00
	MechU32 m_count;    // 0x04
	MechU32 m_sumRed;   // 0x08
	MechU32 m_sumGreen; // 0x0c
	MechU32 m_sumBlue;  // 0x10
	MechU32 m_red;      // 0x14
	MechU32 m_green;    // 0x18
	MechU32 m_blue;     // 0x1c
} A3DPaletteEntry;

// The display msiInit returns.
typedef struct A3DDisplay {
	undefined m_unk0x00[0x50]; // 0x00
	MechU32 m_unk0x50;         // 0x50 — the texture cache's size
} A3DDisplay;

// A block of the texture heap, on two lists: all blocks (m_unk0x00, m_unk0x04) and the used or the
// free blocks (m_unk0x08, m_unk0x0c; the free ones in address order).
struct A3DHeapBlock {
	A3DHeapBlock* m_unk0x00; // 0x00
	A3DHeapBlock* m_unk0x04; // 0x04
	A3DHeapBlock* m_unk0x08; // 0x08
	A3DHeapBlock* m_unk0x0c; // 0x0c
	A3DTexture* m_owner;     // 0x10
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
WNDPROC g_unk0x100ac934 = NULL;

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
A3DTexture* g_unk0x100acb30 = NULL;

// GLOBAL: MW2MATROX 0x100acb34
A3DTexture* g_unk0x100acb34 = NULL;

// GLOBAL: MW2MATROX 0x100acb38
MechU32 g_unk0x100acb38 = 0;

// GLOBAL: MW2MATROX 0x100acb3c
MechU32 g_unk0x100acb3c = 0;

// GLOBAL: MW2MATROX 0x100acb40
A3DTexture* g_unk0x100acb40 = NULL;

// GLOBAL: MW2MATROX 0x100acb44
A3DTexture* g_unk0x100acb44 = NULL;

// GLOBAL: MW2MATROX 0x100acb48
MechU32 g_unk0x100acb48 = 0;

// GLOBAL: MW2MATROX 0x100acb4c
MechU32 g_unk0x100acb4c = 0;

// GLOBAL: MW2MATROX 0x100acb50
MechS32 g_unk0x100acb50 = 0;

// GLOBAL: MW2MATROX 0x100acb54
undefined4 g_unk0x100acb54 = 0;

// GLOBAL: MW2MATROX 0x100acb58
MechS32 g_unk0x100acb58 = 0;

// GLOBAL: MW2MATROX 0x100acb5c
MechU32 g_unk0x100acb5c = 1;

// The cached items by ID.
// GLOBAL: MW2MATROX 0x100c2680
static A3DTexture* g_unk0x100c2680[0x4000];

// GLOBAL: MW2MATROX 0x100d2680
static A3DTexture* g_unk0x100d2680[0x4000];

// The buffer FUN_10066c40 fills rectangles from.
// GLOBAL: MW2MATROX 0x100e2680
static undefined g_unk0x100e2680[0x10];

// The slopes of the side clip planes (x and y against the depth): one over the view's center.
// GLOBAL: MW2MATROX 0x100e2690
static MechDouble g_unk0x100e2690;

// GLOBAL: MW2MATROX 0x100e2698
static MechDouble g_unk0x100e2698;

// The MIP map thresholds (MYSTIQUE.PAR's, times its MIPmaxZ), ending in 1e8; room for eight before the
// texture at 0x100e26e0.
// GLOBAL: MW2MATROX 0x100e26a0
static MechDouble g_unk0x100e26a0[8];

// The 64x64 16-bit texture HEAPCACHE_loaditem loads for the ID 0x29b6.
// GLOBAL: MW2MATROX 0x100e26e0
undefined g_unk0x100e26e0[0x2000];

// The palette entries of FUN_1005f0c0's 15-bit colors and of its 12-bit colors (the low bit of each
// component dropped).
// GLOBAL: MW2MATROX 0x100e46e0
static A3DPaletteEntry* g_unk0x100e46e0[0x8000];

// GLOBAL: MW2MATROX 0x101046e0
static A3DPaletteEntry* g_unk0x101046e0[0x8000];

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

// Widens the bounds g_unk0x101246e0 to g_unk0x101246ec (set from the first vertex) by the p_count - 1
// vertices from p_vertex on.
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

// A3D_LERP for doubles.
#define A3D_LERP_DOUBLE(p_out, p_a, p_b, p_t)                                                                          \
	if ((MechFloat) fabs((p_a) - (p_b)) < 1e-7f) {                                                                     \
		p_out = p_a;                                                                                                   \
	}                                                                                                                  \
	else {                                                                                                             \
		p_out = ((p_b) - (p_a)) * (p_t) + (p_a);                                                                       \
	}

// The point where the edge from the clipper's vertex a to b (inside and outside, or the reverse)
// crosses the clip edge p_edge, on x or y: its coordinates (p_lerp: A3D_LERP or A3D_LERP_DOUBLE)
// in to, and the interpolation factor in t.
#define A3D_CROSS_X(p_lerp, p_edge)                                                                                    \
	t = ((p_edge) - a->m_x) / (b->m_x - a->m_x);                                                                       \
	to->m_x = p_edge;                                                                                                  \
	p_lerp(to->m_y, a->m_y, b->m_y, t)

#define A3D_CROSS_Y(p_lerp, p_edge)                                                                                    \
	t = ((p_edge) - a->m_y) / (b->m_y - a->m_y);                                                                       \
	p_lerp(to->m_x, a->m_x, b->m_x, t) to->m_y = p_edge;

// One pass of the clippers (FUN_1005fd70, FUN_10063150): clips the polygon from of count vertices
// against the edge p_edge on p_axis (keeping the vertices with p_axis p_inside p_edge) into p_to,
// p_cross and p_lerpRest (FUN_10061510, FUN_10064e20) interpolating the points where it crosses
// the edge (p_index: 0 left, 1 right, 2 top, 3 bottom); from and count become p_to's. Takes the
// locals a, b, to, n, i and t.
#define A3D_CLIP_PASS(                                                                                                 \
	p_to,                                                                                                              \
	p_axis,                                                                                                            \
	p_inside,                                                                                                          \
	p_outside,                                                                                                         \
	p_edge,                                                                                                            \
	p_cross,                                                                                                           \
	p_lerp,                                                                                                            \
	p_lerpRest,                                                                                                        \
	p_flags,                                                                                                           \
	p_clip,                                                                                                            \
	p_index                                                                                                            \
)                                                                                                                      \
	to = p_to;                                                                                                         \
	n = 0;                                                                                                             \
	a = from;                                                                                                          \
	b = from + 1;                                                                                                      \
	for (i = 0; i < count; i++) {                                                                                      \
		if (count - i == 1) {                                                                                          \
			b = from;                                                                                                  \
		}                                                                                                              \
		if (a->p_axis p_inside p_edge) {                                                                               \
			if (b->p_axis p_outside p_edge) {                                                                          \
				*to = *a;                                                                                              \
				n++;                                                                                                   \
				to++;                                                                                                  \
				p_cross(p_lerp, p_edge) p_lerpRest(to, a, b, t, p_flags, p_clip, p_index);                             \
				n++;                                                                                                   \
				to++;                                                                                                  \
			}                                                                                                          \
			else {                                                                                                     \
				*to = *a;                                                                                              \
				n++;                                                                                                   \
				to++;                                                                                                  \
			}                                                                                                          \
		}                                                                                                              \
		else if (b->p_axis p_inside p_edge) {                                                                          \
			p_cross(p_lerp, p_edge) p_lerpRest(to, a, b, t, p_flags, p_clip, p_index);                                 \
			n++;                                                                                                       \
			to++;                                                                                                      \
		}                                                                                                              \
		a++;                                                                                                           \
		b++;                                                                                                           \
	}                                                                                                                  \
	from = p_to;                                                                                                       \
	count = n;

// The steps p_step of the edge from p_start to p_end per row (p_dy rows): its x, texture
// coordinates, m_w and colors.
#define A3D_EDGE_STEP(p_step, p_start, p_end, p_dy)                                                                    \
	p_dy = (p_end)->m_y - (p_start)->m_y;                                                                              \
	p_step.m_x = ((p_end)->m_x - (p_start)->m_x) / p_dy;                                                               \
	p_step.m_u = ((p_end)->m_u - (p_start)->m_u) / p_dy;                                                               \
	p_step.m_v = ((p_end)->m_v - (p_start)->m_v) / p_dy;                                                               \
	p_step.m_w = ((p_end)->m_w - (p_start)->m_w) / p_dy;                                                               \
	p_step.m_red = ((p_end)->m_red - (p_start)->m_red) / p_dy;                                                         \
	p_step.m_green = ((p_end)->m_green - (p_start)->m_green) / p_dy;                                                   \
	p_step.m_blue = ((p_end)->m_blue - (p_start)->m_blue) / p_dy;

// Moves the edge vertex p_vertex down p_rows rows by the steps p_step.
#define A3D_EDGE_ADVANCE(p_vertex, p_step, p_rows)                                                                     \
	p_vertex.m_y += p_rows;                                                                                            \
	p_vertex.m_x += p_step.m_x * p_rows;                                                                               \
	p_vertex.m_u += p_step.m_u * p_rows;                                                                               \
	p_vertex.m_v += p_step.m_v * p_rows;                                                                               \
	p_vertex.m_w += p_step.m_w * p_rows;                                                                               \
	p_vertex.m_red += p_step.m_red * p_rows;                                                                           \
	p_vertex.m_green += p_step.m_green * p_rows;                                                                       \
	p_vertex.m_blue += p_step.m_blue * p_rows;

void FUN_1005d0a0(void);
void FUN_1005d140(A3DHeapBlock* p_block);
void FUN_1005d440(A3DHeapBlock* p_block);
void FUN_1005e220(A3DTexture* p_item);
A3DHeapBlock* FUN_1005d990(A3DTexture* p_item, MechS32 p_defrag);
A3DHeapBlock* VRAM_defrag(MechU32 p_size);
A3DHeapBlock* FUN_1005e770(A3DTexture* p_item, MechU32 p_size, MechS32 p_defrag, MechS32 p_flush);
A3DHeapBlock* HEAP_defrag(MechU32 p_size);
A3DTexture* HEAPCACHE_loaditem(MechS32 p_id, MechS32 p_paletted, MechU32 p_levels);
void FUN_1005ec40(A3DTexture* p_item, MechU16* p_pixels, MechU32 p_level, MechS32 p_paletted);
MechU32 FUN_1005f0c0(A3DTexture* p_item, MechU16* p_pixels, MechU32 p_level);
void FUN_1005ebb0(A3DTexture* p_item);
MechU32 FUN_1005fd70(A3DVertex* p_in, A3DVertex* p_out, MechU32 p_count, MechFloat* p_clip, MechS32 p_flags);
__inline void FUN_10061510(
	A3DVertex* p_out,
	A3DVertex* p_a,
	A3DVertex* p_b,
	MechDouble p_t,
	MechS32 p_perspective,
	MechFloat* p_clip,
	MechS32 p_edge
);
MechU32 FUN_10063150(
	A3DPolyVertex* p_in,
	A3DPolyVertex* p_out,
	MechU32 p_count,
	MechFloat* p_clip,
	MechS32 p_perspective
);
MechU32 FUN_100651a0(A3DPolyVertex* p_vertices, MechU32 p_count);
MechU32 FUN_10065710(A3DPolyVertex* p_vertices, MechU32 p_count, MechDouble p_depth);
void FUN_100659f0(A3DPolyVertex* p_a, A3DPolyVertex* p_b, A3DPolyVertex* p_c, A3DTexture* p_texture, MechS32 p_mip);
void FUN_10065ce0(A3DPolyVertex* p_vertices, MechU32 p_count, MechS32 p_rows, A3DTexture* p_texture, MechS32 p_mip);
__inline void FUN_10064e20(
	A3DPolyVertex* p_out,
	A3DPolyVertex* p_a,
	A3DPolyVertex* p_b,
	MechDouble p_t,
	MechS32 p_perspective,
	MechFloat* p_clip,
	MechS32 p_edge
);
void FUN_10061ca0(void);
void FUN_10066fd0(void);
void HEAP_init(void);
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
__inline static void VRAM_copyfromheap(A3DTexture* p_item, A3DHeapBlock* p_block)
{
	MechU32 level;

	g_unk0x100aca98[5] = (undefined4) g_unk0x100acb20;
	g_unk0x100aca98[3] = p_item->m_format;
	for (level = 0; level < p_item->m_levels; level++) {
		g_unk0x100aca98[1] = p_item->m_width / (1 << level);
		g_unk0x100aca98[2] = p_item->m_height / (1 << level);
		g_unk0x100aca98[4] = (undefined4) (p_item->m_heapBlock->m_unk0x20 + p_item->m_offsets[level]);
		g_unk0x100aca98[6] = p_item->m_offsets[level] + p_block->m_address;
		switch (p_item->m_format) {
		case 4:
		case 8:
			g_unk0x100aca98[7] = (undefined4) (p_item->m_heapBlock->m_unk0x20 + p_item->m_paletteOffsets[level]);
			g_unk0x100aca98[8] = (undefined4) g_unk0x100acb20;
			g_unk0x100aca98[9] = p_item->m_paletteOffsets[level] + p_block->m_address;
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
		msiSetParameters(g_unk0x100aca98);
	}
}

// "TEXTURECACHE_finditem": the texture cache's item of p_id (one with no VRAM block stops in the
// debugger).
__inline static A3DTexture* TEXTURECACHE_finditem(MechS32 p_id)
{
	A3DTexture* item;

	if (p_id >= 0x4000) {
		DebugPrint("TEXTURECACHE_finditem: Out of range ID: %i\n", p_id);
		return NULL;
	}

	item = g_unk0x100c2680[p_id];
	if (item && !item->m_vramBlock) {
#if defined(_MSC_VER) && defined(_M_IX86)
		__asm int 3
#endif
	}

	return item;
}

// "HEAPCACHE_finditem": the heap cache's item of p_id.
__inline static A3DTexture* HEAPCACHE_finditem(MechS32 p_id)
{
	if (p_id >= 0x4000) {
		DebugPrint("HEAPCACHE_finditem: Out of range ID: %i\n", p_id);
		return NULL;
	}

	return g_unk0x100d2680[p_id];
}

// A vertex of the polygon FUN_1005a5f0 clips, in doubles: the view-space position, the texture
// coordinates and the color.
// SIZE 0x40
typedef struct A3DClipVertex {
	MechDouble m_x;     // 0x00
	MechDouble m_y;     // 0x08
	MechDouble m_z;     // 0x10 — the depth
	MechDouble m_u;     // 0x18
	MechDouble m_v;     // 0x20
	MechDouble m_red;   // 0x28
	MechDouble m_green; // 0x30
	MechDouble m_blue;  // 0x38
} A3DClipVertex;

// The clip planes of FUN_1005a5f0 (A3DClipEdge's p_plane), with the flags that select them.
enum A3DClipPlane {
	c_clipPlaneLeft = 0,   // flag 0x10
	c_clipPlaneRight = 1,  // flag 0x20
	c_clipPlaneTop = 2,    // flag 4
	c_clipPlaneBottom = 3, // flag 8
	c_clipPlaneNear = 4,   // flag 1
	c_clipPlaneFar = 5     // flag 2
};

__inline static void A3DClipEdge(A3DClipVertex* p_out, A3DClipVertex* p_a, A3DClipVertex* p_b, MechS32 p_plane);

// Clips the polygon p_in of p_count vertices against the view's planes selected by p_flags (1 near,
// 2 far, 4 top, 8 bottom, 0x10 left, 0x20 right) into p_out; returns the clipped polygon's vertex
// count (0 when nothing is left).
// FUNCTION: MW2MATROX 0x1005a5f0
MechS32 FUN_1005a5f0(ProjectedVertex* p_out, ProjectedVertex* p_in, MechS32 p_count, MechU32 p_flags)
{
	A3DClipVertex in[64];
	A3DClipVertex nearOut[64];
	A3DClipVertex farOut[64];
	A3DClipVertex topOut[64];
	A3DClipVertex bottomOut[64];
	A3DClipVertex leftOut[64];
	A3DClipVertex rightOut[64];
	A3DClipVertex* vertex;
	A3DClipVertex* cur;
	A3DClipVertex* next;
	A3DClipVertex* out;
	A3DClipVertex* src;
	MechDouble minZ;
	MechDouble maxZ;
	MechDouble curEdge;
	MechDouble nextEdge;
	MechS32 i;
	MechS32 n;
	MechS32 outCount;

	vertex = in;
	for (i = 0; i < p_count; i++) {
		vertex->m_x = p_in->m_x;
		vertex->m_y = p_in->m_y;
		vertex->m_z = p_in->m_z;
		vertex->m_u = p_in->m_u;
		vertex->m_v = p_in->m_v;
		vertex->m_red = p_in->m_red;
		vertex->m_green = p_in->m_green;
		vertex->m_blue = p_in->m_blue;
		p_in++;
		vertex++;
	}

	minZ = maxZ = in[0].m_z;
	vertex = &in[1];
	for (i = 1; i < p_count; i++) {
		if (vertex->m_z < minZ) {
			minZ = vertex->m_z;
		}
		if (vertex->m_z > maxZ) {
			maxZ = vertex->m_z;
		}
		vertex++;
	}

	if ((p_flags & 1) && maxZ < g_viewNearPlane) {
		return 0;
	}
	if ((p_flags & 2) && minZ > g_viewFarPlane) {
		return 0;
	}

	g_unk0x100e2690 = 1.0 / g_viewCenterX;
	g_unk0x100e2698 = 1.0 / g_viewCenterY;

	src = in;
	n = p_count;

	if (p_flags & 1) {
		out = nearOut;
		outCount = 0;
		cur = src;
		next = src + 1;
		for (i = 0; i < n; i++) {
			if (n - i == 1) {
				next = src;
			}
			if (cur->m_z >= g_viewNearPlane) {
				*out = *cur;
				outCount++;
				out++;
				if (next->m_z < g_viewNearPlane) {
					A3DClipEdge(out, cur, next, c_clipPlaneNear);
					outCount++;
					out++;
				}
			}
			else if (next->m_z >= g_viewNearPlane) {
				A3DClipEdge(out, cur, next, c_clipPlaneNear);
				outCount++;
				out++;
			}
			cur++;
			next++;
		}
		src = nearOut;
		n = outCount;
	}
	if (n == 0) {
		return 0;
	}

	if (p_flags & 2) {
		out = farOut;
		outCount = 0;
		cur = src;
		next = src + 1;
		for (i = 0; i < n; i++) {
			if (n - i == 1) {
				next = src;
			}
			if (cur->m_z <= g_viewFarPlane) {
				*out = *cur;
				outCount++;
				out++;
				if (next->m_z > g_viewFarPlane) {
					A3DClipEdge(out, cur, next, c_clipPlaneFar);
					outCount++;
					out++;
				}
			}
			else if (next->m_z <= g_viewFarPlane) {
				A3DClipEdge(out, cur, next, c_clipPlaneFar);
				outCount++;
				out++;
			}
			cur++;
			next++;
		}
		src = farOut;
		n = outCount;
	}
	if (n == 0) {
		return 0;
	}

	if (p_flags & 4) {
		out = topOut;
		outCount = 0;
		cur = src;
		next = src + 1;
		for (i = 0; i < n; i++) {
			if (n - i == 1) {
				next = src;
			}
			curEdge = cur->m_y * g_unk0x100e2698;
			nextEdge = next->m_y * g_unk0x100e2698;
			if (cur->m_z >= curEdge) {
				*out = *cur;
				outCount++;
				out++;
				if (next->m_z < nextEdge) {
					A3DClipEdge(out, cur, next, c_clipPlaneTop);
					outCount++;
					out++;
				}
			}
			else if (next->m_z >= nextEdge) {
				A3DClipEdge(out, cur, next, c_clipPlaneTop);
				outCount++;
				out++;
			}
			cur++;
			next++;
		}
		src = topOut;
		n = outCount;
	}
	if (n == 0) {
		return 0;
	}

	if (p_flags & 8) {
		out = bottomOut;
		outCount = 0;
		cur = src;
		next = src + 1;
		for (i = 0; i < n; i++) {
			if (n - i == 1) {
				next = src;
			}
			curEdge = cur->m_y * g_unk0x100e2698;
			nextEdge = next->m_y * g_unk0x100e2698;
			if (curEdge >= -cur->m_z) {
				*out = *cur;
				outCount++;
				out++;
				if (nextEdge < -next->m_z) {
					A3DClipEdge(out, cur, next, c_clipPlaneBottom);
					outCount++;
					out++;
				}
			}
			else if (nextEdge >= -next->m_z) {
				A3DClipEdge(out, cur, next, c_clipPlaneBottom);
				outCount++;
				out++;
			}
			cur++;
			next++;
		}
		src = bottomOut;
		n = outCount;
	}
	if (n == 0) {
		return 0;
	}

	if (p_flags & 0x10) {
		out = leftOut;
		outCount = 0;
		cur = src;
		next = src + 1;
		for (i = 0; i < n; i++) {
			if (n - i == 1) {
				next = src;
			}
			curEdge = cur->m_x * g_unk0x100e2690;
			nextEdge = next->m_x * g_unk0x100e2690;
			if (curEdge >= -cur->m_z) {
				*out = *cur;
				outCount++;
				out++;
				if (nextEdge < -next->m_z) {
					A3DClipEdge(out, cur, next, c_clipPlaneLeft);
					outCount++;
					out++;
				}
			}
			else if (nextEdge >= -next->m_z) {
				A3DClipEdge(out, cur, next, c_clipPlaneLeft);
				outCount++;
				out++;
			}
			cur++;
			next++;
		}
		src = leftOut;
		n = outCount;
	}
	if (n == 0) {
		return 0;
	}

	if (p_flags & 0x20) {
		out = rightOut;
		outCount = 0;
		cur = src;
		next = src + 1;
		for (i = 0; i < n; i++) {
			if (n - i == 1) {
				next = src;
			}
			curEdge = cur->m_x * g_unk0x100e2690;
			nextEdge = next->m_x * g_unk0x100e2690;
			if (cur->m_z >= curEdge) {
				*out = *cur;
				outCount++;
				out++;
				if (next->m_z < nextEdge) {
					A3DClipEdge(out, cur, next, c_clipPlaneRight);
					outCount++;
					out++;
				}
			}
			else if (next->m_z >= nextEdge) {
				A3DClipEdge(out, cur, next, c_clipPlaneRight);
				outCount++;
				out++;
			}
			cur++;
			next++;
		}
		src = rightOut;
		n = outCount;
	}
	if (n == 0) {
		return 0;
	}

	for (i = 0; i < n; i++) {
		p_out->m_x = (MechFloat) src->m_x;
		p_out->m_y = (MechFloat) src->m_y;
		p_out->m_z = (MechFloat) src->m_z;
		p_out->m_u = (MechFloat) src->m_u;
		p_out->m_v = (MechFloat) src->m_v;
		p_out->m_red = (MechFloat) src->m_red;
		p_out->m_green = (MechFloat) src->m_green;
		p_out->m_blue = (MechFloat) src->m_blue;
		src++;
		p_out++;
	}

	return n;
}

// Puts in p_out the point where the edge from p_a to p_b crosses the clip plane p_plane. FUN_1005a5f0
// expands its first two calls (the near plane's) and calls it out of line for the others.
// FUNCTION: MW2MATROX 0x1005c430
__inline static void A3DClipEdge(A3DClipVertex* p_out, A3DClipVertex* p_a, A3DClipVertex* p_b, MechS32 p_plane)
{
	MechDouble t;
	MechDouble slope;
	MechDouble intercept;

	if ((MechFloat) fabs(p_a->m_z - p_b->m_z) < 1e-7f) {
		switch (p_plane) {
		case c_clipPlaneLeft:
			p_out->m_x = -(p_a->m_z / g_unk0x100e2690);
			t = (p_out->m_x - p_a->m_x) / (p_b->m_x - p_a->m_x);
			A3D_LERP_DOUBLE(p_out->m_y, p_a->m_y, p_b->m_y, t);
			A3D_LERP_DOUBLE(p_out->m_z, p_a->m_z, p_b->m_z, t);
			break;
		case c_clipPlaneRight:
			p_out->m_x = p_a->m_z / g_unk0x100e2690;
			t = (p_out->m_x - p_a->m_x) / (p_b->m_x - p_a->m_x);
			A3D_LERP_DOUBLE(p_out->m_y, p_a->m_y, p_b->m_y, t);
			A3D_LERP_DOUBLE(p_out->m_z, p_a->m_z, p_b->m_z, t);
			break;
		case c_clipPlaneTop:
			p_out->m_y = p_a->m_z / g_unk0x100e2698;
			t = (p_out->m_y - p_a->m_y) / (p_b->m_y - p_a->m_y);
			A3D_LERP_DOUBLE(p_out->m_x, p_a->m_x, p_b->m_x, t);
			A3D_LERP_DOUBLE(p_out->m_z, p_a->m_z, p_b->m_z, t);
			break;
		case c_clipPlaneBottom:
			p_out->m_y = -(p_a->m_z / g_unk0x100e2698);
			t = (p_out->m_y - p_a->m_y) / (p_b->m_y - p_a->m_y);
			A3D_LERP_DOUBLE(p_out->m_x, p_a->m_x, p_b->m_x, t);
			A3D_LERP_DOUBLE(p_out->m_z, p_a->m_z, p_b->m_z, t);
			break;
		case c_clipPlaneNear:
		case c_clipPlaneFar:
#if defined(_MSC_VER) && defined(_M_IX86)
			__asm int 3
#endif
				break;
		}
	}
	else {
		switch (p_plane) {
		case c_clipPlaneLeft:
			if ((MechFloat) fabs(p_a->m_x - p_b->m_x) < 1e-7f) {
				p_out->m_z = -(p_a->m_x * g_unk0x100e2690);
			}
			else {
				slope = (p_b->m_z - p_a->m_z) / (p_b->m_x - p_a->m_x);
				intercept = p_a->m_z - p_a->m_x * slope;
				p_out->m_z = -(intercept * g_unk0x100e2690) / (-g_unk0x100e2690 - slope);
			}
			break;
		case c_clipPlaneRight:
			if ((MechFloat) fabs(p_a->m_x - p_b->m_x) < 1e-7f) {
				p_out->m_z = p_a->m_x * g_unk0x100e2690;
			}
			else {
				slope = (p_b->m_z - p_a->m_z) / (p_b->m_x - p_a->m_x);
				intercept = p_a->m_z - p_a->m_x * slope;
				p_out->m_z = intercept * g_unk0x100e2690 / (g_unk0x100e2690 - slope);
			}
			break;
		case c_clipPlaneTop:
			if ((MechFloat) fabs(p_a->m_y - p_b->m_y) < 1e-7f) {
				p_out->m_z = p_a->m_y * g_unk0x100e2698;
			}
			else {
				slope = (p_b->m_z - p_a->m_z) / (p_b->m_y - p_a->m_y);
				intercept = p_a->m_z - p_a->m_y * slope;
				p_out->m_z = intercept * g_unk0x100e2698 / (g_unk0x100e2698 - slope);
			}
			break;
		case c_clipPlaneBottom:
			if ((MechFloat) fabs(p_a->m_y - p_b->m_y) < 1e-7f) {
				p_out->m_z = -(p_a->m_y * g_unk0x100e2698);
			}
			else {
				slope = (p_b->m_z - p_a->m_z) / (p_b->m_y - p_a->m_y);
				intercept = p_a->m_z - p_a->m_y * slope;
				p_out->m_z = -(intercept * g_unk0x100e2698) / (-g_unk0x100e2698 - slope);
			}
			break;
		case c_clipPlaneNear:
			p_out->m_z = g_viewNearPlane;
			break;
		case c_clipPlaneFar:
			p_out->m_z = g_viewFarPlane;
			break;
		}

		t = (p_out->m_z - p_a->m_z) / (p_b->m_z - p_a->m_z);
		A3D_LERP_DOUBLE(p_out->m_x, p_a->m_x, p_b->m_x, t);
		A3D_LERP_DOUBLE(p_out->m_y, p_a->m_y, p_b->m_y, t);
	}

	A3D_LERP_DOUBLE(p_out->m_red, p_a->m_red, p_b->m_red, t);
	A3D_LERP_DOUBLE(p_out->m_green, p_a->m_green, p_b->m_green, t);
	A3D_LERP_DOUBLE(p_out->m_blue, p_a->m_blue, p_b->m_blue, t);
	A3D_LERP_DOUBLE(p_out->m_u, p_a->m_u, p_b->m_u, t);
	A3D_LERP_DOUBLE(p_out->m_v, p_a->m_v, p_b->m_v, t);
}

// Prints the texture caches' statistics with OutputDebugString every 60 calls, and resets the
// counters.
// FUNCTION: MW2MATROX 0x1005ce20
void FUN_1005ce20(void)
{
	MechChar buffer[0x100];
	MechU32 hits;
	MechU32 misses;
	MechFloat ratio;
	MechS32 heapUsed;
	MechS32 vramUsed;

	if (g_unk0x100acb5c++ % 60 == 0) {
		hits = g_unk0x100acb38;
		misses = g_unk0x100acb3c;
		ratio = (MechFloat) hits / (MechFloat) (hits + misses);
		sprintf(buffer, "TEXTURE Hits: %i  Misses: %i   Hit Percentage: %i\n", hits, misses, (MechS32) (ratio * 100.0));
		OutputDebugString(buffer);
		g_unk0x100acb38 = 0;
		g_unk0x100acb3c = 0;

		hits = g_unk0x100acb48;
		misses = g_unk0x100acb4c;
		ratio = (MechFloat) hits / (MechFloat) (hits + misses);
		sprintf(
			buffer,
			"HEAP    Hits: %i  Misses: %i   Hit Percentage: %i\n\n",
			hits,
			misses,
			(MechS32) (ratio * 100.0)
		);
		OutputDebugString(buffer);
		g_unk0x100acb48 = 0;
		g_unk0x100acb4c = 0;

		heapUsed = 100 - (MechS32) ((MechDouble) g_unk0x100acb18 / (g_unk0x100ac91c << 12) * 100.0);
		vramUsed = 100 - (MechS32) ((MechDouble) g_unk0x100acafc / (MechS32) g_unk0x100ac930->m_unk0x50 * 100.0);
		sprintf(
			buffer,
			"HEAP Used:%i%% (%i left)     VRAM Used:%i%%  (%i left)\n",
			heapUsed,
			g_unk0x100acb18,
			vramUsed,
			g_unk0x100acafc
		);
		OutputDebugString(buffer);

		sprintf(
			buffer,
			"HEAP Defrags:%i    VRAM Defrags:%i    Forced Renders:%i\n",
			g_unk0x100acb24,
			g_unk0x100acb08,
			g_unk0x100acb2c
		);
		OutputDebugString(buffer);
		g_unk0x100acb24 = 0;
		g_unk0x100acb08 = 0;

		sprintf(
			buffer,
			"HEAPCACHEnotextures:%i  HEAPnoblocks:%i  VRAMnoblocks:%i\n",
			g_unk0x100acb50,
			g_unk0x100acb28,
			g_unk0x100acb04
		);
		OutputDebugString(buffer);
		OutputDebugString("\n\n");
	}
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
	A3DTexture* item;
	A3DTexture* next;

	while ((item = g_unk0x100acb30) != NULL) {
		A3D_UNLINK(item, next, m_vramNext, m_vramPrev, g_unk0x100acb30, g_unk0x100acb34);
		g_unk0x100c2680[item->m_id] = NULL;
		FUN_1005d140(item->m_vramBlock);
		item->m_vramBlock = NULL;
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
		p_block->m_owner->m_vramBlock = NULL;
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
		p_block->m_owner->m_heapBlock = NULL;
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
// heap cache, else loaded (HEAPCACHE_loaditem's p_paletted and p_levels); made the most recent in both.
// FUNCTION: MW2MATROX 0x1005d600
A3DTexture* A3D_LoadTexture(MechS32 p_id, MechS32 p_paletted, MechU32 p_levels)
{
	A3DTexture* item;
	A3DTexture* next;
	A3DTexture* oldest;
	A3DTexture* last;

	if (p_id >= 0x4000) {
		DebugPrint("A3D_LoadTexture(): Out of range ID: %i\n", p_id);
		return NULL;
	}

	item = TEXTURECACHE_finditem(p_id);
	if (item) {
		g_unk0x100acb38++;
		g_unk0x100acb48++;
		A3D_UNLINK(item, next, m_heapNext, m_heapPrev, g_unk0x100acb40, g_unk0x100acb44);
		g_unk0x100d2680[item->m_id] = NULL;
		A3D_UNLINK(item, next, m_vramNext, m_vramPrev, g_unk0x100acb30, g_unk0x100acb34);
		g_unk0x100c2680[item->m_id] = NULL;
	}
	else {
		g_unk0x100acb3c++;
		item = HEAPCACHE_finditem(p_id);
		if (item) {
			g_unk0x100acb48++;
			A3D_UNLINK(item, next, m_heapNext, m_heapPrev, g_unk0x100acb40, g_unk0x100acb44);
			g_unk0x100d2680[item->m_id] = NULL;
		}
		else {
			g_unk0x100acb4c++;
			item = HEAPCACHE_loaditem(p_id, p_paletted, p_levels);
			if (!item) {
				DebugPrint("A3D_LoadTexture(): Can't load texture ID: %i\n", p_id);
				return NULL;
			}
		}
		while (!FUN_1005d990(item, TRUE)) {
			oldest = g_unk0x100acb30;
			if (oldest) {
				FUN_1005e220(oldest);
				FUN_1005d140(oldest->m_vramBlock);
				oldest->m_vramBlock = NULL;
			}
		}
	}

	last = g_unk0x100acb44;
	g_unk0x100acb44 = item;
	item->m_heapPrev = last;
	item->m_heapNext = NULL;
	if (last) {
		last->m_heapNext = item;
	}
	else {
		g_unk0x100acb40 = item;
	}
	g_unk0x100d2680[item->m_id] = item;

	last = g_unk0x100acb34;
	g_unk0x100acb34 = item;
	item->m_vramPrev = last;
	item->m_vramNext = NULL;
	if (last) {
		last->m_vramNext = item;
	}
	else {
		g_unk0x100acb30 = item;
	}
	g_unk0x100c2680[item->m_id] = item;

	return item;
}

// Allocates a VRAM block for p_item (freeing the one it has), defragmenting VRAM if needed
// (p_defrag), and copies the texture's levels into it from the heap.
// Register and scheduling entropy remains (the free block's size is updated in memory).
// FUNCTION: MW2MATROX 0x1005d990
A3DHeapBlock* FUN_1005d990(A3DTexture* p_item, MechS32 p_defrag)
{
	A3DHeapBlock* cur;
	A3DHeapBlock* found;
	A3DHeapBlock* block;
	A3DHeapBlock* prev;
	MechU32 size;
	MechU32 remaining;

	FUN_1005d140(p_item->m_vramBlock);
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
			found = VRAM_defrag(size);
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

	VRAM_copyfromheap(block->m_owner, block);
	g_unk0x100acafc -= block->m_size;
	g_unk0x100acb04++;
	p_item->m_vramBlock = block;
	g_unk0x100acb00 = 0;
	return block;
}

// "VRAM_defrag()": merges free VRAM blocks into the largest one, moving the textures between them,
// until it holds p_size bytes; returns it.
// Not matched yet: register allocation and the order of the level copy's loads differ.
// FUNCTION: MW2MATROX 0x1005dcf0
A3DHeapBlock* VRAM_defrag(MechU32 p_size)
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
					VRAM_copyfromheap(block->m_owner, block);
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
					VRAM_copyfromheap(block->m_owner, block);
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

// Takes p_item off the texture cache (the items in VRAM; their list is m_vramNext and m_vramPrev).
// Register entropy: next and the address of p_item->m_vramPrev take each other's registers.
// FUNCTION: MW2MATROX 0x1005e220
void FUN_1005e220(A3DTexture* p_item)
{
	A3DTexture* prev;
	A3DTexture* next;

	next = p_item->m_vramNext;
	if (!next && !p_item->m_vramPrev) {
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
		next->m_vramPrev = p_item->m_vramPrev;
	}
	else {
		g_unk0x100acb34 = p_item->m_vramPrev;
	}
	prev = p_item->m_vramPrev;
	if (prev) {
		prev->m_vramNext = next;
	}
	else {
		g_unk0x100acb30 = next;
	}
	p_item->m_vramNext = NULL;
	g_unk0x100c2680[p_item->m_id] = NULL;
	p_item->m_vramPrev = NULL;
}

// "HEAPCACHE_loaditem()": loads the CEL resource p_id into a new item's heap block, with p_levels
// levels (p_paletted: 8-bit with a palette per level, else 16-bit).
// Not matched yet: register allocation and block order differ; the calls and stores follow the
// original.
// FUNCTION: MW2MATROX 0x1005e2c0
A3DTexture* HEAPCACHE_loaditem(MechS32 p_id, MechS32 p_paletted, MechU32 p_levels)
{
	A3DTexture* item;
	A3DTexture* oldest;
	A3DTexture* next;
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

	item = (A3DTexture*) HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(A3DTexture));
	if (item) {
		memset(item, 0, sizeof(A3DTexture));
	}
	item->m_heapBlock = FUN_1005e770(item, size, TRUE, TRUE);
	if (!item->m_heapBlock) {
		g_unk0x100ac9d0 = -1;
		g_unk0x100acb2c++;
		msiSetParameters(NULL);
		msiSetParameters((void*) -1);
		while (!item->m_heapBlock) {
			oldest = g_unk0x100acb40;
			if (oldest) {
				FUN_1005e220(oldest);
				FUN_1005d140(oldest->m_vramBlock);
				FUN_1005d440(oldest->m_heapBlock);
				FUN_1005ebb0(oldest);
				HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, oldest);
				g_unk0x100acb50--;
			}
			item->m_heapBlock = FUN_1005e770(item, size, TRUE, FALSE);
		}
	}

	if (p_id == 0x29b6) {
		data = (MechS16*) item->m_heapBlock->m_address;
		memcpy(data, g_unk0x100e26e0, size);
		item->m_format = 0xf;
		item->m_id = 0x29b6;
		item->m_heapBlock->m_unk0x20 = (undefined*) data;
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
			item->m_level = 0;
			item->m_heapBlock->m_unk0x20 = (undefined*) item->m_heapBlock->m_address;
			FUN_1005ec40(item, (MechU16*) (data + 2), 0, p_paletted);
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
			g_unk0x100acb50++;
			return item;
		}

		DebugPrint("HEAPCACHE_loaditem(): RetrieveByIndex() failed on ID #%i\n", p_id);
		if (item) {
			A3D_UNLINK(item, next, m_vramNext, m_vramPrev, g_unk0x100acb30, g_unk0x100acb34);
			g_unk0x100c2680[item->m_id] = NULL;
			FUN_1005d140(item->m_vramBlock);
			FUN_1005d440(item->m_heapBlock);
			FUN_1005ebb0(item);
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, item);
			g_unk0x100acb50--;
		}
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
		return NULL;
	}

	data = (MechS16*) item->m_heapBlock->m_address;
	if (ReadPrjResource(g_mw2PrjHandle, g_resourceTypeTags[c_resTagCel], p_id, data) >= 0) {
		item->m_format = 0xf;
		item->m_id = p_id;
		item->m_width = data[0];
		item->m_height = data[1];
		item->m_levels = p_levels;
		item->m_unk0x28 = 0;
		item->m_transparent = 1;
		item->m_level = 0;
		item->m_heapBlock->m_unk0x20 = (undefined*) (item->m_heapBlock->m_address + 4);
		FUN_1005ec40(item, (MechU16*) (data + 2), 0, 0);
		g_unk0x100acb50++;
		return item;
	}

	DebugPrint("HEAPCACHE_loaditem(): RetrieveByIndex() failed on ID #%i\n", p_id);
	if (item) {
		A3D_UNLINK(item, next, m_vramNext, m_vramPrev, g_unk0x100acb30, g_unk0x100acb34);
		g_unk0x100c2680[item->m_id] = NULL;
		FUN_1005d140(item->m_vramBlock);
		FUN_1005d440(item->m_heapBlock);
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
A3DHeapBlock* FUN_1005e770(A3DTexture* p_item, MechU32 p_size, MechS32 p_defrag, MechS32 p_flush)
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
				msiSetParameters(NULL);
				msiSetParameters((void*) -1);
			}
			found = HEAP_defrag(size);
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
	p_item->m_heapBlock = block;
	g_unk0x100acb1c = 0;
	return block;
}

// "HEAP_defrag()": merges free heap blocks into the largest one, moving the data between them, until
// it holds p_size bytes; returns it.
// Not matched yet: register allocation of the defragmentation loop differs.
// FUNCTION: MW2MATROX 0x1005e990
A3DHeapBlock* HEAP_defrag(MechU32 p_size)
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
// Register entropy: next and the address of p_item->m_heapPrev take each other's registers.
// FUNCTION: MW2MATROX 0x1005ebb0
void FUN_1005ebb0(A3DTexture* p_item)
{
	A3DTexture* prev;
	A3DTexture* next;

	next = p_item->m_heapNext;
	if (!next && !p_item->m_heapPrev) {
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
		next->m_heapPrev = p_item->m_heapPrev;
	}
	else {
		g_unk0x100acb44 = p_item->m_heapPrev;
	}
	prev = p_item->m_heapPrev;
	if (prev) {
		prev->m_heapNext = next;
	}
	else {
		g_unk0x100acb40 = next;
	}
	p_item->m_heapNext = NULL;
	g_unk0x100d2680[p_item->m_id] = NULL;
	p_item->m_heapPrev = NULL;
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
void FUN_1005ec40(A3DTexture* p_item, MechU16* p_pixels, MechU32 p_level, MechS32 p_paletted)
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
// "Distance" search of FUN_1005f0c0: the entry of p_colors (255 entries) nearest the color p_red,
// p_green, p_blue, stopping at the first within a distance of 3.
#define A3D_NEAREST_COLOR(p_best, p_colors, p_red, p_green, p_blue)                                                    \
	{                                                                                                                  \
		A3DPaletteEntry* entry;                                                                                        \
		MechU32 bestDist;                                                                                              \
		MechU32 dist;                                                                                                  \
                                                                                                                       \
		p_best = NULL;                                                                                                 \
		bestDist = 0xffffffff;                                                                                         \
		for (entry = p_colors; entry < p_colors + 255; entry++) {                                                      \
			dist = (entry->m_red - (p_red)) * (entry->m_red - (p_red)) +                                               \
				   (entry->m_green - (p_green)) * (entry->m_green - (p_green)) +                                       \
				   (entry->m_blue - (p_blue)) * (entry->m_blue - (p_blue));                                            \
			if (dist < bestDist) {                                                                                     \
				p_best = entry;                                                                                        \
				bestDist = dist;                                                                                       \
				if (dist <= 3) {                                                                                       \
					break;                                                                                             \
				}                                                                                                      \
			}                                                                                                          \
		}                                                                                                              \
	}

// Converts the level p_level of p_item, the 15-bit pixels p_pixels (bit 15 set where opaque), into a
// 4-bit (16 colors or fewer, one level) or 8-bit paletted texture in its heap block, with the palette
// before it; returns the palette's color count. Colors past 255 fall back to 12-bit colors, and past
// 255 of those, to the nearest of the 255 first.
// FUNCTION: MW2MATROX 0x1005f0c0
MechU32 FUN_1005f0c0(A3DTexture* p_item, MechU16* p_pixels, MechU32 p_level)
{
	A3DPaletteEntry reduced[256];
	A3DPaletteEntry full[256];
	A3DPaletteEntry* nextFull;
	A3DPaletteEntry* nextReduced;
	A3DPaletteEntry* colors;
	A3DPaletteEntry* entry;
	A3DPaletteEntry* best;
	A3DPaletteEntry** table;
	MechU16* pixel;
	MechU16* end;
	MechU16* palette;
	MechU8* dest;
	MechU32 size;
	MechU32 fullCount;
	MechU32 reducedCount;
	MechU32 count;
	MechU32 color;
	MechU32 red;
	MechU32 green;
	MechU32 blue;
	MechU32 index;
	MechU32 i;
	MechS32 fullOverflow;
	MechS32 reducedOverflow;
	MechS32 transparent;
	MechU16 mask;

	memset(g_unk0x100e46e0, 0, sizeof(g_unk0x100e46e0));
	memset(g_unk0x101046e0, 0, sizeof(g_unk0x101046e0));

	size = p_item->m_height * p_item->m_width / ((1 << p_level) << p_level);
	fullCount = 0;
	reducedCount = 0;
	fullOverflow = FALSE;
	transparent = 0;
	reducedOverflow = FALSE;
	end = p_pixels + size;
	nextReduced = reduced;
	nextFull = full;
	for (pixel = p_pixels; pixel < end; pixel++) {
		color = *pixel;
		if (!(color & 0x8000)) {
			transparent = 1;
			continue;
		}

		color &= 0x7fff;
		if (!fullOverflow) {
			if (g_unk0x100e46e0[color]) {
				g_unk0x100e46e0[color]->m_count++;
			}
			else if (nextFull < &full[255]) {
				g_unk0x100e46e0[color] = nextFull;
				nextFull->m_index = fullCount++;
				nextFull->m_red = color >> 10;
				nextFull->m_count = 1;
				nextFull->m_green = (color & 0x3e0) >> 5;
				nextFull->m_blue = color & 0x1f;
				nextFull++;
			}
			else {
				fullOverflow = TRUE;
			}
		}

		color &= 0x7bde;
		if (g_unk0x101046e0[color]) {
			g_unk0x101046e0[color]->m_count++;
		}
		else if (nextReduced == &reduced[255]) {
			reducedOverflow = TRUE;
			break;
		}
		else {
			g_unk0x101046e0[color] = nextReduced;
			nextReduced->m_index = reducedCount++;
			nextReduced->m_red = color >> 10;
			nextReduced->m_count = 1;
			nextReduced->m_green = (color & 0x3e0) >> 5;
			nextReduced->m_blue = color & 0x1f;
			nextReduced++;
		}
	}

	if (fullOverflow) {
		count = reducedCount;
		table = g_unk0x101046e0;
		mask = 0x7bde;
		colors = reduced;
	}
	else {
		count = fullCount;
		table = g_unk0x100e46e0;
		mask = 0x7fff;
		colors = full;
	}

	if (reducedOverflow) {
		for (entry = colors; entry < colors + count; entry++) {
			entry->m_sumRed = entry->m_red * entry->m_count;
			entry->m_sumGreen = entry->m_green * entry->m_count;
			entry->m_sumBlue = entry->m_blue * entry->m_count;
		}

		for (; pixel < end; pixel++) {
			color = *pixel;
			if (!(color & 0x8000)) {
				transparent = 1;
				continue;
			}

			color &= mask;
			red = color >> 10;
			green = (color & 0x3e0) >> 5;
			blue = color & 0x1f;
			A3D_NEAREST_COLOR(best, colors, red, green, blue);
			best->m_count++;
			best->m_sumRed += red;
			best->m_sumGreen += green;
			best->m_sumBlue += blue;
			best->m_red = (best->m_sumRed + best->m_count / 2) / best->m_count;
			best->m_green = (best->m_sumGreen + best->m_count / 2) / best->m_count;
			best->m_blue = (best->m_sumBlue + best->m_count / 2) / best->m_count;
			table[color] = best;
		}

		for (color = 0; color < 0x8000; color++) {
			if (table[color]) {
				red = color >> 10;
				green = (color & 0x3e0) >> 5;
				blue = color & 0x1f;
				A3D_NEAREST_COLOR(best, colors, red, green, blue);
				table[color] = best;
			}
		}
	}

	if (transparent) {
		count++;
	}
	if (p_level == 0) {
		p_item->m_paletteOffsets[p_level] = 0;
	}
	p_item->m_unk0x28 = count;
	p_item->m_transparent = transparent;

	if (count <= 16 && p_item->m_levels == 1) {
		p_item->m_format = 4;
		p_item->m_offsets[p_level] = p_item->m_paletteOffsets[p_level] + 0x20;
		p_item->m_size += (size >> 1) + 0x20;
		dest = (MechU8*) p_item->m_heapBlock->m_unk0x20 + p_item->m_offsets[p_level];
		for (i = 0; p_pixels < end; p_pixels++) {
			if (!(*p_pixels & 0x8000)) {
				index = 0;
			}
			else {
				index = table[*p_pixels & mask]->m_index + transparent;
			}
			if (i & 1) {
				*dest++ |= index;
			}
			else {
				*dest = index << 4;
			}
			i++;
		}
	}
	else {
		p_item->m_format = 8;
		p_item->m_offsets[p_level] = p_item->m_paletteOffsets[p_level] + 0x200;
		p_item->m_size += size + 0x200;
		dest = (MechU8*) p_item->m_heapBlock->m_unk0x20 + p_item->m_offsets[p_level];
		for (; p_pixels < end; p_pixels++) {
			if (!(*p_pixels & 0x8000)) {
				*dest = 0;
			}
			else {
				*dest = table[*p_pixels & mask]->m_index + transparent;
			}
			dest++;
		}
	}

	p_item->m_paletteOffsets[p_level + 1] = p_item->m_offsets[p_level] + size;
	palette = (MechU16*) (p_item->m_heapBlock->m_unk0x20 + p_item->m_paletteOffsets[p_level]);
	if (transparent) {
		*palette++ = 0;
	}
	for (entry = colors; entry < colors + (count - transparent); entry++) {
		*palette++ = entry->m_green << 6 | entry->m_red << 11 | entry->m_blue;
	}

	return count;
}

// The counterpart of FUN_1005f6e0: sets the parameters to 0 (msiSetParameters), unless they are set.
// FUNCTION: MW2MATROX 0x1005f6b0
void FUN_1005f6b0(void)
{
	g_unk0x100acb54++;
	if (!g_unk0x100ac904) {
		g_unk0x100ac9d0 = -1;
		msiSetParameters(NULL);
		g_unk0x100ac904 = 1;
	}
}

// FUNCTION: MW2MATROX 0x1005f6e0
void FUN_1005f6e0(void)
{
	g_unk0x100acb58++;
	if (g_unk0x100ac904) {
		g_unk0x100ac9d0 = -1;
		msiSetParameters((void*) -1);
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
		msiSetParameters(g_unk0x100aca40);
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
// FUNCTION: MW2MATROX 0x1005fd70
MechU32 FUN_1005fd70(A3DVertex* p_in, A3DVertex* p_out, MechU32 p_count, MechFloat* p_clip, MechS32 p_flags)
{
	A3DVertex* from;
	A3DVertex* a;
	A3DVertex* b;
	A3DVertex* to;
	MechU32 count;
	MechU32 n;
	MechU32 i;
	MechDouble t;
	A3DVertex top[0x40];
	A3DVertex bottom[0x40];
	A3DVertex left[0x40];
	A3DVertex right[0x40];

	from = p_in;
	count = p_count;
	if (g_unk0x101246ec < p_clip[2]) {
		A3D_CLIP_PASS(top, m_y, >=, <, p_clip[2], A3D_CROSS_Y, A3D_LERP, FUN_10061510, p_flags, p_clip, 2)
	}
	if (g_unk0x101246e4 > p_clip[3]) {
		A3D_CLIP_PASS(bottom, m_y, <=, >, p_clip[3], A3D_CROSS_Y, A3D_LERP, FUN_10061510, p_flags, p_clip, 3)
	}
	if (g_unk0x101246e8 < p_clip[0]) {
		A3D_CLIP_PASS(left, m_x, >=, <, p_clip[0], A3D_CROSS_X, A3D_LERP, FUN_10061510, p_flags, p_clip, 0)
	}
	if (g_unk0x101246e0 > p_clip[1]) {
		A3D_CLIP_PASS(right, m_x, <=, >, p_clip[1], A3D_CROSS_X, A3D_LERP, FUN_10061510, p_flags, p_clip, 1)
	}

	memcpy(p_out, from, count * sizeof(A3DVertex));
	return count;
}

// Interpolates the vertices p_a and p_b at p_t into p_out, past a clip edge on x: the colors and
// the texture coordinates, and with p_perspective, m_w and the depth from it.
// FUNCTION: MW2MATROX 0x10061510
__inline void FUN_10061510(
	A3DVertex* p_out,
	A3DVertex* p_a,
	A3DVertex* p_b,
	MechDouble p_t,
	MechS32 p_perspective,
	MechFloat* p_clip,
	MechS32 p_edge
)
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
			msiSetParameters(g_unk0x100aca40);
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
void A3D_map_polygon(PANE* p_pane, MechU32 p_count, A3DVertex* p_vertices, A3DTexture* p_texture, MechU32 p_flags)
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
	if (g_unk0x100ac9d0 != 2 || p_texture->m_id != g_unk0x100ac9d4 ||
		(MechS32) p_texture->m_vramBlock->m_address != g_unk0x100ac9dc || transparent != g_unk0x100ac9e4 ||
		shade != g_unk0x100ac9e0 || p_texture->m_level != g_unk0x100ac9d8) {
		g_unk0x100ac9d4 = p_texture->m_id;
		g_unk0x100ac9e0 = shade;
		g_unk0x100ac9e4 = transparent;
		g_unk0x100ac9dc = p_texture->m_vramBlock->m_address;
		g_unk0x100ac9d8 = p_texture->m_level;
		scale = 1 << p_texture->m_level;
		g_unk0x100ac9d0 = 2;
		if (transparent) {
			g_unk0x100acad0 = 1;
			switch (p_texture->m_format) {
			case 4:
				if (p_texture->m_transparent) {
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
				if (p_texture->m_transparent) {
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
		g_unk0x100aca98[1] = p_texture->m_width / scale;
		g_unk0x100aca98[2] = p_texture->m_height / scale;
		g_unk0x100aca98[3] = p_texture->m_format;
		g_unk0x100aca98[4] = 0;
		g_unk0x100aca98[5] = 0;
		g_unk0x100aca98[6] = p_texture->m_offsets[p_texture->m_level] + p_texture->m_vramBlock->m_address;
		if (p_texture->m_format >= 0xf) {
			g_unk0x100aca98[7] = 0;
			g_unk0x100aca98[8] = 0;
			g_unk0x100aca98[9] = 0;
		}
		else {
			g_unk0x100aca98[7] = 0;
			g_unk0x100aca98[8] = 0;
			g_unk0x100aca98[9] = p_texture->m_paletteOffsets[p_texture->m_level] + p_texture->m_vramBlock->m_address;
		}
		msiSetParameters(g_unk0x100aca98);
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
void A3D_polygon_clip_XY_and_render(
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
	if (g_unk0x100ac9d0 != 3 || p_texture->m_id != g_unk0x100ac9d4 ||
		(MechS32) p_texture->m_vramBlock->m_address != g_unk0x100ac9dc || transparent != g_unk0x100ac9e4 ||
		p_texture->m_level != g_unk0x100ac9d8) {
		g_unk0x100ac9d4 = p_texture->m_id;
		g_unk0x100ac9e4 = transparent;
		g_unk0x100ac9dc = p_texture->m_vramBlock->m_address;
		g_unk0x100ac9d8 = p_texture->m_level;
		scale = 1 << p_texture->m_level;
		g_unk0x100ac9d0 = 3;
		if (transparent) {
			g_unk0x100acad0 = 1;
			switch (p_texture->m_format) {
			case 4:
				if (p_texture->m_transparent) {
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
				if (p_texture->m_transparent) {
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
		g_unk0x100aca98[1] = p_texture->m_width / scale;
		g_unk0x100aca98[2] = p_texture->m_height / scale;
		g_unk0x100aca98[3] = p_texture->m_format;
		g_unk0x100aca98[4] = 0;
		g_unk0x100aca98[5] = 0;
		g_unk0x100aca98[6] = p_texture->m_offsets[p_texture->m_level] + p_texture->m_vramBlock->m_address;
		if (p_texture->m_format >= 0xf) {
			g_unk0x100aca98[7] = 0;
			g_unk0x100aca98[8] = 0;
			g_unk0x100aca98[9] = 0;
		}
		else {
			g_unk0x100aca98[7] = 0;
			g_unk0x100aca98[8] = 0;
			g_unk0x100aca98[9] = p_texture->m_paletteOffsets[p_texture->m_level] + p_texture->m_vramBlock->m_address;
		}
		msiSetParameters(g_unk0x100aca98);
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

// "A3D_GroundSkyPolyPlot()": draws the textured polygon p_vertices of p_count vertices on p_pane
// (the ground and the sky: AnimateGroundPolygon's and AnimateSkyPolygon's), in strips of
// p_unk0x10 rows unless it is a triangle (g_unk0x100ac92c's with p_mip), choosing the MIP map level
// by depth with p_mip. A positive p_depth clips it to that depth; a negative one, -p_depth, puts the
// polygon at that depth instead.
// FUNCTION: MW2MATROX 0x10062cd0
void A3D_GroundSkyPolyPlot(
	PANE* p_pane,
	MechU32 p_count,
	A3DPolyVertex* p_vertices,
	A3DTexture* p_texture,
	MechS32 p_unk0x10,
	MechDouble p_depth,
	MechS32 p_mip
)
{
	MechDouble dx;
	MechDouble dy;
	MechDouble scale;
	MechDouble z;
	A3DPolyVertex* vertex;
	A3DPolyVertex* end;
	MechS32 left;
	MechS32 top;
	MechS32 right;
	MechS32 bottom;
	MechU32 i;
	MechFloat clip[4];
	A3DPolyVertex polygon[0x40];

	if (!p_count) {
		return;
	}
	if (!p_texture) {
		DebugPrint("A3D_GroundSkyPolyPlot(): NULL textureptr passed\n");
		return;
	}

	if (p_mip) {
		p_unk0x10 = g_unk0x100ac92c;
	}
	g_unk0x100ac9d0 = 4;
	g_unk0x100ac9d8 = -1;
	p_texture->m_level = 0;
	g_unk0x100acad8 = 0;
	g_unk0x100acad0 = 0;
	g_unk0x100acada = 0;
	g_unk0x100acacc = 0;
	g_unk0x100acac8 = 1;
	g_unk0x100acad4 = 0;
	g_unk0x100acad6 = 0;
	g_unk0x100aca98[1] = p_texture->m_width;
	g_unk0x100aca98[2] = p_texture->m_height;
	g_unk0x100aca98[3] = p_texture->m_format;
	g_unk0x100aca98[4] = 0;
	g_unk0x100aca98[5] = 0;
	g_unk0x100aca98[6] = p_texture->m_offsets[p_texture->m_level] + p_texture->m_vramBlock->m_address;
	if (p_texture->m_format >= 0xf) {
		g_unk0x100aca98[7] = 0;
		g_unk0x100aca98[8] = 0;
		g_unk0x100aca98[9] = 0;
	}
	else {
		g_unk0x100aca98[7] = 0;
		g_unk0x100aca98[8] = 0;
		g_unk0x100aca98[9] = p_texture->m_paletteOffsets[p_texture->m_level] + p_texture->m_vramBlock->m_address;
	}
	if (!p_mip) {
		msiSetParameters(g_unk0x100aca98);
		g_unk0x100ac904 = 0;
	}

	left = p_pane->m_x0;
	if (left || p_pane->m_y0) {
		dx = (MechDouble) left;
		dy = (MechDouble) p_pane->m_y0;
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

	p_count = FUN_10063150(p_vertices, polygon, p_count, clip, 1);
	if (p_count < 3) {
		return;
	}
	if (p_count > 3) {
		p_count = FUN_100651a0(polygon, p_count);
	}
	if (p_count < 3) {
		return;
	}
	if (p_depth > 1.0) {
		p_count = FUN_10065710(polygon, p_count, p_depth);
	}
	if (p_count < 3) {
		return;
	}

	if (p_depth < 0.0) {
		p_depth = -p_depth;
		scale = g_eyepoint->m_projectScaleX;
		p_depth -= scale;
		end = polygon + p_count;
		for (vertex = polygon; vertex < end; vertex++) {
			vertex->m_u = vertex->m_u / vertex->m_w;
			vertex->m_v = vertex->m_v / vertex->m_w;
			z = scale / vertex->m_w;
			z = (z - scale) * p_depth / (153600000.0 - scale);
			z = z + scale;
			vertex->m_z = z;
			vertex->m_w = scale / z;
			vertex->m_u *= vertex->m_w;
			vertex->m_v *= vertex->m_w;
		}
	}

	if (p_count != 3 && p_unk0x10) {
		FUN_10065ce0(polygon, p_count, p_unk0x10, p_texture, p_mip);
		return;
	}

	vertex = &polygon[1];
	for (i = p_count - 2; i; i--) {
		FUN_100659f0(polygon, vertex, vertex + 1, p_texture, p_mip);
		vertex++;
	}
}

// Clips the polygon p_in of p_count vertices to the rectangle p_clip (left, right, top, bottom) into
// p_out, interpolating m_w with p_perspective; returns the clipped polygon's vertex count.
// FUNCTION: MW2MATROX 0x10063150
MechU32 FUN_10063150(
	A3DPolyVertex* p_in,
	A3DPolyVertex* p_out,
	MechU32 p_count,
	MechFloat* p_clip,
	MechS32 p_perspective
)
{
	MechDouble minX;
	MechDouble maxX;
	MechDouble minY;
	MechDouble maxY;
	MechDouble t;
	A3DPolyVertex* vertex;
	A3DPolyVertex* from;
	A3DPolyVertex* a;
	A3DPolyVertex* b;
	A3DPolyVertex* to;
	MechU32 count;
	MechU32 n;
	MechU32 i;
	A3DPolyVertex top[0x40];
	A3DPolyVertex bottom[0x40];
	A3DPolyVertex left[0x40];
	A3DPolyVertex right[0x40];

	minX = maxX = p_in->m_x;
	minY = maxY = p_in->m_y;
	vertex = p_in + 1;
	for (i = 1; i < p_count; i++) {
		if (vertex->m_x < minX) {
			minX = vertex->m_x;
		}
		if (vertex->m_x > maxX) {
			maxX = vertex->m_x;
		}
		if (vertex->m_y < minY) {
			minY = vertex->m_y;
		}
		if (vertex->m_y > maxY) {
			maxY = vertex->m_y;
		}
		vertex++;
	}

	if (p_clip[0] <= minX && maxX <= p_clip[1] && p_clip[2] <= minY && maxY <= p_clip[3]) {
		memcpy(p_out, p_in, p_count * sizeof(A3DPolyVertex));
		return p_count;
	}
	if (p_clip[1] < minX || maxX < p_clip[0] || p_clip[3] < minY || maxY < p_clip[2]) {
		return 0;
	}

	from = p_in;
	count = p_count;
	if (minY < p_clip[2]) {
		A3D_CLIP_PASS(top, m_y, >=, <, p_clip[2], A3D_CROSS_Y, A3D_LERP_DOUBLE, FUN_10064e20, p_perspective, p_clip, 2)
	}
	if (maxY > p_clip[3]) {
		A3D_CLIP_PASS(
			bottom,
			m_y,
			<=,
			>,
			p_clip[3],
			A3D_CROSS_Y,
			A3D_LERP_DOUBLE,
			FUN_10064e20,
			p_perspective,
			p_clip,
			3
		)
	}
	if (minX < p_clip[0]) {
		A3D_CLIP_PASS(left, m_x, >=, <, p_clip[0], A3D_CROSS_X, A3D_LERP_DOUBLE, FUN_10064e20, p_perspective, p_clip, 0)
	}
	if (maxX > p_clip[1]) {
		A3D_CLIP_PASS(
			right,
			m_x,
			<=,
			>,
			p_clip[1],
			A3D_CROSS_X,
			A3D_LERP_DOUBLE,
			FUN_10064e20,
			p_perspective,
			p_clip,
			1
		)
	}

	memcpy(p_out, from, count * sizeof(A3DPolyVertex));
	return count;
}

// FUN_10061510 for A3DPolyVertex: interpolates the vertices p_a and p_b at p_t into p_out, past the
// clip edge p_edge of p_clip (unused).
// FUNCTION: MW2MATROX 0x10064e20
__inline void FUN_10064e20(
	A3DPolyVertex* p_out,
	A3DPolyVertex* p_a,
	A3DPolyVertex* p_b,
	MechDouble p_t,
	MechS32 p_perspective,
	MechFloat* p_clip,
	MechS32 p_edge
)
{
	if (!p_perspective) {
		A3D_LERP_DOUBLE(p_out->m_red, p_a->m_red, p_b->m_red, p_t)
		A3D_LERP_DOUBLE(p_out->m_green, p_a->m_green, p_b->m_green, p_t)
		A3D_LERP_DOUBLE(p_out->m_blue, p_a->m_blue, p_b->m_blue, p_t)
		A3D_LERP_DOUBLE(p_out->m_u, p_a->m_u, p_b->m_u, p_t)
		A3D_LERP_DOUBLE(p_out->m_v, p_a->m_v, p_b->m_v, p_t)
		p_out->m_z = 0.0;
	}
	else {
		A3D_LERP_DOUBLE(p_out->m_u, p_a->m_u, p_b->m_u, p_t)
		A3D_LERP_DOUBLE(p_out->m_v, p_a->m_v, p_b->m_v, p_t)
		A3D_LERP_DOUBLE(p_out->m_red, p_a->m_red, p_b->m_red, p_t)
		A3D_LERP_DOUBLE(p_out->m_green, p_a->m_green, p_b->m_green, p_t)
		A3D_LERP_DOUBLE(p_out->m_blue, p_a->m_blue, p_b->m_blue, p_t)
		A3D_LERP_DOUBLE(p_out->m_w, p_a->m_w, p_b->m_w, p_t)
		p_out->m_z = g_eyepoint->m_projectScaleX / p_out->m_w;
	}
}

// Whether the vertex p_vertex lies on the edge from p_prev to p_next (within a pixel of either end,
// or of the line where the edge isn't axis-aligned).
__inline static MechS32 A3DIsRedundantVertex(A3DPolyVertex* p_prev, A3DPolyVertex* p_vertex, A3DPolyVertex* p_next)
{
	MechDouble tx;
	MechDouble ty;

	if (((p_prev->m_x <= p_vertex->m_x && p_vertex->m_x <= p_next->m_x) ||
		 (p_vertex->m_x <= p_prev->m_x && p_next->m_x <= p_vertex->m_x)) &&
		((p_prev->m_y <= p_vertex->m_y && p_vertex->m_y <= p_next->m_y) ||
		 (p_vertex->m_y <= p_prev->m_y && p_next->m_y <= p_vertex->m_y))) {
		if (fabs(p_vertex->m_x - p_prev->m_x) < 1.0 && fabs(p_vertex->m_y - p_prev->m_y) < 1.0) {
			return TRUE;
		}
		if (fabs(p_vertex->m_x - p_next->m_x) < 1.0 && fabs(p_vertex->m_y - p_next->m_y) < 1.0) {
			return TRUE;
		}
		if (fabs(p_next->m_x - p_prev->m_x) < 1.0) {
			return TRUE;
		}
		if (fabs(p_next->m_y - p_prev->m_y) < 1.0) {
			return TRUE;
		}
		tx = (p_vertex->m_x - p_prev->m_x) / (p_next->m_x - p_prev->m_x);
		ty = (p_vertex->m_y - p_prev->m_y) / (p_next->m_y - p_prev->m_y);
		if (fabs(tx - ty) < 0.001) {
			return TRUE;
		}
	}

	return FALSE;
}

// Drops the vertices of the polygon p_vertices of p_count vertices that are within a pixel of the
// next one or on an edge between their neighbours; returns the remaining vertex count.
// FUNCTION: MW2MATROX 0x100651a0
MechU32 FUN_100651a0(A3DPolyVertex* p_vertices, MechU32 p_count)
{
	A3DPolyVertex* a;
	A3DPolyVertex* b;
	A3DPolyVertex* to;
	A3DPolyVertex* out;
	MechU32 i;
	MechU32 n;
	A3DPolyVertex polygon[0x40];

	out = p_vertices;
	n = 0;
	to = polygon;
	a = p_vertices;
	for (i = 0; i < p_count; i++) {
		b = a + 1;
		if (p_count - i == 1) {
			b = out;
		}
		if (fabs(a->m_x - b->m_x) >= 1.0 || fabs(a->m_y - b->m_y) >= 1.0) {
			*to = *a;
			to++;
			n++;
		}
		a++;
	}

	p_count = n;
	n = 0;
	if (!A3DIsRedundantVertex(&polygon[p_count - 1], polygon, &polygon[1])) {
		*out = polygon[0];
		out++;
		n++;
	}
	a = polygon;
	for (i = 1; i < p_count; i++) {
		b = a + 2;
		if (p_count - i == 1) {
			b = polygon;
		}
		if (!A3DIsRedundantVertex(a, a + 1, b)) {
			*out = a[1];
			out++;
			n++;
		}
		a++;
	}

	return n;
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
			if (b->m_w < w) {
				*p_vertices = *a;
				count++;
				p_vertices++;
				*p_vertices = *b;
				t = (w - a->m_w) / (b->m_w - a->m_w);
				A3D_LERP_DOUBLE(p_vertices->m_u, a->m_u, b->m_u, t)
				A3D_LERP_DOUBLE(p_vertices->m_v, a->m_v, b->m_v, t)
				p_vertices->m_w = w;
				count++;
				p_vertices++;
			}
			else {
				*p_vertices = *a;
				count++;
				p_vertices++;
			}
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
		if (level >= p_texture->m_levels) {
			level = p_texture->m_levels - 1;
		}
		if (level != (MechU32) g_unk0x100ac9d8) {
			scale = 1 << level;
			g_unk0x100aca98[1] = p_texture->m_width / scale;
			g_unk0x100ac9d8 = level;
			p_texture->m_level = level;
			g_unk0x100aca98[2] = p_texture->m_height / scale;
			g_unk0x100aca98[6] = p_texture->m_offsets[p_texture->m_level] + p_texture->m_vramBlock->m_address;
			g_unk0x100aca98[9] = p_texture->m_paletteOffsets[p_texture->m_level] + p_texture->m_vramBlock->m_address;
			msiSetParameters(g_unk0x100aca98);
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

// Draws the textured polygon p_vertices of p_count vertices in strips of p_rows rows, from its top
// down: each strip is a pair of triangles between the left and right edges.
// FUNCTION: MW2MATROX 0x10065ce0
void FUN_10065ce0(A3DPolyVertex* p_vertices, MechU32 p_count, MechS32 p_rows, A3DTexture* p_texture, MechS32 p_mip)
{
	A3DPolyVertex* last;
	A3DPolyVertex* top;
	A3DPolyVertex* vertex;
	A3DPolyVertex* leftStart;
	A3DPolyVertex* leftEnd;
	A3DPolyVertex* rightStart;
	A3DPolyVertex* rightEnd;
	MechDouble maxY;
	MechDouble rows;
	MechDouble step;
	MechDouble remain;
	MechDouble dy;
	MechU32 i;
	MechS32 leftDone;
	MechS32 rightDone;
	A3DPolyVertex right;
	A3DPolyVertex left;
	A3DPolyVertex rightStep;
	A3DPolyVertex leftStep;
	A3DPolyVertex left0;
	A3DPolyVertex right0;

	if ((MechS32) p_count < 3) {
		return;
	}

	top = p_vertices;
	maxY = -1.0;
	last = &p_vertices[p_count - 1];
	vertex = p_vertices + 1;
	for (i = 1; i < p_count; i++) {
		vertex->m_x = floor(vertex->m_x + 0.5);
		vertex->m_y = floor(vertex->m_y + 0.5);
		if (vertex->m_y > maxY) {
			maxY = vertex->m_y;
		}
		if (vertex->m_y <= top->m_y && (vertex->m_y != top->m_y || top->m_x > vertex->m_x)) {
			top = vertex;
		}
		vertex++;
	}

	leftEnd = top;
	do {
		leftStart = leftEnd;
		leftEnd = leftStart - 1;
		if (leftEnd < p_vertices) {
			leftEnd = last;
		}
	} while (leftEnd != top && leftEnd->m_y - leftStart->m_y < 1.0);
	left = *leftStart;
	left0 = left;
	A3D_EDGE_STEP(leftStep, leftStart, leftEnd, dy)

	rightEnd = top;
	do {
		rightStart = rightEnd;
		rightEnd = rightStart + 1;
		if (rightEnd > last) {
			rightEnd = p_vertices;
		}
	} while (rightEnd != leftEnd && rightEnd->m_y - rightStart->m_y < 1.0);
	right = *rightStart;
	right0 = right;
	A3D_EDGE_STEP(rightStep, rightStart, rightEnd, dy)

	rows = (MechDouble) p_rows;
	if (maxY - top->m_y < rows) {
		rows = maxY - top->m_y;
	}
	leftDone = FALSE;
	rightDone = FALSE;
	remain = 0.0;
	while (TRUE) {
		step = rows;
		if (remain != 0.0) {
			step = remain;
		}

		if (rightEnd->m_y > leftEnd->m_y) {
			A3D_EDGE_ADVANCE(left, leftStep, step)
			if (leftEnd->m_y <= left.m_y + 0.5) {
				left = *leftEnd;
				step = left.m_y - left0.m_y;
				leftDone = TRUE;
			}
			A3D_EDGE_ADVANCE(right, rightStep, step)
			if (rightEnd->m_y <= right.m_y + 0.5) {
				right = *rightEnd;
				step = right.m_y - right0.m_y;
				rightDone = TRUE;
			}
		}
		else {
			A3D_EDGE_ADVANCE(right, rightStep, step)
			if (rightEnd->m_y <= right.m_y + 0.5) {
				right = *rightEnd;
				step = right.m_y - right0.m_y;
				rightDone = TRUE;
			}
			A3D_EDGE_ADVANCE(left, leftStep, step)
			if (leftEnd->m_y <= left.m_y + 0.5) {
				left = *leftEnd;
				step = left.m_y - left0.m_y;
				leftDone = TRUE;
			}
		}

		if (leftStart != rightStart || fabs(left0.m_x - right0.m_x) >= 1.0 || fabs(left0.m_y - right0.m_y) >= 1.0) {
			if (rightEnd == leftEnd) {
				FUN_100659f0(&left0, &right0, leftEnd, p_texture, p_mip);
				return;
			}
			FUN_100659f0(&left0, &right0, &right, p_texture, p_mip);
		}
		FUN_100659f0(&left0, &right, &left, p_texture, p_mip);
		left0 = left;
		right0 = right;

		remain = 0.0;
		if (leftDone) {
			remain = rows - step;
			do {
				leftStart = leftEnd;
				leftEnd = leftStart - 1;
				if (leftEnd < p_vertices) {
					leftEnd = last;
				}
			} while (rightEnd != leftEnd && leftEnd->m_y - leftStart->m_y < 1.0);
			left = *leftStart;
			left0 = left;
			A3D_EDGE_STEP(leftStep, leftStart, leftEnd, dy)
			if (dy < 1.0) {
				return;
			}
			leftDone = FALSE;
		}
		else if (rightDone) {
			remain = rows - step;
			do {
				rightStart = rightEnd;
				rightEnd = rightStart + 1;
				if (rightEnd > last) {
					rightEnd = p_vertices;
				}
			} while (rightEnd != leftEnd && rightEnd->m_y - rightStart->m_y < 1.0);
			right = *rightStart;
			right0 = right;
			A3D_EDGE_STEP(rightStep, rightStart, rightEnd, dy)
			if (dy < 1.0) {
				return;
			}
			rightDone = FALSE;
		}
	}
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

	g_unk0x100ac934 = p_windowProc;
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
	g_unk0x100ac930 = msiInit((MechS32) g_unk0x100ac938, (MechS32) g_unk0x100ac93c, 0x10, 0, 0, g_unk0x100ac934);
	if (!g_unk0x100ac930) {
		DebugPrint("A3D_Init(): msiInit() returned error!\n");
		return 1;
	}

	FUN_10066fd0();
	HEAP_init();
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
void HEAP_init(void)
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
	A3DTexture* item;
	A3DTexture* next;
	HANDLE heap;

	item = g_unk0x100acb40;
	if (item) {
		while (item) {
			A3D_UNLINK(item, next, m_vramNext, m_vramPrev, g_unk0x100acb30, g_unk0x100acb34);
			g_unk0x100c2680[item->m_id] = NULL;
			FUN_1005d140(item->m_vramBlock);
			FUN_1005d440(item->m_heapBlock);
			heap = g_primaryHeap;
			A3D_UNLINK(item, next, m_heapNext, m_heapPrev, g_unk0x100acb40, g_unk0x100acb44);
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
