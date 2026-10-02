#include "unk10007c30.h"

#include "bwdwriter.h"
#include "decomp.h"
#include "mw2prj.h"
#include "prjfile.h"
#include "resourcecache.h"
#include "resourcefile.h"
#include "resourcename.h"
#include "types.h"
#include "weapondata.h"

#include <io.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The selected mech in g_unk0x1001c318 (FUN_10002e0e).
// GLOBAL: NETMECHW 0x10023388
LRESULT g_unk0x10023388 = 0;

// GLOBAL: NETMECHW 0x1002338c
MechS32 g_unk0x1002338c = 0;

// GLOBAL: NETMECHW 0x10023390
MechS32 g_unk0x10023390 = 0;

// GLOBAL: NETMECHW 0x10023394
MechS32 g_unk0x10023394 = 0;

// The names of a mech's sections, for its weapons' locations (FUN_10007c5a).
// GLOBAL: NETMECHW 0x100233b8
MechChar* g_sectionNames[8] = {"HD", "RT", "CT", "LT", "RA", "LA", "RL", "LL"};

// An "MTAB" resource: the count of entries, then the entries.
typedef struct MechTable {
	MechS32 m_count;             // 0x00
	MechTableEntry m_entries[1]; // 0x04
} MechTable;

// Loads the mech table of the chassis "MECH" into p_table; returns its entry count.
// FUNCTION: NETMECHW 0x10007c30
MechS32 FUN_10007c30(MechTableEntry* p_table)
{
	MechS32 count;

	count = FUN_1000834d("MECH", p_table);
	return count;
}

// Writes the description of the mech p_mechFile to p_text: its mass, heat sinks and jump jets,
// then each weapon with its location and ammunition. A player's copy (<xxx><nn>PLR) is read from
// the MEK directory, any other mech from mw2.prj. Sets g_unk0x10023390 to the mech's mass.
// p_textSize, the size of p_text, goes unused.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes, the operand order of
// the ammunition comparison (the original loads ammo[j].m_type) and of the slot index (it scales
// section first), and the reads of g_resourceTypeTags[c_resTagMek], which has no symbol in the
// original (mw2prj.c).
// FUNCTION: NETMECHW 0x10007c5a
void FUN_10007c5a(MechChar* p_mechFile, MechChar* p_text, MechS32 p_textSize)
{
	MechS32 fromFile;
	MechChar path[20];
	MechS32 handle;
	MechS32 size;
	MechFile* mech;
	MechS32 id;
	MechSection* sections;
	MechWeapon* weapons;
	MechAmmo* ammo;
	MechTrailer* trailer;
	MechS32 i;
	MechS32 section;
	MechS32 slot;
	MechS32 location;
	MechS32 ammoCount;
	MechS32 j;
	MechChar line[0x100];
	MechChar text[0x100];

	fromFile = FALSE;
	if (!_strnicmp(p_mechFile + 5, "PLR", 3)) {
		strcpy(path, ".\\mek\\");
		strcat(path, p_mechFile);
		strcat(path, ".MEK");
		handle = LoadFile(path, &size, (void**) &mech, 0);
		_close(handle);
		fromFile = TRUE;
	}
	else {
		id = FindResourceIdByName(6, p_mechFile);
		mech = (MechFile*) LoadCachedResource(g_mw2PrjHandle, id, g_resourceTypeTags[c_resTagMek], 0);
	}

	sections = mech->m_sections;
	weapons = (MechWeapon*) &sections[8];
	ammo = (MechAmmo*) &weapons[mech->m_weaponCount];
	trailer = (MechTrailer*) &ammo[mech->m_ammoCount];
	g_unk0x10023390 = mech->m_mass;

	*p_text = '\0';
	sprintf(
		p_text,
		"'MECH INFO:\r\n\tMass:\t\t\t%d T\r\n\tHeat sinks\t\t\t%d (%d)\r\n\tJump jets:\t\t\t%d\r\n",
		mech->m_mass,
		mech->m_heatSinks / 2,
		mech->m_heatSinks,
		mech->m_jumpJets
	);
	strcat(p_text, "\r\n\t\tWeapon\tLocation\tAmmo\r\n");

	for (i = 0; i < mech->m_weaponCount; i++) {
		for (section = 0; section < 8; section++) {
			for (slot = 0; slot < 12; slot++) {
				if (sections[section].m_slots[slot] == weapons[i].m_type) {
					location = section;
				}
			}
		}

		ammoCount = 0;
		for (j = 0; j < mech->m_ammoCount; j++) {
			if (weapons[i].m_type == ammo[j].m_type) {
				ammoCount++;
			}
		}

		sprintf(line, "\t\t%s\t%s", g_weaponDefs[weapons[i].m_type / 100].m_name, g_sectionNames[location]);
		if (ammoCount > 0) {
			sprintf(
				text,
				"\t%d T (%d)\r\n",
				ammoCount,
				g_weaponDefs[weapons[i].m_type / 100].m_volley * g_weaponDefs[weapons[i].m_type / 100].m_unk0x20 *
					ammoCount
			);
			strcat(line, text);
		}
		else {
			strcat(line, "\r\n");
		}

		strcat(p_text, line);
	}

	if (fromFile) {
		free(mech);
	}
	else {
		UnlockCachedResource(id, g_resourceTypeTags[c_resTagMek]);
	}
}

