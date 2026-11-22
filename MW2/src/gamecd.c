#include "gamecd.h"

#include "simmain.h"
#include "types.h"

#include <stdio.h>
#include <windows.h>

// GLOBAL: MW2 0x100a00f8
// GLOBAL: MW2GBL 0x1009ef10
// GLOBAL: MW2MATROX 0x100a20f0
// GLOBAL: MW23DFX 0x1024a9b0
// GLOBAL: MW2ATI 0x1017ee08
// GLOBAL: MW2S3 0x10060fb0
// GLOBAL: MW2POWERVR 0x101a9b58
MechChar g_gameCdDrive = 0;

// GLOBAL: MW2 0x100a00fc
// GLOBAL: MW2GBL 0x1009ef14
// GLOBAL: MW2MATROX 0x100a20f4
// GLOBAL: MW23DFX 0x1024a9b4
// GLOBAL: MW2ATI 0x1017ee0c
// GLOBAL: MW2S3 0x10060fb4
// GLOBAL: MW2POWERVR 0x101a9b5c
MechS32 g_gameCdNumber = 0;

// Finds the CD-ROM drive holding the game disc (the one with \OLD_HERC.DRV) and caches its
// letter; g_gameCdNumber counts the CD-ROM drives up to it.
// FUNCTION: MW2 0x10002cb0
// FUNCTION: MW2GBL 0x1001a740
// FUNCTION: MW2MATROX 0x10003ca0
// FUNCTION: MW23DFX 0x10007b70
// FUNCTION: MW2ATI 0x10008450
// FUNCTION: MW2S3 0x10005280
// FUNCTION: MW2POWERVR 0x100071c0
MechChar FindGameCdDrive(void)
{
	LPSTR drive;
	HANDLE find;
	LPSTR drives;
	MechChar path[20];
	WIN32_FIND_DATA findData;

	if (g_gameCdDrive) {
		return g_gameCdDrive;
	}

	drives = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 0x69);
	GetLogicalDriveStrings(0x69, drives);
	sprintf(path, " :\\OLD_HERC.DRV");
	drive = drives;
	g_gameCdNumber = 0;
	while (*drive) {
		if (GetDriveType(drive) == DRIVE_CDROM) {
			g_gameCdNumber++;
			path[0] = *drive;
			find = FindFirstFile(path, &findData);
			if (find != INVALID_HANDLE_VALUE) {
				FindClose(find);
				break;
			}
		}

		drive += 4;
	}

	if (*drive) {
		g_gameCdDrive = *drive;
	}
	else {
		g_gameCdNumber = 0;
	}

	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, drives);
	return g_gameCdDrive;
}

// FUNCTION: MW2 0x10002df5
// FUNCTION: MW2GBL 0x1001a885
// FUNCTION: MW2MATROX 0x10003dc7
// FUNCTION: MW23DFX 0x10007c50
// FUNCTION: MW2ATI 0x10008530
// FUNCTION: MW2S3 0x10005360
// FUNCTION: MW2POWERVR 0x100072a0
MechS32 GetGameCdNumber(void)
{
	if (g_gameCdNumber == 0) {
		FindGameCdDrive();
	}

	return g_gameCdNumber;
}
