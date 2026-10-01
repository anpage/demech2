#include "unk10003660.h"

#include "bwdwriter.h"
#include "chatlog.h"
#include "decomp.h"
#include "netlaunchinfo.h"
#include "sessionlist.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10006b20.h"
#include "unk10007c30.h"
#include "unk1000aa90.h"
#include "unk1000b400.h"
#include "unk1000f8e0.h"

#include <dplay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// The mech table of the chassis "MECH" (FUN_10007c30). Its entry count is g_unk0x1001fe70
// (bwdwriter.c).
// GLOBAL: NETMECHW 0x1001c318
MechTableEntry g_unk0x1001c318[32];

// Guards the player table.
// GLOBAL: NETMECHW 0x1001ca78
CRITICAL_SECTION g_unk0x1001ca78;

// GLOBAL: NETMECHW 0x1001ca90
IronLantern0x160 g_unk0x1001ca90;

// Constructed after g_unk0x1001ca90, by the unit's static initializer.
// GLOBAL: NETMECHW 0x1001c8b8
ChatLog g_chatLog;

// GLOBAL: NETMECHW 0x1001ce14
DWORD g_unk0x1001ce14;

// The current directory when the lobby started.
// GLOBAL: NETMECHW 0x1001ccf8
MechChar g_unk0x1001ccf8[MAX_PATH];

// The launch record MECH2.EXE passed to Launcher.
// GLOBAL: NETMECHW 0x1001ce10
NetLaunchInfo* g_unk0x1001ce10;

// A counter per player slot, for the names of the players' mech copies (CopyPlayerMech).
// GLOBAL: NETMECHW 0x1001ce18
MechU8 g_unk0x1001ce18[8];

// GLOBAL: NETMECHW 0x1001ce20
DWORD g_unk0x1001ce20;

// GLOBAL: NETMECHW 0x1001ce2c
HINSTANCE g_hInstance;

// The text of the About box (FUN_10004909).
// GLOBAL: NETMECHW 0x1001ce50
MechChar g_unk0x1001ce50[0x2000];

// GLOBAL: NETMECHW 0x1001ee54
DWORD g_unk0x1001ee54;

// The item data of the last session in the join dialog's list box (FUN_1000f8e0).
// GLOBAL: NETMECHW 0x1001ee58
LRESULT g_unk0x1001ee58;

// Guards the lobby's settings (g_unk0x1001ca90.m_settings).
// GLOBAL: NETMECHW 0x1001ee60
CRITICAL_SECTION g_unk0x1001ee60;

// The application GUID of NetMech's DirectPlay sessions.
// GLOBAL: NETMECHW 0x100230f8
GUID g_unk0x100230f8 = {0x528519a0, 0x1001, 0x11cf, {0x80, 0x5c, 0x00, 0xaa, 0x00, 0x44, 0x43, 0x1f}};

// GLOBAL: NETMECHW 0x10023110
HWND g_unk0x10023110 = NULL;

// GLOBAL: NETMECHW 0x10023120
SessionList* g_sessionList = NULL;

// The receive thread (ReceiveThread), the host's settings broadcast (BroadcastSettingsThread)
// and the session watch (WatchPlayersThread).
// GLOBAL: NETMECHW 0x10023128
HANDLE g_unk0x10023128 = NULL;

// GLOBAL: NETMECHW 0x1002312c
HANDLE g_unk0x1002312c = NULL;

// GLOBAL: NETMECHW 0x10023130
HANDLE g_unk0x10023130 = NULL;

// The lobby's dialogs.
// GLOBAL: NETMECHW 0x10023138
HWND g_unk0x10023138[10] = {NULL};

// The lobby window.
// GLOBAL: NETMECHW 0x10023160
HWND g_unk0x10023160 = NULL;

// The lobby window's font.
// GLOBAL: NETMECHW 0x10023164
HFONT g_unk0x10023164 = NULL;

// GLOBAL: NETMECHW 0x10023168
HGDIOBJ g_unk0x10023168 = NULL;

