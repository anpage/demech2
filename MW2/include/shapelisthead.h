#ifndef SHAPELISTHEAD_H
#define SHAPELISTHEAD_H

#include "types.h"

struct Shape;

// The head of a list of shapes: the first 0x18 bytes of a Shape, which the list
// code links and unlinks as if it were a shape (shapelists.c).
// SIZE 0x18
typedef struct ShapeListHead {
	MechU16 m_unk0x00;       // 0x00
	MechU16 m_unk0x02;       // 0x02
	struct Shape* m_unk0x04; // 0x04
	struct Shape* m_unk0x08; // 0x08 — the first shape in the list
	struct Shape* m_unk0x0c; // 0x0c
	struct Shape* m_unk0x10; // 0x10 — the first shape in the second list
	MechU16 m_unk0x14;       // 0x14
	MechU16 m_unk0x16;       // 0x16
} ShapeListHead;

#endif // SHAPELISTHEAD_H
