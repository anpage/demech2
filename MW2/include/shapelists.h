#ifndef SHAPELISTS_H
#define SHAPELISTS_H

#include "shape.h"
#include "shapelisthead.h"
#include "types.h"

// The functions and globals of shapelists.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern Shape* g_unk0x100ad5e8;
	extern Shape* g_unk0x100ad5ec;

	extern ShapeListHead g_unk0x100bef10;
	extern ShapeListHead g_unk0x100bef28;
	extern ShapeListHead g_unk0x100bef40;

	void FUN_1006d680(void);
	void FUN_1006d732(Shape* p_shape);
	void FUN_1006d7fb(Shape* p_shape);
	void FUN_1006d88a(Shape* p_shape);
	void FUN_1006d8d1(Shape* p_shape);
	void FUN_1006d989(Shape* p_shape);
	void FUN_1006da2d(Shape* p_shape);
	void FUN_1006daa0(Shape* p_shape);
	void FUN_1006db28(void);
	void FUN_1006dbe2(Shape* p_shape);
	void FUN_1006dc3b(Shape* p_shape, Shape* p_list);
	void FUN_1006dc7d(MechS32 p_enable);

#ifdef __cplusplus
}
#endif

#endif // SHAPELISTS_H
