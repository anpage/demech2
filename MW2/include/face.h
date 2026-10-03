#ifndef FACE_H
#define FACE_H

#include "decomp.h"
#include "types.h"

struct Shape;

// A face of a model (Model): its vertex indices are bytes at m_unk0x04 from
// the face itself, m_unk0x02 of them, and its plane's normal.
// SIZE 0x24
typedef struct Face {
	MechU16 m_unk0x00;       // 0x00
	MechU16 m_unk0x02;       // 0x02
	MechU32 m_unk0x04;       // 0x04
	MechS32 m_unk0x08;       // 0x08 — a copy of m_normal (ComputeFaceNormal)
	MechS32 m_unk0x0c;       // 0x0c
	MechS32 m_unk0x10;       // 0x10
	MechS32 m_normal[3];     // 0x14 — 2.29 fixed point
	struct Shape* m_unk0x20; // 0x20
} Face;

#endif // FACE_H
