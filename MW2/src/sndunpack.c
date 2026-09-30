/* Stubs for sndunpack.asm's routines and its data, for builds with other compilers (COMPAT_MODE):
   the VC++ 4.1 build assembles sndunpack.asm with MASM 6.11 instead. */
#include "sndunpack.h"

#include "decomp.h"
#include "types.h"

MechU8 g_unk0x100a2f04[0x400] = {0};
MechU8 g_unk0x100a3304[0x401] = {0};
MechU8 g_unk0x100a3705[0x43] = {0};

MechU8* FUN_1001a63c(MechU8* p_src, MechU8* p_dst, MechU32 p_count, MechU32 p_frameSize, MechS32* p_state)
{
	STUB(0x1001a63c);
	return NULL;
}
