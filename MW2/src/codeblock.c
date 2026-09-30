/* Hand-written assembly (originally a MASM object): MW2's copy of the shell's codeblock.c, and
   like it transcribed as naked functions. The object starts at 0x10021cf4, right after
   unk10021460.c, with the 0x2000-byte routine table and the self-modifying code block
   (0x10023cf4 onward, through 0x1003291d); only the routines after them are transcribed here.
   MASM pads between some routines with cs: mov eax, eax, which the inline assembler can't
   prefix, so the padding is _emit bytes at the end of the routine before it.

   Not yet transcribed: the table and the code block, declared as data below so the routines can
   reference them. Unlike the shell, MW2 reaches the block: FUN_1006dd50 draws textured polygons
   through CallCodeBlockRoutineClipped. */
#include "codeblock.h"

#include "compat.h"
#include "decomp.h"
#include "types.h"

#pragma warning(disable : 4102) /* the labels mark the targets of the _emit short jumps */
#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// The table of routines the code block dispatches through: 0x800 entries, indexed by the drawing
// mode's flags. It lives in .text in the original.
// GLOBAL: MW2 0x10021cf4
undefined4 g_codeBlockRoutines[0x800];

// The start of the self-modifying code block (through 0x1003291d), declared as data.
// GLOBAL: MW2 0x10023cf4
undefined g_codeBlock[1];

// The pair FUN_1003291e sets. Only the untranscribed code block reads them.
// GLOBAL: MW2 0x100a3958
undefined4 g_unk0x100a3958 = 0x8000;

// GLOBAL: MW2 0x100a395c
undefined4 g_unk0x100a395c = 0;

// The code block's working variables, as in the shell: the view (+0x8, +0x10), the vertex list's
// bounds (+0x14, +0x18) and the routine's arguments (+0x98, +0x100 to +0x108).
// GLOBAL: MW2 0x100a3960
undefined4 g_codeBlockVars[0x43] = {0};

// CallCodeBlockRoutineClipped's clipped polygon: up to 0x100 vertices of six dwords. The clip
// passes alternate between it and the caller's vertex list.
// GLOBAL: MW2 0x100a3a6c
MechS32 g_codeBlockClipVertices[0x600] = {0};

// Flags CallCodeBlockRoutineClipped derives from the routine index before clipping, read by its
// edge intersections: 0x400 set; both 0x400 and 0x200 set; a mode among 0x40, 0xc0 and 0x100 in
// bits 6-8, or 4 or 7 in bits 0-2.
// GLOBAL: MW2 0x100a526c
undefined4 g_unk0x100a526c = 0;

// GLOBAL: MW2 0x100a5270
undefined4 g_unk0x100a5270 = 0;

// GLOBAL: MW2 0x100a5274
undefined4 g_unk0x100a5274 = 0;

// Stores its arguments in g_unk0x100a3958 and g_unk0x100a395c. Nothing calls it, and only the
// untranscribed code block reads the pair, so it stays unnamed with them.
#ifdef COMPAT_MODE
void FUN_1003291e(undefined4 p_unk0x00, undefined4 p_unk0x04)
{
	STUB(0x1003291e);
}
#else
// FUNCTION: MW2 0x1003291e
__declspec(naked) void FUN_1003291e(undefined4 p_unk0x00, undefined4 p_unk0x04)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov eax, dword ptr [ebp+0x8]
		mov dword ptr [g_unk0x100a3958], eax
		mov eax, dword ptr [ebp+0xc]
		mov dword ptr [g_unk0x100a395c], eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Stores the self-modifying code block's start and the code segment, and returns the block's
// size (it ends where FUN_1003291e starts).
#ifdef COMPAT_MODE
MechS32 GetCodeBlock(undefined4* p_start, undefined4* p_segment)
{
	STUB(0x1003293b);
	return 0;
}
#else
// FUNCTION: MW2 0x1003293b
__declspec(naked) MechS32 GetCodeBlock(undefined4* p_start, undefined4* p_segment)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		xor ebx, ebx
		mov eax, dword ptr [ebp+0xc]
		mov bx, cs
		mov dword ptr [eax], ebx
		mov eax, dword ptr [ebp+0x8]
		mov ebx, offset g_codeBlock
		mov dword ptr [eax], ebx
		mov eax, offset FUN_1003291e
		sub eax, ebx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Sets up the working variables for a list of six-dword vertices and calls the table's routine
