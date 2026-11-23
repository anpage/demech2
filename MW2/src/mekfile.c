#include "mekfile.h"

#include "config.h"
#include "decomp.h"
#include "error.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "loadres.h"
#include "mech.h"
#include "mechdamage.h"
#include "mechreload.h"
#include "mechsection.h"
#include "mw2log.h"
#include "mw2prj.h"
#include "players.h"
#include "simmain.h"
#include "soundfx.h"
#include "team.h"
#include "types.h"
#include "weapondata.h"
#include "weapons.h"
#include "weaponslot.h"

#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// A .MEK file's header. The file goes on with the eight sections (MechSection), the weapons
// (MekWeapon) and the ammunition bins (MekAmmo).
// SIZE 0x18
typedef struct MekHeader {
	MechS32 m_tons;        // 0x00
	MechS32 m_speed;       // 0x04
	MechS32 m_jump;        // 0x08 — 0: no jump jets, negative: none either
	MechS32 m_heatSinks;   // 0x0c
	MechS32 m_weaponCount; // 0x10
	MechS32 m_ammoCount;   // 0x14
} MekHeader;

// A .MEK file's weapon: its type and the part it fires from.
// SIZE 0x08
typedef struct MekWeapon {
	MechS32 m_type;      // 0x00 — a weapon type, times 100, and its id in MechSection::m_slots
	MechS32 m_hardpoint; // 0x04 — an index into Mech::m_objects; negative: the section holding it
} MekWeapon;

// A .MEK file's ammunition bin.
// SIZE 0x08
typedef struct MekAmmo {
	MechS32 m_id;   // 0x00 — its id in MechSection::m_slots
	MechS32 m_type; // 0x04 — the MekWeapon::m_type it feeds
} MekAmmo;

// The value each weapon type adds to a mech's (GetMechValue).
// GLOBAL: MW2 0x100aa730
// GLOBAL: MW23DFX 0x10259f60
// GLOBAL: MW2ATI 0x1018e258
// GLOBAL: MW2POWERVR 0x101b9540
MechU16 g_weaponValues[30] = {183, 137, 91,  46,  51,  34, 17,  74,  49, 25, 2,   228, 42, 82, 123,
							  157, 74,  144, 247, 329, 4,  228, 166, 51, 21, 177, 74,  16, 50, 40};

