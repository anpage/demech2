#include "unk10040b30.h"

#include "decomp.h"
#include "loadres.h"
#include "simmain.h"
#include "types.h"

// Draws the cockpit overlays enabled in the display options: the message boxes, the radar
// (FUN_100412dd), FUN_100414ab and FUN_10040cbc.
// FUNCTION: MW2 0x10040b30
void FUN_10040b30(
	RenderTarget* p_target,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18
)
{
	if (!g_unk0x100a5f18) {
		return;
	}

	if (g_unk0x100a5f24) {
		FUN_1004183a(g_unk0x100a5ee8[0].m_x, g_unk0x100a5ee8[0].m_y, p_unk0x04, p_unk0x08);
		FUN_1004161f(p_target, g_unk0x100a5ee8[0].m_x, g_unk0x100a5ee8[0].m_y, p_unk0x0c, p_unk0x10, p_unk0x14);
	}

	if (g_unk0x100a5f1c) {
		FUN_100412dd(p_target, p_unk0x10, p_unk0x14, p_unk0x18);
	}

	if (g_unk0x100a5f20) {
		FUN_100414ab(p_target);
	}

	if (g_unk0x100a5f2c) {
		FUN_10040cbc(p_target, g_unk0x100a5ee8[1].m_x, g_unk0x100a5ee8[1].m_y);
	}
}

// FUN_10040b30 with the overlays at their default places, and the radar only with p_unk0x1c.
// FUNCTION: MW2 0x10040bfd
void FUN_10040bfd(
	RenderTarget* p_target,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18,
	MechS32 p_unk0x1c
)
{
	if (!g_unk0x100a5f18) {
		return;
	}

	if (g_unk0x100a5f24) {
		FUN_1004183a(0x73, 0x10, p_unk0x04, p_unk0x08);
		FUN_1004161f(p_target, 0x73, 0x10, p_unk0x0c, p_unk0x10, p_unk0x14);
	}

	if (p_unk0x1c && g_unk0x100a5f1c) {
		FUN_100412dd(p_target, p_unk0x10, p_unk0x14, p_unk0x18);
	}

	if (g_unk0x100a5f20) {
		FUN_100414ab(p_target);
	}

	if (g_unk0x100a5f2c) {
		FUN_10040cbc(p_target, 8, 0x4a);
	}
}

// STUB: MW2 0x10040cbc
void FUN_10040cbc(RenderTarget* p_target, MechS32 p_x, MechS32 p_y)
{
	STUB(0x10040cbc);
}

// Returns the positions of the cockpit's message boxes.
// FUNCTION: MW2 0x100412c8
Point* FUN_100412c8(void)
{
	return g_unk0x100a5ee8;
}

// STUB: MW2 0x100412dd
void FUN_100412dd(RenderTarget* p_target, MechS32 p_unk0x04, MechS32 p_unk0x08, MechS32 p_unk0x0c)
{
	STUB(0x100412dd);
}

// STUB: MW2 0x100414ab
void FUN_100414ab(RenderTarget* p_target)
{
	STUB(0x100414ab);
}

// STUB: MW2 0x1004161f
void FUN_1004161f(
	RenderTarget* p_target,
	MechS32 p_x,
	MechS32 p_y,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14
)
{
	STUB(0x1004161f);
}

// STUB: MW2 0x1004183a
void FUN_1004183a(MechS32 p_x, MechS32 p_y, MechS32 p_unk0x08, MechS32 p_unk0x0c)
{
	STUB(0x1004183a);
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
