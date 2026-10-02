#ifndef UNK10007C30_H
#define UNK10007C30_H

#include "decomp.h"
#include "types.h"

#include <windows.h>

// An entry of a mech table, an "MTAB" resource of mw2.prj: a count, then the entries.
// SIZE 0x2d
typedef struct MechTableEntry {
	undefined m_unk0x00[3];       // 0x00
	MechChar m_code[4];           // 0x03: the chassis code, the first three letters of its mech files
	MechChar m_mechFile[9];       // 0x07
	MechChar m_name[0x2d - 0x10]; // 0x10: shown in the mech list (FUN_10003154)
} MechTableEntry;

#pragma pack(1)

// A section of a mech in a .MEK file.
// SIZE 0x28
typedef struct MechSection {
	MechS32 m_unk0x00;   // 0x00
	MechS32 m_unk0x04;   // 0x04
	MechS32 m_unk0x08;   // 0x08
	MechU16 m_slots[12]; // 0x0c: the weapon types in the section's slots
	MechS16 m_unk0x24;   // 0x24
	MechS16 m_unk0x26;   // 0x26
} MechSection;

// A weapon of a mech in a .MEK file.
// SIZE 0x08
typedef struct MechWeapon {
	MechS32 m_type;     // 0x00
	MechS32 m_location; // 0x04: the index of its section
} MechWeapon;

// An ammunition bin of a mech in a .MEK file.
// SIZE 0x08
typedef struct MechAmmo {
	MechS32 m_unk0x00; // 0x00
	MechS32 m_type;    // 0x04: the weapon type it feeds
} MechAmmo;

// The last part of a .MEK file, after the weapons and the ammunition.
// SIZE 0x32
typedef struct MechTrailer {
	MechChar m_name[0x1e]; // 0x00
	MechS32 m_unk0x1e;     // 0x1e
	MechS32 m_unk0x22;     // 0x22
	MechS32 m_unk0x26;     // 0x26
	MechS32 m_unk0x2a;     // 0x2a
	MechS32 m_unk0x2e;     // 0x2e
} MechTrailer;

// A .MEK mech file: this fixed part, then m_weaponCount MechWeapons, m_ammoCount MechAmmos and a
// MechTrailer.
// SIZE 0x158
typedef struct MechFile {
	MechS32 m_mass;            // 0x00
	MechS32 m_unk0x04;         // 0x04
	MechS32 m_jumpJets;        // 0x08
	MechS32 m_heatSinks;       // 0x0c
	MechS32 m_weaponCount;     // 0x10
	MechS32 m_ammoCount;       // 0x14
	MechSection m_sections[8]; // 0x18
} MechFile;

// A section of a PackedMech.
// SIZE 0x21
typedef struct PackedMechSection {
	MechS16 m_unk0x00;   // 0x00
	MechS16 m_unk0x02;   // 0x02
	MechS16 m_unk0x04;   // 0x04
	MechU16 m_slots[12]; // 0x06
	MechS8 m_unk0x1e;    // 0x1e
	MechS16 m_unk0x1f;   // 0x1f
} PackedMechSection;

// A weapon of a PackedMech.
// SIZE 0x03
typedef struct PackedMechWeapon {
	MechS16 m_type;    // 0x00
	MechS8 m_location; // 0x02
} PackedMechWeapon;

// An ammunition bin of a PackedMech.
// SIZE 0x04
typedef struct PackedMechAmmo {
	MechS16 m_unk0x00; // 0x00
	MechS16 m_type;    // 0x02
} PackedMechAmmo;

// A mech file in the fixed-size form the players send each other ("MI", FUN_1000ba4d).
// SIZE 0x1c1
typedef struct PackedMech {
	MechS8 m_mass;                   // 0x00
	MechS8 m_unk0x01;                // 0x01
	MechS8 m_unk0x02;                // 0x02
	MechS8 m_unk0x03;                // 0x03
	MechS8 m_weaponCount;            // 0x04
	MechS8 m_ammoCount;              // 0x05
	PackedMechSection m_sections[8]; // 0x06
	PackedMechWeapon m_weapons[14];  // 0x10e
	PackedMechAmmo m_ammo[25];       // 0x138
	MechChar m_name[0x1e];           // 0x19c
	MechS16 m_unk0x1ba;              // 0x1ba
	MechS8 m_unk0x1bc;               // 0x1bc
	MechS8 m_unk0x1bd;               // 0x1bd
	MechS8 m_unk0x1be;               // 0x1be
	MechS16 m_unk0x1bf;              // 0x1bf
} PackedMech;

#pragma pack()

// The functions and globals of unk10007c30.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern LRESULT g_unk0x10023388;
	extern MechS32 g_unk0x1002338c;
	extern MechS32 g_unk0x10023390;
	extern MechS32 g_unk0x10023394;

	MechS32 FUN_10007c30(MechTableEntry* p_table);
	void FUN_10007c5a(MechChar* p_mechFile, MechChar* p_text, MechS32 p_textSize);
	void FUN_100080e1(HWND p_hWnd, MechChar* p_code);
	MechS32 FUN_1000834d(MechChar* p_name, MechTableEntry* p_table);
	void FUN_100083e3(void* p_data, PackedMech* p_packed);
	void FUN_100086b1(PackedMech* p_packed, void* p_data, MechS32* p_size);

#ifdef __cplusplus
}
#endif

#endif // UNK10007C30_H
