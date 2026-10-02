#include "unk100089d0.h"

#include "chatlog.h"
#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000a650.h"
#include "unk1000aa90.h"
#include "unk1000d250.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The players list box of the guest's lobby dialog (FUN_10008d5d).
// GLOBAL: NETMECHW 0x1001ef84
HWND g_unk0x1001ef84;

// GLOBAL: NETMECHW 0x1001ef90
HWND g_unk0x1001ef90;

// GLOBAL: NETMECHW 0x1001ef98
HWND g_unk0x1001ef98;

// GLOBAL: NETMECHW 0x1001ef9c
HWND g_unk0x1001ef9c;

// GLOBAL: NETMECHW 0x1001efa0
HWND g_unk0x1001efa0;

// Shows the game options in the host's dialog p_dialog, if they changed since they were last
// shown or p_force is set: the mission's name, the players' names, the option check boxes and
// values.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100089d0
void FUN_100089d0(HWND p_dialog, MechS32 p_force)
{
	MechS32 unused0;
	MechS32 unused1;
	MechS32 unused2;
	CopperField0x4d::Options flags;
	MechChar code[4];
	MechChar name[12];
	MechChar* missionName;
	MechS32 length;
	MechS32 i;
	MechS32 count;
	NetPlayer player;
	MechS32 id;
	MechChar text[256];

	unused0 = 0;
	unused1 = 5;
	if (p_force || g_unk0x1001ca90.FUN_100035c0()) {
		EnterCriticalSection(&g_unk0x1001ee60);
		flags = g_unk0x1001ca90.m_settings.m_options.m_bits;

		if (g_unk0x1001ca90.m_settings.m_unk0x44[0]) {
			memcpy(code, g_unk0x1001ca90.m_settings.m_unk0x44, sizeof(code));
			sprintf(name, "%.4sNAME", code);
			missionName = LoadMissionText(name);
			length = strlen(missionName);
			missionName[length - 2] = '\0';
			SetDlgItemText(p_dialog, 0x41b, missionName);
			free(missionName);
		}

		count = i = 0;
		EnterCriticalSection(&g_unk0x1001ca78);
		while (i < 8) {
			if (FUN_1000afa8(i, &player)) {
				id = g_unk0x10023708[count++];
				SetDlgItemText(p_dialog, id, player.m_name);
			}

			i++;
		}

		LeaveCriticalSection(&g_unk0x1001ca78);
		while (count < 8) {
			id = g_unk0x10023708[count];
			SetDlgItemText(p_dialog, id, "");
			count++;
		}

		CheckDlgButton(p_dialog, 0x421, flags.m_option0);
		CheckDlgButton(p_dialog, 0x422, flags.m_option1);
		CheckDlgButton(p_dialog, 0x423, flags.m_option2);
		CheckDlgButton(p_dialog, 0x424, flags.m_option3);
		CheckDlgButton(p_dialog, 0x425, flags.m_option4);
		CheckDlgButton(p_dialog, 0x426, flags.m_option5);

		g_unk0x1001ef9c = GetDlgItem(p_dialog, 0x429);
		SetDlgItemText(p_dialog, 0x429, LoadResString(g_unk0x100234f8[g_unk0x1001ca90.m_settings.m_unk0x4c]));
		g_unk0x1001ef90 = GetDlgItem(p_dialog, 0x42a);
		SetDlgItemText(p_dialog, 0x42a, LoadResString(g_unk0x10023508[g_unk0x1001ca90.m_settings.m_unk0x4a]));
		g_unk0x1001ef98 = GetDlgItem(p_dialog, 0x433);
		SetDlgItemInt(p_dialog, 0x433, g_unk0x1001ca90.m_settings.m_unk0x49, FALSE);
		g_unk0x1001efa0 = GetDlgItem(p_dialog, 0x3fb);
		sprintf(text, "%4.2f", g_unk0x1001ca90.m_settings.m_unk0x4b / 16.0);
		SetDlgItemText(p_dialog, 0x3fb, text);

		g_unk0x1001ca90.FUN_10003620();
		LeaveCriticalSection(&g_unk0x1001ee60);
	}
}

