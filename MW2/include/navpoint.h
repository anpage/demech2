#ifndef NAVPOINT_H
#define NAVPOINT_H

#include "decomp.h"
#include "types.h"

// A nav point (AI target type 0x100): mission navs, and the ones AI players place for
// themselves.
// SIZE 0x54
typedef struct NavPoint {
	undefined m_unk0x00[0x08 - 0x00]; // 0x00
	MechU32 m_owner;                  // 0x08 — the AI target id (player | 0x200) that placed it
	undefined4 m_unk0x0c;             // 0x0c
	MechS32 m_radius;                 // 0x10
	undefined4 m_unk0x14;             // 0x14
	MechS32 m_position[3];            // 0x18
	MechS16 m_flags;                  // 0x24
	undefined m_unk0x26[0x54 - 0x26]; // 0x26
} NavPoint;

#endif // NAVPOINT_H
