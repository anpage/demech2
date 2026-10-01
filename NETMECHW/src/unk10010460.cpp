#include "unk10010460.h"

#include "decomp.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// The end-of-mission results dialog (it reads MW2CAR.CFG and shows who completed the mission),
// followed by the CD check the shell's cdcheck.cpp has as an object of its own. A C++ unit:
// FUN_10010c06 ends with the jmp to the epilogue.

// A record of the results, sorted by m_unk0x24 (FUN_10010c06).
// SIZE 0x26
struct AmberRecord0x26 {
	undefined m_unk0x00[0x24]; // 0x00
	MechU16 m_unk0x24;         // 0x24
};

// The block of MW2CAR.CFG that FUN_10010b7f reads.
// SIZE 0x86
struct SilverBlock0x86 {
	undefined m_unk0x00[0x86]; // 0x00
};

// MW2CAR.CFG.
// SIZE 0xd6
struct CareerFile0xd6 {
	undefined m_unk0x00[0x50]; // 0x00
	SilverBlock0x86 m_unk0x50; // 0x50
};

DECOMP_SIZE_ASSERT(AmberRecord0x26, 0x26)
DECOMP_SIZE_ASSERT(SilverBlock0x86, 0x86)
DECOMP_SIZE_ASSERT(CareerFile0xd6, 0xd6)

// GLOBAL: NETMECHW 0x10023930
MechChar g_cdDriveLetter = '\0';

// The game CD's drive among the CD-ROM drives, from 1; 0 if not found.
// GLOBAL: NETMECHW 0x10023934
MechS32 g_cdDriveNumber = 0;

// Returns 1. Called by FUN_100105ee when the dialog is dismissed with OK.
// FUNCTION: NETMECHW 0x10010460
MechS32 FUN_10010460()
{
	return 1;
}

// STUB: NETMECHW 0x10010475
void FUN_10010475(HWND)
{
	STUB(0x10010475);
}

// STUB: NETMECHW 0x100105ee
BOOL CALLBACK FUN_100105ee(HWND, UINT, WPARAM, LPARAM)
{
	STUB(0x100105ee);
	return FALSE;
}

// STUB: NETMECHW 0x10010920
undefined4 FUN_10010920(undefined4*)
{
	STUB(0x10010920);
	return 0;
}

// Reads the block at 0x50 of MW2CAR.CFG into p_block; returns whether the file opened.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10010b7f
MechS32 FUN_10010b7f(SilverBlock0x86* p_block)
{
	DWORD read;
	CareerFile0xd6 file;
	HANDLE handle;

	handle = CreateFile("MW2CAR.CFG", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	ReadFile(handle, &file, sizeof(file), &read, NULL);
	*p_block = file.m_unk0x50;
	CloseHandle(handle);
	return TRUE;
}

// Sorts the p_count records p_records by m_unk0x24, largest first.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10010c06
void FUN_10010c06(AmberRecord0x26* p_records, MechS32 p_count)
{
	MechS32 j;
	MechS32 i;
	AmberRecord0x26 swap;
	MechS32 best;

	for (i = 0; i < p_count; i++) {
		best = i;
		for (j = i + 1; j < p_count; j++) {
			if (p_records[best].m_unk0x24 < p_records[j].m_unk0x24) {
				best = j;
			}
		}

		swap = p_records[i];
		p_records[i] = p_records[best];
		p_records[best] = swap;
	}
}

// Stack-slot permutation (VC++ 2.2): drive, driveStrings, findData, hFindFile and path. The
// shorter recompiled stack displacements truncate the original's tail in reccmp.
// FUNCTION: NETMECHW 0x10010d00
char CdCheck()
{
	WIN32_FIND_DATA findData;
	MechChar path[20];
	LPSTR driveStrings;
	HANDLE hFindFile;
	LPCSTR drive;

	if (g_cdDriveLetter != '\0') {
		return g_cdDriveLetter;
	}

	driveStrings = (LPSTR) calloc(0x69, 1);
	GetLogicalDriveStrings(0x69, driveStrings);
	sprintf(path, " :\\OLD_HERC.DRV");

	drive = driveStrings;
	g_cdDriveNumber = 0;
	while (*drive != '\0') {
		if (GetDriveType(drive) == DRIVE_CDROM) {
			g_cdDriveNumber++;
			path[0] = *drive;
			hFindFile = FindFirstFile(path, &findData);
			if (hFindFile != INVALID_HANDLE_VALUE) {
				FindClose(hFindFile);
				break;
			}
		}
		drive += 4;
	}

	if (*drive != '\0') {
		g_cdDriveLetter = *drive;
	}
	else {
		g_cdDriveNumber = 0;
	}

	free(driveStrings);
	return g_cdDriveLetter;
}

// Unused.
// FUNCTION: NETMECHW 0x10010e1d
MechS32 GetCdDriveNumber()
{
	if (g_cdDriveNumber == 0) {
		CdCheck();
	}

	return g_cdDriveNumber;
}
