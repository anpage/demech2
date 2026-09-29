#ifndef UNK10004F40_H
#define UNK10004F40_H

#include "decomp.h"
#include "types.h"

// The functions and globals of unk10004f40.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechChar* FUN_10004f40(MechS32 p_ticks);
	MechChar* FUN_10004ff5(MechS32 p_seconds);
	MechS32 FUN_1000507d(const MechChar* p_text, undefined4 p_font);

#ifdef __cplusplus
}
#endif

#endif // UNK10004F40_H
