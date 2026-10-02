#include "unk1000d250.h"

#include "chatlog.h"
#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000aa90.h"
#include "unk1000b400.h"

#include <windows.h>

// The pictures of the free-for-all and team slot dialogs (FUN_1000d31f, FUN_1000dae1).
// GLOBAL: NETMECHW 0x1001f0f0
HBITMAP g_unk0x1001f0f0;

// GLOBAL: NETMECHW 0x1001f0f4
HBITMAP g_unk0x1001f0f4;

// The controls of the player slots in the lobby's dialogs: eight for a free-for-all or team 0,
// then eight for team 1.
// GLOBAL: NETMECHW 0x10023708
MechS32 g_unk0x10023708[16] =
	{0x3f9, 0x3fc, 0x3fd, 0x40d, 0x3ff, 0x401, 0x403, 0x40e, 0x405, 0x407, 0x409, 0x410, 0x40a, 0x40b, 0x40c, 0x412};

// Shows the players' names in the slots of the dialog p_dialog, if the options changed since they
// were last shown or p_force is set.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000d250
void FUN_1000d250(HWND p_dialog, MechS32 p_force)
{
	MechS32 unused;
	MechS32 count;
	MechU8 i;
	MechS32 id;
	NetPlayer player;

	count = 0;
	unused = 0;
	i = 0;
	if (g_unk0x1001ca90.FUN_100035c0() || p_force) {
		while (i < 8) {
			if (FUN_1000afa8(i, &player)) {
				id = g_unk0x10023708[count++];
				SetDlgItemText(p_dialog, id, player.m_name);
			}

			i++;
		}

		while (count < 8) {
			id = g_unk0x10023708[count];
			SetDlgItemText(p_dialog, id, "");
			count++;
		}
	}

	g_unk0x1001ca90.FUN_10003620();
}

// The free-for-all slot dialog (pane 4): the players in their slots, and the chat.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000d31f
BOOL CALLBACK FUN_1000d31f(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam)
{
	MechU32 command;
	MechU32 notify;
	HDC dc;
	HDC memoryDC;
	PAINTSTRUCT paint;
	BITMAP info;
	RECT rect;

	command = LOWORD(p_wParam);
	notify = HIWORD(p_wParam);

	switch (p_message) {
	case WM_INITDIALOG:
		g_unk0x1001f0f0 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xd5),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		FUN_1000d250(p_dialog, TRUE);
		CheckDlgButton(p_dialog, 0x434, BST_CHECKED);
		return TRUE;
	case WM_SHOWWINDOW:
		if (p_wParam) {
			FUN_1000e2e2(p_dialog);
			g_chatLog.Attach(p_dialog);
			g_chatLog.Restore();
			SetDlgItemText(p_dialog, 0x432, g_unk0x1001ca90.m_isHost ? LoadResString(0x77) : LoadResString(0x78));
			FUN_1000d250(p_dialog, TRUE);
			g_unk0x1001ca90.m_unk0x15c = 4;
		}
		else {
			FUN_1000e396(p_dialog);
		}

		return FALSE;
	case WM_DRAWITEM:
		FUN_10004be3(p_wParam, p_lParam);
		return TRUE;
	case WM_PAINT:
		dc = BeginPaint(p_dialog, &paint);
		memoryDC = CreateCompatibleDC(dc);
		SelectObject(memoryDC, g_unk0x1001f0f0);
		if (g_unk0x10023118) {
			SelectPalette(dc, g_unk0x10023118, FALSE);
			RealizePalette(dc);
		}

		GetObject(g_unk0x1001f0f0, sizeof(info), &info);
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
			FUN_1000d250(p_dialog, FALSE);
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
		if (notify == EN_SETFOCUS) {
			if (GetDlgItem(p_dialog, 0x3f2) != (HWND) p_lParam) {
				SetFocus(NULL);
			}

			return FALSE;
		}

		switch (command) {
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
			WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 6);
			break;
		case 0x43b:
			FUN_100042ab();
			break;
		case 0x3e9:
			break;
		case 0x3ea:
			FUN_10005023(7);
			break;
		case 0x3eb:
			FUN_10005023(6);
			break;
		case 0x431:
			FUN_10005023(8);
			break;
		case 0x3ec:
			g_chatLog.SendToAll(p_dialog);
			break;
		case 0x3ee:
			if (notify == LBN_SELCHANGE) {
				SetDlgItemText(p_dialog, 0x3ec, LoadResString(0x76));
			}

			break;
		default:
			return TRUE;
		}

		break;
	case WM_DESTROY:
		DeleteObject(g_unk0x1001f0f0);
		return FALSE;
	}

	return FALSE;
}

// Shows the players' names in the slots of the dialog p_dialog by team, if the options changed
// since they were last shown or p_force is set, and disables control 0x42c when the local
// player is alone on their team.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000d91f
void FUN_1000d91f(HWND p_dialog, MechS32 p_force)
{
	MechS32 count1;
	MechS32 count0;
	MechU8 i;
	MechS32 id;
	NetPlayer player;

	count0 = 0;
	count1 = 0;
	i = 0;
	if (g_unk0x1001ca90.FUN_100035c0() || p_force) {
		EnterCriticalSection(&g_unk0x1001ca78);
		while (i < 8) {
			if (FUN_1000afa8(i, &player)) {
				if (player.m_team == 1) {
					id = g_unk0x10023708[8 + count1++];
				}
				else {
					id = g_unk0x10023708[count0++];
				}

				SetDlgItemText(p_dialog, id, player.m_name);
			}

			i++;
		}

		LeaveCriticalSection(&g_unk0x1001ca78);
		FUN_1000aeff(g_unk0x1001ca90.m_playerId, &player);

		if (player.m_team == 0 && count0 == 1) {
			EnableWindow(GetDlgItem(p_dialog, 0x42c), FALSE);
		}
		else if (player.m_team == 1 && count1 == 1) {
			EnableWindow(GetDlgItem(p_dialog, 0x42c), FALSE);
		}
		else {
			EnableWindow(GetDlgItem(p_dialog, 0x42c), TRUE);
		}

		while (count0 < 8) {
			id = g_unk0x10023708[count0];
			SetDlgItemText(p_dialog, id, "");
			count0++;
		}

		while (count1 < 8) {
			id = g_unk0x10023708[8 + count1];
			SetDlgItemText(p_dialog, id, "");
			count1++;
		}
	}

	g_unk0x1001ca90.FUN_10003620();
}

