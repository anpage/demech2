#include "unk10006060.h"

#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"

#include <dplay.h>
#include <string.h>
#include <windows.h>

// The connection dialog's picture (FUN_1000654c).
// GLOBAL: NETMECHW 0x1001ee78
HBITMAP g_unk0x1001ee78;

// GLOBAL: NETMECHW 0x1001ee80
MechChar g_unk0x1001ee80[256];

// Returns the first word of p_name, in upper case, in a static buffer.
// FUNCTION: NETMECHW 0x10006060
MechChar* FUN_10006060(MechChar* p_name)
{
	MechChar name[256];

	strncpy(name, p_name, sizeof(name) - 2);
	_strupr(name);
	strcpy(g_unk0x1001ee80, strtok(name, " "));
	return g_unk0x1001ee80;
}

// The kind of the DirectPlay service provider p_name: 2 to 6 for the providers named by the
// strings 0x7c to 0x80, 0 for any other.
// FUNCTION: NETMECHW 0x100060dc
MechS32 FUN_100060dc(MechChar* p_name)
{
	MechChar* name;

	name = FUN_10006060(p_name);
	if (!strcmp(name, LoadResString(0x7c))) {
		return 2;
	}
	else if (!strcmp(name, LoadResString(0x7d))) {
		return 3;
	}
	else if (!strcmp(name, LoadResString(0x7e))) {
		return 4;
	}
	else if (!strcmp(name, LoadResString(0x7f))) {
		return 5;
	}
	else if (!strcmp(name, LoadResString(0x80))) {
		return 6;
	}
	else {
		return 0;
	}
}

// The DirectPlayEnumerate callback of the connection dialog: adds the service provider to the
// list box *p_context, with its GUID as item data.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100062d9
BOOL FAR PASCAL AddServiceProvider(LPGUID p_guid, LPSTR p_name, DWORD, DWORD, LPVOID p_context)
{
	HWND* context;
	HWND listBox;
	LRESULT index;

	context = (HWND*) p_context;
	listBox = *context;
	if (listBox) {
		index = SendMessage(listBox, LB_ADDSTRING, 0, (LPARAM) p_name);
		SendMessage(listBox, LB_SETITEMDATA, index, (LPARAM) p_guid);
	}

	return TRUE;
}

// Takes the player name and the service provider from the connection dialog p_dialog; returns
// whether both are set.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000633a
MechS32 FUN_1000633a(HWND p_dialog)
{
	HWND listBox;
	MechChar name[DPSHORTNAMELEN + 1];
	LRESULT index;
	MechChar provider[2048];
	MechChar message[128];
	MechChar message2[128];

	listBox = GetDlgItem(p_dialog, 0x3f4);
	GetDlgItemText(p_dialog, 0x3f7, name, DPSHORTNAMELEN);
	TrimLeadingSpace(name);

	if (!name[0]) {
		wsprintf(message, LoadResString(0x6c));
		MessageBox(p_dialog, message, "NetMech for Windows\xae 95", MB_OK);
		SetDlgItemText(p_dialog, 0x3f7, "");
		SetFocus(GetDlgItem(p_dialog, 0x3f7));
		return FALSE;
	}

	index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
	if (index == LB_ERR) {
		wsprintf(message2, LoadResString(0x6d));
		MessageBox(p_dialog, message2, "NetMech for Windows\xae 95", MB_OK);
		SetFocus(GetDlgItem(p_dialog, 0x3f4));
		return FALSE;
	}

	strcpy(g_unk0x1001ca90.m_playerName, name);
	g_unk0x1001ca90.m_unk0x04 = (LPGUID) SendMessage(listBox, LB_GETITEMDATA, index, 0);
	SendMessage(listBox, LB_GETTEXT, index, (LPARAM) provider);
	g_unk0x1001ca90.m_unk0x00 = FUN_100060dc(provider);
	return TRUE;
}

// Fills the connection dialog p_dialog: the player name, and the first service provider selected.
// FUNCTION: NETMECHW 0x100064e4
void FUN_100064e4(HWND p_dialog)
{
	SendMessage(GetDlgItem(p_dialog, 0x3f7), EM_LIMITTEXT, 15, 0);
	SetDlgItemText(p_dialog, 0x3f7, g_unk0x1001ca90.m_playerName);
	SendMessage(GetDlgItem(p_dialog, 0x3f4), LB_SETCURSEL, 0, 0);
}

