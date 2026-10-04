#include "unk10003660.h"

#include "bwdwriter.h"
#include "chatlog.h"
#include "decomp.h"
#include "mw2prj.h"
#include "netlaunchinfo.h"
#include "sessionlist.h"
#include "types.h"
#include "unk10001070.h"
#include "unk100025f0.h"
#include "unk10006060.h"
#include "unk10006b20.h"
#include "unk10006ce0.h"
#include "unk10007c30.h"
#include "unk100089d0.h"
#include "unk10009230.h"
#include "unk1000aa90.h"
#include "unk1000b400.h"
#include "unk1000d250.h"
#include "unk1000e410.h"
#include "unk1000f0f0.h"
#include "unk1000f8e0.h"
#include "unk10010460.h"
#include "unk10011120.h"

#include <commctrl.h>
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

// The path of the help file (FUN_10005e50).
// GLOBAL: NETMECHW 0x1001cbf0
MechChar g_unk0x1001cbf0[MAX_PATH];

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

// The lobby's bitmaps, loaded when its window is created (FUN_100042d9).
// GLOBAL: NETMECHW 0x1001ce24
HBITMAP g_unk0x1001ce24;

// GLOBAL: NETMECHW 0x1001ce28
HBITMAP g_unk0x1001ce28;

// Bitmap 105, whose colors make the lobby's palette (FUN_10004a40).
// GLOBAL: NETMECHW 0x1001ce30
HBITMAP g_unk0x1001ce30;

// GLOBAL: NETMECHW 0x1001ce34
HBITMAP g_unk0x1001ce34;

// GLOBAL: NETMECHW 0x1001ce38
HBITMAP g_unk0x1001ce38;

// GLOBAL: NETMECHW 0x1001ce3c
HBITMAP g_unk0x1001ce3c;

// The lobby window's one-second timer.
// GLOBAL: NETMECHW 0x1001ce40
UINT g_unk0x1001ce40;

// GLOBAL: NETMECHW 0x1001ce44
HBITMAP g_unk0x1001ce44;

// GLOBAL: NETMECHW 0x1001ce48
HBITMAP g_unk0x1001ce48;

// GLOBAL: NETMECHW 0x1001ce2c
HINSTANCE g_hInstance;

// The text of the About box (FUN_10004909).
// GLOBAL: NETMECHW 0x1001ce50
MechChar g_unk0x1001ce50[0x2000];

// GLOBAL: NETMECHW 0x1001ee50
HBITMAP g_unk0x1001ee50;

// GLOBAL: NETMECHW 0x1001ee54
DWORD g_unk0x1001ee54;

// The item data of the last session in the join dialog's list box (FUN_1000f8e0).
// GLOBAL: NETMECHW 0x1001ee58
LRESULT g_unk0x1001ee58;

// GLOBAL: NETMECHW 0x1001ee5c
HBITMAP g_unk0x1001ee5c;

// Guards the lobby's settings (g_unk0x1001ca90.m_settings).
// GLOBAL: NETMECHW 0x1001ee60
CRITICAL_SECTION g_unk0x1001ee60;

// The application GUID of NetMech's DirectPlay sessions.
// GLOBAL: NETMECHW 0x100230f8
GUID g_unk0x100230f8 = {0x528519a0, 0x1001, 0x11cf, {0x80, 0x5c, 0x00, 0xaa, 0x00, 0x44, 0x43, 0x1f}};

// The index in g_unk0x10023138 of the dialog that gets the lobby's dialog messages.
// GLOBAL: NETMECHW 0x10023108
MechS32 g_unk0x10023108 = -1;

// Set when the lobby window has been destroyed.
// GLOBAL: NETMECHW 0x1002310c
MechS32 g_unk0x1002310c = 1;

// GLOBAL: NETMECHW 0x10023110
HWND g_unk0x10023110 = NULL;

