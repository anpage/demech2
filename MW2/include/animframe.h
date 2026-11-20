#ifndef ANIMFRAME_H
#define ANIMFRAME_H

#include "decomp.h"
#include "types.h"

// One frame of an animation set: a CEL resource, loaded on first use.
// SIZE 0x08
typedef struct AnimFrame {
	MechS16 m_resourceId; // 0x00 — -1 for a free slot
	MechS16 m_useCount;   // 0x02
#ifdef MW2_A3D
	// The Matrox edition's frames are 0x14 bytes, with the texture's average color (0-31 per
	// component, CalcAverageBitmapColors) and m_data at 0x10.
	MechFloat m_red;   // 0x04
	MechFloat m_green; // 0x08
	MechFloat m_blue;  // 0x0c
#endif
	MechU16* m_data; // 0x04 (0x10 in the Matrox edition) — width, height, then the pixels
#ifdef MW2_POWERVR
	// The PowerVR edition's frames are 0x20 bytes: the SGL cached texture made of them, and the
	// frame it was last used in.
	undefined4 m_unk0x14; // 0x14
	MechS32 m_texture;    // 0x18
	MechS32 m_lastUsed;   // 0x1c
#endif
} AnimFrame;

#endif // ANIMFRAME_H
