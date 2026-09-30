#ifndef CHEATCODES_H
#define CHEATCODES_H

#include "types.h"

// The functions and globals of cheatcodes.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechChar g_unk0x100e9620[0xf];

	MechS32 FUN_1005b7c0(MechS16 p_key);
	MechS32 FUN_1005b807(MechChar* p_code);
	void FUN_1005bf7c(MechChar* p_char);

#ifdef __cplusplus
}
#endif

#endif // CHEATCODES_H
