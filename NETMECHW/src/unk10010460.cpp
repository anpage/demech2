#include "unk10010460.h"

#include "decomp.h"
#include "types.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000aa90.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The end-of-mission results dialog (it reads MW2CAR.CFG and shows who completed the mission),
// followed by the CD check the shell's cdcheck.cpp has as an object of its own. A C++ unit:
// FUN_10010c06 ends with the jmp to the epilogue.

#pragma pack(1)

// A player's line of the results: the player and their score, the kills of others minus their
// own deaths (FUN_10010920).
// SIZE 0x26
struct AmberRecord0x26 {
	NetPlayer m_player; // 0x00
	MechU16 m_score;    // 0x24
};

// The block of MW2CAR.CFG that FUN_10010b7f reads: the kills by player and victim, and the
// winner (-1 for nobody, -2 for a draw).
// SIZE 0x86
struct SilverBlock0x86 {
	undefined2 m_unk0x00;  // 0x00
	MechU16 m_kills[8][8]; // 0x02
	MechS32 m_winner;      // 0x82
};

#pragma pack()

// MW2CAR.CFG.
// SIZE 0xd6
struct CareerFile0xd6 {
	undefined m_unk0x00[0x50]; // 0x00
	SilverBlock0x86 m_unk0x50; // 0x50
};

DECOMP_SIZE_ASSERT(AmberRecord0x26, 0x26)
DECOMP_SIZE_ASSERT(SilverBlock0x86, 0x86)
DECOMP_SIZE_ASSERT(CareerFile0xd6, 0xd6)

MechS32 FUN_10010b7f(SilverBlock0x86* p_block);
void FUN_10010c06(AmberRecord0x26* p_records, MechS32 p_count);

// The results dialog's name and score controls, by line.
// GLOBAL: NETMECHW 0x10023858
MechS32 g_unk0x10023858[8] = {0x3f9, 0x3fc, 0x3fd, 0x40d, 0x3ff, 0x401, 0x403, 0x40e};

// GLOBAL: NETMECHW 0x10023878
MechS32 g_unk0x10023878[8] = {0x3fa, 0x3fe, 0x400, 0x40f, 0x402, 0x404, 0x405, 0x410};

// The winner's name, "" for a draw (FUN_10010920).
// GLOBAL: NETMECHW 0x1001f108
MechChar g_unk0x1001f108[0x50];

// The results dialog's picture (FUN_100105ee).
// GLOBAL: NETMECHW 0x1001f158
HBITMAP g_unk0x1001f158;

// The number of lines of the results.
// GLOBAL: NETMECHW 0x1001f160
MechS32 g_unk0x1001f160;

// The lines of the results, best score first.
// GLOBAL: NETMECHW 0x1001f168
AmberRecord0x26 g_unk0x1001f168[8];

// GLOBAL: NETMECHW 0x10023930
MechChar g_cdDriveLetter = '\0';

// The game CD's drive among the CD-ROM drives, from 1; 0 if not found.
// GLOBAL: NETMECHW 0x10023934
MechS32 g_cdDriveNumber = 0;

// Returns 1. Called by FUN_100105ee when the dialog is dismissed with OK.
// FUNCTION: NETMECHW 0x10010460
MechS32 FUN_10010460(HWND)
{
	return 1;
}

// Shows the results in the dialog p_dialog: each player's name and score, and the winner.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes, and the operand
// order of i < g_unk0x1001f160 (the original loads i).
// FUNCTION: NETMECHW 0x10010475
void FUN_10010475(HWND p_dialog)
{
	MechS32 i;
	MechChar score[10];
	MechChar text[0x50];

	i = 0;
	for (i = 0; i < g_unk0x1001f160; i++) {
		SetDlgItemText(p_dialog, g_unk0x10023858[i], g_unk0x1001f168[i].m_player.m_name);
		_snprintf(score, sizeof(score), "%d", (MechS16) g_unk0x1001f168[i].m_score);
		SetDlgItemText(p_dialog, g_unk0x10023878[i], score);
	}

	for (; i < 8; i++) {
		SetDlgItemText(p_dialog, g_unk0x10023858[i], "");
		SetDlgItemText(p_dialog, g_unk0x10023878[i], "");
	}

	if (strcmp(g_unk0x1001f108, "")) {
		sprintf(text, "%s completed the mission.", g_unk0x1001f108);
	}
	else {
		sprintf(text, "The Keshik has declared a draw.");
	}

	SetDlgItemText(p_dialog, 0x7d7, text);
}

