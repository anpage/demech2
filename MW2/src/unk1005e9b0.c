#include "unk1005e9b0.h"

#include "decomp.h"
#include "environment.h"
#include "gamekeys.h"
#include "simmain.h"
#include "slateheron.h"
#include "speech.h"
#include "types.h"
#include "unk100079d0.h"

// Returns the setting p_id (0x13 the cockpit overlays, 0x3c and 0x40 two game-key toggles, 0xa6
// FUN_1007d875's state, 0xa7 outlined polygons), or 0.
// FUNCTION: MW2 0x1005e9b0
MechS32 FUN_1005e9b0(MechS32 p_id)
{
	MechS32 value;

	switch (p_id) {
	case 0xa6:
		value = FUN_1007d875(0);
		break;
	case 0xa7:
		if (g_unk0x100a6cc8.m_unk0x34 == 1) {
			value = 1;
		}
		else {
			value = 0;
		}
		break;
	case 0x13:
		value = g_unk0x100a5f18;
		break;
	case 0x40:
		value = g_unk0x100aa298;
		break;
	case 0x3c:
		value = g_unk0x100a1590;
		break;
	default:
		value = 0;
		break;
	}

	return value;
}

// Changes the setting p_id (see FUN_1005e9b0) to p_value.
// FUNCTION: MW2 0x1005eb10
void FUN_1005eb10(MechS32 p_id, MechS32 p_value)
{
	switch (p_id) {
	case 0xa6:
		FUN_1007d88a(0, p_value);
		break;
	case 0xa7:
		if (p_value) {
			g_unk0x100a6cc8.m_unk0x34 = 1;
			PlayCockpitSound(0x1b, 1);
		}
		else {
			g_unk0x100a6cc8.m_unk0x34 = 0;
		}
		break;
	case 0x13:
		g_unk0x100a5f18 = p_value;
		break;
	case 0x40:
		g_unk0x100aa298 = p_value;
		break;
	case 0x3c:
		g_unk0x100a1590 = p_value;
		break;
	default:
		break;
	}
}
