#ifndef MECH_H
#define MECH_H

#include "decomp.h"
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
	undefined m_unk0x40[0x50 - 0x40];  // 0x40
	MechS32 m_unk0x50;                 // 0x50
	MechS32 m_unk0x54;                 // 0x54
	MechS32 m_unk0x58;                 // 0x58
	undefined m_unk0x5c[0x80 - 0x5c];  // 0x5c
	MechU32 m_unk0x80;                 // 0x80
	MechS32 m_unk0x84;                 // 0x84
	MechS32 m_unk0x88;                 // 0x88
	MechS32 m_unk0x8c;                 // 0x8c
	MechS32 m_unk0x90;                 // 0x90
	MechS32 m_deltaHeat;               // 0x94 — heat added this tick
	MechS32 m_unk0x98;                 // 0x98
	MechS32 m_unk0x9c;                 // 0x9c
	MechS32 m_unk0xa0;                 // 0xa0
	MechS32 m_unk0xa4;                 // 0xa4
	undefined m_unk0xa8[0xe8 - 0xa8];  // 0xa8
	MechS32 m_radius;                  // 0xe8 — splash damage reaches it this much further
	undefined m_unk0xec[0x10c - 0xec]; // 0xec
	MechU16 m_unk0x10c;                // 0x10c
} Mech;

#endif // MECH_H
