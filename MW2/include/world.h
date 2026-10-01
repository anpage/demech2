#ifndef WORLD_H
#define WORLD_H

#include "types.h"

struct BwdStream;

// The functions and globals of world.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechChar g_unk0x100e9330[12];
	extern MechS32 g_unk0x100e9340;
	MechU32 FUN_1000a9c0(MechU32 p_flags);
	MechS32 BwdExecuteStream(struct BwdStream* p_stream);
	MechS32 LoadWorld(MechChar* p_name);
	void AfterWorldLoader(void);

#ifdef __cplusplus
}
#endif

#endif // WORLD_H
