#ifndef IVORYDELTA_H
#define IVORYDELTA_H

#include "types.h"

struct CopperVale0x20;
struct Face;

// A polygon queued for drawing, taken from the bottom of the draw buffer (FUN_1007d296):
// m_count pointers to its projected vertices (struct CopperVale0x20*) follow the header.
// SIZE 0xc
typedef struct IvoryDelta0xc {
	MechS16 m_count;     // 0x00 — bit 15 marks a shape queued in its place (FUN_100338bb)
	MechU16 m_unk0x02;   // 0x02 — the drawing flags
	struct Face* m_face; // 0x04
	MechS32 m_depth;     // 0x08
} IvoryDelta0xc;

#endif // IVORYDELTA_H
