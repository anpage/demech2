#include "unk10002140.h"

#include "decomp.h"
#include "types.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000aa90.h"
#include "unk1000b400.h"

#include <dplay.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(NetMessage, 0x200)

// GLOBAL: NETMECHW 0x10023040
MechS32 g_unk0x10023040 = 0;

// Receives the DirectPlay messages and hands each to p_handler (HostMessageHandler or
// ClientMessageHandler), until it receives message 0x418.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10002140
DWORD WINAPI ReceiveThread(LPVOID p_handler)
{
	MSG msg;
	NetMessageHandler handler;
	DWORD size;
	DPID to;
	MechU8 buffer[0x200];
	MechS32 running;
	DPID from;
	HRESULT result;

	running = TRUE;
	size = sizeof(buffer);
	handler = (NetMessageHandler) p_handler;

	while (running) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == 0x418) {
				running = FALSE;
			}
		}
		else if (g_unk0x1001ca90.m_directPlay) {
			result = g_unk0x1001ca90.m_directPlay->Receive(&from, &to, DPRECEIVE_ALL, buffer, &size);
			if (result == DP_OK) {
				if (g_unk0x10023040 < 30) {
					g_unk0x10023040++;
				}
			}
			else if (result != DPERR_NOMESSAGES) {
			}

			switch (result) {
			case DP_OK:
				handler(buffer, from);
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

// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10002289
void HostMessageHandler(void* p_message, DPID p_from)
{
	DPMSG_GENERIC* system;
	DPMSG_ADDPLAYER* add;
	DPMSG_DELETEPLAYER* deleted;
	MechChar name[DPSHORTNAMELEN + 1];
	NetMessage* message;

	if (!p_from) {
		system = (DPMSG_GENERIC*) p_message;
		switch (system->dwType) {
		case DPSYS_ADDPLAYER:
			add = (DPMSG_ADDPLAYER*) system;
			if (add->dwPlayerType) {
				strcpy(name, add->szShortName);
				name[DPSHORTNAMELEN] = '\0';
				FUN_1000b400(add->dpId, name);
			}
			break;
		case DPSYS_DELETEPLAYER:
			deleted = (DPMSG_DELETEPLAYER*) system;
			FUN_1000b54e(deleted->dpId);
			break;
		default:
			break;
		}
	}
	else {
		FUN_1000b3c5(FUN_1000b02b(p_from));
		message = (NetMessage*) p_message;

		switch (message->m_tag) {
		case NetMessage::c_tagCP:
			FUN_1000b6a6(p_from, message->m_data);
			break;
		case NetMessage::c_tagSD:
			FUN_1000bd99(p_from, (SettingsMessage*) message->m_data);
			break;
		case NetMessage::c_tagMQ:
			FUN_1000ba4d(p_from);
			break;
		case NetMessage::c_tagMI:
			FUN_1000bbb0(p_from, (MechChar*) message->m_data);
			break;
		default:
			break;
		}
	}
}

// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10002424
void ClientMessageHandler(void* p_message, DPID p_from)
{
	DPMSG_GENERIC* system;
	DPMSG_ADDPLAYER* add;
	DPMSG_DELETEPLAYER* deleted;
	MechChar name[DPSHORTNAMELEN + 1];
	NetMessage* message;

	if (!p_from) {
		system = (DPMSG_GENERIC*) p_message;
		switch (system->dwType) {
		case DPSYS_ADDPLAYER:
			add = (DPMSG_ADDPLAYER*) system;
			if (add->dwPlayerType) {
				strcpy(name, add->szShortName);
				name[DPSHORTNAMELEN] = '\0';
				FUN_1000b400(add->dpId, name);
			}
			break;
		case DPSYS_DELETEPLAYER:
			deleted = (DPMSG_DELETEPLAYER*) system;
			FUN_1000b54e(deleted->dpId);
			break;
		default:
			break;
		}
	}
	else {
		message = (NetMessage*) p_message;

		switch (message->m_tag) {
		case NetMessage::c_tagCP:
			FUN_1000b6a6(p_from, message->m_data);
			break;
		case NetMessage::c_tagXX:
			g_unk0x1001ca90.m_directPlay
				->Send(g_unk0x1001ca90.m_playerId, p_from, DPSEND_GUARANTEE, message, sizeof(message->m_tag));
			break;
		case NetMessage::c_tagST:
			FUN_1000b6cb(message->m_data);
			if (!g_unk0x1001ca90.m_hostId) {
				g_unk0x1001ca90.m_hostId = p_from;
			}
			break;
		case NetMessage::c_tagMQ:
			FUN_1000ba4d(p_from);
			break;
		case NetMessage::c_tagMI:
			FUN_1000bbb0(p_from, (MechChar*) message->m_data);
			break;
		default:
			break;
		}
	}
}
