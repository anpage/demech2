#include "unk1000b400.h"

#include "chatlog.h"
#include "decomp.h"
#include "types.h"
#include "unk10002140.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk10007c30.h"
#include "unk1000aa90.h"

#include <dplay.h>
#include <mbstring.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(SettingsMessage, 0x0d)

// Set by FUN_1000c072 when the local player sends a new mech: FUN_1000b6cb then ignores the
// next settings it receives.
// GLOBAL: NETMECHW 0x10023620
MechS32 g_unk0x10023620 = 0;

// The window that had the focus before FUN_1000b6cb captured the mouse.
// GLOBAL: NETMECHW 0x10023624
HWND g_unk0x10023624 = NULL;

// The teams and the player count FUN_1000b6cb last applied.
// GLOBAL: NETMECHW 0x10023628
MechU8 g_unk0x10023628 = 0;

// GLOBAL: NETMECHW 0x1002362c
MechS32 g_unk0x1002362c = 0;

// A player joined the session: adds them to the player table and announces it in the chat.
// FUNCTION: NETMECHW 0x1000b400
void FUN_1000b400(DPID p_id, MechChar* p_name)
{
	MechChar text[80];
	MechS32 index;
	NetPlayer player;

	EnterCriticalSection(&g_unk0x1001ee60);
	EnterCriticalSection(&g_unk0x1001ca78);
	if (FUN_1000b02b(p_id) != -1) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		LeaveCriticalSection(&g_unk0x1001ee60);
		return;
	}

	if (!FUN_1000aa90(p_name, p_id)) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		LeaveCriticalSection(&g_unk0x1001ee60);
		return;
	}

	index = FUN_1000b02b(p_id);
	FUN_1000aeff(p_id, &player);
	FUN_100057fa(player, index);

	if (g_unk0x1001ca90.m_isHost) {
		g_unk0x1001ca90.m_settings.m_unk0x02 = 0;
		g_unk0x1001ca90.m_settings.m_unk0x03 = 0;
	}

	LeaveCriticalSection(&g_unk0x1001ca78);
	LeaveCriticalSection(&g_unk0x1001ee60);

	sprintf(text, "*** %s has joined the game", p_name);
	PostMessage(g_unk0x10023160, 0x419, 0, (LPARAM) _mbsdup((unsigned char*) text));
	if (g_unk0x10023160) {
		PostMessage(g_unk0x10023160, 0x416, 0, 0);
	}
}

// A player left the session: removes them from the player table and announces it in the chat.
// FUNCTION: NETMECHW 0x1000b54e
void FUN_1000b54e(DPID p_id)
{
	MechChar text[80];
	MechS32 index;
	NetPlayer player;

	EnterCriticalSection(&g_unk0x1001ee60);
	EnterCriticalSection(&g_unk0x1001ca78);
	index = FUN_1000b02b(p_id);
	if (index == -1) {
		LeaveCriticalSection(&g_unk0x1001ca78);
		LeaveCriticalSection(&g_unk0x1001ee60);
		return;
	}

	FUN_1000afa8(index, &player);
	FUN_1000acc9(p_id);
	FUN_10005923(index);
	g_unk0x1001ca90.m_settings.m_unk0x02 = 0;
	g_unk0x1001ca90.m_settings.m_unk0x03 = 0;

	if (g_unk0x1001ca90.m_isHost && FUN_1000b295(player.m_team) == 0 && FUN_1000b0b3() > 1) {
		g_unk0x1001ca90.m_settings.m_unk0x01 ^= 1 << (FUN_1000b0b3() - 1);
		FUN_1000b1fe(g_unk0x1001ca90.m_settings.m_unk0x01);
	}

	LeaveCriticalSection(&g_unk0x1001ca78);
	LeaveCriticalSection(&g_unk0x1001ee60);

	sprintf(text, "*** %s has left the game", player.m_name);
	PostMessage(g_unk0x10023160, 0x419, 0, (LPARAM) _mbsdup((unsigned char*) text));
	if (g_unk0x10023160) {
		PostMessage(g_unk0x10023160, 0x416, 0, 0);
	}
}

// A chat message ("CP").
// FUNCTION: NETMECHW 0x1000b6a6
void FUN_1000b6a6(DPID p_from, void* p_data)
{
	g_chatLog.AddLine(p_from, ((ChatLog::Message*) p_data)->m_text);
}

