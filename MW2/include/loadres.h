#ifndef LOADRES_H
#define LOADRES_H

#include "decomp.h"
#include "types.h"

// The functions and globals of loadres.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void* FUN_1001a19f(undefined4 p_unk0x00, MechS32 p_unk0x04, const char* p_unk0x08, undefined4 p_unk0x0c);
	void* MemAlloc(MechU32 p_size);
	void* MemCopy(void* p_dst, const void* p_src, MechU32 p_size);
	void* MemSet(void* p_dst, MechS32 p_value, MechU32 p_size);
	void MemFree(void* p_block);

#ifdef __cplusplus
}
#endif

#endif // LOADRES_H
