/* Stubs for polyfill.asm's routines (common/src) and its data, for builds with other compilers
   (COMPAT_MODE): the VC++ 4.1 build assembles polyfill.asm with MASM 6.11 instead. */
#include "polyfill.h"

#include "compat.h"
#include "decomp.h"

undefined4 g_polyVars[0x74];
void (*g_polySpanRoutines[4])(void) = {FUN_1002d724, FUN_1002d457, FUN_1002d5c3, FUN_1002d2b0};

void FillPolygonFlat(RenderTarget* p_target, MechS32 p_count, MechU32* p_points)
{
	STUB(0x10036918);
}

void FUN_1002ae41(RenderTarget* p_target, MechS32 p_count, MechU32* p_points)
{
	STUB(0x10036df1);
}

void FUN_1002b68b(RenderTarget* p_target, MechS32 p_unk0x04, MechS32 p_count, MechU32* p_points)
{
	STUB(0x1003763b);
}

void FUN_1002bf39(RenderTarget* p_target, MechS32 p_count, MechU32* p_points, undefined4 p_unk0x0c)
{
	STUB(0x10037ee9);
}

void FUN_1002c48d(RenderTarget* p_target, MechS32 p_unk0x04, MechS32 p_count, MechU32* p_points)
{
	STUB(0x1003843d);
}

void SetLumaTable(MechU16* p_table)
{
	STUB(0x10038ced);
}

void FillPolygonTextured(
	RenderTarget* p_target,
	MechS32 p_count,
	MechU32* p_points,
	PixelBuffer* p_source,
	MechS32 p_mode
)
{
	STUB(0x10038d0d);
}

void FUN_1002d2b0(void)
{
	STUB(0x10039260);
}

void FUN_1002d457(void)
{
	STUB(0x10039407);
}

void FUN_1002d5c3(void)
{
	STUB(0x10039573);
}

void FUN_1002d724(void)
{
	STUB(0x100396d4);
}
