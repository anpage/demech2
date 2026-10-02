#ifndef CHATLOG_H
#define CHATLOG_H

#include "decomp.h"
#include "types.h"

#include <dplay.h>
#include <windows.h>

// The lobby's chat: the last 40 lines, word-wrapped to the width of the dialog's chat list box
// (control 0x3ee), and the text being typed in its edit control (0x3f2), kept while the dialog
// is closed. "ChatMutex" guards it. One global instance, g_chatLog (unk10003660.cpp).
// SIZE 0x1bc
class ChatLog {
public:
#pragma pack(1)
	// The data of a chat message, a NetMessage tagged "CP".
	// SIZE 0x1fe
	struct Message {
		MechChar m_name[DPSHORTNAMELEN]; // 0x00
		MechU32 m_length;                // 0x14
		MechChar m_text[0x1fe - 0x18];   // 0x18
	};
#pragma pack()

	// SIZE 0x198
	struct Line {
		DPID m_from;          // 0x00
		MechChar m_text[400]; // 0x04
		Line* m_next;         // 0x194
	};

	ChatLog();
	~ChatLog();

	void AddLine(DPID p_from, MechChar* p_text);
	void Reset();
	void SaveInput();
	void SaveTopIndex();
	void Attach(HWND p_dialog);
	void Restore();
	void SendToAll(HWND p_dialog);
	void SendToTeam(HWND p_dialog);
	void Send(DPID p_from, DPID p_to, MechChar* p_text);

	// FUNCTION: NETMECHW 0x10006000
	void Detach() { m_attached = FALSE; }

	// FUNCTION: NETMECHW 0x1000d1f0
	void Lock() { WaitForSingleObject(m_mutex, INFINITE); }

	// FUNCTION: NETMECHW 0x1000d220
	void Unlock() { ReleaseMutex(m_mutex); }

	Line* m_head;          // 0x00
	Line* m_tail;          // 0x04
	MechS32 m_topIndex;    // 0x08
	HWND m_dialog;         // 0x0c
	HWND m_listBox;        // 0x10
	HFONT m_font;          // 0x14
	MechS32 m_lineCount;   // 0x18
	HANDLE m_mutex;        // 0x1c
	MechS32 m_attached;    // 0x20
	MechChar m_input[400]; // 0x24
	DWORD m_inputSelStart; // 0x1b4
	DWORD m_inputSelEnd;   // 0x1b8
};

#endif // CHATLOG_H
