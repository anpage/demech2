#include "decomp.h"
#include "linenpacket0x218.h"
#include "types.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(LinenPacket0x218, 0x218)

// GLOBAL: MW2SHELL 0x10090288
LinenPacket0x218 g_unk0x10090288;

// STUB: MW2SHELL 0x10039b50
void FUN_10039b50(BOOL p_fromSim, MechS32* p_campaign, MechU8* p_pilotChosen, char** p_scenario)
{
	STUB(0x10039b50);
}

// STUB: MW2SHELL 0x10039c92
void WriteSimHandoff(UINT p_msg, MechS32 p_campaign, MechU8 p_pilotChosen, const char* p_scenario)
{
	STUB(0x10039c92);
}