// p_index, if it has one.
#ifdef COMPAT_MODE
void CallCodeBlockRoutine(
	RenderTarget* p_view,
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
#else
// FUNCTION: MW2 0x10032963
__declspec(naked) void CallCodeBlockRoutine(
	RenderTarget* p_view,
	MechS32* p_vertices,
	MechS32 p_count,
	MechS32 p_index,
	undefined4 p_unk0x18,
	undefined4 p_unk0x1c,
	undefined4 p_unk0x20,
	undefined4 p_unk0x24
)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		push ds
		pop es
		mov edi, dword ptr [ebp+0x8]
		mov edi, dword ptr [edi]
		mov eax, dword ptr [edi+0x4]
		inc eax
		mov dword ptr [g_codeBlockVars+0x8], eax
		mov eax, dword ptr [edi]
		mov dword ptr [g_codeBlockVars+0x10], eax
		mov ebx, dword ptr [ebp+0xc]
		mov eax, dword ptr [ebp+0x10]
		shl eax, 0x3
		mov edx, eax
		shl eax, 0x1
		add eax, edx
		add eax, ebx
		mov dword ptr [g_codeBlockVars+0x14], ebx
		mov dword ptr [g_codeBlockVars+0x18], eax
		mov eax, dword ptr [ebp+0x1c]
		mov dword ptr [g_codeBlockVars+0x100], eax
		mov eax, dword ptr [ebp+0x20]
		mov dword ptr [g_codeBlockVars+0x104], eax
		mov eax, dword ptr [ebp+0x24]
		mov dword ptr [g_codeBlockVars+0x108], eax
		mov eax, dword ptr [ebp+0x18]
		mov dword ptr [g_codeBlockVars+0x98], eax
		mov ebx, dword ptr [ebp+0x14]
		mov eax, dword ptr [g_codeBlockRoutines+ebx*0x4]
		cmp eax, 0x0
		_emit 0x74 /* je jmp_100329ce */
		_emit 0x02
		call eax
jmp_100329ce:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Divides two fixed-point values into a 2.30 result, rounded: (p_a << 30) / p_b.
#ifdef COMPAT_MODE
MechS32 FixedDiv30(MechS32 p_a, MechS32 p_b)
{
	STUB(0x100329d4);
	return 0;
}
#else
// FUNCTION: MW2 0x100329d4
__declspec(naked) MechS32 FixedDiv30(MechS32 p_a, MechS32 p_b)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		sub ecx, ecx
		mov eax, dword ptr [ebp+0x8]
		and eax, eax
		_emit 0x79 /* jns jmp_100329e4 */
		_emit 0x03
		inc ecx
		neg eax
jmp_100329e4:
		mov edx, eax
		sar edx, 0x2
		shl eax, 0x1e
		mov ebx, dword ptr [ebp+0xc]
		and ebx, ebx
		_emit 0x79 /* jns jmp_100329f6 */
		_emit 0x03
		dec ecx
		neg ebx
jmp_100329f6:
		div ebx
		shr ebx, 0x1
		adc ebx, 0x0
		dec ebx
		cmp ebx, edx
		adc eax, 0x0
		and ecx, ecx
		_emit 0x74 /* je jmp_10032a09 */
		_emit 0x02
		neg eax
jmp_10032a09:
		pop ebx
		leave
		ret
	}
}
#endif

// The rounded reciprocal of p_value: 2^46 / p_value.
#ifdef COMPAT_MODE
MechS32 FixedReciprocal30(MechS32 p_value)
{
	STUB(0x10032a0c);
	return 0;
}
#else
// FUNCTION: MW2 0x10032a0c
__declspec(naked) MechS32 FixedReciprocal30(MechS32 p_value)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		sub ecx, ecx
		mov edx, 0x4000
		xor eax, eax
		mov ebx, dword ptr [ebp+0x8]
		and ebx, ebx
		_emit 0x79 /* jns jmp_10032a23 */
		_emit 0x03
		dec ecx
		neg ebx
jmp_10032a23:
		div ebx
		shr ebx, 0x1
		adc ebx, 0x0
		dec ebx
		cmp ebx, edx
		adc eax, 0x0
		and ecx, ecx
		_emit 0x74 /* je jmp_10032a36 */
		_emit 0x02
		neg eax
jmp_10032a36:
		pop ebx
		leave
		ret
		_emit 0x2e /* mov eax, eax with a segment prefix: MASM alignment padding */
		_emit 0x8b
		_emit 0xc0
	}
}
#endif

