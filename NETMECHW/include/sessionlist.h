#ifndef SESSIONLIST_H
#define SESSIONLIST_H

#include "decomp.h"
#include "types.h"

#include <dplay.h>
#include <windows.h>

// The sessions DirectPlay finds, shown in the join dialog's list box. A thread enumerates the
// sessions, and each session's players, while the dialog is open; "SessionMutex" guards the
// list, "SessionFlagMutex" the state the thread and the dialog signal each other with.
// SIZE 0xac
class SessionList {
public:
	// SIZE 0x228
	struct Session {
		MechS32 m_stale;                          // 0x00
		DPSESSIONDESC m_desc;                     // 0x04
		Session* m_next;                          // 0x80
		MechChar m_playerNames[8][DPLONGNAMELEN]; // 0x84
		MechS32 m_playersEnumerated;              // 0x224
	};

	enum {
		c_stateEnumerate = 0,
		c_stateDone = 1,
		c_stateBusy = 2
	};

	SessionList(HWND p_listBox);
	~SessionList();

	// SYNTHETIC: NETMECHW 0x10006020
	// SessionList::`scalar deleting destructor'

	void Clear();
	void MarkStale();
	MechS32 Find(MechChar* p_name, Session* p_session);
	MechS32 FindEntry(MechChar* p_name, Session** p_session);
	MechS32 Add(DPSESSIONDESC* p_desc);
	void RemoveStale();
	void StopThread();
	void Restart();
	void Update(DPSESSIONDESC* p_desc);
	MechS32 SetState(MechS32 p_state);
	MechS32 GetState();
	void EnumPlayers();
	MechS32 ShowPlayers(HWND p_dialog);

	// FUNCTION: NETMECHW 0x10002080
	void Lock() { WaitForSingleObject(m_mutex, INFINITE); }

	// FUNCTION: NETMECHW 0x100020b0
	void Unlock() { ReleaseMutex(m_mutex); }

	// FUNCTION: NETMECHW 0x100020e0
	void LockState() { WaitForSingleObject(m_stateMutex, INFINITE); }

	// FUNCTION: NETMECHW 0x10002110
	void UnlockState() { ReleaseMutex(m_stateMutex); }

	undefined4 m_unk0x00;             // 0x00
	MechS32 m_count;                  // 0x04
	Session* m_head;                  // 0x08
	Session* m_tail;                  // 0x0c
	HANDLE m_thread;                  // 0x10
	undefined4 m_unk0x14;             // 0x14
	DWORD m_threadId;                 // 0x18
	HANDLE m_mutex;                   // 0x1c
	undefined4 m_unk0x20;             // 0x20
	HANDLE m_stateMutex;              // 0x24
	MechS32 m_state;                  // 0x28
	HWND m_listBox;                   // 0x2c
	undefined m_unk0x30[0xac - 0x30]; // 0x30
};

#endif // SESSIONLIST_H