// The connection dialog: the player name and the service provider.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes (which moves the
// targets of the WM_HOTKEY jump table).
// FUNCTION: NETMECHW 0x1000654c
BOOL CALLBACK FUN_1000654c(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM)
{
	HWND listBox;
	HRESULT result;
	HDC dc;
	HDC memoryDC;
	PAINTSTRUCT paint;
	BITMAP info;
	RECT rect;
	MechU32 command;
	MechU32 notify;
	HRESULT created;
	DPCAPS caps;

	switch (p_message) {
	case WM_INITDIALOG:
		listBox = GetDlgItem(p_dialog, 0x3f4);
		result = DirectPlayEnumerate(AddServiceProvider, &listBox) & 0xfff;
		g_unk0x1001ee78 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xdb),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		FUN_100064e4(p_dialog);
		return TRUE;
	case WM_SHOWWINDOW:
		if (p_wParam) {
			FUN_10006a68(p_dialog);
		}
		else {
			FUN_10006ac8(p_dialog);
		}

		return FALSE;
	case WM_PAINT:
		dc = BeginPaint(p_dialog, &paint);
		memoryDC = CreateCompatibleDC(dc);
		SelectObject(memoryDC, g_unk0x1001ee78);
		if (g_unk0x10023118) {
			SelectPalette(dc, g_unk0x10023118, FALSE);
			RealizePalette(dc);
		}

		GetObject(g_unk0x1001ee78, sizeof(info), &info);
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
	case WM_HOTKEY:
		switch (p_wParam) {
		case 100:
			SetFocus(GetDlgItem(p_dialog, 0x3f7));
			break;
		case 101:
			SetFocus(GetDlgItem(p_dialog, 0x3f4));
			break;
		case 2:
			SendMessage(p_dialog, WM_COMMAND, 0x7e0, 0);
			break;
		case 102:
			SendMessage(p_dialog, WM_COMMAND, IDOK, 0);
			break;
		case 3:
			SendMessage(p_dialog, WM_COMMAND, 0x43b, 0);
			break;
		}

		break;
	case WM_COMMAND:
		command = LOWORD(p_wParam);
		notify = HIWORD(p_wParam);
		switch (command) {
		case IDOK:
			if (FUN_1000633a(p_dialog)) {
				if (g_unk0x1001ca90.m_directPlay) {
					g_unk0x1001ca90.m_directPlay->Close();
					g_unk0x1001ca90.m_directPlay->Release();
					g_unk0x1001ca90.m_directPlay = NULL;
				}

				FUN_10005d9e(g_unk0x1001ca90.m_playerName, strlen(g_unk0x1001ca90.m_playerName) + 1);
				created = DirectPlayCreate(g_unk0x1001ca90.m_unk0x04, &g_unk0x1001ca90.m_directPlay, NULL) & 0xfff;
				g_unk0x1001ca90.m_directPlay->GetCaps(&caps);
				if (caps.dwHundredBaud < 96 && g_unk0x1001ca90.m_unk0x00 == 2) {
					MessageBox(p_dialog, LoadResString(0x88), LoadResString(0x79), MB_OK);
				}

				FUN_10005023(1);
			}

			break;
		case 0x7e0:
			FUN_10005e50(g_unk0x1001cbf0, sizeof(g_unk0x1001cbf0));
			WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 1);
			break;
		case 0x43b:
			FUN_100042ab();
			break;
		default:
			return TRUE;
		}

		return FALSE;
	case WM_DESTROY:
		DeleteObject(g_unk0x1001ee78);
		return FALSE;
	}

	return FALSE;
}

// FUNCTION: NETMECHW 0x10006a68
void FUN_10006a68(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 100, MOD_ALT, 'E');
	RegisterHotKey(p_hWnd, 101, MOD_ALT, 'C');
	RegisterHotKey(p_hWnd, 102, MOD_ALT, 'S');
	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x10006ac8
void FUN_10006ac8(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 100);
	UnregisterHotKey(p_hWnd, 101);
	UnregisterHotKey(p_hWnd, 102);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
