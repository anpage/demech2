#include "decomp.h"
#include "types.h"

// Widens p_flags: any of the bits 0x730 sets them all, as does either of the bits 3.
// FUNCTION: MW2 0x1000a9c0
MechU32 FUN_1000a9c0(MechU32 p_flags)
{
	if (p_flags & 0x730) {
		p_flags |= 0x730;
	}

	if (p_flags & 3) {
		p_flags |= 3;
	}

	return p_flags;
}

// STUB: MW2 0x1000d30f
void LoadWorld(char* p_unk0x00)
{
	STUB(0x1000d30f);
}

// STUB: MW2 0x1000d4a6
void AfterWorldLoader(void)
{
	STUB(0x1000d4a6);
}