// Fills the variant list box p_hWnd with the variants of the chassis p_code: its standard ones
// from mw2.prj, then the player's own, MEK\<p_code><nn>USR.MEK, labelled with their names.
// Selects the current variant's first entry.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes, and the commutative
// order of i + g_unk0x1001fe74 (the original loads i first).
// FUNCTION: NETMECHW 0x100080e1
void FUN_100080e1(HWND p_hWnd, MechChar* p_code)
{
	LRESULT index;
	MechChar path[20];
	MechS32 i;
	MechS32 id;
	MechS32 size;
	MechU8* data;
	MechS32 failures;
	MechChar text[0x40];
	MechChar name[12];
	MechChar* label;
	MechS32 handle;

	index = 0;
	SendMessage(p_hWnd, LB_RESETCONTENT, 0, 0);
	for (i = 0; i < 100; i++) {
		g_unk0x1001fe74 = i;
		sprintf(path, "%3.3s%02dSTD", p_code, i);
		id = FindResourceIdByName(6, path);
		if (id == -1) {
			break;
		}

		if (i == 0) {
			sprintf(text, "Primary config");
		}
		else {
			sprintf(text, "Alt config %c", i + '@');
		}

		index = SendMessage(p_hWnd, LB_ADDSTRING, 0, (LPARAM) text);
		SendMessage(p_hWnd, LB_SETITEMDATA, index, i);
	}

	failures = 0;
	for (i = 0; i < 100; i++) {
		sprintf(name, "%3.3s%02dUSR", p_code, i);
		strcpy(path, ".\\mek\\");
		strcat(path, name);
		strcat(path, ".MEK");
		handle = LoadFile(path, &size, (void**) &data, 0);
		if (handle != -1) {
			label = (MechChar*) data + size - sizeof(MechTrailer);
			index = SendMessage(p_hWnd, LB_ADDSTRING, 0, (LPARAM) label);
			SendMessage(p_hWnd, LB_SETITEMDATA, index, i + g_unk0x1001fe74);
			_close(handle);
			free(data);
		}
		else {
			failures++;
			if (failures > 3) {
				break;
			}
		}
	}

	SendMessage(p_hWnd, LB_SETCURSEL, g_unk0x10023394 + g_unk0x1002338c ? g_unk0x1001fe74 : 0, 0);
}

// Loads the mech table p_name of mw2.prj into p_table; returns its entry count.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000834d
MechS32 FUN_1000834d(MechChar* p_name, MechTableEntry* p_table)
{
	MechTable* table;
	MechS32 id;
	MechS32 count;
	MechS32 size;

	id = FindResourceIdByName(c_resTagMpit, p_name);
	size = GetArchiveItemSize(g_mw2PrjHandle, "MTAB", id);
	table = (MechTable*) LoadCachedResource(g_mw2PrjHandle, id, "MTAB", 0);
	count = table->m_count;
	memcpy(p_table, table->m_entries, count * sizeof(MechTableEntry));
	UnlockCachedResource(id, "MTAB");
	return count;
}