// Multiplies two 2.30 fixed-point values, rounded. fixedmul30.c's FixedMul30 has the name in MW2.
#ifdef COMPAT_MODE
MechS32 CodeBlockFixedMul30(MechS32 p_a, MechS32 p_b)
{
	STUB(0x10032a3c);
	return 0;
}
#else
// FUNCTION: MW2 0x10032a3c
__declspec(naked) MechS32 CodeBlockFixedMul30(MechS32 p_a, MechS32 p_b)
{
	__asm {
		push ebp
		mov ebp, esp
		mov eax, dword ptr [ebp+0x8]
		imul dword ptr [ebp+0xc]
		add eax, 0x20000000
		adc edx, 0x0
		shld edx, eax, 0x2
		mov eax, edx
		leave
		ret
		_emit 0x2e /* mov eax, eax with a segment prefix: MASM alignment padding */
		_emit 0x8b
		_emit 0xc0
	}
}
#endif

// CallCodeBlockRoutine for a polygon that may cross the target's edges: clips the list of
// p_count six-dword vertices against each edge of the target's rectangle it crosses (the passes
// alternate between p_vertices and g_codeBlockClipVertices), then calls the table's routine
// p_index on what is left, if it has at least three vertices.
#ifdef COMPAT_MODE
void CallCodeBlockRoutineClipped(
	RenderTarget* p_target,
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
#else
// FUNCTION: MW2 0x10032a58
__declspec(naked) void CallCodeBlockRoutineClipped(
	RenderTarget* p_target,
	MechU32* p_vertices,
	MechS32 p_count,
	MechS32 p_index,
	undefined4 p_unk0x18,
	CodeBlockTexture* p_texture,
	MechU16* p_luma,
	undefined4 p_unk0x24
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x20
		push es
		push esi
		push edi
		push ebx
		push ds
		pop es
		cld
		mov dword ptr [ebp - 0x1c], 0
		mov esi, dword ptr [ebp + 0xc]
		mov edi, dword ptr [ebp + 8]
		mov eax, dword ptr [edi + 0xc]
		mov ebx, dword ptr [edi + 4]
		mov ecx, dword ptr [edi + 0x10]
		mov edx, dword ptr [edi + 8]
		mov dword ptr [ebp - 4], eax
		mov dword ptr [ebp - 0xc], ebx
		mov dword ptr [ebp - 8], ecx
		mov dword ptr [ebp - 0x10], edx
		mov edi, dword ptr [edi]
		mov eax, dword ptr [edi + 4]
		inc eax
		mov dword ptr [g_codeBlockVars+0x8], eax
		mov eax, dword ptr [edi]
		mov dword ptr [g_codeBlockVars+0x10], eax
		mov ecx, dword ptr [ebp + 0x10]
jmp_10032a9f:
		mov edx, 0
		mov eax, dword ptr [ebp - 4]
		mov ebx, dword ptr [esi]
		sub eax, ebx
		shld edx, eax, 1
		mov eax, dword ptr [ebp - 0xc]
		sub ebx, eax
		shld edx, ebx, 1
		mov eax, dword ptr [ebp - 8]
		mov ebx, dword ptr [esi + 4]
		sub eax, ebx
		shld edx, eax, 1
		mov eax, dword ptr [ebp - 0x10]
		sub ebx, eax
		shld edx, ebx, 1
		or dword ptr [ebp - 0x1c], edx
		add esi, 0x18
		loop jmp_10032a9f
		mov ebx, dword ptr [ebp + 0xc]
		mov eax, dword ptr [ebp + 0x10]
		dec eax
		mov esi, eax
		shl eax, 4
		shl esi, 3
		add esi, eax
		add esi, ebx
		mov edi, offset g_codeBlockClipVertices
		mov dword ptr [ebp - 0x18], 0
		cmp dword ptr [ebp - 0x1c], 0
		je jmp_10033228
		mov dword ptr [ebp - 0x18], 0
		mov dword ptr [ebp - 0x20], 0
		mov eax, dword ptr [ebp + 0x14]
		and eax, 0x400
		mov dword ptr [g_unk0x100a526c], eax
		mov ecx, 0
		mov eax, dword ptr [ebp + 0x14]
		and eax, 0x600
		cmp eax, 0x600
		_emit 0x75 /* jne jmp_10032b32 */
		_emit 0x05
		mov ecx, 1
jmp_10032b32:
		mov dword ptr [g_unk0x100a5270], ecx
		mov ecx, 1
		mov eax, dword ptr [ebp + 0x14]
		and eax, 0x1c0
		cmp eax, 0x40
		_emit 0x74 /* je jmp_10032b6d */
		_emit 0x23
		cmp eax, 0xc0
		_emit 0x74 /* je jmp_10032b6d */
		_emit 0x1c
		cmp eax, 0x100
		_emit 0x74 /* je jmp_10032b6d */
		_emit 0x15
		mov eax, dword ptr [ebp + 0x14]
		and eax, 7
		cmp eax, 4
		_emit 0x74 /* je jmp_10032b6d */
		_emit 0x0a
		cmp eax, 7
		_emit 0x74 /* je jmp_10032b6d */
		_emit 0x05
		mov ecx, 0
jmp_10032b6d:
		mov dword ptr [g_unk0x100a5274], ecx
		mov eax, dword ptr [ebp - 0x1c]
		and eax, 8
		je jmp_10032d03
		mov ecx, dword ptr [ebp + 0x10]
jmp_10032b82:
		mov eax, dword ptr [esi]
		mov edx, dword ptr [ebx]
		cmp eax, dword ptr [ebp - 4]
		_emit 0x7f /* jg jmp_10032bbf */
		_emit 0x34
		push eax
		mov eax, dword ptr [esi]
		mov dword ptr [edi], eax
		mov eax, dword ptr [esi + 4]
		mov dword ptr [edi + 4], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [edi + 8], eax
		mov eax, dword ptr [esi + 0xc]
		mov dword ptr [edi + 0xc], eax
		mov eax, dword ptr [esi + 0x10]
		mov dword ptr [edi + 0x10], eax
		mov eax, dword ptr [esi + 0x14]
		mov dword ptr [edi + 0x14], eax
		add edi, 0x18
		pop eax
		inc dword ptr [ebp - 0x18]
		cmp edx, dword ptr [ebp - 4]
		_emit 0x7f /* jg jmp_10032bc8 */
		_emit 0x0e
		jmp jmp_10032cc6
jmp_10032bbf:
		cmp edx, dword ptr [ebp - 4]
		jg jmp_10032cc6
jmp_10032bc8:
		sub eax, edx
		neg edx
		add edx, dword ptr [ebp - 4]
		push ebx
		push ecx
		push eax
		sub ecx, ecx
		mov eax, edx
		and eax, eax
		_emit 0x79 /* jns jmp_10032bdd */
		_emit 0x03
		inc ecx
		neg eax
jmp_10032bdd:
		mov edx, eax
		sar edx, 2
		shl eax, 0x1e
		pop ebx
		and ebx, ebx
		_emit 0x79 /* jns jmp_10032bed */
		_emit 0x03
		dec ecx
		neg ebx
jmp_10032bed:
		div ebx
		shr ebx, 1
		adc ebx, 0
		dec ebx
		cmp ebx, edx
		adc eax, 0
		and ecx, ecx
		_emit 0x74 /* je jmp_10032c00 */
		_emit 0x02
		neg eax
jmp_10032c00:
		pop ecx
		pop ebx
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [ebp - 4]
		mov dword ptr [edi], eax
		mov eax, dword ptr [esi + 4]
		sub eax, dword ptr [ebx + 4]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 4]
		add edx, eax
		mov dword ptr [edi + 4], edx
		cmp dword ptr [g_unk0x100a526c], 0
		_emit 0x74 /* je jmp_10032c70 */
		_emit 0x3e
		mov eax, dword ptr [esi + 0xc]
		sub eax, dword ptr [ebx + 0xc]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0xc]
		add edx, eax
		mov dword ptr [edi + 0xc], edx
		mov eax, dword ptr [esi + 0x10]
		sub eax, dword ptr [ebx + 0x10]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0x10]
		add edx, eax
		mov dword ptr [edi + 0x10], edx
