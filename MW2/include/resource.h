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
	void FirstResource(void);
	void CloseResourceFile(void);
	void CachePreloads(void);

#ifdef __cplusplus
}
#endif

#endif // RESOURCE_H
