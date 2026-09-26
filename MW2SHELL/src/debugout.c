#include "decomp.h"
#include "types.h"

#include <string.h>

// FUNCTION: MW2SHELL 0x10017710
void FUN_10017710(void)
{
	// The ranges overlap (dst 0xb0000 < src 0xb00a0), so this is memmove: /Oi would expand
	// memcpy inline even at this constant size.
	memmove((void*) 0xb0000, (void*) 0xb00a0, 0xf00);
}

// STUB: MW2SHELL 0x100178cc
void FUN_100178cc(MechS32 p_mode)
{
	STUB(0x100178cc);
}

// STUB: MW2SHELL 0x10017982
void DebugPrintInternal(MechChar* p_message)
{
	STUB(0x10017982);
}
