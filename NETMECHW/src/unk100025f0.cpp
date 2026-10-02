#include "unk100025f0.h"

#include "bwdwriter.h"
#include "chatlog.h"
#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk10007c30.h"
#include "unk1000aa90.h"
#include "unk1000b400.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The mech selection dialog's picture, and its mech and variant list boxes (FUN_1000268a).
// GLOBAL: NETMECHW 0x1001c308
HBITMAP g_unk0x1001c308;

// GLOBAL: NETMECHW 0x1001c30c
HWND g_unk0x1001c30c;

// GLOBAL: NETMECHW 0x1001c310
HWND g_unk0x1001c310;

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

// The mech selection dialog (pane 7): the player picks a mech and a variant while the host
// sets up the game, and chats.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000268a
BOOL CALLBACK FUN_1000268a(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam)
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
		g_unk0x1001c30c = GetDlgItem(p_dialog, 0x3fe);
		g_unk0x1001c310 = GetDlgItem(p_dialog, 0x400);
		FUN_10003154(g_unk0x1001c30c);
		SendMessage(g_unk0x1001c30c, LB_SETCURSEL, 0, 0);
		FUN_10002e0e(p_dialog);
		SendMessage(GetDlgItem(p_dialog, 0x413), WM_SETFONT, (WPARAM) g_unk0x10023164, 0);
		g_unk0x1001c308 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xda),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		FUN_100025f0(p_dialog, TRUE);
		return TRUE;
	case WM_SHOWWINDOW:
		if (p_wParam) {
			RegisterLobbyHotKeys(p_dialog);
			g_chatLog.Attach(p_dialog);
			g_chatLog.Restore();
			FUN_100025f0(p_dialog, TRUE);
			g_unk0x1001ca90.m_unk0x15c = 7;
			EnterCriticalSection(&g_unk0x1001ee60);
			EnableWindow(GetDlgItem(p_dialog, 0x434), g_unk0x1001ca90.m_settings.m_unk0x00 & 2);
			EnableWindow(GetDlgItem(p_dialog, 0x438), g_unk0x1001ca90.m_settings.m_unk0x00 & 2);
			EnableWindow(GetDlgItem(p_dialog, 0x7d3), g_unk0x1001ca90.m_settings.m_unk0x00 & 2);
			LeaveCriticalSection(&g_unk0x1001ee60);
		}
		else {
			UnregisterLobbyHotKeys(p_dialog);
		}

		return FALSE;
	case WM_DRAWITEM:
		FUN_10004be3(p_wParam, p_lParam);
		return TRUE;
	case WM_PAINT:
		dc = BeginPaint(p_dialog, &paint);
		memoryDC = CreateCompatibleDC(dc);
		SelectObject(memoryDC, g_unk0x1001c308);
		if (g_unk0x10023118) {
			SelectPalette(dc, g_unk0x10023118, FALSE);
			RealizePalette(dc);
		}

		GetObject(g_unk0x1001c308, sizeof(info), &info);
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

		return FALSE;
	case 0x417:
		FUN_10005023(1);
		break;
	case WM_HOTKEY:
		switch ((MechS32) p_wParam) {
		case 400:
			SetFocus(GetDlgItem(p_dialog, 0x3fe));
			break;
		case 401:
			SetFocus(GetDlgItem(p_dialog, 0x400));
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
		case 0x7e0:
			FUN_10005e50(g_unk0x1001cbf0, sizeof(g_unk0x1001cbf0));
			WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 9);
			break;
		case 0x43b:
			FUN_100042ab();
			break;
		case 0x3fe:
			if (notify == LBN_SELCHANGE) {
				FUN_10002e0e(p_dialog);
			}

			break;
		case 0x400:
			if (notify == LBN_SELCHANGE) {
				FUN_10002ec3(p_dialog);
			}

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
			break;
		case 0x3eb:
			FUN_10005023(6);
			break;
		case 0x431:
			FUN_10005023(8);
			break;
		case 0x432:
			if (g_unk0x1001ca90.m_isHost) {
				FUN_10005023(2);
			}
			else {
				FUN_10005023(1);
			}

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
		DeleteObject(g_unk0x1001c308);
		return FALSE;
	}

	return FALSE;
}

