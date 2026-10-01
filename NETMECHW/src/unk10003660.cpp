#include "unk10003660.h"

#include "chatlog.h"
#include "decomp.h"
#include "netlaunchinfo.h"
#include "sessionlist.h"
#include "types.h"
#include "unk10006b20.h"
#include "unk1000aa90.h"

#include <windows.h>

// Guards the player table.
// GLOBAL: NETMECHW 0x1001ca78
CRITICAL_SECTION g_unk0x1001ca78;

// GLOBAL: NETMECHW 0x1001ca90
IronLantern0x160 g_unk0x1001ca90;

// Constructed after g_unk0x1001ca90, by the unit's static initializer.
// GLOBAL: NETMECHW 0x1001c8b8
ChatLog g_chatLog;

// GLOBAL: NETMECHW 0x1001ce14
DWORD g_unk0x1001ce14;

// A counter per player slot, for the names of the players' mech copies (CopyPlayerMech).
// GLOBAL: NETMECHW 0x1001ce18
MechU8 g_unk0x1001ce18[8];

// GLOBAL: NETMECHW 0x1001ce20
DWORD g_unk0x1001ce20;

// GLOBAL: NETMECHW 0x1001ce2c
HINSTANCE g_hInstance;

// GLOBAL: NETMECHW 0x1001ee54
DWORD g_unk0x1001ee54;

// Guards the lobby's settings (g_unk0x1001ca90.m_settings).
// GLOBAL: NETMECHW 0x1001ee60
CRITICAL_SECTION g_unk0x1001ee60;

// The application GUID of NetMech's DirectPlay sessions.
// GLOBAL: NETMECHW 0x100230f8
GUID g_unk0x100230f8 = {0x528519a0, 0x1001, 0x11cf, {0x80, 0x5c, 0x00, 0xaa, 0x00, 0x44, 0x43, 0x1f}};

// GLOBAL: NETMECHW 0x10023110
HWND g_unk0x10023110 = NULL;

// GLOBAL: NETMECHW 0x10023120
SessionList* g_sessionList = NULL;

// The receive thread (ReceiveThread), the host's settings broadcast (BroadcastSettingsThread)
// and the session watch (WatchPlayersThread).
// GLOBAL: NETMECHW 0x10023128
HANDLE g_unk0x10023128 = NULL;

// GLOBAL: NETMECHW 0x1002312c
HANDLE g_unk0x1002312c = NULL;

// GLOBAL: NETMECHW 0x10023130
HANDLE g_unk0x10023130 = NULL;

// The lobby window.
// GLOBAL: NETMECHW 0x10023160
HWND g_unk0x10023160 = NULL;

// GLOBAL: NETMECHW 0x1002317c
MechS32 g_unk0x1002317c = 0;

// Whether new players may join the session (FUN_10005a55, FUN_10005a9a).
// GLOBAL: NETMECHW 0x10023180
MechS32 g_unk0x10023180 = 0;

// NetMech's lobby. MECH2.EXE calls it with the launch record to fill in, runs the simulator
// when it returns nonzero, and calls it again when the mission is over.
// STUB: NETMECHW 0x10003684
extern "C" MechS32 __stdcall Launcher(NetLaunchInfo*)
{
	STUB(0x10003684);
	return 0;
}

// Stops the lobby: posts message 0x418 to the lobby window's thread.
// FUNCTION: NETMECHW 0x100042ab
void FUN_100042ab()
{
	PostThreadMessage(GetWindowThreadProcessId(g_unk0x10023160, NULL), 0x418, 0, 0);
}

// Empties the player table and forgets the players' mech copies.
// FUNCTION: NETMECHW 0x100057ba
void FUN_100057ba()
{
	MechS32 i;

	for (i = 0; i < 8; i++) {
		g_unk0x1001ce18[i] = 0xff;
	}

	FUN_1000b0c8();
}

// STUB: NETMECHW 0x100057fa
void FUN_100057fa(NetPlayer, MechS32)
{
	STUB(0x100057fa);
}

// STUB: NETMECHW 0x10005923
void FUN_10005923(MechS32)
{
	STUB(0x10005923);
}

// Lets new players join the session; returns whether they couldn't already.
// FUNCTION: NETMECHW 0x10005a55
MechS32 FUN_10005a55()
{
	if (g_unk0x10023180) {
		return FALSE;
	}

	g_unk0x1001ca90.m_directPlay->EnableNewPlayers(TRUE);
	g_unk0x10023180 = TRUE;
	return TRUE;
}

// Stops new players from joining the session; returns whether they could.
// FUNCTION: NETMECHW 0x10005a9a
MechS32 FUN_10005a9a()
{
	if (!g_unk0x10023180) {
		return FALSE;
	}

	g_unk0x1001ca90.m_directPlay->EnableNewPlayers(FALSE);
	g_unk0x10023180 = FALSE;
	return TRUE;
}

// STUB: NETMECHW 0x10005cdc
MechS32 FUN_10005cdc(MechChar*, DWORD*)
{
	STUB(0x10005cdc);
	return 0;
}

// FUNCTION: NETMECHW 0x10005e36
void FUN_10005e36()
{
	g_unk0x1002317c = 0;
}
