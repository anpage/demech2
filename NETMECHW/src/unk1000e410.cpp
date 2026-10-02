#include "unk1000e410.h"

#include "chatlog.h"
#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk10007c30.h"
#include "unk1000aa90.h"
#include "unk1000b400.h"
#include "unk1000d250.h"

#include <windows.h>

// The launch dialog's picture (FUN_1000e58d).
// GLOBAL: NETMECHW 0x1001f0f8
HBITMAP g_unk0x1001f0f8;

// The ready check boxes of the player slots of g_unk0x10023708.
// GLOBAL: NETMECHW 0x10023758
MechS32 g_unk0x10023758[8] = {0x7d1, 0x7d2, 0x7da, 0x7db, 0x7dc, 0x7dd, 0x7de, 0x7df};

// Shows the players' names and ready check boxes in the slots of the dialog p_dialog, if the
// options changed since they were last shown, p_force is set or the local player is the host.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000e410
void FUN_1000e410(HWND p_dialog, MechS32 p_force)
{
	MechS32 count;
	MechS32 unused0;
	MechS32 unused1;
	MechS32 i;
	MechS32 id;
	NetPlayer player;

	unused0 = 0;
	unused1 = 5;
	if (p_force || g_unk0x1001ca90.FUN_100035c0() || g_unk0x1001ca90.m_isHost) {
		EnterCriticalSection(&g_unk0x1001ee60);
		if (g_unk0x1001ca90.m_settings.m_unk0x44[0]) {
			count = i = 0;
			EnterCriticalSection(&g_unk0x1001ca78);
			while (i < 8) {
				if (FUN_1000afa8(i, &player)) {
					id = g_unk0x10023708[count];
					SetDlgItemText(p_dialog, id, player.m_name);

					if ((1 << i) & g_unk0x1001ca90.m_settings.m_unk0x02) {
						CheckDlgButton(p_dialog, g_unk0x10023758[count], 1);
					}
					else {
						CheckDlgButton(p_dialog, g_unk0x10023758[count], 0);
					}

					count++;
				}

				i++;
			}

			LeaveCriticalSection(&g_unk0x1001ca78);
			while (count < 8) {
				id = g_unk0x10023708[count];
				SetDlgItemText(p_dialog, id, "");
				CheckDlgButton(p_dialog, g_unk0x10023758[count], 0);
				count++;
			}
		}

		g_unk0x1001ca90.FUN_10003620();
		LeaveCriticalSection(&g_unk0x1001ee60);
	}
}

