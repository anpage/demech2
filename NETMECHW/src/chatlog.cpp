#include "chatlog.h"

#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10002140.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000aa90.h"

#include <ctype.h>
#include <dplay.h>
#include <mbstring.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(ChatLog, 0x1bc)
DECOMP_SIZE_ASSERT(ChatLog::Message, 0x1fe)
DECOMP_SIZE_ASSERT(ChatLog::Line, 0x198)

// FUNCTION: NETMECHW 0x1000c200
ChatLog::ChatLog()
{
	m_attached = FALSE;
	m_head = NULL;
	m_tail = NULL;
	m_topIndex = -1;
	m_dialog = NULL;
	m_listBox = NULL;
	m_lineCount = 0;
	m_mutex = CreateMutex(NULL, FALSE, "ChatMutex");
}

// Adds p_text from the player p_from (0: the lobby itself) to the chat, word-wrapped to the
// list box's width in lines of their own, and shows the new lines if the dialog is open.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000c273
void ChatLog::AddLine(DPID p_from, MechChar* p_text)
{
	MechS32 i;
	Line* first;
	MechS32 removed;
	INT tabStop;
	SCROLLINFO scrollInfo;
	MechChar text[424];
	MechChar* body;
	NetPlayer player;
	Line* line;
	RECT rect;
	MechS32 width;
	HDC dc;
	TEXTMETRIC metrics;
	HDC infoDc;
	MechChar* end;
	MechChar* wrap;
	MechChar saved;
	MechChar* rest;
	WPARAM index;
	LRESULT topIndex;

	first = NULL;
	removed = 0;
	tabStop = 60;
	Lock();

	scrollInfo.fMask = SIF_ALL;
	scrollInfo.cbSize = sizeof(scrollInfo);
	GetScrollInfo(m_listBox, SB_VERT, &scrollInfo);

	if (p_from) {
		FUN_1000aeff(p_from, &player);
		strcpy(text, player.m_name);
		strcat(text, "\t");
		body = strchr(text, '\t') + 1;
	}
	else {
		strcpy(text, "");
		body = text;
	}

	strcat(text, p_text);
	rest = p_text;

	while (*body != '\0') {
		if (m_lineCount == 40) {
			line = m_head->m_next;
			delete m_head;
			m_head = line;
			m_lineCount--;
			removed++;
		}

		line = new Line;
		if (!first) {
			first = line;
		}

		if (m_tail) {
			m_tail->m_next = line;
		}
		else {
			m_head = line;
		}

		line->m_next = NULL;
		m_tail = line;

		if (GetWindowRect(m_listBox, &rect)) {
			width = rect.right - rect.left;
			tabStop = width * 60 / 222;
			width -= GetSystemMetrics(SM_CXVSCROLL) + (width << 2) / 222;
		}
		else {
			infoDc = GetDC(NULL);
			GetTextMetrics(infoDc, &metrics);
			width = metrics.tmAveCharWidth * 50;
		}

		dc = GetDC(m_listBox);
		SetMapMode(dc, MM_TEXT);
		SelectObject(dc, m_font);

		end = body;
		while (*end != '\0') {
			wrap = end;

			while (isspace(*end) && *end != '\0') {
				end++;
			}

			while (!isspace(*end) && *end != '\0') {
				end++;
			}

			saved = *end;
			*end = '\0';

			if ((MechS32) LOWORD(GetTabbedTextExtent(dc, text, end - text, 1, &tabStop)) >= width) {
				*end = saved;
				break;
			}

			*end = saved;
		}

		ReleaseDC(m_listBox, dc);

		if (body == wrap && *end == '\0') {
			wrap = end;
		}
		else {
			dc = GetDC(m_listBox);
			SetMapMode(dc, MM_TEXT);
			SelectObject(dc, m_font);

			if ((MechS32) LOWORD(GetTabbedTextExtent(dc, text, end - text, 1, &tabStop)) < width) {
				wrap = end;
			}

			ReleaseDC(m_listBox, dc);
		}

		saved = *wrap;
		*wrap = '\0';
		strncpy(line->m_text, text, sizeof(line->m_text) - 1);
		line->m_text[strlen(line->m_text)] = '\0';
		line->m_from = p_from;
		m_lineCount++;
		*wrap = saved;

		rest += wrap - body;
		while (isspace(*rest) && *rest != '\0') {
			rest++;
		}

		*body = '\0';
		strcat(body, rest);
	}

	Unlock();

	if (m_attached) {
		for (i = 0; i < removed; i++) {
			if (!topIndex) {
				topIndex = SendMessage(m_listBox, LB_GETTOPINDEX, 0, 0);
			}

			SendMessage(m_listBox, LB_DELETESTRING, 0, 0);
		}

		while (first) {
			index = SendMessage(m_listBox, LB_ADDSTRING, 0, (LPARAM) first->m_text);
			SendMessage(m_listBox, LB_SETITEMDATA, index, p_from);
			first = first->m_next;
		}

		if (scrollInfo.nPos + scrollInfo.nPage >= (UINT) scrollInfo.nMax) {
			SendMessage(m_listBox, LB_SETTOPINDEX, index, 0);
		}
		else if (removed) {
			SendMessage(m_listBox, LB_SETTOPINDEX, topIndex - removed, 0);
		}
	}
}

