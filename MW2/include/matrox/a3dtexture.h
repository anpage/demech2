#ifndef MATROX_A3DTEXTURE_H
#define MATROX_A3DTEXTURE_H

#include "decomp.h"
#include "types.h"

struct A3DHeapBlock;

// A texture of the A3D renderer (A3D_LoadTexture returns it), an item of its two caches: the heap
// cache, whose items' levels are loaded into a block of the texture heap in system memory
// (m_heapBlock; its list is m_heapNext and m_heapPrev), and the texture cache, whose items are also
// copied from there into a block of video memory to be drawn (m_vramBlock; m_vramNext, m_vramPrev).
// SIZE 0x84
typedef struct A3DTexture A3DTexture;
struct A3DTexture {
	A3DTexture* m_heapNext;           // 0x00
	A3DTexture* m_heapPrev;           // 0x04
	A3DTexture* m_vramNext;           // 0x08
	A3DTexture* m_vramPrev;           // 0x0c
	MechS32 m_id;                     // 0x10 — the CEL resource's ID
	struct A3DHeapBlock* m_vramBlock; // 0x14
	struct A3DHeapBlock* m_heapBlock; // 0x18
	MechS32 m_width;                  // 0x1c
	MechS32 m_height;                 // 0x20
	MechS32 m_format;                 // 0x24 — the pixel format (4, 8, 0xf or 0x10)
	undefined4 m_unk0x28;             // 0x28 — the palette's color count
	MechS32 m_transparent;            // 0x2c — whether it has transparent pixels (formats under 0xf)
	MechU32 m_size;                   // 0x30 — the levels' size in bytes
	MechU32 m_levels;                 // 0x34 — the mipmap level count
	MechS32 m_level;                  // 0x38 — the mipmap level drawn
	MechU32 m_offsets[9];             // 0x3c — each level's offset in the blocks
	MechU32 m_paletteOffsets[9];      // 0x60 — each level's palette offset (formats under 0xf)
};

#endif // MATROX_A3DTEXTURE_H
