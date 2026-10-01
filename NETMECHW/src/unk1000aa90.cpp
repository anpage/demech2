#include "unk1000aa90.h"

#include "decomp.h"
#include "types.h"

#include <dplay.h>

DECOMP_SIZE_ASSERT(NetPlayer, 0x24)

// Copies the player p_id's entry to p_player; returns whether the player is in the table.
// STUB: NETMECHW 0x1000aeff
MechS32 FUN_1000aeff(DPID, NetPlayer*)
{
	STUB(0x1000aeff);
	return 0;
}

// Copies the entry of slot p_index to p_player; returns whether the slot is in use.
// STUB: NETMECHW 0x1000afa8
MechS32 FUN_1000afa8(MechS32, NetPlayer*)
{
	STUB(0x1000afa8);
	return 0;
}

// The slot of the player p_id in the player table, -1 if none.
// STUB: NETMECHW 0x1000b02b
MechS32 FUN_1000b02b(DPID)
{
	STUB(0x1000b02b);
	return 0;
}

// STUB: NETMECHW 0x1000b0b3
undefined4 FUN_1000b0b3()
{
	STUB(0x1000b0b3);
	return 0;
}

// STUB: NETMECHW 0x1000b3c5
void FUN_1000b3c5(MechS32)
{
	STUB(0x1000b3c5);
}