// FUNCTION: NETMECHW 0x1000c9a7
ChatLog::~ChatLog()
{
	Line* line;
	Line* next;

	line = m_head;
	while (line) {
		next = line->m_next;
		delete line;
		line = next;
	}

	m_head = NULL;
	m_tail = NULL;
	ReleaseMutex(m_mutex);
	CloseHandle(m_mutex);
}

// FUNCTION: NETMECHW 0x1000ca2b
void ChatLog::Reset()
{
	Line* line;
	Line* next;

	Lock();

	line = m_head;
	while (line) {
		next = line->m_next;
		delete line;
		line = next;
	}

	m_head = NULL;
	m_tail = NULL;
	strcpy(m_input, "");
	m_topIndex = 0;
	m_lineCount = 0;
	Unlock();
}

// FUNCTION: NETMECHW 0x1000cae3
void ChatLog::SaveInput()
{
	Lock();
	GetDlgItemText(m_dialog, 0x3f2, m_input, sizeof(m_input) - 1);
	SendMessage(GetDlgItem(m_dialog, 0x3f2), EM_GETSEL, (WPARAM) &m_inputSelStart, (LPARAM) &m_inputSelEnd);
	Unlock();
}

// FUNCTION: NETMECHW 0x1000cb57
void ChatLog::SaveTopIndex()
{
	Lock();
	m_topIndex = SendMessage(m_listBox, LB_GETTOPINDEX, 0, 0);
	Unlock();
}

// Matches except for the stack slots of tabStop and scrollInfo, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000cb99
void ChatLog::Attach(HWND p_dialog)
{
	INT tabStop;
	SCROLLINFO scrollInfo;

	tabStop = 60;
	Lock();

	m_dialog = p_dialog;
	m_listBox = GetDlgItem(p_dialog, 0x3ee);
	m_font = (HFONT) SendMessage(m_listBox, WM_GETFONT, 0, 0);
	SendMessage(GetDlgItem(p_dialog, 0x3f2), EM_LIMITTEXT, sizeof(m_input) - 1, 0);

	if (GetDlgItem(p_dialog, 0x434)) {
		if (!IsDlgButtonChecked(m_dialog, 0x434) && !IsDlgButtonChecked(m_dialog, 0x438)) {
			CheckDlgButton(m_dialog, 0x434, BST_CHECKED);
		}
	}

	scrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	scrollInfo.nPage = 41;
	scrollInfo.nPos = 40;
	scrollInfo.nMin = 0;
	scrollInfo.nMax = 40;
	scrollInfo.cbSize = sizeof(scrollInfo);
	SetScrollInfo(m_listBox, SB_VERT, &scrollInfo, FALSE);

	SendMessage(m_listBox, LB_SETTABSTOPS, 1, (LPARAM) &tabStop);
	m_attached = TRUE;
	Unlock();
}

