#include "decomp.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <windows.h>

MechS32 FUN_10016b11(MechS32 p_index);
MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_x,
	undefined4 p_y,
	MechU32 p_flags,
	MechU32 p_unk0x14
);

// Play the second faction grid animation only when video slot zero is idle.
// FUNCTION: MW2SHELL 0x10039de0
void FUN_10039de0(MechS32 p_campaign)
{
	if (!FUN_10016b11(0)) {
		switch (p_campaign) {
		case 0:
			FUN_10017460(0, "awogrid2", 0x12f, 0x149, 0x48, 0);
			break;
		case 1:
			FUN_10017460(0, "ajfgrid2", 0x115, 0x155, 0x48, 0);
		default:
			break;
		}
	}
}

// STUB: MW2SHELL 0x10039e72
void FUN_10039e72(TMPackDataBase* p_database, MechS32 p_campaign, char** p_scenario, WPARAM p_wParam)
{
	STUB(0x10039e72);
}
