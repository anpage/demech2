#ifndef DUSKMOTH_H
#define DUSKMOTH_H

#include "decomp.h"
#include "types.h"

struct ScarletOrchid0x4c;

// A face of a model (GraniteLattice0x18): its vertex indices are bytes at m_unk0x04 from
// the face itself, m_unk0x02 of them, and its plane's normal.
// SIZE 0x24
typedef struct DuskMoth0x24 {
	MechU16 m_unk0x00;                   // 0x00
	MechU16 m_unk0x02;                   // 0x02
	MechU32 m_unk0x04;                   // 0x04
	undefined m_unk0x08[0x14 - 0x08];    // 0x08
	MechS32 m_normal[3];                 // 0x14 — 2.29 fixed point
	struct ScarletOrchid0x4c* m_unk0x20; // 0x20
} DuskMoth0x24;

#endif // DUSKMOTH_H
