#ifndef MATROX_A3DTEXTURE_H
#define MATROX_A3DTEXTURE_H

#include "decomp.h"
#include "types.h"

// A texture as the A3D renderer takes it, from the edition's texture cache (DrawAnimatedPolygon).
// Only the members the game code reads are known; the size isn't.
typedef struct A3DTexture {
	undefined m_unk0x00[0x1c]; // 0x00
	MechS32 m_unk0x1c;         // 0x1c — the width (DrawAnimatedPolygon scales u by it)
	MechS32 m_unk0x20;         // 0x20 — the height (v)
} A3DTexture;

#endif // MATROX_A3DTEXTURE_H
