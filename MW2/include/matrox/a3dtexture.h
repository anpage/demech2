#ifndef MATROX_A3DTEXTURE_H
#define MATROX_A3DTEXTURE_H

#include "decomp.h"
#include "types.h"

// A texture as the A3D renderer takes it, from the edition's texture cache (DrawAnimatedPolygon).
// Only the members the game code reads are known; the size isn't.
typedef struct A3DTexture {
	undefined m_unk0x00[0x10];        // 0x00
	MechS32 m_unk0x10;                // 0x10 — the texture's ID (A3D_map_polygon's parameter cache)
	struct A3DHeapBlock* m_unk0x14;   // 0x14 — its block of the texture heap
	undefined4 m_unk0x18;             // 0x18
	MechU32 m_unk0x1c;                // 0x1c — the width (DrawAnimatedPolygon scales u by it)
	MechU32 m_unk0x20;                // 0x20 — the height (v)
	MechS32 m_unk0x24;                // 0x24 — the pixel format (4, 8, 0xf or 0x10)
	undefined4 m_unk0x28;             // 0x28
	MechS32 m_unk0x2c;                // 0x2c
	undefined m_unk0x30[0x34 - 0x30]; // 0x30
	MechU32 m_unk0x34;                // 0x34 — the mipmap level count
	MechS32 m_unk0x38;                // 0x38 — the mipmap level drawn
	MechU32 m_unk0x3c[9];             // 0x3c — each level's offset in the heap block
	MechU32 m_unk0x60[9];             // 0x60 — each level's palette offset (formats under 0xf)
} A3DTexture;

#endif // MATROX_A3DTEXTURE_H
