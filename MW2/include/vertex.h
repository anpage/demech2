#ifndef VERTEX_H
#define VERTEX_H

#include "decomp.h"
#include "types.h"

struct ProjectedVertex;

// A vertex of a model (Model): its position in the model, the position
// FUN_10039a30 transforms it to, and two more values AddShapeVertex sets.
// SIZE 0x2c
typedef struct Vertex {
	MechS32 m_unk0x00;                 // 0x00
	MechS32 m_unk0x04;                 // 0x04
	MechS32 m_unk0x08;                 // 0x08
	MechS32 m_unk0x0c;                 // 0x0c
	MechS32 m_unk0x10;                 // 0x10
	MechS32 m_unk0x14;                 // 0x14
	undefined4 m_unk0x18;              // 0x18
	undefined4 m_unk0x1c;              // 0x1c
	undefined4 m_unk0x20;              // 0x20
	struct ProjectedVertex* m_unk0x24; // 0x24 — the projected copy (FUN_10048c50), once made
	MechU8 m_unk0x28;                  // 0x28 — bit 2: the vertex has been projected this frame
	undefined m_unk0x29[0x2c - 0x29];  // 0x29
} Vertex;

#endif // VERTEX_H
