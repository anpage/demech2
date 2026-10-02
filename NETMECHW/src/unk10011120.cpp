#include "unk10011120.h"

#include "chatlog.h"
#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk100089d0.h"
#include "unk1000a650.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The mission whose briefing g_unk0x1001f2a4 shows.
// GLOBAL: NETMECHW 0x10023948
MechChar g_unk0x10023948[4] = "";

// The briefing dialog's picture (FUN_100113ec).
// GLOBAL: NETMECHW 0x1001f29c
HBITMAP g_unk0x1001f29c;

// The briefing's edit control.
// GLOBAL: NETMECHW 0x1001f2a4
HWND g_unk0x1001f2a4;

// GLOBAL: NETMECHW 0x1001f2a8
HWND g_unk0x1001f2a8;

// GLOBAL: NETMECHW 0x1001f2b0
HWND g_unk0x1001f2b0;

// GLOBAL: NETMECHW 0x1001f2b4
HWND g_unk0x1001f2b4;

// GLOBAL: NETMECHW 0x1001f2b8
HWND g_unk0x1001f2b8;

// Shows the game options in a client's dialog p_dialog, if they changed since they were last
// shown, p_force is set or the local player is the host: the briefing of the mission, if it
// changed, the option check boxes and values.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10011120
void FUN_10011120(HWND p_dialog, MechS32 p_force)
{
	MechS32 unused0;
	MechS32 unused1;
	MechS32 unused2;
	CopperField0x4d::Options flags;
	MechChar code[4];
	MechChar text[256];

	unused0 = 0;
	unused1 = 5;
	if (p_force || g_unk0x1001ca90.FUN_100035c0() || g_unk0x1001ca90.m_isHost) {
		EnterCriticalSection(&g_unk0x1001ee60);
		if (p_force) {
			strcpy(g_unk0x10023948, "");
		}

		flags = g_unk0x1001ca90.m_settings.m_options.m_bits;

		if (g_unk0x1001ca90.m_settings.m_unk0x44[0] &&
			_strnicmp(g_unk0x1001ca90.m_settings.m_unk0x44, g_unk0x10023948, 4)) {
			memcpy(g_unk0x10023948, g_unk0x1001ca90.m_settings.m_unk0x44, sizeof(g_unk0x10023948));
			memcpy(code, g_unk0x1001ca90.m_settings.m_unk0x44, sizeof(code));
			FUN_1000a73b(g_unk0x1001f2a4, code);
		}

		CheckDlgButton(p_dialog, 0x421, flags.m_option0);
		CheckDlgButton(p_dialog, 0x422, flags.m_option1);
		CheckDlgButton(p_dialog, 0x423, flags.m_option2);
		CheckDlgButton(p_dialog, 0x424, flags.m_option3);
		CheckDlgButton(p_dialog, 0x425, flags.m_option4);
		CheckDlgButton(p_dialog, 0x426, flags.m_option5);

		g_unk0x1001f2b4 = GetDlgItem(p_dialog, 0x429);
		SetDlgItemText(p_dialog, 0x429, LoadResString(g_unk0x100234f8[g_unk0x1001ca90.m_settings.m_unk0x4c]));
		g_unk0x1001f2a8 = GetDlgItem(p_dialog, 0x42a);
		SetDlgItemText(p_dialog, 0x42a, LoadResString(g_unk0x10023508[g_unk0x1001ca90.m_settings.m_unk0x4a]));
		g_unk0x1001f2b0 = GetDlgItem(p_dialog, 0x433);
		SetDlgItemInt(p_dialog, 0x433, g_unk0x1001ca90.m_settings.m_unk0x49, FALSE);
		g_unk0x1001f2b8 = GetDlgItem(p_dialog, 0x3fb);
		sprintf(text, "%4.2f", g_unk0x1001ca90.m_settings.m_unk0x4b / 16.0);
		SetDlgItemText(p_dialog, 0x3fb, text);

		g_unk0x1001ca90.FUN_10003620();
		LeaveCriticalSection(&g_unk0x1001ee60);
	}
}

// The briefing dialog (pane 6): the mission's briefing and the game options, and the chat.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100113ec
BOOL CALLBACK FUN_100113ec(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam)
{
	MechU32 command;
	MechU32 notify;
	HDC dc;
	HDC memoryDC;
	PAINTSTRUCT paint;
	BITMAP info;
	RECT rect;

	switch (p_message) {
	case WM_INITDIALOG:
		g_unk0x1001f2a4 = GetDlgItem(p_dialog, 0x41b);
		SendMessage(g_unk0x1001f2a4, WM_SETFONT, (WPARAM) g_unk0x10023164, 0);
		g_unk0x1001f29c = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xdd),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		FUN_10011120(p_dialog, TRUE);
		ShowWindow(p_dialog, SW_HIDE);
		return TRUE;
	case WM_SHOWWINDOW:
		if (p_wParam) {
			FUN_10011b0b(p_dialog);
			g_chatLog.Attach(p_dialog);
			g_chatLog.Restore();
			SetDlgItemText(p_dialog, 0x432, g_unk0x1001ca90.m_isHost ? LoadResString(0x77) : LoadResString(0x78));
			FUN_10011120(p_dialog, TRUE);
			EnterCriticalSection(&g_unk0x1001ee60);
			EnableWindow(GetDlgItem(p_dialog, 0x434), g_unk0x1001ca90.m_settings.m_unk0x00 & 2);
			EnableWindow(GetDlgItem(p_dialog, 0x438), g_unk0x1001ca90.m_settings.m_unk0x00 & 2);
			EnableWindow(GetDlgItem(p_dialog, 0x7d3), g_unk0x1001ca90.m_settings.m_unk0x00 & 2);
			LeaveCriticalSection(&g_unk0x1001ee60);
			g_unk0x1001ca90.m_unk0x15c = 6;
		}
		else {
			FUN_10011bad(p_dialog);
		}

		return FALSE;
	case WM_DESTROY:
		DeleteObject(g_unk0x1001f29c);
		g_chatLog.Detach();
		return FALSE;
	case WM_DRAWITEM:
		FUN_10004be3(p_wParam, p_lParam);
		return TRUE;
	case WM_PAINT:
		dc = BeginPaint(p_dialog, &paint);
		memoryDC = CreateCompatibleDC(dc);
		SelectObject(memoryDC, g_unk0x1001f29c);
		if (g_unk0x10023118) {
			SelectPalette(dc, g_unk0x10023118, FALSE);
			RealizePalette(dc);
		}

		GetObject(g_unk0x1001f29c, sizeof(info), &info);
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
			FUN_10011120(p_dialog, FALSE);
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
			WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 8);
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
			break;
		case 0x431:
			FUN_10005023(8);
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
			break;
		}

		break;
	}

	return FALSE;
}

// FUNCTION: NETMECHW 0x10011b0b
void FUN_10011b0b(HWND p_hWnd)
{
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

// FUNCTION: NETMECHW 0x10011bad
void FUN_10011bad(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);

	if (g_unk0x1001ca90.m_settings.m_unk0x00 == 2) {
		UnregisterHotKey(p_hWnd, 6);
	}

	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
