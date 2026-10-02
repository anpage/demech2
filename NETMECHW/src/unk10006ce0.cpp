#include "unk10006ce0.h"

#include "decomp.h"
#include "sessionlist.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10002140.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000b400.h"
#include "unk1000f8e0.h"
#include "unk10010460.h"
#include "unk10010e50.h"
#include "unk10011030.h"

#include <dplay.h>
#include <string.h>
#include <windows.h>

// The session dialog's timer, which refreshes the selected session's players.
// GLOBAL: NETMECHW 0x10023368
UINT g_unk0x10023368 = 0;

// Creates the session, as its host, and the local player in it, and starts the host's threads.
// Returns whether it succeeded; tells the user why not otherwise.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10006ce0
MechS32 HostSession(HWND p_dialog)
{
	HANDLE event;
	MechChar unused[DPSHORTNAMELEN];
	MechS32 failed;
	MechChar name[DPSHORTNAMELEN + 1];

	event = NULL;
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
		MessageBox(p_dialog, LoadResString(0x81), LoadResString(0x79), MB_OK);
		SetBusyCursor(FALSE);
		return FALSE;
	}

	FUN_100057ba();
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

	g_unk0x10023128 = CreateThread(NULL, 0, ReceiveThread, HostMessageHandler, 0, &g_unk0x1001ce20);
	g_unk0x1002312c = CreateThread(NULL, 0, BroadcastSettingsThread, HostMessageHandler, 0, &g_unk0x1001ce14);
	g_unk0x10023130 = CreateThread(NULL, 0, WatchPlayersThread, NULL, 0, &g_unk0x1001ee54);

	g_unk0x1001ca90.m_isHost = TRUE;
	g_unk0x1001ca90.m_hostId = g_unk0x1001ca90.m_playerId;
	SetBusyCursor(FALSE);
	return TRUE;
}

// Joins the session selected in the list box (control 0x418) of the dialog p_dialog, creates the
// local player in it, and starts the client's threads. Returns whether it succeeded; tells the
// user why not otherwise.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10006f81
MechS32 JoinSession(HWND p_dialog)
{
	HANDLE event;
	SessionList::Session session;
	MechChar unused[DPSHORTNAMELEN];
	HRESULT result;
	DPSESSIONDESC desc;
	HWND listBox;
	MechChar sessionName[32];
	LRESULT index;
	MechChar name[DPSHORTNAMELEN + 1];

	event = NULL;
	listBox = GetDlgItem(p_dialog, 0x418);
	SetBusyCursor(TRUE);

	// Clears the lobby's session description, though it opens the session with desc.
	memset(&g_unk0x1001ca90.m_sessionDesc, 0, sizeof(g_unk0x1001ca90.m_sessionDesc));
	desc.dwSize = sizeof(desc);
	desc.dwMaxPlayers = 8;
	desc.dwFlags = DPOPEN_OPENSESSION;
	desc.guidSession = g_unk0x100230f8;
	strcpy(desc.szPassword, "");
	strcpy(desc.szSessionName, g_unk0x1001ca90.m_playerName);

	index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
	SendMessage(listBox, LB_GETTEXT, index, (LPARAM) sessionName);

	if (g_sessionList->Find(sessionName, &session)) {
		desc.dwSession = session.m_desc.dwSession;
		result = g_unk0x1001ca90.m_directPlay->Open(&desc) & 0xfff;
		if (result) {
			MessageBox(p_dialog, LoadResString(0x82), LoadResString(0x79), MB_OK);
			SetBusyCursor(FALSE);
			return FALSE;
		}
	}
	else {
		MessageBox(p_dialog, "Can't find session", LoadResString(0x79), MB_OK);
		SetBusyCursor(FALSE);
		return FALSE;
	}

	FUN_100057ba();
	result = g_unk0x1001ca90.m_directPlay->EnumPlayers(desc.dwSession, AddSessionPlayer, NULL, DPENUMPLAYERS_REMOTE);
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

	g_unk0x10023128 = CreateThread(NULL, 0, ReceiveThread, ClientMessageHandler, 0, &g_unk0x1001ce20);
	g_unk0x10023130 = CreateThread(NULL, 0, WatchPlayersThread, NULL, 0, &g_unk0x1001ee54);

	SetBusyCursor(FALSE);
	return TRUE;
}

