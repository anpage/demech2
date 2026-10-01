#include "sessionlist.h"

#include "decomp.h"
#include "types.h"
#include "unk10003660.h"
#include "unk10006b20.h"

#include <dplay.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(SessionList, 0x30)
DECOMP_SIZE_ASSERT(SessionList::Session, 0x228)

// The number of players EnumPlayersCallback has stored in the session being enumerated.
// GLOBAL: NETMECHW 0x1002300c
MechS32 g_enumPlayerCount = 0;

DWORD WINAPI SessionThread(LPVOID);

// FUNCTION: NETMECHW 0x10001470
SessionList::SessionList(HWND p_listBox)
{
	m_unk0x00 = 0;
	m_count = 0;
	m_head = NULL;
	m_tail = NULL;
	m_thread = NULL;
	m_unk0x14 = 0;
	m_threadId = 0;
	m_listBox = p_listBox;
	m_mutex = CreateMutex(NULL, FALSE, "SessionMutex");
	m_stateMutex = CreateMutex(NULL, FALSE, "SessionFlagMutex");
	m_state = c_stateEnumerate;
}

// FUNCTION: NETMECHW 0x1000150d
SessionList::~SessionList()
{
	StopThread();
	Clear();
	ReleaseMutex(m_mutex);
	CloseHandle(m_mutex);
	ReleaseMutex(m_stateMutex);
	CloseHandle(m_stateMutex);
}

// Matches except for the stack slots of session and next, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10001567
void SessionList::Clear()
{
	Session* session;
	Session* next;

	session = m_head;
	while (session) {
		next = session->m_next;
		delete session;
		session = next;
	}

	m_head = NULL;
	m_tail = NULL;
}

// FUNCTION: NETMECHW 0x100015d3
void SessionList::MarkStale()
{
	Session* session;

	Lock();

	session = m_head;
	while (session) {
		session->m_stale = TRUE;
		session = session->m_next;
	}

	Unlock();
}

// Copies the session named p_name to p_session (if not NULL); returns whether it was found.
// Matches except for the stack slots of unused and found, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10001626
MechS32 SessionList::Find(MechChar* p_name, Session* p_session)
{
	MechS32 unused;
	MechS32 found;
	Session* session;

	unused = 0;
	found = FALSE;
	Lock();

	session = m_head;
	while (session) {
		if (!strcmp(session->m_desc.szSessionName, p_name)) {
			if (p_session) {
				*p_session = *session;
			}

			found = TRUE;
			break;
		}
		else {
			session = session->m_next;
		}
	}

	Unlock();
	return found;
}

// Matches except for the stack slots of unused and found, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100016f4
MechS32 SessionList::FindEntry(MechChar* p_name, Session** p_session)
{
	MechS32 unused;
	MechS32 found;
	Session* session;

	unused = 0;
	found = FALSE;
	Lock();

	session = m_head;
	while (session) {
		if (!strcmp(session->m_desc.szSessionName, p_name)) {
			*p_session = session;
			found = TRUE;
			break;
		}
		else {
			session = session->m_next;
		}
	}

	Unlock();
	return found;
}

// FUNCTION: NETMECHW 0x100017b3
MechS32 SessionList::Add(DPSESSIONDESC* p_desc)
{
	MechS32 result;
	Session* session;

	result = 0;
	Lock();

	session = new Session;
	if (session) {
		session->m_playersEnumerated = FALSE;
		session->m_desc = *p_desc;
		session->m_next = NULL;
		session->m_stale = FALSE;

		if (!m_head) {
			m_head = session;
			m_tail = session;
		}
		else {
			m_tail->m_next = session;
			m_tail = session;
		}

		m_count++;

		if (m_listBox) {
			SendMessage(m_listBox, LB_ADDSTRING, 0, (LPARAM) p_desc->szSessionName);
		}
	}

	Unlock();
	return result;
}

// Removes the sessions the last enumeration didn't find, from the list and the list box.
// Matches except for the stack slots of stale, prev, session and index, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000189c
void SessionList::RemoveStale()
{
	Session* stale;
	Session* prev;
	Session* session;
	WPARAM index;

	Lock();
	prev = m_head;
	session = prev;

	while (session) {
		if (session->m_stale) {
			stale = session;

			if (m_head == stale) {
				m_head = session->m_next;
			}
			else {
				prev->m_next = session->m_next;
			}

			if (m_tail == stale) {
				m_tail = prev;
			}

			index = SendMessage(m_listBox, LB_FINDSTRINGEXACT, 0, (LPARAM) session->m_desc.szSessionName);
			if (index != LB_ERR) {
				index = SendMessage(m_listBox, LB_DELETESTRING, index, 0);
			}

			session = session->m_next;
			delete stale;
			m_count--;
		}
		else {
			prev = session;
			session = session->m_next;
		}
	}

	Unlock();
	SetState(c_stateEnumerate);
}

// FUNCTION: NETMECHW 0x100019cd
void SessionList::StopThread()
{
	DWORD result;

	if (m_thread) {
		PostThreadMessage(m_threadId, WM_QUIT, 0, 0);
		result = WaitForSingleObject(m_thread, INFINITE);
		if (result != WAIT_OBJECT_0) {
		}

		CloseHandle(m_thread);
		m_thread = NULL;
	}
}

// FUNCTION: NETMECHW 0x10001a36
void SessionList::Restart()
{
	MechS32 unused;

	unused = 0;
	StopThread();
	Clear();
	SendMessage(m_listBox, LB_RESETCONTENT, 0, 0);
	SetState(c_stateEnumerate);

	if (!m_thread) {
		m_thread = CreateThread(NULL, 0, SessionThread, NULL, 0, &m_threadId);
		if (!m_thread) {
			m_threadId = 0;
		}

		Sleep(100);
	}
}

