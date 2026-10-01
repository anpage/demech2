#include "unk1000f8e0.h"

#include "decomp.h"
#include "sessionlist.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10002140.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk10006ce0.h"
#include "unk1000aa90.h"
#include "unk1000b400.h"
#include "unk10010e50.h"
#include "unk10011030.h"

#include <dplay.h>
#include <string.h>
#include <windows.h>

// The session of a modem connection: the host waits for the call in a dialog, while a thread
// (FUN_1000ffcf) watches DirectPlay's system messages; the client dials the host.

// The thread of FUN_1000ffcf, while the host waits for a call.
// GLOBAL: NETMECHW 0x10023800
HANDLE g_unk0x10023800 = NULL;

// GLOBAL: NETMECHW 0x1001f100
DWORD g_unk0x1001f100;

DWORD WINAPI FUN_1000ffcf(LPVOID p_dialog);
BOOL CALLBACK FUN_1001022a(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam);
BOOL CALLBACK FUN_1001032a(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam);
MechS32 FUN_100103a7(HWND p_parent);
void FUN_100103ea(HWND p_dialog, MechS32 p_result);

// Dials the host of the session selected in the list box (control 0x418) of the dialog
// p_dialog, joins the session and creates the local player in it, and starts the client's
// threads. Returns whether it succeeded; tells the user why not otherwise.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000f8e0
MechS32 FUN_1000f8e0(HWND p_dialog)
{
	HANDLE event;
	SessionList::Session session;
	MechChar unused[DPSHORTNAMELEN];
	HWND dialog;
	HRESULT result;
	DPSESSIONDESC desc;
	HWND listBox;
	MechChar sessionName[32];
	LRESULT index;
	NetPlayer player;
	MechChar name[DPSHORTNAMELEN + 1];

	event = NULL;
	listBox = GetDlgItem(p_dialog, 0x418);
	SetBusyCursor(TRUE);
	dialog = CreateDialogParam(g_hInstance, MAKEINTRESOURCE(0x71), p_dialog, (DLGPROC) FUN_1001032a, 0);

	memset(&g_unk0x1001ca90.m_sessionDesc, 0, sizeof(g_unk0x1001ca90.m_sessionDesc));
	desc.dwSize = sizeof(desc);
	desc.dwMaxPlayers = 8;
	desc.dwFlags = DPOPEN_OPENSESSION;
	desc.guidSession = g_unk0x100230f8;
	strcpy(desc.szPassword, "");
	strcpy(desc.szSessionName, g_unk0x1001ca90.m_playerName);

	index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
	g_unk0x1001ee58 = SendMessage(listBox, LB_GETITEMDATA, index, 0);
	index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
	SendMessage(listBox, LB_GETTEXT, index, (LPARAM) sessionName);

	if (g_sessionList->Find(sessionName, &session)) {
		desc.dwSession = session.m_desc.dwSession;
		result = g_unk0x1001ca90.m_directPlay->Open(&desc) & 0xfff;
		if (result) {
			if (result != DPERR_USERCANCEL) {
				SetBusyCursor(FALSE);
				MessageBox(p_dialog, LoadResString(0x82), LoadResString(0x79), MB_OK);
			}

			EndDialog(dialog, 1);
			return FALSE;
		}
	}

	EndDialog(dialog, 1);
	g_unk0x10023128 = CreateThread(NULL, 0, ReceiveThread, ClientMessageHandler, 0, &g_unk0x1001ce20);
	g_unk0x10023130 = CreateThread(NULL, 0, WatchPlayersThread, NULL, 0, &g_unk0x1001ee54);
	Sleep(2000);

	g_unk0x1001ca90.m_directPlay->EnumPlayers(desc.dwSession, AddSessionPlayer, NULL, DPENUMPLAYERS_REMOTE);
	FUN_1000afa8(0, &player);
	g_unk0x1001ca90.m_directPlay->SaveSession("Last Number Dialed");
	strncpy(unused, g_unk0x1001ca90.m_playerName, sizeof(unused));

	if (g_unk0x1001ca90.m_directPlay->CreatePlayer(
			&g_unk0x1001ca90.m_playerId,
			g_unk0x1001ca90.m_playerName,
			g_unk0x1001ca90.m_playerName,
			&event
		)) {
		MessageBox(p_dialog, LoadResString(0x83), LoadResString(0x79), MB_OK);
		g_unk0x1001ca90.m_directPlay->Close();
		SetBusyCursor(FALSE);
		return FALSE;
	}

	strcpy(name, g_unk0x1001ca90.m_playerName);
	name[DPSHORTNAMELEN] = '\0';
	FUN_1000b400(g_unk0x1001ca90.m_playerId, name);
	SetBusyCursor(FALSE);
	return TRUE;
}

