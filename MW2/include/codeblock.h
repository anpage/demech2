#ifndef CODEBLOCK_H
#define CODEBLOCK_H

#include "rendertarget.h"
#include "types.h"

// The texture CallCodeBlockRoutineClipped's routines map: a table of p_rowCount row pointers
// (FUN_1006dd50 sets both counts to the texture's height).
// SIZE 0xc
typedef struct CodeBlockTexture {
	MechU8** m_rows;    // 0x00
	MechS32 m_rowCount; // 0x04
	MechS32 m_height;   // 0x08
} CodeBlockTexture;

// The routines of codeblock.asm (common/src, shared with the shell; codeblock.c in COMPAT_MODE)
// that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 GetCodeBlock(undefined4* p_start, undefined4* p_segment);
	void CallCodeBlockRoutineClipped(
		RenderTarget* p_target,
		MechU32* p_vertices,
		MechS32 p_count,
		MechS32 p_index,
		undefined4 p_unk0x18,
		CodeBlockTexture* p_texture,
		MechU16* p_luma,
		undefined4 p_unk0x24
	);

#ifdef __cplusplus
}
#endif

// codeblock.asm's routines and data. reccmp reads annotations from C sources only, so they're
// here, by name.

// GLOBAL: MW2 0x10021cf4
// g_codeBlockRoutines

// FUNCTION: MW2 0x10023cf4
// CodeBlock

// FUNCTION: MW2 0x1003291e
// FUN_100286a6

// FUNCTION: MW2 0x1003293b
// GetCodeBlock

// FUNCTION: MW2 0x10032963
// CallCodeBlockRoutine

// FUNCTION: MW2 0x100329d4
// FixedDiv30

// FUNCTION: MW2 0x10032a0c
// FixedReciprocal30

// FUNCTION: MW2 0x10032a3c
// CodeBlockFixedMul30

// FUNCTION: MW2 0x10032a58
// CallCodeBlockRoutineClipped

// GLOBAL: MW2 0x100a3958
// g_unk0x10064cd8

// GLOBAL: MW2 0x100a395c
// g_unk0x10064cdc

// GLOBAL: MW2 0x100a3960
// g_codeBlockVars

// GLOBAL: MW2 0x100a3a6c
// g_codeBlockClipVertices

// GLOBAL: MW2 0x100a526c
// g_unk0x100665ec

// GLOBAL: MW2 0x100a5270
// g_unk0x100665f0

// GLOBAL: MW2 0x100a5274
// g_unk0x100665f4

#endif // CODEBLOCK_H
