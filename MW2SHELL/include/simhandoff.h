#ifndef SIMHANDOFF_H
#define SIMHANDOFF_H

#include "linenpacket0x218.h"
#include "types.h"

#include <windows.h>

// The functions and globals of simhandoff.cpp that other units use.
extern MechChar g_unk0x1006a550[0x10];
extern LinenPacket0x218 g_unk0x10090288;

void FUN_10039b50(BOOL p_fromSim, MechS32* p_campaign, MechU8* p_pilotChosen, char** p_scenario);
void WriteSimHandoff(UINT p_msg, MechS32 p_campaign, MechU8 p_pilotChosen, const char* p_scenario);

#endif // SIMHANDOFF_H