// The lobby's palette (FUN_10004a40).
// GLOBAL: NETMECHW 0x10023118
HPALETTE g_unk0x10023118 = NULL;

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

// Set when the lobby starts with the results of the last mission (flag 2 of the launch record).
// GLOBAL: NETMECHW 0x1002316c
MechS32 g_unk0x1002316c = 0;

// GLOBAL: NETMECHW 0x10023178
MechS32 g_unk0x10023178 = 0;

// The timer ticks since the simulator of a modem game's guest returned (FUN_100042d9).
// GLOBAL: NETMECHW 0x1002317c
MechU32 g_unk0x1002317c = 0;

// Whether new players may join the session (FUN_10005a55, FUN_10005a9a).
// GLOBAL: NETMECHW 0x10023180
MechS32 g_unk0x10023180 = 0;

// Set until the lobby has run once.
// GLOBAL: NETMECHW 0x10023184
MechS32 g_unk0x10023184 = 1;

// Set when a modem game's guest launched the simulator.
// GLOBAL: NETMECHW 0x10023188
MechS32 g_unk0x10023188 = 0;

// NetMech's lobby. MECH2.EXE calls it with the launch record to fill in, runs the simulator
// when it returns nonzero, and calls it again when the mission is over.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10003684
extern "C" MechS32 __stdcall Launcher(NetLaunchInfo* p_info)
{
	MechS32 done;
	MechS32 i;
	MSG msg;
	MechChar mission[80];
	CopperField0x4d::OptionFlags options;
	MechChar message[0x100];
	DWORD error;
	MechS32 unused;
	BOOL peeked;

	done = FALSE;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
	}

	InitializeCriticalSection(&g_unk0x1001ee60);
	InitializeCriticalSection(&g_unk0x1001ca78);
	g_unk0x1001ce10 = p_info;
	p_info->m_unk0x14 = 0;

	if (g_unk0x1001ce10->m_unk0x0c & 2) {
		FUN_10010920(g_unk0x1001ce10->m_playerIds);
	}

	if (g_unk0x10023184) {
		InitCommonControls();
		g_unk0x1001ca90.m_directPlay = g_unk0x1001ce10->m_directPlay;
		if (!FUN_10003e52(g_hInstance)) {
			DeleteCriticalSection(&g_unk0x1001ee60);
			DeleteCriticalSection(&g_unk0x1001ca78);
			return FALSE;
		}

		if (!GetCurrentDirectory(sizeof(g_unk0x1001ccf8), g_unk0x1001ccf8)) {
			error = GetLastError();
			sprintf(message, LoadResString(1), error);
			MessageBox(NULL, message, LoadResString(2), MB_OK);
			DeleteCriticalSection(&g_unk0x1001ee60);
			DeleteCriticalSection(&g_unk0x1001ca78);
			return FALSE;
		}
	}
	else if ((g_unk0x1001ce10->m_unk0x0c & 4) && g_unk0x1001ca90.m_unk0x00 != 2) {
		FUN_10005adf();
		g_unk0x1001ca90.m_settings.m_unk0x01 = 0;
		g_unk0x1001ca90.m_hostId = 0;
		g_unk0x1001ca90.m_isHost = FALSE;
		g_unk0x1001ca90.m_settings.m_unk0x49 = 100;
		g_unk0x1001ca90.m_settings.m_unk0x4a = 1;
		g_unk0x1001ca90.m_settings.m_unk0x4b = 0x10;
		g_unk0x1001ca90.m_settings.m_unk0x4c = 1;
		options.m_byte = 0;
		options.m_bits.m_option2 = 1;
		options.m_bits.m_option3 = 1;
		options.m_bits.m_option4 = 1;
		options.m_bits.m_option5 = 1;
		g_unk0x1001ca90.m_settings.m_options = options;
	}

	g_unk0x1001ca90.m_unk0x158 = 0;
	g_unk0x10023178 = 0;
	strcpy(g_unk0x1001ca90.m_mechFile, "frm00std");
	g_unk0x1001ca90.m_unk0x154 = 0;
	g_unk0x1001ca90.m_settings.m_unk0x02 = 0;
	g_unk0x1001ca90.m_settings.m_unk0x03 = 0;
	g_unk0x1001ca90.m_settings.m_unk0x00 = 0;
	for (i = 0; i < 8; i++) {
		strcpy(g_unk0x1001ca90.m_settings.m_mechs[i], "");
	}

	g_unk0x1001ca90.FUN_10003620();
	g_unk0x10023164 = CreateFont(11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, "");
	g_unk0x10023168 = CreateFont(24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, "");
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
		g_hInstance,
		NULL
	);
	while (ShowCursor(TRUE) < 1) {
	}

	if (!g_unk0x10023160) {
		DeleteObject(g_unk0x10023164);
		DeleteObject(g_unk0x10023168);
		g_unk0x10023164 = NULL;
		g_unk0x10023168 = NULL;
		DeleteCriticalSection(&g_unk0x1001ee60);
		DeleteCriticalSection(&g_unk0x1001ca78);
		return FALSE;
	}

	if ((g_unk0x1001ce10->m_unk0x0c & 4) && g_unk0x1001ca90.m_unk0x00 != 2) {
		FUN_100057ba();
	}

	if (!InitializeMw2Prj()) {
		MessageBox(NULL, "The resource file MW2.PRJ could not be found.", "File not found", MB_OK);
		DeleteCriticalSection(&g_unk0x1001ee60);
		DeleteCriticalSection(&g_unk0x1001ca78);
		return FALSE;
	}

	g_unk0x1001fe70 = FUN_10007c30(g_unk0x1001c318);
	if (g_unk0x1001ce10->m_unk0x0c & 2) {
		FUN_10005023(9);
		g_unk0x1002316c = TRUE;
	}
	else {
		FUN_10003cb7();
	}

	CenterWindow(g_unk0x10023160);
	ShowWindow(g_unk0x10023160, SW_SHOWNORMAL);
	UpdateWindow(g_unk0x10023160);

	while (!done) {
		unused = 0;
		peeked = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (peeked) {
			if (msg.message == 0x418) {
				done = TRUE;
			}
			else if (g_unk0x10023138[g_unk0x10023108] && !IsDialogMessage(g_unk0x10023138[g_unk0x10023108], &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	if (g_unk0x1001ca90.m_unk0x158) {
		if (g_unk0x1001ca90.m_settings.m_unk0x00 & 2) {
			FUN_1000f172();
		}
		else {
			FUN_1000f0f0();
		}

		options = g_unk0x1001ca90.m_settings.m_options;
		WriteNetDifficultyCfg(
			options.m_bits,
			g_unk0x1001ca90.m_settings.m_unk0x4a,
			g_unk0x1001ca90.m_settings.m_unk0x4b,
			g_unk0x1001ca90.m_settings.m_unk0x4c,
			g_unk0x1001ca90.m_settings.m_unk0x00 & 2
		);
	}

	ShutdownMw2Prj();
	if (g_unk0x1001ca90.m_unk0x158) {
		FUN_10004118();
		g_unk0x1001ce10->m_localPlayerId = g_unk0x1001ca90.m_playerId;
		memset(mission, 0, sizeof(mission));
		strncpy(mission, g_unk0x1001ca90.m_settings.m_unk0x44, 4);
		strcat(mission, "SCN1");
		strncpy(g_unk0x1001ce10->m_missionName, mission, p_info->m_missionNameSize);
		if (g_unk0x1001ca90.m_unk0x00 == 2 && !g_unk0x1001ca90.m_isHost) {
			g_unk0x10023188 = TRUE;
		}
	}
	else {
		FUN_100041f5();
	}

	g_unk0x1001ce10->m_directPlay = g_unk0x1001ca90.m_directPlay;
	FUN_10005b5d(g_unk0x1001ce10->m_playerIds);
	DeleteCriticalSection(&g_unk0x1001ee60);
	DeleteCriticalSection(&g_unk0x1001ca78);
	return g_unk0x1001ca90.m_unk0x158;
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

// The lobby window's procedure.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes, and the operand
// order of the WM_PALETTECHANGED comparison (the original loads p_wParam).
// FUNCTION: NETMECHW 0x100042d9
LRESULT CALLBACK FUN_100042d9(HWND p_hWnd, UINT p_message, WPARAM p_wParam, LPARAM p_lParam)
{
	MechU32 command;
	MechU32 notify;
	HMENU menu;
	MechChar text[0x100];
	HWND button;
	HDC dc;
	HPALETTE palette;
	UINT changed;
	MechS32 index;
	MechS32 index2;

	command = LOWORD(p_wParam);
	notify = HIWORD(p_wParam);

	switch (p_message) {
	case WM_CREATE:
		g_unk0x1001ce34 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0x38a),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		g_unk0x1001ce30 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0x69),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		g_unk0x1001ce38 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0x70),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		g_unk0x1001ce48 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0x38f),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		g_unk0x1001ce3c = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0x383),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		g_unk0x1001ee50 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0x38e),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		g_unk0x1001ce44 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0x72),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		g_unk0x1001ee5c = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0x71),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		g_unk0x1001ce24 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xd9),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		g_unk0x1001ce28 = (HBITMAP) LoadImage(
			g_hInstance,
			MAKEINTRESOURCE(0xd8),
			IMAGE_BITMAP,
			0,
			0,
			LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE
		);
		FUN_10004a40(&g_unk0x10023118);

		menu = GetSystemMenu(p_hWnd, FALSE);
		strcpy(text, LoadResString(3));
		AppendMenu(menu, MF_SEPARATOR, 0, NULL);
		AppendMenu(menu, MF_STRING, 2000, text);
		ShowWindow(p_hWnd, SW_HIDE);
		button = CreateWindowEx(0, "button", "", 0, 0, 0, 0, 0, p_hWnd, NULL, g_hInstance, NULL);
		g_unk0x1001ce40 = SetTimer(p_hWnd, 2, 1000, NULL);
		break;
	case WM_PALETTECHANGED:
		if (p_hWnd == (HWND) p_wParam) {
			break;
		}
	case WM_QUERYNEWPALETTE:
		dc = GetDC(p_hWnd);
		palette = SelectPalette(dc, g_unk0x10023118, FALSE);
		changed = RealizePalette(dc);
		if (changed) {
			InvalidateRect(p_hWnd, NULL, TRUE);
		}

		SelectPalette(dc, palette, TRUE);
		RealizePalette(dc);
		return changed;
	case WM_TIMER:
		if (g_unk0x10023178 && ++g_unk0x1002317c > 2) {
			g_unk0x10023178 = 0;
			g_unk0x1002317c = 0;
			index = g_unk0x10023108;
			if (g_unk0x10023138[index]) {
				MessageBox(g_unk0x10023138[index], LoadResString(0x7a), LoadResString(0x79), MB_OK);
				PostMessage(g_unk0x10023138[index], 0x417, 0, 0);
			}
		}

		break;
	case WM_HOTKEY:
		break;
	case WM_SETFOCUS:
		SetFocus(g_unk0x10023138[g_unk0x10023108]);
		break;
	case 0x419:
		g_chatLog.AddLine((DPID) p_wParam, (MechChar*) p_lParam);
		free((void*) p_lParam);
		break;
	case WM_DESTROY:
		g_unk0x10023178 = 0;
		KillTimer(p_hWnd, g_unk0x1001ce40);
		DeleteObject(g_unk0x1001ce34);
		DeleteObject(g_unk0x1001ce30);
		DeleteObject(g_unk0x1001ce38);
		DeleteObject(g_unk0x1001ce48);
		DeleteObject(g_unk0x1001ce3c);
		DeleteObject(g_unk0x1001ee50);
		DeleteObject(g_unk0x1001ce44);
		DeleteObject(g_unk0x1001ee5c);
		if (g_unk0x10023118) {
			DeleteObject(g_unk0x10023118);
			g_unk0x10023118 = NULL;
		}

		FUN_100042ab();
		g_unk0x1002310c = TRUE;
		break;
	case WM_SYSCOMMAND:
		if ((command & 0xfff0) == 2000) {
			DialogBoxParam(g_hInstance, "AboutBox", p_hWnd, (DLGPROC) FUN_10004909, 0);
		}

		if (p_wParam == SC_CLOSE) {
			if (FUN_100048ec()) {
				FUN_100042ab();
			}
		}
		else {
			return DefWindowProc(p_hWnd, p_message, p_wParam, p_lParam);
		}

		break;
	case 0x416:
		index2 = g_unk0x10023108;
		if (g_unk0x10023188) {
			g_unk0x10023178 = 1;
			g_unk0x10023188 = 0;
		}

		if (g_unk0x10023138[index2]) {
			PostMessage(g_unk0x10023138[index2], 0x416, 0, 0);
		}

		FUN_10005e36();
		break;
	default:
		return DefWindowProc(p_hWnd, p_message, p_wParam, p_lParam);
	}

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

