#ifndef GEOCACHE_H
#define GEOCACHE_H

#include "decomp.h"
#include "types.h"

// The functions and globals of geocache.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void SetExplosionChunks(undefined4 p_unk0x00, MechS32 p_explosionChunks);
	void FUN_10020c6f(MechS32 p_id, MechS32* p_x, MechS32* p_y, MechS32* p_z);

#ifdef __cplusplus
}
#endif

#endif // GEOCACHE_H
