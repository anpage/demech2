#ifndef STATICMEM_H
#define STATICMEM_H

#include "types.h"

// The functions and globals of staticmem.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechU32 g_unk0x100a9434;

	void InitStaticMem(char* p_unk0x00);
	void* StaticPoolAlloc(MechU32 p_size, MechU32 p_tag);

#ifdef __cplusplus
}
#endif

#endif // STATICMEM_H
