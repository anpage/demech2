#include "unk10003660.h"

#include "chatlog.h"
#include "decomp.h"
#include "netlaunchinfo.h"
#include "sessionlist.h"
#include "types.h"
#include "unk10006b20.h"

#include <windows.h>

// Guards the player table.
// GLOBAL: NETMECHW 0x1001ca78
CRITICAL_SECTION g_unk0x1001ca78;

// GLOBAL: NETMECHW 0x1001ca90
IronLantern0x160 g_unk0x1001ca90;

// Constructed after g_unk0x1001ca90, by the unit's static initializer.
// GLOBAL: NETMECHW 0x1001c8b8
ChatLog g_chatLog;

// A counter per player slot, for the names of the players' mech copies (CopyPlayerMech).
// GLOBAL: NETMECHW 0x1001ce18
MechU8 g_unk0x1001ce18[8];

// GLOBAL: NETMECHW 0x1001ce2c
HINSTANCE g_hInstance;

// The application GUID of NetMech's DirectPlay sessions.
// GLOBAL: NETMECHW 0x100230f8
GUID g_unk0x100230f8 = {0x528519a0, 0x1001, 0x11cf, {0x80, 0x5c, 0x00, 0xaa, 0x00, 0x44, 0x43, 0x1f}};

// GLOBAL: NETMECHW 0x10023120
SessionList* g_sessionList = NULL;

// The lobby window.
// GLOBAL: NETMECHW 0x10023160
HWND g_unk0x10023160 = NULL;

// NetMech's lobby. MECH2.EXE calls it with the launch record to fill in, runs the simulator
// when it returns nonzero, and calls it again when the mission is over.
// STUB: NETMECHW 0x10003684
extern "C" MechS32 __stdcall Launcher(NetLaunchInfo*)
{
	STUB(0x10003684);
	return 0;
}

// STUB: NETMECHW 0x10005cdc
MechS32 FUN_10005cdc(MechChar*, DWORD*)
{
	STUB(0x10005cdc);
	return 0;
}
