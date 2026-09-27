#ifndef HAZELSTAR0X80_H
#define HAZELSTAR0X80_H

#include "decomp.h"
#include "sableroster0x24.h"
#include "types.h"

// SIZE 0x80
// A star of a custom battle: the player's (userstar.bwd) or the enemy's.
struct HazelStar0x80 {
	MechS32 m_unk0x00;            // 0x00 — formation, an index into g_unk0x1005b820 (mechvariant.cpp)
	MechS32 m_unk0x04;            // 0x04 — selected mech
	MechS32 m_unk0x08;            // 0x08
	MechS32 m_unk0x0c;            // 0x0c — mechs in the star
	MechS32 m_unk0x10;            // 0x10
	SableRoster0x24 m_unk0x14[3]; // 0x14
};

#endif // HAZELSTAR0X80_H
