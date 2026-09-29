#ifndef QUIETMARSH_H
#define QUIETMARSH_H

#include "decomp.h"
#include "transform.h"
#include "twilightgrove.h"
#include "types.h"

// A block of the static object cache (g_unk0x1010b6b0, 32 of them): the box BeginBlock
// opens, its center, the transform it was opened with, the matrix that places its objects
// and the enclosing block.
// SIZE 0x7c
typedef struct QuietMarsh0x7c {
	MechS32 m_unk0x00;           // 0x00
	MechS32 m_unk0x04;           // 0x04
	MechS32 m_unk0x08;           // 0x08
	MechS32 m_unk0x0c;           // 0x0c
	MechS32 m_unk0x10;           // 0x10
	MechS32 m_unk0x14;           // 0x14
	MechS32 m_unk0x18;           // 0x18
	MechS32 m_unk0x1c;           // 0x1c
	MechS32 m_unk0x20;           // 0x20
	TwilightGrove0x24 m_unk0x24; // 0x24
	Matrix m_unk0x48;            // 0x48
	MechS32 m_unk0x78;           // 0x78 — the enclosing block, -1: none
} QuietMarsh0x7c;

#endif // QUIETMARSH_H
