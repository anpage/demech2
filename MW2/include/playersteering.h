#ifndef PLAYERSTEERING_H
#define PLAYERSTEERING_H

#include "decomp.h"
#include "types.h"

// The AI's steering: how the mech's controls are set each frame.
typedef struct PlayerSteering {
	undefined4 m_unk0x00;             // 0x00
	MechS32 m_unk0x04;                // 0x04 — swept between ±45 degrees by FUN_100562b4
	MechS32 m_throttle;               // 0x08
	MechS32 m_turn;                   // 0x0c
	undefined m_unk0x10[0x1d - 0x10]; // 0x10
	MechS8 m_unk0x1d;                 // 0x1d
	MechS8 m_unk0x1e;                 // 0x1e
	MechS8 m_unk0x1f;                 // 0x1f
	MechS8 m_unk0x20;                 // 0x20
	MechS8 m_unk0x21;                 // 0x21
	undefined m_unk0x22[0x25 - 0x22]; // 0x22
	MechS8 m_unk0x25;                 // 0x25
} PlayerSteering;

#endif // PLAYERSTEERING_H
