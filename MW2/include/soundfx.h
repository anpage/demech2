#ifndef SOUNDFX_H
#define SOUNDFX_H

#include "decomp.h"
#include "types.h"

// The functions and globals of soundfx.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1007eb23(
		undefined4 p_unk0x00,
		undefined4 p_unk0x04,
		undefined4 p_unk0x08,
		undefined4 p_unk0x0c,
		undefined4 p_unk0x10
	);
	void FUN_1007eb64(
		undefined4 p_unk0x00,
		undefined4 p_unk0x04,
		undefined4 p_unk0x08,
		undefined4 p_unk0x0c,
		undefined4 p_unk0x10,
		undefined4 p_unk0x14
	);
	void FUN_1007ebd1(MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32 p_sound, undefined4 p_unk0x10);

#ifdef __cplusplus
}
#endif

#endif // SOUNDFX_H
