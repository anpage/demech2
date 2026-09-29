#include "unk100563d0.h"

#include "decomp.h"
#include "types.h"

DECOMP_SIZE_ASSERT(StaticPoolSize, 0x08)

// One of the pool's block tags ("SEG").
// GLOBAL: MW2 0x100a9430
MechU32 g_unk0x100a9430 = 0x474553;

// One of the pool's block tags ("TLIS").
// GLOBAL: MW2 0x100a9434
MechU32 g_unk0x100a9434 = 0x53494c54;

// The mission's static memory table, which FUN_1005640e fills.
// GLOBAL: MW2 0x100e9dc0
StaticPoolSize g_staticPoolSizes[10];

// Returns the size of the table's entry at an index, or 0.
// FUNCTION: MW2 0x100563d0
MechU32 GetStaticPoolSize(MechS32 p_index)
{
	MechU32 size;

	size = 0;
	if (p_index >= 0 && p_index < 10) {
		size = g_staticPoolSizes[p_index].m_size;
	}

	return size;
}

// Reads a mission's static memory table (seven tag and size pairs), or returns NULL.
// STUB: MW2 0x1005640e
MechS32* FUN_1005640e(char* p_mission)
{
	STUB(0x1005640e);
	return NULL;
}
