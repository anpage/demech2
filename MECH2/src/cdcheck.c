#include "cdcheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// GLOBAL: MECH2 0x0040d798
MechChar g_cdDriveLetter = '\0';

// 1-based index of the game CD's drive among the CD-ROM drives, 0 if not found
// GLOBAL: MECH2 0x0040d79c
MechS32 g_cdDriveNumber = 0;

// Finds the CD-ROM drive holding the MechWarrior 2 CD (identified by OLD_HERC.DRV)
// and returns its drive letter, or 0 if there is none. The result is cached.
// Stack-slot permutation: drive, driveStrings, find, path and findData. The shorter
// recompiled stack displacements truncate the original's tail in reccmp.
// FUNCTION: MECH2 0x00401ce0
MechChar CdCheck(void)
{
	MechChar* drive;
	WIN32_FIND_DATA findData;
	MechChar* driveStrings;
	HANDLE find;
	MechChar path[20];

	if (g_cdDriveLetter != '\0') {
		return g_cdDriveLetter;
	}

	// 26 drives of "X:\" plus the list terminator
	driveStrings = calloc(0x69, 1);
	GetLogicalDriveStrings(0x69, driveStrings);
	sprintf(path, " :\\OLD_HERC.DRV");
	drive = driveStrings;
	g_cdDriveNumber = 0;

	while (*drive != '\0') {
		if (GetDriveType(drive) == DRIVE_CDROM) {
			g_cdDriveNumber++;
			path[0] = *drive;
			find = FindFirstFile(path, &findData);
			if (find != INVALID_HANDLE_VALUE) {
				FindClose(find);
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

// FUNCTION: MECH2 0x00401dfd
MechS32 GetCdDriveNumber(void)
{
	if (g_cdDriveNumber == 0) {
		CdCheck();
	}

	return g_cdDriveNumber;
}
