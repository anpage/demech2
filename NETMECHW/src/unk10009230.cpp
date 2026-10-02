#include "unk10009230.h"

#include "chatlog.h"
#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk100089d0.h"
#include "unk1000a650.h"

#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The host's options pane: the mission lists, the option check boxes, and the values.

// The time limit shown in control 0x3fb, in units of 16 of m_settings.m_unk0x4b.
// GLOBAL: NETMECHW 0x10023518
double g_unk0x10023518 = 1.0;

// The options pane's picture (FUN_100098a2).
// GLOBAL: NETMECHW 0x1001efa8
HBITMAP g_unk0x1001efa8;

// The list box of the missions (control 0x41c).
// GLOBAL: NETMECHW 0x1001efac
HWND g_unk0x1001efac;

// The briefing of the selected mission (control 0x41b).
// GLOBAL: NETMECHW 0x1001efb0
HWND g_unk0x1001efb0;

// The combo box of m_settings.m_unk0x4a (control 0x42a).
// GLOBAL: NETMECHW 0x1001efb4
HWND g_unk0x1001efb4;

// The up-down control of control 0x3fb (control 0x428).
// GLOBAL: NETMECHW 0x1001efb8
HWND g_unk0x1001efb8;

// GLOBAL: NETMECHW 0x1001efbc
HWND g_unk0x1001efbc;

// The combo box of m_settings.m_unk0x4c (control 0x429).
// GLOBAL: NETMECHW 0x1001efc0
HWND g_unk0x1001efc0;

// GLOBAL: NETMECHW 0x1001efc4
HWND g_unk0x1001efc4;

// GLOBAL: NETMECHW 0x1001efc8
HWND g_unk0x1001efc8;

// The up-down control of m_settings.m_unk0x49 (control 0x427).
// GLOBAL: NETMECHW 0x1001efcc
HWND g_unk0x1001efcc;

// Sets up the options pane p_dialog: fills the mission lists and shows the game options.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10009230
void FUN_10009230(HWND p_dialog)
{
	CopperField0x4d::Options flags;
	MechChar text[8];
	MechS32 i;
	LRESULT index;
	LRESULT data;

	EnterCriticalSection(&g_unk0x1001ee60);
	flags = g_unk0x1001ca90.m_settings.m_options.m_bits;

	g_unk0x1001efac = GetDlgItem(p_dialog, 0x41c);
	g_unk0x1001efc8 = GetDlgItem(p_dialog, 0x41e);
	g_unk0x1001efb0 = GetDlgItem(p_dialog, 0x41b);
	FUN_1000a650(g_unk0x1001efac, "MISSIONT");
	FUN_1000a650(g_unk0x1001efc8, "MISSIONF");

	SendMessage(g_unk0x1001efac, LB_SETCURSEL, 0, 0);
	index = SendMessage(g_unk0x1001efac, LB_GETCURSEL, 0, 0);
	data = SendMessage(g_unk0x1001efac, LB_GETITEMDATA, index, 0);
	FUN_1000a73b(g_unk0x1001efb0, (MechChar*) data);

	CheckDlgButton(p_dialog, 0x421, flags.m_option0);
	CheckDlgButton(p_dialog, 0x422, flags.m_option1);
	CheckDlgButton(p_dialog, 0x423, flags.m_option2);
	CheckDlgButton(p_dialog, 0x424, flags.m_option3);
	CheckDlgButton(p_dialog, 0x425, flags.m_option4);
	CheckDlgButton(p_dialog, 0x426, flags.m_option5);

	g_unk0x1001efc0 = GetDlgItem(p_dialog, 0x429);
	for (i = 0; i < 4; i++) {
		index = SendMessage(g_unk0x1001efc0, CB_ADDSTRING, 0, (LPARAM) LoadResString(g_unk0x100234f8[i]));
		SendMessage(g_unk0x1001efc0, CB_SETITEMDATA, i, i);
	}

	g_unk0x1001efb4 = GetDlgItem(p_dialog, 0x42a);
	for (i = 0; i < 3; i++) {
		index = SendMessage(g_unk0x1001efb4, CB_ADDSTRING, 0, (LPARAM) LoadResString(g_unk0x10023508[i]));
		SendMessage(g_unk0x1001efb4, CB_SETITEMDATA, i, i);
	}

	g_unk0x1001efbc = GetDlgItem(p_dialog, 0x3f8);
	g_unk0x1001efcc = GetDlgItem(p_dialog, 0x427);
	SendMessage(g_unk0x1001efcc, UDM_SETBUDDY, (WPARAM) GetDlgItem(p_dialog, 0x3f8), 0);
	SendMessage(g_unk0x1001efcc, UDM_SETRANGE, 0, MAKELONG(100, 25));
	SendMessage(g_unk0x1001efcc, UDM_SETPOS, 0, 1);
	SetDlgItemInt(p_dialog, 0x3f8, g_unk0x1001ca90.m_settings.m_unk0x49, FALSE);

	g_unk0x1001efc4 = GetDlgItem(p_dialog, 0x3fb);
	g_unk0x1001efb8 = GetDlgItem(p_dialog, 0x428);
	SendMessage(g_unk0x1001efb8, UDM_SETBUDDY, (WPARAM) g_unk0x1001efc4, 0);
	SendMessage(g_unk0x1001efb8, UDM_SETRANGE, 0, MAKELONG(4, 0));
	SendMessage(g_unk0x1001efb8, UDM_SETPOS, 0, 1);

	g_unk0x10023518 = (MechU32) g_unk0x1001ca90.m_settings.m_unk0x4b / 16.0;
	sprintf(text, "%4.2f", g_unk0x10023518);
	SetDlgItemText(p_dialog, 0x3fb, text);

	LeaveCriticalSection(&g_unk0x1001ee60);
}

