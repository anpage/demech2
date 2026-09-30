#ifndef WEAPONDEF_H
#define WEAPONDEF_H

#include "decomp.h"
#include "types.h"

// A weapon type, from the table of weapon definitions (g_weaponDefs: LLASER, MLASER, ...).
// SIZE 0x58
typedef struct WeaponDef {
	MechS32 m_shotType; // 0x00 — the Shot::m_type a launch takes from the free shots
	MechS32 m_impact;   // 0x04 — the effect the shot sets off (Shot::m_impact)
	MechS32 m_unk0x08;  // 0x08 — an effect started at each launch (FUN_1006b1c8)
	MechS32 m_unk0x0c;  // 0x0c — another
	MechS32 m_unk0x10;  // 0x10 — the next volley follows at once while the trigger is held
	MechS32 m_unk0x14;  // 0x14 — a launch takes a shot
	MechS32 m_unk0x18;  // 0x18 — the shots follow the locked target
	MechS32 m_volley;   // 0x1c — shots per volley
	MechS32 m_unk0x20;  // 0x20
	MechS32 m_sound;    // 0x24 — the launch sound, or <= 0
	MechS32 m_unk0x28;  // 0x28 — scales the gravity on the shot
	MechS32 m_speed;    // 0x2c
	MechS32 m_damage;   // 0x30
	MechS32 m_heat;     // 0x34 — added to the firing mech's heat per shot
	MechS32 m_shotHeat; // 0x38 — added to the heat of the mech the shot hits
	MechS32 m_unk0x3c;  // 0x3c — a range, shorter than m_unk0x40
	MechS32 m_unk0x40;  // 0x40 — a range
	MechS32 m_recycle;  // 0x44 — ticks between volleys
	MechS32 m_interval; // 0x48 — ticks between the shots of a volley
	MechS32 m_lifetime; // 0x4c — the shot's, in ticks
	MechChar m_name[8]; // 0x50
} WeaponDef;

#endif // WEAPONDEF_H
