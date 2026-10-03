#ifndef RECORDSTACKS_H
#define RECORDSTACKS_H

#include "depthsort.h"
#include "projectedvertex.h"
#include "types.h"

// The functions and globals of recordstacks.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100c1a68;
	extern DepthEntry* g_unk0x100c2280;
	extern MechU8* g_unk0x100c2698;
	extern DepthEntry* g_unk0x100c269c;
	extern MechS32 g_unk0x1010b5ac;

	void FUN_1007d120(void);
	void FUN_1007d150(MechS32 p_unk0x00, MechS32 p_unk0x04);
	void FUN_1007d220(void);
	ProjectedVertex* FUN_1007d248(void);
	MechU8* FUN_1007d296(void);

#ifdef __cplusplus
}
#endif

#endif // RECORDSTACKS_H
