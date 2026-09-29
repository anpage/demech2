#ifndef MECH_H
#define MECH_H

#include "decomp.h"
#include "types.h"

// A player's mech. Only the members matched code reaches are laid out.
typedef struct Mech {
	undefined4 m_unk0x00;             // 0x00
	MechS32 m_unk0x04;                // 0x04
	undefined m_unk0x08[0x50 - 0x08]; // 0x08
	MechS32 m_unk0x50;                // 0x50
	MechS32 m_unk0x54;                // 0x54
	MechS32 m_unk0x58;                // 0x58
	undefined m_unk0x5c[0x80 - 0x5c]; // 0x5c
	MechU32 m_unk0x80;                // 0x80
	MechS32 m_unk0x84;                // 0x84
	MechS32 m_unk0x88;                // 0x88
	MechS32 m_unk0x8c;                // 0x8c
	MechS32 m_unk0x90;                // 0x90
	undefined m_unk0x94[0x9c - 0x94]; // 0x94
	MechS32 m_unk0x9c;                // 0x9c
} Mech;

#endif // MECH_H
