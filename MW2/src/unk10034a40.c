#include "unk10034a40.h"

#include "decomp.h"
#include "types.h"
#include "unk1003a530.h"
#include "unk1006d680.h"

// FUNCTION: MW2 0x10034a40
void FUN_10034a40(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x24)
{
	p_shape->m_unk0x24 = p_unk0x24;

	if (p_unk0x24 == 4) {
		FUN_1006d989(p_shape);
	}
	else {
		FUN_1006d8d1(p_shape);
	}
}
