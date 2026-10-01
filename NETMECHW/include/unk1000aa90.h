#ifndef UNK1000AA90_H
#define UNK1000AA90_H

#include "decomp.h"
#include "types.h"

#include <dplay.h>

// An entry of the player table, kept by player slot.
// SIZE 0x24
struct NetPlayer {
	MechChar m_name[DPSHORTNAMELEN]; // 0x00
	MechS32 m_team;                  // 0x14
	DPID m_id;                       // 0x18
	MechU32 m_flags;                 // 0x1c
	undefined4 m_unk0x20;            // 0x20
};

// The functions and globals of unk1000aa90.cpp that other units use.
MechS32 FUN_1000aeff(DPID p_id, NetPlayer* p_player);
MechS32 FUN_1000afa8(MechS32 p_index, NetPlayer* p_player);
MechS32 FUN_1000b02b(DPID p_id);
undefined4 FUN_1000b0b3();
void FUN_1000b3c5(MechS32 p_index);

#endif // UNK1000AA90_H