jmp_10032c70:
		cmp dword ptr [g_unk0x100a5270], 0
		_emit 0x74 /* je jmp_10032c98 */
		_emit 0x1f
		mov eax, dword ptr [esi + 0x14]
		sub eax, dword ptr [ebx + 0x14]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0x14]
		add edx, eax
		mov dword ptr [edi + 0x14], edx
jmp_10032c98:
		cmp dword ptr [g_unk0x100a5274], 0
		_emit 0x74 /* je jmp_10032cc0 */
		_emit 0x1f
		mov eax, dword ptr [esi + 8]
		sub eax, dword ptr [ebx + 8]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 8]
		add edx, eax
		mov dword ptr [edi + 8], edx
jmp_10032cc0:
		add edi, 0x18
		inc dword ptr [ebp - 0x18]
jmp_10032cc6:
		mov esi, ebx
		add ebx, 0x18
		dec ecx
		jne jmp_10032b82
		inc dword ptr [ebp - 0x20]
		mov eax, dword ptr [ebp - 0x18]
		mov dword ptr [ebp + 0x10], eax
		cmp eax, 0
		je jmp_10033228
		mov ebx, offset g_codeBlockClipVertices
		mov eax, dword ptr [ebp + 0x10]
		dec eax
		mov esi, eax
		shl eax, 4
		shl esi, 3
		add esi, eax
		add esi, ebx
		mov edi, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x18], 0
