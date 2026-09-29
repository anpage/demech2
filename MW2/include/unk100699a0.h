#ifndef UNK100699A0_H
#define UNK100699A0_H

#include "ray.h"
#include "types.h"
#include "unk1003a530.h"

// The functions and globals of unk100699a0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100ad43c;

	MechS32 FUN_100699a0(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_100699da(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32* p_top);
	void FUN_10069a4b(
		ScarletOrchid0x4c* p_shape,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z,
		MechS32* p_inside,
		MechS32* p_inColumn,
		MechS32* p_top
	);
	MechS32 FUN_10069b2a(ScarletOrchid0x4c* p_shape, Ray* p_ray);
	MechS32 FUN_10069dd4(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_10069e54(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_10069e66(ScarletOrchid0x4c* p_shape, Ray* p_ray);
	MechS32 FUN_10069e78(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_10069f67(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_10069fd4(ScarletOrchid0x4c* p_shape, Ray* p_ray);
	MechS32 FUN_1006a001(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32* p_top);
	MechS32 FUN_1006a037(ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_1006a190(ScarletOrchid0x4c* p_shape, Ray* p_ray);

#ifdef __cplusplus
}
#endif

#endif // UNK100699A0_H
