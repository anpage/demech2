#include "gamekeys.h"

#include "decomp.h"
#include "types.h"

// A game-key toggle (FUN_1005e9b0's setting 0x40).
// GLOBAL: MW2 0x100aa298
MechS32 g_unk0x100aa298 = 0;

// STUB: MW2 0x1005c2e1
void HandleGameKeys(MechS32 p_unk0x00, MechS32 p_unk0x04, MechS32 p_unk0x08)
{
	STUB(0x1005c2e1);
}

// Performs game key p_key's action (0x3b ejects).
// STUB: MW2 0x1005c78a
void FUN_1005c78a(MechS32 p_key)
{
	STUB(0x1005c78a);
}