// Draws the lobby's owner-drawn button p_wParam, as WM_DRAWITEM's p_lParam describes it: pressed
// while it is selected or while its pane is the current one (g_unk0x1001ca90.m_unk0x15c).
// The bitmap is the same whether the button has the focus or not.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10004be3
void FUN_10004be3(WPARAM p_wParam, LPARAM p_lParam)
{
	DRAWITEMSTRUCT* item;
	HDC dc;
	BITMAP info;
	UINT focus;
	WPARAM id;
	HGDIOBJ bitmap;

	item = (DRAWITEMSTRUCT*) p_lParam;
	id = p_wParam;
	dc = CreateCompatibleDC(item->hDC);
	focus = item->itemState & ODS_FOCUS;

	switch (id) {
	case 0x3e9:
		if ((item->itemState & ODS_SELECTED) || g_unk0x1001ca90.m_unk0x15c == 4 || g_unk0x1001ca90.m_unk0x15c == 5) {
			if (focus) {
				SelectObject(dc, g_unk0x1001ce30);
			}
			else {
				SelectObject(dc, g_unk0x1001ce30);
			}

			bitmap = g_unk0x1001ce30;
		}
		else {
			if (focus) {
				SelectObject(dc, g_unk0x1001ce34);
			}
			else {
				SelectObject(dc, g_unk0x1001ce34);
			}

			bitmap = g_unk0x1001ce34;
		}

		break;
	case 0x3ea:
		if ((item->itemState & ODS_SELECTED) || g_unk0x1001ca90.m_unk0x15c == 7) {
			if (focus) {
				SelectObject(dc, g_unk0x1001ce48);
			}
			else {
				SelectObject(dc, g_unk0x1001ce48);
			}

			bitmap = g_unk0x1001ce48;
		}
		else {
			if (focus) {
				SelectObject(dc, g_unk0x1001ce38);
			}
			else {
				SelectObject(dc, g_unk0x1001ce38);
			}

			bitmap = g_unk0x1001ce38;
		}

		break;
	case 0x3eb:
		if ((item->itemState & ODS_SELECTED) || g_unk0x1001ca90.m_unk0x15c == 6) {
			if (focus) {
				SelectObject(dc, g_unk0x1001ee50);
			}
			else {
				SelectObject(dc, g_unk0x1001ee50);
			}

			bitmap = g_unk0x1001ee50;
		}
		else {
			if (focus) {
				SelectObject(dc, g_unk0x1001ce3c);
			}
			else {
				SelectObject(dc, g_unk0x1001ce3c);
			}

			bitmap = g_unk0x1001ce3c;
		}

		break;
	case 0x431:
		if ((item->itemState & ODS_SELECTED) || g_unk0x1001ca90.m_unk0x15c == 8) {
			if (focus) {
				SelectObject(dc, g_unk0x1001ee5c);
			}
			else {
				SelectObject(dc, g_unk0x1001ee5c);
			}

			bitmap = g_unk0x1001ee5c;
		}
		else {
			if (focus) {
				SelectObject(dc, g_unk0x1001ce44);
			}
			else {
				SelectObject(dc, g_unk0x1001ce44);
			}

			bitmap = g_unk0x1001ce44;
		}

		break;
	case 0x435:
		if ((item->itemState & ODS_SELECTED)) {
			if (focus) {
				SelectObject(dc, g_unk0x1001ce28);
			}
			else {
				SelectObject(dc, g_unk0x1001ce28);
			}

			bitmap = g_unk0x1001ce28;
		}
		else {
			if (focus) {
				SelectObject(dc, g_unk0x1001ce24);
			}
			else {
				SelectObject(dc, g_unk0x1001ce24);
			}

			bitmap = g_unk0x1001ce24;
		}

		break;
	}

	if (g_unk0x10023118) {
		SelectPalette(item->hDC, g_unk0x10023118, FALSE);
		RealizePalette(item->hDC);
	}

	GetObject(bitmap, sizeof(info), &info);
	StretchBlt(
		item->hDC,
		item->rcItem.left,
		item->rcItem.top,
		item->rcItem.right - item->rcItem.left,
		item->rcItem.bottom - item->rcItem.top,
		dc,
		0,
		0,
		info.bmWidth,
		info.bmHeight,
		SRCCOPY
	);
	DeleteDC(dc);
}

