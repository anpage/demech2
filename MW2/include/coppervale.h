#ifndef COPPERVALE_H
#define COPPERVALE_H

#include "decomp.h"
#include "types.h"

// A projected vertex, taken from the top of the draw buffer (FUN_1007d248): its view-space
// position, its screen position and two more values the polygon routines interpolate along
// clipped edges (FUN_10033e92).
// SIZE 0x20
typedef struct CopperVale0x20 {
	MechS32 m_unk0x00;                // 0x00 — view x
	MechS32 m_unk0x04;                // 0x04 — view y
	MechS32 m_unk0x08;                // 0x08 — depth
	MechS32 m_unk0x0c;                // 0x0c — screen x
	MechS32 m_unk0x10;                // 0x10 — screen y
	MechS32 m_unk0x14;                // 0x14
	MechS32 m_unk0x18;                // 0x18
	undefined m_unk0x1c[0x20 - 0x1c]; // 0x1c
} CopperVale0x20;

#endif // COPPERVALE_H
