#ifndef SILVERTERN_H
#define SILVERTERN_H

#include "decomp.h"
#include "types.h"

// An ammunition bin of a mech (Mech::m_unk0x5c, Mech::m_unk0xc8 of them): the weapon it feeds and
// the critical-slot id it occupies (FUN_10007d06).
// SIZE 0x14
typedef struct SilverTern0x14 {
	MechS16 m_unk0x00; // 0x00
	MechS16 m_unk0x02; // 0x02
	MechS16 m_weapon;  // 0x04 — an index into Mech::m_weapons, or -1
	MechS16 m_id;      // 0x06 — the id in MechSection::m_slots
	MechS16 m_unk0x08; // 0x08
	MechS16 m_unk0x0a; // 0x0a
	MechS32 m_unk0x0c; // 0x0c
	MechS32 m_unk0x10; // 0x10
} SilverTern0x14;

#endif // SILVERTERN_H