// Loads mech p_mech's configuration mek\<p_config>.mek (or MEK resource p_id when the file is
// missing) for chassis p_name, logging it to mw2.log: the sections, whose armor the difficulty
// scales (the local player's by g_localArmorPerLevel, its side's by armorScale, the others' by
// g_otherArmorPerLevel), the weapons and their ammunition bins, the heat sinks (scaled by the difficulty
// and the temperature, g_temperature) and the jump jets. Returns TRUE.
// Matches but for the registers, which VC++ 2.2 allocates differently.
// FUNCTION: MW2 0x1005d6d0
// FUNCTION: MW2GBL 0x10045e90
// FUNCTION: MW2MATROX 0x1008c850
// FUNCTION: MW23DFX 0x10041410
// FUNCTION: MW2ATI 0x1003efc0
// FUNCTION: MW2S3 0x10038b10
// FUNCTION: MW2POWERVR 0x10039d20
MechS32 LoadMechConfig(Mech* p_mech, MechChar* p_name, MechS32 p_id, MechChar* p_config)
{
	MechScalar armorScale;
	MechS32 binSlot;
	MechS32 group;
	MechS32 i;
	MechS32 fromResource;
	MechChar name[52];
	WeaponBin* bin;
	MechSection* mekSections;
	MechChar text[128];
	MekWeapon* weapon;
	MechS32 binCount;
	WeaponSlot* slot;
	MechScalar heat;
	MechScalar jumpRatio;
	MechS32 level;
	MekHeader* header;
	MechSection* section;
	MechS32 size;
	MekWeapon* weapons;
	MekAmmo* ammos;
	MekAmmo* ammo;
	MechS32 file;
	MechS32 type;
	MechS32 j;
	MechS32 piece;
#ifdef MW2_FLOAT_SCALARS
	MechS32* bits;
#endif

	fromResource = 0;
	RememberLoadMech(p_mech, p_name, p_id, p_config);
	switch (g_difficulty->m_enemySkill) {
	case 0:
		g_otherArmorPerLevel = 1;
		armorScale = 3;
		g_localArmorPerLevel = 4;
		break;
	case 1:
		g_otherArmorPerLevel = 3;
		armorScale = 3;
		g_localArmorPerLevel = 4;
		break;
	case 2:
		g_otherArmorPerLevel = 4;
		armorScale = 4;
		g_localArmorPerLevel = 4;
		break;
	}

	strcpy(name, "mek\\");
	strcat(name, p_config);
	if (!strchr(name, '.')) {
		strcat(name, g_resExtMek);
	}

	file = LoadFile(BuildGamePath(name), &size, (void**) &header, NULL);
	if (file == -1) {
		if (p_id > 0) {
			header = LoadCachedResource(g_mw2PrjHandle, p_id, g_resTagMek, 0);
			if (header == NULL) {
				Error(0x21, "%s ID %d", name, p_id, 0);
			}
			else {
				fromResource = 1;
			}
		}
		else {
			Error(0x21, "%s ID %d", name, p_id, 0);
		}
	}
	else {
		_close(file);
	}

	p_mech->m_flags = 0;
	if (header->m_weaponCount > WEAPON_SLOT_COUNT) {
		header->m_weaponCount = WEAPON_SLOT_COUNT;
	}

	sprintf(text, "\n\n chassis: %s  config: %s  ", p_name, p_config);
	WriteToMw2Log(text);
	sprintf(text, "\n  tons wt: %d ", header->m_tons);
	WriteToMw2Log(text);
	sprintf(text, "\n  move: %d  jump: %d  heat: %d  ", header->m_speed, header->m_jump, header->m_heatSinks);
	WriteToMw2Log(text);
	mekSections = (MechSection*) (header + 1);
	weapons = (MekWeapon*) (mekSections + 8);
	ammos = (MekAmmo*) (weapons + header->m_weaponCount);
	p_mech->m_player->m_ai.m_value = GetMechValue(header, mekSections, weapons);
	memcpy(p_mech->m_sections, mekSections, 8 * sizeof(MechSection));

	section = p_mech->m_sections;
	for (piece = 0; piece < 8; piece++) {
		sprintf(
			text,
#ifdef MW2_MEK_LOG_FLOATS
			"\n\npiece %d- armor: %ff %fr int: %f flags:%d\n",
#else
			"\n\npiece %d- armor: %df %dr int: %d flags:%d\n",
#endif
			piece,
			section->m_armor[0],
			section->m_armor[1],
			section->m_internal,
			section->m_flags
		);
		WriteToMw2Log(text);
		for (j = 0; j < 12; j++) {
			if (section->m_slots[j] >= 5000 && section->m_slots[j] < 5050) {
				p_mech->m_flags |= 0x10;
			}

			sprintf(text, "%3d ", section->m_slots[j]);
			WriteToMw2Log(text);
		}

#ifdef MW2_FLOAT_SCALARS
		// The .MEK file holds integers, which the Matrox edition converts to floats in place.
		bits = (MechS32*) &section->m_armor[0];
		section->m_armor[0] = *bits;
		bits = (MechS32*) &section->m_armor[1];
		section->m_armor[1] = *bits;
		bits = (MechS32*) &section->m_internal;
		section->m_internal = *bits;
#endif
		section->m_flags &= 0xff00;
		level = (section->m_armor[0] + section->m_internal) / 5;
		if (level > 15) {
			level = 15;
		}
		else if (level < 1) {
			level = 0;
		}

		section->m_flags |= level;
		level = (section->m_armor[1] + section->m_internal) / 5;
		if (level > 15) {
			level = 15;
		}
		else if (level < 1) {
			level = 0;
		}

		section->m_flags |= level << 4;
		if (p_mech->m_player->m_index == g_localPlayerId) {
			section->m_armor[0] *= g_localArmorPerLevel;
			section->m_armor[1] *= g_localArmorPerLevel;
		}
		else if (!GetPlayerSide(p_mech->m_player->m_index)) {
			section->m_armor[0] *= armorScale;
			section->m_armor[1] *= armorScale;
		}
		else {
			section->m_armor[0] *= g_otherArmorPerLevel;
			section->m_armor[1] *= g_otherArmorPerLevel;
		}

#ifndef MW2_FLOAT_SCALARS
		section->m_armor[0] <<= 16;
		section->m_armor[1] <<= 16;
		section->m_internal <<= 16;
#endif
		section++;
	}

	slot = p_mech->m_weapons;
	bin = p_mech->m_ammoBins;
	piece = j = 0;
	binCount = 0;
	i = 0;
	group = 0;
	weapon = weapons;
	if (header->m_weaponCount > 0) {
		do {
			type = weapon->m_type / 100;
			slot->m_status = 1;
			slot->m_type = type;
			slot->m_state = c_weaponReady;
			slot->m_time = 0;
			slot->m_unk0x14 = 0;
			slot->m_group = group;
			slot->m_slotId = weapon->m_type;
			slot->m_binCount = 0;
			slot->m_binIndex = 0;
			slot->m_bin = bin;
			if (g_weaponDefs[type].m_volleysPerBin == -1) {
				slot->m_ammo = -1;
			}
			else {
				slot->m_ammo = 0;
			}

			ammo = ammos;
			binSlot = 0;
			for (j = 0; j < header->m_ammoCount && binCount < 25; j++) {
				if (ammo->m_type == weapon->m_type) {
					bin->m_type = ammo->m_type / 100;
					bin->m_shots = g_weaponDefs[bin->m_type].m_volley * g_weaponDefs[bin->m_type].m_volleysPerBin;
					bin->m_weapon = i;
					bin->m_id = ammo->m_id;
					bin->m_damage = g_weaponDefs[bin->m_type].m_damage;
					bin->m_heat = g_weaponDefs[bin->m_type].m_heat;
					bin->m_shotHeat = g_weaponDefs[bin->m_type].m_shotHeat;
					slot->m_ammo += bin->m_shots;
					slot->m_bins[binSlot] = binCount;
					binSlot++;
					slot->m_binCount++;
					bin++;
					binCount++;
				}

				ammo++;
			}

			section = mekSections;
			if (weapon->m_hardpoint > -1) {
				slot->m_hardpoint = weapon->m_hardpoint;
			}
			else {
				for (piece = 0; piece < 8; piece++) {
					for (j = 0; j < 12; j++) {
						if (section->m_slots[j] == weapon->m_type) {
							if (piece == 6) {
								slot->m_hardpoint = 1;
							}
							else if (piece == 7) {
								slot->m_hardpoint = 3;
							}
							else {
								slot->m_hardpoint = piece;
							}
						}
					}

					section++;
				}
			}

			sprintf(text, "\nweapon %d- type %d  ammo: %ld  ", i, slot->m_type, slot->m_ammo);
			WriteToMw2Log(text);
			slot++;
			weapon++;
			i++;
		} while (i < header->m_weaponCount && i < WEAPON_SLOT_COUNT);
	}

	ammo = ammos;
	for (piece = 0; piece < header->m_ammoCount; piece++) {
		sprintf(text, "\nammo %d - class %d wpn %d ", piece, ammo->m_id, ammo->m_type);
		WriteToMw2Log(text);
		ammo++;
	}

#ifdef MW2_FLOAT_SCALARS
	p_mech->m_topSpeed = header->m_speed * FIXED_RAW(0x697e98);
#else
	p_mech->m_topSpeed = FixedMul16(header->m_speed, 0x697e98);
#endif
	p_mech->m_tons = header->m_tons;
	p_mech->m_weaponCount = header->m_weaponCount;
	p_mech->m_ammoBinCount = header->m_ammoCount;
#ifdef MW2_DEEP_COLD_COOLING
	heat = FIXED_RAW(50);
	if (p_mech->m_player->m_index == g_localPlayerId) {
		switch (g_difficulty->m_enemySkill) {
		case 0:
			heat *= FIXED_LITERAL(1.4, 1.4f);
			if (g_temperature <= -100) {
				heat *= 5;
			}

			if (g_temperature < -30) {
				heat *= 2;
			}
			else if (g_temperature > 50) {
			}
			break;
		case 1:
			if (g_temperature <= -100) {
				heat *= 5;
			}

			if (g_temperature < -30) {
				heat *= 2;
			}
			else if (g_temperature > 50) {
				heat *= FIXED_LITERAL(0.9, 0.9f);
			}
			break;
		case 2:
			heat *= FIXED_LITERAL(0.9, 0.9f);
			if (g_temperature <= -100) {
				heat *= 5;
			}

			if (g_temperature < -30) {
				heat *= FIXED_LITERAL(1.5, 1.5f);
			}
			else if (g_temperature > 50) {
				heat *= FIXED_LITERAL(0.8, 0.8f);
			}
			break;
		}
	}
	else {
		if (g_temperature <= -100) {
			heat *= 5;
		}

		if (g_temperature < -30) {
			heat *= 2;
		}
		else if (g_temperature > 50) {
			heat *= FIXED_LITERAL(0.9, 0.9f);
		}
	}

#else
	heat = FIXED_RAW(50);
	if (p_mech->m_player->m_index == g_localPlayerId) {
		switch (g_difficulty->m_enemySkill) {
		case 0:
			heat *= FIXED_LITERAL(1.4, 1.4f);
			if (g_temperature < -30) {
				heat *= 2;
			}
#ifndef MW2_ACCEL
			else if (g_temperature > 50) {
			}
#endif
			break;
		case 1:
			if (g_temperature < -30) {
				heat *= 2;
			}
			else if (g_temperature > 50) {
				heat *= FIXED_LITERAL(0.9, 0.9f);
			}
			break;
		case 2:
			heat *= FIXED_LITERAL(0.9, 0.9f);
			if (g_temperature < -30) {
				heat *= FIXED_LITERAL(1.5, 1.5f);
			}
			else if (g_temperature > 50) {
				heat *= FIXED_LITERAL(0.8, 0.8f);
			}
			break;
		}
	}
	else if (g_temperature < -30) {
		heat *= 2;
	}
	else if (g_temperature > 50) {
		heat *= FIXED_LITERAL(0.9, 0.9f);
	}

#endif
	p_mech->m_cooling = header->m_heatSinks * heat;
	if (header->m_jump == 0) {
		p_mech->m_jumpFuel = -2;
		p_mech->m_jumpJets = 0;
		p_mech->m_jumpThrust = 0;
	}
	else if (header->m_jump < 0) {
		p_mech->m_jumpFuel = -1;
		p_mech->m_jumpJets = 0;
		p_mech->m_jumpThrust = 0;
	}
	else {
		p_mech->m_jumpFuel = 0x712;
		p_mech->m_jumpJets = header->m_jump;
#ifdef MW2_FLOAT_SCALARS
		// The accelerated editions' constant is 0.1184, where 1.1's 0x1e50 is 0.118408.
		jumpRatio = (MechScalar) header->m_jump / header->m_speed;
		p_mech->m_jumpThrust = jumpRatio * 0.1184f;
#else
		jumpRatio = FixedDiv16(header->m_jump, header->m_speed);
		p_mech->m_jumpThrust = FixedMul16(0x1e50, jumpRatio);
#endif
	}

#ifdef MW2_MEK_LOG_FLOATS
	sprintf(text, "\njet ddy: %f", p_mech->m_jumpThrust);
#else
	sprintf(text, "\njet ddy: %ld", p_mech->m_jumpThrust);
#endif
	WriteToMw2Log(text);
	if (fromResource) {
		UnlockCachedResource(p_id, g_resTagMek);
	}
	else {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, header);
	}

	return TRUE;
}

// Returns a mech's value: its tonnage, scaled by the critical slots it fills (by kind), plus its
// armor and the value of each of its weapons (up to ten).
// FUNCTION: MW2 0x1005e534
// FUNCTION: MW2GBL 0x10046d28
// FUNCTION: MW2MATROX 0x1008d62b
// FUNCTION: MW23DFX 0x10041c80
// FUNCTION: MW2ATI 0x1003f830
// FUNCTION: MW2S3 0x10039370
// FUNCTION: MW2POWERVR 0x1003a590
MechU16 GetMechValue(MekHeader* p_header, MechSection* p_sections, MekWeapon* p_weapons)
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
		for (j = 0; j < p_sections->m_slotCount; j++) {
			armor += SCALAR_TO_INT(p_sections->m_armor[0] + p_sections->m_armor[1]);
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
		value += (MechU16) p_sections->m_internal;
	}

	for (i = 0; i < p_header->m_weaponCount && i < WEAPON_SLOT_COUNT; i++, p_weapons++) {
		value += g_weaponValues[p_weapons->m_type / 100];
	}

	return value;
}
