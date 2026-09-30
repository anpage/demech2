#ifndef CODEBLOCK_H
#define CODEBLOCK_H

#include "decomp.h"
#include "pixelview.h"
#include "types.h"

// The routines and data of codeblock.asm (codeblock.c in COMPAT_MODE). No other unit uses them.
#ifdef __cplusplus
extern "C"
{
#endif

	extern undefined4 g_codeBlockRoutines[0x800];
	extern undefined4 g_unk0x10064cd8;
	extern undefined4 g_unk0x10064cdc;
	extern undefined4 g_codeBlockVars[0x43];
	extern MechS32 g_codeBlockClipVertices[0x600];
	extern undefined4 g_unk0x100665ec;
	extern undefined4 g_unk0x100665f0;
	extern undefined4 g_unk0x100665f4;

	void CodeBlock(void);
	void FUN_100286a6(undefined4 p_unk0x00, undefined4 p_unk0x04);
	MechS32 GetCodeBlock(undefined4* p_start, undefined4* p_segment);
	void CallCodeBlockRoutine(
		PixelView* p_view,
		MechS32* p_vertices,
		MechS32 p_count,
		MechS32 p_index,
		undefined4 p_unk0x18,
		undefined4 p_unk0x1c,
		undefined4 p_unk0x20,
		undefined4 p_unk0x24
	);
	MechS32 FixedDiv30(MechS32 p_a, MechS32 p_b);
	MechS32 FixedReciprocal30(MechS32 p_value);
	MechS32 CodeBlockFixedMul30(MechS32 p_a, MechS32 p_b);
	void CallCodeBlockRoutineClipped(
		PixelView* p_view,
		MechU32* p_vertices,
		MechS32 p_count,
		MechS32 p_index,
		undefined4 p_unk0x18,
		void* p_texture,
		MechU16* p_luma,
		undefined4 p_unk0x24
	);

#ifdef __cplusplus
}
#endif

// codeblock.asm's routines and data. reccmp reads annotations from C sources only, so they're here,
// by name.

// GLOBAL: MW2SHELL 0x10017a7c
// g_codeBlockRoutines

// FUNCTION: MW2SHELL 0x10019a7c
// CodeBlock

// FUNCTION: MW2SHELL 0x100286a6
// FUN_100286a6

// FUNCTION: MW2SHELL 0x100286c3
// GetCodeBlock

// FUNCTION: MW2SHELL 0x100286eb
// CallCodeBlockRoutine

// FUNCTION: MW2SHELL 0x1002875c
// FixedDiv30

// FUNCTION: MW2SHELL 0x10028794
// FixedReciprocal30

// FUNCTION: MW2SHELL 0x100287c4
// CodeBlockFixedMul30

// FUNCTION: MW2SHELL 0x100287e0
// CallCodeBlockRoutineClipped

// GLOBAL: MW2SHELL 0x10064cd8
// g_unk0x10064cd8

// GLOBAL: MW2SHELL 0x10064cdc
// g_unk0x10064cdc

// GLOBAL: MW2SHELL 0x10064ce0
// g_codeBlockVars

// GLOBAL: MW2SHELL 0x10064dec
// g_codeBlockClipVertices

// GLOBAL: MW2SHELL 0x100665ec
// g_unk0x100665ec

// GLOBAL: MW2SHELL 0x100665f0
// g_unk0x100665f0

// GLOBAL: MW2SHELL 0x100665f4
// g_unk0x100665f4

#endif // CODEBLOCK_H
