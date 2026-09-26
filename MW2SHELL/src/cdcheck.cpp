#include "decomp.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// GLOBAL: MW2SHELL 0x10066ddc
MechChar g_cdDriveLetter = '\0';

// GLOBAL: MW2SHELL 0x10066de0
MechS32 g_unk0x10066de0 = 0;

// FUNCTION: MW2SHELL 0x10030a20
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
	g_unk0x10066de0 = 0;
	while (*drive != '\0') {
		if (GetDriveType(drive) == DRIVE_CDROM) {
			g_unk0x10066de0++;
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
		g_unk0x10066de0 = 0;
	}

	free(driveStrings);
	return g_cdDriveLetter;
}

// FUNCTION: MW2SHELL 0x10030b5b
MechS32 FUN_10030b5b()
{
	if (g_unk0x10066de0 == 0) {
		CdCheck();
	}

	return g_unk0x10066de0;
}