// The launch dialog (pane 8): the players and whether they are ready. "Ready" (0x435) tells the
// host the local player is, if the mission's tonnage allows the mech and another player is in;
// "Cancel" takes it back. In a team game the players' names are in their team's color.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes (which moves the
// targets of the jump tables).
// FUNCTION: NETMECHW 0x1000e58d
BOOL CALLBACK FUN_1000e58d(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam)
{
	MechS32 i;
	NetPlayer player;
	HDC dc;
	HDC memoryDC;
	PAINTSTRUCT paint;
	BITMAP info;
	RECT rect;
	MechU32 command;
	MechU32 notify;
	MechS32 ready;

	switch (p_message) {
	case WM_INITDIALOG:
		g_unk0x1001f0f8 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xd7),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		FUN_1000e410(p_dialog, TRUE);
		ShowWindow(p_dialog, SW_HIDE);
		return TRUE;
	case WM_SHOWWINDOW:
		if (p_wParam) {
			FUN_1000ef3d(p_dialog);
			g_chatLog.Attach(p_dialog);
			g_chatLog.Restore();
			SetDlgItemText(p_dialog, 0x432, g_unk0x1001ca90.m_isHost ? LoadResString(0x77) : LoadResString(0x78));
			FUN_1000e410(p_dialog, TRUE);
			EnterCriticalSection(&g_unk0x1001ee60);
			EnableWindow(GetDlgItem(p_dialog, 0x434), g_unk0x1001ca90.m_settings.m_unk0x00 & 2);
			EnableWindow(GetDlgItem(p_dialog, 0x438), g_unk0x1001ca90.m_settings.m_unk0x00 & 2);
			EnableWindow(GetDlgItem(p_dialog, 0x7d3), g_unk0x1001ca90.m_settings.m_unk0x00 & 2);
			LeaveCriticalSection(&g_unk0x1001ee60);
			g_unk0x1001ca90.m_unk0x15c = 8;
		}
		else {
			FUN_1000efcd(p_dialog);
		}

		return FALSE;
	case WM_CTLCOLORSTATIC:
		if (!(g_unk0x1001ca90.m_settings.m_unk0x00 & 2)) {
			return FALSE;
		}

		for (i = 0; i < 8; i++) {
			if (GetDlgItem(p_dialog, g_unk0x10023708[i]) == (HWND) p_lParam) {
				break;
			}
		}

		if (i == 8) {
			return FALSE;
		}

		FUN_1000afa8(i, &player);
		SetTextColor((HDC) p_wParam, player.m_team ? RGB(0, 0x80, 0) : RGB(0x80, 0, 0));
		SetBkColor((HDC) p_wParam, GetSysColor(COLOR_BTNFACE));
		return (BOOL) GetSysColorBrush(COLOR_BTNFACE);
	case WM_PAINT:
		dc = BeginPaint(p_dialog, &paint);
		memoryDC = CreateCompatibleDC(dc);
		SelectObject(memoryDC, g_unk0x1001f0f8);
		if (g_unk0x10023118) {
			SelectPalette(dc, g_unk0x10023118, FALSE);
			RealizePalette(dc);
		}

		GetObject(g_unk0x1001f0f8, sizeof(info), &info);
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
	case WM_DESTROY:
		DeleteObject(g_unk0x1001f0f8);
		g_chatLog.Detach();
		return FALSE;
	case WM_DRAWITEM:
		FUN_10004be3(p_wParam, p_lParam);
		return TRUE;
	case 0x416:
		if (!(g_unk0x1001ca90.m_settings.m_unk0x00 & 1)) {
			if (g_unk0x1001ca90.m_isHost) {
				FUN_10005023(2);
			}
			else {
				FUN_10005023(3);
			}

			MessageBox(g_unk0x10023138[g_unk0x10023108], LoadResString(0x7b), LoadResString(0x79), MB_OK);
		}
		else {
			FUN_1000e410(p_dialog, FALSE);
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
		case 6:
			SetFocus(GetDlgItem(p_dialog, 0x434));
			break;
		case 1:
			SendMessage(p_dialog, WM_COMMAND, 0x432, 0);
			break;
		case 7:
			SendMessage(p_dialog, WM_COMMAND, IDCANCEL, 0);
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
		case IDOK:
			break;
		case 0x432:
			if (g_unk0x1001ca90.m_isHost) {
				FUN_10005023(2);
			}
			else {
				FUN_10005023(1);
			}

			break;
		case 0x7e0:
			FUN_10005e50(g_unk0x1001cbf0, sizeof(g_unk0x1001cbf0));
			WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 10);
			break;
		case 0x43b:
			FUN_100042ab();
			break;
		case 0x3e9:
			if (g_unk0x1001ca90.m_settings.m_unk0x00 & 2) {
				FUN_10005023(5);
			}
			else {
				FUN_10005023(4);
			}

			break;
		case 0x3ea:
			FUN_10005023(7);
			break;
		case 0x3eb:
			FUN_10005023(6);
			break;
		case 0x431:
			break;
		case 0x3ec:
			if (IsDlgButtonChecked(p_dialog, 0x438)) {
				g_chatLog.SendToTeam(p_dialog);
			}
			else {
				g_chatLog.SendToAll(p_dialog);
			}

			break;
		case 0x3ee:
			if (notify == LBN_SELCHANGE) {
				SetDlgItemText(p_dialog, 0x3ec, LoadResString(0x76));
			}

			break;
		case 0x435:
			if (g_unk0x1001ca90.m_settings.m_unk0x49 < g_unk0x10023390) {
				MessageBox(g_unk0x10023138[g_unk0x10023108], LoadResString(0x84), LoadResString(0x79), MB_OK);
			}
			else if (FUN_1000b0b3() == 1) {
				MessageBox(g_unk0x10023138[g_unk0x10023108], LoadResString(0x85), LoadResString(0x79), MB_OK);
			}
			else if (g_unk0x1001ca90.m_isHost) {
				g_unk0x1001ca90.FUN_10003620();
				g_unk0x1001ca90.FUN_1000f050();
				FUN_1000b6cb(&g_unk0x1001ca90.m_savedSettings);
			}
			else {
				ready = TRUE;
				FUN_1000c08c(1, &ready, sizeof(ready), 0);
				FUN_1000c072();
			}

			break;
		case IDCANCEL:
			if (g_unk0x1001ca90.m_isHost) {
				g_unk0x1001ca90.FUN_10003620();
				g_unk0x1001ca90.FUN_1000f0a0();
				FUN_1000b6cb(&g_unk0x1001ca90.m_savedSettings);
			}
			else {
				ready = FALSE;
				FUN_1000c08c(1, &ready, sizeof(ready), 0);
				FUN_1000c072();
			}

			break;
		default:
			return TRUE;
			break;
		}

		break;
	}

	return FALSE;
}

// FUNCTION: NETMECHW 0x1000ef3d
void FUN_1000ef3d(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'U');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');

	if (g_unk0x1001ca90.m_settings.m_unk0x00 == 2) {
		RegisterHotKey(p_hWnd, 6, MOD_ALT, 'T');
	}

	RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	RegisterHotKey(p_hWnd, 7, MOD_ALT, 'A');
	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x1000efcd
void FUN_1000efcd(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);

	if (g_unk0x1001ca90.m_settings.m_unk0x00 == 2) {
		UnregisterHotKey(p_hWnd, 6);
	}

	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 7);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