// New settings from the host ("ST"), or from the local host itself (FUN_1000bd99).
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000b6cb
void FUN_1000b6cb(void* p_settings)
{
	MechS32 ready;
	MechS32 i;

	ready = TRUE;
	FUN_10005e36();

	if (g_unk0x10023620) {
		g_unk0x10023620 = 0;
		return;
	}

	EnterCriticalSection(&g_unk0x1001ee60);
	g_unk0x1001ca90.m_settings = *(CopperField0x4d*) p_settings;
	EnterCriticalSection(&g_unk0x1001ca78);

	for (i = 0; i < FUN_1000b0b3(); i++) {
		if (FUN_1000b02b(g_unk0x1001ca90.m_playerId) != i &&
			!_strnicmp(g_unk0x1001ca90.m_settings.m_mechs[i] + 5, "PLR", 3) &&
			FUN_1000c142(i, g_unk0x1001ca90.m_settings.m_mechs[i])) {
			ready = FALSE;
		}
	}

	if (g_unk0x1001ca90.m_settings.m_unk0x01 != g_unk0x10023628 || FUN_1000b0b3() != g_unk0x1002362c) {
		FUN_1000b1fe(g_unk0x1001ca90.m_settings.m_unk0x01);
	}

	g_unk0x10023628 = g_unk0x1001ca90.m_settings.m_unk0x01;
	g_unk0x1002362c = FUN_1000b0b3();

	if (_strnicmp(
			g_unk0x1001ca90.m_mechFile,
			g_unk0x1001ca90.m_settings.m_mechs[FUN_1000b02b(g_unk0x1001ca90.m_playerId)],
			8
		)) {
		if (!g_unk0x1001ca90.m_isHost) {
			FUN_1000c08c(SettingsMessage::c_typeMech, g_unk0x1001ca90.m_mechFile, 8, TRUE);
			FUN_1000c072();
		}

		strncpy(
			g_unk0x1001ca90.m_settings.m_mechs[FUN_1000b02b(g_unk0x1001ca90.m_playerId)],
			g_unk0x1001ca90.m_mechFile,
			8
		);
		ready = FALSE;
	}

	if ((1 << FUN_1000b0b3()) - 1 == g_unk0x1001ca90.m_settings.m_unk0x02) {
		if (!g_unk0x10023624) {
			g_unk0x10023624 = SetFocus(g_unk0x10023110);
			SetCapture(g_unk0x10023110);
		}

		if (g_unk0x1001ca90.m_isHost) {
			FUN_10005a9a();
		}

		if (ready) {
			if (!g_unk0x1001ca90.m_isHost) {
				FUN_1000c08c(SettingsMessage::c_typeAccept, NULL, 0, FALSE);
			}
			else {
				i = FUN_1000b02b(g_unk0x1001ca90.m_playerId);
				g_unk0x1001ca90.m_settings.m_unk0x03 |= 1 << i;
			}
		}
	}
	else {
		if (g_unk0x10023624) {
			SetFocus(g_unk0x10023624);
			ReleaseCapture();
			g_unk0x10023624 = NULL;
		}

		if (g_unk0x1001ca90.m_isHost) {
			FUN_10005a55();
		}
	}

	if (!g_unk0x1001ca90.m_isHost && (1 << FUN_1000b0b3()) - 1 == g_unk0x1001ca90.m_settings.m_unk0x03) {
		FUN_1000c08c(SettingsMessage::c_typeLaunch, NULL, 0, FALSE);
		g_unk0x1001ca90.m_unk0x158 = 1;
		FUN_100042ab();
	}
	else if ((1 << FUN_1000b0b3()) - 1 == g_unk0x1001ca90.m_settings.m_unk0x02 && FUN_1000b0b3() == 1) {
		g_unk0x1001ca90.m_unk0x158 = 1;
		FUN_100042ab();
	}

	LeaveCriticalSection(&g_unk0x1001ca78);
	LeaveCriticalSection(&g_unk0x1001ee60);

	if (g_unk0x10023160) {
		PostMessage(g_unk0x10023160, 0x416, 0, 0);
	}
}