// Takes the value of the control p_control of the options pane p_dialog into the game options;
// one of the option check boxes takes them all.
// FUNCTION: NETMECHW 0x10009615
MechS32 FUN_10009615(HWND p_dialog, MechS32 p_control)
{
	CopperField0x4d::Options flags;
	BOOL translated;
	LRESULT index;

	EnterCriticalSection(&g_unk0x1001ee60);
	switch (p_control) {
	case 0x3fb:
		g_unk0x1001ca90.m_settings.m_unk0x4b = (MechU8) (g_unk0x10023518 * 16.0);
		break;
	case 0x421:
	case 0x422:
	case 0x423:
	case 0x424:
	case 0x425:
	case 0x426:
		flags.m_option0 = IsDlgButtonChecked(p_dialog, 0x421);
		flags.m_option1 = IsDlgButtonChecked(p_dialog, 0x422);
		flags.m_option2 = IsDlgButtonChecked(p_dialog, 0x423);
		flags.m_option3 = IsDlgButtonChecked(p_dialog, 0x424);
		flags.m_option4 = IsDlgButtonChecked(p_dialog, 0x425);
		flags.m_option5 = IsDlgButtonChecked(p_dialog, 0x426);
		flags.m_option6 = 0;
		flags.m_option7 = 0;
		g_unk0x1001ca90.m_settings.m_options.m_bits = flags;
		break;
	case 0x3f8:
		g_unk0x1001ca90.m_settings.m_unk0x49 = GetDlgItemInt(p_dialog, 0x3f8, &translated, FALSE);
		break;
	case 0x42a:
		index = SendMessage(g_unk0x1001efb4, CB_GETCURSEL, 0, 0);
		g_unk0x1001ca90.m_settings.m_unk0x4a = (MechU8) SendMessage(g_unk0x1001efb4, CB_GETITEMDATA, index, 0);
		break;
	case 0x429:
		index = SendMessage(g_unk0x1001efc0, CB_GETCURSEL, 0, 0);
		g_unk0x1001ca90.m_settings.m_unk0x4c = (MechU8) SendMessage(g_unk0x1001efc0, CB_GETITEMDATA, index, 0);
		break;
	}

	LeaveCriticalSection(&g_unk0x1001ee60);
	return TRUE;
}

// Takes all the values of the options pane p_dialog into the game options.
// FUNCTION: NETMECHW 0x10009838
MechS32 FUN_10009838(HWND p_dialog)
{
	FUN_10009615(p_dialog, 0x3fb);
	FUN_10009615(p_dialog, 0x421);
	FUN_10009615(p_dialog, 0x3f8);
	FUN_10009615(p_dialog, 0x42a);
	FUN_10009615(p_dialog, 0x429);
	return TRUE;
}

