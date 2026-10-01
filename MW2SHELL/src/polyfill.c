/* Stubs for polyfill.asm's routines (common/src) and its data, for builds with other compilers
   (COMPAT_MODE): the VC++ 4.1 build assembles polyfill.asm with MASM 6.11 instead. */
#include "polyfill.h"

#include "compat.h"
#include "decomp.h"

undefined4 g_polyVars[0x74];
void (*g_polySpanRoutines[4])(void) = {FUN_1002d724, FUN_1002d457, FUN_1002d5c3, FUN_1002d2b0};

void FillPolygonFlat(PixelView* p_view, MechS32 p_count, MechS32* p_vertices)
{
	STUB(0x1002a968);
}

void FUN_1002ae41(PixelView* p_view, MechS32 p_count, MechS32* p_vertices)
{
	STUB(0x1002ae41);
}

void FUN_1002b68b(PixelView* p_view, undefined4 p_unk0x04, MechS32 p_count, MechS32* p_vertices)
{
	STUB(0x1002b68b);
}

void FUN_1002bf39(PixelView* p_view, MechS32 p_count, MechS32* p_vertices, undefined4 p_unk0x0c)
{
	STUB(0x1002bf39);
}

void FUN_1002c48d(PixelView* p_view, undefined4 p_unk0x04, MechS32 p_count, MechS32* p_vertices)
{
	STUB(0x1002c48d);
}

void SetLumaTable(MechU16* p_table)
{
	STUB(0x1002cd3d);
}

void FillPolygonTextured(PixelView* p_view, MechS32 p_count, MechS32* p_vertices, undefined4* p_unk0x10, MechS32 p_mode)
{
	STUB(0x1002cd5d);
}

void FUN_1002d2b0(void)
{
	STUB(0x1002d2b0);
}

void FUN_1002d457(void)
{
	STUB(0x1002d457);
}

void FUN_1002d5c3(void)
{
	STUB(0x1002d5c3);
}

void FUN_1002d724(void)
{
	STUB(0x1002d724);
}