// Takes the mech selected in the list box (control 0x3fe) of the dialog p_dialog, and shows it.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10002e0e
void FUN_10002e0e(HWND p_dialog)
{
	MechS32 unused0;
	MechS32 unused1;
	HWND listBox;
	HWND text;
	LRESULT index;

	unused0 = 0;
	unused1 = 0;
	listBox = GetDlgItem(p_dialog, 0x3fe);
	text = GetDlgItem(p_dialog, 0x400);

	index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
	g_unk0x10023388 = SendMessage(listBox, LB_GETITEMDATA, index, 0);
	g_unk0x1002338c = 0;
	g_unk0x10023394 = 0;

	FUN_100080e1(text, g_unk0x1001c318[g_unk0x10023388].m_code);
	FUN_10002ec3(p_dialog);
}

// Takes the mech and variant selected in the dialog p_dialog's list boxes (controls 0x3fe and
// 0x400) as the local player's mech file, shows its description and sends it to the others: a
// standard variant by name, a variant of the player's own as a copy (CopyPlayerMech).
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes, and the operand
// order of the first comparison (the original loads g_unk0x1001fe74).
// FUNCTION: NETMECHW 0x10002ec3
void FUN_10002ec3(HWND p_dialog)
{
	LRESULT mech;
	LRESULT variant;
	HWND mechList;
	HWND variantList;
	HWND text;
	LRESULT index;
	MechChar name[12];
	MechChar description[0x800];

	mech = 0;
	variant = 0;
	mechList = GetDlgItem(p_dialog, 0x3fe);
	variantList = GetDlgItem(p_dialog, 0x400);
	text = GetDlgItem(p_dialog, 0x413);

	index = SendMessage(mechList, LB_GETCURSEL, 0, 0);
	mech = SendMessage(mechList, LB_GETITEMDATA, index, 0);
	index = SendMessage(variantList, LB_GETCURSEL, 0, 0);
	variant = SendMessage(variantList, LB_GETITEMDATA, index, 0);

	if (variant >= g_unk0x1001fe74) {
		g_unk0x10023394 = 1;
		g_unk0x1002338c = variant - g_unk0x1001fe74;
	}
	else {
		g_unk0x10023394 = 0;
		g_unk0x1002338c = variant;
	}

	EnterCriticalSection(&g_unk0x1001ee60);
	if (!g_unk0x10023394) {
		sprintf(name, "%3.3s%02dSTD", g_unk0x1001c318[mech].m_code, variant);
		strcpy(g_unk0x1001ca90.m_mechFile, name);
	}
	else {
		sprintf(name, "%3.3s%02dUSR", g_unk0x1001c318[mech].m_code, variant - g_unk0x1001fe74);
		CopyPlayerMech(name);
		EnterCriticalSection(&g_unk0x1001ca78);
		sprintf(
			g_unk0x1001ca90.m_mechFile,
			g_unk0x1002308c,
			g_unk0x1001c318[mech].m_code,
			g_unk0x1001ce18[FUN_1000b02b(g_unk0x1001ca90.m_playerId)]
		);
		LeaveCriticalSection(&g_unk0x1001ca78);
	}

	FUN_10007c5a(name, description, sizeof(description));
	SendMessage(text, WM_SETTEXT, 0, (LPARAM) description);
	if (g_unk0x1001ca90.m_isHost) {
		FUN_1000c08c(4, g_unk0x1001ca90.m_mechFile, 9, 0);
	}
	else {
		FUN_1000c08c(4, g_unk0x1001ca90.m_mechFile, 9, 0);
		FUN_1000c072();
	}

	LeaveCriticalSection(&g_unk0x1001ee60);
}

// Fills the list box p_listBox with the mechs of g_unk0x1001c318 and selects g_unk0x10023388.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10003154
void FUN_10003154(HWND p_listBox)
{
	MechS32 i;
	LRESULT index;

	index = 0;
	SendMessage(p_listBox, LB_RESETCONTENT, 0, 0);
	for (i = 0; i < g_unk0x1001fe70; i++) {
		index = SendMessage(p_listBox, LB_ADDSTRING, 0, (LPARAM) g_unk0x1001c318[i].m_name);
		SendMessage(p_listBox, LB_SETITEMDATA, index, i);
	}

	SendMessage(p_listBox, LB_SETCURSEL, g_unk0x10023388, 0);
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
