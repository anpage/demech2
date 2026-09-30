#include "error.h"

#include "bwdnames.h"
#include "clock.h"
#include "decomp.h"
#include "inputmap.h"
#include "loadres.h"
#include "mw2log.h"
#include "network.h"
#include "render.h"
#include "resource.h"
#include "types.h"

// STUB: MW2 0x1003b8c0
void Error(MechS32 p_unk0x00, const char* p_unk0x04, ...)
{
	STUB(0x1003b8c0);
}

// Shuts the subsystems down before the game exits on an error.
// FUNCTION: MW2 0x1003ba07
void FUN_1003ba07(void)
{
	FUN_100586ec();
	FUN_1004fd55();
	FUN_10019cc2();
	ShutdownNetwork();
	CloseInputDevices();
	CloseMw2Log();
	ShutdownRender();
	StopTimers();
}
