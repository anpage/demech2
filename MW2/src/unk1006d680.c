#include "unk1006d680.h"

#include "decomp.h"
#include "types.h"
#include "unk1003a530.h"

#include <stddef.h>

// GLOBAL: MW2 0x100ad5e8
ScarletOrchid0x4c* g_unk0x100ad5e8 = NULL;

// GLOBAL: MW2 0x100ad5ec
ScarletOrchid0x4c* g_unk0x100ad5ec = NULL;

// GLOBAL: MW2 0x100bef18
ScarletOrchid0x4c* g_unk0x100bef18;

// GLOBAL: MW2 0x100bef30
ScarletOrchid0x4c* g_unk0x100bef30;

// STUB: MW2 0x1006d7fb
void FUN_1006d7fb(ScarletOrchid0x4c* p_shape)
{
	STUB(0x1006d7fb);
}

// STUB: MW2 0x1006d88a
void FUN_1006d88a(ScarletOrchid0x4c* p_shape)
{
	STUB(0x1006d88a);
}

// STUB: MW2 0x1006d8d1
void FUN_1006d8d1(ScarletOrchid0x4c* p_shape)
{
	STUB(0x1006d8d1);
}

// STUB: MW2 0x1006d989
void FUN_1006d989(ScarletOrchid0x4c* p_shape)
{
	STUB(0x1006d989);
}

// STUB: MW2 0x1006da2d
void FUN_1006da2d(ScarletOrchid0x4c* p_shape)
{
	STUB(0x1006da2d);
}

// STUB: MW2 0x1006daa0
void FUN_1006daa0(ScarletOrchid0x4c* p_shape)
{
	STUB(0x1006daa0);
}

// FUNCTION: MW2 0x1006dc7d
void FUN_1006dc7d(MechS32 p_enable)
{
	ScarletOrchid0x4c* shape;
	ScarletOrchid0x4c* next;

	if (p_enable) {
		for (shape = g_unk0x100bef30; shape != NULL; shape = next) {
			next = shape->m_unk0x08;
			if ((shape->m_unk0x02 & 0xf0) == 0xc0) {
				FUN_1006daa0(shape);
				FUN_1006d8d1(shape);
			}
		}
	}
	else {
		for (shape = g_unk0x100bef18; shape != NULL; shape = next) {
			next = shape->m_unk0x08;
			if ((shape->m_unk0x02 & 0xf0) == 0xc0) {
				FUN_1006da2d(shape);
				FUN_1006d989(shape);
			}
		}
	}
}
