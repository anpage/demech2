#include "unk10006060.h"

#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"

#include <dplay.h>
#include <string.h>
#include <windows.h>

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
