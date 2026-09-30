#ifndef SNDUNPACK_H
#define SNDUNPACK_H

#include "types.h"

// The sound-block decoder of sndunpack.asm (sndunpack.c in COMPAT_MODE).
#ifdef __cplusplus
extern "C"
{
#endif

	MechU8* FUN_1001a63c(MechU8* p_src, MechU8* p_dst, MechU32 p_count, MechU32 p_frameSize, MechS32* p_state);

#ifdef __cplusplus
}
#endif

// sndunpack.asm's routines and data. reccmp reads annotations from C sources only, so they're
// here, by name.

// FUNCTION: MW2 0x1001a63c
// FUN_1001a63c

// FUNCTION: MW2 0x1001a87b
// FUN_1001a87b

// FUNCTION: MW2 0x1001a8d3
// FUN_1001a8d3

// GLOBAL: MW2 0x100a2f04
// g_unk0x100a2f04

// GLOBAL: MW2 0x100a3304
// g_unk0x100a3304

// GLOBAL: MW2 0x100a3705
// g_unk0x100a3705

#endif // SNDUNPACK_H
