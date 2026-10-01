#include "unk10006ce0.h"

#include "decomp.h"
#include "sessionlist.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10002140.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000b400.h"
#include "unk10010e50.h"
#include "unk10011030.h"

#include <dplay.h>
#include <string.h>
#include <windows.h>

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

// The EnumPlayers callback of JoinSession: adds the session's players to the player table.
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
