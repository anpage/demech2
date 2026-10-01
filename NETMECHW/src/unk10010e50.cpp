#include "unk10010e50.h"

#include "decomp.h"
#include "types.h"
#include "unk10002140.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000aa90.h"
#include "unk1000b400.h"

#include <dplay.h>
#include <string.h>
#include <windows.h>

// Watches the session every 100 ms, until it receives message 0x418: the host pings ("XX") the
// players it hasn't heard from for 20 periods and drops those silent for 50 (FUN_1000b30e), and
// every player adds the remote players missing from the player table (AddMissingPlayer).
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10010e50
DWORD WINAPI WatchPlayersThread(LPVOID)
{
	MechS32 running;
	HRESULT result;
	MSG msg;
	MechU16 ping;
	NetPlayer player;
	MechS32 status;
	MechS32 i;

	running = TRUE;
	while (running) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == 0x418) {
				running = FALSE;
			}
		}
		else {
			EnterCriticalSection(&g_unk0x1001ca78);
			for (i = 0; i < FUN_1000b0b3(); i++) {
				if (g_unk0x1001ca90.m_isHost) {
					status = FUN_1000b30e(i);
					if (status == -1) {
						FUN_1000afa8(i, &player);
						ping = NetMessage::c_tagXX;
						g_unk0x1001ca90.m_directPlay
							->Send(g_unk0x1001ca90.m_playerId, player.m_id, DPSEND_GUARANTEE, &ping, sizeof(ping));
					}
					else if (status == -2) {
						FUN_1000afa8(i, &player);
						FUN_1000b54e(player.m_id);
						g_unk0x1001ca90.m_directPlay->DestroyPlayer(player.m_id);
					}
				}
			}

			LeaveCriticalSection(&g_unk0x1001ca78);
			result = g_unk0x1001ca90.m_directPlay->EnumPlayers(0, AddMissingPlayer, NULL, DPENUMPLAYERS_REMOTE);
		}

		Sleep(100);
	}

	return 0;
}

// The EnumPlayers callback of WatchPlayersThread: adds the player to the player table if it
// isn't in it yet.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10010fab
BOOL FAR PASCAL AddMissingPlayer(DPID p_id, LPSTR p_friendlyName, LPSTR, DWORD, LPVOID)
{
	MechS32 index;
	MechChar name[DPSHORTNAMELEN + 1];

	EnterCriticalSection(&g_unk0x1001ca78);
	index = FUN_1000b02b(p_id);
	if (index == -1) {
		strcpy(name, p_friendlyName);
		name[DPSHORTNAMELEN] = '\0';
		FUN_1000b400(p_id, name);
	}

	LeaveCriticalSection(&g_unk0x1001ca78);
	return TRUE;
}
