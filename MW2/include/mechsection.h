#ifndef MECHSECTION_H
#define MECHSECTION_H

#include "decomp.h"
#include "types.h"

// One of a mech's eight sections (Mech::m_sections), as the network state message carries it.
// The low two nibbles of m_unk0x26 scale the damage level of m_unk0x00 and m_unk0x04; a
// section with m_unk0x08 used up and bit 0x2000 clear is destroyed.
// SIZE 0x28
typedef struct MechSection {
	MechS32 m_unk0x00;                // 0x00
	MechS32 m_unk0x04;                // 0x04
	MechS32 m_unk0x08;                // 0x08
	undefined m_unk0x0c[0x24 - 0x0c]; // 0x0c
	MechS16 m_unk0x24;                // 0x24
	MechS16 m_unk0x26;                // 0x26
} MechSection;

#endif // MECHSECTION_H
