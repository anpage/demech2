#include "unk10040b30.h"

#include "decomp.h"
#include "loadres.h"
#include "simmain.h"
#include "types.h"

// Draws frame 0 of the "SHP" resource p_id (relative to g_unk0x100e9614) at p_x, p_y.
// Operand order: p_id + g_unk0x100e9614 loads p_id first in the original.
// FUNCTION: MW2 0x10041f06
void FUN_10041f06(MechS32 p_x, MechS32 p_y, MechS32 p_id, RenderTarget* p_target)
{
	void* shape;

	shape = FUN_1001a19f(g_unk0x100a8740, p_id + g_unk0x100e9614, g_unk0x100a8680, 0);
	if (shape) {
		DrawShapeFrame(p_target, shape, 0, p_x, p_y);
		FUN_1001a163(p_id + g_unk0x100e9614, g_unk0x100a8680);
	}
}
