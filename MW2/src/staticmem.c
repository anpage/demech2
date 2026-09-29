#include "staticmem.h"

#include "decomp.h"
#include "types.h"

// One of the pool's block tags ("TLIS").
// GLOBAL: MW2 0x100a9434
MechU32 g_unk0x100a9434 = 0x53494c54;

// STUB: MW2 0x100498b0
void InitStaticMem(char* p_unk0x00)
{
	STUB(0x100498b0);
}

// STUB: MW2 0x10049afb
void* StaticPoolAlloc(MechU32 p_size, MechU32 p_tag)
{
	STUB(0x10049afb);
	return NULL;
}
