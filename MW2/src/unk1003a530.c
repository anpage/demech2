#include "unk1003a530.h"

#include "decomp.h"
#include "types.h"

// FUNCTION: MW2 0x1003acbe
void FUN_1003acbe(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x04)
{
	if (p_shape) {
		p_shape->m_unk0x00 = (p_shape->m_unk0x00 & ~0x7ef0) | (p_unk0x04 & 0x7ef0) | 0x8000;
	}
}

// FUNCTION: MW2 0x1003acf7
MechU32 FUN_1003acf7(ScarletOrchid0x4c* p_shape)
{
	if (p_shape) {
		return p_shape->m_unk0x00 & 0x7ef0;
	}
	else {
		return 0;
	}
}

// FUNCTION: MW2 0x1003ad2d
void FUN_1003ad2d(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x02)
{
	if (p_shape) {
		p_shape->m_unk0x02 = p_unk0x02;
	}
}

// FUNCTION: MW2 0x1003ad4c
void FUN_1003ad4c(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x16)
{
	p_shape->m_unk0x16 = p_unk0x16;
}

// FUNCTION: MW2 0x1003ad62
void FUN_1003ad62(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x14)
{
	p_shape->m_unk0x14 = p_unk0x14;
}

// FUNCTION: MW2 0x1003ad78
MechU32 FUN_1003ad78(ScarletOrchid0x4c* p_shape)
{
	return p_shape->m_unk0x02;
}

// FUNCTION: MW2 0x1003ad93
MechU32 FUN_1003ad93(ScarletOrchid0x4c* p_shape)
{
	return p_shape->m_unk0x16;
}

// FUNCTION: MW2 0x1003adae
MechU32 FUN_1003adae(ScarletOrchid0x4c* p_shape)
{
	return p_shape->m_unk0x14;
}

// FUNCTION: MW2 0x1003b6e5
struct AmberWillow0x7c* FUN_1003b6e5(ScarletOrchid0x4c* p_shape)
{
	return p_shape->m_unk0x18;
}

// FUNCTION: MW2 0x1003b6fb
void FUN_1003b6fb(ScarletOrchid0x4c* p_shape, struct AmberWillow0x7c* p_unk0x18)
{
	p_shape->m_unk0x18 = p_unk0x18;
}
