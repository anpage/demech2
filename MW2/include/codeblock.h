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

// The functions and globals of codeblock.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

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

#endif // CODEBLOCK_H
