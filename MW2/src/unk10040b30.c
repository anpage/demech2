#include "unk10040b30.h"

#include "decomp.h"
#include "loadres.h"
#include "simmain.h"
#include "types.h"

// Returns the positions of the cockpit's message boxes.
// FUNCTION: MW2 0x100412c8
Point* FUN_100412c8(void)
{
	return g_unk0x100a5ee8;
}

// Draws frame 0 of the "SHP" resource p_id (relative to g_unk0x100e9614) at p_x, p_y of the
// current render target.
// FUNCTION: MW2 0x10041e98
void FUN_10041e98(MechS32 p_x, MechS32 p_y, MechS32 p_id)
{
	void* shape;

	shape = FUN_1001a19f(g_unk0x100a8740, p_id + g_unk0x100e9614, g_unk0x100a8680, 0);
	if (shape) {
		DrawShapeFrame(&g_currentRenderTarget, shape, 0, p_x, p_y);
		FUN_1001a163(p_id + g_unk0x100e9614, g_unk0x100a8680);
	}
}

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

// Draws FUN_10041e98's shape at p_x, p_y of render target p_target.
// FUNCTION: MW2 0x10041f73
void FUN_10041f73(MechS32 p_x, MechS32 p_y, MechS32 p_id, RenderTarget* p_target)
{
	FUN_10041e98(p_target->m_left + p_x, p_target->m_top + p_y, p_id);
}