// The results dialog, shown when the lobby starts after a mission (pane 9): the local player's
// line is in green. OK goes back to the lobby.
// The original tests notify for EN_SETFOCUS before it sets it.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100105ee
BOOL CALLBACK FUN_100105ee(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam)
{
	MechS32 i;
	HDC dc;
	HDC memoryDC;
	PAINTSTRUCT paint;
	BITMAP info;
	RECT rect;
	MechU32 command;
	MechU32 notify;

	switch (p_message) {
	case WM_INITDIALOG:
		g_unk0x1001f158 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xd6),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		FUN_10010475(p_dialog);
		return TRUE;
	case WM_CTLCOLORSTATIC:
		for (i = 0; i < 8; i++) {
			if (GetDlgItem(p_dialog, g_unk0x10023858[i]) == (HWND) p_lParam) {
				break;
			}
		}

		if (i == 8) {
			for (i = 0; i < 8; i++) {
				if (GetDlgItem(p_dialog, g_unk0x10023878[i]) == (HWND) p_lParam) {
					break;
				}
			}

			if (i == 8) {
				return FALSE;
			}
		}

		if (g_unk0x1001f168[i].m_player.m_id != g_unk0x1001ca90.m_playerId) {
			return FALSE;
		}

		SetTextColor((HDC) p_wParam, RGB(0, 0x80, 0));
		SetBkColor((HDC) p_wParam, GetSysColor(COLOR_BTNFACE));
		return (BOOL) GetSysColorBrush(COLOR_BTNFACE);
	case WM_PAINT:
		dc = BeginPaint(p_dialog, &paint);
		memoryDC = CreateCompatibleDC(dc);
		SelectObject(memoryDC, g_unk0x1001f158);
		if (g_unk0x10023118) {
			SelectPalette(dc, g_unk0x10023118, FALSE);
			RealizePalette(dc);
		}

		GetObject(g_unk0x1001f158, sizeof(info), &info);
		GetWindowRect(GetDlgItem(p_dialog, 0x7e4), &rect);
		ScreenToClient(p_dialog, (LPPOINT) &rect.left);
		ScreenToClient(p_dialog, (LPPOINT) &rect.right);
		StretchBlt(
			dc,
			rect.left,
			rect.top,
			rect.right - rect.left,
			rect.bottom - rect.top,
			memoryDC,
			0,
			0,
			info.bmWidth,
			info.bmHeight,
			SRCCOPY
		);
		DeleteDC(memoryDC);
		EndPaint(p_dialog, &paint);
		break;
	case WM_COMMAND:
		if (notify == EN_SETFOCUS) {
			SetFocus(NULL);
			return FALSE;
		}

		command = LOWORD(p_wParam);
		notify = HIWORD(p_wParam);
		switch (command) {
		case IDOK:
			if (FUN_10010460(p_dialog)) {
				FUN_10003cb7();
			}

			break;
		default:
			return TRUE;
			break;
		}

		return FALSE;
	case WM_DESTROY:
		DeleteObject(g_unk0x1001f158);
		return FALSE;
	}

	return FALSE;
}

// Reads the results of the last mission from MW2CAR.CFG for the players p_ids: their scores,
// sorted, and the winner's name in g_unk0x1001f108. Returns whether the file opened.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes, and the operand
// order of the loops' comparisons with g_unk0x1001f160 (the original loads the counter).
// FUNCTION: NETMECHW 0x10010920
MechS32 FUN_10010920(DPID* p_ids)
{
	SilverBlock0x86 block;
	MechS32 i;
	MechS32 j;

	if (!FUN_10010b7f(&block)) {
		return FALSE;
	}

	g_unk0x1001f160 = FUN_1000b0b3();
	for (i = 0; i < g_unk0x1001f160; i++) {
		FUN_1000aeff(p_ids[i], &g_unk0x1001f168[i].m_player);
		g_unk0x1001f168[i].m_score = 0;
		for (j = 0; j < g_unk0x1001f160; j++) {
			if (j == i) {
				g_unk0x1001f168[i].m_score -= block.m_kills[i][j];
			}
			else {
				g_unk0x1001f168[i].m_score += block.m_kills[i][j];
			}
		}
	}

	if (block.m_winner == -1) {
		strcpy(g_unk0x1001f108, "Nobody");
	}
	else if (block.m_winner == -2) {
		strcpy(g_unk0x1001f108, "");
	}
	else if (g_unk0x1001ca90.m_settings.m_unk0x00 & 2) {
		sprintf(
			g_unk0x1001f108,
			"\"%s\" (%s)",
			g_unk0x1001f168[block.m_winner].m_player.m_name,
			g_unk0x1001f168[block.m_winner].m_player.m_team == 1 ? "Clan Jade Falcon" : "Clan Wolf"
		);
	}
	else {
		sprintf(g_unk0x1001f108, "\"%s\"", g_unk0x1001f168[block.m_winner].m_player.m_name);
	}

	FUN_10010c06(g_unk0x1001f168, g_unk0x1001f160);
	return TRUE;
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
			if (p_records[best].m_score < p_records[j].m_score) {
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