// The string IDs of the values of m_settings.m_unk0x4c.
// GLOBAL: NETMECHW 0x100234f8
UINT g_unk0x100234f8[4] = {0x71, 0x72, 0x73, 0x74};

// The string IDs of the values of m_settings.m_unk0x4a.
// GLOBAL: NETMECHW 0x10023508
UINT g_unk0x10023508[3] = {0x6e, 0x6f, 0x70};

// The guest's lobby dialog: the game options the host set, and the chat. When the host is ready
// (message 0x416), it moves on to the player-slot pane of the game's kind.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes, and the operand
// order of the second | of pane tests (the original computes the test for 5 first), whose longer
// encoding also shifts the WM_HOTKEY jump table.
// FUNCTION: NETMECHW 0x10008d5d
BOOL CALLBACK FUN_10008d5d(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam)
{
	MechU32 notify;
	MechU32 command;

	switch (p_message) {
	case WM_INITDIALOG:
		g_unk0x1001ef84 = GetDlgItem(p_dialog, 0x42b);
		SendMessage(GetDlgItem(p_dialog, 0x7d6), WM_SETFONT, (WPARAM) g_unk0x10023168, 0);
		FUN_100089d0(p_dialog, TRUE);
		return TRUE;
	case WM_SHOWWINDOW:
		if (p_wParam) {
			FUN_10009183(p_dialog);
			g_chatLog.Attach(p_dialog);
			g_chatLog.Restore();
		}
		else {
			FUN_100091e3(p_dialog);
		}

		return FALSE;
	case 0x416:
		FUN_100089d0(p_dialog, FALSE);
		if (g_unk0x1001ca90.m_settings.m_unk0x00 & 1) {
			if (g_unk0x1001ca90.m_settings.m_unk0x00 & 2) {
				if ((g_unk0x1001ca90.m_unk0x15c == 4) | (g_unk0x1001ca90.m_unk0x15c == 0)) {
					FUN_10005023(5);
				}
				else {
					FUN_10005023(g_unk0x1001ca90.m_unk0x15c);
				}
			}
			else if ((g_unk0x1001ca90.m_unk0x15c == 5) | (g_unk0x1001ca90.m_unk0x15c == 0)) {
				FUN_10005023(4);
			}
			else {
				FUN_10005023(g_unk0x1001ca90.m_unk0x15c);
			}
		}

		return FALSE;
	case 0x417:
		FUN_10005023(1);
		break;
	case WM_HOTKEY:
		switch (p_wParam) {
		case 4:
			SetFocus(GetDlgItem(p_dialog, 0x3f2));
			break;
		case 5:
			SendMessage(p_dialog, WM_COMMAND, 0x3ec, 0);
			break;
		case 1:
			SendMessage(p_dialog, WM_COMMAND, 0x432, 0);
			break;
		case 2:
			SendMessage(p_dialog, WM_COMMAND, 0x7e0, 0);
			break;
		case 3:
			SendMessage(p_dialog, WM_COMMAND, 0x43b, 0);
			break;
		}

		break;
	case WM_COMMAND:
		command = LOWORD(p_wParam);
		notify = HIWORD(p_wParam);
		if (notify == EN_SETFOCUS) {
			if (GetDlgItem(p_dialog, 0x3f2) != (HWND) p_lParam) {
				SetFocus(NULL);
			}

			return FALSE;
		}

		switch (command) {
		case 0x3ec:
			g_chatLog.SendToAll(p_dialog);
			break;
		case IDOK:
			FUN_10005023(1);
			break;
		case 0x432:
			FUN_10005023(1);
			break;
		case 0x7e0:
			FUN_10005e50(g_unk0x1001cbf0, sizeof(g_unk0x1001cbf0));
			WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 5);
			break;
		case 0x43b:
			FUN_100042ab();
			break;
		case 0x3ee:
			if (notify == LBN_SELCHANGE) {
				SetDlgItemText(p_dialog, 0x3ec, LoadResString(0x76));
			}

			break;
		default:
			return TRUE;
			break;
		}

		return FALSE;
	case WM_DESTROY:
		return FALSE;
	}

	return FALSE;
}

// FUNCTION: NETMECHW 0x10009183
void FUN_10009183(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'C');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');
	RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x100091e3
void FUN_100091e3(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);
	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
