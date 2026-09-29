#ifndef RESOURCE_H
#define RESOURCE_H

#include "types.h"

// The functions and globals of resource.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 MapResourceId(MechS32 p_id);
	void SetMangleBase(MechS32 p_base);
	MechS32 FUN_1005072f(void);
	MechS32 FirstResource(void);
	void CloseResourceFile(void);
	void CachePreloads(void);
	MechS32 FUN_10050862(MechS32 p_id, const char* p_type);
	void* FUN_100508c0(MechU32 p_size);
	void FUN_100508dc(void* p_block);

#ifdef __cplusplus
}
#endif

#endif // RESOURCE_H
