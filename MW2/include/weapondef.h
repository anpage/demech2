#ifndef WEAPONDEF_H
#define WEAPONDEF_H

#include "decomp.h"
#include "types.h"

// A weapon type, from the table of weapon definitions (LLASER, MLASER, ...).
// SIZE 0x58
typedef struct WeaponDef {
	undefined m_unk0x00[0x3c - 0x00]; // 0x00
	MechS32 m_unk0x3c;                // 0x3c — a range, shorter than m_unk0x40
	MechS32 m_unk0x40;                // 0x40 — a range
	undefined m_unk0x44[0x58 - 0x44]; // 0x44
} WeaponDef;

#endif // WEAPONDEF_H