// FUNCTION: NETMECHW 0x1000cce1
void ChatLog::Restore()
{
	Line* line;

	line = m_head;
	Lock();

	SetDlgItemText(m_dialog, 0x3f2, m_input);
	SendMessage(GetDlgItem(m_dialog, 0x3f2), EM_SETSEL, m_inputSelStart, m_inputSelEnd);
	SendMessage(m_listBox, LB_RESETCONTENT, 0, 0);

	if (m_attached) {
		while (line) {
			SendMessage(m_listBox, LB_ADDSTRING, 0, (LPARAM) line->m_text);
			line = line->m_next;
		}

		if (m_topIndex != -1) {
			SendMessage(m_listBox, LB_SETTOPINDEX, m_topIndex, 0);
		}
		else {
			m_topIndex = SendMessage(m_listBox, LB_GETTOPINDEX, 0, 0);
		}
	}

	Unlock();
}

// Sends the typed text to every player, and shows it.
// FUNCTION: NETMECHW 0x1000cdfc
void ChatLog::SendToAll(HWND p_dialog)
{
	DPID to;
	MechChar text[400];

	to = 0;
	GetDlgItemText(p_dialog, 0x3f2, text, sizeof(text));
	if (!IsBlankString(text)) {
		Send(g_unk0x1001ca90.m_playerId, to, text);
		Send(g_unk0x1001ca90.m_playerId, g_unk0x1001ca90.m_playerId, text);
	}

	SetDlgItemText(p_dialog, 0x3f2, "");
	SetDlgItemText(p_dialog, 0x3ec, LoadResString(0x76));
}

// Sends the typed text to the players of the local player's team, and shows it.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes, and the operand
// order of team == myTeam and player.m_id != m_playerId (one declaration order tried).
// FUNCTION: NETMECHW 0x1000cec1
void ChatLog::SendToTeam(HWND p_dialog)
{
	MechS32 send;
	MechS32 i;
	DPID to;
	MechS32 team;
	MechS32 myTeam;
	NetPlayer player;
	NetPlayer self;
	MechU8 teamMask;
	MechChar text[400];

	to = 0;
	teamMask = 1 << FUN_1000aeff(g_unk0x1001ca90.m_playerId, &self);
	myTeam = self.m_team;

	GetDlgItemText(p_dialog, 0x3f2, text, sizeof(text));
	if (!IsBlankString(text)) {
		if (to) {
			Send(g_unk0x1001ca90.m_playerId, to, text);
		}
		else {
			i = 0;
			EnterCriticalSection(&g_unk0x1001ca78);

			while (i < 8) {
				if (FUN_1000afa8(i, &player)) {
					team = player.m_team;
					if (team == myTeam && player.m_id != g_unk0x1001ca90.m_playerId) {
						send = TRUE;
					}
					else {
						send = FALSE;
					}

					if (send) {
						Send(g_unk0x1001ca90.m_playerId, player.m_id, text);
					}
				}

				i++;
			}

			LeaveCriticalSection(&g_unk0x1001ca78);
			Send(g_unk0x1001ca90.m_playerId, g_unk0x1001ca90.m_playerId, text);
		}
	}

	SetDlgItemText(p_dialog, 0x3f2, "");
	SetDlgItemText(p_dialog, 0x3ec, LoadResString(0x76));
}

// Sends p_text from p_from to the player p_to (0: everyone); to the local player itself, it
// posts the text to the lobby window instead.
// Matches except for the stack slots of chat, packet, message and self, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000d092
void ChatLog::Send(DPID p_from, DPID p_to, MechChar* p_text)
{
	Message* chat;
	NetMessage packet;
	NetMessage* message;
	NetPlayer self;

	message = &packet;
	message->m_tag = NetMessage::c_tagCP;
	chat = (Message*) message->m_data;
	FUN_1000aeff(g_unk0x1001ca90.m_playerId, &self);

	if (p_from) {
		strcpy(chat->m_name, self.m_name);
	}
	else {
		strcpy(chat->m_name, "");
	}

	strcpy(chat->m_text, p_text);
	chat->m_length = strlen(chat->m_text) + 1;

	if (p_to == p_from) {
		PostMessage(g_unk0x10023160, 0x419, p_from, (LPARAM) _mbsdup((unsigned char*) chat->m_text));
	}
	else {
		g_unk0x1001ca90.m_directPlay->Send(
			p_from,
			p_to,
			DPSEND_TRYONCE,
			message,
			chat->m_length + sizeof(message->m_tag) + sizeof(*chat) - sizeof(chat->m_text)
		);
	}
}
