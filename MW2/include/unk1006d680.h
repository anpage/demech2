#ifndef UNK1006D680_H
#define UNK1006D680_H

#include "shapelisthead.h"
#include "types.h"
#include "unk1003a530.h"

// The functions and globals of unk1006d680.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern ScarletOrchid0x4c* g_unk0x100ad5e8;
	extern ScarletOrchid0x4c* g_unk0x100ad5ec;

	extern ShapeListHead g_unk0x100bef10;
	extern ShapeListHead g_unk0x100bef28;
	extern ShapeListHead g_unk0x100bef40;

	void FUN_1006d680(void);
	void FUN_1006d732(ScarletOrchid0x4c* p_shape);
	void FUN_1006d7fb(ScarletOrchid0x4c* p_shape);
	void FUN_1006d88a(ScarletOrchid0x4c* p_shape);
	void FUN_1006d8d1(ScarletOrchid0x4c* p_shape);
	void FUN_1006d989(ScarletOrchid0x4c* p_shape);
	void FUN_1006da2d(ScarletOrchid0x4c* p_shape);
	void FUN_1006daa0(ScarletOrchid0x4c* p_shape);
	void FUN_1006db28(void);
	void FUN_1006dbe2(ScarletOrchid0x4c* p_shape);
	void FUN_1006dc3b(ScarletOrchid0x4c* p_shape, ScarletOrchid0x4c* p_list);
	void FUN_1006dc7d(MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // UNK1006D680_H
