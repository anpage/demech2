#ifndef CARCFG_H
#define CARCFG_H

#include "decomp.h"
#include "types.h"

#pragma pack(push, 1)

// The mission's combat record, saved to MW2CAR.CFG for the shell as one block. The hit and kill
// counters are indexed by side (GetPlayerSide, FUN_1003c30e): 0, 1 and 2 are counted apart.
// SIZE 0xd6
typedef struct CarCfg {
	undefined m_unk0x00[0x07 - 0x00]; // 0x00
	MechU16 m_unk0x07;                // 0x07 — players' mechs of side 1 the local player destroyed
	MechU16 m_unk0x09;                // 0x09 — side 2
	MechU16 m_unk0x0b;                // 0x0b — side 0
	MechU16 m_unk0x0d;                // 0x0d — things of side 1 the local player destroyed
	MechU16 m_unk0x0f;                // 0x0f — side 2
	MechU16 m_unk0x11;                // 0x11 — side 0
	MechU16 m_unk0x13;                // 0x13 — shots fired by the local player
	MechU16 m_unk0x15;                // 0x15 — hits by the local player on side 1
	MechU16 m_unk0x17;                // 0x17 — side 2
	MechU16 m_unk0x19;                // 0x19 — side 0
	MechU16 m_unk0x1b;                // 0x1b — hits taken by the local player
	MechU8 m_unk0x1d;                 // 0x1d — how the local player's mech went down (2, 4)
	MechU16 m_unk0x1e;                // 0x1e — players' mechs of side 1 with flag 0x1400 destroyed
	MechU16 m_unk0x20;                // 0x20 — side 2
	MechU16 m_unk0x22;                // 0x22 — side 0
	MechU16 m_unk0x24;                // 0x24 — things of side 1 the local team destroyed
	MechU16 m_unk0x26;                // 0x26 — side 2
	MechU16 m_unk0x28;                // 0x28 — side 0
	MechU16 m_unk0x2a;                // 0x2a — shots fired by the local team
	MechU16 m_unk0x2c;                // 0x2c — hits by the local team on side 0
	MechU16 m_unk0x2e;                // 0x2e — side 2
	MechU16 m_unk0x30;                // 0x30 — side 1
	MechU16 m_unk0x32;                // 0x32 — hits taken by the local team
	MechU16 m_unk0x34;                // 0x34 — mechs of the local team destroyed
	MechU16 m_unk0x36;                // 0x36 — of those, the ones in m_unk0xa0 state 5
	MechS16 m_unk0x38[6];             // 0x38 — players (0-2) and game things (3-5) of sides 0, 2 and 1
	MechU16 m_unk0x44;                // 0x44 — other mechs of side 1 the local player destroyed
	MechU16 m_unk0x46;                // 0x46 — side 2
	MechU16 m_unk0x48;                // 0x48 — side 0
	MechU16 m_unk0x4a;                // 0x4a — other mechs of side 1 with flag 0x1400 destroyed
	MechU16 m_unk0x4c;                // 0x4c — side 2
	MechU16 m_unk0x4e;                // 0x4e — side 0
	MechU16 m_unk0x50;                // 0x50 — the players of a network game
	MechU16 m_unk0x52[8][8];          // 0x52 — kills, by killer and victim player
	MechS32 m_unk0xd2;                // 0xd2 — the last player whose SU message arrived
} CarCfg;

#pragma pack(pop)

#endif // CARCFG_H
