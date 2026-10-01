#include "unk100025f0.h"

#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000aa90.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The tab stops of the game options text (control 0x413).
// GLOBAL: NETMECHW 0x10023048
MechS32 g_optionTabStops[5] = {20, 40, 80, 120, 140};

// The mech file CopyPlayerMech last copied.
// GLOBAL: NETMECHW 0x10023060
MechChar g_lastMechFile[12] = "";

// FUN_10002ec3's format, which reccmp would otherwise pair with CopyPlayerMech's identical
// one at 0x100230e4.
// GLOBAL: NETMECHW 0x1002308c
MechChar g_unk0x1002308c[] = "%3.3s%02dPLR";

// Shows the game options, if they changed since they were last shown or p_force is set.
// FUNCTION: NETMECHW 0x100025f0
void FUN_100025f0(HWND p_dialog, MechS32 p_force)
{
	LPCSTR text;

	if (p_force || g_unk0x1001ca90.FUN_100035c0()) {
		SendMessage(GetDlgItem(p_dialog, 0x413), EM_SETTABSTOPS, 5, (LPARAM) g_optionTabStops);

		if (g_unk0x1001ca90.m_isHost) {
			text = LoadResString(0x77);
		}
		else {
			text = LoadResString(0x78);
		}

		SetDlgItemText(p_dialog, 0x432, text);
		g_unk0x1001ca90.FUN_10003620();
	}
}

// STUB: NETMECHW 0x1000268a
BOOL CALLBACK FUN_1000268a(HWND, UINT, WPARAM, LPARAM)
{
	STUB(0x1000268a);
	return FALSE;
}

// STUB: NETMECHW 0x10002e0e
void FUN_10002e0e(HWND)
{
	STUB(0x10002e0e);
}

// STUB: NETMECHW 0x10002ec3
void FUN_10002ec3(HWND)
{
	STUB(0x10002ec3);
}

// STUB: NETMECHW 0x10003154
void FUN_10003154(HWND)
{
	STUB(0x10003154);
}

// Copies the mech MEK\<p_mechFile>.MEK to the local player's MEK\<xxx><slot>PLR.MEK, deleting
// the slot's earlier copies, unless it is the mech copied last; then renames p_mechFile to the
// copy.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100031f8
void CopyPlayerMech(MechChar* p_mechFile)
{
	MechChar path[276];
	HANDLE find;
	WIN32_FIND_DATA findData;
	MechChar source[276];
	MechU32 index;
	MechS32 more;

	find = FindFirstFile("MEK", &findData);
	if (find == INVALID_HANDLE_VALUE) {
		CreateDirectory("MEK", NULL);
	}
	else {
		FindClose(find);
	}

	EnterCriticalSection(&g_unk0x1001ca78);
	index = g_unk0x1001ce18[FUN_1000b02b(g_unk0x1001ca90.m_playerId)];

	if (strcmp(g_lastMechFile, p_mechFile) || index == 0xff) {
		strcpy(g_lastMechFile, p_mechFile);
		index = g_unk0x1001ce18[FUN_1000b02b(g_unk0x1001ca90.m_playerId)] = (MechU8) (index + 1) % 100;

		sprintf(path, "MEK\\???%02dPLR.MEK", FUN_1000b02b(g_unk0x1001ca90.m_playerId));
		find = FindFirstFile(path, &findData);
		more = TRUE;
		while (more) {
			sprintf(path, "MEK\\%s", findData.cFileName);
			remove(path);
			more = FindNextFile(find, &findData);
		}

		FindClose(find);
		sprintf(path, "MEK\\%3.3s%02dPLR.MEK", p_mechFile, FUN_1000b02b(g_unk0x1001ca90.m_playerId));
		sprintf(source, "MEK\\%s.MEK", p_mechFile);
		CopyFile(source, path, FALSE);
		SetFileAttributes(path, FILE_ATTRIBUTE_NORMAL);
	}

	sprintf(p_mechFile, "%3.3s%02dPLR", p_mechFile, FUN_1000b02b(g_unk0x1001ca90.m_playerId));
	LeaveCriticalSection(&g_unk0x1001ca78);
}

// FUNCTION: NETMECHW 0x1000346c
void RegisterLobbyHotKeys(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 400, MOD_ALT, 'A');
	RegisterHotKey(p_hWnd, 401, MOD_ALT, 'V');
	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'C');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');

	if (g_unk0x1001ca90.m_settings.m_unk0x00 == 2) {
		RegisterHotKey(p_hWnd, 6, MOD_ALT, 'T');
	}

	if (g_unk0x1001ca90.m_isHost) {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	}
	else {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'L');
	}

	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x10003534
void UnregisterLobbyHotKeys(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 400);
	UnregisterHotKey(p_hWnd, 401);
	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);

	if (g_unk0x1001ca90.m_settings.m_unk0x00 == 2) {
		UnregisterHotKey(p_hWnd, 6);
	}

	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
