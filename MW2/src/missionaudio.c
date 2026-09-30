/* The mission's sound files: a table of the names in the mission's project file, hashed by
   FUN_100074e0, and the directory the files are read from. */
#include "missionaudio.h"

#include "decomp.h"
#include "error.h"
#include "mss.h"
#include "namehash.h"
#include "types.h"

#include <string.h>

// A project file entry: a sound file name, chained in its hash bucket.
// SIZE 0x14
typedef struct ProjectFileEntry {
	MechChar m_name[0x10];           // 0x00
	struct ProjectFileEntry* m_next; // 0x10
} ProjectFileEntry;

DECOMP_SIZE_ASSERT(ProjectFileEntry, 0x14)

// GLOBAL: MW2 0x100a14f4
MechS32 g_unk0x100a14f4 = 0;

// GLOBAL: MW2 0x100bcdb8
static ProjectFileEntry g_unk0x100bcdb8[200];

// GLOBAL: MW2 0x100bdd58
static ProjectFileEntry* g_unk0x100bdd58[0x65];

// The directory of the mission's sound files.
// GLOBAL: MW2 0x100bdef0
MechChar g_unk0x100bdef0[0x100];

// Returns the entry named p_name, or adds it if p_add; NULL when it is missing.
// Stack-slot permutation of entry, added and slot.
// FUNCTION: MW2 0x10007140
ProjectFileEntry* FUN_10007140(MechChar* p_name, MechS32 p_add)
{
	ProjectFileEntry* entry;
	ProjectFileEntry* added;
	MechU32 slot;

	slot = FUN_100074e0(p_name) % 0x65;
	for (entry = g_unk0x100bdd58[slot]; entry; entry = entry->m_next) {
		if (!_strcmpi(entry->m_name, p_name)) {
			return entry;
		}
	}

	if (!p_add) {
		return NULL;
	}

	entry = g_unk0x100bdd58[slot];
	if (g_unk0x100a14f4 >= 200) {
		Error(0x23, "Too many project file entries", 0);
	}

	added = &g_unk0x100bcdb8[g_unk0x100a14f4++];
	strcpy(added->m_name, p_name);
	added->m_next = g_unk0x100bdd58[slot];
	g_unk0x100bdd58[slot] = added;
	return added;
}

// STUB: MW2 0x10007252
void CollectMissionAudio(void)
{
	STUB(0x10007252);
}

// Reads the sound file p_name (".sfl" appended) from the mission's directory, if the project
// file lists it. Returns the data, or NULL.
// Stack-slot permutation of path and name.
// FUNCTION: MW2 0x100073bb
void* FUN_100073bb(MechChar* p_name)
{
	MechChar path[0x100];
	MechChar name[0x100];

	if (!g_unk0x100bdef0) {
		return NULL;
	}

	strcpy(name, p_name);
	strcat(name, ".sfl");
	if (!FUN_10007140(name, FALSE)) {
		return NULL;
	}

	strcpy(path, g_unk0x100bdef0);
	strcat(path, "\\");
	strcat(path, name);
	return FILE_read(path, NULL);
}