jmp_10032d03:
		mov eax, dword ptr [ebp - 0x1c]
		and eax, 1
		je jmp_10032ebb
		mov ecx, dword ptr [ebp + 0x10]
jmp_10032d12:
		mov eax, dword ptr [esi + 4]
		mov edx, dword ptr [ebx + 4]
		cmp eax, dword ptr [ebp - 0x10]
		_emit 0x7c /* jl jmp_10032d51 */
		_emit 0x34
		push eax
		mov eax, dword ptr [esi]
		mov dword ptr [edi], eax
		mov eax, dword ptr [esi + 4]
		mov dword ptr [edi + 4], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [edi + 8], eax
		mov eax, dword ptr [esi + 0xc]
		mov dword ptr [edi + 0xc], eax
		mov eax, dword ptr [esi + 0x10]
		mov dword ptr [edi + 0x10], eax
		mov eax, dword ptr [esi + 0x14]
		mov dword ptr [edi + 0x14], eax
		add edi, 0x18
		pop eax
		inc dword ptr [ebp - 0x18]
		cmp edx, dword ptr [ebp - 0x10]
		_emit 0x7c /* jl jmp_10032d5a */
		_emit 0x0e
		jmp jmp_10032e55
jmp_10032d51:
		cmp edx, dword ptr [ebp - 0x10]
		jl jmp_10032e55
jmp_10032d5a:
		sub eax, edx
		neg edx
		add edx, dword ptr [ebp - 0x10]
		push ebx
		push ecx
		push eax
		sub ecx, ecx
		mov eax, edx
		and eax, eax
		_emit 0x79 /* jns jmp_10032d6f */
		_emit 0x03
		inc ecx
		neg eax
jmp_10032d6f:
		mov edx, eax
		sar edx, 2
		shl eax, 0x1e
		pop ebx
		and ebx, ebx
		_emit 0x79 /* jns jmp_10032d7f */
		_emit 0x03
		dec ecx
		neg ebx
jmp_10032d7f:
		div ebx
		shr ebx, 1
		adc ebx, 0
		dec ebx
		cmp ebx, edx
		adc eax, 0
		and ecx, ecx
		_emit 0x74 /* je jmp_10032d92 */
		_emit 0x02
		neg eax
jmp_10032d92:
		pop ecx
		pop ebx
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [ebp - 0x10]
		mov dword ptr [edi + 4], eax
		mov eax, dword ptr [esi]
		sub eax, dword ptr [ebx]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx]
		add edx, eax
		mov dword ptr [edi], edx
		cmp dword ptr [g_unk0x100a526c], 0
		_emit 0x74 /* je jmp_10032dff */
		_emit 0x3e
		mov eax, dword ptr [esi + 0xc]
		sub eax, dword ptr [ebx + 0xc]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0xc]
		add edx, eax
		mov dword ptr [edi + 0xc], edx
		mov eax, dword ptr [esi + 0x10]
		sub eax, dword ptr [ebx + 0x10]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0x10]
		add edx, eax
		mov dword ptr [edi + 0x10], edx
jmp_10032dff:
		cmp dword ptr [g_unk0x100a5270], 0
		_emit 0x74 /* je jmp_10032e27 */
		_emit 0x1f
		mov eax, dword ptr [esi + 0x14]
		sub eax, dword ptr [ebx + 0x14]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0x14]
		add edx, eax
		mov dword ptr [edi + 0x14], edx
