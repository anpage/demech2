#include "cdcheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

MechChar g_cdDriveLetter;

// 1-based index of the game CD's drive among the CD-ROM drives, 0 if not found
MechS32 g_cdDriveNumber;

// Finds the CD-ROM drive holding the MechWarrior 2 CD (identified by OLD_HERC.DRV)
// and returns its drive letter, or 0 if there is none. The result is cached.
MechChar CdCheck(void)
{
	UINT driveType;
	MechChar* drive;
	WIN32_FIND_DATA findData;
	MechChar* driveStrings;
	HANDLE hFind;
	MechChar path[20];

	if (g_cdDriveLetter == '\0') {
		// 26 drives of "X:\" plus the list terminator
		driveStrings = calloc(0x69, 1);
		GetLogicalDriveStrings(0x69, driveStrings);
		sprintf(path, " :\\OLD_HERC.DRV");
		g_cdDriveNumber = 0;

		for (drive = driveStrings; *drive != '\0'; drive += 4) {
			driveType = GetDriveType(drive);
			if (driveType == DRIVE_CDROM) {
				g_cdDriveNumber++;
				path[0] = *drive;
				hFind = FindFirstFile(path, &findData);
				if (hFind != INVALID_HANDLE_VALUE) {
					FindClose(hFind);
					break;
				}
			}
		}

		if (*drive == '\0') {
			g_cdDriveNumber = 0;
		}
		else {
			g_cdDriveLetter = *drive;
		}

		free(driveStrings);
	}

	return g_cdDriveLetter;
}

MechS32 GetCdDriveNumber(void)
{
	if (g_cdDriveNumber == 0) {
		CdCheck();
	}

	return g_cdDriveNumber;
}
