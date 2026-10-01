#include "unk10011030.h"

#include "decomp.h"
#include "types.h"
#include "unk10002140.h"
#include "unk10003660.h"
#include "unk10006b20.h"

#include <dplay.h>
#include <windows.h>

// The host's settings broadcast: sends the settings ("ST") to every player every 200 ms, until
// it receives message 0x418.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10011030
DWORD WINAPI BroadcastSettingsThread(LPVOID)
{
	MSG msg;
	MechS32 running;
	MechU8 buffer[0x200];
	CopperField0x4d* settings;
	NetMessage* message;

	running = TRUE;
	while (running) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == 0x418) {
				running = FALSE;
			}
		}
		else {
			message = (NetMessage*) buffer;
			message->m_tag = NetMessage::c_tagST;
			settings = (CopperField0x4d*) message->m_data;

			EnterCriticalSection(&g_unk0x1001ee60);
			*settings = g_unk0x1001ca90.m_settings;
			LeaveCriticalSection(&g_unk0x1001ee60);

			g_unk0x1001ca90.m_directPlay->Send(
				g_unk0x1001ca90.m_playerId,
				0,
				DPSEND_GUARANTEE,
				buffer,
				sizeof(message->m_tag) + sizeof(*settings)
			);
		}

		Sleep(200);
	}

	return 0;
}