// Starts the client's receive thread.
// FUNCTION: NETMECHW 0x1000fc72
MechS32 FUN_1000fc72()
{
	g_unk0x10023128 = CreateThread(NULL, 0, ReceiveThread, ClientMessageHandler, 0, &g_unk0x1001ce20);
	return TRUE;
}

// Creates the session of a modem connection, as its host, waits for the call, creates the local
// player in it and starts the host's threads. Returns whether it succeeded; tells the user why
// not otherwise.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000fca7
MechS32 FUN_1000fca7(HWND p_dialog)
{
	HANDLE event;
	MechChar unused[DPSHORTNAMELEN];
	MechS32 failed;
	MechChar name[DPSHORTNAMELEN + 1];

	SetBusyCursor(TRUE);

	memset(&g_unk0x1001ca90.m_sessionDesc, 0, sizeof(g_unk0x1001ca90.m_sessionDesc));
	g_unk0x1001ca90.m_sessionDesc.dwSize = sizeof(g_unk0x1001ca90.m_sessionDesc);
	g_unk0x1001ca90.m_sessionDesc.dwMaxPlayers = 8;
	g_unk0x1001ca90.m_sessionDesc.dwFlags = DPOPEN_CREATESESSION;
	g_unk0x1001ca90.m_sessionDesc.guidSession = g_unk0x100230f8;
	strcpy(g_unk0x1001ca90.m_sessionDesc.szPassword, "");
	strcpy(g_unk0x1001ca90.m_sessionDesc.szSessionName, g_unk0x1001ca90.m_playerName);

	if (g_unk0x1001ca90.m_directPlay->Open(&g_unk0x1001ca90.m_sessionDesc)) {
		failed = TRUE;
	}
	else {
		failed = FALSE;
	}

	if (failed) {
		SetBusyCursor(FALSE);
		MessageBox(p_dialog, LoadResString(0x81), LoadResString(0x79), MB_OK);
		return FALSE;
	}

	if (!FUN_100103a7(p_dialog)) {
		SetBusyCursor(FALSE);
		return FALSE;
	}

	g_unk0x10023128 = CreateThread(NULL, 0, ReceiveThread, HostMessageHandler, 0, &g_unk0x1001ce20);
	FUN_10005a55();
	strncpy(unused, g_unk0x1001ca90.m_playerName, sizeof(unused));

	if (g_unk0x1001ca90.m_directPlay->CreatePlayer(
			&g_unk0x1001ca90.m_playerId,
			g_unk0x1001ca90.m_playerName,
			g_unk0x1001ca90.m_playerName,
			&event
		)) {
		MessageBox(p_dialog, LoadResString(0x83), LoadResString(0x79), MB_OK);
		g_unk0x1001ca90.m_directPlay->Close();
		SetBusyCursor(FALSE);
		return FALSE;
	}

	strcpy(name, g_unk0x1001ca90.m_playerName);
	name[DPSHORTNAMELEN] = '\0';
	FUN_1000b400(g_unk0x1001ca90.m_playerId, name);

	g_unk0x1002312c = CreateThread(NULL, 0, BroadcastSettingsThread, HostMessageHandler, 0, &g_unk0x1001ce14);
	g_unk0x1001ca90.m_isHost = TRUE;
	g_unk0x1001ca90.m_hostId = g_unk0x1001ca90.m_playerId;
	SetBusyCursor(FALSE);
	g_unk0x10023130 = CreateThread(NULL, 0, WatchPlayersThread, NULL, 0, &g_unk0x1001ee54);
	return TRUE;
}

// Starts the host's receive and settings broadcast threads, and lets players join.
// FUNCTION: NETMECHW 0x1000ff61
MechS32 FUN_1000ff61()
{
	g_unk0x10023128 = CreateThread(NULL, 0, ReceiveThread, HostMessageHandler, 0, &g_unk0x1001ce20);
	FUN_10005a55();
	g_unk0x1002312c = CreateThread(NULL, 0, BroadcastSettingsThread, HostMessageHandler, 0, &g_unk0x1001ce14);
	g_unk0x1001ca90.m_isHost = TRUE;
	g_unk0x1001ca90.m_hostId = g_unk0x1001ca90.m_playerId;
	return TRUE;
}

