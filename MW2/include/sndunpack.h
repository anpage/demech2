#ifndef SNDUNPACK_H
#define SNDUNPACK_H

#include "compat.h"
#include "types.h"

// The sound-block decoder of sndunpack.asm (sndunpack.c in COMPAT_MODE).
#ifdef __cplusplus
extern "C"
{
#endif

	MechU8* __cdecl DecodeSoundFrames(
		MechU8* p_src,
		MechU8* p_dst,
		MechU32 p_count,
		MechU32 p_frameSize,
		MechS32* p_state
	);

#ifdef __cplusplus
}
#endif

// sndunpack.asm's routines and data. reccmp reads annotations from C sources only, so they're
// here, by name.

// FUNCTION: MW2 0x1001a63c
// FUNCTION: MW2MATROX 0x1000a9f8
// FUNCTION: MW23DFX 0x1003356c
// FUNCTION: MW2ATI 0x100321b0
// FUNCTION: MW2S3 0x10048580
// FUNCTION: MW2POWERVR 0x1002cc30
// DecodeSoundFrames

// FUNCTION: MW2 0x1001a87b
// FUNCTION: MW2MATROX 0x1000ac37
// FUNCTION: MW23DFX 0x100337ab
// FUNCTION: MW2ATI 0x100323ef
// FUNCTION: MW2S3 0x100487bf
// FUNCTION: MW2POWERVR 0x1002ce6f
// UpsampleSoundFrame4

// FUNCTION: MW2 0x1001a8d3
// FUNCTION: MW2MATROX 0x1000ac8f
// FUNCTION: MW23DFX 0x10033803
// FUNCTION: MW2ATI 0x10032447
// FUNCTION: MW2S3 0x10048817
// FUNCTION: MW2POWERVR 0x1002cec7
// UpsampleSoundFrame2

// GLOBAL: MW2 0x100a2f04
// GLOBAL: MW2MATROX 0x100a36b8
// GLOBAL: MW23DFX 0x1024f780
// GLOBAL: MW2ATI 0x10183a90
// GLOBAL: MW2S3 0x100771f8
// GLOBAL: MW2POWERVR 0x101aeaf8
// g_soundUpsampleBuffer

// GLOBAL: MW2 0x100a3304
// GLOBAL: MW2MATROX 0x100a3ab8
// GLOBAL: MW23DFX 0x1024fb80
// GLOBAL: MW2ATI 0x10183e90
// GLOBAL: MW2S3 0x100775f8
// GLOBAL: MW2POWERVR 0x101aeef8
// g_soundFrame

// GLOBAL: MW2 0x100a3705
// GLOBAL: MW2MATROX 0x100a3eb9
// GLOBAL: MW23DFX 0x1024ff81
// GLOBAL: MW2ATI 0x10184291
// GLOBAL: MW2S3 0x100779f9
// GLOBAL: MW2POWERVR 0x101af2f9
// g_soundDeltas

#endif // SNDUNPACK_H