// Shows the lobby's pane p_state, creating its dialog the first time; returns whether it could.
// Entering a pane does its setup: 1, the connection, starts the lobby over.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes, and the operand
// order of the first comparison (the original loads p_state), which also shifts the jump table.
// FUNCTION: NETMECHW 0x10005023
MechS32 FUN_10005023(MechS32 p_state)
{
	MechS32 result;
	MechS32 previous;
	CopperField0x4d::OptionFlags options;

	result = TRUE;
	if (g_unk0x10023108 != p_state) {
		previous = g_unk0x10023108;
		g_unk0x10023108 = p_state;
		g_chatLog.SaveTopIndex();
		g_chatLog.SaveInput();
		g_chatLog.Detach();

		switch (p_state) {
		case 0:
			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x46a), g_unk0x10023160, (DLGPROC) FUN_1000654c, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			if (result) {
				SetFocus(GetDlgItem(g_unk0x10023138[g_unk0x10023108], 0x3f7));
				g_unk0x10023178 = 0;
			}

			break;
		case 1:
			FUN_10003ec8();
			if (previous) {
				FUN_1000b54e(g_unk0x1001ca90.m_playerId);
				g_unk0x1001ca90.m_directPlay->DestroyPlayer(g_unk0x1001ca90.m_playerId);
				g_unk0x1001ca90.m_directPlay->Close();
				FUN_10005a9a();
			}
			else if (g_unk0x1001ca90.m_unk0x00 == 2) {
				g_unk0x1001ce10->m_unk0x14 |= 1;
			}

			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x46b), g_unk0x10023160, (DLGPROC) FUN_100072a2, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			if (result) {
				g_chatLog.Reset();
				SetFocus(GetDlgItem(g_unk0x10023138[g_unk0x10023108], 0x418));
				g_unk0x10023178 = 0;
				g_unk0x1001ca90.m_unk0x154 = 0;
				g_unk0x1001ca90.m_settings.m_unk0x02 = 0;
				g_unk0x1001ca90.m_settings.m_unk0x03 = 0;
				g_unk0x1001ca90.m_settings.m_unk0x00 = 0;
				options.m_byte = 0;
				options.m_bits.m_option2 = 1;
				options.m_bits.m_option3 = 1;
				options.m_bits.m_option4 = 1;
				options.m_bits.m_option5 = 1;
				g_unk0x1001ca90.m_settings.m_options = options;
				g_unk0x1001ca90.m_settings.m_unk0x01 = 0;
				g_unk0x1001ca90.m_hostId = 0;
				g_unk0x1001ca90.m_isHost = FALSE;
				g_unk0x1001ca90.FUN_10003620();
				g_unk0x1001ca90.m_unk0x158 = 0;
				g_unk0x1001ca90.m_unk0x15c = 0;
				FUN_100057ba();
			}

			break;
		case 2:
			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x44f), g_unk0x10023160, (DLGPROC) FUN_100098a2, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			if (result) {
				SetFocus(GetDlgItem(g_unk0x10023138[g_unk0x10023108], 0x41c));
				EnterCriticalSection(&g_unk0x1001ee60);
				g_unk0x1001ca90.m_settings.m_unk0x00 &= ~1;
				g_unk0x1001ca90.m_settings.m_unk0x02 = 0;
				LeaveCriticalSection(&g_unk0x1001ee60);
			}

			break;
		case 3:
			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x453), g_unk0x10023160, (DLGPROC) FUN_10008d5d, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			if (result) {
				SetFocus(GetDlgItem(g_unk0x10023138[g_unk0x10023108], 0x3f2));
				if (previous != 9 || g_unk0x1001ca90.m_unk0x00 != 2) {
					g_unk0x10023178 = 1;
				}
				else {
					g_unk0x10023178 = 0;
				}

				g_unk0x1002317c = 0;
			}

			break;
		case 4:
			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x468), g_unk0x10023160, (DLGPROC) FUN_1000d31f, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			if (result) {
				SetFocus(GetDlgItem(g_unk0x10023138[g_unk0x10023108], 0x3f2));
			}

			break;
		case 5:
			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x466), g_unk0x10023160, (DLGPROC) FUN_1000dae1, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			if (result) {
				SetFocus(GetDlgItem(g_unk0x10023138[g_unk0x10023108], 0x3f2));
			}

			break;
		case 6:
			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x46c), g_unk0x10023160, (DLGPROC) FUN_100113ec, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			if (result) {
				SetFocus(GetDlgItem(g_unk0x10023138[g_unk0x10023108], 0x3f2));
			}

			break;
		case 8:
			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x46d), g_unk0x10023160, (DLGPROC) FUN_1000e58d, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			if (result) {
				SetFocus(GetDlgItem(g_unk0x10023138[g_unk0x10023108], 0x3f2));
			}

			break;
		case 7:
			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x463), g_unk0x10023160, (DLGPROC) FUN_1000268a, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			if (result) {
				SetFocus(GetDlgItem(g_unk0x10023138[g_unk0x10023108], 0x3f2));
			}

			break;
		case 9:
			if (!g_unk0x10023138[g_unk0x10023108]) {
				g_unk0x10023138[g_unk0x10023108] =
					CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0xc8), g_unk0x10023160, (DLGPROC) FUN_100105ee, 0);
				if (!g_unk0x10023138[g_unk0x10023108]) {
					result = FALSE;
				}
			}

			break;
		default:
			g_unk0x10023108 = previous;
			return FALSE;
			break;
		}

		if (result) {
			if (g_unk0x10023138[previous]) {
				ShowWindow(g_unk0x10023138[previous], SW_HIDE);
			}

			FUN_1000120a(g_unk0x10023160, g_unk0x10023138[g_unk0x10023108], 0);
			ShowWindow(g_unk0x10023138[g_unk0x10023108], SW_SHOW);
		}
	}

	return result;
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
