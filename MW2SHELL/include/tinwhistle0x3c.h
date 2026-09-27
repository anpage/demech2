#ifndef TINWHISTLE0X3C_H
#define TINWHISTLE0X3C_H

#include "decomp.h"
#include "types.h"

class EmberGlyph0x3e;

// SIZE 0x3c
// One pilot career record; the roster file MW2REG.CFG holds 20 of them.
struct TinWhistle0x3c {
	undefined4 m_unk0x00;      // 0x00
	undefined4 m_unk0x04;      // 0x04
	undefined4 m_unk0x08;      // 0x08
	MechS32 m_mission;         // 0x0c — missions completed, the index of the next
	MechS32 m_rank;            // 0x10 — index into g_rankNames
	MechS32 m_honor;           // 0x14
	undefined4 m_unk0x18;      // 0x18
	undefined4 m_unk0x1c;      // 0x1c
	undefined4 m_unk0x20;      // 0x20
	undefined4 m_unk0x24;      // 0x24
	MechChar m_callsign[0x10]; // 0x28
	EmberGlyph0x3e* m_glyph;   // 0x38 — the callsign on the roster screen, not saved
};

#endif // TINWHISTLE0X3C_H