// GLOBAL: NETMECHW 0x1002317c
MechS32 g_unk0x1002317c = 0;

// Whether new players may join the session (FUN_10005a55, FUN_10005a9a).
// GLOBAL: NETMECHW 0x10023180
MechS32 g_unk0x10023180 = 0;

// GLOBAL: NETMECHW 0x10023184
MechS32 g_unk0x10023184 = 1;

// NetMech's lobby. MECH2.EXE calls it with the launch record to fill in, runs the simulator
// when it returns nonzero, and calls it again when the mission is over.
// STUB: NETMECHW 0x10003684
extern "C" MechS32 __stdcall Launcher(NetLaunchInfo*)
{
	STUB(0x10003684);
	return 0;
}

// Starts the lobby after the simulator returned, as the launch record's flags say.
// FUNCTION: NETMECHW 0x10003cb7
void FUN_10003cb7()
{
	MechU32 mode;

	if (g_unk0x1001ce10->m_unk0x0c & 1) {
		g_unk0x1001ca90.m_unk0x158 = 0;
		FUN_100042ab();
	}
	else if (g_unk0x1001ca90.m_unk0x00 == 2) {
		if (g_unk0x1001ca90.m_isHost) {
			FUN_1000ff61();
			FUN_10005023(2);
		}
		else {
			FUN_1000fc72();
			FUN_10005023(3);
		}
	}
	else {
		mode = g_unk0x1001ce10->m_unk0x0c & 0xf000;
		switch (mode) {
		case 0x1000:
			if (g_unk0x10023184) {
				g_unk0x10023184 = 0;
				FUN_10005023(0);
			}
			else {
				FUN_1000b0c8();
				if (g_unk0x1001ce10->m_unk0x0c & 4) {
					FUN_10005023(1);
				}
				else if (g_unk0x1001ce10->m_unk0x0c & 8) {
					FUN_10005023(0);
				}
			}
			break;
		case 0x2000:
			if (g_unk0x10023184) {
				g_unk0x10023184 = 0;
				FUN_10005023(0);
			}
			else {
				FUN_1000b0c8();
				FUN_10005023(1);
			}
			break;
		case 0x3000:
			FUN_10005023(0);
			break;
		case 0x4000:
			FUN_10005023(0);
			break;
		}
	}
}

// Registers the lobby window's class.
// FUNCTION: NETMECHW 0x10003e52
BOOL FUN_10003e52(HINSTANCE p_hInstance)
{
	WNDCLASS wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = FUN_100042d9;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = p_hInstance;
	wc.hIcon = LoadIcon(p_hInstance, MAKEINTRESOURCE(0x68));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "NetMech for Windows\xae 95";
	return RegisterClass(&wc);
}

// Stops the receive, settings broadcast and session watch threads.
// FUNCTION: NETMECHW 0x10003ec8
void FUN_10003ec8()
{
	DWORD wait;

	if (g_unk0x10023128) {
		PostThreadMessage(g_unk0x1001ce20, 0x418, 0, 0);
		wait = WaitForSingleObject(g_unk0x10023128, INFINITE);
		CloseHandle(g_unk0x10023128);
		g_unk0x10023128 = NULL;
	}

	if (g_unk0x1002312c) {
		PostThreadMessage(g_unk0x1001ce14, 0x418, 0, 0);
		wait = WaitForSingleObject(g_unk0x1002312c, INFINITE);
		CloseHandle(g_unk0x1002312c);
		g_unk0x1002312c = NULL;
	}

	if (g_unk0x10023130) {
		PostThreadMessage(g_unk0x1001ee54, 0x418, 0, 0);
		wait = WaitForSingleObject(g_unk0x10023130, INFINITE);
		CloseHandle(g_unk0x10023130);
		g_unk0x10023130 = NULL;
	}
}

