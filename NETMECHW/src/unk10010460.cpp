#include "unk10010460.h"

#include "decomp.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// The end-of-mission results dialog (it reads MW2CAR.CFG and shows who completed the mission),
// followed by the CD check the shell's cdcheck.cpp has as an object of its own. A C++ unit:
// FUN_10010c06 ends with the jmp to the epilogue.

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

// STUB: NETMECHW 0x10010b7f
undefined4 FUN_10010b7f(undefined4*)
{
	STUB(0x10010b7f);
	return 0;
}

// STUB: NETMECHW 0x10010c06
void FUN_10010c06(undefined4*, MechS32)
{
	STUB(0x10010c06);
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
