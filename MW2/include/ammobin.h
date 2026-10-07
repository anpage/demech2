#ifndef AMMOBIN_H
#define AMMOBIN_H

#include "decomp.h"
#include "fixedfloat.h"
#include "types.h"

// An ammunition bin of a mech (Mech::m_ammoBins, Mech::m_ammoBinCount of them): the weapon it feeds and
// the critical-slot id it occupies (DestroyMech).
// The edition's m_damage is a float, which moves the members after it by two bytes (SIZE 0x16).
#pragma pack(push, 1)

// SIZE 0x14
typedef struct AmmoBin {
	MechS16 m_type;    // 0x00
	MechS16 m_shots;   // 0x02
	MechS16 m_weapon;  // 0x04 — an index into Mech::m_weapons, or -1
	MechS16 m_id;      // 0x06 — the id in MechSection::m_slots
	MechS16 m_unk0x08; // 0x08
#ifdef MW2_MATROX
	MechScalar m_damage; // 0x0a
#else
	MechS16 m_damage; // 0x0a
#endif
	MechS32 m_shotHeat; // 0x0c (0x0e in the edition)
	MechS32 m_heat;     // 0x10 (0x12 in the edition)
} AmmoBin;
#ifdef MW2_MATROX
#pragma pack(pop)
#endif

#pragma pack(pop)

#endif // AMMOBIN_H
