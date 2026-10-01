#ifndef UNK1000AA90_H
#define UNK1000AA90_H

#include "decomp.h"
#include "types.h"

#include <dplay.h>

// An entry of the player table, kept by player slot.
// SIZE 0x24
struct NetPlayer {
	enum {
		c_flagFree = 0x01 // the slot is empty
	};

	MechChar m_name[DPSHORTNAMELEN]; // 0x00
	MechS32 m_team;                  // 0x14
	DPID m_id;                       // 0x18
	MechU32 m_flags;                 // 0x1c
	MechS32 m_unk0x20;               // 0x20: periods without a message from the player
};

// The functions and globals of unk1000aa90.cpp that other units use.
extern NetPlayer g_players[8];

// Whether the slot p_index of g_players holds a player. The original's test has the shape of a
// short-circuit || whose second operand is constant false (cmp; je; jmp), as a macro would
// leave it; a plain test of m_flags compiles to a single jne.
#define IS_PLAYER_SLOT_USED(p_index) (g_players[p_index].m_flags == 0 || FALSE)
extern MechS32 g_playerCount;

MechS32 FUN_1000aa90(MechChar* p_name, DPID p_id);
MechS32 FUN_1000acc9(DPID p_id);
MechS32 FUN_1000aeff(DPID p_id, NetPlayer* p_player);
MechS32 FUN_1000afa8(MechS32 p_index, NetPlayer* p_player);
MechS32 FUN_1000b02b(DPID p_id);
MechS32 FUN_1000b0b3();
void FUN_1000b0c8();
DPID FUN_1000b178();
void FUN_1000b1fe(MechU8 p_teams);
MechS32 FUN_1000b295(MechS32 p_team);
MechS32 FUN_1000b30e(MechS32 p_index);
void FUN_1000b3c5(MechS32 p_index);

#endif // UNK1000AA90_H
