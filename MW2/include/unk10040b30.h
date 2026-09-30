#ifndef UNK10040B30_H
#define UNK10040B30_H

#include "mech.h"
#include "point.h"
#include "rendertarget.h"
#include "types.h"

// The functions and globals of unk10040b30.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10040b30(
		RenderTarget* p_target,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		MechS32 p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_unk0x14,
		MechS32 p_unk0x18
	);
	void FUN_10040bfd(
		RenderTarget* p_target,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		MechS32 p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_unk0x14,
		MechS32 p_unk0x18,
		MechS32 p_unk0x1c
	);
	void FUN_10040cbc(RenderTarget* p_target, MechS32 p_x, MechS32 p_y);
	Point* FUN_100412c8(void);
	void FUN_100412dd(RenderTarget* p_target, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c);
	void FUN_100414ab(RenderTarget* p_target);
	void FUN_1004161f(
		RenderTarget* p_target,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_unk0x14
	);
	void FUN_1004183a(MechS32 p_x, MechS32 p_y, MechS32 p_unk0x08, MechS32 p_unk0x0c);
	void FUN_10041e98(MechS32 p_x, MechS32 p_y, MechS32 p_id);
	void FUN_10041f06(MechS32 p_x, MechS32 p_y, MechS32 p_id, RenderTarget* p_target);
	void FUN_10041f73(MechS32 p_x, MechS32 p_y, MechS32 p_id, RenderTarget* p_target);

#ifdef __cplusplus
}
#endif

#endif // UNK10040B30_H