jmp_10032e27:
		cmp dword ptr [g_unk0x100a5274], 0
		_emit 0x74 /* je jmp_10032e4f */
		_emit 0x1f
		mov eax, dword ptr [esi + 8]
		sub eax, dword ptr [ebx + 8]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 8]
		add edx, eax
		mov dword ptr [edi + 8], edx
jmp_10032e4f:
		add edi, 0x18
		inc dword ptr [ebp - 0x18]
jmp_10032e55:
		mov esi, ebx
		add ebx, 0x18
		dec ecx
		jne jmp_10032d12
		mov eax, dword ptr [ebp - 0x18]
		mov dword ptr [ebp + 0x10], eax
		cmp eax, 0
		je jmp_10033228
		inc dword ptr [ebp - 0x20]
		mov eax, dword ptr [ebp - 0x20]
		and eax, 1
		_emit 0x74 /* je jmp_10032e9c */
		_emit 0x21
		mov ebx, offset g_codeBlockClipVertices
		mov eax, dword ptr [ebp + 0x10]
		dec eax
		mov esi, eax
		shl eax, 4
		shl esi, 3
		add esi, eax
		add esi, ebx
		mov edi, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x18], 0
		jmp short jmp_10032ebb
jmp_10032e9c:
		mov ebx, dword ptr [ebp + 0xc]
		mov eax, dword ptr [ebp + 0x10]
		dec eax
		mov esi, eax
		shl eax, 4
		shl esi, 3
		add esi, eax
		add esi, ebx
		mov edi, offset g_codeBlockClipVertices
		mov dword ptr [ebp - 0x18], 0
jmp_10032ebb:
		mov eax, dword ptr [ebp - 0x1c]
		and eax, 4
		je jmp_10033074
		mov ecx, dword ptr [ebp + 0x10]
jmp_10032eca:
		mov eax, dword ptr [esi]
		mov edx, dword ptr [ebx]
		cmp eax, dword ptr [ebp - 0xc]
		_emit 0x7c /* jl jmp_10032f07 */
		_emit 0x34
		push eax
		mov eax, dword ptr [esi]
		mov dword ptr [edi], eax
		mov eax, dword ptr [esi + 4]
		mov dword ptr [edi + 4], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [edi + 8], eax
		mov eax, dword ptr [esi + 0xc]
		mov dword ptr [edi + 0xc], eax
		mov eax, dword ptr [esi + 0x10]
		mov dword ptr [edi + 0x10], eax
		mov eax, dword ptr [esi + 0x14]
		mov dword ptr [edi + 0x14], eax
		add edi, 0x18
		pop eax
		inc dword ptr [ebp - 0x18]
		cmp edx, dword ptr [ebp - 0xc]
		_emit 0x7c /* jl jmp_10032f10 */
		_emit 0x0e
		jmp jmp_1003300e
jmp_10032f07:
		cmp edx, dword ptr [ebp - 0xc]
		jl jmp_1003300e
jmp_10032f10:
		sub eax, edx
		neg edx
		add edx, dword ptr [ebp - 0xc]
		push ebx
		push ecx
		push eax
		sub ecx, ecx
		mov eax, edx
		and eax, eax
		_emit 0x79 /* jns jmp_10032f25 */
		_emit 0x03
		inc ecx
		neg eax
jmp_10032f25:
		mov edx, eax
		sar edx, 2
		shl eax, 0x1e
		pop ebx
		and ebx, ebx
		_emit 0x79 /* jns jmp_10032f35 */
		_emit 0x03
		dec ecx
		neg ebx
jmp_10032f35:
		div ebx
		shr ebx, 1
		adc ebx, 0
		dec ebx
		cmp ebx, edx
		adc eax, 0
		and ecx, ecx
		_emit 0x74 /* je jmp_10032f48 */
		_emit 0x02
		neg eax
jmp_10032f48:
		pop ecx
		pop ebx
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [ebp - 0xc]
		mov dword ptr [edi], eax
		mov eax, dword ptr [esi + 4]
		sub eax, dword ptr [ebx + 4]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 4]
		add edx, eax
		mov dword ptr [edi + 4], edx
		cmp dword ptr [g_unk0x100a526c], 0
		_emit 0x74 /* je jmp_10032fb8 */
		_emit 0x3e
		mov eax, dword ptr [esi + 0xc]
		sub eax, dword ptr [ebx + 0xc]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0xc]
		add edx, eax
		mov dword ptr [edi + 0xc], edx
		mov eax, dword ptr [esi + 0x10]
		sub eax, dword ptr [ebx + 0x10]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0x10]
		add edx, eax
		mov dword ptr [edi + 0x10], edx
