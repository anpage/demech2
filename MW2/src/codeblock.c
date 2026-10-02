/* Stubs for codeblock.asm's routines (common/src) and its data, for builds with other compilers
   (COMPAT_MODE): the VC++ 4.1 build assembles codeblock.asm with MASM 6.11 instead. */
#include "codeblock.h"

#include "compat.h"
#include "decomp.h"
#include "types.h"

undefined4 g_codeBlockRoutines[0x800];

undefined4 g_unk0x10064cd8 = 0x8000;

undefined4 g_unk0x10064cdc = 0;

undefined4 g_codeBlockVars[0x43] = {0};

MechS32 g_codeBlockClipVertices[0x600] = {0};

undefined4 g_unk0x100665ec = 0;

undefined4 g_unk0x100665f0 = 0;

undefined4 g_unk0x100665f4 = 0;

void CodeBlock(void)
{
	STUB(0x10023cf4);
}

void FUN_100286a6(undefined4 p_unk0x00, undefined4 p_unk0x04)
{
	STUB(0x1003291e);
}

MechS32 GetCodeBlock(undefined4* p_start, undefined4* p_segment)
{
	STUB(0x1003293b);
	return 0;
}

void CallCodeBlockRoutine(
	Pane* p_view,
	MechS32* p_vertices,
	MechS32 p_count,
	MechS32 p_index,
	undefined4 p_unk0x18,
	undefined4 p_unk0x1c,
	undefined4 p_unk0x20,
	undefined4 p_unk0x24
)
{
	STUB(0x10032963);
}

MechS32 FixedDiv30(MechS32 p_a, MechS32 p_b)
{
	STUB(0x100329d4);
	return 0;
}

MechS32 FixedReciprocal30(MechS32 p_value)
{
	STUB(0x10032a0c);
	return 0;
}

MechS32 CodeBlockFixedMul30(MechS32 p_a, MechS32 p_b)
{
	STUB(0x10032a3c);
	return 0;
}

void CallCodeBlockRoutineClipped(
	Pane* p_target,
	MechU32* p_vertices,
	MechS32 p_count,
	MechS32 p_index,
	undefined4 p_unk0x18,
	CodeBlockTexture* p_texture,
	MechU16* p_luma,
	undefined4 p_unk0x24
)
{
	STUB(0x10032a58);
}
