#ifndef PLAYERSTEERING_H
#define PLAYERSTEERING_H

#include "decomp.h"
#include "types.h"

// The AI's steering: how the mech's controls are set each frame.
// SIZE 0x48
typedef struct PlayerSteering {
	MechS32 m_unk0x00;                // 0x00
	MechS32 m_unk0x04;                // 0x04 — swept between ±45 degrees by FUN_100562b4
	MechS32 m_throttle;               // 0x08
	MechS32 m_turn;                   // 0x0c
	undefined m_unk0x10[0x14 - 0x10]; // 0x10
	MechS16 m_keyCode;                // 0x14 — the local player's: the key INPUT.MAP passed on
	undefined m_unk0x16[0x1d - 0x16]; // 0x16
	MechS8 m_unk0x1d;                 // 0x1d
	MechS8 m_unk0x1e;                 // 0x1e
	MechS8 m_unk0x1f;                 // 0x1f
	MechS8 m_unk0x20;                 // 0x20
	MechS8 m_unk0x21;                 // 0x21
	undefined m_unk0x22[0x24 - 0x22]; // 0x22
	MechS8 m_unk0x24;                 // 0x24
	MechS8 m_unk0x25;                 // 0x25 — fire the selected weapon
	MechS8 m_unk0x26;                 // 0x26
	MechS8 m_unk0x27;                 // 0x27 — fire every weapon
	MechS8 m_unk0x28;                 // 0x28 — fire weapon group 0
	MechS8 m_unk0x29;                 // 0x29 — group 1
	MechS8 m_unk0x2a;                 // 0x2a — group 2
	undefined m_unk0x2b;              // 0x2b
	MechS8 m_unk0x2c;                 // 0x2c — toggles g_unk0x100a6d38
	MechS8 m_unk0x2d;                 // 0x2d
	MechS8 m_unk0x2e;                 // 0x2e
	MechS8 m_unk0x2f;                 // 0x2f
	MechS8 m_unk0x30;                 // 0x30
	undefined m_unk0x31[0x3a - 0x31]; // 0x31
	MechS8 m_unk0x3a;                 // 0x3a — FUN_1005fa22 claims the target (a key press)
	undefined m_unk0x3b[0x42 - 0x3b]; // 0x3b
	MechS8 m_unk0x42;                 // 0x42
	MechS8 m_unk0x43;                 // 0x43
	undefined m_unk0x44[0x48 - 0x44]; // 0x44
} PlayerSteering;

#endif // PLAYERSTEERING_H