jmp_10032fb8:
		cmp dword ptr [g_unk0x100a5270], 0
		_emit 0x74 /* je jmp_10032fe0 */
		_emit 0x1f
		mov eax, dword ptr [esi + 0x14]
		sub eax, dword ptr [ebx + 0x14]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0x14]
		add edx, eax
		mov dword ptr [edi + 0x14], edx
jmp_10032fe0:
		cmp dword ptr [g_unk0x100a5274], 0
		_emit 0x74 /* je jmp_10033008 */
		_emit 0x1f
		mov eax, dword ptr [esi + 8]
		sub eax, dword ptr [ebx + 8]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 8]
		add edx, eax
		mov dword ptr [edi + 8], edx
jmp_10033008:
		add edi, 0x18
		inc dword ptr [ebp - 0x18]
jmp_1003300e:
		mov esi, ebx
		add ebx, 0x18
		dec ecx
		jne jmp_10032eca
		mov eax, dword ptr [ebp - 0x18]
		mov dword ptr [ebp + 0x10], eax
		cmp eax, 0
		je jmp_10033228
		inc dword ptr [ebp - 0x20]
		mov eax, dword ptr [ebp - 0x20]
		and eax, 1
		_emit 0x74 /* je jmp_10033055 */
		_emit 0x21
		mov ebx, offset g_codeBlockClipVertices
		mov eax, dword ptr [ebp + 0x10]
		dec eax
		mov esi, eax
		shl eax, 4
		shl esi, 3
		add esi, eax
		add esi, ebx
		mov edi, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x18], 0
		jmp short jmp_10033074
jmp_10033055:
		mov ebx, dword ptr [ebp + 0xc]
		mov eax, dword ptr [ebp + 0x10]
		dec eax
		mov esi, eax
		shl eax, 4
		shl esi, 3
		add esi, eax
		add esi, ebx
		mov edi, offset g_codeBlockClipVertices
		mov dword ptr [ebp - 0x18], 0
jmp_10033074:
		mov eax, dword ptr [ebp - 0x1c]
		and eax, 2
		je jmp_10033228
		mov ecx, dword ptr [ebp + 0x10]
jmp_10033083:
		mov eax, dword ptr [esi + 4]
		mov edx, dword ptr [ebx + 4]
		cmp eax, dword ptr [ebp - 8]
		_emit 0x7f /* jg jmp_100330c2 */
		_emit 0x34
		push eax
		mov eax, dword ptr [esi]
		mov dword ptr [edi], eax
		mov eax, dword ptr [esi + 4]
		mov dword ptr [edi + 4], eax
		mov eax, dword ptr [esi + 8]
		mov dword ptr [edi + 8], eax
		mov eax, dword ptr [esi + 0xc]
		mov dword ptr [edi + 0xc], eax
		mov eax, dword ptr [esi + 0x10]
		mov dword ptr [edi + 0x10], eax
		mov eax, dword ptr [esi + 0x14]
		mov dword ptr [edi + 0x14], eax
		add edi, 0x18
		pop eax
		inc dword ptr [ebp - 0x18]
		cmp edx, dword ptr [ebp - 8]
		_emit 0x7f /* jg jmp_100330cb */
		_emit 0x0e
		jmp jmp_100331c6
jmp_100330c2:
		cmp edx, dword ptr [ebp - 8]
		jg jmp_100331c6
jmp_100330cb:
		sub eax, edx
		neg edx
		add edx, dword ptr [ebp - 8]
		push ebx
		push ecx
		push eax
		sub ecx, ecx
		mov eax, edx
		and eax, eax
		_emit 0x79 /* jns jmp_100330e0 */
		_emit 0x03
		inc ecx
		neg eax
jmp_100330e0:
		mov edx, eax
		sar edx, 2
		shl eax, 0x1e
		pop ebx
		and ebx, ebx
		_emit 0x79 /* jns jmp_100330f0 */
		_emit 0x03
		dec ecx
		neg ebx
jmp_100330f0:
		div ebx
		shr ebx, 1
		adc ebx, 0
		dec ebx
		cmp ebx, edx
		adc eax, 0
		and ecx, ecx
		_emit 0x74 /* je jmp_10033103 */
		_emit 0x02
		neg eax
