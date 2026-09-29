#ifndef FADEPAL_H
#define FADEPAL_H

#include "decomp.h"
#include "types.h"

// The functions of fadepal.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1004c890(undefined4 p_unk0x00, MechS32 p_unk0x04, undefined4 p_unk0x08);
	void FUN_1004c8bd(MechU32 p_target, undefined4 p_unk0x04, undefined4* p_unk0x08, undefined4 p_unk0x0c);
	void FUN_1004ca0d(void);
	void FUN_1004ca29(MechU32 p_level);
	void FadeToEndPalette(MechS32 p_alternate);
	void PlayPlayerHitFeedback(MechS32 p_x, MechS32 p_y, MechS32 p_z);

#ifdef __cplusplus
}
#endif

#endif // FADEPAL_H