// The session dialog: the sessions found, and the buttons to join one or host a new one (or,
// for a modem, to dial or to answer).
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100072a2
BOOL CALLBACK FUN_100072a2(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM)
{
	LRESULT selection;
	MechU32 notify;
	MechU32 command;
	MechS32 hosted;

	switch (p_message) {
	case WM_INITDIALOG:
		EnableWindow(GetDlgItem(p_dialog, IDOK), FALSE);
		return TRUE;
	case WM_SHOWWINDOW:
		if (p_wParam) {
			FUN_10007b30(p_dialog);
			if (!g_sessionList) {
				g_sessionList = new SessionList(GetDlgItem(p_dialog, 0x418));
			}

			SetDlgItemText(p_dialog, IDOK, g_unk0x1001ca90.m_unk0x00 == 2 ? LoadResString(0x8a) : LoadResString(0x89));
			EnableWindow(GetDlgItem(p_dialog, IDOK), FALSE);
			SendMessage(GetDlgItem(p_dialog, 0x42b), LB_RESETCONTENT, 0, 0);
			g_sessionList->Restart();
			g_unk0x10023368 = SetTimer(p_dialog, 1, 500, NULL);
		}
		else {
			FUN_10007bce(p_dialog);
			if (g_unk0x10023368) {
				KillTimer(p_dialog, g_unk0x10023368);
				g_unk0x10023368 = 0;
			}

			if (g_sessionList) {
				g_sessionList->StopThread();
				delete g_sessionList;
				g_sessionList = NULL;
			}
		}

		return FALSE;
	case WM_DESTROY:
		if (g_unk0x10023368) {
			KillTimer(p_dialog, g_unk0x10023368);
			g_unk0x10023368 = 0;
		}

		if (g_sessionList) {
			g_sessionList->StopThread();
			delete g_sessionList;
			g_sessionList = NULL;
		}

		return FALSE;
	case WM_TIMER:
		g_sessionList->ShowPlayers(p_dialog);
		break;
	case WM_HOTKEY:
		switch (p_wParam) {
		case 200:
			SetFocus(GetDlgItem(p_dialog, 0x418));
			break;
		case 202:
			SendMessage(p_dialog, WM_COMMAND, IDOK, 0);
			break;
		case 201:
			SendMessage(p_dialog, WM_COMMAND, 0x420, 0);
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
		switch (command) {
		case IDOK:
			switch (g_unk0x1001ca90.m_unk0x00) {
			case 3:
				if (JoinSession(p_dialog)) {
					FUN_10005023(3);
				}

				break;
			case 2:
				if (FUN_1000f8e0(p_dialog)) {
					FUN_10005023(3);
				}

				break;
			case 0:
			case 4:
			case 5:
			case 6:
				MessageBeep((UINT) -1);
				break;
			}

			break;
		case 0x7e0:
			if (g_unk0x1001ca90.m_unk0x00 == 2) {
				FUN_10005e50(g_unk0x1001cbf0, sizeof(g_unk0x1001cbf0));
				WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 2);
			}
			else {
				FUN_10005e50(g_unk0x1001cbf0, sizeof(g_unk0x1001cbf0));
				WinHelp(p_dialog, g_unk0x1001cbf0, HELP_CONTEXT, 3);
			}

			break;
		case 0x43b:
			if (g_unk0x10023368) {
				KillTimer(p_dialog, g_unk0x10023368);
				g_unk0x10023368 = 0;
			}

			FUN_100042ab();
			break;
		case 0x420:
			if (!CdCheck()) {
				MessageBox(p_dialog, LoadResString(0x86), LoadResString(0x79), MB_OK);
				break;
			}

			hosted = TRUE;
			switch (g_unk0x1001ca90.m_unk0x00) {
			case 3:
				hosted = HostSession(p_dialog);
				if (hosted) {
					if (g_unk0x10023368) {
						KillTimer(p_dialog, g_unk0x10023368);
						g_unk0x10023368 = 0;
					}

					FUN_10005023(2);
				}

				break;
			case 2:
				if (FUN_1000fca7(p_dialog)) {
					FUN_10005023(2);
				}

				break;
			case 0:
			case 4:
			case 5:
			case 6:
				MessageBeep((UINT) -1);
				break;
			}

			break;
		case 0x432:
			g_unk0x1001ca90.m_unk0xff = 1;
			if (g_unk0x10023368) {
				KillTimer(p_dialog, g_unk0x10023368);
				g_unk0x10023368 = 0;
			}

			FUN_10005023(0);
			break;
		case 0x418:
			if (notify == LBN_SELCHANGE) {
				if (SendMessage(GetDlgItem(p_dialog, 0x418), LB_GETCOUNT, 0, 0)) {
					selection = SendMessage(GetDlgItem(p_dialog, 0x418), LB_GETCURSEL, 0, 0);
					EnableWindow(GetDlgItem(p_dialog, IDOK), selection != LB_ERR);
					if (selection == LB_ERR) {
						SendMessage(GetDlgItem(p_dialog, 0x42b), LB_RESETCONTENT, 0, 0);
					}
				}

				return FALSE;
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

// FUNCTION: NETMECHW 0x10007add
BOOL FAR PASCAL AddSessionPlayer(DPID p_id, LPSTR p_friendlyName, LPSTR, DWORD, LPVOID)
{
	MechChar name[DPSHORTNAMELEN + 1];

	strcpy(name, p_friendlyName);
	name[DPSHORTNAMELEN] = '\0';
	FUN_1000b400(p_id, name);
	return TRUE;
}

// FUNCTION: NETMECHW 0x10007b30
void FUN_10007b30(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 200, MOD_ALT, 'G');

	if (g_unk0x1001ca90.m_unk0x00 == 2) {
		RegisterHotKey(p_hWnd, 202, MOD_ALT, 'D');
	}
	else {
		RegisterHotKey(p_hWnd, 202, MOD_ALT, 'J');
	}

	RegisterHotKey(p_hWnd, 201, MOD_ALT, 'O');
	RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x10007bce
void FUN_10007bce(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 200);
	UnregisterHotKey(p_hWnd, 202);
	UnregisterHotKey(p_hWnd, 201);
	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