// The host's options dialog (pane 2): the mission lists, the options, and the chat. "Start"
// (0x43a) takes the options and moves on to the player-slot pane of the game's kind.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes (which moves the
// targets of the WM_HOTKEY jump table).
// FUNCTION: NETMECHW 0x100098a2
BOOL CALLBACK FUN_100098a2(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam)
{
	LRESULT index;
	LRESULT mission;
	MechS32 i;
	HDC dc;
	HDC memoryDC;
	PAINTSTRUCT paint;
	BITMAP info;
	RECT rect;
	NMHDR* header;
	NM_UPDOWN* upDown;
	MechU32 command;
	double delta;
	MechChar text[8];
	MechU32 notify;

	switch (p_message) {
	case WM_INITDIALOG:
		ShowWindow(p_dialog, SW_HIDE);
		EnterCriticalSection(&g_unk0x1001ee60);
		g_unk0x1001ca90.m_settings.m_unk0x00 |= 2;
		g_unk0x1001efa8 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xdc),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		SendMessage(GetDlgItem(p_dialog, 0x41b), WM_SETFONT, (WPARAM) g_unk0x10023164, 0);
		FUN_10009230(p_dialog);

		index = SendMessage(g_unk0x1001efac, LB_GETCURSEL, 0, 0);
		mission = SendMessage(g_unk0x1001efac, LB_GETITEMDATA, index, 0);
		strncpy(g_unk0x1001ca90.m_settings.m_unk0x44, (MechChar*) mission, 4);

		SendMessage(g_unk0x1001efc0, CB_SETCURSEL, 0, 0);
		for (i = 0; i < 4; i++) {
			if (SendMessage(g_unk0x1001efc0, CB_GETITEMDATA, i, 0) == g_unk0x1001ca90.m_settings.m_unk0x4c) {
				SendMessage(g_unk0x1001efc0, CB_SETCURSEL, i, 0);
				break;
			}
		}

		SendMessage(g_unk0x1001efb4, CB_SETCURSEL, 0, 0);
		for (i = 0; i < 3; i++) {
			if (SendMessage(g_unk0x1001efb4, CB_GETITEMDATA, i, 0) == g_unk0x1001ca90.m_settings.m_unk0x4a) {
				SendMessage(g_unk0x1001efb4, CB_SETCURSEL, i, 0);
				break;
			}
		}

		FUN_10009838(p_dialog);
		LeaveCriticalSection(&g_unk0x1001ee60);
		return TRUE;
	case WM_SHOWWINDOW:
		if (p_wParam) {
			SetDlgItemText(p_dialog, 0x432, g_unk0x1001ca90.m_unk0x00 == 2 ? "Ha&ng Up" : LoadResString(0x77));
			FUN_1000a4f6(p_dialog);
			g_chatLog.Attach(p_dialog);
			g_chatLog.Restore();
		}
		else {
			FUN_1000a5c4(p_dialog);
		}

		FUN_10009838(p_dialog);
		return FALSE;
	case WM_PAINT:
		dc = BeginPaint(p_dialog, &paint);
		memoryDC = CreateCompatibleDC(dc);
		SelectObject(memoryDC, g_unk0x1001efa8);
		if (g_unk0x10023118) {
			SelectPalette(dc, g_unk0x10023118, FALSE);
			RealizePalette(dc);
		}

		GetObject(g_unk0x1001efa8, sizeof(info), &info);
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
	case WM_NOTIFY:
		header = (NMHDR*) p_lParam;
		if (header->code == UDN_DELTAPOS) {
			upDown = (NM_UPDOWN*) header;
			command = header->idFrom;
			if (command == 0x428) {
				delta = upDown->iDelta * 0.25;
				if ((g_unk0x10023518 += delta) < 0.25) {
					g_unk0x10023518 = 4.0;
				}
				else if (g_unk0x10023518 > 4.0) {
					g_unk0x10023518 = 0.25;
				}

				sprintf(text, "%4.2f", g_unk0x10023518);
				SetDlgItemText(p_dialog, 0x3fb, text);
				FUN_10009615(p_dialog, 0x3fb);
			}
			else if (command == 0x427) {
				EnterCriticalSection(&g_unk0x1001ee60);
				g_unk0x1001ca90.m_settings.m_unk0x49 += upDown->iDelta * 5;
				if (g_unk0x1001ca90.m_settings.m_unk0x49 < 25) {
					g_unk0x1001ca90.m_settings.m_unk0x49 = 100;
				}
				else if (g_unk0x1001ca90.m_settings.m_unk0x49 > 100) {
					g_unk0x1001ca90.m_settings.m_unk0x49 = 25;
				}

				SetDlgItemInt(p_dialog, 0x3f8, g_unk0x1001ca90.m_settings.m_unk0x49, FALSE);
				LeaveCriticalSection(&g_unk0x1001ee60);
				FUN_10009615(p_dialog, 0x3f8);
				return FALSE;
			}

			return TRUE;
		}
	case WM_DESTROY:
		DeleteObject(g_unk0x1001efa8);
		g_chatLog.Detach();
		free((void*) SendMessage(g_unk0x1001efac, LB_GETITEMDATA, 0, 0));
		free((void*) SendMessage(g_unk0x1001efc8, LB_GETITEMDATA, 0, 0));
		return FALSE;
	case WM_HOTKEY:
		switch ((MechS32) p_wParam) {
		case 300:
			SetFocus(GetDlgItem(p_dialog, 0x41c));
			break;
		case 301:
			SetFocus(GetDlgItem(p_dialog, 0x41e));
			break;
		case 302:
			SetFocus(GetDlgItem(p_dialog, 0x421));
			break;
		case 303:
			SendMessage(p_dialog, WM_COMMAND, 0x43a, 0);
			break;
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

		return TRUE;
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
		case 0x43a:
			if (FUN_10009838(p_dialog)) {
				EnterCriticalSection(&g_unk0x1001ee60);
				g_unk0x1001ca90.m_settings.m_unk0x00 |= 1;
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

				LeaveCriticalSection(&g_unk0x1001ee60);
			}

			break;
		case 0x7e0:
			FUN_10005e50(g_unk0x1001cbf0, sizeof(g_unk0x1001cbf0));
			WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 4);
			break;
		case 0x43b:
			FUN_100042ab();
			break;
		case 0x432:
			FUN_10005023(1);
			break;
		case 0x41c:
			if (notify == LBN_SELCHANGE) {
				SendMessage(g_unk0x1001efc8, LB_SETCURSEL, (WPARAM) -1, 0);
				g_unk0x1001efb0 = GetDlgItem(p_dialog, 0x41b);
				index = SendMessage(g_unk0x1001efac, LB_GETCURSEL, 0, 0);
				mission = SendMessage(g_unk0x1001efac, LB_GETITEMDATA, index, 0);
				FUN_1000a73b(g_unk0x1001efb0, (MechChar*) mission);
				EnterCriticalSection(&g_unk0x1001ee60);
				strncpy(g_unk0x1001ca90.m_settings.m_unk0x44, (MechChar*) mission, 4);
				g_unk0x1001ca90.m_settings.m_unk0x00 |= 2;
				LeaveCriticalSection(&g_unk0x1001ee60);
			}

			break;
		case 0x41e:
			if (notify == LBN_SELCHANGE) {
				SendMessage(g_unk0x1001efac, LB_SETCURSEL, (WPARAM) -1, 0);
				g_unk0x1001efb0 = GetDlgItem(p_dialog, 0x41b);
				index = SendMessage(g_unk0x1001efc8, LB_GETCURSEL, 0, 0);
				mission = SendMessage(g_unk0x1001efc8, LB_GETITEMDATA, index, 0);
				FUN_1000a73b(g_unk0x1001efb0, (MechChar*) mission);
				EnterCriticalSection(&g_unk0x1001ee60);
				strncpy(g_unk0x1001ca90.m_settings.m_unk0x44, (MechChar*) mission, 4);
				g_unk0x1001ca90.m_settings.m_unk0x00 &= ~2;
				LeaveCriticalSection(&g_unk0x1001ee60);
			}

			break;
		case 0x421:
		case 0x422:
		case 0x423:
		case 0x424:
		case 0x425:
		case 0x426:
			FUN_10009615(p_dialog, 0x421);
			break;
		case 0x42a:
			if (notify == CBN_SELCHANGE) {
				FUN_10009615(p_dialog, 0x42a);
			}

			break;
		case 0x429:
			if (notify == CBN_SELCHANGE) {
				FUN_10009615(p_dialog, 0x429);
			}

			break;
		case 0x3ee:
			if (notify == LBN_SELCHANGE) {
				SetDlgItemText(p_dialog, 0x3ec, LoadResString(0x75));
			}

			break;
		default:
			return TRUE;
			break;
		}

		return FALSE;
	}

	return FALSE;
}

// FUNCTION: NETMECHW 0x1000a4f6
void FUN_1000a4f6(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 300, MOD_ALT, 'T');
	RegisterHotKey(p_hWnd, 301, MOD_ALT, 'F');
	RegisterHotKey(p_hWnd, 302, MOD_ALT, 'O');
	RegisterHotKey(p_hWnd, 303, MOD_ALT, 'A');
	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'C');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');

	if (g_unk0x1001ca90.m_unk0x00 == 2) {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'N');
	}
	else {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	}

	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x1000a5c4
void FUN_1000a5c4(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 300);
	UnregisterHotKey(p_hWnd, 301);
	UnregisterHotKey(p_hWnd, 302);
	UnregisterHotKey(p_hWnd, 303);
	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);
	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
