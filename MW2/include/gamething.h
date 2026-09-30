#ifndef GAMETHING_H
#define GAMETHING_H

#include "decomp.h"
#include "types.h"

// SIZE 0x40
typedef struct GameThing {
	MechS16 m_unk0x00;                // 0x00
	MechS16 m_unk0x02;                // 0x02 — a bit per team that reached it
	MechS32 m_unk0x04;                // 0x04
	MechS32 m_unk0x08;                // 0x08 — hit points: destroyed when they run out
	MechS32 m_unk0x0c;                // 0x0c
	undefined m_unk0x10[0x14 - 0x10]; // 0x10
	MechU8 m_unk0x14;                 // 0x14
	undefined m_unk0x15[0x40 - 0x15]; // 0x15
} GameThing;

#endif // GAMETHING_H