// While the host waits for a call, passes DirectPlay's system messages on to the waiting dialog
// p_dialog as messages 0x400 to 0x406, until it receives message 0x418.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000ffcf
DWORD WINAPI FUN_1000ffcf(LPVOID p_dialog)
{
	MSG msg;
	DWORD size;
	HWND dialog;
	DPID to;
	MechU8 buffer[0x100];
	MechS32 running;
	DPMSG_GENERIC* message;
	DPID from;
	HRESULT result;

	running = TRUE;
	size = 0x80;
	dialog = (HWND) p_dialog;
	while (running) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == 0x418) {
				running = FALSE;
			}
		}
		else {
			result = g_unk0x1001ca90.m_directPlay->Receive(&from, &to, DPRECEIVE_ALL, buffer, &size);
			switch (result) {
			case DP_OK:
				if (from == 0) {
					message = (DPMSG_GENERIC*) buffer;
					switch (message->dwType) {
					case DPSYS_ADDPLAYER:
						PostMessage(dialog, 0x400, 0, 0);
						break;
					case DPSYS_DELETEPLAYER:
						PostMessage(dialog, 0x401, 0, 0);
						break;
					case DPSYS_ADDPLAYERTOGROUP:
						PostMessage(dialog, 0x402, 0, 0);
						break;
					case DPSYS_INVITE:
						PostMessage(dialog, 0x403, 0, 0);
						break;
					case DPSYS_DELETEGROUP:
						PostMessage(dialog, 0x404, 0, 0);
						break;
					case DPSYS_DELETEPLAYERFROMGRP:
						PostMessage(dialog, 0x405, 0, 0);
						break;
					case DPSYS_CONNECT:
						PostMessage(dialog, 0x406, 0, 0);
						break;
					default:
						break;
					}

					break;
				}
				else {
				}
			case DPERR_NOMESSAGES:
				Sleep(50);
				break;
			default:
				Sleep(50);
				break;
			}
		}
	}

	return 0;
}

// The dialog the host waits for a call in: ends with 1 when a player connects (message 0x406
// from FUN_1000ffcf), with 0 when the user cancels.
// FUNCTION: NETMECHW 0x1001022a
BOOL CALLBACK FUN_1001022a(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM)
{
	UINT id;

	switch (p_message) {
	case WM_INITDIALOG:
		g_unk0x10023800 = CreateThread(NULL, 0, FUN_1000ffcf, p_dialog, 0, &g_unk0x1001f100);
		CenterWindow(p_dialog);
		SetDlgItemText(p_dialog, 0x3f6, "Waiting for incoming call...");
		return TRUE;
	case 0x406:
		FUN_100103ea(p_dialog, 1);
		return TRUE;
	case WM_COMMAND:
		id = LOWORD(p_wParam);
		switch (id) {
		case IDCANCEL:
			FUN_100103ea(p_dialog, 0);
			return TRUE;
			break;
		}

		break;
	}

	return FALSE;
}

// The dialog the client shows while it dials the host.
// FUNCTION: NETMECHW 0x1001032a
BOOL CALLBACK FUN_1001032a(HWND p_dialog, UINT p_message, WPARAM, LPARAM)
{
	switch (p_message) {
	case WM_INITDIALOG:
		CenterWindow(p_dialog);
		SetDlgItemText(p_dialog, 0x3f6, "Dialing host player...");
		ShowWindow(GetDlgItem(p_dialog, IDCANCEL), SW_HIDE);
		return TRUE;
		break;
	}

	return FALSE;
}

// Waits for a call in the dialog of FUN_1001022a; returns whether a player connected.
// FUNCTION: NETMECHW 0x100103a7
MechS32 FUN_100103a7(HWND p_parent)
{
	MechS32 result;

	result = DialogBoxParam(g_hInstance, MAKEINTRESOURCE(0x71), p_parent, (DLGPROC) FUN_1001022a, 0);
	if (result == -1) {
		result = 0;
	}

	return result;
}

// Stops the thread of FUN_1000ffcf and ends the waiting dialog p_dialog with p_result.
// FUNCTION: NETMECHW 0x100103ea
void FUN_100103ea(HWND p_dialog, MechS32 p_result)
{
	DWORD wait;

	if (g_unk0x10023800) {
		PostThreadMessage(g_unk0x1001f100, 0x418, 0, 0);
		wait = WaitForSingleObject(g_unk0x10023800, INFINITE);
		CloseHandle(g_unk0x10023800);
		g_unk0x10023800 = NULL;
	}

	EndDialog(p_dialog, p_result);
}
