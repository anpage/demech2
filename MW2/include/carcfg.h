#ifndef CARCFG_H
#define CARCFG_H

#include "decomp.h"
#include "types.h"

#pragma pack(push, 1)

// The mission's combat record, saved to MW2CAR.CFG for the shell as one block. The hit and kill
// counters are indexed by side (GetPlayerSide, FUN_1003c30e): 0, 1 and 2 are counted apart.
// SIZE 0xd6
typedef struct CarCfg {
	undefined m_unk0x00[0x0d - 0x00]; // 0x00
	MechU16 m_unk0x0d;                // 0x0d — things of side 1 the local player destroyed
	MechU16 m_unk0x0f;                // 0x0f — side 2
	MechU16 m_unk0x11;                // 0x11 — side 0
	MechU16 m_unk0x13;                // 0x13 — shots fired by the local player
	MechU16 m_unk0x15;                // 0x15 — hits by the local player on side 1
	MechU16 m_unk0x17;                // 0x17 — side 2
	MechU16 m_unk0x19;                // 0x19 — side 0
	MechU16 m_unk0x1b;                // 0x1b — hits taken by the local player
	MechU8 m_unk0x1d;                 // 0x1d
	undefined m_unk0x1e[0x24 - 0x1e]; // 0x1e
	MechU16 m_unk0x24;                // 0x24 — things of side 1 the local team destroyed
	MechU16 m_unk0x26;                // 0x26 — side 2
	MechU16 m_unk0x28;                // 0x28 — side 0
	MechU16 m_unk0x2a;                // 0x2a — shots fired by the local team
	MechU16 m_unk0x2c;                // 0x2c — hits by the local team on side 0
	MechU16 m_unk0x2e;                // 0x2e — side 2
	MechU16 m_unk0x30;                // 0x30 — side 1
	MechU16 m_unk0x32;                // 0x32 — hits taken by the local team
	undefined m_unk0x34[0x50 - 0x34]; // 0x34
	MechU16 m_unk0x50;                // 0x50 — the players of a network game
	MechU16 m_unk0x52[8][8];          // 0x52 — kills, by killer and victim player
	MechS32 m_unk0xd2;                // 0xd2 — the last player whose SU message arrived
} CarCfg;

#pragma pack(pop)

#endif // CARCFG_H