jmp_10033103:
		pop ecx
		pop ebx
		mov dword ptr [ebp - 0x14], eax
		mov eax, dword ptr [ebp - 8]
		mov dword ptr [edi + 4], eax
		mov eax, dword ptr [esi]
		sub eax, dword ptr [ebx]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx]
		add edx, eax
		mov dword ptr [edi], edx
		cmp dword ptr [g_unk0x100a526c], 0
		_emit 0x74 /* je jmp_10033170 */
		_emit 0x3e
		mov eax, dword ptr [esi + 0xc]
		sub eax, dword ptr [ebx + 0xc]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0xc]
		add edx, eax
		mov dword ptr [edi + 0xc], edx
		mov eax, dword ptr [esi + 0x10]
		sub eax, dword ptr [ebx + 0x10]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0x10]
		add edx, eax
		mov dword ptr [edi + 0x10], edx
jmp_10033170:
		cmp dword ptr [g_unk0x100a5270], 0
		_emit 0x74 /* je jmp_10033198 */
		_emit 0x1f
		mov eax, dword ptr [esi + 0x14]
		sub eax, dword ptr [ebx + 0x14]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 0x14]
		add edx, eax
		mov dword ptr [edi + 0x14], edx
jmp_10033198:
		cmp dword ptr [g_unk0x100a5274], 0
		_emit 0x74 /* je jmp_100331c0 */
		_emit 0x1f
		mov eax, dword ptr [esi + 8]
		sub eax, dword ptr [ebx + 8]
		imul dword ptr [ebp - 0x14]
		add eax, 0x20000000
		adc edx, 0
		shld edx, eax, 2
		mov eax, edx
		mov edx, dword ptr [ebx + 8]
		add edx, eax
		mov dword ptr [edi + 8], edx
jmp_100331c0:
		add edi, 0x18
		inc dword ptr [ebp - 0x18]
jmp_100331c6:
		mov esi, ebx
		add ebx, 0x18
		dec ecx
		jne jmp_10033083
		mov eax, dword ptr [ebp - 0x18]
		mov dword ptr [ebp + 0x10], eax
		cmp eax, 0
		_emit 0x74 /* je jmp_10033228 */
		_emit 0x4b
		inc dword ptr [ebp - 0x20]
		mov eax, dword ptr [ebp - 0x20]
		and eax, 1
		_emit 0x74 /* je jmp_10033209 */
		_emit 0x21
		mov ebx, offset g_codeBlockClipVertices
		mov eax, dword ptr [ebp + 0x10]
		dec eax
		mov esi, eax
		shl eax, 4
		shl esi, 3
		add esi, eax
		add esi, ebx
		mov edi, dword ptr [ebp + 0xc]
		mov dword ptr [ebp - 0x18], 0
		jmp short jmp_10033228
jmp_10033209:
		mov ebx, dword ptr [ebp + 0xc]
		mov eax, dword ptr [ebp + 0x10]
		dec eax
		mov esi, eax
		shl eax, 4
		shl esi, 3
		add esi, eax
		add esi, ebx
		mov edi, offset g_codeBlockClipVertices
		mov dword ptr [ebp - 0x18], 0
jmp_10033228:
		mov eax, dword ptr [ebp + 0x10]
		cmp eax, 3
		_emit 0x7c /* jl jmp_10033277 */
		_emit 0x47
		shl eax, 3
		mov edx, eax
		shl eax, 1
		add eax, edx
		add eax, ebx
		mov dword ptr [g_codeBlockVars+0x14], ebx
		mov dword ptr [g_codeBlockVars+0x18], eax
		mov eax, dword ptr [ebp + 0x1c]
		mov dword ptr [g_codeBlockVars+0x100], eax
		mov eax, dword ptr [ebp + 0x20]
		mov dword ptr [g_codeBlockVars+0x104], eax
		mov eax, dword ptr [ebp + 0x24]
		mov dword ptr [g_codeBlockVars+0x108], eax
		mov eax, dword ptr [ebp + 0x18]
		mov dword ptr [g_codeBlockVars+0x98], eax
		mov ebx, dword ptr [ebp + 0x14]
		mov eax, dword ptr [g_codeBlockRoutines + ebx*4]
		cmp eax, 0
		_emit 0x74 /* je jmp_10033277 */
		_emit 0x02
		call eax
jmp_10033277:
		pop ebx
		pop edi
		pop esi
		pop es
		leave
		ret
	}
}
#endif