// Creates the lobby window, and loads the mech table. Returns whether it succeeded; tells the
// user why not otherwise.
// FUNCTION: NETMECHW 0x10003fb6
BOOL FUN_10003fb6(HINSTANCE p_hInstance)
{
	DWORD error;
	MechChar message[0x100];

	g_hInstance = p_hInstance;
	g_unk0x10023164 = CreateFont(11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, "");

	if (!GetCurrentDirectory(sizeof(g_unk0x1001ccf8), g_unk0x1001ccf8)) {
		error = GetLastError();
		sprintf(message, LoadResString(1), error);
		MessageBox(NULL, message, LoadResString(2), MB_OK);
		return FALSE;
	}

	FUN_100057ba();
	g_unk0x1001fe70 = FUN_10007c30(g_unk0x1001c318);

	g_unk0x10023160 = CreateWindowEx(
		0,
		"NetMech for Windows\xae 95",
		"NetMech for Windows\xae 95",
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		0,
		600,
		400,
		NULL,
		NULL,
		p_hInstance,
		NULL
	);
	if (!g_unk0x10023160) {
		return FALSE;
	}

	if (g_unk0x10023160) {
		CenterWindow(g_unk0x10023160);
		ShowWindow(g_unk0x10023160, SW_SHOWNORMAL);
		UpdateWindow(g_unk0x10023160);
		FUN_10005023(0);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// Stops the threads, deletes the session list and destroys the lobby's dialogs and window.
// FUNCTION: NETMECHW 0x10004118
void FUN_10004118()
{
	MechS32 i;
	HWND dialog;

	FUN_10003ec8();
	if (g_sessionList) {
		g_sessionList->StopThread();
		delete g_sessionList;
		g_sessionList = NULL;
	}

	if (g_unk0x10023160) {
		i = 0;
		while (i < 10) {
			if (g_unk0x10023138[i]) {
				dialog = g_unk0x10023138[i];
				g_unk0x10023138[i] = NULL;
				if (dialog) {
					DestroyWindow(dialog);
				}
			}

			i++;
		}

		DestroyWindow(g_unk0x10023160);
		g_unk0x10023160 = NULL;
	}
}

// Shuts the lobby down: FUN_10004118, then leaves and releases the DirectPlay session, and
// deletes the GDI objects.
// FUNCTION: NETMECHW 0x100041f5
void FUN_100041f5()
{
	FUN_10004118();
	if (g_unk0x1001ca90.m_directPlay) {
		FUN_1000b54e(g_unk0x1001ca90.m_playerId);
		g_unk0x1001ca90.m_directPlay->DestroyPlayer(g_unk0x1001ca90.m_playerId);
		g_unk0x1001ca90.m_directPlay->Close();
		g_unk0x1001ca90.m_directPlay->Release();
		g_unk0x1001ca90.m_directPlay = NULL;
	}

	if (g_unk0x10023164) {
		DeleteObject(g_unk0x10023164);
		g_unk0x10023164 = NULL;
	}

	if (g_unk0x10023168) {
		DeleteObject(g_unk0x10023168);
		g_unk0x10023168 = NULL;
	}
}

// Stops the lobby: posts message 0x418 to the lobby window's thread.
// FUNCTION: NETMECHW 0x100042ab
void FUN_100042ab()
{
	PostThreadMessage(GetWindowThreadProcessId(g_unk0x10023160, NULL), 0x418, 0, 0);
}

// STUB: NETMECHW 0x100042d9
LRESULT CALLBACK FUN_100042d9(HWND, UINT, WPARAM, LPARAM)
{
	STUB(0x100042d9);
	return 0;
}

// FUNCTION: NETMECHW 0x100048ec
MechS32 FUN_100048ec()
{
	MechS32 result;

	result = TRUE;
	return result;
}

// The About box: the text of string resources 0x8b to 0x99.
// FUNCTION: NETMECHW 0x10004909
BOOL CALLBACK FUN_10004909(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM)
{
	MechS32 i;
	MechS32 tabStop;

	tabStop = 0x5f;
	switch (p_message) {
	case WM_INITDIALOG:
		g_unk0x1001ce50[0] = '\0';
		for (i = 0; i < 15; i++) {
			strcat(g_unk0x1001ce50, LoadResString(i + 0x8b));
		}

		SendMessage(GetDlgItem(p_dialog, 0x7d8), EM_SETTABSTOPS, 1, (LPARAM) &tabStop);
		SendMessage(GetDlgItem(p_dialog, 0x7d8), WM_SETTEXT, 0, (LPARAM) g_unk0x1001ce50);
		break;
	case WM_COMMAND:
		if (LOWORD(p_wParam) == IDOK || LOWORD(p_wParam) == IDCANCEL) {
			EndDialog(p_dialog, TRUE);
			return TRUE;
		}

		break;
	}

	return FALSE;
}

// Creates *p_palette from the colors of bitmap 105, with the system's twenty static colors.
// It reads the colors from the start of the bitmap's header rather than from its color table.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10004a40
void FUN_10004a40(HPALETTE* p_palette)
{
	MechS32 count;
	HGLOBAL resource;
	HDC dc;
	void* data;
	HGLOBAL memory;
	MechS32 i;
	BITMAPINFO* info;
	MechU8* bits;
	LOGPALETTE* palette;

	if (*p_palette) {
		DeleteObject(*p_palette);
		*p_palette = NULL;
	}

	resource = LoadResource(g_hInstance, FindResource(g_hInstance, MAKEINTRESOURCE(0x69), RT_BITMAP));
	data = LockResource(resource);
	info = (BITMAPINFO*) data;

	count = info->bmiHeader.biClrUsed;
	if (count == 0) {
		count = 1 << info->bmiHeader.biBitCount;
	}

	bits = (MechU8*) &info->bmiColors[count];
	memory = GlobalAlloc(GHND, sizeof(LOGPALETTE) + (count - 1) * sizeof(PALETTEENTRY));
	palette = (LOGPALETTE*) GlobalLock(memory);
	palette->palVersion = 0x300;
	palette->palNumEntries = count;

	dc = GetDC(NULL);
	GetSystemPaletteEntries(dc, 0, 10, palette->palPalEntry);
	GetSystemPaletteEntries(dc, 246, 10, &palette->palPalEntry[246]);
	ReleaseDC(NULL, dc);

	for (i = 10; i < count - 9; i++) {
		palette->palPalEntry[i].peRed = ((RGBQUAD*) info)[i].rgbRed;
		palette->palPalEntry[i].peGreen = ((RGBQUAD*) info)[i].rgbGreen;
		palette->palPalEntry[i].peBlue = ((RGBQUAD*) info)[i].rgbBlue;
		palette->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
	}

	*p_palette = CreatePalette(palette);
	GlobalUnlock(memory);
	GlobalFree(memory);
}

// STUB: NETMECHW 0x10004be3
void FUN_10004be3(WPARAM, LPARAM)
{
	STUB(0x10004be3);
}

// STUB: NETMECHW 0x10005023
void FUN_10005023(MechS32)
{
	STUB(0x10005023);
}

// Empties the player table and forgets the players' mech copies.
// FUNCTION: NETMECHW 0x100057ba
void FUN_100057ba()
{
	MechS32 i;

	for (i = 0; i < 8; i++) {
		g_unk0x1001ce18[i] = 0xff;
	}

	FUN_1000b0c8();
}

// Makes room in the game options for the player p_player, who took the slot p_index: the team
// and ready bits and the mech files of the slots from p_index up move up a slot.
// The new team bit is set only for a player on team 1 when no slot above p_index is on team 1,
// and the bits above stay in place.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes; reccmp also names
// the addresses one element before g_unk0x1001ca90's m_mechs and g_unk0x1001ce18 differently.
// FUNCTION: NETMECHW 0x100057fa
void FUN_100057fa(NetPlayer p_player, MechS32 p_index)
{
	MechU8 high;
	MechS32 i;
	MechU8 low;

	EnterCriticalSection(&g_unk0x1001ee60);

	high = g_unk0x1001ca90.m_settings.m_unk0x01 >> p_index;
	low = g_unk0x1001ca90.m_settings.m_unk0x01 & ((1 << p_index) - 1);
	high = (p_player.m_team == 0 | high & 0xfe) ? 0 : 1;
	g_unk0x1001ca90.m_settings.m_unk0x01 = (high << p_index) | low;

	high = g_unk0x1001ca90.m_settings.m_unk0x02 >> p_index;
	low = g_unk0x1001ca90.m_settings.m_unk0x02 & ((1 << p_index) - 1);
	high &= 0xfe;
	g_unk0x1001ca90.m_settings.m_unk0x02 = (high << p_index) | low;

	for (i = FUN_1000b0b3() - 1; i > p_index; i--) {
		strncpy(g_unk0x1001ca90.m_settings.m_mechs[i], g_unk0x1001ca90.m_settings.m_mechs[i - 1], 8);
		g_unk0x1001ce18[i] = g_unk0x1001ce18[i - 1];
	}

	LeaveCriticalSection(&g_unk0x1001ee60);
}

// Removes the slot p_index from the game options: the team and ready bits and the mech files of
// the slots above it move down a slot.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10005923
void FUN_10005923(MechS32 p_index)
{
	MechU8 high;
	MechS32 i;
	MechU8 low;

	EnterCriticalSection(&g_unk0x1001ee60);

	high = g_unk0x1001ca90.m_settings.m_unk0x01 >> (p_index + 1);
	low = g_unk0x1001ca90.m_settings.m_unk0x01 & ((1 << p_index) - 1);
	g_unk0x1001ca90.m_settings.m_unk0x01 = (high << p_index) | low;

	high = g_unk0x1001ca90.m_settings.m_unk0x02 >> (p_index + 1);
	low = g_unk0x1001ca90.m_settings.m_unk0x02 & ((1 << p_index) - 1);
	g_unk0x1001ca90.m_settings.m_unk0x02 = (high << p_index) | low;

	for (i = p_index; i < FUN_1000b0b3(); i++) {
		strncpy(g_unk0x1001ca90.m_settings.m_mechs[i], g_unk0x1001ca90.m_settings.m_mechs[i + 1], 8);
		g_unk0x1001ce18[i] = g_unk0x1001ce18[i + 1];
	}

	strcpy(g_unk0x1001ca90.m_settings.m_mechs[i], "");
	g_unk0x1001ce18[i] = 0xff;

	LeaveCriticalSection(&g_unk0x1001ee60);
}

// Lets new players join the session; returns whether they couldn't already.
// FUNCTION: NETMECHW 0x10005a55
MechS32 FUN_10005a55()
{
	if (g_unk0x10023180) {
		return FALSE;
	}

	g_unk0x1001ca90.m_directPlay->EnableNewPlayers(TRUE);
	g_unk0x10023180 = TRUE;
	return TRUE;
}

// Stops new players from joining the session; returns whether they could.
// FUNCTION: NETMECHW 0x10005a9a
MechS32 FUN_10005a9a()
{
	if (!g_unk0x10023180) {
		return FALSE;
	}

	g_unk0x1001ca90.m_directPlay->EnableNewPlayers(FALSE);
	g_unk0x10023180 = FALSE;
	return TRUE;
}

// Creates the DirectPlay object for the selected service provider; returns whether it
// succeeded.
// FUNCTION: NETMECHW 0x10005adf
MechS32 FUN_10005adf()
{
	HRESULT result;

	if (g_unk0x1001ca90.m_directPlay) {
		g_unk0x1001ca90.m_directPlay->Close();
		g_unk0x1001ca90.m_directPlay->Release();
		g_unk0x1001ca90.m_directPlay = NULL;
	}

	result = DirectPlayCreate(g_unk0x1001ca90.m_unk0x04, &g_unk0x1001ca90.m_directPlay, NULL) & 0xfff;
	return result == 0 ? TRUE : FALSE;
}

// Fills p_ids with the players' DirectPlay IDs for the simulator: by slot, or in a team game
// team 1's players first, then team 0's; 0 for the rest.
// FUNCTION: NETMECHW 0x10005b5d
void FUN_10005b5d(DPID* p_ids)
{
	MechS32 count;
	MechS32 i;

	if (!(g_unk0x1001ca90.m_settings.m_unk0x00 & 2)) {
		for (i = 0; i < 8; i++) {
			if (IS_PLAYER_SLOT_USED(i)) {
				p_ids[i] = g_players[i].m_id;
			}
			else {
				p_ids[i] = 0;
			}
		}
	}
	else {
		count = 0;
		for (i = 0; i < 8; i++) {
			if (IS_PLAYER_SLOT_USED(i) && g_players[i].m_team == 1) {
				p_ids[count++] = g_players[i].m_id;
			}
		}

		for (i = 0; i < 8; i++) {
			if (IS_PLAYER_SLOT_USED(i) && g_players[i].m_team == 0) {
				p_ids[count++] = g_players[i].m_id;
			}
		}

		for (i = count; i < 8; i++) {
			p_ids[i] = 0;
		}
	}
}

// Reads MechWarrior 2's callsign from the registry into p_name, of *p_size bytes; returns whether
// it is set.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10005cdc
MechS32 FUN_10005cdc(MechChar* p_name, DWORD* p_size)
{
	LONG result;
	HKEY key;
	DWORD type;

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Activision\\MechWarrior 2\\1.0", 0, KEY_QUERY_VALUE, &key);
	if (result != ERROR_SUCCESS) {
		return FALSE;
	}

	result = RegQueryValueEx(key, "Callsign", NULL, &type, (LPBYTE) p_name, p_size);
	RegCloseKey(key);

	if (result == ERROR_SUCCESS && (strncmp(p_name, "", *p_size) || strncmp(p_name, " ", *p_size))) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// Writes the callsign p_name, of p_size bytes, to MechWarrior 2's registry key; returns whether it
// succeeded.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10005d9e
MechS32 FUN_10005d9e(MechChar* p_name, DWORD p_size)
{
	LONG result;
	DWORD disposition;
	HKEY key;
	DWORD type;

	result = RegCreateKeyEx(
		HKEY_LOCAL_MACHINE,
		"SOFTWARE\\Activision\\MechWarrior 2\\1.0",
		0,
		"",
		REG_OPTION_NON_VOLATILE,
		KEY_SET_VALUE,
		NULL,
		&key,
		&disposition
	);
	if (result != ERROR_SUCCESS) {
		return FALSE;
	}

	type = REG_SZ;
	result = RegSetValueEx(key, "Callsign", 0, type, (LPBYTE) p_name, p_size);
	RegCloseKey(key);

	if (result == ERROR_SUCCESS) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// FUNCTION: NETMECHW 0x10005e36
void FUN_10005e36()
{
	g_unk0x1002317c = 0;
}

// Puts the path of the help file in p_path, of p_size bytes: mw2help.hlp in the current
// directory, else on the first CD-ROM drive that has it. When the current directory has it, the
// original closes a find handle it never set. The original's frame has room for one more local.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10005e50
void FUN_10005e50(MechChar* p_path, MechS32 p_size)
{
	MechChar* drive;
	WIN32_FIND_DATA data;
	MechChar* drives;
	HANDLE find;
	MechChar path[0x100];
	MechS32 unused;

	strncpy(p_path, "mw2help.hlp", p_size);
	if (FindFirstFile("mw2help.hlp", &data) != INVALID_HANDLE_VALUE) {
		FindClose(find);
	}
	else {
		drives = (MechChar*) calloc(0x69, 1);
		GetLogicalDriveStrings(0x69, drives);
		sprintf(path, " :\\%s", "mw2help.hlp");

		drive = drives;
		while (*drive) {
			if (GetDriveType(drive) == DRIVE_CDROM) {
				path[0] = *drive;
				find = FindFirstFile(path, &data);
				if (find != INVALID_HANDLE_VALUE) {
					FindClose(find);
					strncpy(p_path, path, p_size);
					free(drives);
					return;
				}
			}

			drive += 4;
		}

		free(drives);
	}
}
