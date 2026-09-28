#ifndef UNK1002FB90_H
#define UNK1002FB90_H

#include "decomp.h"
#include "types.h"

// The functions and globals of unk1002fb90.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1002fb90(void* (*p_alloc)(undefined4), void (*p_free)(void*));
	MechS32 FUN_1002fcdc(char* p_name, MechChar p_mode);
	MechS32 FUN_1002ffc9(MechS32 p_handle);
	MechS32 FUN_1003024f(MechS32 p_handle);
	MechS32 FUN_100303b5(MechS32 p_handle, MechChar* p_name, MechU16 p_index);
	MechS32 FUN_1003066e(MechS32 p_handle, MechChar* p_name, MechU16 p_index, void* p_data);

#ifdef __cplusplus
}
#endif

#endif // UNK1002FB90_H
