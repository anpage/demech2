/* The mission's sound files: a table of the names in the mission's project file, hashed by
   HashName, and the directory the files are read from. */
#include "missionaudio.h"

#include "decomp.h"
#include "error.h"
#include "gamecd.h"
#include "mss.h"
#include "namehash.h"
#include "types.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// A project file entry: a sound file name, chained in its hash bucket.
// SIZE 0x14
typedef struct ProjectFileEntry {
	MechChar m_name[0x10];           // 0x00
	struct ProjectFileEntry* m_next; // 0x10
} ProjectFileEntry;

DECOMP_SIZE_ASSERT(ProjectFileEntry, 0x14)

// GLOBAL: MW2 0x100a14f4
// GLOBAL: MW2MATROX 0x100a2030
// GLOBAL: MW23DFX 0x1024abe4
// GLOBAL: MW2ATI 0x1017f01c
// GLOBAL: MW2S3 0x100759ec
// GLOBAL: MW2POWERVR 0x101a9d6c
MechS32 g_soundFileCount = 0;

// GLOBAL: MW2 0x100bcdb8
// GLOBAL: MW2MATROX 0x100c0af8
// GLOBAL: MW2S3 0x1007dd98
static ProjectFileEntry g_soundFileEntries[200];

// GLOBAL: MW2 0x100bdd58
// GLOBAL: MW2MATROX 0x100c1a98
// GLOBAL: MW2S3 0x1007ed38
static ProjectFileEntry* g_soundFileTable[0x65];

// The directory of the mission's sound files.
// GLOBAL: MW2 0x100bdef0
// GLOBAL: MW2MATROX 0x100c09f8
// GLOBAL: MW23DFX 0x1007b078
// GLOBAL: MW2ATI 0x10070078
// GLOBAL: MW2S3 0x1007eed0
// GLOBAL: MW2POWERVR 0x1006e078
MechChar g_soundFileDir[0x100];

// Returns the entry named p_name, or adds it if p_add; NULL when it is missing.
// Matches but for the registers, which VC++ 2.2 allocates differently.
// FUNCTION: MW2 0x10007140
// FUNCTION: MW2MATROX 0x10001000
// FUNCTION: MW23DFX 0x100084e0
// FUNCTION: MW2ATI 0x10008cd0
// FUNCTION: MW2S3 0x1003ba10
// FUNCTION: MW2POWERVR 0x10007a40
ProjectFileEntry* FindSoundFile(MechChar* p_name, MechS32 p_add)
{
	ProjectFileEntry* entry;
	ProjectFileEntry* added;
	MechU32 slot;

	slot = HashName(p_name) % 0x65;
	for (entry = g_soundFileTable[slot]; entry; entry = entry->m_next) {
		if (!_strcmpi(entry->m_name, p_name)) {
			return entry;
		}
	}

	if (!p_add) {
		return NULL;
	}

	entry = g_soundFileTable[slot];
	if (g_soundFileCount >= 200) {
		Error(0x23, "Too many project file entries", 0);
	}

	added = &g_soundFileEntries[g_soundFileCount++];
	strcpy(added->m_name, p_name);
	added->m_next = g_soundFileTable[slot];
	g_soundFileTable[slot] = added;
	return added;
}

// Lists the mission's sound files (keating\*.sfl, on the game CD when they aren't installed) in
// the project file table and keeps their directory in g_soundFileDir.
// FUNCTION: MW2 0x10007252
// FUNCTION: MW2MATROX 0x10001112
// FUNCTION: MW23DFX 0x100085d0
// FUNCTION: MW2ATI 0x10008dc0
// FUNCTION: MW2S3 0x1003bb00
// FUNCTION: MW2POWERVR 0x10007b30
void CollectMissionAudio(void)
{
	MechChar drive;
	HANDLE find;
	WIN32_FIND_DATA data;

	drive = '\0';
	sprintf(g_soundFileDir, "%s\\*.sfl", "keating");
	find = FindFirstFile(g_soundFileDir, &data);
	if (find == INVALID_HANDLE_VALUE) {
		drive = FindGameCdDrive();
		if (!drive) {
			return;
		}

		sprintf(g_soundFileDir, "%c:\\%s\\*.sfl", drive, "keating");
		find = FindFirstFile(g_soundFileDir, &data);
		if (find == INVALID_HANDLE_VALUE) {
			return;
		}
	}

	for (;;) {
		if (!data.cAlternateFileName[0]) {
			FindSoundFile(data.cFileName, 1);
		}
		else {
			FindSoundFile(data.cAlternateFileName, 1);
		}

		if (!FindNextFile(find, &data)) {
			break;
		}
	}

	FindClose(find);
	if (drive) {
		sprintf(g_soundFileDir, "%c:\\%s", drive, "keating");
	}
	else {
		sprintf(g_soundFileDir, "%s", "keating");
	}
}

// Reads the sound file p_name (".sfl" appended) from the mission's directory, if the project
// file lists it. Returns the data, or NULL.
// FUNCTION: MW2 0x100073bb
// FUNCTION: MW2MATROX 0x10001257
// FUNCTION: MW23DFX 0x100086d0
// FUNCTION: MW2ATI 0x10008ec0
// FUNCTION: MW2S3 0x1003bc00
// FUNCTION: MW2POWERVR 0x10007c30
void* ReadSoundFile(MechChar* p_name)
{
	MechChar path[0x100];
	MechChar name[0x100];

	if (!g_soundFileDir) {
		return NULL;
	}

	strcpy(name, p_name);
	strcat(name, ".sfl");
	if (!FindSoundFile(name, FALSE)) {
		return NULL;
	}

	strcpy(path, g_soundFileDir);
	strcat(path, "\\");
	strcat(path, name);
	return FILE_read(path, NULL);
}
