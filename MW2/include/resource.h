#ifndef RESOURCE_H
#define RESOURCE_H

#include "types.h"

// The functions and globals of resource.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FirstResource(void);
	void CloseResourceFile(void);
	void CachePreloads(void);
	void DebugLog(const MechChar* p_format, ...);

#ifdef __cplusplus
}
#endif

#endif // RESOURCE_H