// The team slot dialog (pane 5): the players in their teams' slots, the team labels in their
// colors, and the chat, to everyone or to the team.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000dae1
BOOL CALLBACK FUN_1000dae1(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam)
{
	MechU32 command;
	MechU32 notify;
	HDC dc;
	HDC memoryDC;
	PAINTSTRUCT paint;
	BITMAP info;
	RECT rect;
	POINT point;

	command = LOWORD(p_wParam);
	notify = HIWORD(p_wParam);

	switch (p_message) {
	case WM_INITDIALOG:
		g_unk0x1001f0f4 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xde),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		FUN_1000d91f(p_dialog, TRUE);
		return TRUE;
	case WM_SHOWWINDOW:
		if (p_wParam) {
			FUN_1000e2e2(p_dialog);
			g_chatLog.Attach(p_dialog);
			g_chatLog.Restore();
			SetDlgItemText(p_dialog, 0x432, g_unk0x1001ca90.m_isHost ? LoadResString(0x77) : LoadResString(0x78));
			FUN_1000d91f(p_dialog, TRUE);
			g_unk0x1001ca90.m_unk0x15c = 5;
		}
		else {
			FUN_1000e396(p_dialog);
		}

		return FALSE;
	case WM_CTLCOLORSTATIC:
		if (GetDlgItem(p_dialog, 0x7d4) == (HWND) p_lParam) {
			SetTextColor((HDC) p_wParam, RGB(0x80, 0, 0));
		}
		else if (GetDlgItem(p_dialog, 0x7d5) == (HWND) p_lParam) {
			SetTextColor((HDC) p_wParam, RGB(0, 0x80, 0));
		}
		else {
			return FALSE;
		}

		SetBkColor((HDC) p_wParam, GetSysColor(COLOR_BTNFACE));
		return (BOOL) GetSysColorBrush(COLOR_BTNFACE);
	case WM_PAINT:
		dc = BeginPaint(p_dialog, &paint);
		memoryDC = CreateCompatibleDC(dc);
		SelectObject(memoryDC, g_unk0x1001f0f4);
		if (g_unk0x10023118) {
			SelectPalette(dc, g_unk0x10023118, FALSE);
			RealizePalette(dc);
		}

		GetObject(g_unk0x1001f0f4, sizeof(info), &info);
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
	case WM_MOUSEMOVE:
		point.x = LOWORD(p_lParam);
		point.y = HIWORD(p_lParam);
		ClientToScreen(p_dialog, &point);
		ScreenToClient(GetParent(p_dialog), &point);
		PostMessage(g_unk0x10023160, p_message, p_wParam, MAKELPARAM(point.x, point.y));
		break;
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
			FUN_1000d91f(p_dialog, FALSE);
		}

		return FALSE;
	case 0x417:
		FUN_10005023(1);
		break;
	case WM_HOTKEY:
		switch ((MechS32) p_wParam) {
		case 500:
			SendMessage(p_dialog, WM_COMMAND, 0x42c, 0);
			break;
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
		if (notify == EN_SETFOCUS) {
			if (GetDlgItem(p_dialog, 0x3f2) != (HWND) p_lParam) {
				SetFocus(NULL);
			}

			return FALSE;
		}

		switch (command) {
		case 0x432:
			if (g_unk0x1001ca90.m_isHost) {
				FUN_10005023(2);
			}
			else {
				FUN_10005023(1);
			}

			break;
		case 0x42c:
			if (g_unk0x1001ca90.m_isHost) {
				FUN_1000c08c(0, NULL, 0, 0);
			}
			else {
				FUN_1000c08c(0, NULL, 0, 0);
				FUN_1000c072();
			}

			break;
		case 0x3e9:
			break;
		case 0x3ea:
			FUN_10005023(7);
			break;
		case 0x3eb:
			FUN_10005023(6);
			break;
		case 0x431:
			FUN_10005023(8);
			break;
		case 0x7e0:
			FUN_10005e50(g_unk0x1001cbf0, sizeof(g_unk0x1001cbf0));
			WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 7);
			break;
		case 0x43b:
			FUN_100042ab();
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
		default:
			return TRUE;
		}

		break;
	case WM_DESTROY:
		DeleteObject(g_unk0x1001f0f4);
		return FALSE;
	}

	return FALSE;
}

// FUNCTION: NETMECHW 0x1000e2e2
void FUN_1000e2e2(HWND p_hWnd)
{
	if (g_unk0x10023138[5] == p_hWnd) {
		RegisterHotKey(p_hWnd, 500, MOD_ALT, 'A');
		RegisterHotKey(p_hWnd, 6, MOD_ALT, 'T');
	}

	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'C');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');

	if (g_unk0x1001ca90.m_isHost) {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	}
	else {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'L');
	}

	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x1000e396
void FUN_1000e396(HWND p_hWnd)
{
	if (g_unk0x10023138[5] == p_hWnd) {
		UnregisterHotKey(p_hWnd, 500);
		UnregisterHotKey(p_hWnd, 6);
	}

	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);
	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