// A player asks for the local player's mech ("MQ"): sends it back to them as "MI", if it is a
// player mech (MEK\<xxx><slot>PLR.MEK).
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000ba4d
void FUN_1000ba4d(DPID p_to)
{
	DWORD read;
	MechChar path[276];
	HANDLE file;
	MechU8 mech[0x2c2];
	MechU8 buffer[0x200];
	NetMessage* message;
	MechChar* data;

	if (strncmp(g_unk0x1001ca90.m_mechFile + 5, "PLR", 3)) {
		return;
	}

	message = (NetMessage*) buffer;
	message->m_tag = NetMessage::c_tagMI;
	data = (MechChar*) message->m_data;

	EnterCriticalSection(&g_unk0x1001ee60);
	sprintf(path, "MEK\\%3.3s%02dPLR.MEK", g_unk0x1001ca90.m_mechFile, FUN_1000b02b(g_unk0x1001ca90.m_playerId));

	file = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE) {
		LeaveCriticalSection(&g_unk0x1001ee60);
		return;
	}

	strncpy(data, g_unk0x1001ca90.m_mechFile, 9);
	LeaveCriticalSection(&g_unk0x1001ee60);

	ReadFile(file, mech, sizeof(mech), &read, NULL);
	CloseHandle(file);
	FUN_100083e3(mech, data + 9);

	g_unk0x1001ca90.m_directPlay->Send(g_unk0x1001ca90.m_playerId, p_to, DPSEND_GUARANTEE, buffer, 0x1cc);
}

// A player's mech ("MI", the answer to FUN_1000c142's "MQ"): writes it to the player's mech copy,
// MEK\<xxx><slot>PLR.MEK, replacing their earlier copies.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000bbb0
void FUN_1000bbb0(DPID p_from, MechChar* p_data)
{
	MechS32 copy;
	WIN32_FIND_DATA findData;
	HANDLE find;
	DWORD written;
	MechChar path[276];
	HANDLE file;
	MechS32 index;
	DWORD size;
	MechU8 mech[0x2c4];
	MechS32 more;

	find = FindFirstFile("MEK", &findData);
	if (find == INVALID_HANDLE_VALUE) {
		CreateDirectory("MEK", NULL);
	}
	else {
		FindClose(find);
	}

	EnterCriticalSection(&g_unk0x1001ca78);
	index = FUN_1000b02b(p_from);

	sprintf(path, "MEK\\???%02dPLR.MEK", index);
	find = FindFirstFile(path, &findData);
	more = TRUE;
	while (more) {
		sprintf(path, "MEK\\%s", findData.cFileName);
		remove(path);
		more = FindNextFile(find, &findData);
	}

	FindClose(find);
	FUN_100086b1(p_data + 9, mech, &size);
	sprintf(path, "MEK\\%3.3s%02dPLR.MEK", p_data, index);

	file = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!file) {
		return;
	}

	WriteFile(file, mech, size, &written, NULL);
	CloseHandle(file);
	sscanf(p_data, "%*c%*c%*c%dPLR", &copy);
	g_unk0x1001ce18[index] = copy;
	LeaveCriticalSection(&g_unk0x1001ca78);
}

