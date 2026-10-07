#ifndef ANIMFRAME_H
#define ANIMFRAME_H

#include "decomp.h"
#include "types.h"

// One frame of an animation set: a CEL resource, loaded on first use.
// SIZE 0x08
typedef struct AnimFrame {
	MechS16 m_resourceId; // 0x00 — -1 for a free slot
	MechS16 m_useCount;   // 0x02
#ifdef MW2_MATROX
	// The edition's frames are 0x14 bytes, with m_data at 0x10.
	undefined m_unk0x04[0x10 - 0x04]; // 0x04
#endif
	MechU16* m_data; // 0x04 (0x10 in the edition) — width, height, then the pixels
} AnimFrame;

#endif // ANIMFRAME_H
