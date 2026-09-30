#ifndef MECH_H
#define MECH_H

#include "decomp.h"
#include "mechsection.h"
#include "types.h"
#include "weaponslot.h"

// A player's mech. Only the members matched code reaches are laid out.
struct AmberWillow0x7c;
struct Player;

#pragma pack(push, 1)

// SIZE 0x10e
typedef struct Mech {
	struct Player* m_player;              // 0x00
	MechS32 m_unk0x04;                    // 0x04
	MechS32 m_unk0x08;                    // 0x08
	MechS32 m_unk0x0c;                    // 0x0c
	undefined m_unk0x10[0x18 - 0x10];     // 0x10
	MechS32 m_unk0x18;                    // 0x18
	MechS32 m_unk0x1c;                    // 0x1c
	undefined m_unk0x20[0x28 - 0x20];     // 0x20
	MechS32 m_unk0x28;                    // 0x28
	MechS32 m_unk0x2c;                    // 0x2c
	undefined m_unk0x30[0x38 - 0x30];     // 0x30
	MechS32 m_unk0x38;                    // 0x38
	MechS32 m_unk0x3c;                    // 0x3c
	undefined m_unk0x40[0x48 - 0x40];     // 0x40
	MechS32 m_unk0x48;                    // 0x48
	MechS32 m_unk0x4c;                    // 0x4c
	undefined m_unk0x50[0x54 - 0x50];     // 0x50
	WeaponSlot* m_weapons;                // 0x54 — ten
	MechSection* m_sections;              // 0x58 — the eight sections
	void* m_unk0x5c;                      // 0x5c — the rest of the allocation, after the sections
	struct AmberWillow0x7c* m_unk0x60;    // 0x60
	struct AmberWillow0x7c* m_unk0x64;    // 0x64
	struct AmberWillow0x7c* m_objects[9]; // 0x68 — parts: the weapons fire from them, 6 and 7 are the jump jets
	MechS32 m_unk0x8c;                    // 0x8c — the weapons' lock-on countdown (FUN_10045eac)
	undefined m_unk0x90[0x94 - 0x90];     // 0x90
	MechS32 m_deltaHeat;                  // 0x94 — heat added this tick
	MechS32 m_unk0x98;                    // 0x98
	MechS32 m_unk0x9c;                    // 0x9c
	MechS32 m_unk0xa0;                    // 0xa0
	MechS32 m_unk0xa4;                    // 0xa4
	MechS32 m_weaponCount;                // 0xa8
	MechS32 m_selectedWeapon;             // 0xac — an index into m_weapons, or -1
	undefined m_unk0xb0[0xb8 - 0xb0];     // 0xb0
	MechS32 m_unk0xb8;                    // 0xb8
	MechS32 m_unk0xbc;                    // 0xbc — the autopilot: 1 and 2 are on
	MechS32 m_unk0xc0;                    // 0xc0 — the jump jets fire while it is positive
	undefined m_unk0xc4[0xcc - 0xc4];     // 0xc4
	MechS32 m_unk0xcc;                    // 0xcc
	undefined m_unk0xd0[0xe0 - 0xd0];     // 0xd0
	MechS32 m_unk0xe0;                    // 0xe0
	undefined m_unk0xe4[0xe8 - 0xe4];     // 0xe4
	MechS32 m_radius;                     // 0xe8 — splash damage reaches it this much further
	MechS32 m_unk0xec;                    // 0xec
	undefined m_unk0xf0[0xf4 - 0xf0];     // 0xf0
	MechS32 m_unk0xf4;                    // 0xf4 — the velocity
	MechS32 m_unk0xf8;                    // 0xf8
	MechS32 m_unk0xfc;                    // 0xfc
	MechS32 m_unk0x100;                   // 0x100
	MechS32 m_unk0x104;                   // 0x104
	MechS32 m_unk0x108;                   // 0x108
	// 0x10c: 0x1 a trigger is held, 0x40 locking on, 0x80 locked on, 0x4000 moving a weapon between
	// groups, 0x8000 the target is in the lock cone.
	MechU16 m_unk0x10c; // 0x10c
} Mech;

#pragma pack(pop)

#endif // MECH_H
