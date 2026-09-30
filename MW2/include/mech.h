#ifndef MECH_H
#define MECH_H

#include "decomp.h"
#include "mechsection.h"
#include "types.h"

// A player's mech. Only the members matched code reaches are laid out.
struct Player;

typedef struct Mech {
	struct Player* m_player;           // 0x00
	MechS32 m_unk0x04;                 // 0x04
	MechS32 m_unk0x08;                 // 0x08
	MechS32 m_unk0x0c;                 // 0x0c
	undefined m_unk0x10[0x18 - 0x10];  // 0x10
	MechS32 m_unk0x18;                 // 0x18
	MechS32 m_unk0x1c;                 // 0x1c
	undefined m_unk0x20[0x28 - 0x20];  // 0x20
	MechS32 m_unk0x28;                 // 0x28
	MechS32 m_unk0x2c;                 // 0x2c
	undefined m_unk0x30[0x38 - 0x30];  // 0x30
	MechS32 m_unk0x38;                 // 0x38
	MechS32 m_unk0x3c;                 // 0x3c
	undefined m_unk0x40[0x48 - 0x40];  // 0x40
	MechS32 m_unk0x48;                 // 0x48
	MechS32 m_unk0x4c;                 // 0x4c
	undefined m_unk0x50[0x58 - 0x50];  // 0x50
	MechSection* m_sections;           // 0x58 — the eight sections
	undefined m_unk0x5c[0x94 - 0x5c];  // 0x5c
	MechS32 m_deltaHeat;               // 0x94 — heat added this tick
	MechS32 m_unk0x98;                 // 0x98
	MechS32 m_unk0x9c;                 // 0x9c
	MechS32 m_unk0xa0;                 // 0xa0
	MechS32 m_unk0xa4;                 // 0xa4
	undefined m_unk0xa8[0xc0 - 0xa8];  // 0xa8
	MechS32 m_unk0xc0;                 // 0xc0 — the jump jets fire while it is positive
	undefined m_unk0xc4[0xe0 - 0xc4];  // 0xc4
	MechS32 m_unk0xe0;                 // 0xe0
	undefined m_unk0xe4[0xe8 - 0xe4];  // 0xe4
	MechS32 m_radius;                  // 0xe8 — splash damage reaches it this much further
	MechS32 m_unk0xec;                 // 0xec
	undefined m_unk0xf0[0x100 - 0xf0]; // 0xf0
	MechS32 m_unk0x100;                // 0x100
	MechS32 m_unk0x104;                // 0x104
	MechS32 m_unk0x108;                // 0x108
	MechU16 m_unk0x10c;                // 0x10c
} Mech;

#endif // MECH_H
