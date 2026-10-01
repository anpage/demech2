#include "unk10003660.h"

#include "decomp.h"
#include "netlaunchinfo.h"
#include "types.h"

#include <windows.h>

// GLOBAL: NETMECHW 0x1001ce2c
HINSTANCE g_hInstance;

// NetMech's lobby. MECH2.EXE calls it with the launch record to fill in, runs the simulator
// when it returns nonzero, and calls it again when the mission is over.
// STUB: NETMECHW 0x10003684
extern "C" MechS32 __stdcall Launcher(NetLaunchInfo*)
{
	STUB(0x10003684);
	return 0;
}