// A settings change ("SD"): the host applies it to a copy of the settings and sends the result
// to everyone through FUN_1000b6cb.
// FUNCTION: NETMECHW 0x1000bd99
void FUN_1000bd99(DPID p_from, SettingsMessage* p_message)
{
	MechS32 index;
	CopperField0x4d settings;

	EnterCriticalSection(&g_unk0x1001ee60);
	settings = g_unk0x1001ca90.m_settings;
	LeaveCriticalSection(&g_unk0x1001ee60);

	switch (p_message->m_type) {
	case SettingsMessage::c_typeTeam:
		EnterCriticalSection(&g_unk0x1001ca78);
		index = FUN_1000b02b(p_from);
		if (FUN_1000b295((settings.m_unk0x01 & (1 << index)) != 0) == 1) {
			LeaveCriticalSection(&g_unk0x1001ca78);
			break;
		}

		settings.m_unk0x01 ^= 1 << index;
		LeaveCriticalSection(&g_unk0x1001ca78);
		if (!(p_message->m_flag & 1)) {
			settings.m_unk0x02 = 0;
		}
		break;
	case SettingsMessage::c_typeReady:
		EnterCriticalSection(&g_unk0x1001ca78);
		index = FUN_1000b02b(p_from);
		settings.m_unk0x02 = (settings.m_unk0x02 & ~(1 << index)) | (p_message->m_data.m_value << index);
		LeaveCriticalSection(&g_unk0x1001ca78);
		break;
	case SettingsMessage::c_typeAccept:
		EnterCriticalSection(&g_unk0x1001ca78);
		index = FUN_1000b02b(p_from);
		settings.m_unk0x03 |= 1 << index;
		LeaveCriticalSection(&g_unk0x1001ca78);
		break;
	case SettingsMessage::c_typeMech:
		EnterCriticalSection(&g_unk0x1001ca78);
		index = FUN_1000b02b(p_from);
		strncpy(settings.m_mechs[index], p_message->m_data.m_text, 8);
		if (!(p_message->m_flag & 1)) {
			settings.m_unk0x02 &= ~(1 << index);
		}
		LeaveCriticalSection(&g_unk0x1001ca78);
		break;
	case 5:
		strncpy(settings.m_unk0x44, p_message->m_data.m_text, 4);
		if (!(p_message->m_flag & 1)) {
			settings.m_unk0x02 = 0;
		}
		break;
	case 6:
		settings.m_options.m_byte =
			(~p_message->m_data.m_bytes[1] & settings.m_options.m_byte) | p_message->m_data.m_bytes[0];
		if (!(p_message->m_flag & 1)) {
			settings.m_unk0x02 = 0;
		}
		break;
	case 7:
		if (!(p_message->m_flag & 1)) {
			settings.m_unk0x02 = 0;
		}
		break;
	case SettingsMessage::c_typeLaunch:
		g_unk0x1001ca90.m_unk0x154++;
		if (FUN_1000b0b3() - 1 == g_unk0x1001ca90.m_unk0x154) {
			g_unk0x1001ca90.m_unk0x158 = 1;
			FUN_100042ab();
		}
		return;
	}

	if ((1 << FUN_1000b0b3()) - 1 != settings.m_unk0x02) {
		settings.m_unk0x03 = 0;
	}

	FUN_1000b6cb(&settings);
}

// FUNCTION: NETMECHW 0x1000c072
void FUN_1000c072()
{
	g_unk0x10023620 = 1;
}

// Sends a settings change to the host, or applies it directly on the host.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000c08c
void FUN_1000c08c(MechU32 p_type, void* p_data, MechU32 p_size, MechU8 p_flag)
{
	MechU8 buffer[0x200];
	NetMessage* message;
	SettingsMessage* change;

	message = (NetMessage*) buffer;
	message->m_tag = NetMessage::c_tagSD;
	change = (SettingsMessage*) message->m_data;
	change->m_type = p_type;
	change->m_flag = p_flag;
	memcpy(&change->m_data, p_data, p_size);

	if (g_unk0x1001ca90.m_isHost) {
		FUN_1000bd99(g_unk0x1001ca90.m_playerId, change);
	}
	else {
		while (g_unk0x1001ca90.m_directPlay
				   ->Send(g_unk0x1001ca90.m_playerId, g_unk0x1001ca90.m_hostId, DPSEND_GUARANTEE, buffer, 0xf) ==
			   DPERR_BUSY) {
		}
	}
}

// Asks the player in slot p_index for their mech ("MQ") if the copy named p_name isn't the one
// the lobby has; returns whether it asked.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000c142
MechS32 FUN_1000c142(MechS32 p_index, MechChar* p_name)
{
	MechS32 copy;
	MechChar name[12];
	NetPlayer player;
	MechU8 buffer[0x200];
	NetMessage* message;

	strncpy(name, p_name, 8);
	name[8] = '\0';
	sscanf(name, "%*c%*c%*c%dPLR", &copy);

	if (g_unk0x1001ce18[p_index] == copy) {
		return FALSE;
	}

	FUN_1000afa8(p_index, &player);
	message = (NetMessage*) buffer;
	message->m_tag = NetMessage::c_tagMQ;
	g_unk0x1001ca90.m_directPlay
		->Send(g_unk0x1001ca90.m_playerId, player.m_id, DPSEND_GUARANTEE, message, sizeof(message->m_tag));
	return TRUE;
}