// Packs the mech file p_data into p_packed, keeping at most 14 weapons and 25 ammunition bins in
// the counts (but copying all of them).
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100083e3
void FUN_100083e3(void* p_data, PackedMech* p_packed)
{
	MechWeapon* weapons;
	MechSection* sections;
	MechFile* mech;
	MechS32 i;
	MechS32 j;
	MechTrailer* trailer;
	MechAmmo* ammo;

	mech = (MechFile*) p_data;
	sections = mech->m_sections;
	weapons = (MechWeapon*) &sections[8];
	ammo = (MechAmmo*) &weapons[mech->m_weaponCount];
	trailer = (MechTrailer*) &ammo[mech->m_ammoCount];

	p_packed->m_mass = (MechS8) mech->m_mass;
	p_packed->m_unk0x01 = (MechS8) mech->m_unk0x04;
	p_packed->m_unk0x02 = (MechS8) mech->m_jumpJets;
	p_packed->m_unk0x03 = (MechS8) mech->m_heatSinks;
	p_packed->m_weaponCount = min(mech->m_weaponCount, 14);
	p_packed->m_ammoCount = min(mech->m_ammoCount, 25);

	for (i = 0; i < 8; i++) {
		p_packed->m_sections[i].m_unk0x00 = (MechS16) sections[i].m_unk0x00;
		p_packed->m_sections[i].m_unk0x02 = (MechS16) sections[i].m_unk0x04;
		p_packed->m_sections[i].m_unk0x04 = (MechS16) sections[i].m_unk0x08;
		p_packed->m_sections[i].m_unk0x1e = (MechS8) sections[i].m_unk0x24;
		p_packed->m_sections[i].m_unk0x1f = sections[i].m_unk0x26;
		for (j = 0; j < 12; j++) {
			p_packed->m_sections[i].m_slots[j] = sections[i].m_slots[j];
		}
	}

	for (i = 0; i < mech->m_weaponCount; i++) {
		p_packed->m_weapons[i].m_type = (MechS16) weapons[i].m_type;
		p_packed->m_weapons[i].m_location = (MechS8) weapons[i].m_location;
	}

	for (i = 0; i < mech->m_ammoCount; i++) {
		p_packed->m_ammo[i].m_unk0x00 = (MechS16) ammo[i].m_unk0x00;
		p_packed->m_ammo[i].m_type = (MechS16) ammo[i].m_type;
	}

	strcpy(p_packed->m_name, trailer->m_name);
	p_packed->m_unk0x1ba = (MechS16) trailer->m_unk0x1e;
	p_packed->m_unk0x1bc = (MechS8) trailer->m_unk0x22;
	p_packed->m_unk0x1bd = (MechS8) trailer->m_unk0x26;
	p_packed->m_unk0x1be = (MechS8) trailer->m_unk0x2a;
	p_packed->m_unk0x1bf = (MechS16) trailer->m_unk0x2e;
}

// Unpacks p_packed into the mech file p_data; returns its size in *p_size.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100086b1
void FUN_100086b1(PackedMech* p_packed, void* p_data, MechS32* p_size)
{
	MechWeapon* weapons;
	MechSection* sections;
	MechFile* mech;
	MechS32 i;
	MechS32 j;
	MechTrailer* trailer;
	MechAmmo* ammo;

	*p_size = 0;
	mech = (MechFile*) p_data;
	sections = mech->m_sections;
	weapons = (MechWeapon*) &sections[8];
	ammo = (MechAmmo*) &weapons[p_packed->m_weaponCount];
	trailer = (MechTrailer*) &ammo[p_packed->m_ammoCount];

	mech->m_mass = p_packed->m_mass;
	mech->m_unk0x04 = p_packed->m_unk0x01;
	mech->m_jumpJets = p_packed->m_unk0x02;
	mech->m_heatSinks = p_packed->m_unk0x03;
	mech->m_weaponCount = p_packed->m_weaponCount;
	mech->m_ammoCount = p_packed->m_ammoCount;
	*p_size += offsetof(MechFile, m_sections);

	for (i = 0; i < 8; i++) {
		sections[i].m_unk0x00 = p_packed->m_sections[i].m_unk0x00;
		sections[i].m_unk0x04 = p_packed->m_sections[i].m_unk0x02;
		sections[i].m_unk0x08 = p_packed->m_sections[i].m_unk0x04;
		sections[i].m_unk0x24 = p_packed->m_sections[i].m_unk0x1e;
		sections[i].m_unk0x26 = p_packed->m_sections[i].m_unk0x1f;
		for (j = 0; j < 12; j++) {
			sections[i].m_slots[j] = p_packed->m_sections[i].m_slots[j];
		}

		*p_size += sizeof(MechSection);
	}

	for (i = 0; i < mech->m_weaponCount; i++) {
		weapons[i].m_type = p_packed->m_weapons[i].m_type;
		weapons[i].m_location = p_packed->m_weapons[i].m_location;
		*p_size += sizeof(MechWeapon);
	}

	for (i = 0; i < mech->m_ammoCount; i++) {
		ammo[i].m_unk0x00 = p_packed->m_ammo[i].m_unk0x00;
		ammo[i].m_type = p_packed->m_ammo[i].m_type;
		*p_size += sizeof(MechAmmo);
	}

	memset(trailer->m_name, 0, sizeof(trailer->m_name));
	strcpy(trailer->m_name, p_packed->m_name);
	trailer->m_unk0x1e = p_packed->m_unk0x1ba;
	trailer->m_unk0x22 = p_packed->m_unk0x1bc;
	trailer->m_unk0x26 = p_packed->m_unk0x1bd;
	trailer->m_unk0x2a = p_packed->m_unk0x1be;
	trailer->m_unk0x2e = p_packed->m_unk0x1bf;
	*p_size += sizeof(MechTrailer);
}
