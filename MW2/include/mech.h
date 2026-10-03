#ifndef MECH_H
#define MECH_H

#include "decomp.h"
#include "mechsection.h"
#include "ramp.h"
#include "types.h"
#include "weaponslot.h"

// A player's mech. Only the members matched code reaches are laid out.
struct SceneObject;
struct Player;

#pragma pack(push, 1)

// SIZE 0x10e
typedef struct Mech {
	struct Player* m_player;          // 0x00
	Ramp m_unk0x04;                   // 0x04 — the torso twist (FUN_1005a203)
	Ramp m_unk0x14;                   // 0x14 — the torso pitch (FUN_1005a203)
	Ramp m_unk0x24;                   // 0x24 — the position's x (FUN_1006831a)
	Ramp m_unk0x34;                   // 0x34 — the position's z
	Ramp m_unk0x44;                   // 0x44 — the position's y
	WeaponSlot* m_weapons;            // 0x54 — ten
	MechSection* m_sections;          // 0x58 — the eight sections
	void* m_ammoBins;                 // 0x5c — the rest of the allocation, after the sections: AmmoBin records
	struct SceneObject* m_unk0x60;    // 0x60
	struct SceneObject* m_unk0x64;    // 0x64
	struct SceneObject* m_objects[8]; // 0x68 — parts: the weapons fire from them, 6 and 7 are the jump jets
	MechS32 m_unk0x88;                // 0x88 — FUN_1006844e's speed towards the nav point
	MechS32 m_unk0x8c;                // 0x8c — the weapons' lock-on countdown (FUN_10045eac)
	MechS32 m_unk0x90;                // 0x90
	MechS32 m_deltaHeat;              // 0x94 — heat added this tick
	MechS32 m_unk0x98;                // 0x98
	MechS32 m_unk0x9c;                // 0x9c
	MechS32 m_unk0xa0;                // 0xa0
	MechS32 m_unk0xa4;                // 0xa4
	MechS32 m_weaponCount;            // 0xa8
	MechS32 m_selectedWeapon;         // 0xac — an index into m_weapons, or -1
	MechS32 m_unk0xb0;                // 0xb0
	MechS32 m_unk0xb4;                // 0xb4
	MechS32 m_unk0xb8;                // 0xb8
	MechS32 m_unk0xbc;                // 0xbc — the autopilot: 1 and 2 are on
	MechS32 m_unk0xc0;                // 0xc0 — the jump jets fire while it is positive
	MechS32 m_unk0xc4;                // 0xc4 — jump jet fuel units; each critical hit takes one
	MechS32 m_ammoBinCount;           // 0xc8
	MechS32 m_unk0xcc;                // 0xcc
	MechS32 m_unk0xd0;                // 0xd0 — the cockpit's height (g_unk0x100a2434)
	MechS32 m_unk0xd4;                // 0xd4
	MechS32 m_unk0xd8;                // 0xd8
	MechS32 m_unk0xdc;                // 0xdc
	MechS32 m_unk0xe0;                // 0xe0
	MechS32 m_unk0xe4;                // 0xe4 — its mass: FUN_1007669e scales collision damage by it
	MechS32 m_radius;                 // 0xe8 — splash damage reaches it this much further
	MechS32 m_unk0xec;                // 0xec
	MechS32 m_unk0xf0;                // 0xf0
	MechS32 m_unk0xf4;                // 0xf4 — the velocity
	MechS32 m_unk0xf8;                // 0xf8
	MechS32 m_unk0xfc;                // 0xfc
	MechS32 m_unk0x100;               // 0x100
	MechS32 m_unk0x104;               // 0x104
	MechS32 m_unk0x108;               // 0x108
	// 0x10c: 0x1 a trigger is held, 0x40 locking on, 0x80 locked on, 0x4000 moving a weapon between
	// groups, 0x8000 the target is in the lock cone.
	MechU16 m_unk0x10c; // 0x10c
} Mech;

#pragma pack(pop)

#endif // MECH_H
