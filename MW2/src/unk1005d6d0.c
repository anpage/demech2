#include "unk1005d6d0.h"

#include "decomp.h"
#include "mechsection.h"
#include "types.h"

// A .MEK file's header, as far as FUN_1005e534 reads it.
typedef struct MekHeader {
	MechS32 m_tons;                   // 0x00
	undefined m_unk0x04[0x10 - 0x04]; // 0x04
	MechS32 m_weaponCount;            // 0x10
} MekHeader;

// A .MEK file's weapon: its type and a second value.
typedef struct MekWeapon {
	MechS32 m_type;       // 0x00 — a weapon type, times 100
	undefined4 m_unk0x04; // 0x04
} MekWeapon;

// The value each weapon type adds to a mech's (FUN_1005e534).
// GLOBAL: MW2 0x100aa730
MechU16 g_unk0x100aa730[30] = {183, 137, 91,  46,  51,  34, 17,  74,  49, 25, 2,   228, 42, 82, 123,
							   157, 74,  144, 247, 329, 4,  228, 166, 51, 21, 177, 74,  16, 50, 40};

// STUB: MW2 0x1005d6d0
MechS32 FUN_1005d6d0(struct Mech* p_mech, MechChar* p_name, MechS32 p_unk0x08, MechChar* p_unk0x0c)
{
	STUB(0x1005d6d0);
	return 0;
}

// Returns a mech's value: its tonnage, scaled by the critical slots it fills (by kind), plus its
// armor and the value of each of its weapons (up to ten).
// Stack-slot permutation: the kinds table sits elsewhere in the frame, so its accesses and the
// jumps over them differ in their encoding.
// FUNCTION: MW2 0x1005e534
MechU16 FUN_1005e534(MekHeader* p_header, MechSection* p_sections, MekWeapon* p_weapons)
{
	MechS32 armor;
	MechS16 kinds[23][3] = {{5000, 1, 0}, {5050, 1, 0}, {5100, -60, 0}, {5150, -60, 0}, {5200, -50, 0}, {5250, 1, 0},
							{5300, 1, 0}, {5350, 1, 0}, {5400, 1, 0},   {5450, 1, 0},   {5500, 1, 0},   {5550, 1, 0},
							{5600, 1, 0}, {5650, 1, 0}, {5700, 2, 0},   {5750, 0, 0},   {5800, 2, 0},   {5850, 3, 0},
							{5900, 2, 0}, {6000, 1, 0}, {7000, 1, 0},   {8000, 0, 0},   {9000, 0, 0}};
	MechS32 count;
	MechS32 i;
	MechU16 value;
	MechS32 j;
	MechS32 k;

	value = p_header->m_tons;
	count = 0;
	armor = 0;
	for (i = 0; i < 8; i++, p_sections++) {
		for (j = 0; j < p_sections->m_unk0x24; j++) {
			armor += p_sections->m_armor[0] + p_sections->m_armor[1];
			for (k = 22; k >= 0; k--) {
				if (p_sections->m_slots[j] > kinds[k][0]) {
					kinds[k][2]++;
					count++;
					break;
				}
			}
		}

		for (k = 20; k >= 0; k--) {
			if (kinds[k][2]) {
				if (kinds[k][1] < 0) {
					value += -(kinds[k][1] * kinds[k][2]);
				}
				else {
					value += kinds[k][1] * kinds[k][2] * p_header->m_tons;
				}
			}
		}

		if (kinds[21][2]) {
			value += count * 2;
		}
		else {
			value += count;
		}

		value += armor;
		value += (MechU16) p_sections->m_unk0x08;
	}

	for (i = 0; i < p_header->m_weaponCount && i < 10; i++, p_weapons++) {
		value += g_unk0x100aa730[p_weapons->m_type / 100];
	}

	return value;
}
