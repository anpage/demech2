#ifndef SNDUNPACK_H
#define SNDUNPACK_H

#include "types.h"

// The sound-block decoder of sndunpack.asm (sndunpack.c in COMPAT_MODE).
#ifdef __cplusplus
extern "C"
{
#endif

	MechU8* DecodeSoundFrames(MechU8* p_src, MechU8* p_dst, MechU32 p_count, MechU32 p_frameSize, MechS32* p_state);

#ifdef __cplusplus
}
#endif

// sndunpack.asm's routines and data. reccmp reads annotations from C sources only, so they're
// here, by name.

// FUNCTION: MW2 0x1001a63c
// FUNCTION: MW2MATROX 0x1000a9f8
// DecodeSoundFrames

// FUNCTION: MW2 0x1001a87b
// FUNCTION: MW2MATROX 0x1000ac37
// UpsampleSoundFrame4

// FUNCTION: MW2 0x1001a8d3
// FUNCTION: MW2MATROX 0x1000ac8f
// UpsampleSoundFrame2

// GLOBAL: MW2 0x100a2f04
// GLOBAL: MW2MATROX 0x100a36b8
// g_soundUpsampleBuffer

// GLOBAL: MW2 0x100a3304
// GLOBAL: MW2MATROX 0x100a3ab8
// g_soundFrame

// GLOBAL: MW2 0x100a3705
// GLOBAL: MW2MATROX 0x100a3eb9
// g_soundDeltas

#endif // SNDUNPACK_H