// Adds a session the enumeration found, or refreshes the one it found again.
// FUNCTION: NETMECHW 0x10001acf
void SessionList::Update(DPSESSIONDESC* p_desc)
{
	Session* session;

	session = NULL;
	Lock();

	if (FindEntry(p_desc->szSessionName, &session)) {
		if (session) {
			session->m_stale = FALSE;
			session->m_desc = *p_desc;
		}
		else {
		}
	}
	else if (strcmp(p_desc->szSessionName, "")) {
		Add(p_desc);
	}

	Unlock();
}

// FUNCTION: NETMECHW 0x10001b9d
MechS32 SessionList::SetState(MechS32 p_state)
{
	MechS32 state;

	LockState();
	state = m_state;
	m_state = p_state;
	UnlockState();
	return state;
}

// FUNCTION: NETMECHW 0x10001bda
MechS32 SessionList::GetState()
{
	MechS32 state;

	LockState();
	state = m_state;
	UnlockState();
	return state;
}

BOOL PASCAL EnumPlayersCallback(DPID, LPSTR, LPSTR p_formalName, DWORD, LPVOID p_context);

// Matches except for the stack slots of i, session and result, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10001c0c
void SessionList::EnumPlayers()
{
	MechS32 i;
	Session* session;
	HRESULT result;

	session = m_head;
	while (session) {
		Lock();

		for (i = 0; i < 8; i++) {
			strcpy(session->m_playerNames[i], "");
		}

		g_enumPlayerCount = 0;
		result = g_unk0x1001ca90.m_directPlay
					 ->EnumPlayers(session->m_desc.dwSession, EnumPlayersCallback, session, DPENUMPLAYERS_SESSION) &
				 0xfff;
		session->m_playersEnumerated = TRUE;
		session = session->m_next;
		Unlock();
	}
}

// Lists the players of the session selected in the dialog's session list box.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10001cf1
MechS32 SessionList::ShowPlayers(HWND p_dialog)
{
	Session session;
	MechS32 i;
	HWND sessions;
	HWND players;
	MechChar name[DPSESSIONNAMELEN];
	MechS32 result;
	WPARAM index;

	sessions = GetDlgItem(p_dialog, 0x418);
	players = GetDlgItem(p_dialog, 0x42b);
	result = FALSE;

	index = SendMessage(sessions, LB_GETCURSEL, 0, 0);
	if (index != LB_ERR) {
		SendMessage(sessions, LB_GETTEXT, index, (LPARAM) name);
		if (g_sessionList->Find(name, &session) && session.m_playersEnumerated) {
			SendMessage(players, LB_RESETCONTENT, 0, 0);
			i = 0;
			Lock();

			while (session.m_playerNames[i][0] != '\0') {
				SendMessage(players, LB_ADDSTRING, 0, (LPARAM) session.m_playerNames[i]);
				i++;
			}

			Unlock();
			result = TRUE;
		}
	}

	return result;
}

// FUNCTION: NETMECHW 0x10001e1f
BOOL PASCAL EnumSessionsCallback(LPDPSESSIONDESC p_desc, LPVOID, LPDWORD, DWORD p_flags)
{
	if (p_flags & DPESC_TIMEDOUT) {
		g_sessionList->SetState(SessionList::c_stateDone);
		return FALSE;
	}

	g_sessionList->Update(p_desc);
	return TRUE;
}

// Enumerates the sessions, then their players, whenever the list's state asks for it, until
// it receives WM_QUIT.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10001e63
DWORD WINAPI SessionThread(LPVOID)
{
	MechS32 state;
	MSG msg;
	HRESULT result;
	DPSESSIONDESC desc;
	DPSESSIONDESC context;
	MechS32 running;

	running = TRUE;
	while (running) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				running = FALSE;
			}
		}
		else if (g_unk0x1001ca90.m_directPlay) {
			state = g_sessionList->GetState();
			if (state == SessionList::c_stateEnumerate) {
				g_sessionList->SetState(SessionList::c_stateBusy);
				g_sessionList->MarkStale();

				memset(&desc, 0, sizeof(desc));
				desc.dwSize = sizeof(desc);
				desc.guidSession = g_unk0x100230f8;
				strcpy(desc.szPassword, "");
				result = g_unk0x1001ca90.m_directPlay->EnumSessions(
							 &desc,
							 1000,
							 EnumSessionsCallback,
							 &context,
							 (g_unk0x1001ca90.m_unk0x00 == 2 ? DPENUMSESSIONS_PREVIOUS : 0) + DPENUMSESSIONS_AVAILABLE
						 ) &
						 0xff;

				g_sessionList->EnumPlayers();
			}
			else if (state == SessionList::c_stateDone) {
				g_sessionList->RemoveStale();
				PostMessage(GetParent(g_sessionList->m_listBox), WM_COMMAND, MAKEWPARAM(0x418, LBN_SELCHANGE), 0);
			}
		}

		Sleep(50);
	}

	return 0;
}

// Stores the long name of each player of the session p_context, up to eight.
// FUNCTION: NETMECHW 0x10002004
BOOL PASCAL EnumPlayersCallback(DPID, LPSTR, LPSTR p_formalName, DWORD, LPVOID p_context)
{
	SessionList::Session* session;

	session = (SessionList::Session*) p_context;

	if (g_enumPlayerCount < 8) {
		strcpy(session->m_playerNames[g_enumPlayerCount], p_formalName);
		g_enumPlayerCount++;
	}
	else {
		return FALSE;
	}

	return TRUE;
}
