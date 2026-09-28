/* Hand-written assembly (originally a MASM object): every routine in this file is a whole
   assembly routine, transcribed as a __declspec(naked) function whose body is one __asm
   block (modern compilers, COMPAT_MODE, get STUB() bodies instead: x86_64 has no naked
   functions). The object starts at 0x10032250, flush against the compiled C code that ends at
   0x1003224f; nothing from 0x10032250 to 0x10037bd2 has a compiler frame. The evidence:

   - The `add esp, -N` locals allocation is MASM PROC style. The VC++ 4.1 front ends emit
	 `sub esp, N` for compiler frames (see DebugPrint 0x10015ce8, ShellMain 0x1000f35e);
	 outside the CRT, the original uses `add esp, -N` only in this region and at 0x100287e0.
   - Every routine saves and reloads es (`push es; cld; push ds; pop es`), which the
	 compiler never does for flat-model code, and every exit path carries its own full
	 epilogue instead of a jump to a shared one.
   - The original uses short (rel8) jumps wherever they reach and near (rel32) jumps only
	 where they don't. Compiled /Od code and the VC++ inline assembler both emit rel32 for
	 every jump (see below), so the jump sizing came from a real assembler.
   - Its data is packed without alignment: the "MCGA.DLL" buffer at 0x10068800 is followed
	 by variables from 0x1006880d onward.

   This is a C translation unit on purpose: the C++ front end appends its void-return
   `jmp` even after a naked function's __asm block (the first build showed one after every
   void routine here), while the C front end appends nothing.

   The inline assembler emits only near (rel32) jumps to __asm labels — never the 2-byte
   short form, not even for backward jumps (every label jump below grew 3-4 bytes in the
   first build). So each original short jump is reproduced byte-exactly with _emit (opcode
   + displacement from the original listing); only FUN_10034f18's three original far
   (rel32) jumps (0x10034fe9, 0x10035120, 0x1003512c) stay plain label jumps, which the
   inline assembler encodes identically. `loop` and `jecxz` have no rel32 form, so their
   label jumps stay plain.

   The inline assembler also encodes `xchg r, r` and `test r8, r8` with the two registers
   the other way round in the ModRM byte (87 da for `xchg ebx, edx`, where the original has
   87 d3), so those few instructions are _emit bytes too.

   Data the original keeps in .text is split out into globals, which the routines reference by
   name: the fixed-point cosine table at 0x10035af0 (FUN_10036904) and the IFF chunk tags that
   follow FUN_10036c9e (FUN_10036def, FUN_10036fb6, FUN_10036fe7), and the LFSR tap table at
   0x1003775b, between FUN_100376f9 and FUN_100377d7 (FUN_100377d7). */
#include "blit.h"

#include "compat.h"
#include "decomp.h"
#include "palettecolor.h"
#include "pixelview.h"
#include "types.h"

#pragma warning(disable : 4102) /* the labels mark the targets of the _emit short jumps */
#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Routines that earlier routines call.
MechS32 FUN_100333f8(
	PixelView* p_view,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10
);
MechS32 FUN_10033980(
	PixelView* p_view,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10
);
void FUN_10034aaf(MechS32 p_count, MechU8 p_transparent, MechS32 p_left);
void FUN_10034c38(MechS32 p_op, MechS32 p_back, MechS32 p_left);
void FUN_100369e2(
	MechS32* p_point,
	MechS32* p_result,
	MechS32* p_origin,
	MechS32 p_angle,
	MechS32 p_scaleX,
	MechS32 p_scaleY
);
MechU32 FUN_10037549(void* p_data, MechS32 p_index);
MechU32 FUN_1003757d(void* p_data, MechS32 p_index);

// 13 dwords that FUN_10032279 copies in from its argument.
// GLOBAL: MW2SHELL 0x100687cc
undefined4 g_unk0x100687cc[0xd] = {0};

// FUN_10032250 copies a name into it and returns it. The next variable the assembly
// references starts at 0x1006880d, so the buffer is 0xd bytes.
// GLOBAL: MW2SHELL 0x10068800
MechChar g_unk0x10068800[0xd] = "MCGA.DLL";

// The run-length encoder's state (FUN_1003479a, FUN_10034aaf, FUN_10034c38): the output
// buffer (NULL only measures), the pending skip, the row and run pointers, the output
// cursor and the start of the current run.
// GLOBAL: MW2SHELL 0x1006880d
undefined4 g_unk0x1006880d = 0;

// GLOBAL: MW2SHELL 0x10068811
undefined4 g_unk0x10068811 = 0;

// GLOBAL: MW2SHELL 0x10068815
undefined4 g_unk0x10068815 = 0;

// GLOBAL: MW2SHELL 0x10068819
undefined4 g_unk0x10068819 = 0;

// GLOBAL: MW2SHELL 0x1006881d
undefined4 g_unk0x1006881d = 0;

// GLOBAL: MW2SHELL 0x10068821
undefined4 g_unk0x10068821 = 0;

// The bounding box of the opaque pixels that FUN_1003479a finds: left, top, right, bottom.
// GLOBAL: MW2SHELL 0x10068835
undefined4 g_unk0x10068835 = 0;

// GLOBAL: MW2SHELL 0x10068839
undefined4 g_unk0x10068839 = 0;

// GLOBAL: MW2SHELL 0x1006883d
undefined4 g_unk0x1006883d = 0;

// GLOBAL: MW2SHELL 0x10068841
undefined4 g_unk0x10068841 = 0;

// A dword table that FUN_100377d7 fills and reads. The next variable starts at 0x10068d45.
// GLOBAL: MW2SHELL 0x10068845
undefined4 g_unk0x10068845[0x140] = {0};

// Scanline buffer: FUN_10037014 decodes one RLE scanline (up to the ushort width at data
// header +0x42) into it, and FUN_100371d5 collects one GIF row, before blitting it through
// FUN_10036c9e. FUN_10037a4e also keeps its working palette here. The next variable starts at
// 0x10069045.
// GLOBAL: MW2SHELL 0x10068d45
undefined g_unk0x10068d45[0x300] = {0};

// The colors FUN_10037a4e found in the view.
// GLOBAL: MW2SHELL 0x10069045
undefined g_unk0x10069045[0x100] = {0};

// FUN_10037a4e's per-component distances to the target palette.
// GLOBAL: MW2SHELL 0x10069145
undefined g_unk0x10069145[0x300] = {0};

// A second dword table of FUN_100377d7. The next variable starts at 0x10069a45.
// GLOBAL: MW2SHELL 0x10069445
undefined4 g_unk0x10069445[0x180] = {0};

// Flags per color index (FUN_10037a4e, FUN_10037bd2), then FUN_10037a4e's per-component
// directions.
// GLOBAL: MW2SHELL 0x10069a45
undefined g_unk0x10069a45[0x300] = {0};

// FUN_10037a4e's per-component error accumulators.
// GLOBAL: MW2SHELL 0x10069d45
undefined g_unk0x10069d45[0x300] = {0};

// Masks of the low n bits, indexed by the code width.
// GLOBAL: MW2SHELL 0x1006a045
MechU8 g_unk0x1006a045[9] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};

// GIF interlacing: the row step of each pass...
// GLOBAL: MW2SHELL 0x1006a04e
MechU8 g_unk0x1006a04e[5] = {8, 8, 4, 2, 0};

// ...and the first row of the next pass.
// GLOBAL: MW2SHELL 0x1006a053
MechU8 g_unk0x1006a053[5] = {0, 4, 2, 1, 0};

// The view FUN_10037252 decodes into.
// GLOBAL: MW2SHELL 0x1006a058
PixelView* g_unk0x1006a058 = NULL;

// The color remap table that FUN_100334fb loads and FUN_10033980 and FUN_10034a1d apply.
// GLOBAL: MW2SHELL 0x1006a05c
MechU8 g_unk0x1006a05c[0x100] = {0};

// FUN_10033a76's four transformed corners, five dwords each.
// GLOBAL: MW2SHELL 0x1006a15c
undefined4 g_unk0x1006a15c[0x14] = {0};

// FUN_10033a76's per-corner steps, indexed by corner.
// GLOBAL: MW2SHELL 0x1006a1ac
undefined4 g_unk0x1006a1ac[4] = {0};

// Calls the function pointer at the start of its argument, copies the string it returns
// into g_unk0x10068800 and returns that buffer.
#ifdef COMPAT_MODE
MechChar* FUN_10032250(undefined4* p_unk0x00)
{
	STUB(0x10032250);
	return NULL;
}
#else
// FUNCTION: MW2SHELL 0x10032250
__declspec(naked) MechChar* FUN_10032250(undefined4* p_unk0x00)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		call dword ptr [esi]
		mov edi, offset g_unk0x10068800
jmp_10032264:
		mov bl, byte ptr [eax]
		mov byte ptr [edi], bl
		inc edi
		inc eax
		or bl, bl
		_emit 0x75 /* jnz jmp_10032264 */
		_emit 0xf6
		mov eax, offset g_unk0x10068800
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_10032279(undefined4* p_unk0x00)
{
	STUB(0x10032279);
}
#else
// FUNCTION: MW2SHELL 0x10032279
__declspec(naked) void FUN_10032279(undefined4* p_unk0x00)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov edi, offset g_unk0x100687cc
		mov ecx, 0xd
		rep movsd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Writes one pixel at (p_x, p_y), relative to the view, and returns the pixel it replaced.
// Returns -1 for an empty buffer, -2 for an empty view and -3 when the point is clipped.
#ifdef COMPAT_MODE
MechS32 FUN_10032298(PixelView* p_view, MechS32 p_x, MechS32 p_y, MechS32 p_color)
{
	STUB(0x10032298);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10032298
__declspec(naked) MechS32 FUN_10032298(PixelView* p_view, MechS32 p_x, MechS32 p_y, MechS32 p_color)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x20
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x18], eax
		_emit 0x7e /* jle jmp_10032317 */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10032317 */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x1c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_100322cb */
		_emit 0x05
		mov eax, 0x0
jmp_100322cb:
		mov dword ptr [ebp-0x4], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x20], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_100322de */
		_emit 0x05
		mov eax, 0x0
jmp_100322de:
		mov dword ptr [ebp-0x8], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x18]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100322ee */
		_emit 0x02
		mov eax, edx
jmp_100322ee:
		mov dword ptr [ebp-0xc], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100322fd */
		_emit 0x02
		mov eax, edx
jmp_100322fd:
		mov dword ptr [ebp-0x10], eax
		mov eax, dword ptr [ebp-0xc]
		cmp eax, dword ptr [ebp-0x4]
		_emit 0x7c /* jl jmp_10032322 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x10]
		cmp eax, dword ptr [ebp-0x8]
		_emit 0x7c /* jl jmp_10032322 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x14], eax
		_emit 0xeb /* jmp jmp_1003232d */
		_emit 0x16
jmp_10032317:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10032322:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1003232d:
		mov ecx, dword ptr [ebp+0xc]
		mov ebx, dword ptr [ebp+0x10]
		add ecx, dword ptr [ebp-0x1c]
		add ebx, dword ptr [ebp-0x20]
		cmp ecx, dword ptr [ebp-0x4]
		_emit 0x7c /* jl jmp_10032368 */
		_emit 0x2a
		cmp ecx, dword ptr [ebp-0xc]
		_emit 0x7f /* jg jmp_10032368 */
		_emit 0x25
		cmp ebx, dword ptr [ebp-0x8]
		_emit 0x7c /* jl jmp_10032368 */
		_emit 0x20
		cmp ebx, dword ptr [ebp-0x10]
		_emit 0x7f /* jg jmp_10032368 */
		_emit 0x1b
		mov eax, ebx
		imul dword ptr [ebp-0x18]
		add eax, dword ptr [ebp-0x14]
		add eax, ecx
		mov ebx, eax
		xor eax, eax
		mov al, byte ptr [ebx]
		mov dl, byte ptr [ebp+0x14]
		mov byte ptr [ebx], dl
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10032368:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Reads the pixel at (p_x, p_y), relative to the view. Returns -1 for an empty buffer, -2 for
// an empty view and -3 when the point is clipped.
#ifdef COMPAT_MODE
MechS32 FUN_10032373(PixelView* p_view, MechS32 p_x, MechS32 p_y)
{
	STUB(0x10032373);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10032373
__declspec(naked) MechS32 FUN_10032373(PixelView* p_view, MechS32 p_x, MechS32 p_y)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x20
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x18], eax
		_emit 0x7e /* jle jmp_100323f2 */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_100323f2 */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x1c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_100323a6 */
		_emit 0x05
		mov eax, 0x0
jmp_100323a6:
		mov dword ptr [ebp-0x4], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x20], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_100323b9 */
		_emit 0x05
		mov eax, 0x0
jmp_100323b9:
		mov dword ptr [ebp-0x8], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x18]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100323c9 */
		_emit 0x02
		mov eax, edx
jmp_100323c9:
		mov dword ptr [ebp-0xc], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100323d8 */
		_emit 0x02
		mov eax, edx
jmp_100323d8:
		mov dword ptr [ebp-0x10], eax
		mov eax, dword ptr [ebp-0xc]
		cmp eax, dword ptr [ebp-0x4]
		_emit 0x7c /* jl jmp_100323fd */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x10]
		cmp eax, dword ptr [ebp-0x8]
		_emit 0x7c /* jl jmp_100323fd */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x14], eax
		_emit 0xeb /* jmp jmp_10032408 */
		_emit 0x16
jmp_100323f2:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100323fd:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10032408:
		mov ecx, dword ptr [ebp+0xc]
		mov ebx, dword ptr [ebp+0x10]
		add ecx, dword ptr [ebp-0x1c]
		add ebx, dword ptr [ebp-0x20]
		cmp ecx, dword ptr [ebp-0x4]
		_emit 0x7c /* jl jmp_1003243e */
		_emit 0x25
		cmp ecx, dword ptr [ebp-0xc]
		_emit 0x7f /* jg jmp_1003243e */
		_emit 0x20
		cmp ebx, dword ptr [ebp-0x8]
		_emit 0x7c /* jl jmp_1003243e */
		_emit 0x1b
		cmp ebx, dword ptr [ebp-0x10]
		_emit 0x7f /* jg jmp_1003243e */
		_emit 0x16
		mov eax, ebx
		imul dword ptr [ebp-0x18]
		add eax, dword ptr [ebp-0x14]
		add eax, ecx
		mov ebx, eax
		xor eax, eax
		mov al, byte ptr [ebx]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1003243e:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Fills or processes the clipped rectangle of the view. p_unk0x14 selects the mode; in some
// modes p_color is a callback the routine calls. Returns a negative code when the view is empty
// or the rectangle is fully clipped.
#ifdef COMPAT_MODE
MechS32 FUN_10032449(
	PixelView* p_view,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_right,
	MechS32 p_bottom,
	MechS32 p_unk0x14,
	MechS32 p_color
)
{
	STUB(0x10032449);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10032449
__declspec(naked) MechS32 FUN_10032449(
	PixelView* p_view,
	MechS32 p_left,
	MechS32 p_top,
	MechS32 p_right,
	MechS32 p_bottom,
	MechS32 p_unk0x14,
	MechS32 p_color
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x58
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x50], eax
		_emit 0x7e /* jle jmp_100324c8 */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_100324c8 */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x54], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_1003247c */
		_emit 0x05
		mov eax, 0x0
	jmp_1003247c:
		mov dword ptr [ebp-0x3c], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x58], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_1003248f */
		_emit 0x05
		mov eax, 0x0
	jmp_1003248f:
		mov dword ptr [ebp-0x40], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x50]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_1003249f */
		_emit 0x02
		mov eax, edx
	jmp_1003249f:
		mov dword ptr [ebp-0x44], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100324ae */
		_emit 0x02
		mov eax, edx
	jmp_100324ae:
		mov dword ptr [ebp-0x48], eax
		mov eax, dword ptr [ebp-0x44]
		cmp eax, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_100324d3 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x48]
		cmp eax, dword ptr [ebp-0x40]
		_emit 0x7c /* jl jmp_100324d3 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x4c], eax
		_emit 0xeb /* jmp jmp_100324de */
		_emit 0x16
	jmp_100324c8:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_100324d3:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_100324de:
		mov eax, dword ptr [ebp-0x54]
		add dword ptr [ebp+0xc], eax
		add dword ptr [ebp+0x14], eax
		mov eax, dword ptr [ebp-0x58]
		add dword ptr [ebp+0x10], eax
		add dword ptr [ebp+0x18], eax
		mov eax, dword ptr [ebp+0x14]
		sub eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x4], eax
		cdq
		mov dword ptr [ebp-0xc], edx
		xor eax, edx
		sub eax, edx
		mov dword ptr [ebp-0x8], eax
		mov eax, dword ptr [ebp+0x18]
		sub eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x10], eax
		cdq
		mov dword ptr [ebp-0x18], edx
		xor eax, edx
		sub eax, edx
		mov dword ptr [ebp-0x14], eax
		mov eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x24], eax
		mov eax, dword ptr [ebp+0x14]
		mov dword ptr [ebp-0x2c], eax
		mov eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x28], eax
		mov eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x30], eax
		cmp dword ptr [ebp-0x4], 0x0
		jz jmp_10032c5f
		cmp dword ptr [ebp-0x10], 0x0
		jz jmp_10032cea
		mov eax, dword ptr [ebp-0xc]
		xor eax, dword ptr [ebp-0x18]
		mov dword ptr [ebp-0x1c], eax
		mov edx, dword ptr [ebp-0x8]
		mov ebx, dword ptr [ebp-0x14]
		mov eax, 0xffffffff
		cmp edx, ebx
		_emit 0x74 /* jz jmp_10032564 */
		_emit 0x08
		_emit 0x7c /* jl jmp_10032560 */
		_emit 0x02
		_emit 0x87 /* xchg ebx, edx: the inline assembler encodes the operands the other way */
		_emit 0xd3
	jmp_10032560:
		xor eax, eax
		div ebx
	jmp_10032564:
		mov dword ptr [ebp-0x20], eax
		mov dword ptr [ebp-0x34], 0x0
	jmp_1003256e:
		xor edx, edx
		mov eax, dword ptr [ebp-0x24]
		sub eax, dword ptr [ebp-0x3c]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x44]
		sub eax, dword ptr [ebp-0x24]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [ebp-0x40]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x48]
		sub eax, dword ptr [ebp-0x28]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x2c]
		sub eax, dword ptr [ebp-0x3c]
		shl eax, 0x1
		adc dh, dh
		mov eax, dword ptr [ebp-0x44]
		sub eax, dword ptr [ebp-0x2c]
		shl eax, 0x1
		adc dh, dh
		mov eax, dword ptr [ebp-0x30]
		sub eax, dword ptr [ebp-0x40]
		shl eax, 0x1
		adc dh, dh
		mov eax, dword ptr [ebp-0x48]
		sub eax, dword ptr [ebp-0x30]
		shl eax, 0x1
		adc dh, dh
		or dword ptr [ebp-0x34], edx
		or edx, edx
		jz jmp_1003292d
		_emit 0x84 /* test dh, dl: the inline assembler encodes the operands the other way */
		_emit 0xd6
		jnz jmp_10032e40
		mov ebx, dword ptr [ebp-0x8]
		cmp ebx, dword ptr [ebp-0x14]
		_emit 0x7c /* jl jmp_10032628 */
		_emit 0x4d
		test dl, 0x8
		jnz jmp_10032671
		test dl, 0x4
		jnz jmp_100326c5
		test dl, 0x2
		jnz jmp_10032749
		test dl, 0x1
		jnz jmp_100327a1
		test dh, 0x8
		jnz jmp_100327d1
		test dh, 0x4
		jnz jmp_1003282c
		test dh, 0x2
		jnz jmp_100328ab
		test dh, 0x1
		jnz jmp_10032902
		jmp jmp_1003256e
	jmp_10032628:
		test dl, 0x8
		_emit 0x75 /* jnz jmp_10032699 */
		_emit 0x6c
		test dl, 0x4
		jnz jmp_100326f1
		test dl, 0x2
		jnz jmp_10032721
		test dl, 0x1
		jnz jmp_10032775
		test dh, 0x8
		jnz jmp_100327fd
		test dh, 0x4
		jnz jmp_10032854
		test dh, 0x2
		jnz jmp_1003287f
		test dh, 0x1
		jnz jmp_100328da
		jmp jmp_1003256e
	jmp_10032671:
		mov eax, dword ptr [ebp-0x3c]
		mov dword ptr [ebp-0x24], eax
		sub eax, dword ptr [ebp+0xc]
		mul dword ptr [ebp-0x20]
		add eax, 0x80000000
		adc edx, 0x0
		mov eax, edx
		mov edx, dword ptr [ebp-0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x28], eax
		jmp jmp_1003256e
	jmp_10032699:
		mov eax, dword ptr [ebp-0x3c]
		mov dword ptr [ebp-0x24], eax
		sub eax, dword ptr [ebp+0xc]
		mov edx, eax
		dec edx
		mov eax, 0x80000000
		div dword ptr [ebp-0x20]
		cmp edx, 0x1
		sbb eax, -0x1
		mov edx, dword ptr [ebp-0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x28], eax
		jmp jmp_1003256e
	jmp_100326c5:
		mov eax, dword ptr [ebp-0x44]
		mov dword ptr [ebp-0x24], eax
		sub eax, dword ptr [ebp+0xc]
		neg eax
		mul dword ptr [ebp-0x20]
		add eax, 0x80000000
		adc edx, 0x0
		mov eax, edx
		mov edx, dword ptr [ebp-0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x28], eax
		jmp jmp_1003256e
	jmp_100326f1:
		mov eax, dword ptr [ebp-0x44]
		mov dword ptr [ebp-0x24], eax
		sub eax, dword ptr [ebp+0xc]
		neg eax
		mov edx, eax
		dec edx
		mov eax, 0x80000000
		div dword ptr [ebp-0x20]
		cmp edx, 0x1
		sbb eax, -0x1
		mov edx, dword ptr [ebp-0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x28], eax
		jmp jmp_1003256e
	jmp_10032721:
		mov eax, dword ptr [ebp-0x40]
		mov dword ptr [ebp-0x28], eax
		sub eax, dword ptr [ebp+0x10]
		mul dword ptr [ebp-0x20]
		add eax, 0x80000000
		adc edx, 0x0
		mov eax, edx
		mov edx, dword ptr [ebp-0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x24], eax
		jmp jmp_1003256e
	jmp_10032749:
		mov eax, dword ptr [ebp-0x40]
		mov dword ptr [ebp-0x28], eax
		sub eax, dword ptr [ebp+0x10]
		mov edx, eax
		dec edx
		mov eax, 0x80000000
		div dword ptr [ebp-0x20]
		cmp edx, 0x1
		sbb eax, -0x1
		mov edx, dword ptr [ebp-0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x24], eax
		jmp jmp_1003256e
	jmp_10032775:
		mov eax, dword ptr [ebp-0x48]
		mov dword ptr [ebp-0x28], eax
		sub eax, dword ptr [ebp+0x10]
		neg eax
		mul dword ptr [ebp-0x20]
		add eax, 0x80000000
		adc edx, 0x0
		mov eax, edx
		mov edx, dword ptr [ebp-0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x24], eax
		jmp jmp_1003256e
	jmp_100327a1:
		mov eax, dword ptr [ebp-0x48]
		mov dword ptr [ebp-0x28], eax
		sub eax, dword ptr [ebp+0x10]
		neg eax
		mov edx, eax
		dec edx
		mov eax, 0x80000000
		div dword ptr [ebp-0x20]
		cmp edx, 0x1
		sbb eax, -0x1
		mov edx, dword ptr [ebp-0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x24], eax
		jmp jmp_1003256e
	jmp_100327d1:
		mov eax, dword ptr [ebp-0x3c]
		mov dword ptr [ebp-0x2c], eax
		sub eax, dword ptr [ebp+0xc]
		neg eax
		mul dword ptr [ebp-0x20]
		add eax, 0x80000000
		adc edx, 0x0
		mov eax, edx
		mov edx, dword ptr [ebp-0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x30], eax
		jmp jmp_1003256e
	jmp_100327fd:
		mov eax, dword ptr [ebp-0x3c]
		mov dword ptr [ebp-0x2c], eax
		sub eax, dword ptr [ebp+0xc]
		neg eax
		mov edx, eax
		mov eax, 0x80000000
		div dword ptr [ebp-0x20]
		cmp edx, 0x1
		sbb eax, 0x0
		mov edx, dword ptr [ebp-0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x30], eax
		jmp jmp_1003256e
	jmp_1003282c:
		mov eax, dword ptr [ebp-0x44]
		mov dword ptr [ebp-0x2c], eax
		sub eax, dword ptr [ebp+0xc]
		mul dword ptr [ebp-0x20]
		add eax, 0x80000000
		adc edx, 0x0
		mov eax, edx
		mov edx, dword ptr [ebp-0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x30], eax
		jmp jmp_1003256e
	jmp_10032854:
		mov eax, dword ptr [ebp-0x44]
		mov dword ptr [ebp-0x2c], eax
		sub eax, dword ptr [ebp+0xc]
		mov edx, eax
		mov eax, 0x80000000
		div dword ptr [ebp-0x20]
		cmp edx, 0x1
		sbb eax, 0x0
		mov edx, dword ptr [ebp-0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x30], eax
		jmp jmp_1003256e
	jmp_1003287f:
		mov eax, dword ptr [ebp-0x40]
		mov dword ptr [ebp-0x30], eax
		sub eax, dword ptr [ebp+0x10]
		neg eax
		mul dword ptr [ebp-0x20]
		add eax, 0x80000000
		adc edx, 0x0
		mov eax, edx
		mov edx, dword ptr [ebp-0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x2c], eax
		jmp jmp_1003256e
	jmp_100328ab:
		mov eax, dword ptr [ebp-0x40]
		mov dword ptr [ebp-0x30], eax
		sub eax, dword ptr [ebp+0x10]
		neg eax
		mov edx, eax
		mov eax, 0x80000000
		div dword ptr [ebp-0x20]
		cmp edx, 0x1
		sbb eax, 0x0
		mov edx, dword ptr [ebp-0x1c]
		not edx
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x2c], eax
		jmp jmp_1003256e
	jmp_100328da:
		mov eax, dword ptr [ebp-0x48]
		mov dword ptr [ebp-0x30], eax
		sub eax, dword ptr [ebp+0x10]
		mul dword ptr [ebp-0x20]
		add eax, 0x80000000
		adc edx, 0x0
		mov eax, edx
		mov edx, dword ptr [ebp-0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x2c], eax
		jmp jmp_1003256e
	jmp_10032902:
		mov eax, dword ptr [ebp-0x48]
		mov dword ptr [ebp-0x30], eax
		sub eax, dword ptr [ebp+0x10]
		mov edx, eax
		mov eax, 0x80000000
		div dword ptr [ebp-0x20]
		cmp edx, 0x1
		sbb eax, 0x0
		mov edx, dword ptr [ebp-0x1c]
		xor eax, edx
		sub eax, edx
		add eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x2c], eax
		jmp jmp_1003256e
	jmp_1003292d:
		mov eax, dword ptr [ebp-0x28]
		imul dword ptr [ebp-0x50]
		add eax, dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x24]
		mov edi, eax
		mov esi, dword ptr [ebp-0x50]
		xor esi, dword ptr [ebp-0x18]
		sub esi, dword ptr [ebp-0x18]
		mov ebx, dword ptr [ebp-0x20]
		mov eax, dword ptr [ebp-0x8]
		cmp eax, dword ptr [ebp-0x14]
		jz jmp_10032d70
		jg jmp_10032ac1
		mov eax, dword ptr [ebp-0x30]
		sub eax, dword ptr [ebp-0x28]
		cdq
		xor eax, edx
		sub eax, edx
		inc eax
		mov ecx, eax
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [ebp+0x10]
		cdq
		xor eax, edx
		sub eax, edx
		mul ebx
		add eax, 0x80000000
		mov edx, eax
		cmp dword ptr [ebp-0xc], -0x1
		jz jmp_10032a22
		cmp dword ptr [ebp+0x1c], 0x1
		_emit 0x74 /* jz jmp_100329b9 */
		_emit 0x2e
		_emit 0x7f /* jg jmp_100329f7 */
		_emit 0x6a
		mov eax, dword ptr [ebp+0x20]
	jmp_10032990:
		mov byte ptr [edi], al
		add edx, ebx
		adc edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_100329b4 */
		_emit 0x1b
		mov byte ptr [edi], al
		add edx, ebx
		adc edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_100329b4 */
		_emit 0x12
		mov byte ptr [edi], al
		add edx, ebx
		adc edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_100329b4 */
		_emit 0x09
		mov byte ptr [edi], al
		add edx, ebx
		adc edi, esi
		dec ecx
		_emit 0x75 /* jnz jmp_10032990 */
		_emit 0xdc
	jmp_100329b4:
		jmp jmp_10032e31
	jmp_100329b9:
		mov eax, dword ptr [ebp+0x20]
		push ebp
		mov ebp, ebx
		mov ebx, eax
	jmp_100329c1:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		adc edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_100329f1 */
		_emit 0x24
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		adc edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_100329f1 */
		_emit 0x18
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		adc edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_100329f1 */
		_emit 0x0c
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		adc edi, esi
		dec ecx
		_emit 0x75 /* jnz jmp_100329c1 */
		_emit 0xd0
	jmp_100329f1:
		pop ebp
		jmp jmp_10032e31
	jmp_100329f7:
		mov esi, dword ptr [ebp+0x8]
		mov edi, dword ptr [ebp-0x24]
		sub edi, dword ptr [esi+0x4]
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [esi+0x8]
		mov esi, eax
		mov eax, dword ptr [ebp-0x18]
		add eax, eax
		inc eax
	jmp_10032a0e:
		pushad
		call dword ptr [ebp+0x20]
		popad
		add edx, ebx
		_emit 0x73 /* jae jmp_10032a18 */
		_emit 0x01
		inc edi
	jmp_10032a18:
		add esi, eax
		dec ecx
		_emit 0x75 /* jnz jmp_10032a0e */
		_emit 0xf1
		jmp jmp_10032e31
	jmp_10032a22:
		neg esi
		cmp dword ptr [ebp+0x1c], 0x1
		_emit 0x74 /* jz jmp_10032a58 */
		_emit 0x2e
		_emit 0x7f /* jg jmp_10032a96 */
		_emit 0x6a
		mov eax, dword ptr [ebp+0x20]
	jmp_10032a2f:
		mov byte ptr [edi], al
		add edx, ebx
		sbb edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032a53 */
		_emit 0x1b
		mov byte ptr [edi], al
		add edx, ebx
		sbb edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032a53 */
		_emit 0x12
		mov byte ptr [edi], al
		add edx, ebx
		sbb edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032a53 */
		_emit 0x09
		mov byte ptr [edi], al
		add edx, ebx
		sbb edi, esi
		dec ecx
		_emit 0x75 /* jnz jmp_10032a2f */
		_emit 0xdc
	jmp_10032a53:
		jmp jmp_10032e31
	jmp_10032a58:
		mov eax, dword ptr [ebp+0x20]
		push ebp
		mov ebp, ebx
		mov ebx, eax
	jmp_10032a60:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		sbb edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032a90 */
		_emit 0x24
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		sbb edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032a90 */
		_emit 0x18
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		sbb edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032a90 */
		_emit 0x0c
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edx, ebp
		sbb edi, esi
		dec ecx
		_emit 0x75 /* jnz jmp_10032a60 */
		_emit 0xd0
	jmp_10032a90:
		pop ebp
		jmp jmp_10032e31
	jmp_10032a96:
		mov esi, dword ptr [ebp+0x8]
		mov edi, dword ptr [ebp-0x24]
		sub edi, dword ptr [esi+0x4]
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [esi+0x8]
		mov esi, eax
		mov eax, dword ptr [ebp-0x18]
		add eax, eax
		inc eax
	jmp_10032aad:
		pushad
		call dword ptr [ebp+0x20]
		popad
		add edx, ebx
		_emit 0x73 /* jae jmp_10032ab7 */
		_emit 0x01
		dec edi
	jmp_10032ab7:
		add esi, eax
		dec ecx
		_emit 0x75 /* jnz jmp_10032aad */
		_emit 0xf1
		jmp jmp_10032e31
	jmp_10032ac1:
		mov eax, dword ptr [ebp-0x2c]
		sub eax, dword ptr [ebp-0x24]
		cdq
		xor eax, edx
		sub eax, edx
		inc eax
		mov ecx, eax
		mov eax, dword ptr [ebp-0x24]
		sub eax, dword ptr [ebp+0xc]
		cdq
		xor eax, edx
		sub eax, edx
		mul ebx
		add eax, 0x80000000
		mov edx, eax
		cmp dword ptr [ebp-0xc], -0x1
		jz jmp_10032ba6
		cmp dword ptr [ebp+0x1c], 0x1
		_emit 0x74 /* jz jmp_10032b31 */
		_emit 0x3e
		jg jmp_10032b7b
		mov eax, dword ptr [ebp+0x20]
	jmp_10032afc:
		mov byte ptr [edi], al
		inc edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10032b05 */
		_emit 0x02
		add edi, esi
	jmp_10032b05:
		dec ecx
		_emit 0x74 /* jz jmp_10032b2c */
		_emit 0x24
		mov byte ptr [edi], al
		inc edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10032b11 */
		_emit 0x02
		add edi, esi
	jmp_10032b11:
		dec ecx
		_emit 0x74 /* jz jmp_10032b2c */
		_emit 0x18
		mov byte ptr [edi], al
		inc edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10032b1d */
		_emit 0x02
		add edi, esi
	jmp_10032b1d:
		dec ecx
		_emit 0x74 /* jz jmp_10032b2c */
		_emit 0x0c
		mov byte ptr [edi], al
		inc edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10032b29 */
		_emit 0x02
		add edi, esi
	jmp_10032b29:
		dec ecx
		_emit 0x75 /* jnz jmp_10032afc */
		_emit 0xd0
	jmp_10032b2c:
		jmp jmp_10032e31
	jmp_10032b31:
		mov eax, dword ptr [ebp+0x20]
		push ebp
		mov ebp, ebx
		mov ebx, eax
	jmp_10032b39:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		inc edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10032b45 */
		_emit 0x02
		add edi, esi
	jmp_10032b45:
		dec ecx
		_emit 0x74 /* jz jmp_10032b75 */
		_emit 0x2d
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		inc edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10032b54 */
		_emit 0x02
		add edi, esi
	jmp_10032b54:
		dec ecx
		_emit 0x74 /* jz jmp_10032b75 */
		_emit 0x1e
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		inc edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10032b63 */
		_emit 0x02
		add edi, esi
	jmp_10032b63:
		dec ecx
		_emit 0x74 /* jz jmp_10032b75 */
		_emit 0x0f
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		inc edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10032b72 */
		_emit 0x02
		add edi, esi
	jmp_10032b72:
		dec ecx
		_emit 0x75 /* jnz jmp_10032b39 */
		_emit 0xc4
	jmp_10032b75:
		pop ebp
		jmp jmp_10032e31
	jmp_10032b7b:
		mov esi, dword ptr [ebp+0x8]
		mov edi, dword ptr [ebp-0x24]
		sub edi, dword ptr [esi+0x4]
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [esi+0x8]
		mov esi, eax
		mov eax, dword ptr [ebp-0x18]
		add eax, eax
		inc eax
	jmp_10032b92:
		pushad
		call dword ptr [ebp+0x20]
		popad
		add edx, ebx
		_emit 0x73 /* jae jmp_10032b9c */
		_emit 0x01
		inc esi
	jmp_10032b9c:
		add edi, eax
		dec ecx
		_emit 0x75 /* jnz jmp_10032b92 */
		_emit 0xf1
		jmp jmp_10032e31
	jmp_10032ba6:
		cmp dword ptr [ebp+0x1c], 0x1
		_emit 0x74 /* jz jmp_10032bea */
		_emit 0x3e
		jg jmp_10032c34
		mov eax, dword ptr [ebp+0x20]
	jmp_10032bb5:
		mov byte ptr [edi], al
		dec edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10032bbe */
		_emit 0x02
		add edi, esi
	jmp_10032bbe:
		dec ecx
		_emit 0x74 /* jz jmp_10032be5 */
		_emit 0x24
		mov byte ptr [edi], al
		dec edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10032bca */
		_emit 0x02
		add edi, esi
	jmp_10032bca:
		dec ecx
		_emit 0x74 /* jz jmp_10032be5 */
		_emit 0x18
		mov byte ptr [edi], al
		dec edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10032bd6 */
		_emit 0x02
		add edi, esi
	jmp_10032bd6:
		dec ecx
		_emit 0x74 /* jz jmp_10032be5 */
		_emit 0x0c
		mov byte ptr [edi], al
		dec edi
		add edx, ebx
		_emit 0x73 /* jae jmp_10032be2 */
		_emit 0x02
		add edi, esi
	jmp_10032be2:
		dec ecx
		_emit 0x75 /* jnz jmp_10032bb5 */
		_emit 0xd0
	jmp_10032be5:
		jmp jmp_10032e31
	jmp_10032bea:
		mov eax, dword ptr [ebp+0x20]
		push ebp
		mov ebp, ebx
		mov ebx, eax
	jmp_10032bf2:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		dec edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10032bfe */
		_emit 0x02
		add edi, esi
	jmp_10032bfe:
		dec ecx
		_emit 0x74 /* jz jmp_10032c2e */
		_emit 0x2d
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		dec edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10032c0d */
		_emit 0x02
		add edi, esi
	jmp_10032c0d:
		dec ecx
		_emit 0x74 /* jz jmp_10032c2e */
		_emit 0x1e
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		dec edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10032c1c */
		_emit 0x02
		add edi, esi
	jmp_10032c1c:
		dec ecx
		_emit 0x74 /* jz jmp_10032c2e */
		_emit 0x0f
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		dec edi
		add edx, ebp
		_emit 0x73 /* jae jmp_10032c2b */
		_emit 0x02
		add edi, esi
	jmp_10032c2b:
		dec ecx
		_emit 0x75 /* jnz jmp_10032bf2 */
		_emit 0xc4
	jmp_10032c2e:
		pop ebp
		jmp jmp_10032e31
	jmp_10032c34:
		mov esi, dword ptr [ebp+0x8]
		mov edi, dword ptr [ebp-0x24]
		sub edi, dword ptr [esi+0x4]
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [esi+0x8]
		mov esi, eax
		mov eax, dword ptr [ebp-0x18]
		add eax, eax
		inc eax
	jmp_10032c4b:
		pushad
		call dword ptr [ebp+0x20]
		popad
		add edx, ebx
		_emit 0x73 /* jae jmp_10032c55 */
		_emit 0x01
		dec esi
	jmp_10032c55:
		add edi, eax
		dec ecx
		_emit 0x75 /* jnz jmp_10032c4b */
		_emit 0xf1
		jmp jmp_10032e31
	jmp_10032c5f:
		mov eax, dword ptr [ebp+0xc]
		cmp eax, dword ptr [ebp-0x3c]
		jl jmp_10032e40
		cmp eax, dword ptr [ebp-0x44]
		jg jmp_10032e40
		mov eax, dword ptr [ebp+0x10]
		cmp eax, dword ptr [ebp+0x18]
		_emit 0x7f /* jg jmp_10032c7f */
		_emit 0x03
		mov eax, dword ptr [ebp+0x18]
	jmp_10032c7f:
		cmp eax, dword ptr [ebp-0x40]
		jl jmp_10032e40
		mov eax, dword ptr [ebp+0x10]
		cmp eax, dword ptr [ebp+0x18]
		_emit 0x7c /* jl jmp_10032c93 */
		_emit 0x03
		mov eax, dword ptr [ebp+0x18]
	jmp_10032c93:
		cmp eax, dword ptr [ebp-0x48]
		jg jmp_10032e40
		mov eax, dword ptr [ebp+0x10]
		cmp eax, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_10032ca7 */
		_emit 0x03
		mov eax, dword ptr [ebp-0x40]
	jmp_10032ca7:
		cmp eax, dword ptr [ebp-0x48]
		_emit 0x7c /* jl jmp_10032caf */
		_emit 0x03
		mov eax, dword ptr [ebp-0x48]
	jmp_10032caf:
		mov dword ptr [ebp-0x28], eax
		mov eax, dword ptr [ebp+0x18]
		cmp eax, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_10032cbd */
		_emit 0x03
		mov eax, dword ptr [ebp-0x40]
	jmp_10032cbd:
		cmp eax, dword ptr [ebp-0x48]
		_emit 0x7c /* jl jmp_10032cc5 */
		_emit 0x03
		mov eax, dword ptr [ebp-0x48]
	jmp_10032cc5:
		mov dword ptr [ebp-0x30], eax
		mov eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x24], eax
		mov esi, dword ptr [ebp-0x50]
		xor esi, dword ptr [ebp-0x18]
		sub esi, dword ptr [ebp-0x18]
		mov eax, dword ptr [ebp-0x30]
		sub eax, dword ptr [ebp-0x28]
		cdq
		xor eax, edx
		sub eax, edx
		mov ecx, eax
		inc ecx
		jmp jmp_10032d90
	jmp_10032cea:
		mov eax, dword ptr [ebp+0x10]
		cmp eax, dword ptr [ebp-0x40]
		jl jmp_10032e40
		cmp eax, dword ptr [ebp-0x48]
		jg jmp_10032e40
		mov eax, dword ptr [ebp+0xc]
		cmp eax, dword ptr [ebp+0x14]
		_emit 0x7f /* jg jmp_10032d0a */
		_emit 0x03
		mov eax, dword ptr [ebp+0x14]
	jmp_10032d0a:
		cmp eax, dword ptr [ebp-0x3c]
		jl jmp_10032e40
		mov eax, dword ptr [ebp+0xc]
		cmp eax, dword ptr [ebp+0x14]
		_emit 0x7c /* jl jmp_10032d1e */
		_emit 0x03
		mov eax, dword ptr [ebp+0x14]
	jmp_10032d1e:
		cmp eax, dword ptr [ebp-0x44]
		jg jmp_10032e40
		mov eax, dword ptr [ebp+0xc]
		cmp eax, dword ptr [ebp-0x3c]
		_emit 0x7f /* jg jmp_10032d32 */
		_emit 0x03
		mov eax, dword ptr [ebp-0x3c]
	jmp_10032d32:
		cmp eax, dword ptr [ebp-0x44]
		_emit 0x7c /* jl jmp_10032d3a */
		_emit 0x03
		mov eax, dword ptr [ebp-0x44]
	jmp_10032d3a:
		mov dword ptr [ebp-0x24], eax
		mov eax, dword ptr [ebp+0x14]
		cmp eax, dword ptr [ebp-0x3c]
		_emit 0x7f /* jg jmp_10032d48 */
		_emit 0x03
		mov eax, dword ptr [ebp-0x3c]
	jmp_10032d48:
		cmp eax, dword ptr [ebp-0x44]
		_emit 0x7c /* jl jmp_10032d50 */
		_emit 0x03
		mov eax, dword ptr [ebp-0x44]
	jmp_10032d50:
		mov dword ptr [ebp-0x2c], eax
		mov eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x28], eax
		mov esi, dword ptr [ebp-0xc]
		inc esi
		or esi, dword ptr [ebp-0xc]
		mov eax, dword ptr [ebp-0x2c]
		sub eax, dword ptr [ebp-0x24]
		cdq
		xor eax, edx
		sub eax, edx
		mov ecx, eax
		inc ecx
		_emit 0xeb /* jmp jmp_10032d90 */
		_emit 0x20
	jmp_10032d70:
		mov esi, dword ptr [ebp-0x50]
		xor esi, dword ptr [ebp-0x18]
		sub esi, dword ptr [ebp-0x18]
		mov eax, dword ptr [ebp-0xc]
		inc eax
		or eax, dword ptr [ebp-0xc]
		add esi, eax
		mov eax, dword ptr [ebp-0x2c]
		sub eax, dword ptr [ebp-0x24]
		cdq
		xor eax, edx
		sub eax, edx
		mov ecx, eax
		inc ecx
	jmp_10032d90:
		mov eax, dword ptr [ebp-0x28]
		imul dword ptr [ebp-0x50]
		add eax, dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x24]
		mov edi, eax
		cmp dword ptr [ebp+0x1c], 0x1
		_emit 0x74 /* jz jmp_10032dc7 */
		_emit 0x23
		_emit 0x7f /* jg jmp_10032df4 */
		_emit 0x4e
		mov eax, dword ptr [ebp+0x20]
	jmp_10032da9:
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032dc5 */
		_emit 0x15
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032dc5 */
		_emit 0x0e
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032dc5 */
		_emit 0x07
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x75 /* jnz jmp_10032da9 */
		_emit 0xe4
	jmp_10032dc5:
		_emit 0xeb /* jmp jmp_10032e31 */
		_emit 0x6a
	jmp_10032dc7:
		mov ebx, dword ptr [ebp+0x20]
	jmp_10032dca:
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032df2 */
		_emit 0x1e
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032df2 */
		_emit 0x14
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x74 /* jz jmp_10032df2 */
		_emit 0x0a
		mov al, byte ptr [edi]
		xlatb
		mov byte ptr [edi], al
		add edi, esi
		dec ecx
		_emit 0x75 /* jnz jmp_10032dca */
		_emit 0xd8
	jmp_10032df2:
		_emit 0xeb /* jmp jmp_10032e31 */
		_emit 0x3d
	jmp_10032df4:
		mov esi, dword ptr [ebp+0x8]
		mov edi, dword ptr [ebp-0x24]
		sub edi, dword ptr [esi+0x4]
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [esi+0x8]
		mov esi, eax
		xor eax, eax
		test dword ptr [ebp-0x10], 0xffffffff
		setne al
		or eax, dword ptr [ebp-0x18]
		xor ebx, ebx
		test dword ptr [ebp-0x4], 0xffffffff
		setne bl
		or ebx, dword ptr [ebp-0xc]
	jmp_10032e23:
		pushad
		call dword ptr [ebp+0x20]
		popad
		add esi, eax
		add edi, ebx
		dec ecx
		_emit 0x75 /* jnz jmp_10032e23 */
		_emit 0xf4
		_emit 0xeb /* jmp jmp_10032e31 */
		_emit 0x00
	jmp_10032e31:
		xor eax, eax
		cmp dword ptr [ebp-0x34], 0x1
		setae al
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10032e40:
		mov eax, 0x2
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns 0 when drawn, or a negative code when the view is empty or everything is clipped.
#ifdef COMPAT_MODE
MechS32 FUN_10032e4b(
	PixelView* p_view,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14
)
{
	STUB(0x10032e4b);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10032e4b
__declspec(naked) MechS32 FUN_10032e4b(
	PixelView* p_view,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x20
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x18], eax
		_emit 0x7e /* jle jmp_10032eca */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10032eca */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x1c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10032e7e */
		_emit 0x05
		mov eax, 0x0
	jmp_10032e7e:
		mov dword ptr [ebp-0x4], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x20], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10032e91 */
		_emit 0x05
		mov eax, 0x0
	jmp_10032e91:
		mov dword ptr [ebp-0x8], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x18]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10032ea1 */
		_emit 0x02
		mov eax, edx
	jmp_10032ea1:
		mov dword ptr [ebp-0xc], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10032eb0 */
		_emit 0x02
		mov eax, edx
	jmp_10032eb0:
		mov dword ptr [ebp-0x10], eax
		mov eax, dword ptr [ebp-0xc]
		cmp eax, dword ptr [ebp-0x4]
		_emit 0x7c /* jl jmp_10032ed5 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x10]
		cmp eax, dword ptr [ebp-0x8]
		_emit 0x7c /* jl jmp_10032ed5 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x14], eax
		_emit 0xeb /* jmp jmp_10032ee0 */
		_emit 0x16
	jmp_10032eca:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10032ed5:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10032ee0:
		mov eax, dword ptr [ebp-0x1c]
		add dword ptr [ebp+0xc], eax
		add dword ptr [ebp+0x14], eax
		mov eax, dword ptr [ebp-0x20]
		add dword ptr [ebp+0x10], eax
		add dword ptr [ebp+0x18], eax
		mov eax, dword ptr [ebp-0x4]
		cmp dword ptr [ebp+0xc], eax
		_emit 0x7f /* jg jmp_10032efd */
		_emit 0x03
		mov dword ptr [ebp+0xc], eax
	jmp_10032efd:
		mov eax, dword ptr [ebp-0x8]
		cmp dword ptr [ebp+0x10], eax
		_emit 0x7f /* jg jmp_10032f08 */
		_emit 0x03
		mov dword ptr [ebp+0x10], eax
	jmp_10032f08:
		mov eax, dword ptr [ebp-0xc]
		cmp dword ptr [ebp+0x14], eax
		_emit 0x7c /* jl jmp_10032f13 */
		_emit 0x03
		mov dword ptr [ebp+0x14], eax
	jmp_10032f13:
		mov eax, dword ptr [ebp-0x10]
		cmp dword ptr [ebp+0x18], eax
		_emit 0x7c /* jl jmp_10032f1e */
		_emit 0x03
		mov dword ptr [ebp+0x18], eax
	jmp_10032f1e:
		mov ecx, dword ptr [ebp+0x14]
		sub ecx, dword ptr [ebp+0xc]
		_emit 0x7c /* jl jmp_10032f79 */
		_emit 0x53
		inc ecx
		mov eax, dword ptr [ebp+0x10]
		imul dword ptr [ebp-0x18]
		add eax, dword ptr [ebp-0x14]
		add eax, dword ptr [ebp+0xc]
		mov edi, eax
		mov edx, dword ptr [ebp+0x18]
		sub edx, dword ptr [ebp+0x10]
		_emit 0x7c /* jl jmp_10032f79 */
		_emit 0x3c
		mov eax, dword ptr [ebp+0x1c]
		mov esi, edi
		mov ebx, ecx
		_emit 0xeb /* jmp jmp_10032f4d */
		_emit 0x07
	jmp_10032f46:
		add esi, dword ptr [ebp-0x18]
		mov edi, esi
		mov ecx, ebx
	jmp_10032f4d:
		push edx
		and edx, 0x1
		_emit 0x74 /* jz jmp_10032f58 */
		_emit 0x05
		pop edx
		inc edi
		dec ecx
		_emit 0xeb /* jmp jmp_10032f59 */
		_emit 0x01
	jmp_10032f58:
		pop edx
	jmp_10032f59:
		mov byte ptr [edi], al
		add edi, 0x2
		sub ecx, 0x2
		_emit 0x7f /* jg jmp_10032f59 */
		_emit 0xf6
		dec edx
		_emit 0x79 /* jns jmp_10032f46 */
		_emit 0xe0
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10032f79:
		mov eax, 0xfffffffc
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Draws an image of the data into the view, clipped, through FUN_100333f8. Returns 0 when
// drawn, or a negative code when the view is empty or everything is clipped.
#ifdef COMPAT_MODE
MechS32 FUN_10032f84(PixelView* p_view, undefined4 p_unk0x04, undefined4 p_unk0x08, MechS32 p_left, MechS32 p_top)
{
	STUB(0x10032f84);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10032f84
__declspec(naked) MechS32
FUN_10032f84(PixelView* p_view, undefined4 p_unk0x04, undefined4 p_unk0x08, MechS32 p_left, MechS32 p_top)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x50
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x48], eax
		_emit 0x7e /* jle jmp_10033003 */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10033003 */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x4c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10032fb7 */
		_emit 0x05
		mov eax, 0x0
	jmp_10032fb7:
		mov dword ptr [ebp-0x34], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x50], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10032fca */
		_emit 0x05
		mov eax, 0x0
	jmp_10032fca:
		mov dword ptr [ebp-0x38], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x48]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10032fda */
		_emit 0x02
		mov eax, edx
	jmp_10032fda:
		mov dword ptr [ebp-0x3c], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10032fe9 */
		_emit 0x02
		mov eax, edx
	jmp_10032fe9:
		mov dword ptr [ebp-0x40], eax
		mov eax, dword ptr [ebp-0x3c]
		cmp eax, dword ptr [ebp-0x34]
		_emit 0x7c /* jl jmp_1003300e */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x40]
		cmp eax, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_1003300e */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x44], eax
		_emit 0xeb /* jmp jmp_10033019 */
		_emit 0x16
	jmp_10033003:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_1003300e:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10033019:
		mov eax, dword ptr [ebp-0x4c]
		add dword ptr [ebp+0x14], eax
		mov eax, dword ptr [ebp-0x50]
		add dword ptr [ebp+0x18], eax
		mov esi, dword ptr [ebp+0x10]
		shl esi, 0x3
		add esi, 0x8
		add esi, dword ptr [ebp+0xc]
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x30], esi
		mov eax, dword ptr [esi+0x8]
		add eax, dword ptr [ebp+0x14]
		mov dword ptr [ebp-0xc], eax
		mov eax, dword ptr [esi+0xc]
		add eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x10], eax
		mov eax, dword ptr [esi+0x10]
		add eax, dword ptr [ebp+0x14]
		mov dword ptr [ebp-0x14], eax
		mov eax, dword ptr [esi+0x14]
		add eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x18], eax
		add esi, 0x18
		mov eax, dword ptr [ebp-0x14]
		cmp eax, dword ptr [ebp-0xc]
		jl jmp_100333ed
		mov eax, dword ptr [ebp-0x18]
		cmp eax, dword ptr [ebp-0x10]
		jl jmp_100333ed
		xor edx, edx
		mov eax, dword ptr [ebp-0xc]
		sub eax, dword ptr [ebp-0x34]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x3c]
		sub eax, dword ptr [ebp-0xc]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x10]
		sub eax, dword ptr [ebp-0x38]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x40]
		sub eax, dword ptr [ebp-0x10]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x14]
		sub eax, dword ptr [ebp-0x34]
		shl eax, 0x1
		adc dh, dh
		mov eax, dword ptr [ebp-0x3c]
		sub eax, dword ptr [ebp-0x14]
		shl eax, 0x1
		adc dh, dh
		mov eax, dword ptr [ebp-0x18]
		sub eax, dword ptr [ebp-0x38]
		shl eax, 0x1
		adc dh, dh
		mov eax, dword ptr [ebp-0x40]
		sub eax, dword ptr [ebp-0x18]
		shl eax, 0x1
		adc dh, dh
		mov dword ptr [ebp-0x1c], edx
		_emit 0x84 /* test dh, dl: the inline assembler encodes the operands the other way */
		_emit 0xd6
		jnz jmp_100333e2
		or dl, dh
		_emit 0x75 /* jnz jmp_10033104 */
		_emit 0x2b
		mov esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0x4]
		sub dword ptr [ebp+0x14], eax
		mov eax, dword ptr [esi+0x8]
		sub dword ptr [ebp+0x18], eax
		push dword ptr [ebp-0x48]
		push dword ptr [ebp+0x18]
		push dword ptr [ebp+0x14]
		push dword ptr [ebp-0x30]
		push dword ptr [ebp+0x8]
		call FUN_100333f8
		add esp, 0x14
		jmp jmp_100333da
	jmp_10033104:
		mov eax, dword ptr [ebp-0x10]
		imul dword ptr [ebp-0x48]
		add eax, dword ptr [ebp-0x44]
		add eax, dword ptr [ebp-0xc]
		mov edi, eax
		mov ecx, dword ptr [ebp-0x10]
		mov dword ptr [ebp-0x20], ecx
		_emit 0xeb /* jmp jmp_10033130 */
		_emit 0x16
	jmp_1003311a:
		movzx eax, al
		add esi, eax
		dec esi
	jmp_10033120:
		inc esi
	jmp_10033121:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033120 */
		_emit 0xf8
		_emit 0x75 /* jnz jmp_1003311a */
		_emit 0xf0
		_emit 0x72 /* jb jmp_10033120 */
		_emit 0xf4
		add edi, dword ptr [ebp-0x48]
		inc ecx
	jmp_10033130:
		cmp ecx, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_10033121 */
		_emit 0xec
		mov dword ptr [ebp-0x24], edi
		mov dword ptr [ebp-0x20], ecx
		mov eax, edi
		sub eax, dword ptr [ebp-0xc]
		add eax, dword ptr [ebp-0x34]
		mov dword ptr [ebp-0x28], eax
		mov eax, edi
		sub eax, dword ptr [ebp-0xc]
		add eax, dword ptr [ebp-0x3c]
		mov dword ptr [ebp-0x2c], eax
		jmp jmp_100333ce
	jmp_10033156:
		mov eax, dword ptr [ebp-0x20]
		cmp eax, dword ptr [ebp-0x40]
		jg jmp_100333da
		mov edi, dword ptr [ebp-0x24]
		test dword ptr [ebp-0x1c], 0x8
		jnz jmp_1003321b
		test dword ptr [ebp-0x1c], 0x400
		jnz jmp_100332a9
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100331a3 */
		_emit 0x1d
		_emit 0x75 /* jnz jmp_100331e2 */
		_emit 0x5a
		jae jmp_10033216
	jmp_1003318e:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100331a3 */
		_emit 0x06
		_emit 0x75 /* jnz jmp_100331e2 */
		_emit 0x43
		_emit 0x72 /* jb jmp_1003318e */
		_emit 0xed
		_emit 0x73 /* jae jmp_10033216 */
		_emit 0x73
	jmp_100331a3:
		movzx ecx, al
	jmp_100331a6:
		mov al, byte ptr [esi]
		inc esi
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_100331d5 */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 0x8
		neg ecx
		and ecx, 0x3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 0x8
		mov ecx, ebx
		mov al, ah
		and ebx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, ebx
		pop ebx
	jmp_100331d5:
		rep stosb
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100331a3 */
		_emit 0xc5
		_emit 0x73 /* jae jmp_10033216 */
		_emit 0x36
		_emit 0x74 /* jz jmp_1003318e */
		_emit 0xac
	jmp_100331e2:
		movzx ecx, al
	jmp_100331e5:
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_10033205 */
		_emit 0x1b
		push ebx
		mov ebx, ecx
		mov ecx, edi
		neg ecx
		and ecx, 0x3
		sub ebx, ecx
		rep movsb
		mov ecx, ebx
		and ebx, 0x3
		shr ecx, 0x2
		rep movsd
		mov ecx, ebx
		pop ebx
	jmp_10033205:
		rep movsb
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100331a3 */
		_emit 0x95
		_emit 0x75 /* jnz jmp_100331e2 */
		_emit 0xd2
		jb jmp_1003318e
	jmp_10033216:
		jmp jmp_100333bf
	jmp_1003321b:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003323b */
		_emit 0x19
		_emit 0x75 /* jnz jmp_1003326c */
		_emit 0x48
		_emit 0x73 /* jae jmp_100332a4 */
		_emit 0x7e
	jmp_10033226:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003323b */
		_emit 0x06
		_emit 0x75 /* jnz jmp_1003326c */
		_emit 0x35
		_emit 0x72 /* jb jmp_10033226 */
		_emit 0xed
		_emit 0x73 /* jae jmp_100332a4 */
		_emit 0x69
	jmp_1003323b:
		movzx ecx, al
		mov eax, dword ptr [ebp-0x28]
		sub eax, edi
		cmp eax, ecx
		_emit 0x7d /* jge jmp_1003325e */
		_emit 0x17
		or eax, eax
		_emit 0x78 /* js jmp_1003324f */
		_emit 0x04
		add edi, eax
		sub ecx, eax
	jmp_1003324f:
		test dword ptr [ebp-0x1c], 0x400
		jz jmp_100331a6
		_emit 0x75 /* jnz jmp_100332d4 */
		_emit 0x76
	jmp_1003325e:
		add edi, ecx
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003323b */
		_emit 0xd3
		_emit 0x73 /* jae jmp_100332a4 */
		_emit 0x3a
		_emit 0x74 /* jz jmp_10033226 */
		_emit 0xba
	jmp_1003326c:
		movzx ecx, al
		mov eax, dword ptr [ebp-0x28]
		sub eax, edi
		cmp eax, ecx
		_emit 0x7d /* jge jmp_10033295 */
		_emit 0x1d
		or eax, eax
		_emit 0x78 /* js jmp_10033282 */
		_emit 0x06
		add edi, eax
		sub ecx, eax
		add esi, eax
	jmp_10033282:
		test dword ptr [ebp-0x1c], 0x400
		jz jmp_100331e5
		jnz jmp_1003332d
	jmp_10033295:
		add edi, ecx
		add esi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003323b */
		_emit 0x9b
		_emit 0x75 /* jnz jmp_1003326c */
		_emit 0xca
		_emit 0x72 /* jb jmp_10033226 */
		_emit 0x82
	jmp_100332a4:
		jmp jmp_100333bf
	jmp_100332a9:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100332d1 */
		_emit 0x21
		_emit 0x75 /* jnz jmp_1003332a */
		_emit 0x78
		jae jmp_1003337a
	jmp_100332b8:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100332d1 */
		_emit 0x0a
		_emit 0x75 /* jnz jmp_1003332a */
		_emit 0x61
		_emit 0x72 /* jb jmp_100332b8 */
		_emit 0xed
		jae jmp_1003337a
	jmp_100332d1:
		movzx ecx, al
	jmp_100332d4:
		cmp edi, dword ptr [ebp-0x2c]
		jg jmp_1003339f
		mov eax, edi
		add eax, ecx
		dec eax
		sub eax, dword ptr [ebp-0x2c]
		cdq
		not edx
		and edx, eax
		sub ecx, edx
		mov al, byte ptr [esi]
		inc esi
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_1003331b */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 0x8
		neg ecx
		and ecx, 0x3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 0x8
		mov ecx, ebx
		mov al, ah
		and ebx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, ebx
		pop ebx
	jmp_1003331b:
		rep stosb
		add edi, edx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100332d1 */
		_emit 0xab
		_emit 0x73 /* jae jmp_1003337a */
		_emit 0x52
		_emit 0x74 /* jz jmp_100332b8 */
		_emit 0x8e
	jmp_1003332a:
		movzx ecx, al
	jmp_1003332d:
		cmp edi, dword ptr [ebp-0x2c]
		_emit 0x7f /* jg jmp_100333b0 */
		_emit 0x7e
		mov eax, edi
		add eax, ecx
		dec eax
		sub eax, dword ptr [ebp-0x2c]
		cdq
		not edx
		and edx, eax
		sub ecx, edx
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_10033361 */
		_emit 0x1b
		push ebx
		mov ebx, ecx
		mov ecx, edi
		neg ecx
		and ecx, 0x3
		sub ebx, ecx
		rep movsb
		mov ecx, ebx
		and ebx, 0x3
		shr ecx, 0x2
		rep movsd
		mov ecx, ebx
		pop ebx
	jmp_10033361:
		rep movsb
		add edi, edx
		add esi, edx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		ja jmp_100332d1
		_emit 0x75 /* jnz jmp_1003332a */
		_emit 0xb6
		jb jmp_100332b8
	jmp_1003337a:
		_emit 0xeb /* jmp jmp_100333bf */
		_emit 0x43
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003339c */
		_emit 0x19
		_emit 0x75 /* jnz jmp_100333ad */
		_emit 0x28
		_emit 0x73 /* jae jmp_100333bf */
		_emit 0x38
	jmp_10033387:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003339c */
		_emit 0x06
		_emit 0x75 /* jnz jmp_100333ad */
		_emit 0x15
		_emit 0x72 /* jb jmp_10033387 */
		_emit 0xed
		_emit 0x73 /* jae jmp_100333bf */
		_emit 0x23
	jmp_1003339c:
		movzx ecx, al
	jmp_1003339f:
		add edi, ecx
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003339c */
		_emit 0xf3
		_emit 0x73 /* jae jmp_100333bf */
		_emit 0x14
		_emit 0x74 /* jz jmp_10033387 */
		_emit 0xda
	jmp_100333ad:
		movzx ecx, al
	jmp_100333b0:
		add edi, ecx
		add esi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003339c */
		_emit 0xe1
		_emit 0x75 /* jnz jmp_100333ad */
		_emit 0xf0
		_emit 0x72 /* jb jmp_10033387 */
		_emit 0xc8
	jmp_100333bf:
		mov eax, dword ptr [ebp-0x48]
		add dword ptr [ebp-0x24], eax
		add dword ptr [ebp-0x28], eax
		add dword ptr [ebp-0x2c], eax
		inc dword ptr [ebp-0x20]
	jmp_100333ce:
		mov eax, dword ptr [ebp-0x20]
		cmp eax, dword ptr [ebp-0x18]
		jle jmp_10033156
	jmp_100333da:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_100333e2:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_100333ed:
		mov eax, 0xfffffffc
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
MechS32 FUN_100333f8(
	PixelView* p_view,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10
)
{
	STUB(0x100333f8);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100333f8
__declspec(naked) MechS32
FUN_100333f8(PixelView* p_view, undefined4 p_unk0x04, undefined4 p_unk0x08, undefined4 p_unk0x0c, undefined4 p_unk0x10)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [esi+0x4]
		add dword ptr [ebp+0x10], eax
		mov eax, dword ptr [esi+0x8]
		add dword ptr [ebp+0x14], eax
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp+0x18], eax
		jle jmp_100334f3
		mov esi, dword ptr [ebp+0xc]
		mov edi, dword ptr [ebx]
		mov eax, dword ptr [esi+0x8]
		add eax, dword ptr [ebp+0x10]
		add edi, eax
		mov eax, dword ptr [esi+0xc]
		mov ebx, eax
		add eax, dword ptr [ebp+0x14]
		mul dword ptr [ebp+0x18]
		add edi, eax
		mov edx, edi
		mov eax, dword ptr [esi+0x10]
		mov eax, dword ptr [esi+0x14]
		inc eax
		sub eax, ebx
		mov ebx, eax
		jle jmp_100334f3
		add esi, 0x18
	jmp_10033450:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033474 */
		_emit 0x1d
		_emit 0x75 /* jnz jmp_100334b3 */
		_emit 0x5a
		jae jmp_100334e7
	jmp_1003345f:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033474 */
		_emit 0x06
		_emit 0x75 /* jnz jmp_100334b3 */
		_emit 0x43
		_emit 0x72 /* jb jmp_1003345f */
		_emit 0xed
		_emit 0x73 /* jae jmp_100334e7 */
		_emit 0x73
	jmp_10033474:
		movzx ecx, al
		mov al, byte ptr [esi]
		inc esi
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_100334a6 */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 0x8
		neg ecx
		and ecx, 0x3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 0x8
		mov ecx, ebx
		mov al, ah
		and ebx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, ebx
		pop ebx
	jmp_100334a6:
		rep stosb
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033474 */
		_emit 0xc5
		_emit 0x73 /* jae jmp_100334e7 */
		_emit 0x36
		_emit 0x74 /* jz jmp_1003345f */
		_emit 0xac
	jmp_100334b3:
		movzx ecx, al
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_100334d6 */
		_emit 0x1b
		push ebx
		mov ebx, ecx
		mov ecx, edi
		neg ecx
		and ecx, 0x3
		sub ebx, ecx
		rep movsb
		mov ecx, ebx
		and ebx, 0x3
		shr ecx, 0x2
		rep movsd
		mov ecx, ebx
		pop ebx
	jmp_100334d6:
		rep movsb
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033474 */
		_emit 0x95
		_emit 0x75 /* jnz jmp_100334b3 */
		_emit 0xd2
		jb jmp_1003345f
	jmp_100334e7:
		add edx, dword ptr [ebp+0x18]
		mov edi, edx
		dec ebx
		jnz jmp_10033450
	jmp_100334f3:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Loads the 256-entry color remap table from p_table.
#ifdef COMPAT_MODE
void FUN_100334fb(undefined* p_table)
{
	STUB(0x100334fb);
}
#else
// FUNCTION: MW2SHELL 0x100334fb
__declspec(naked) void FUN_100334fb(undefined* p_table)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov edi, offset g_unk0x1006a05c
		mov ecx, 0x40
		rep movsd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// FUN_10032f84 with the pixels mapped through the color remap table (FUN_10033980).
#ifdef COMPAT_MODE
MechS32 FUN_1003351a(PixelView* p_view, undefined4 p_unk0x04, undefined4 p_unk0x08, MechS32 p_left, MechS32 p_top)
{
	STUB(0x1003351a);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x1003351a
__declspec(naked) MechS32
FUN_1003351a(PixelView* p_view, undefined4 p_unk0x04, undefined4 p_unk0x08, MechS32 p_left, MechS32 p_top)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x50
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x48], eax
		_emit 0x7e /* jle jmp_10033599 */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10033599 */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x4c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_1003354d */
		_emit 0x05
		mov eax, 0x0
	jmp_1003354d:
		mov dword ptr [ebp-0x34], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x50], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10033560 */
		_emit 0x05
		mov eax, 0x0
	jmp_10033560:
		mov dword ptr [ebp-0x38], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x48]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10033570 */
		_emit 0x02
		mov eax, edx
	jmp_10033570:
		mov dword ptr [ebp-0x3c], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_1003357f */
		_emit 0x02
		mov eax, edx
	jmp_1003357f:
		mov dword ptr [ebp-0x40], eax
		mov eax, dword ptr [ebp-0x3c]
		cmp eax, dword ptr [ebp-0x34]
		_emit 0x7c /* jl jmp_100335a4 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x40]
		cmp eax, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_100335a4 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x44], eax
		_emit 0xeb /* jmp jmp_100335af */
		_emit 0x16
	jmp_10033599:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_100335a4:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_100335af:
		mov eax, dword ptr [ebp-0x4c]
		add dword ptr [ebp+0x14], eax
		mov eax, dword ptr [ebp-0x50]
		add dword ptr [ebp+0x18], eax
		mov esi, dword ptr [ebp+0x10]
		shl esi, 0x3
		add esi, 0x8
		add esi, dword ptr [ebp+0xc]
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x30], esi
		mov eax, dword ptr [esi+0x8]
		add eax, dword ptr [ebp+0x14]
		mov dword ptr [ebp-0xc], eax
		mov eax, dword ptr [esi+0xc]
		add eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x10], eax
		mov eax, dword ptr [esi+0x10]
		add eax, dword ptr [ebp+0x14]
		mov dword ptr [ebp-0x14], eax
		mov eax, dword ptr [esi+0x14]
		add eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x18], eax
		add esi, 0x18
		mov eax, dword ptr [ebp-0x14]
		cmp eax, dword ptr [ebp-0xc]
		jl jmp_10033975
		mov eax, dword ptr [ebp-0x18]
		cmp eax, dword ptr [ebp-0x10]
		jl jmp_10033975
		xor edx, edx
		mov eax, dword ptr [ebp-0xc]
		sub eax, dword ptr [ebp-0x34]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x3c]
		sub eax, dword ptr [ebp-0xc]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x10]
		sub eax, dword ptr [ebp-0x38]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x40]
		sub eax, dword ptr [ebp-0x10]
		shl eax, 0x1
		adc dl, dl
		mov eax, dword ptr [ebp-0x14]
		sub eax, dword ptr [ebp-0x34]
		shl eax, 0x1
		adc dh, dh
		mov eax, dword ptr [ebp-0x3c]
		sub eax, dword ptr [ebp-0x14]
		shl eax, 0x1
		adc dh, dh
		mov eax, dword ptr [ebp-0x18]
		sub eax, dword ptr [ebp-0x38]
		shl eax, 0x1
		adc dh, dh
		mov eax, dword ptr [ebp-0x40]
		sub eax, dword ptr [ebp-0x18]
		shl eax, 0x1
		adc dh, dh
		mov dword ptr [ebp-0x1c], edx
		_emit 0x84 /* test dh, dl: the inline assembler encodes the operands the other way */
		_emit 0xd6
		jnz jmp_1003396a
		or dl, dh
		_emit 0x75 /* jnz jmp_1003369a */
		_emit 0x2b
		mov esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0x4]
		sub dword ptr [ebp+0x14], eax
		mov eax, dword ptr [esi+0x8]
		sub dword ptr [ebp+0x18], eax
		push dword ptr [ebp-0x48]
		push dword ptr [ebp+0x18]
		push dword ptr [ebp+0x14]
		push dword ptr [ebp-0x30]
		push dword ptr [ebp+0x8]
		call FUN_10033980
		add esp, 0x14
		jmp jmp_10033962
	jmp_1003369a:
		mov eax, dword ptr [ebp-0x10]
		imul dword ptr [ebp-0x48]
		add eax, dword ptr [ebp-0x44]
		add eax, dword ptr [ebp-0xc]
		mov edi, eax
		mov ecx, dword ptr [ebp-0x10]
		mov dword ptr [ebp-0x20], ecx
		_emit 0xeb /* jmp jmp_100336c6 */
		_emit 0x16
	jmp_100336b0:
		movzx eax, al
		add esi, eax
		dec esi
	jmp_100336b6:
		inc esi
	jmp_100336b7:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100336b6 */
		_emit 0xf8
		_emit 0x75 /* jnz jmp_100336b0 */
		_emit 0xf0
		_emit 0x72 /* jb jmp_100336b6 */
		_emit 0xf4
		add edi, dword ptr [ebp-0x48]
		inc ecx
	jmp_100336c6:
		cmp ecx, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_100336b7 */
		_emit 0xec
		mov dword ptr [ebp-0x24], edi
		mov dword ptr [ebp-0x20], ecx
		mov eax, edi
		sub eax, dword ptr [ebp-0xc]
		add eax, dword ptr [ebp-0x34]
		mov dword ptr [ebp-0x28], eax
		mov eax, edi
		sub eax, dword ptr [ebp-0xc]
		add eax, dword ptr [ebp-0x3c]
		mov dword ptr [ebp-0x2c], eax
		jmp jmp_10033956
	jmp_100336ec:
		mov eax, dword ptr [ebp-0x20]
		cmp eax, dword ptr [ebp-0x40]
		jg jmp_10033962
		mov edi, dword ptr [ebp-0x24]
		test dword ptr [ebp-0x1c], 0x8
		jnz jmp_100337a4
		test dword ptr [ebp-0x1c], 0x400
		jnz jmp_10033832
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033735 */
		_emit 0x19
		_emit 0x75 /* jnz jmp_1003377c */
		_emit 0x5e
		_emit 0x73 /* jae jmp_1003379f */
		_emit 0x7f
	jmp_10033720:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033735 */
		_emit 0x06
		_emit 0x75 /* jnz jmp_1003377c */
		_emit 0x4b
		_emit 0x72 /* jb jmp_10033720 */
		_emit 0xed
		_emit 0x73 /* jae jmp_1003379f */
		_emit 0x6a
	jmp_10033735:
		movzx ecx, al
	jmp_10033738:
		xor eax, eax
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_unk0x1006a05c+eax]
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_1003376f */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 0x8
		neg ecx
		and ecx, 0x3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 0x8
		mov ecx, ebx
		mov al, ah
		and ebx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, ebx
		pop ebx
	jmp_1003376f:
		rep stosb
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033735 */
		_emit 0xbd
		_emit 0x73 /* jae jmp_1003379f */
		_emit 0x25
		_emit 0x74 /* jz jmp_10033720 */
		_emit 0xa4
	jmp_1003377c:
		movzx ecx, al
	jmp_1003377f:
		xor eax, eax
		or ecx, ecx
		_emit 0x74 /* jz jmp_10033794 */
		_emit 0x0f
	jmp_10033785:
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_unk0x1006a05c+eax]
		mov byte ptr [edi], al
		inc edi
		dec ecx
		_emit 0x75 /* jnz jmp_10033785 */
		_emit 0xf1
	jmp_10033794:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033735 */
		_emit 0x9a
		_emit 0x75 /* jnz jmp_1003377c */
		_emit 0xdf
		_emit 0x72 /* jb jmp_10033720 */
		_emit 0x81
	jmp_1003379f:
		jmp jmp_10033947
	jmp_100337a4:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100337c4 */
		_emit 0x19
		_emit 0x75 /* jnz jmp_100337f5 */
		_emit 0x48
		_emit 0x73 /* jae jmp_1003382d */
		_emit 0x7e
	jmp_100337af:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100337c4 */
		_emit 0x06
		_emit 0x75 /* jnz jmp_100337f5 */
		_emit 0x35
		_emit 0x72 /* jb jmp_100337af */
		_emit 0xed
		_emit 0x73 /* jae jmp_1003382d */
		_emit 0x69
	jmp_100337c4:
		movzx ecx, al
		mov eax, dword ptr [ebp-0x28]
		sub eax, edi
		cmp eax, ecx
		_emit 0x7d /* jge jmp_100337e7 */
		_emit 0x17
		or eax, eax
		_emit 0x78 /* js jmp_100337d8 */
		_emit 0x04
		add edi, eax
		sub ecx, eax
	jmp_100337d8:
		test dword ptr [ebp-0x1c], 0x400
		jz jmp_10033738
		_emit 0x75 /* jnz jmp_10033861 */
		_emit 0x7a
	jmp_100337e7:
		add edi, ecx
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100337c4 */
		_emit 0xd3
		_emit 0x73 /* jae jmp_1003382d */
		_emit 0x3a
		_emit 0x74 /* jz jmp_100337af */
		_emit 0xba
	jmp_100337f5:
		movzx ecx, al
		mov eax, dword ptr [ebp-0x28]
		sub eax, edi
		cmp eax, ecx
		_emit 0x7d /* jge jmp_1003381e */
		_emit 0x1d
		or eax, eax
		_emit 0x78 /* js jmp_1003380b */
		_emit 0x06
		add edi, eax
		sub ecx, eax
		add esi, eax
	jmp_1003380b:
		test dword ptr [ebp-0x1c], 0x400
		jz jmp_1003377f
		jnz jmp_100338c2
	jmp_1003381e:
		add edi, ecx
		add esi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100337c4 */
		_emit 0x9b
		_emit 0x75 /* jnz jmp_100337f5 */
		_emit 0xca
		_emit 0x72 /* jb jmp_100337af */
		_emit 0x82
	jmp_1003382d:
		jmp jmp_10033947
	jmp_10033832:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003385e */
		_emit 0x25
		jnz jmp_100338bf
		jae jmp_10033902
	jmp_10033845:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003385e */
		_emit 0x0a
		_emit 0x75 /* jnz jmp_100338bf */
		_emit 0x69
		_emit 0x72 /* jb jmp_10033845 */
		_emit 0xed
		jae jmp_10033902
	jmp_1003385e:
		movzx ecx, al
	jmp_10033861:
		cmp edi, dword ptr [ebp-0x2c]
		jg jmp_10033927
		mov eax, edi
		add eax, ecx
		dec eax
		sub eax, dword ptr [ebp-0x2c]
		cdq
		not edx
		and edx, eax
		sub ecx, edx
		xor eax, eax
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_unk0x1006a05c+eax]
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_100338b0 */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 0x8
		neg ecx
		and ecx, 0x3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 0x8
		mov ecx, ebx
		mov al, ah
		and ebx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, ebx
		pop ebx
	jmp_100338b0:
		rep stosb
		add edi, edx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_1003385e */
		_emit 0xa3
		_emit 0x73 /* jae jmp_10033902 */
		_emit 0x45
		_emit 0x74 /* jz jmp_10033845 */
		_emit 0x86
	jmp_100338bf:
		movzx ecx, al
	jmp_100338c2:
		cmp edi, dword ptr [ebp-0x2c]
		_emit 0x7f /* jg jmp_10033938 */
		_emit 0x71
		mov eax, edi
		add eax, ecx
		dec eax
		sub eax, dword ptr [ebp-0x2c]
		cdq
		not edx
		and edx, eax
		sub ecx, edx
		xor eax, eax
		or ecx, ecx
		_emit 0x74 /* jz jmp_100338eb */
		_emit 0x0f
	jmp_100338dc:
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_unk0x1006a05c+eax]
		mov byte ptr [edi], al
		inc edi
		dec ecx
		_emit 0x75 /* jnz jmp_100338dc */
		_emit 0xf1
	jmp_100338eb:
		add edi, edx
		add esi, edx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		ja jmp_1003385e
		_emit 0x75 /* jnz jmp_100338bf */
		_emit 0xc3
		jb jmp_10033845
	jmp_10033902:
		_emit 0xeb /* jmp jmp_10033947 */
		_emit 0x43
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033924 */
		_emit 0x19
		_emit 0x75 /* jnz jmp_10033935 */
		_emit 0x28
		_emit 0x73 /* jae jmp_10033947 */
		_emit 0x38
	jmp_1003390f:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033924 */
		_emit 0x06
		_emit 0x75 /* jnz jmp_10033935 */
		_emit 0x15
		_emit 0x72 /* jb jmp_1003390f */
		_emit 0xed
		_emit 0x73 /* jae jmp_10033947 */
		_emit 0x23
	jmp_10033924:
		movzx ecx, al
	jmp_10033927:
		add edi, ecx
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033924 */
		_emit 0xf3
		_emit 0x73 /* jae jmp_10033947 */
		_emit 0x14
		_emit 0x74 /* jz jmp_1003390f */
		_emit 0xda
	jmp_10033935:
		movzx ecx, al
	jmp_10033938:
		add edi, ecx
		add esi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10033924 */
		_emit 0xe1
		_emit 0x75 /* jnz jmp_10033935 */
		_emit 0xf0
		_emit 0x72 /* jb jmp_1003390f */
		_emit 0xc8
	jmp_10033947:
		mov eax, dword ptr [ebp-0x48]
		add dword ptr [ebp-0x24], eax
		add dword ptr [ebp-0x28], eax
		add dword ptr [ebp-0x2c], eax
		inc dword ptr [ebp-0x20]
	jmp_10033956:
		mov eax, dword ptr [ebp-0x20]
		cmp eax, dword ptr [ebp-0x18]
		jle jmp_100336ec
	jmp_10033962:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_1003396a:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10033975:
		mov eax, 0xfffffffc
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
MechS32 FUN_10033980(
	PixelView* p_view,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10
)
{
	STUB(0x10033980);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10033980
__declspec(naked) MechS32
FUN_10033980(PixelView* p_view, undefined4 p_unk0x04, undefined4 p_unk0x08, undefined4 p_unk0x0c, undefined4 p_unk0x10)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [esi+0x4]
		add dword ptr [ebp+0x10], eax
		mov eax, dword ptr [esi+0x8]
		add dword ptr [ebp+0x14], eax
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp+0x18], eax
		jle jmp_10033a6e
		mov esi, dword ptr [ebp+0xc]
		mov edi, dword ptr [ebx]
		mov eax, dword ptr [esi+0x8]
		add eax, dword ptr [ebp+0x10]
		add edi, eax
		mov eax, dword ptr [esi+0xc]
		mov ebx, eax
		add eax, dword ptr [ebp+0x14]
		mul dword ptr [ebp+0x18]
		add edi, eax
		mov edx, edi
		mov eax, dword ptr [esi+0x10]
		mov eax, dword ptr [esi+0x14]
		inc eax
		sub eax, ebx
		mov ebx, eax
		jle jmp_10033a6e
		add esi, 0x18
	jmp_100339d8:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100339f8 */
		_emit 0x19
		_emit 0x75 /* jnz jmp_10033a3f */
		_emit 0x5e
		_emit 0x73 /* jae jmp_10033a62 */
		_emit 0x7f
	jmp_100339e3:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100339f8 */
		_emit 0x06
		_emit 0x75 /* jnz jmp_10033a3f */
		_emit 0x4b
		_emit 0x72 /* jb jmp_100339e3 */
		_emit 0xed
		_emit 0x73 /* jae jmp_10033a62 */
		_emit 0x6a
	jmp_100339f8:
		movzx ecx, al
		xor eax, eax
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_unk0x1006a05c+eax]
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_10033a32 */
		_emit 0x27
		push ebx
		mov ah, al
		mov ebx, ecx
		mov ecx, edi
		rol eax, 0x8
		neg ecx
		and ecx, 0x3
		mov al, ah
		sub ebx, ecx
		rep stosb
		rol eax, 0x8
		mov ecx, ebx
		mov al, ah
		and ebx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, ebx
		pop ebx
	jmp_10033a32:
		rep stosb
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100339f8 */
		_emit 0xbd
		_emit 0x73 /* jae jmp_10033a62 */
		_emit 0x25
		_emit 0x74 /* jz jmp_100339e3 */
		_emit 0xa4
	jmp_10033a3f:
		movzx ecx, al
		xor eax, eax
		or ecx, ecx
		_emit 0x74 /* jz jmp_10033a57 */
		_emit 0x0f
	jmp_10033a48:
		mov al, byte ptr [esi]
		inc esi
		mov al, byte ptr [g_unk0x1006a05c+eax]
		mov byte ptr [edi], al
		inc edi
		dec ecx
		_emit 0x75 /* jnz jmp_10033a48 */
		_emit 0xf1
	jmp_10033a57:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100339f8 */
		_emit 0x9a
		_emit 0x75 /* jnz jmp_10033a3f */
		_emit 0xdf
		_emit 0x72 /* jb jmp_100339e3 */
		_emit 0x81
	jmp_10033a62:
		add edx, dword ptr [ebp+0x18]
		mov edi, edx
		dec ebx
		jnz jmp_100339d8
	jmp_10033a6e:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Blits with rotation (p_angle) and 16.16 scaling (p_scaleX, p_scaleY). The rotated corners
// go into g_unk0x1006a15c; with no rotation and unit scale, it takes a plain copy path.
#ifdef COMPAT_MODE
MechS32 FUN_10033a76(
	PixelView* p_view,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14,
	undefined4 p_unk0x18,
	MechS32 p_angle,
	MechS32 p_scaleX,
	MechS32 p_scaleY
)
{
	STUB(0x10033a76);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10033a76
__declspec(naked) MechS32 FUN_10033a76(
	PixelView* p_view,
	undefined4 p_unk0x04,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14,
	undefined4 p_unk0x18,
	MechS32 p_angle,
	MechS32 p_scaleX,
	MechS32 p_scaleY
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, 0xffffff10
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		cmp dword ptr [ebp+0x24], 0x10000
		_emit 0x75 /* jne jmp_10033aa2 */
		_emit 0x13
		cmp dword ptr [ebp+0x28], 0x10000
		_emit 0x75 /* jne jmp_10033aa2 */
		_emit 0x0a
		cmp dword ptr [ebp+0x20], 0x0
		je jmp_100345e0
jmp_10033aa2:
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		call FUN_10037549
		add esp, 0x8
		mov ecx, eax
		shr eax, 0x10
		dec eax
		mov dword ptr [ebp-0x44], eax
		and ecx, 0xffff
		dec ecx
		mov dword ptr [ebp-0x48], ecx
		lea ebx, [ebp-0x14]
		lea esi, [ebp-0x28]
		mov dword ptr [esi], ebx
		mov edx, dword ptr [ebp+0x1c]
		mov dword ptr [ebx], edx
		mov dword ptr [esi+0x4], 0x0
		mov dword ptr [g_unk0x1006a15c+0xc], 0x0
		mov dword ptr [g_unk0x1006a15c+0x48], 0x0
		mov dword ptr [esi+0x8], 0x0
		mov dword ptr [g_unk0x1006a15c+0x10], 0x0
		mov dword ptr [g_unk0x1006a15c+0x24], 0x0
		mov dword ptr [ebx+0x4], eax
		mov dword ptr [esi+0xc], eax
		mov dword ptr [g_unk0x1006a15c+0x20], eax
		mov dword ptr [g_unk0x1006a15c+0x34], eax
		mov dword ptr [ebx+0x8], ecx
		mov dword ptr [esi+0x10], ecx
		mov dword ptr [g_unk0x1006a15c+0x38], ecx
		mov dword ptr [g_unk0x1006a15c+0x4c], ecx
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		call FUN_1003757d
		add esp, 0x8
		mov ebx, eax
		cwde
		neg eax
		mov dword ptr [ebp-0x3c], eax
		sar ebx, 0x10
		neg ebx
		mov dword ptr [ebp-0x40], ebx
		test dword ptr [ebp+0x2c], 0x2
		_emit 0x75 /* jne jmp_10033b9b */
		_emit 0x4c
		push 0xff
		lea eax, [ebp-0x28]
		push eax
		call FUN_10034e15
		add esp, 0x8
		test dword ptr [ebp+0x2c], 0x1
		_emit 0x74 /* je jmp_10033b83 */
		_emit 0x1a
		push dword ptr [ebp-0x3c]
		push dword ptr [ebp-0x40]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		lea eax, [ebp-0x28]
		push eax
		call FUN_1003351a
		add esp, 0x14
		_emit 0xeb /* jmp jmp_10033b9b */
		_emit 0x18
jmp_10033b83:
		push dword ptr [ebp-0x3c]
		push dword ptr [ebp-0x40]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		lea eax, [ebp-0x28]
		push eax
		call FUN_10032f84
		add esp, 0x14
jmp_10033b9b:
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0xe8], eax
		jle jmp_10033c3c
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		jle jmp_10033c3c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0xec], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10033bcf */
		_emit 0x05
		mov eax, 0x0
jmp_10033bcf:
		mov dword ptr [ebp-0xd4], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0xf0], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10033be8 */
		_emit 0x05
		mov eax, 0x0
jmp_10033be8:
		mov dword ptr [ebp-0xd8], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0xe8]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10033bfe */
		_emit 0x02
		mov eax, edx
jmp_10033bfe:
		mov dword ptr [ebp-0xdc], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10033c10 */
		_emit 0x02
		mov eax, edx
jmp_10033c10:
		mov dword ptr [ebp-0xe0], eax
		mov eax, dword ptr [ebp-0xdc]
		cmp eax, dword ptr [ebp-0xd4]
		_emit 0x7c /* jl jmp_10033c47 */
		_emit 0x23
		mov eax, dword ptr [ebp-0xe0]
		cmp eax, dword ptr [ebp-0xd8]
		_emit 0x7c /* jl jmp_10033c47 */
		_emit 0x15
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0xe4], eax
		_emit 0xeb /* jmp jmp_10033c52 */
		_emit 0x16
jmp_10033c3c:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10033c47:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10033c52:
		mov eax, dword ptr [ebp+0x14]
		sub eax, dword ptr [ebp-0x40]
		mov dword ptr [ebp-0x4c], eax
		mov eax, dword ptr [ebp+0x18]
		sub eax, dword ptr [ebp-0x3c]
		mov dword ptr [ebp-0x50], eax
		mov dword ptr [ebp-0x30], 0x0
		mov dword ptr [ebp-0x2c], 0x0
		push dword ptr [ebp+0x28]
		push dword ptr [ebp+0x24]
		push dword ptr [ebp+0x20]
		lea eax, [ebp-0x40]
		push eax
		lea eax, [ebp-0x38]
		push eax
		lea eax, [ebp-0x30]
		push eax
		call FUN_100369e2
		add esp, 0x18
		mov eax, dword ptr [ebp-0x38]
		add eax, dword ptr [ebp-0x4c]
		mov dword ptr [g_unk0x1006a15c], eax
		mov eax, dword ptr [ebp-0x34]
		add eax, dword ptr [ebp-0x50]
		mov dword ptr [g_unk0x1006a15c+0x4], eax
		mov eax, dword ptr [ebp-0x44]
		mov dword ptr [ebp-0x30], eax
		mov dword ptr [ebp-0x2c], 0x0
		push dword ptr [ebp+0x28]
		push dword ptr [ebp+0x24]
		push dword ptr [ebp+0x20]
		lea eax, [ebp-0x40]
		push eax
		lea eax, [ebp-0x38]
		push eax
		lea eax, [ebp-0x30]
		push eax
		call FUN_100369e2
		add esp, 0x18
		mov eax, dword ptr [ebp-0x38]
		add eax, dword ptr [ebp-0x4c]
		mov dword ptr [g_unk0x1006a15c+0x14], eax
		mov eax, dword ptr [ebp-0x34]
		add eax, dword ptr [ebp-0x50]
		mov dword ptr [g_unk0x1006a15c+0x18], eax
		mov eax, dword ptr [ebp-0xec]
		add dword ptr [g_unk0x1006a15c], eax
		add dword ptr [g_unk0x1006a15c+0x14], eax
		mov eax, dword ptr [ebp-0xf0]
		add dword ptr [g_unk0x1006a15c+0x4], eax
		add dword ptr [g_unk0x1006a15c+0x18], eax
		mov eax, dword ptr [ebp-0x44]
		mov ebx, dword ptr [ebp-0x48]
		mov dword ptr [ebp-0x30], eax
		mov dword ptr [ebp-0x2c], ebx
		push dword ptr [ebp+0x28]
		push dword ptr [ebp+0x24]
		push dword ptr [ebp+0x20]
		lea eax, [ebp-0x40]
		push eax
		lea eax, [ebp-0x38]
		push eax
		lea eax, [ebp-0x30]
		push eax
		call FUN_100369e2
		add esp, 0x18
		mov eax, dword ptr [ebp-0x38]
		add eax, dword ptr [ebp-0x4c]
		mov dword ptr [g_unk0x1006a15c+0x28], eax
		mov eax, dword ptr [ebp-0x34]
		add eax, dword ptr [ebp-0x50]
		mov dword ptr [g_unk0x1006a15c+0x2c], eax
		mov eax, dword ptr [ebp-0x48]
		mov dword ptr [ebp-0x30], 0x0
		mov dword ptr [ebp-0x2c], eax
		push dword ptr [ebp+0x28]
		push dword ptr [ebp+0x24]
		push dword ptr [ebp+0x20]
		lea eax, [ebp-0x40]
		push eax
		lea eax, [ebp-0x38]
		push eax
		lea eax, [ebp-0x30]
		push eax
		call FUN_100369e2
		add esp, 0x18
		mov eax, dword ptr [ebp-0x38]
		add eax, dword ptr [ebp-0x4c]
		mov dword ptr [g_unk0x1006a15c+0x3c], eax
		mov eax, dword ptr [ebp-0x34]
		add eax, dword ptr [ebp-0x50]
		mov dword ptr [g_unk0x1006a15c+0x40], eax
		mov eax, dword ptr [ebp-0xec]
		add dword ptr [g_unk0x1006a15c+0x28], eax
		add dword ptr [g_unk0x1006a15c+0x3c], eax
		mov eax, dword ptr [ebp-0xf0]
		add dword ptr [g_unk0x1006a15c+0x2c], eax
		add dword ptr [g_unk0x1006a15c+0x40], eax
		lea ebx, [ebp-0x14]
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x58], eax
		mov ecx, dword ptr [ebx+0x4]
		inc ecx
		mov dword ptr [ebp-0x54], ecx
		push ds
		pop es
		mov ebx, offset g_unk0x1006a15c
		mov eax, ebx
		add eax, 0x50
		mov dword ptr [ebp-0x64], ebx
		mov dword ptr [ebp-0x68], eax
		mov esi, 0x7fff
		mov edi, 0xffff8000
		mov ecx, 0xf
jmp_10033ddc:
		mov edx, 0x0
		mov eax, dword ptr [ebx]
		sub eax, dword ptr [ebp-0xd4]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebp-0xdc]
		sub eax, dword ptr [ebx]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		sub eax, dword ptr [ebp-0xd8]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebp-0xe0]
		sub eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		cmp eax, esi
		_emit 0x7f /* jg jmp_10033e1f */
		_emit 0x05
		mov esi, eax
		mov dword ptr [ebp-0x6c], ebx
jmp_10033e1f:
		cmp eax, edi
		_emit 0x7c /* jl jmp_10033e25 */
		_emit 0x02
		mov edi, eax
jmp_10033e25:
		and ecx, edx
		add ebx, 0x14
		cmp ebx, dword ptr [ebp-0x68]
		_emit 0x75 /* jne jmp_10033ddc */
		_emit 0xad
		or ecx, ecx
		jne jmp_10034411
		mov eax, dword ptr [ebp-0x6c]
		mov dword ptr [ebp-0x78], eax
		mov dword ptr [ebp-0x7c], eax
		mov dword ptr [ebp-0x8c], esi
		cmp edi, esi
		je jmp_10034411
jmp_10033e4e:
		mov ebx, dword ptr [ebp-0x78]
		mov dword ptr [ebp-0x70], ebx
		mov esi, ebx
		sub esi, 0x14
		cmp esi, dword ptr [ebp-0x64]
		_emit 0x7d /* jge jmp_10033e64 */
		_emit 0x06
		mov esi, dword ptr [ebp-0x68]
		sub esi, 0x14
jmp_10033e64:
		mov dword ptr [ebp-0x78], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, dword ptr [ebp-0xd8]
		_emit 0x7d /* jge jmp_10033e7d */
		_emit 0x08
		cmp ecx, dword ptr [ebp-0xd8]
		_emit 0x7e /* jle jmp_10033e4e */
		_emit 0xd1
jmp_10033e7d:
		sub ecx, edx
		_emit 0x74 /* je jmp_10033e4e */
		_emit 0xcd
		mov dword ptr [ebp-0x80], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xa8], eax
		mov ecx, dword ptr [ebp-0x80]
		mov edx, dword ptr [esi+0xc]
		sub edx, dword ptr [ebx+0xc]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xb0], eax
		mov ecx, dword ptr [ebp-0x80]
		mov edx, dword ptr [esi+0x10]
		sub edx, dword ptr [ebx+0x10]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xb8], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [ebp-0x90], edx
		mov edx, dword ptr [ebx+0xc]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [ebp-0x98], edx
		mov edx, dword ptr [ebx+0x10]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [ebp-0xa0], edx
jmp_10033f14:
		mov ebx, dword ptr [ebp-0x7c]
		mov dword ptr [ebp-0x74], ebx
		mov esi, ebx
		add esi, 0x14
		cmp esi, dword ptr [ebp-0x68]
		_emit 0x7c /* jl jmp_10033f27 */
		_emit 0x03
		mov esi, dword ptr [ebp-0x64]
jmp_10033f27:
		mov dword ptr [ebp-0x7c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, dword ptr [ebp-0xd8]
		_emit 0x7d /* jge jmp_10033f40 */
		_emit 0x08
		cmp ecx, dword ptr [ebp-0xd8]
		_emit 0x7e /* jle jmp_10033f14 */
		_emit 0xd4
jmp_10033f40:
		sub ecx, edx
		_emit 0x74 /* je jmp_10033f14 */
		_emit 0xd0
		mov dword ptr [ebp-0x84], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xac], eax
		mov ecx, dword ptr [ebp-0x84]
		mov edx, dword ptr [esi+0xc]
		sub edx, dword ptr [ebx+0xc]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xb4], eax
		mov ecx, dword ptr [ebp-0x84]
		mov edx, dword ptr [esi+0x10]
		sub edx, dword ptr [ebx+0x10]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xbc], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [ebp-0x94], edx
		mov edx, dword ptr [ebx+0xc]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [ebp-0x9c], edx
		mov edx, dword ptr [ebx+0x10]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [ebp-0xa4], edx
		mov eax, dword ptr [ebp-0xe0]
		sub eax, dword ptr [ebp-0x8c]
		sub edi, dword ptr [ebp-0xe0]
		_emit 0x7f /* jg jmp_10033ff6 */
		_emit 0x02
		add eax, edi
jmp_10033ff6:
		mov dword ptr [ebp-0x88], eax
		mov eax, dword ptr [ebp-0xd8]
		sub eax, dword ptr [ebp-0x8c]
		jle jmp_100340ad
		sub dword ptr [ebp-0x88], eax
		mov ecx, dword ptr [ebp-0xd8]
		mov dword ptr [ebp-0x8c], ecx
		mov ebx, dword ptr [ebp-0x70]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [ebp-0x80], ecx
		shl ecx, 0x10
		mov eax, dword ptr [ebp-0xa8]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [ebp-0x90], eax
		mov eax, dword ptr [ebp-0xb0]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [ebp-0x98], eax
		mov eax, dword ptr [ebp-0xb8]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [ebp-0xa0], eax
		mov ecx, dword ptr [ebp-0xd8]
		mov ebx, dword ptr [ebp-0x74]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [ebp-0x84], ecx
		shl ecx, 0x10
		mov eax, dword ptr [ebp-0xac]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [ebp-0x94], eax
		mov eax, dword ptr [ebp-0xb4]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [ebp-0x9c], eax
		mov eax, dword ptr [ebp-0xbc]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [ebp-0xa4], eax
jmp_100340ad:
		mov eax, dword ptr [ebp-0x8c]
		imul dword ptr [ebp-0xe8]
		add eax, dword ptr [ebp-0xe4]
		add eax, 0x0
		mov dword ptr [ebp-0x5c], eax
		mov eax, dword ptr [ebp-0x90]
		mov ebx, dword ptr [ebp-0x94]
		mov ecx, dword ptr [ebp-0x98]
		mov edx, dword ptr [ebp-0x9c]
		mov esi, dword ptr [ebp-0xa0]
		mov edi, dword ptr [ebp-0xa4]
jmp_100340e9:
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		cmp ebx, eax
		_emit 0x7f /* jg jmp_100340f8 */
		_emit 0x05
		xchg ebx, eax
		_emit 0x87 /* xchg edx, ecx: the inline assembler encodes the operands the other way */
		_emit 0xca
		_emit 0x87 /* xchg edi, esi: the inline assembler encodes the operands the other way */
		_emit 0xf7
jmp_100340f8:
		sar eax, 0x10
		cmp eax, dword ptr [ebp-0xdc]
		jg jmp_100343ba
		sar ebx, 0x10
		cmp ebx, dword ptr [ebp-0xd4]
		jl jmp_100343ba
		mov dword ptr [ebp-0xc0], eax
		mov dword ptr [ebp-0xc4], ebx
		mov dword ptr [ebp-0xd0], ecx
		sub ebx, eax
		je jmp_100341fd
		push ebx
		sub edx, ecx
		xor eax, eax
		shrd eax, edx, 0x10
		shl ebx, 0x10
		sar edx, 0x10
		idiv ebx
		mov dword ptr [ebp-0xc8], eax
		shld edx, eax, 0x10
		pop ebx
		and eax, 0xffff
		and edx, 0xffff
		mov ecx, 0x1
		test edx, 0x8000
		_emit 0x74 /* je jmp_10034172 */
		_emit 0x0e
		or edx, 0xffff0000
		neg ecx
		cmp eax, 0x1
		sbb edx, -0x1
jmp_10034172:
		add ecx, edx
		push ecx
		push edx
		sub edi, esi
		mov edx, edi
		xor eax, eax
		shrd eax, edx, 0x10
		shl ebx, 0x10
		sar edx, 0x10
		idiv ebx
		mov dword ptr [ebp-0xcc], eax
		shld edx, eax, 0x10
		and eax, 0xffff
		and edx, 0xffff
		mov ecx, dword ptr [ebp-0x54]
		test edx, 0x8000
		_emit 0x74 /* je jmp_100341b0 */
		_emit 0x08
		neg ecx
		cmp eax, 0x1
		sbb edx, -0x1
jmp_100341b0:
		mov eax, dword ptr [ebp-0x54]
		imul dx
		cwde
		pop edx
		pop ebx
		add edx, eax
		mov dword ptr [g_unk0x1006a1ac], edx
		add edx, ecx
		mov dword ptr [g_unk0x1006a1ac+0x4], edx
		add ebx, eax
		mov dword ptr [g_unk0x1006a1ac+0x8], ebx
		add ebx, ecx
		mov dword ptr [g_unk0x1006a1ac+0xc], ebx
		mov ecx, dword ptr [ebp-0xd4]
		sub ecx, dword ptr [ebp-0xc0]
		jg jmp_10034440
jmp_100341eb:
		mov eax, dword ptr [ebp-0xc4]
		sub eax, dword ptr [ebp-0xdc]
		jg jmp_1003446e
jmp_100341fd:
		mov ecx, esi
		shr esi, 0x10
		mov eax, esi
		mul dword ptr [ebp-0x54]
		add eax, dword ptr [ebp-0x58]
		mov esi, dword ptr [ebp-0xd0]
		shr esi, 0x10
		add esi, eax
		mov eax, dword ptr [ebp-0xc0]
		mov edi, dword ptr [ebp-0x5c]
		add edi, eax
		mov ebx, dword ptr [ebp-0xc4]
		sub ebx, eax
		push ebp
		mov edx, dword ptr [ebp-0xd0]
		mov eax, dword ptr [ebp-0xc8]
		or eax, eax
		_emit 0x79 /* jns jmp_1003423d */
		_emit 0x04
		neg eax
		not edx
jmp_1003423d:
		shl eax, 0x10
		shl edx, 0x10
		mov ebp, dword ptr [ebp-0xcc]
		or ebp, ebp
		_emit 0x79 /* jns jmp_10034251 */
		_emit 0x04
		neg ebp
		not ecx
jmp_10034251:
		shl ebp, 0x10
		shl ecx, 0x10
		push ebx
		xor ebx, ebx
		cmp dword ptr [esp], 0x5
		jl jmp_1003431c
jmp_10034264:
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1003426d */
		_emit 0x02
		mov byte ptr [edi], bl
jmp_1003426d:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_10034288 */
		_emit 0x03
		mov byte ptr [edi+0x1], bl
jmp_10034288:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_100342a3 */
		_emit 0x03
		mov byte ptr [edi+0x2], bl
jmp_100342a3:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_100342be */
		_emit 0x03
		mov byte ptr [edi+0x3], bl
jmp_100342be:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_100342d9 */
		_emit 0x03
		mov byte ptr [edi+0x4], bl
jmp_100342d9:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_100342f4 */
		_emit 0x03
		mov byte ptr [edi+0x5], bl
jmp_100342f4:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		add edi, 0x6
		sub dword ptr [esp], 0x6
		js jmp_100343b6
		cmp dword ptr [esp], 0x5
		jge jmp_10034264
jmp_1003431c:
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_10034325 */
		_emit 0x02
		mov byte ptr [edi], bl
jmp_10034325:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_100343b6 */
		_emit 0x7b
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_10034345 */
		_emit 0x03
		mov byte ptr [edi+0x1], bl
jmp_10034345:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_100343b6 */
		_emit 0x5b
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_10034365 */
		_emit 0x03
		mov byte ptr [edi+0x2], bl
jmp_10034365:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_100343b6 */
		_emit 0x3b
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_10034385 */
		_emit 0x03
		mov byte ptr [edi+0x3], bl
jmp_10034385:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_100343b6 */
		_emit 0x1b
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_100343a5 */
		_emit 0x03
		mov byte ptr [edi+0x4], bl
jmp_100343a5:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x1006a1ac+ebx*0x4]
jmp_100343b6:
		add esp, 0x4
		pop ebp
jmp_100343ba:
		mov edi, dword ptr [ebp-0xe8]
		add dword ptr [ebp-0x5c], edi
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		dec dword ptr [ebp-0x88]
		_emit 0x78 /* js jmp_10034411 */
		_emit 0x40
		_emit 0x74 /* je jmp_10034417 */
		_emit 0x44
		dec dword ptr [ebp-0x80]
		je jmp_10034479
		add eax, dword ptr [ebp-0xa8]
		add ecx, dword ptr [ebp-0xb0]
		add esi, dword ptr [ebp-0xb8]
jmp_100343ee:
		dec dword ptr [ebp-0x84]
		je jmp_10034529
		add ebx, dword ptr [ebp-0xac]
		add edx, dword ptr [ebp-0xb4]
		add edi, dword ptr [ebp-0xbc]
		jmp jmp_100340e9
jmp_10034411:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10034417:
		add eax, dword ptr [ebp-0xa8]
		add ecx, dword ptr [ebp-0xb0]
		add esi, dword ptr [ebp-0xb8]
		add ebx, dword ptr [ebp-0xac]
		add edx, dword ptr [ebp-0xb4]
		add edi, dword ptr [ebp-0xbc]
		jmp jmp_100340e9
jmp_10034440:
		add dword ptr [ebp-0xc0], ecx
		shl ecx, 0x10
		mov eax, dword ptr [ebp-0xc8]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [ebp-0xd0], eax
		mov eax, dword ptr [ebp-0xcc]
		imul ecx
		shrd eax, edx, 0x10
		add esi, eax
		jmp jmp_100341eb
jmp_1003446e:
		sub dword ptr [ebp-0xc4], eax
		jmp jmp_100341fd
jmp_10034479:
		push ebx
		push edx
		mov ebx, dword ptr [ebp-0x78]
		mov dword ptr [ebp-0x70], ebx
		mov esi, ebx
		sub esi, 0x14
		cmp esi, dword ptr [ebp-0x64]
		_emit 0x7d /* jge jmp_10034491 */
		_emit 0x06
		mov esi, dword ptr [ebp-0x68]
		sub esi, 0x14
jmp_10034491:
		mov dword ptr [ebp-0x78], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [ebp-0x80], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xa8], eax
		mov ecx, dword ptr [ebp-0x80]
		mov edx, dword ptr [esi+0xc]
		sub edx, dword ptr [ebx+0xc]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xb0], eax
		mov ecx, dword ptr [ebp-0x80]
		mov edx, dword ptr [esi+0x10]
		sub edx, dword ptr [ebx+0x10]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xb8], eax
		mov eax, dword ptr [ebx]
		shl eax, 0x10
		add eax, 0x8000
		mov ecx, dword ptr [ebx+0xc]
		shl ecx, 0x10
		add ecx, 0x8000
		mov esi, dword ptr [ebx+0x10]
		shl esi, 0x10
		add esi, 0x8000
		pop edx
		pop ebx
		jmp jmp_100343ee
jmp_10034529:
		push eax
		push ecx
		mov ebx, dword ptr [ebp-0x7c]
		mov dword ptr [ebp-0x74], ebx
		mov edi, ebx
		add edi, 0x14
		cmp edi, dword ptr [ebp-0x68]
		_emit 0x7c /* jl jmp_1003453e */
		_emit 0x03
		mov edi, dword ptr [ebp-0x64]
jmp_1003453e:
		mov dword ptr [ebp-0x7c], edi
		mov ecx, dword ptr [edi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [ebp-0x84], ecx
		mov edx, dword ptr [edi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xac], eax
		mov ecx, dword ptr [ebp-0x84]
		mov edx, dword ptr [edi+0xc]
		sub edx, dword ptr [ebx+0xc]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xb4], eax
		mov ecx, dword ptr [ebp-0x84]
		mov edx, dword ptr [edi+0x10]
		sub edx, dword ptr [ebx+0x10]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [ebp-0xbc], eax
		mov edx, dword ptr [ebx+0xc]
		shl edx, 0x10
		add edx, 0x8000
		mov edi, dword ptr [ebx+0x10]
		shl edi, 0x10
		add edi, 0x8000
		mov ebx, dword ptr [ebx]
		shl ebx, 0x10
		add ebx, 0x8000
		pop ecx
		pop eax
		jmp jmp_100340e9
jmp_100345e0:
		test dword ptr [ebp+0x2c], 0x1
		_emit 0x74 /* je jmp_10034605 */
		_emit 0x1c
		push dword ptr [ebp+0x18]
		push dword ptr [ebp+0x14]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push dword ptr [ebp+0x8]
		call FUN_1003351a
		add esp, 0x14
		jmp jmp_10034411
jmp_10034605:
		push dword ptr [ebp+0x18]
		push dword ptr [ebp+0x14]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push dword ptr [ebp+0x8]
		call FUN_10032f84
		add esp, 0x14
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
MechS32 FUN_10034622(
	void* p_data,
	MechS32 p_index,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14
)
{
	STUB(0x10034622);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10034622
__declspec(naked) MechS32 FUN_10034622(
	void* p_data,
	MechS32 p_index,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	undefined4 p_unk0x14
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x24
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov dword ptr [ebp-0x18], 0x0
		mov dword ptr [ebp-0x1c], 0x0
		mov dword ptr [ebp-0x20], 0x0
		mov dword ptr [ebp-0x24], 0x0
		mov esi, dword ptr [ebp+0xc]
		shl esi, 0x3
		add esi, 0x8
		add esi, dword ptr [ebp+0x8]
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp+0x8]
		mov dword ptr [ebp-0x4], esi
		mov esi, dword ptr [ebp-0x4]
		mov eax, dword ptr [esi+0x8]
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x8], eax
		mov eax, dword ptr [esi+0xc]
		mov ebx, eax
		add eax, dword ptr [ebp+0x14]
		mov dword ptr [ebp-0xc], eax
		mov eax, dword ptr [esi+0x10]
		add eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x10], eax
		mov eax, dword ptr [esi+0x14]
		mov ecx, eax
		add ecx, dword ptr [ebp+0x14]
		mov dword ptr [ebp-0x14], ecx
		inc eax
		sub eax, ebx
		mov ebx, eax
		jle jmp_10034747
		add esi, 0x18
		mov dword ptr [ebp-0x18], 0x7fffffff
		mov dword ptr [ebp-0x1c], 0x7fffffff
		mov dword ptr [ebp-0x20], 0x80000000
		mov dword ptr [ebp-0x24], 0x80000000
		mov edx, dword ptr [ebp-0xc]
	jmp_100346b7:
		mov edi, dword ptr [ebp-0x8]
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100346da */
		_emit 0x19
		_emit 0x75 /* jnz jmp_1003470d */
		_emit 0x4a
		_emit 0x73 /* jae jmp_1003473f */
		_emit 0x7a
	jmp_100346c5:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		add edi, ecx
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100346da */
		_emit 0x06
		_emit 0x75 /* jnz jmp_1003470d */
		_emit 0x37
		_emit 0x72 /* jb jmp_100346c5 */
		_emit 0xed
		_emit 0x73 /* jae jmp_1003473f */
		_emit 0x65
	jmp_100346da:
		movzx ecx, al
		mov al, byte ptr [esi]
		inc esi
		cmp dword ptr [ebp-0x18], edi
		_emit 0x7c /* jl jmp_100346e8 */
		_emit 0x03
		mov dword ptr [ebp-0x18], edi
	jmp_100346e8:
		add edi, ecx
		cmp dword ptr [ebp-0x20], edi
		_emit 0x7f /* jg jmp_100346f2 */
		_emit 0x03
		mov dword ptr [ebp-0x20], edi
	jmp_100346f2:
		cmp dword ptr [ebp-0x1c], edx
		_emit 0x7c /* jl jmp_100346fa */
		_emit 0x03
		mov dword ptr [ebp-0x1c], edx
	jmp_100346fa:
		cmp dword ptr [ebp-0x24], edx
		_emit 0x7f /* jg jmp_10034702 */
		_emit 0x03
		mov dword ptr [ebp-0x24], edx
	jmp_10034702:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100346da */
		_emit 0xd1
		_emit 0x73 /* jae jmp_1003473f */
		_emit 0x34
		_emit 0x74 /* jz jmp_100346c5 */
		_emit 0xb8
	jmp_1003470d:
		movzx ecx, al
		add esi, ecx
		cmp dword ptr [ebp-0x18], edi
		_emit 0x7c /* jl jmp_1003471a */
		_emit 0x03
		mov dword ptr [ebp-0x18], edi
	jmp_1003471a:
		add edi, ecx
		cmp dword ptr [ebp-0x20], edi
		_emit 0x7f /* jg jmp_10034724 */
		_emit 0x03
		mov dword ptr [ebp-0x20], edi
	jmp_10034724:
		cmp dword ptr [ebp-0x1c], edx
		_emit 0x7c /* jl jmp_1003472c */
		_emit 0x03
		mov dword ptr [ebp-0x1c], edx
	jmp_1003472c:
		cmp dword ptr [ebp-0x24], edx
		_emit 0x7f /* jg jmp_10034734 */
		_emit 0x03
		mov dword ptr [ebp-0x24], edx
	jmp_10034734:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_100346da */
		_emit 0x9f
		_emit 0x75 /* jnz jmp_1003470d */
		_emit 0xd0
		_emit 0x72 /* jb jmp_100346c5 */
		_emit 0x86
	jmp_1003473f:
		inc edx
		dec ebx
		jnz jmp_100346b7
	jmp_10034747:
		mov eax, dword ptr [ebp+0x18]
		and eax, 0x1
		_emit 0x74 /* jz jmp_10034763 */
		_emit 0x14
		mov eax, dword ptr [ebp+0x10]
		add eax, dword ptr [ebp+0x10]
		mov ebx, eax
		sub eax, dword ptr [ebp-0x18]
		sub ebx, dword ptr [ebp-0x20]
		mov dword ptr [ebp-0x20], eax
		mov dword ptr [ebp-0x18], ebx
	jmp_10034763:
		mov eax, dword ptr [ebp+0x18]
		and eax, 0x2
		_emit 0x74 /* jz jmp_1003477f */
		_emit 0x14
		mov eax, dword ptr [ebp+0x14]
		add eax, dword ptr [ebp+0x14]
		mov ebx, eax
		sub eax, dword ptr [ebp-0x1c]
		sub ebx, dword ptr [ebp-0x24]
		mov dword ptr [ebp-0x24], eax
		mov dword ptr [ebp-0x1c], ebx
	jmp_1003477f:
		mov edi, dword ptr [ebp+0x1c]
		mov eax, dword ptr [ebp-0x18]
		stosd
		mov eax, dword ptr [ebp-0x1c]
		stosd
		mov eax, dword ptr [ebp-0x20]
		stosd
		mov eax, dword ptr [ebp-0x24]
		stosd
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Run-length encodes the view, skipping pixels equal to p_transparent: finds the bounding box of
// the opaque pixels, writes a 0x18-byte header to p_out (when not NULL) followed by the rows
// (FUN_10034aaf) and returns the encoded size.
#ifdef COMPAT_MODE
MechS32 FUN_1003479a(PixelView* p_view, MechU8 p_transparent, MechS32 p_x, MechS32 p_y, undefined4* p_out)
{
	STUB(0x1003479a);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x1003479a
__declspec(naked) MechS32
FUN_1003479a(PixelView* p_view, MechU8 p_transparent, MechS32 p_x, MechS32 p_y, undefined4* p_out)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x38
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x30], eax
		_emit 0x7e /* jle jmp_10034819 */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10034819 */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x34], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_100347cd */
		_emit 0x05
		mov eax, 0x0
jmp_100347cd:
		mov dword ptr [ebp-0x1c], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x38], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_100347e0 */
		_emit 0x05
		mov eax, 0x0
jmp_100347e0:
		mov dword ptr [ebp-0x20], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x30]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100347f0 */
		_emit 0x02
		mov eax, edx
jmp_100347f0:
		mov dword ptr [ebp-0x24], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100347ff */
		_emit 0x02
		mov eax, edx
jmp_100347ff:
		mov dword ptr [ebp-0x28], eax
		mov eax, dword ptr [ebp-0x24]
		cmp eax, dword ptr [ebp-0x1c]
		_emit 0x7c /* jl jmp_10034824 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x28]
		cmp eax, dword ptr [ebp-0x20]
		_emit 0x7c /* jl jmp_10034824 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x2c], eax
		_emit 0xeb /* jmp jmp_1003482f */
		_emit 0x16
jmp_10034819:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10034824:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1003482f:
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov edi, dword ptr [ebp+0x18]
		mov dword ptr [g_unk0x1006880d], edi
		or edi, edi
		_emit 0x74 /* je jmp_1003487d */
		_emit 0x3c
		mov eax, dword ptr [esi+0xc]
		sub eax, dword ptr [esi+0x4]
		shl eax, 0x10
		mov ax, word ptr [esi+0x10]
		sub ax, word ptr [esi+0x8]
		mov dword ptr [edi], eax
		add edi, 0x4
		mov eax, dword ptr [ebp+0x10]
		shl eax, 0x10
		mov ax, word ptr [ebp+0x14]
		mov dword ptr [edi], eax
		add edi, 0x4
		xor eax, eax
		mov dword ptr [edi], eax
		add edi, 0x4
		mov dword ptr [edi], eax
		add edi, 0x4
		dec eax
		mov dword ptr [edi], eax
		add edi, 0x4
		mov dword ptr [edi], eax
		add edi, 0x4
jmp_1003487d:
		mov eax, dword ptr [ebp-0x34]
		add dword ptr [ebp+0x10], eax
		mov eax, dword ptr [ebp-0x38]
		add dword ptr [ebp+0x14], eax
		mov eax, dword ptr [ebp-0x24]
		inc eax
		sub eax, dword ptr [ebp-0x1c]
		mov dword ptr [ebp-0x8], eax
		mov eax, dword ptr [ebp-0x24]
		inc eax
		sub eax, dword ptr [ebp-0x1c]
		mov dword ptr [ebp-0xc], eax
		mov eax, 0x7fffffff
		mov dword ptr [g_unk0x10068835], eax
		mov dword ptr [g_unk0x10068839], eax
		neg eax
		mov dword ptr [g_unk0x1006883d], eax
		mov dword ptr [g_unk0x10068841], eax
		mov eax, dword ptr [ebp-0x20]
		imul dword ptr [ebp-0x30]
		add eax, dword ptr [ebp-0x2c]
		add eax, dword ptr [ebp-0x1c]
		mov dword ptr [g_unk0x10068815], eax
		mov esi, eax
		mov eax, dword ptr [ebp-0x20]
		mov dword ptr [ebp-0x10], eax
		jmp jmp_1003496d
jmp_100348d6:
		mov edi, dword ptr [g_unk0x10068815]
		mov dword ptr [g_unk0x10068819], edi
		mov ecx, dword ptr [ebp-0x8]
		mov dword ptr [ebp-0x14], ecx
		mov al, byte ptr [ebp+0xc]
		repe scasb
		_emit 0x74 /* je jmp_10034961 */
		_emit 0x72
		mov eax, dword ptr [g_unk0x10068839]
		cmp eax, dword ptr [ebp-0x10]
		_emit 0x7c /* jl jmp_100348fc */
		_emit 0x03
		mov eax, dword ptr [ebp-0x10]
jmp_100348fc:
		mov dword ptr [g_unk0x10068839], eax
		mov eax, dword ptr [g_unk0x10068841]
		cmp eax, dword ptr [ebp-0x10]
		_emit 0x7f /* jg jmp_1003490e */
		_emit 0x03
		mov eax, dword ptr [ebp-0x10]
jmp_1003490e:
		mov dword ptr [g_unk0x10068841], eax
		mov eax, dword ptr [ebp-0x24]
		sub eax, ecx
		cmp eax, dword ptr [g_unk0x10068835]
		_emit 0x7c /* jl jmp_10034925 */
		_emit 0x05
		mov eax, dword ptr [g_unk0x10068835]
jmp_10034925:
		mov dword ptr [g_unk0x10068835], eax
		mov eax, dword ptr [g_unk0x10068815]
		add eax, dword ptr [ebp-0x8]
		dec eax
		mov edi, eax
		mov dword ptr [g_unk0x10068819], edi
		mov ecx, dword ptr [ebp-0x8]
		mov dword ptr [ebp-0x14], ecx
		mov al, byte ptr [ebp+0xc]
		std
		repe scasb
		cld
		_emit 0x74 /* je jmp_10034961 */
		_emit 0x17
		mov eax, dword ptr [ebp-0x1c]
		add eax, ecx
		cmp eax, dword ptr [g_unk0x1006883d]
		_emit 0x7f /* jg jmp_1003495c */
		_emit 0x05
		mov eax, dword ptr [g_unk0x1006883d]
jmp_1003495c:
		mov dword ptr [g_unk0x1006883d], eax
jmp_10034961:
		mov eax, dword ptr [ebp-0x30]
		add dword ptr [g_unk0x10068815], eax
		inc dword ptr [ebp-0x10]
jmp_1003496d:
		mov eax, dword ptr [ebp-0x10]
		cmp eax, dword ptr [ebp-0x28]
		jle jmp_100348d6
		mov edi, dword ptr [ebp+0x18]
		or edi, edi
		_emit 0x74 /* je jmp_100349ac */
		_emit 0x2c
		mov eax, dword ptr [g_unk0x10068835]
		sub eax, dword ptr [ebp+0x10]
		mov dword ptr [edi+0x8], eax
		mov eax, dword ptr [g_unk0x10068839]
		sub eax, dword ptr [ebp+0x14]
		mov dword ptr [edi+0xc], eax
		mov eax, dword ptr [g_unk0x1006883d]
		sub eax, dword ptr [ebp+0x10]
		mov dword ptr [edi+0x10], eax
		mov eax, dword ptr [g_unk0x10068841]
		sub eax, dword ptr [ebp+0x14]
		mov dword ptr [edi+0x14], eax
jmp_100349ac:
		add edi, 0x18
		mov dword ptr [g_unk0x1006881d], edi
		mov eax, dword ptr [g_unk0x1006883d]
		inc eax
		sub eax, dword ptr [g_unk0x10068835]
		mov dword ptr [ebp-0x18], eax
		mov eax, dword ptr [g_unk0x10068839]
		imul dword ptr [ebp-0x30]
		add eax, dword ptr [ebp-0x2c]
		add eax, dword ptr [g_unk0x10068835]
		mov esi, eax
		mov dword ptr [g_unk0x10068815], esi
		mov eax, dword ptr [g_unk0x10068839]
		mov dword ptr [ebp-0x10], eax
		_emit 0xeb /* jmp jmp_10034a04 */
		_emit 0x1d
jmp_100349e7:
		push dword ptr [ebp-0x1c]
		push dword ptr [ebp+0xc]
		push dword ptr [ebp-0x18]
		call FUN_10034aaf
		add esp, 0xc
		mov eax, dword ptr [ebp-0x30]
		add dword ptr [g_unk0x10068815], eax
		inc dword ptr [ebp-0x10]
jmp_10034a04:
		mov eax, dword ptr [ebp-0x10]
		cmp eax, dword ptr [g_unk0x10068841]
		_emit 0x7e /* jle jmp_100349e7 */
		_emit 0xd8
		mov eax, dword ptr [g_unk0x1006881d]
		sub eax, dword ptr [ebp+0x18]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Maps the pixels of an entry of the data's offset table through the color remap table, in
// place, following its run-length codes.
#ifdef COMPAT_MODE
MechS32 FUN_10034a1d(void* p_data, MechS32 p_index)
{
	STUB(0x10034a1d);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10034a1d
__declspec(naked) MechS32 FUN_10034a1d(void* p_data, MechS32 p_index)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0xc]
		shl esi, 0x3
		add esi, 0x8
		add esi, dword ptr [ebp+0x8]
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi+0xc]
		mov eax, dword ptr [esi+0x14]
		inc eax
		sub eax, ebx
		mov ebx, eax
		_emit 0x7e /* jle jmp_10034aa7 */
		_emit 0x62
		add esi, 0x18
	jmp_10034a48:
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10034a66 */
		_emit 0x17
		_emit 0x75 /* jnz jmp_10034a84 */
		_emit 0x33
		_emit 0x73 /* jae jmp_10034aa4 */
		_emit 0x51
	jmp_10034a53:
		mov al, byte ptr [esi]
		inc esi
		movzx ecx, al
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10034a66 */
		_emit 0x06
		_emit 0x75 /* jnz jmp_10034a84 */
		_emit 0x22
		_emit 0x72 /* jb jmp_10034a53 */
		_emit 0xef
		_emit 0x73 /* jae jmp_10034aa4 */
		_emit 0x3e
	jmp_10034a66:
		movzx ecx, al
		mov eax, 0x0
		mov al, byte ptr [esi]
		mov al, byte ptr [g_unk0x1006a05c+eax]
		mov byte ptr [esi], al
		inc esi
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10034a66 */
		_emit 0xe6
		_emit 0x73 /* jae jmp_10034aa4 */
		_emit 0x22
		_emit 0x74 /* jz jmp_10034a53 */
		_emit 0xcf
	jmp_10034a84:
		movzx ecx, al
		mov eax, 0x0
	jmp_10034a8c:
		mov al, byte ptr [esi]
		mov al, byte ptr [g_unk0x1006a05c+eax]
		mov byte ptr [esi], al
		inc esi
		loop jmp_10034a8c
		mov al, byte ptr [esi]
		inc esi
		shr al, 0x1
		_emit 0x77 /* ja jmp_10034a66 */
		_emit 0xc6
		_emit 0x75 /* jnz jmp_10034a84 */
		_emit 0xe2
		_emit 0x72 /* jb jmp_10034a53 */
		_emit 0xaf
	jmp_10034aa4:
		dec ebx
		_emit 0x75 /* jnz jmp_10034a48 */
		_emit 0xa1
	jmp_10034aa7:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Encodes one row of p_count pixels at g_unk0x10068815 for FUN_1003479a.
#ifdef COMPAT_MODE
void FUN_10034aaf(MechS32 p_count, MechU8 p_transparent, MechS32 p_left)
{
	STUB(0x10034aaf);
}
#else
// FUNCTION: MW2SHELL 0x10034aaf
__declspec(naked) void FUN_10034aaf(MechS32 p_count, MechU8 p_transparent, MechS32 p_left)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x4
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [g_unk0x10068815]
		mov dword ptr [g_unk0x10068819], esi
		push dword ptr [ebp+0x10]
		push 0x0
		push 0x0
		call FUN_10034c38
		add esp, 0xc
		mov dword ptr [ebp-0x4], 0x5
		mov ecx, dword ptr [ebp+0x8]
		or ecx, ecx
		je jmp_10034c0d
		mov al, byte ptr [esi]
		inc esi
		dec ecx
		mov ah, al
		cmp ah, byte ptr [ebp+0xc]
		je jmp_10034bde
jmp_10034af8:
		mov dword ptr [ebp-0x4], 0x1
		or ecx, ecx
		je jmp_10034c0d
		mov al, byte ptr [esi]
		inc esi
		dec ecx
		xor al, ah
		xor ah, al
		or al, al
		je jmp_10034bab
		cmp ah, byte ptr [ebp+0xc]
		_emit 0x75 /* jne jmp_10034b36 */
		_emit 0x1a
		mov dword ptr [g_unk0x10068819], esi
		push dword ptr [ebp+0x10]
		push 0x1
		push 0x1
		call FUN_10034c38
		add esp, 0xc
		jmp jmp_10034bde
jmp_10034b36:
		or ecx, ecx
		je jmp_10034c0d
		mov al, byte ptr [esi]
		inc esi
		dec ecx
		xor al, ah
		xor ah, al
		cmp ah, byte ptr [ebp+0xc]
		_emit 0x75 /* jne jmp_10034b62 */
		_emit 0x17
		mov dword ptr [g_unk0x10068819], esi
		push dword ptr [ebp+0x10]
		push 0x1
		push 0x1
		call FUN_10034c38
		add esp, 0xc
		_emit 0xeb /* jmp jmp_10034bde */
		_emit 0x7c
jmp_10034b62:
		or al, al
		_emit 0x75 /* jne jmp_10034b36 */
		_emit 0xd0
		or ecx, ecx
		je jmp_10034c0d
		mov al, byte ptr [esi]
		inc esi
		dec ecx
		xor al, ah
		xor ah, al
		cmp ah, byte ptr [ebp+0xc]
		_emit 0x75 /* jne jmp_10034b92 */
		_emit 0x17
		mov dword ptr [g_unk0x10068819], esi
		push dword ptr [ebp+0x10]
		push 0x1
		push 0x1
		call FUN_10034c38
		add esp, 0xc
		_emit 0xeb /* jmp jmp_10034bde */
		_emit 0x4c
jmp_10034b92:
		or al, al
		_emit 0x75 /* jne jmp_10034b36 */
		_emit 0xa0
		mov dword ptr [g_unk0x10068819], esi
		push dword ptr [ebp+0x10]
		push 0x3
		push 0x1
		call FUN_10034c38
		add esp, 0xc
jmp_10034bab:
		mov dword ptr [ebp-0x4], 0x2
		or ecx, ecx
		_emit 0x74 /* je jmp_10034c0d */
		_emit 0x57
		mov al, byte ptr [esi]
		inc esi
		dec ecx
		xor al, ah
		_emit 0x74 /* je jmp_10034bab */
		_emit 0xed
		xor ah, al
		mov dword ptr [g_unk0x10068819], esi
		push dword ptr [ebp+0x10]
		push 0x1
		push 0x2
		call FUN_10034c38
		add esp, 0xc
		cmp ah, byte ptr [ebp+0xc]
		jne jmp_10034af8
jmp_10034bde:
		mov dword ptr [ebp-0x4], 0x3
		or ecx, ecx
		_emit 0x74 /* je jmp_10034c0d */
		_emit 0x24
		mov al, byte ptr [esi]
		inc esi
		dec ecx
		xor al, ah
		_emit 0x74 /* je jmp_10034bde */
		_emit 0xed
		xor ah, al
		mov dword ptr [g_unk0x10068819], esi
		push dword ptr [ebp+0x10]
		push 0x1
		push 0x3
		call FUN_10034c38
		add esp, 0xc
		jmp jmp_10034af8
jmp_10034c0d:
		mov dword ptr [g_unk0x10068819], esi
		push dword ptr [ebp+0x10]
		push 0x0
		push dword ptr [ebp-0x4]
		call FUN_10034c38
		add esp, 0xc
		push dword ptr [ebp+0x10]
		push 0x0
		push 0x4
		call FUN_10034c38
		add esp, 0xc
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Emits one run of FUN_10034aaf's row encoding; p_op selects the kind (0 starts a row, 1 a
// literal run, 2 a repeated run, 3 a skip, 4 ends the row).
#ifdef COMPAT_MODE
void FUN_10034c38(MechS32 p_op, MechS32 p_back, MechS32 p_left)
{
	STUB(0x10034c38);
}
#else
// FUNCTION: MW2SHELL 0x10034c38
__declspec(naked) void FUN_10034c38(MechS32 p_op, MechS32 p_back, MechS32 p_left)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		push eax
		push ecx
		mov esi, dword ptr [g_unk0x10068821]
		mov edi, dword ptr [g_unk0x1006881d]
		mov eax, dword ptr [ebp+0x8]
		cmp eax, 0x2
		_emit 0x74 /* je jmp_10034c94 */
		_emit 0x3c
		cmp eax, 0x1
		je jmp_10034d3b
		cmp eax, 0x3
		je jmp_10034de0
		cmp eax, 0x4
		je jmp_10034df3
		cmp eax, 0x0
		jne jmp_10034e01
		xor eax, eax
		mov dword ptr [g_unk0x10068811], eax
		mov esi, dword ptr [g_unk0x10068819]
		mov dword ptr [g_unk0x10068821], esi
		jmp jmp_10034e01
jmp_10034c94:
		mov ebx, dword ptr [g_unk0x10068811]
		or ebx, ebx
		_emit 0x74 /* je jmp_10034cd3 */
		_emit 0x35
jmp_10034c9e:
		mov ecx, ebx
		cmp ecx, 0xff
		_emit 0x7c /* jl jmp_10034cad */
		_emit 0x05
		mov ecx, 0xff
jmp_10034cad:
		sub ebx, ecx
		cmp dword ptr [g_unk0x1006880d], 0x0
		_emit 0x74 /* je jmp_10034cc4 */
		_emit 0x0c
		mov al, 0x1
		mov byte ptr [edi], al
		inc edi
		mov al, cl
		mov byte ptr [edi], al
		inc edi
		_emit 0xeb /* jmp jmp_10034cc7 */
		_emit 0x03
jmp_10034cc4:
		add edi, 0x2
jmp_10034cc7:
		add esi, ecx
		or ebx, ebx
		_emit 0x75 /* jne jmp_10034c9e */
		_emit 0xd1
		mov dword ptr [g_unk0x10068811], ebx
jmp_10034cd3:
		mov ebx, dword ptr [g_unk0x10068819]
		sub ebx, esi
		sub ebx, dword ptr [ebp+0xc]
		mov eax, dword ptr [ebp+0x10]
		add eax, esi
		sub eax, dword ptr [g_unk0x10068815]
		cmp eax, dword ptr [g_unk0x10068835]
		_emit 0x7d /* jge jmp_10034cf6 */
		_emit 0x05
		mov dword ptr [g_unk0x10068835], eax
jmp_10034cf6:
		add eax, ebx
		dec eax
		cmp eax, dword ptr [g_unk0x1006883d]
		_emit 0x7e /* jle jmp_10034d08 */
		_emit 0x07
		mov dword ptr [g_unk0x1006883d], eax
		_emit 0xeb /* jmp jmp_10034d32 */
		_emit 0x2a
jmp_10034d08:
		mov ecx, ebx
		cmp ecx, 0x7f
		_emit 0x7c /* jl jmp_10034d14 */
		_emit 0x05
		mov ecx, 0x7f
jmp_10034d14:
		cmp dword ptr [g_unk0x1006880d], 0x0
		_emit 0x74 /* je jmp_10034d2b */
		_emit 0x0e
		mov al, cl
		add al, al
		mov byte ptr [edi], al
		inc edi
		mov al, byte ptr [esi]
		mov byte ptr [edi], al
		inc edi
		_emit 0xeb /* jmp jmp_10034d2e */
		_emit 0x03
jmp_10034d2b:
		add edi, 0x2
jmp_10034d2e:
		add esi, ecx
		sub ebx, ecx
jmp_10034d32:
		or ebx, ebx
		_emit 0x75 /* jne jmp_10034d08 */
		_emit 0xd2
		jmp jmp_10034e01
jmp_10034d3b:
		mov ebx, dword ptr [g_unk0x10068811]
		or ebx, ebx
		_emit 0x74 /* je jmp_10034d7a */
		_emit 0x35
jmp_10034d45:
		mov ecx, ebx
		cmp ecx, 0xff
		_emit 0x7c /* jl jmp_10034d54 */
		_emit 0x05
		mov ecx, 0xff
jmp_10034d54:
		sub ebx, ecx
		cmp dword ptr [g_unk0x1006880d], 0x0
		_emit 0x74 /* je jmp_10034d6b */
		_emit 0x0c
		mov al, 0x1
		mov byte ptr [edi], al
		inc edi
		mov al, cl
		mov byte ptr [edi], al
		inc edi
		_emit 0xeb /* jmp jmp_10034d6e */
		_emit 0x03
jmp_10034d6b:
		add edi, 0x2
jmp_10034d6e:
		add esi, ecx
		or ebx, ebx
		_emit 0x75 /* jne jmp_10034d45 */
		_emit 0xd1
		mov dword ptr [g_unk0x10068811], ebx
jmp_10034d7a:
		mov ebx, dword ptr [g_unk0x10068819]
		sub ebx, esi
		sub ebx, dword ptr [ebp+0xc]
		mov eax, dword ptr [ebp+0x10]
		add eax, esi
		sub eax, dword ptr [g_unk0x10068815]
		cmp eax, dword ptr [g_unk0x10068835]
		_emit 0x7d /* jge jmp_10034d9d */
		_emit 0x05
		mov dword ptr [g_unk0x10068835], eax
jmp_10034d9d:
		add eax, ebx
		dec eax
		cmp eax, dword ptr [g_unk0x1006883d]
		_emit 0x7e /* jle jmp_10034daf */
		_emit 0x07
		mov dword ptr [g_unk0x1006883d], eax
		_emit 0xeb /* jmp jmp_10034dda */
		_emit 0x2b
jmp_10034daf:
		mov ecx, ebx
		cmp ecx, 0x7f
		_emit 0x7c /* jl jmp_10034dbb */
		_emit 0x05
		mov ecx, 0x7f
jmp_10034dbb:
		mov edx, ecx
		mov al, cl
		add al, al
		inc al
		cmp dword ptr [g_unk0x1006880d], 0x0
		_emit 0x74 /* je jmp_10034dd3 */
		_emit 0x07
		mov byte ptr [edi], al
		inc edi
		rep movsb
		_emit 0xeb /* jmp jmp_10034dd8 */
		_emit 0x05
jmp_10034dd3:
		inc edi
		add esi, ecx
		add edi, ecx
jmp_10034dd8:
		sub ebx, edx
jmp_10034dda:
		or ebx, ebx
		_emit 0x75 /* jne jmp_10034daf */
		_emit 0xd1
		_emit 0xeb /* jmp jmp_10034e01 */
		_emit 0x21
jmp_10034de0:
		mov ebx, dword ptr [g_unk0x10068819]
		sub ebx, esi
		sub ebx, dword ptr [ebp+0xc]
		mov dword ptr [g_unk0x10068811], ebx
		_emit 0xeb /* jmp jmp_10034e01 */
		_emit 0x0e
jmp_10034df3:
		xor eax, eax
		cmp dword ptr [g_unk0x1006880d], 0x0
		_emit 0x74 /* je jmp_10034e00 */
		_emit 0x02
		mov byte ptr [edi], al
jmp_10034e00:
		inc edi
jmp_10034e01:
		mov dword ptr [g_unk0x1006881d], edi
		mov dword ptr [g_unk0x10068821], esi
		pop ecx
		pop eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_10034e15(PixelView* p_view, MechS32 p_unk0x04)
{
	STUB(0x10034e15);
}
#else
// FUNCTION: MW2SHELL 0x10034e15
__declspec(naked) void FUN_10034e15(PixelView* p_view, MechS32 p_unk0x04)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x24
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x18], eax
		_emit 0x7e /* jle jmp_10034e94 */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10034e94 */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x1c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10034e48 */
		_emit 0x05
		mov eax, 0x0
jmp_10034e48:
		mov dword ptr [ebp-0x4], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x20], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10034e5b */
		_emit 0x05
		mov eax, 0x0
jmp_10034e5b:
		mov dword ptr [ebp-0x8], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x18]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10034e6b */
		_emit 0x02
		mov eax, edx
jmp_10034e6b:
		mov dword ptr [ebp-0xc], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10034e7a */
		_emit 0x02
		mov eax, edx
jmp_10034e7a:
		mov dword ptr [ebp-0x10], eax
		mov eax, dword ptr [ebp-0xc]
		cmp eax, dword ptr [ebp-0x4]
		_emit 0x7c /* jl jmp_10034e9f */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x10]
		cmp eax, dword ptr [ebp-0x8]
		_emit 0x7c /* jl jmp_10034e9f */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x14], eax
		_emit 0xeb /* jmp jmp_10034eaa */
		_emit 0x16
jmp_10034e94:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10034e9f:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10034eaa:
		mov eax, dword ptr [ebp-0x8]
		imul dword ptr [ebp-0x18]
		add eax, dword ptr [ebp-0x14]
		add eax, dword ptr [ebp-0x4]
		mov edi, eax
		mov ebx, dword ptr [ebp-0xc]
		inc ebx
		sub ebx, dword ptr [ebp-0x4]
		mov esi, dword ptr [ebp-0x18]
		sub esi, ebx
		mov al, byte ptr [ebp+0xc]
		mov ah, al
		shl eax, 0x10
		mov al, byte ptr [ebp+0xc]
		mov ah, al
		mov edx, dword ptr [ebp-0x8]
		mov dword ptr [ebp-0x24], ebx
		cmp ebx, 0x4
		_emit 0x7e /* jle jmp_10034ee5 */
		_emit 0x09
		_emit 0xeb /* jmp jmp_10034f0b */
		_emit 0x2d
jmp_10034ede:
		mov ecx, ebx
		rep stosb
		add edi, esi
		inc edx
jmp_10034ee5:
		cmp edx, dword ptr [ebp-0x10]
		_emit 0x7e /* jle jmp_10034ede */
		_emit 0xf4
		_emit 0xeb /* jmp jmp_10034f10 */
		_emit 0x24
jmp_10034eec:
		mov ecx, edi
		mov ebx, dword ptr [ebp-0x24]
		neg ecx
		and ecx, 0x3
		sub ebx, ecx
		rep stosb
		mov ecx, ebx
		and ebx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, ebx
		rep stosb
		add edi, esi
		inc edx
jmp_10034f0b:
		cmp edx, dword ptr [ebp-0x10]
		_emit 0x7e /* jle jmp_10034eec */
		_emit 0xdc
jmp_10034f10:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_10034f18(
	PixelView* p_unk0x00,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	PixelView* p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18
)
{
	STUB(0x10034f18);
}
#else
// FUNCTION: MW2SHELL 0x10034f18
__declspec(naked) void FUN_10034f18(
	PixelView* p_unk0x00,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	PixelView* p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x9c
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x70], eax
		_emit 0x7e /* jle jmp_10034f9a */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10034f9a */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x78], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10034f4e */
		_emit 0x05
		mov eax, 0x0
jmp_10034f4e:
		mov dword ptr [ebp-0x60], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x7c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10034f61 */
		_emit 0x05
		mov eax, 0x0
jmp_10034f61:
		mov dword ptr [ebp-0x64], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x70]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10034f71 */
		_emit 0x02
		mov eax, edx
jmp_10034f71:
		mov dword ptr [ebp-0x68], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10034f80 */
		_emit 0x02
		mov eax, edx
jmp_10034f80:
		mov dword ptr [ebp-0x6c], eax
		mov eax, dword ptr [ebp-0x68]
		cmp eax, dword ptr [ebp-0x60]
		_emit 0x7c /* jl jmp_10034fa5 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x6c]
		cmp eax, dword ptr [ebp-0x64]
		_emit 0x7c /* jl jmp_10034fa5 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x74], eax
		_emit 0xeb /* jmp jmp_10034fb0 */
		_emit 0x16
jmp_10034f9a:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10034fa5:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10034fb0:
		mov eax, dword ptr [ebp-0x60]
		mov dword ptr [ebp-0x2c], eax
		mov eax, dword ptr [ebp-0x64]
		mov dword ptr [ebp-0x30], eax
		mov eax, dword ptr [ebp-0x68]
		mov dword ptr [ebp-0x34], eax
		mov eax, dword ptr [ebp-0x6c]
		mov dword ptr [ebp-0x38], eax
		mov eax, dword ptr [ebp-0x78]
		sub dword ptr [ebp-0x2c], eax
		sub dword ptr [ebp-0x34], eax
		mov eax, dword ptr [ebp-0x7c]
		sub dword ptr [ebp-0x30], eax
		sub dword ptr [ebp-0x38], eax
		mov esi, dword ptr [ebp+0x14]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x94], eax
		jle jmp_10035071
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10035071 */
		_emit 0x7a
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x98], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_1003500a */
		_emit 0x05
		mov eax, 0x0
jmp_1003500a:
		mov dword ptr [ebp-0x80], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x9c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10035020 */
		_emit 0x05
		mov eax, 0x0
jmp_10035020:
		mov dword ptr [ebp-0x84], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x94]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035036 */
		_emit 0x02
		mov eax, edx
jmp_10035036:
		mov dword ptr [ebp-0x88], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035048 */
		_emit 0x02
		mov eax, edx
jmp_10035048:
		mov dword ptr [ebp-0x8c], eax
		mov eax, dword ptr [ebp-0x88]
		cmp eax, dword ptr [ebp-0x80]
		_emit 0x7c /* jl jmp_1003507c */
		_emit 0x23
		mov eax, dword ptr [ebp-0x8c]
		cmp eax, dword ptr [ebp-0x84]
		_emit 0x7c /* jl jmp_1003507c */
		_emit 0x15
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x90], eax
		_emit 0xeb /* jmp jmp_10035087 */
		_emit 0x16
jmp_10035071:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1003507c:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10035087:
		mov eax, dword ptr [ebp-0x80]
		mov dword ptr [ebp-0x40], eax
		mov eax, dword ptr [ebp-0x84]
		mov dword ptr [ebp-0x44], eax
		mov eax, dword ptr [ebp-0x88]
		mov dword ptr [ebp-0x48], eax
		mov eax, dword ptr [ebp-0x8c]
		mov dword ptr [ebp-0x4c], eax
		mov eax, dword ptr [ebp-0x98]
		sub dword ptr [ebp-0x40], eax
		sub dword ptr [ebp-0x48], eax
		mov eax, dword ptr [ebp-0x9c]
		sub dword ptr [ebp-0x44], eax
		sub dword ptr [ebp-0x4c], eax
		mov eax, dword ptr [ebp+0xc]
		sub eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x24], eax
		mov eax, dword ptr [ebp+0x10]
		sub eax, dword ptr [ebp+0x1c]
		mov dword ptr [ebp-0x28], eax
		mov eax, dword ptr [ebp-0x2c]
		mov edx, dword ptr [ebp-0x40]
		add edx, dword ptr [ebp-0x24]
		cmp eax, edx
		_emit 0x7f /* jg jmp_100350e1 */
		_emit 0x02
		mov eax, edx
jmp_100350e1:
		mov dword ptr [ebp-0x4], eax
		mov eax, dword ptr [ebp-0x30]
		mov edx, dword ptr [ebp-0x44]
		add edx, dword ptr [ebp-0x28]
		cmp eax, edx
		_emit 0x7f /* jg jmp_100350f3 */
		_emit 0x02
		mov eax, edx
jmp_100350f3:
		mov dword ptr [ebp-0x8], eax
		mov eax, dword ptr [ebp-0x34]
		mov edx, dword ptr [ebp-0x48]
		add edx, dword ptr [ebp-0x24]
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035105 */
		_emit 0x02
		mov eax, edx
jmp_10035105:
		mov dword ptr [ebp-0xc], eax
		mov eax, dword ptr [ebp-0x38]
		mov edx, dword ptr [ebp-0x4c]
		add edx, dword ptr [ebp-0x28]
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035117 */
		_emit 0x02
		mov eax, edx
jmp_10035117:
		mov dword ptr [ebp-0x10], eax
		mov eax, dword ptr [ebp-0xc]
		cmp eax, dword ptr [ebp-0x4]
		jl jmp_100352a9
		mov eax, dword ptr [ebp-0x10]
		cmp eax, dword ptr [ebp-0x8]
		jl jmp_100352a9
		mov eax, dword ptr [ebp-0x40]
		mov edx, dword ptr [ebp-0x2c]
		sub edx, dword ptr [ebp-0x24]
		cmp eax, edx
		_emit 0x7f /* jg jmp_10035141 */
		_emit 0x02
		mov eax, edx
jmp_10035141:
		mov dword ptr [ebp-0x14], eax
		mov eax, dword ptr [ebp-0x44]
		mov edx, dword ptr [ebp-0x30]
		sub edx, dword ptr [ebp-0x28]
		cmp eax, edx
		_emit 0x7f /* jg jmp_10035153 */
		_emit 0x02
		mov eax, edx
jmp_10035153:
		mov dword ptr [ebp-0x18], eax
		mov eax, dword ptr [ebp-0x48]
		mov edx, dword ptr [ebp-0x34]
		sub edx, dword ptr [ebp-0x24]
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035165 */
		_emit 0x02
		mov eax, edx
jmp_10035165:
		mov dword ptr [ebp-0x1c], eax
		mov eax, dword ptr [ebp-0x4c]
		mov edx, dword ptr [ebp-0x38]
		sub edx, dword ptr [ebp-0x28]
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035177 */
		_emit 0x02
		mov eax, edx
jmp_10035177:
		mov dword ptr [ebp-0x20], eax
		mov eax, dword ptr [ebp-0xc]
		inc eax
		sub eax, dword ptr [ebp-0x4]
		mov dword ptr [ebp-0x58], eax
		mov eax, dword ptr [ebp-0x10]
		inc eax
		sub eax, dword ptr [ebp-0x8]
		mov dword ptr [ebp-0x54], eax
		mov eax, dword ptr [ebp-0x7c]
		imul dword ptr [ebp-0x70]
		add eax, dword ptr [ebp-0x74]
		add eax, dword ptr [ebp-0x78]
		mov esi, eax
		mov eax, dword ptr [ebp-0x9c]
		imul dword ptr [ebp-0x94]
		add eax, dword ptr [ebp-0x90]
		add eax, dword ptr [ebp-0x98]
		mov edi, eax
		mov eax, dword ptr [ebp-0x8]
		mov ebx, dword ptr [ebp-0x18]
		cmp eax, ebx
		_emit 0x7e /* jle jmp_100351e0 */
		_emit 0x20
		mul dword ptr [ebp-0x70]
		add esi, eax
		mov eax, ebx
		mul dword ptr [ebp-0x94]
		add edi, eax
		mov eax, dword ptr [ebp-0x70]
		mov dword ptr [ebp-0x3c], eax
		mov eax, dword ptr [ebp-0x94]
		mov dword ptr [ebp-0x50], eax
		_emit 0xeb /* jmp jmp_10035206 */
		_emit 0x26
jmp_100351e0:
		mov eax, dword ptr [ebp-0x10]
		mul dword ptr [ebp-0x70]
		add esi, eax
		mov eax, dword ptr [ebp-0x20]
		mul dword ptr [ebp-0x94]
		add edi, eax
		mov eax, dword ptr [ebp-0x70]
		neg eax
		mov dword ptr [ebp-0x3c], eax
		mov eax, dword ptr [ebp-0x94]
		neg eax
		mov dword ptr [ebp-0x50], eax
jmp_10035206:
		mov ecx, dword ptr [ebp-0x58]
		mov eax, dword ptr [ebp-0x4]
		mov ebx, dword ptr [ebp-0x14]
		cmp eax, ebx
		_emit 0x7e /* jle jmp_10035227 */
		_emit 0x14
		add esi, eax
		add edi, ebx
		sub dword ptr [ebp-0x3c], ecx
		sub dword ptr [ebp-0x50], ecx
		cld
		mov dword ptr [ebp-0x5c], 0x0
		_emit 0xeb /* jmp jmp_1003523b */
		_emit 0x14
jmp_10035227:
		add esi, dword ptr [ebp-0xc]
		add edi, dword ptr [ebp-0x1c]
		add dword ptr [ebp-0x3c], ecx
		add dword ptr [ebp-0x50], ecx
		std
		mov dword ptr [ebp-0x5c], 0x3
jmp_1003523b:
		mov eax, dword ptr [ebp+0x20]
		test eax, 0xffffff00
		_emit 0x74 /* jz jmp_10035274 */
		_emit 0x2f
		mov edx, dword ptr [ebp-0x54]
		mov eax, dword ptr [ebp-0x3c]
		mov ebx, dword ptr [ebp-0x50]
jmp_1003524e:
		mov ecx, dword ptr [ebp-0x58]
		and ecx, 0x3
		rep movsb
		mov ecx, dword ptr [ebp-0x58]
		shr ecx, 0x2
		sub esi, dword ptr [ebp-0x5c]
		sub edi, dword ptr [ebp-0x5c]
		rep movsd
		add esi, dword ptr [ebp-0x5c]
		add edi, dword ptr [ebp-0x5c]
		add esi, eax
		add edi, ebx
		dec edx
		_emit 0x75 /* jnz jmp_1003524e */
		_emit 0xdd
		cld
		_emit 0xeb /* jmp jmp_100352a1 */
		_emit 0x2d
jmp_10035274:
		mov dl, al
		mov ah, al
		shl eax, 0x10
		mov al, dl
		mov ah, al
		mov edx, dword ptr [ebp-0x54]
		mov ebx, dword ptr [ebp-0x50]
jmp_10035285:
		mov ecx, dword ptr [ebp-0x58]
		and ecx, 0x3
		rep stosb
		mov ecx, dword ptr [ebp-0x58]
		shr ecx, 0x2
		sub edi, dword ptr [ebp-0x5c]
		rep stosd
		add edi, dword ptr [ebp-0x5c]
		add edi, ebx
		dec edx
		_emit 0x75 /* jnz jmp_10035285 */
		_emit 0xe5
		cld
jmp_100352a1:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100352a9:
		mov eax, 0xfffffffd
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Scrolls the view by (p_dx, p_dy) with wrap-around, through nine FUN_10034f18 blits.
#ifdef COMPAT_MODE
MechS32 FUN_100352b4(PixelView* p_view, MechS32 p_dx, MechS32 p_dy, MechS32 p_mode, undefined4 p_color)
{
	STUB(0x100352b4);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100352b4
__declspec(naked) MechS32
FUN_100352b4(PixelView* p_view, MechS32 p_dx, MechS32 p_dy, MechS32 p_mode, undefined4 p_color)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x44
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0xc]
		inc eax
		sub eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x30], eax
		jle jmp_100354a6
		mov edx, dword ptr [esi+0x10]
		inc edx
		sub edx, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x34], edx
		jle jmp_100354a6
		neg eax
		mov dword ptr [ebp-0x38], eax
		neg edx
		mov dword ptr [ebp-0x3c], edx
		cmp dword ptr [ebp+0x14], 0x1
		_emit 0x74 /* je jmp_1003533b */
		_emit 0x47
		mov eax, dword ptr [ebp+0xc]
		cdq
		xor eax, edx
		sub eax, edx
		cmp eax, dword ptr [ebp-0x30]
		_emit 0x7d /* jge jmp_1003531f */
		_emit 0x1e
		mov eax, dword ptr [ebp+0x10]
		cdq
		xor eax, edx
		sub eax, edx
		cmp eax, dword ptr [ebp-0x34]
		_emit 0x7d /* jge jmp_1003531f */
		_emit 0x11
		mov eax, dword ptr [ebp+0x8]
		mov dword ptr [ebp-0x2c], eax
		mov eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x44], eax
		jmp jmp_100353a6
jmp_1003531f:
		mov eax, dword ptr [ebp+0x18]
		push 0x0
		movzx ax, al
		push ax
		push dword ptr [ebp+0x8]
		call FUN_10034e15
		add esp, 0x8
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1003533b:
		cmp dword ptr [ebp+0x18], 0x0
		je jmp_1003549a
		lea eax, [ebp-0x28]
		mov dword ptr [ebp-0x2c], eax
		mov eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x14], eax
		lea eax, [ebp-0x14]
		mov dword ptr [ebp-0x28], eax
		xor eax, eax
		mov dword ptr [ebp-0x24], eax
		mov dword ptr [ebp-0x20], eax
		mov eax, dword ptr [ebp-0x30]
		dec eax
		mov dword ptr [ebp-0x10], eax
		mov dword ptr [ebp-0x1c], eax
		mov eax, dword ptr [ebp-0x34]
		dec eax
		mov dword ptr [ebp-0xc], eax
		mov dword ptr [ebp-0x18], eax
		push -0x1
		push 0x0
		push 0x0
		push dword ptr [ebp-0x2c]
		push 0x0
		push 0x0
		push dword ptr [ebp+0x8]
		call FUN_10034f18
		add esp, 0x1c
		mov eax, dword ptr [ebp+0xc]
		cdq
		idiv dword ptr [ebp-0x30]
		mov dword ptr [ebp+0xc], edx
		mov eax, dword ptr [ebp+0x10]
		cdq
		idiv dword ptr [ebp-0x34]
		mov dword ptr [ebp+0x10], edx
		mov dword ptr [ebp-0x44], 0xffffffff
jmp_100353a6:
		mov eax, dword ptr [ebp+0xc]
		or eax, dword ptr [ebp+0x10]
		je jmp_10035492
		mov esi, dword ptr [ebp-0x2c]
		mov edi, dword ptr [ebp+0x8]
		push -0x1
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push edi
		push 0x0
		push 0x0
		push esi
		call FUN_10034f18
		add esp, 0x1c
		push dword ptr [ebp-0x44]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push edi
		push dword ptr [ebp-0x34]
		push dword ptr [ebp-0x30]
		push esi
		call FUN_10034f18
		add esp, 0x1c
		push dword ptr [ebp-0x44]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push edi
		push 0x0
		push dword ptr [ebp-0x30]
		push esi
		call FUN_10034f18
		add esp, 0x1c
		push dword ptr [ebp-0x44]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push edi
		push dword ptr [ebp-0x3c]
		push dword ptr [ebp-0x30]
		push esi
		call FUN_10034f18
		add esp, 0x1c
		push dword ptr [ebp-0x44]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push edi
		push dword ptr [ebp-0x34]
		push 0x0
		push esi
		call FUN_10034f18
		add esp, 0x1c
		push dword ptr [ebp-0x44]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push edi
		push dword ptr [ebp-0x3c]
		push 0x0
		push esi
		call FUN_10034f18
		add esp, 0x1c
		push dword ptr [ebp-0x44]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push edi
		push dword ptr [ebp-0x34]
		push dword ptr [ebp-0x38]
		push esi
		call FUN_10034f18
		add esp, 0x1c
		push dword ptr [ebp-0x44]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push edi
		push 0x0
		push dword ptr [ebp-0x38]
		push esi
		call FUN_10034f18
		add esp, 0x1c
		push dword ptr [ebp-0x44]
		push dword ptr [ebp+0x10]
		push dword ptr [ebp+0xc]
		push edi
		push dword ptr [ebp-0x3c]
		push dword ptr [ebp-0x38]
		push esi
		call FUN_10034f18
		add esp, 0x1c
jmp_10035492:
		xor eax, eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1003549a:
		mov eax, dword ptr [ebp-0x30]
		mul dword ptr [ebp-0x34]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100354a6:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Draws the outline of an ellipse centered on (p_x, p_y), clipped to the view.
#ifdef COMPAT_MODE
MechS32 FUN_100354b1(PixelView* p_view, MechS32 p_x, MechS32 p_y, MechS32 p_radiusX, MechS32 p_radiusY, MechS32 p_color)
{
	STUB(0x100354b1);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100354b1
__declspec(naked) MechS32
FUN_100354b1(PixelView* p_view, MechS32 p_x, MechS32 p_y, MechS32 p_radiusX, MechS32 p_radiusY, MechS32 p_color)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x54
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		cmp dword ptr [ebp+0x14], 0x0
		_emit 0x74 /* je jmp_100354ca */
		_emit 0x06
		cmp dword ptr [ebp+0x18], 0x0
		_emit 0x75 /* jne jmp_100354fb */
		_emit 0x31
jmp_100354ca:
		mov eax, dword ptr [ebp+0x10]
		add eax, dword ptr [ebp+0x18]
		mov ebx, dword ptr [ebp+0xc]
		add ebx, dword ptr [ebp+0x14]
		mov ecx, dword ptr [ebp+0x10]
		sub ecx, dword ptr [ebp+0x18]
		mov edx, dword ptr [ebp+0xc]
		sub edx, dword ptr [ebp+0x14]
		push dword ptr [ebp+0x1c]
		push 0x0
		push eax
		push ebx
		push ecx
		push edx
		push dword ptr [ebp+0x8]
		call FUN_10032449
		add esp, 0x1c
		jmp jmp_100357ec
jmp_100354fb:
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x4c], eax
		_emit 0x7e /* jle jmp_1003556d */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_1003556d */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x50], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10035521 */
		_emit 0x05
		mov eax, 0x0
jmp_10035521:
		mov dword ptr [ebp-0x38], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x54], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10035534 */
		_emit 0x05
		mov eax, 0x0
jmp_10035534:
		mov dword ptr [ebp-0x3c], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x4c]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035544 */
		_emit 0x02
		mov eax, edx
jmp_10035544:
		mov dword ptr [ebp-0x40], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035553 */
		_emit 0x02
		mov eax, edx
jmp_10035553:
		mov dword ptr [ebp-0x44], eax
		mov eax, dword ptr [ebp-0x40]
		cmp eax, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_10035578 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x44]
		cmp eax, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_10035578 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x48], eax
		_emit 0xeb /* jmp jmp_10035583 */
		_emit 0x16
jmp_1003556d:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10035578:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_10035583:
		mov eax, dword ptr [ebp+0x1c]
		mov ah, al
		mov dword ptr [ebp+0x1c], eax
		mov word ptr [ebp+0x1e], ax
		mov eax, dword ptr [ebp-0x50]
		add dword ptr [ebp+0xc], eax
		mov eax, dword ptr [ebp-0x54]
		add dword ptr [ebp+0x10], eax
		mov eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x4], eax
		mov eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x8], eax
		mov dword ptr [ebp-0xc], 0x0
		mov eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x10], eax
		mul eax
		mov dword ptr [ebp-0x1c], eax
		shl eax, 0x1
		mov dword ptr [ebp-0x20], eax
		mov eax, dword ptr [ebp+0x14]
		mul eax
		mov dword ptr [ebp-0x14], eax
		shl eax, 0x1
		mov dword ptr [ebp-0x18], eax
		mov dword ptr [ebp-0x24], 0x0
		mov eax, dword ptr [ebp-0x18]
		mul dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x28], eax
		mov eax, dword ptr [ebp-0x14]
		shr eax, 0x2
		add eax, dword ptr [ebp-0x1c]
		mov dword ptr [ebp-0x2c], eax
		mov eax, dword ptr [ebp-0x14]
		mul dword ptr [ebp+0x18]
		sub dword ptr [ebp-0x2c], eax
		mov ebx, dword ptr [ebp+0x18]
jmp_100355f3:
		mov eax, dword ptr [ebp-0x24]
		sub eax, dword ptr [ebp-0x28]
		jns jmp_100356e9
		push ebx
		mov ecx, dword ptr [ebp+0x1c]
		mov edi, dword ptr [ebp-0x4]
		add edi, dword ptr [ebp-0xc]
		mov edx, dword ptr [ebp-0x8]
		add edx, dword ptr [ebp-0x10]
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_10035631 */
		_emit 0x1d
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_10035631 */
		_emit 0x18
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_10035631 */
		_emit 0x13
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_10035631 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_10035631:
		mov edi, dword ptr [ebp-0x4]
		add edi, dword ptr [ebp-0xc]
		mov edx, dword ptr [ebp-0x8]
		sub edx, dword ptr [ebp-0x10]
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_1003565f */
		_emit 0x1d
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_1003565f */
		_emit 0x18
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_1003565f */
		_emit 0x13
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_1003565f */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_1003565f:
		mov edi, dword ptr [ebp-0x4]
		sub edi, dword ptr [ebp-0xc]
		mov edx, dword ptr [ebp-0x8]
		add edx, dword ptr [ebp-0x10]
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_1003568d */
		_emit 0x1d
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_1003568d */
		_emit 0x18
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_1003568d */
		_emit 0x13
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_1003568d */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_1003568d:
		mov edi, dword ptr [ebp-0x4]
		sub edi, dword ptr [ebp-0xc]
		mov edx, dword ptr [ebp-0x8]
		sub edx, dword ptr [ebp-0x10]
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_100356bb */
		_emit 0x1d
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_100356bb */
		_emit 0x18
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_100356bb */
		_emit 0x13
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_100356bb */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_100356bb:
		pop ebx
		cmp dword ptr [ebp-0x2c], 0x0
		_emit 0x78 /* js jmp_100356d2 */
		_emit 0x10
		dec dword ptr [ebp-0x10]
		dec ebx
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [ebp-0x18]
		mov dword ptr [ebp-0x28], eax
		sub dword ptr [ebp-0x2c], eax
jmp_100356d2:
		inc dword ptr [ebp-0xc]
		mov eax, dword ptr [ebp-0x24]
		add eax, dword ptr [ebp-0x20]
		mov dword ptr [ebp-0x24], eax
		add eax, dword ptr [ebp-0x1c]
		add dword ptr [ebp-0x2c], eax
		jmp jmp_100355f3
jmp_100356e9:
		mov eax, dword ptr [ebp-0x14]
		sub eax, dword ptr [ebp-0x1c]
		mov edx, eax
		sar eax, 0x1
		add eax, edx
		sub eax, dword ptr [ebp-0x24]
		sub eax, dword ptr [ebp-0x28]
		sar eax, 0x1
		add dword ptr [ebp-0x2c], eax
jmp_10035700:
		push ebx
		mov ecx, dword ptr [ebp+0x1c]
		mov edi, dword ptr [ebp-0x4]
		add edi, dword ptr [ebp-0xc]
		mov edx, dword ptr [ebp-0x8]
		add edx, dword ptr [ebp-0x10]
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_10035732 */
		_emit 0x1d
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_10035732 */
		_emit 0x18
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_10035732 */
		_emit 0x13
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_10035732 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_10035732:
		mov edi, dword ptr [ebp-0x4]
		add edi, dword ptr [ebp-0xc]
		mov edx, dword ptr [ebp-0x8]
		sub edx, dword ptr [ebp-0x10]
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_10035760 */
		_emit 0x1d
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_10035760 */
		_emit 0x18
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_10035760 */
		_emit 0x13
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_10035760 */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_10035760:
		mov edi, dword ptr [ebp-0x4]
		sub edi, dword ptr [ebp-0xc]
		mov edx, dword ptr [ebp-0x8]
		add edx, dword ptr [ebp-0x10]
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_1003578e */
		_emit 0x1d
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_1003578e */
		_emit 0x18
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_1003578e */
		_emit 0x13
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_1003578e */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_1003578e:
		mov edi, dword ptr [ebp-0x4]
		sub edi, dword ptr [ebp-0xc]
		mov edx, dword ptr [ebp-0x8]
		sub edx, dword ptr [ebp-0x10]
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_100357bc */
		_emit 0x1d
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_100357bc */
		_emit 0x18
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_100357bc */
		_emit 0x13
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_100357bc */
		_emit 0x0e
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov byte ptr [edi], cl
jmp_100357bc:
		pop ebx
		cmp dword ptr [ebp-0x2c], 0x0
		_emit 0x79 /* jns jmp_100357d2 */
		_emit 0x0f
		inc dword ptr [ebp-0xc]
		mov eax, dword ptr [ebp-0x24]
		add eax, dword ptr [ebp-0x20]
		mov dword ptr [ebp-0x24], eax
		add dword ptr [ebp-0x2c], eax
jmp_100357d2:
		dec dword ptr [ebp-0x10]
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [ebp-0x18]
		mov dword ptr [ebp-0x28], eax
		sub eax, dword ptr [ebp-0x14]
		sub dword ptr [ebp-0x2c], eax
		dec ebx
		_emit 0x78 /* js jmp_100357ec */
		_emit 0x05
		jmp jmp_10035700
jmp_100357ec:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Fills an ellipse centered on (p_x, p_y), clipped to the view.
#ifdef COMPAT_MODE
MechS32 FUN_100357f2(PixelView* p_view, MechS32 p_x, MechS32 p_y, MechS32 p_radiusX, MechS32 p_radiusY, MechS32 p_color)
{
	STUB(0x100357f2);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100357f2
__declspec(naked) MechS32
FUN_100357f2(PixelView* p_view, MechS32 p_x, MechS32 p_y, MechS32 p_radiusX, MechS32 p_radiusY, MechS32 p_color)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x54
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		cmp dword ptr [ebp+0x14], 0x0
		_emit 0x74 /* je jmp_1003580b */
		_emit 0x06
		cmp dword ptr [ebp+0x18], 0x0
		_emit 0x75 /* jne jmp_1003583c */
		_emit 0x31
jmp_1003580b:
		mov eax, dword ptr [ebp+0x10]
		add eax, dword ptr [ebp+0x18]
		mov ebx, dword ptr [ebp+0xc]
		add ebx, dword ptr [ebp+0x14]
		mov ecx, dword ptr [ebp+0x10]
		sub ecx, dword ptr [ebp+0x18]
		mov edx, dword ptr [ebp+0xc]
		sub edx, dword ptr [ebp+0x14]
		push dword ptr [ebp+0x1c]
		push 0x0
		push eax
		push ebx
		push ecx
		push edx
		push dword ptr [ebp+0x8]
		call FUN_10032449
		add esp, 0x1c
		jmp jmp_10035aea
jmp_1003583c:
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x4c], eax
		_emit 0x7e /* jle jmp_100358ae */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_100358ae */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x50], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10035862 */
		_emit 0x05
		mov eax, 0x0
jmp_10035862:
		mov dword ptr [ebp-0x38], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x54], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10035875 */
		_emit 0x05
		mov eax, 0x0
jmp_10035875:
		mov dword ptr [ebp-0x3c], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x4c]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035885 */
		_emit 0x02
		mov eax, edx
jmp_10035885:
		mov dword ptr [ebp-0x40], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10035894 */
		_emit 0x02
		mov eax, edx
jmp_10035894:
		mov dword ptr [ebp-0x44], eax
		mov eax, dword ptr [ebp-0x40]
		cmp eax, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_100358b9 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x44]
		cmp eax, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_100358b9 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x48], eax
		_emit 0xeb /* jmp jmp_100358c4 */
		_emit 0x16
jmp_100358ae:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100358b9:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_100358c4:
		mov eax, dword ptr [ebp+0x1c]
		mov ah, al
		mov dword ptr [ebp+0x1c], eax
		mov word ptr [ebp+0x1e], ax
		mov eax, dword ptr [ebp-0x50]
		add dword ptr [ebp+0xc], eax
		mov eax, dword ptr [ebp-0x54]
		add dword ptr [ebp+0x10], eax
		mov eax, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x4], eax
		mov eax, dword ptr [ebp+0x10]
		mov dword ptr [ebp-0x8], eax
		mov dword ptr [ebp-0xc], 0x0
		mov eax, dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x10], eax
		mul eax
		mov dword ptr [ebp-0x1c], eax
		shl eax, 0x1
		mov dword ptr [ebp-0x20], eax
		mov eax, dword ptr [ebp+0x14]
		mul eax
		mov dword ptr [ebp-0x14], eax
		shl eax, 0x1
		mov dword ptr [ebp-0x18], eax
		mov dword ptr [ebp-0x24], 0x0
		mov eax, dword ptr [ebp-0x18]
		mul dword ptr [ebp+0x18]
		mov dword ptr [ebp-0x28], eax
		mov eax, dword ptr [ebp-0x14]
		shr eax, 0x2
		add eax, dword ptr [ebp-0x1c]
		mov dword ptr [ebp-0x2c], eax
		mov eax, dword ptr [ebp-0x14]
		mul dword ptr [ebp+0x18]
		sub dword ptr [ebp-0x2c], eax
		mov ebx, dword ptr [ebp+0x18]
jmp_10035934:
		mov eax, dword ptr [ebp-0x24]
		sub eax, dword ptr [ebp-0x28]
		_emit 0x78 /* js jmp_10035941 */
		_emit 0x05
		jmp jmp_10035a09
jmp_10035941:
		mov edi, dword ptr [ebp-0x4]
		add edi, dword ptr [ebp-0xc]
		cmp edi, dword ptr [ebp-0x38]
		jl jmp_100359dc
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7c /* jl jmp_10035958 */
		_emit 0x03
		mov edi, dword ptr [ebp-0x40]
jmp_10035958:
		mov dword ptr [ebp-0x34], edi
		mov edi, dword ptr [ebp-0x4]
		sub edi, dword ptr [ebp-0xc]
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_100359dc */
		_emit 0x76
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7f /* jg jmp_1003596e */
		_emit 0x03
		mov edi, dword ptr [ebp-0x38]
jmp_1003596e:
		mov dword ptr [ebp-0x30], edi
		mov edx, dword ptr [ebp-0x8]
		add edx, dword ptr [ebp-0x10]
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_100359dc */
		_emit 0x60
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_100359a5 */
		_emit 0x24
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov ecx, dword ptr [ebp-0x34]
		sub ecx, dword ptr [ebp-0x30]
		inc ecx
		mov eax, dword ptr [ebp+0x1c]
		mov edx, ecx
		and edx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, edx
		rep stosb
jmp_100359a5:
		mov edi, dword ptr [ebp-0x30]
		mov edx, dword ptr [ebp-0x8]
		sub edx, dword ptr [ebp-0x10]
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_100359dc */
		_emit 0x29
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_100359dc */
		_emit 0x24
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov ecx, dword ptr [ebp-0x34]
		sub ecx, dword ptr [ebp-0x30]
		inc ecx
		mov eax, dword ptr [ebp+0x1c]
		mov edx, ecx
		and edx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, edx
		rep stosb
jmp_100359dc:
		cmp dword ptr [ebp-0x2c], 0x0
		_emit 0x78 /* js jmp_100359f2 */
		_emit 0x10
		dec dword ptr [ebp-0x10]
		dec ebx
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [ebp-0x18]
		mov dword ptr [ebp-0x28], eax
		sub dword ptr [ebp-0x2c], eax
jmp_100359f2:
		inc dword ptr [ebp-0xc]
		mov eax, dword ptr [ebp-0x24]
		add eax, dword ptr [ebp-0x20]
		mov dword ptr [ebp-0x24], eax
		add eax, dword ptr [ebp-0x1c]
		add dword ptr [ebp-0x2c], eax
		jmp jmp_10035934
jmp_10035a09:
		mov eax, dword ptr [ebp-0x14]
		sub eax, dword ptr [ebp-0x1c]
		mov edx, eax
		sar eax, 0x1
		add eax, edx
		sub eax, dword ptr [ebp-0x24]
		sub eax, dword ptr [ebp-0x28]
		sar eax, 0x1
		add dword ptr [ebp-0x2c], eax
jmp_10035a20:
		mov edi, dword ptr [ebp-0x4]
		add edi, dword ptr [ebp-0xc]
		cmp edi, dword ptr [ebp-0x38]
		jl jmp_10035abb
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7c /* jl jmp_10035a37 */
		_emit 0x03
		mov edi, dword ptr [ebp-0x40]
jmp_10035a37:
		mov dword ptr [ebp-0x34], edi
		mov edi, dword ptr [ebp-0x4]
		sub edi, dword ptr [ebp-0xc]
		cmp edi, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_10035abb */
		_emit 0x76
		cmp edi, dword ptr [ebp-0x38]
		_emit 0x7f /* jg jmp_10035a4d */
		_emit 0x03
		mov edi, dword ptr [ebp-0x38]
jmp_10035a4d:
		mov dword ptr [ebp-0x30], edi
		mov edx, dword ptr [ebp-0x8]
		add edx, dword ptr [ebp-0x10]
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_10035abb */
		_emit 0x60
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_10035a84 */
		_emit 0x24
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov ecx, dword ptr [ebp-0x34]
		sub ecx, dword ptr [ebp-0x30]
		inc ecx
		mov eax, dword ptr [ebp+0x1c]
		mov edx, ecx
		and edx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, edx
		rep stosb
jmp_10035a84:
		mov edi, dword ptr [ebp-0x30]
		mov edx, dword ptr [ebp-0x8]
		sub edx, dword ptr [ebp-0x10]
		cmp edx, dword ptr [ebp-0x3c]
		_emit 0x7c /* jl jmp_10035abb */
		_emit 0x29
		cmp edx, dword ptr [ebp-0x44]
		_emit 0x7f /* jg jmp_10035abb */
		_emit 0x24
		mov eax, edx
		imul dword ptr [ebp-0x4c]
		add eax, dword ptr [ebp-0x48]
		add eax, edi
		mov edi, eax
		mov ecx, dword ptr [ebp-0x34]
		sub ecx, dword ptr [ebp-0x30]
		inc ecx
		mov eax, dword ptr [ebp+0x1c]
		mov edx, ecx
		and edx, 0x3
		shr ecx, 0x2
		rep stosd
		mov ecx, edx
		rep stosb
jmp_10035abb:
		cmp dword ptr [ebp-0x2c], 0x0
		_emit 0x79 /* jns jmp_10035ad0 */
		_emit 0x0f
		inc dword ptr [ebp-0xc]
		mov eax, dword ptr [ebp-0x24]
		add eax, dword ptr [ebp-0x20]
		mov dword ptr [ebp-0x24], eax
		add dword ptr [ebp-0x2c], eax
jmp_10035ad0:
		dec dword ptr [ebp-0x10]
		mov eax, dword ptr [ebp-0x28]
		sub eax, dword ptr [ebp-0x18]
		mov dword ptr [ebp-0x28], eax
		sub eax, dword ptr [ebp-0x14]
		sub dword ptr [ebp-0x2c], eax
		dec ebx
		_emit 0x78 /* js jmp_10035aea */
		_emit 0x05
		jmp jmp_10035a20
jmp_10035aea:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// The 16.16 cosine of 0 to 90 degrees in tenths of a degree; read backwards, the sine. The
// original keeps it in .text, between FUN_100357f2 and FUN_10036904.
// GLOBAL: MW2SHELL 0x10035af0
MechS32 g_unk0x10035af0[0x385] = {
	0x10000, 0x10000, 0x10000, 0xffff, 0xfffe, 0xfffe, 0xfffc, 0xfffb, 0xfffa, 0xfff8, 0xfff6, 0xfff4, 0xfff2, 0xffef,
	0xffec,  0xffea,  0xffe6,  0xffe3, 0xffe0, 0xffdc, 0xffd8, 0xffd4, 0xffd0, 0xffcb, 0xffc7, 0xffc2, 0xffbd, 0xffb7,
	0xffb2,  0xffac,  0xffa6,  0xffa0, 0xff9a, 0xff93, 0xff8d, 0xff86, 0xff7f, 0xff77, 0xff70, 0xff68, 0xff60, 0xff58,
	0xff50,  0xff48,  0xff3f,  0xff36, 0xff2d, 0xff24, 0xff1a, 0xff10, 0xff07, 0xfefd, 0xfef2, 0xfee8, 0xfedd, 0xfed2,
	0xfec7,  0xfebc,  0xfeb1,  0xfea5, 0xfe99, 0xfe8d, 0xfe81, 0xfe74, 0xfe68, 0xfe5b, 0xfe4e, 0xfe40, 0xfe33, 0xfe25,
	0xfe18,  0xfe09,  0xfdfb,  0xfded, 0xfdde, 0xfdcf, 0xfdc0, 0xfdb1, 0xfda2, 0xfd92, 0xfd82, 0xfd72, 0xfd62, 0xfd52,
	0xfd41,  0xfd30,  0xfd1f,  0xfd0e, 0xfcfd, 0xfceb, 0xfcd9, 0xfcc7, 0xfcb5, 0xfca3, 0xfc90, 0xfc7d, 0xfc6a, 0xfc57,
	0xfc44,  0xfc30,  0xfc1c,  0xfc08, 0xfbf4, 0xfbe0, 0xfbcb, 0xfbb7, 0xfba2, 0xfb8d, 0xfb77, 0xfb62, 0xfb4c, 0xfb36,
	0xfb20,  0xfb0a,  0xfaf3,  0xfadc, 0xfac5, 0xfaae, 0xfa97, 0xfa80, 0xfa68, 0xfa50, 0xfa38, 0xfa20, 0xfa07, 0xf9ef,
	0xf9d6,  0xf9bd,  0xf9a3,  0xf98a, 0xf970, 0xf956, 0xf93c, 0xf922, 0xf908, 0xf8ed, 0xf8d2, 0xf8b7, 0xf89c, 0xf881,
	0xf865,  0xf84a,  0xf82e,  0xf811, 0xf7f5, 0xf7d9, 0xf7bc, 0xf79f, 0xf782, 0xf764, 0xf747, 0xf729, 0xf70b, 0xf6ed,
	0xf6cf,  0xf6b0,  0xf692,  0xf673, 0xf654, 0xf635, 0xf615, 0xf5f6, 0xf5d6, 0xf5b6, 0xf596, 0xf575, 0xf555, 0xf534,
	0xf513,  0xf4f2,  0xf4d0,  0xf4af, 0xf48d, 0xf46b, 0xf449, 0xf427, 0xf404, 0xf3e2, 0xf3bf, 0xf39c, 0xf378, 0xf355,
	0xf331,  0xf30e,  0xf2ea,  0xf2c5, 0xf2a1, 0xf27c, 0xf258, 0xf233, 0xf20e, 0xf1e8, 0xf1c3, 0xf19d, 0xf177, 0xf151,
	0xf12b,  0xf104,  0xf0de,  0xf0b7, 0xf090, 0xf068, 0xf041, 0xf019, 0xeff2, 0xefca, 0xefa2, 0xef79, 0xef51, 0xef28,
	0xeeff,  0xeed6,  0xeead,  0xee83, 0xee5a, 0xee30, 0xee06, 0xeddc, 0xedb1, 0xed87, 0xed5c, 0xed31, 0xed06, 0xecdb,
	0xecaf,  0xec83,  0xec58,  0xec2b, 0xebff, 0xebd3, 0xeba6, 0xeb79, 0xeb4c, 0xeb1f, 0xeaf2, 0xeac4, 0xea97, 0xea69,
	0xea3b,  0xea0d,  0xe9de,  0xe9b0, 0xe981, 0xe952, 0xe923, 0xe8f3, 0xe8c4, 0xe894, 0xe864, 0xe834, 0xe804, 0xe7d3,
	0xe7a3,  0xe772,  0xe741,  0xe710, 0xe6de, 0xe6ad, 0xe67b, 0xe649, 0xe617, 0xe5e5, 0xe5b3, 0xe580, 0xe54d, 0xe51a,
	0xe4e7,  0xe4b4,  0xe481,  0xe44d, 0xe419, 0xe3e5, 0xe3b1, 0xe37c, 0xe348, 0xe313, 0xe2de, 0xe2a9, 0xe274, 0xe23e,
	0xe209,  0xe1d3,  0xe19d,  0xe167, 0xe131, 0xe0fa, 0xe0c3, 0xe08d, 0xe056, 0xe01e, 0xdfe7, 0xdfb0, 0xdf78, 0xdf40,
	0xdf08,  0xded0,  0xde97,  0xde5f, 0xde26, 0xdded, 0xddb4, 0xdd7b, 0xdd41, 0xdd07, 0xdcce, 0xdc94, 0xdc5a, 0xdc1f,
	0xdbe5,  0xdbaa,  0xdb6f,  0xdb34, 0xdaf9, 0xdabe, 0xda82, 0xda47, 0xda0b, 0xd9cf, 0xd993, 0xd956, 0xd91a, 0xd8dd,
	0xd8a0,  0xd863,  0xd826,  0xd7e9, 0xd7ab, 0xd76d, 0xd72f, 0xd6f1, 0xd6b3, 0xd675, 0xd636, 0xd5f7, 0xd5b9, 0xd57a,
	0xd53a,  0xd4fb,  0xd4bb,  0xd47c, 0xd43c, 0xd3fc, 0xd3bc, 0xd37b, 0xd33b, 0xd2fa, 0xd2b9, 0xd278, 0xd237, 0xd1f5,
	0xd1b4,  0xd172,  0xd130,  0xd0ee, 0xd0ac, 0xd06a, 0xd027, 0xcfe5, 0xcfa2, 0xcf5f, 0xcf1c, 0xced8, 0xce95, 0xce51,
	0xce0e,  0xcdca,  0xcd85,  0xcd41, 0xccfd, 0xccb8, 0xcc73, 0xcc2e, 0xcbe9, 0xcba4, 0xcb5f, 0xcb19, 0xcad3, 0xca8e,
	0xca48,  0xca01,  0xc9bb,  0xc975, 0xc92e, 0xc8e7, 0xc8a0, 0xc859, 0xc812, 0xc7ca, 0xc783, 0xc73b, 0xc6f3, 0xc6ab,
	0xc663,  0xc61a,  0xc5d2,  0xc589, 0xc540, 0xc4f7, 0xc4ae, 0xc465, 0xc41b, 0xc3d2, 0xc388, 0xc33e, 0xc2f4, 0xc2aa,
	0xc260,  0xc215,  0xc1ca,  0xc180, 0xc135, 0xc0ea, 0xc09e, 0xc053, 0xc007, 0xbfbc, 0xbf70, 0xbf24, 0xbed8, 0xbe8b,
	0xbe3f,  0xbdf2,  0xbda5,  0xbd58, 0xbd0b, 0xbcbe, 0xbc71, 0xbc23, 0xbbd6, 0xbb88, 0xbb3a, 0xbaec, 0xba9e, 0xba4f,
	0xba01,  0xb9b2,  0xb963,  0xb914, 0xb8c5, 0xb876, 0xb827, 0xb7d7, 0xb787, 0xb738, 0xb6e8, 0xb698, 0xb647, 0xb5f7,
	0xb5a6,  0xb556,  0xb505,  0xb4b4, 0xb463, 0xb412, 0xb3c0, 0xb36f, 0xb31d, 0xb2cb, 0xb279, 0xb227, 0xb1d5, 0xb183,
	0xb130,  0xb0de,  0xb08b,  0xb038, 0xafe5, 0xaf92, 0xaf3e, 0xaeeb, 0xae97, 0xae44, 0xadf0, 0xad9c, 0xad48, 0xacf3,
	0xac9f,  0xac4b,  0xabf6,  0xaba1, 0xab4c, 0xaaf7, 0xaaa2, 0xaa4d, 0xa9f7, 0xa9a1, 0xa94c, 0xa8f6, 0xa8a0, 0xa84a,
	0xa7f3,  0xa79d,  0xa747,  0xa6f0, 0xa699, 0xa642, 0xa5eb, 0xa594, 0xa53d, 0xa4e5, 0xa48e, 0xa436, 0xa3de, 0xa386,
	0xa32e,  0xa2d6,  0xa27e,  0xa225, 0xa1cd, 0xa174, 0xa11b, 0xa0c2, 0xa069, 0xa010, 0x9fb7, 0x9f5d, 0x9f04, 0x9eaa,
	0x9e50,  0x9df6,  0x9d9c,  0x9d42, 0x9ce7, 0x9c8d, 0x9c32, 0x9bd8, 0x9b7d, 0x9b22, 0x9ac7, 0x9a6c, 0x9a11, 0x99b5,
	0x995a,  0x98fe,  0x98a2,  0x9846, 0x97ea, 0x978e, 0x9732, 0x96d6, 0x9679, 0x961c, 0x95c0, 0x9563, 0x9506, 0x94a9,
	0x944c,  0x93ee,  0x9391,  0x9334, 0x92d6, 0x9278, 0x921a, 0x91bc, 0x915e, 0x9100, 0x90a2, 0x9043, 0x8fe5, 0x8f86,
	0x8f27,  0x8ec8,  0x8e69,  0x8e0a, 0x8dab, 0x8d4c, 0x8cec, 0x8c8d, 0x8c2d, 0x8bcd, 0x8b6d, 0x8b0d, 0x8aad, 0x8a4d,
	0x89ed,  0x898c,  0x892c,  0x88cb, 0x886b, 0x880a, 0x87a9, 0x8748, 0x86e7, 0x8685, 0x8624, 0x85c2, 0x8561, 0x84ff,
	0x849d,  0x843c,  0x83da,  0x8377, 0x8315, 0x82b3, 0x8251, 0x81ee, 0x818b, 0x8129, 0x80c6, 0x8063, 0x8000, 0x7f9d,
	0x7f3a,  0x7ed6,  0x7e73,  0x7e0f, 0x7dac, 0x7d48, 0x7ce4, 0x7c80, 0x7c1c, 0x7bb8, 0x7b54, 0x7af0, 0x7a8c, 0x7a27,
	0x79c3,  0x795e,  0x78f9,  0x7894, 0x782f, 0x77ca, 0x7765, 0x7700, 0x769b, 0x7635, 0x75d0, 0x756a, 0x7504, 0x749f,
	0x7439,  0x73d3,  0x736d,  0x7307, 0x72a0, 0x723a, 0x71d4, 0x716d, 0x7107, 0x70a0, 0x7039, 0x6fd2, 0x6f6b, 0x6f04,
	0x6e9d,  0x6e36,  0x6dcf,  0x6d67, 0x6d00, 0x6c98, 0x6c31, 0x6bc9, 0x6b61, 0x6af9, 0x6a91, 0x6a29, 0x69c1, 0x6959,
	0x68f1,  0x6888,  0x6820,  0x67b7, 0x674f, 0x66e6, 0x667d, 0x6614, 0x65ab, 0x6542, 0x64d9, 0x6470, 0x6407, 0x639e,
	0x6334,  0x62cb,  0x6261,  0x61f8, 0x618e, 0x6124, 0x60ba, 0x6050, 0x5fe6, 0x5f7c, 0x5f12, 0x5ea8, 0x5e3d, 0x5dd3,
	0x5d69,  0x5cfe,  0x5c93,  0x5c29, 0x5bbe, 0x5b53, 0x5ae8, 0x5a7d, 0x5a12, 0x59a7, 0x593c, 0x58d1, 0x5865, 0x57fa,
	0x578f,  0x5723,  0x56b8,  0x564c, 0x55e0, 0x5574, 0x5509, 0x549d, 0x5431, 0x53c5, 0x5358, 0x52ec, 0x5280, 0x5214,
	0x51a7,  0x513b,  0x50ce,  0x5062, 0x4ff5, 0x4f88, 0x4f1c, 0x4eaf, 0x4e42, 0x4dd5, 0x4d68, 0x4cfb, 0x4c8e, 0x4c21,
	0x4bb4,  0x4b46,  0x4ad9,  0x4a6b, 0x49fe, 0x4990, 0x4923, 0x48b5, 0x4848, 0x47da, 0x476c, 0x46fe, 0x4690, 0x4622,
	0x45b4,  0x4546,  0x44d8,  0x446a, 0x43fb, 0x438d, 0x431f, 0x42b0, 0x4242, 0x41d3, 0x4165, 0x40f6, 0x4088, 0x4019,
	0x3faa,  0x3f3b,  0x3ecc,  0x3e5e, 0x3def, 0x3d80, 0x3d11, 0x3ca1, 0x3c32, 0x3bc3, 0x3b54, 0x3ae5, 0x3a75, 0x3a06,
	0x3996,  0x3927,  0x38b7,  0x3848, 0x37d8, 0x3769, 0x36f9, 0x3689, 0x3619, 0x35aa, 0x353a, 0x34ca, 0x345a, 0x33ea,
	0x337a,  0x330a,  0x329a,  0x322a, 0x31b9, 0x3149, 0x30d9, 0x3069, 0x2ff8, 0x2f88, 0x2f17, 0x2ea7, 0x2e37, 0x2dc6,
	0x2d55,  0x2ce5,  0x2c74,  0x2c04, 0x2b93, 0x2b22, 0x2ab1, 0x2a41, 0x29d0, 0x295f, 0x28ee, 0x287d, 0x280c, 0x279b,
	0x272a,  0x26b9,  0x2648,  0x25d7, 0x2566, 0x24f5, 0x2483, 0x2412, 0x23a1, 0x2330, 0x22be, 0x224d, 0x21dc, 0x216a,
	0x20f9,  0x2087,  0x2016,  0x1fa4, 0x1f33, 0x1ec1, 0x1e50, 0x1dde, 0x1d6d, 0x1cfb, 0x1c89, 0x1c18, 0x1ba6, 0x1b34,
	0x1ac2,  0x1a51,  0x19df,  0x196d, 0x18fb, 0x1889, 0x1817, 0x17a6, 0x1734, 0x16c2, 0x1650, 0x15de, 0x156c, 0x14fa,
	0x1488,  0x1416,  0x13a4,  0x1332, 0x12c0, 0x124e, 0x11dc, 0x1169, 0x10f7, 0x1085, 0x1013, 0xfa1,  0xf2f,  0xebd,
	0xe4a,   0xdd8,   0xd66,   0xcf4,  0xc81,  0xc0f,  0xb9d,  0xb2b,  0xab8,  0xa46,  0x9d4,  0x961,  0x8ef,  0x87d,
	0x80b,   0x798,   0x726,   0x6b4,  0x641,  0x5cf,  0x55c,  0x4ea,  0x478,  0x405,  0x393,  0x321,  0x2ae,  0x23c,
	0x1ca,   0x157,   0xe5,    0x72,   0x0
};

// Looks up the 16.16 cosine and sine of p_angle (in tenths of a degree) in g_unk0x10035af0.
#ifdef COMPAT_MODE
void FUN_10036904(MechS32 p_angle, MechS32* p_cos, MechS32* p_sin)
{
	STUB(0x10036904);
}
#else
// FUNCTION: MW2SHELL 0x10036904
__declspec(naked) void FUN_10036904(MechS32 p_angle, MechS32* p_cos, MechS32* p_sin)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov ebx, dword ptr [ebp+0x8]
		and ebx, ebx
		_emit 0x79 /* jns jmp_10036922 */
		_emit 0x10
jmp_10036912:
		add ebx, 0xe10
		_emit 0x78 /* js jmp_10036912 */
		_emit 0xf8
		_emit 0xeb /* jmp jmp_10036922 */
		_emit 0x06
jmp_1003691c:
		sub ebx, 0xe10
jmp_10036922:
		cmp ebx, 0xe10
		_emit 0x7f /* jg jmp_1003691c */
		_emit 0xf2
		cmp ebx, 0x708
		_emit 0x77 /* ja jmp_1003696a */
		_emit 0x38
		cmp ebx, 0x384
		_emit 0x77 /* ja jmp_1003694d */
		_emit 0x13
		shl ebx, 0x2
		mov eax, dword ptr [g_unk0x10035af0+ebx]
		neg ebx
		mov edx, dword ptr [g_unk0x10035af0+ebx+0xe10]
		_emit 0xeb /* jmp jmp_100369ac */
		_emit 0x5f
jmp_1003694d:
		neg ebx
		add ebx, 0x708
		shl ebx, 0x2
		mov eax, dword ptr [g_unk0x10035af0+ebx]
		neg eax
		neg ebx
		mov edx, dword ptr [g_unk0x10035af0+ebx+0xe10]
		_emit 0xeb /* jmp jmp_100369ac */
		_emit 0x42
jmp_1003696a:
		neg ebx
		add ebx, 0xe10
		cmp ebx, 0x384
		_emit 0x77 /* ja jmp_1003698f */
		_emit 0x15
		shl ebx, 0x2
		mov eax, dword ptr [g_unk0x10035af0+ebx]
		neg ebx
		mov edx, dword ptr [g_unk0x10035af0+ebx+0xe10]
		neg edx
		_emit 0xeb /* jmp jmp_100369ac */
		_emit 0x1d
jmp_1003698f:
		neg ebx
		add ebx, 0x708
		shl ebx, 0x2
		mov eax, dword ptr [g_unk0x10035af0+ebx]
		neg eax
		neg ebx
		mov edx, dword ptr [g_unk0x10035af0+ebx+0xe10]
		neg edx
jmp_100369ac:
		mov ebx, dword ptr [ebp+0xc]
		mov dword ptr [ebx], eax
		mov ebx, dword ptr [ebp+0x10]
		mov dword ptr [ebx], edx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Multiplies two 16.16 fixed-point values, rounding, into *p_result.
#ifdef COMPAT_MODE
void FUN_100369bc(MechS32 p_a, MechS32 p_b, MechS32* p_result)
{
	STUB(0x100369bc);
}
#else
// FUNCTION: MW2SHELL 0x100369bc
__declspec(naked) void FUN_100369bc(MechS32 p_a, MechS32 p_b, MechS32* p_result)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov eax, dword ptr [ebp+0x8]
		imul dword ptr [ebp+0xc]
		add eax, 0x8000
		adc edx, 0x0
		mov ax, dx
		ror eax, 0x10
		mov edi, dword ptr [ebp+0x10]
		mov dword ptr [edi], eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Rotates the point p_point about p_origin by p_angle (in tenths of a degree) and scales it
// by the 16.16 factors p_scaleX and p_scaleY, storing the result in p_result.
#ifdef COMPAT_MODE
void FUN_100369e2(
	MechS32* p_point,
	MechS32* p_result,
	MechS32* p_origin,
	MechS32 p_angle,
	MechS32 p_scaleX,
	MechS32 p_scaleY
)
{
	STUB(0x100369e2);
}
#else
// FUNCTION: MW2SHELL 0x100369e2
__declspec(naked) void FUN_100369e2(
	MechS32* p_point,
	MechS32* p_result,
	MechS32* p_origin,
	MechS32 p_angle,
	MechS32 p_scaleX,
	MechS32 p_scaleY
)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x20
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		lea eax, [ebp-0x8]
		push eax
		lea eax, [ebp-0x4]
		push eax
		push dword ptr [ebp+0x14]
		call FUN_10036904
		add esp, 0xc
		mov esi, dword ptr [ebp+0x8]
		mov edi, dword ptr [ebp+0x10]
		mov eax, dword ptr [esi]
		sub eax, dword ptr [edi]
		shl eax, 0x10
		imul dword ptr [ebp+0x18]
		add eax, 0x8000
		adc edx, 0x0
		mov ebx, edx
		mov eax, ebx
		imul dword ptr [ebp-0x4]
		add eax, 0x8000
		adc edx, 0x0
		mov ax, dx
		ror eax, 0x10
		mov dword ptr [ebp-0xc], eax
		mov eax, ebx
		imul dword ptr [ebp-0x8]
		add eax, 0x8000
		adc edx, 0x0
		mov ax, dx
		ror eax, 0x10
		mov dword ptr [ebp-0x14], eax
		mov eax, dword ptr [esi+0x4]
		sub eax, dword ptr [edi+0x4]
		shl eax, 0x10
		imul dword ptr [ebp+0x1c]
		add eax, 0x8000
		adc edx, 0x0
		mov ecx, edx
		mov eax, ecx
		imul dword ptr [ebp-0x4]
		add eax, 0x8000
		adc edx, 0x0
		mov ax, dx
		ror eax, 0x10
		mov dword ptr [ebp-0x18], eax
		mov eax, ecx
		imul dword ptr [ebp-0x8]
		add eax, 0x8000
		adc edx, 0x0
		mov ax, dx
		ror eax, 0x10
		mov dword ptr [ebp-0x10], eax
		mov esi, dword ptr [ebp+0xc]
		mov edx, dword ptr [ebp-0xc]
		sub edx, dword ptr [ebp-0x10]
		add edx, dword ptr [edi]
		mov dword ptr [esi], edx
		mov edx, dword ptr [ebp-0x18]
		add edx, dword ptr [ebp-0x14]
		add edx, dword ptr [edi+0x4]
		mov dword ptr [esi+0x4], edx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the dword at +0x8 of the font data (Font keeps it as the line height).
#ifdef COMPAT_MODE
MechS32 FUN_10036aa9(void* p_data)
{
	STUB(0x10036aa9);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10036aa9
__declspec(naked) MechS32 FUN_10036aa9(void* p_data)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0x8]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Looks up p_char in the font data's offset table at +0x10 and returns the dword at that
// offset (Font sums it as the character's width).
#ifdef COMPAT_MODE
MechS32 FUN_10036abc(void* p_data, MechS32 p_char)
{
	STUB(0x10036abc);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10036abc
__declspec(naked) MechS32 FUN_10036abc(void* p_data, MechS32 p_char)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x2
		add eax, dword ptr [ebp+0x8]
		add eax, 0x10
		mov esi, dword ptr [eax]
		add esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Draws one character of the font data into the view, clipped; a palette maps its pixels and
// skips 0xff. Returns the character's advance, -1 for an empty view or -2 when fully clipped.
#ifdef COMPAT_MODE
MechS32 FUN_10036adc(
	PixelView* p_view,
	MechS32 p_left,
	MechS32 p_top,
	void* p_font,
	MechS32 p_char,
	undefined* p_palette
)
{
	STUB(0x10036adc);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10036adc
__declspec(naked) MechS32
FUN_10036adc(PixelView* p_view, MechS32 p_left, MechS32 p_top, void* p_font, MechS32 p_char, undefined* p_palette)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x30
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x28], eax
		_emit 0x7e /* jle jmp_10036b5b */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10036b5b */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x2c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10036b0f */
		_emit 0x05
		mov eax, 0x0
	jmp_10036b0f:
		mov dword ptr [ebp-0x14], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x30], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10036b22 */
		_emit 0x05
		mov eax, 0x0
	jmp_10036b22:
		mov dword ptr [ebp-0x18], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x28]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10036b32 */
		_emit 0x02
		mov eax, edx
	jmp_10036b32:
		mov dword ptr [ebp-0x1c], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10036b41 */
		_emit 0x02
		mov eax, edx
	jmp_10036b41:
		mov dword ptr [ebp-0x20], eax
		mov eax, dword ptr [ebp-0x1c]
		cmp eax, dword ptr [ebp-0x14]
		_emit 0x7c /* jl jmp_10036b66 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x20]
		cmp eax, dword ptr [ebp-0x18]
		_emit 0x7c /* jl jmp_10036b66 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x24], eax
		_emit 0xeb /* jmp jmp_10036b71 */
		_emit 0x16
	jmp_10036b5b:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10036b66:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10036b71:
		mov eax, dword ptr [ebp-0x2c]
		add dword ptr [ebp+0xc], eax
		mov eax, dword ptr [ebp-0x30]
		add dword ptr [ebp+0x10], eax
		mov esi, dword ptr [ebp+0x14]
		mov edx, dword ptr [esi+0x8]
		mov eax, dword ptr [ebp+0x18]
		shl eax, 0x2
		add eax, dword ptr [ebp+0x14]
		add eax, 0x10
		mov esi, dword ptr [eax]
		add esi, dword ptr [ebp+0x14]
		mov dword ptr [ebp-0x8], 0x0
		mov ecx, dword ptr [esi]
		mov dword ptr [ebp-0x4], ecx
		cmp ecx, 0x0
		jz jmp_10036c5b
		add esi, 0x4
		mov edi, dword ptr [ebp+0x8]
		mov eax, dword ptr [ebp-0x1c]
		inc eax
		sub eax, ecx
		sub eax, dword ptr [ebp+0xc]
		_emit 0x79 /* jns jmp_10036bc2 */
		_emit 0x08
		add ecx, eax
		jle jmp_10036c5b
	jmp_10036bc2:
		mov eax, dword ptr [ebp+0xc]
		sub eax, dword ptr [ebp-0x14]
		_emit 0x79 /* jns jmp_10036bd7 */
		_emit 0x0d
		add ecx, eax
		jle jmp_10036c5b
		sub esi, eax
		sub dword ptr [ebp+0xc], eax
	jmp_10036bd7:
		mov eax, dword ptr [ebp-0x20]
		inc eax
		sub eax, edx
		sub eax, dword ptr [ebp+0x10]
		_emit 0x79 /* jns jmp_10036be6 */
		_emit 0x04
		add edx, eax
		_emit 0x7e /* jle jmp_10036c5b */
		_emit 0x75
	jmp_10036be6:
		mov eax, dword ptr [ebp+0x10]
		sub eax, dword ptr [ebp-0x18]
		_emit 0x79 /* jns jmp_10036bfb */
		_emit 0x0d
		add edx, eax
		_emit 0x7e /* jle jmp_10036c5b */
		_emit 0x69
		sub dword ptr [ebp+0x10], eax
		imul eax, dword ptr [ebp-0x4]
		sub esi, eax
	jmp_10036bfb:
		mov dword ptr [ebp-0x10], edx
		mov eax, dword ptr [ebp+0x10]
		imul dword ptr [ebp-0x28]
		add eax, dword ptr [ebp-0x24]
		add eax, dword ptr [ebp+0xc]
		mov edi, eax
		mov dword ptr [ebp-0x8], ecx
		sub dword ptr [ebp-0x4], ecx
		mov eax, dword ptr [ebp-0x28]
		sub eax, ecx
		mov dword ptr [ebp-0xc], eax
		mov edx, dword ptr [ebp-0x10]
		cmp dword ptr [ebp+0x1c], 0x0
		_emit 0x75 /* jnz jmp_10036c3d */
		_emit 0x1a
	jmp_10036c23:
		rep movsb
		mov ecx, dword ptr [ebp-0x8]
		add esi, dword ptr [ebp-0x4]
		add edi, dword ptr [ebp-0xc]
		dec edx
		_emit 0x75 /* jnz jmp_10036c23 */
		_emit 0xf2
		mov eax, dword ptr [ebp-0x4]
		add eax, dword ptr [ebp-0x8]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10036c3d:
		_emit 0xe3 /* jecxz jmp_10036c5b */
		_emit 0x1c
		mov ebx, dword ptr [ebp+0x1c]
	jmp_10036c42:
		mov al, byte ptr [esi]
		xlatb
		cmp al, 0xff
		_emit 0x74 /* jz jmp_10036c4b */
		_emit 0x02
		mov byte ptr [edi], al
	jmp_10036c4b:
		inc esi
		inc edi
		loop jmp_10036c42
		mov ecx, dword ptr [ebp-0x8]
		add esi, dword ptr [ebp-0x4]
		add edi, dword ptr [ebp-0xc]
		dec edx
		_emit 0x75 /* jnz jmp_10036c42 */
		_emit 0xe7
	jmp_10036c5b:
		mov eax, dword ptr [ebp-0x4]
		add eax, dword ptr [ebp-0x8]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_10036c67(
	PixelView* p_view,
	MechS32 p_left,
	MechS32 p_top,
	void* p_font,
	MechChar* p_text,
	undefined* p_palette
)
{
	STUB(0x10036c67);
}
#else
// FUNCTION: MW2SHELL 0x10036c67
__declspec(naked) void FUN_10036c67(
	PixelView* p_view,
	MechS32 p_left,
	MechS32 p_top,
	void* p_font,
	MechChar* p_text,
	undefined* p_palette
)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x18]
		mov edi, dword ptr [ebp+0xc]
	jmp_10036c77:
		movzx eax, byte ptr [esi]
		push dword ptr [ebp+0x1c]
		push eax
		push dword ptr [ebp+0x14]
		push dword ptr [ebp+0x10]
		push edi
		push dword ptr [ebp+0x8]
		call FUN_10036adc
		add esp, 0x18
		add edi, eax
		inc esi
		cmp byte ptr [esi], 0x0
		_emit 0x75 /* jnz jmp_10036c77 */
		_emit 0xdf
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// The IFF chunk tags that FUN_10036def, FUN_10036fb6 and FUN_10036fe7 look up. The original
// keeps them in .text, right after FUN_10036c9e's ret.
// GLOBAL: MW2SHELL 0x10036da1
MechChar g_unk0x10036da1[4] = {'B', 'M', 'H', 'D'};

// GLOBAL: MW2SHELL 0x10036da5
MechChar g_unk0x10036da5[4] = {'C', 'M', 'A', 'P'};

// GLOBAL: MW2SHELL 0x10036da9
MechChar g_unk0x10036da9[4] = {'B', 'O', 'D', 'Y'};

// Copies p_count pixels into row p_index of the view, clipped.
#ifdef COMPAT_MODE
void FUN_10036c9e(PixelView* p_view, MechS32 p_index, undefined* p_data, MechS32 p_count)
{
	STUB(0x10036c9e);
}
#else
// FUNCTION: MW2SHELL 0x10036c9e
__declspec(naked) void FUN_10036c9e(PixelView* p_view, MechS32 p_index, undefined* p_data, MechS32 p_count)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x24
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x1c], eax
		_emit 0x7e /* jle jmp_10036d1d */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10036d1d */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x20], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10036cd1 */
		_emit 0x05
		mov eax, 0x0
	jmp_10036cd1:
		mov dword ptr [ebp-0x8], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x24], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10036ce4 */
		_emit 0x05
		mov eax, 0x0
	jmp_10036ce4:
		mov dword ptr [ebp-0xc], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x1c]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10036cf4 */
		_emit 0x02
		mov eax, edx
	jmp_10036cf4:
		mov dword ptr [ebp-0x10], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10036d03 */
		_emit 0x02
		mov eax, edx
	jmp_10036d03:
		mov dword ptr [ebp-0x14], eax
		mov eax, dword ptr [ebp-0x10]
		cmp eax, dword ptr [ebp-0x8]
		_emit 0x7c /* jl jmp_10036d28 */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x14]
		cmp eax, dword ptr [ebp-0xc]
		_emit 0x7c /* jl jmp_10036d28 */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x18], eax
		_emit 0xeb /* jmp jmp_10036d33 */
		_emit 0x16
	jmp_10036d1d:
		mov eax, 0xffffffff
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10036d28:
		mov eax, 0xfffffffe
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	jmp_10036d33:
		mov dword ptr [ebp-0x4], 0x0
		mov eax, dword ptr [ebp-0x20]
		add dword ptr [ebp-0x4], eax
		mov eax, dword ptr [ebp-0x24]
		add dword ptr [ebp+0xc], eax
		mov esi, dword ptr [ebp+0x10]
		mov edi, dword ptr [ebp+0x8]
		mov ecx, dword ptr [ebp+0x14]
		mov eax, dword ptr [ebp-0x10]
		sub eax, dword ptr [ebp-0x4]
		inc eax
		sub eax, ecx
		_emit 0x79 /* jns jmp_10036d5e */
		_emit 0x04
		add ecx, eax
		_emit 0x7e /* jle jmp_10036d9b */
		_emit 0x3d
	jmp_10036d5e:
		mov eax, dword ptr [ebp-0x4]
		sub eax, dword ptr [ebp-0x8]
		_emit 0x79 /* jns jmp_10036d6f */
		_emit 0x09
		add ecx, eax
		_emit 0x7e /* jle jmp_10036d9b */
		_emit 0x31
		sub esi, eax
		sub dword ptr [ebp-0x4], eax
	jmp_10036d6f:
		mov eax, dword ptr [ebp-0x14]
		sub eax, dword ptr [ebp+0xc]
		_emit 0x78 /* js jmp_10036d9b */
		_emit 0x24
		mov eax, dword ptr [ebp+0xc]
		sub eax, dword ptr [ebp-0xc]
		_emit 0x78 /* js jmp_10036d9b */
		_emit 0x1c
		mov eax, dword ptr [ebp+0xc]
		imul dword ptr [ebp-0x1c]
		add eax, dword ptr [ebp-0x18]
		add eax, dword ptr [ebp-0x4]
		mov edi, eax
		mov edx, ecx
		and ecx, 0x3
		rep movsb
		mov ecx, edx
		shr ecx, 0x2
		rep movsd
	jmp_10036d9b:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Scan big-endian IFF chunks for a four-byte tag and return its data pointer.
#ifdef COMPAT_MODE
undefined* FUN_10036dad(const void* p_tag, const void* p_chunks)
{
	STUB(0x10036dad);
	return NULL;
}
#else
// FUNCTION: MW2SHELL 0x10036dad
__declspec(naked) undefined* FUN_10036dad(const void* p_tag, const void* p_chunks)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0xc]
		add esi, 0xc
jmp_10036dbd:
		cmp byte ptr [esi], 0x0
		_emit 0x75 /* jne jmp_10036dc5 */
		_emit 0x03
		inc esi
		_emit 0xeb /* jmp jmp_10036dbd */
		_emit 0xf8
jmp_10036dc5:
		mov ecx, 0x2
		mov edi, dword ptr [ebp+0x8]
		mov eax, esi
		repe cmpsw
		_emit 0x74 /* je jmp_10036de6 */
		_emit 0x12
		mov esi, eax
		add esi, 0x6
		lodsw
		_emit 0x86 /* xchg ah, al: the inline assembler encodes the operands the other way */
		_emit 0xc4
		and eax, 0xffff
		add esi, eax
		_emit 0xeb /* jmp jmp_10036dbd */
		_emit 0xd7
jmp_10036de6:
		add eax, 0x8
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Decodes the BODY chunk of an IFF ILBM or PBM image (p_data) into the view, one row at a
// time through FUN_10036c9e. Returns the BMHD compression byte.
#ifdef COMPAT_MODE
MechS32 FUN_10036def(PixelView* p_view, undefined* p_data)
{
	STUB(0x10036def);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10036def
__declspec(naked) MechS32 FUN_10036def(PixelView* p_view, undefined* p_data)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x3c
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov edi, dword ptr [ebp+0x8]
		mov eax, dword ptr [edi+0xc]
		sub eax, dword ptr [edi+0x4]
		inc eax
		mov dword ptr [ebp-0x24], eax
		mov eax, dword ptr [edi+0x10]
		sub eax, dword ptr [edi+0x8]
		inc eax
		mov dword ptr [ebp-0x20], eax
		mov edi, dword ptr [edi]
		mov edi, dword ptr [edi]
		mov dword ptr [ebp-0x3c], edi
		mov dword ptr [ebp-0x28], 0x0
		mov edi, dword ptr [ebp+0xc]
		mov eax, dword ptr [edi+0x8]
		xor eax, 0x4d424c49
		mov dword ptr [ebp-0x4], eax
		push dword ptr [ebp+0xc]
		push offset g_unk0x10036da1
		call FUN_10036dad
		add esp, 0x8
		mov esi, eax
		lodsw
		_emit 0x86 /* xchg ah, al: the inline assembler encodes the operands the other way */
		_emit 0xc4
		and eax, 0xffff
		mov dword ptr [ebp-0xc], eax
		lodsw
		_emit 0x86 /* xchg ah, al: the inline assembler encodes the operands the other way */
		_emit 0xc4
		and eax, 0xffff
		cmp eax, dword ptr [ebp-0x20]
		_emit 0x7c /* jl jmp_10036e5e */
		_emit 0x03
		mov eax, dword ptr [ebp-0x20]
jmp_10036e5e:
		mov dword ptr [ebp-0x8], eax
		add esi, 0x5
		lodsb
		cmp al, 0x1
		je jmp_10036fad
		mov eax, 0x0
		lodsb
		mov dword ptr [ebp-0x2c], eax
		add esi, 0x2
		mov eax, 0x0
		lodsb
		mov dword ptr [ebp-0x38], eax
		mov eax, dword ptr [ebp-0xc]
		mov ebx, eax
		shr eax, 0x3
		and ebx, 0x7
		cmp ebx, 0x1
		sbb eax, -0x1
		mov ebx, eax
		and eax, 0x1
		add ebx, eax
		mov dword ptr [ebp-0x30], ebx
		mov eax, dword ptr [ebp-0xc]
		and eax, 0x1
		add eax, dword ptr [ebp-0xc]
		mov dword ptr [ebp-0x34], eax
		mov eax, dword ptr [ebp-0xc]
		cmp eax, dword ptr [ebp-0x24]
		_emit 0x7c /* jl jmp_10036eb4 */
		_emit 0x03
		mov eax, dword ptr [ebp-0x24]
jmp_10036eb4:
		mov dword ptr [ebp-0xc], eax
		push dword ptr [ebp+0xc]
		push offset g_unk0x10036da9
		call FUN_10036dad
		add esp, 0x8
		mov dword ptr [ebp-0x10], eax
jmp_10036eca:
		mov esi, dword ptr [ebp-0x10]
		cmp dword ptr [ebp-0x2c], 0x1
		_emit 0x75 /* jne jmp_10036f25 */
		_emit 0x52
		mov edi, offset g_unk0x10069a45
		mov edx, dword ptr [ebp-0x34]
		add edx, edi
jmp_10036edd:
		cmp edi, edx
		_emit 0x73 /* jae jmp_10036f1b */
		_emit 0x3a
		lodsb
		movzx ecx, al
		cmp ecx, 0x80
		_emit 0x74 /* je jmp_10036edd */
		_emit 0xf0
		_emit 0x77 /* ja jmp_10036efe */
		_emit 0x0f
		inc ecx
		push ecx
		and ecx, 0x3
		rep movsb
		pop ecx
		shr ecx, 0x2
		rep movsd
		_emit 0xeb /* jmp jmp_10036edd */
		_emit 0xdf
jmp_10036efe:
		lodsb
		mov ah, al
		mov ebx, eax
		shl eax, 0x10
		mov ax, bx
		neg cl
		inc cl
		push ecx
		and ecx, 0x3
		rep stosb
		pop ecx
		shr ecx, 0x2
		rep stosd
		_emit 0xeb /* jmp jmp_10036edd */
		_emit 0xc2
jmp_10036f1b:
		mov dword ptr [ebp-0x10], esi
		mov esi, offset g_unk0x10069a45
		_emit 0xeb /* jmp jmp_10036f2d */
		_emit 0x08
jmp_10036f25:
		mov eax, esi
		add eax, dword ptr [ebp-0x34]
		mov dword ptr [ebp-0x10], eax
jmp_10036f2d:
		cmp dword ptr [ebp-0x4], 0x0
		_emit 0x75 /* jne jmp_10036f8f */
		_emit 0x5c
		mov edi, offset g_unk0x10068d45
		mov eax, dword ptr [ebp-0x30]
		mov dword ptr [ebp-0x18], eax
		mov dword ptr [ebp-0x1c], eax
		mov eax, dword ptr [ebp-0xc]
		mov dword ptr [ebp-0x14], eax
jmp_10036f47:
		mov edx, 0x80
jmp_10036f4c:
		mov ebx, 0x0
		mov eax, 0x100
jmp_10036f56:
		movzx ecx, byte ptr [esi+ebx]
		and ecx, edx
		_emit 0x74 /* je jmp_10036f60 */
		_emit 0x02
		or al, ah
jmp_10036f60:
		add ebx, dword ptr [ebp-0x1c]
		shl ah, 0x1
		_emit 0x75 /* jne jmp_10036f56 */
		_emit 0xef
		stosb
		dec dword ptr [ebp-0x14]
		_emit 0x74 /* je jmp_10036f77 */
		_emit 0x0a
		shr dl, 0x1
		_emit 0x75 /* jne jmp_10036f4c */
		_emit 0xdb
		inc esi
		dec dword ptr [ebp-0x18]
		_emit 0x75 /* jne jmp_10036f47 */
		_emit 0xd0
jmp_10036f77:
		push dword ptr [ebp-0xc]
		push offset g_unk0x10068d45
		push dword ptr [ebp-0x28]
		push dword ptr [ebp+0x8]
		call FUN_10036c9e
		add esp, 0x10
		_emit 0xeb /* jmp jmp_10036fa1 */
		_emit 0x12
jmp_10036f8f:
		push dword ptr [ebp-0xc]
		push esi
		push dword ptr [ebp-0x28]
		push dword ptr [ebp+0x8]
		call FUN_10036c9e
		add esp, 0x10
jmp_10036fa1:
		inc dword ptr [ebp-0x28]
		dec dword ptr [ebp-0x8]
		jne jmp_10036eca
jmp_10036fad:
		mov eax, dword ptr [ebp-0x38]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Copies the IFF image's CMAP chunk into p_palette as 6-bit components.
#ifdef COMPAT_MODE
void FUN_10036fb6(undefined* p_data, MechU8* p_palette)
{
	STUB(0x10036fb6);
}
#else
// FUNCTION: MW2SHELL 0x10036fb6
__declspec(naked) void FUN_10036fb6(undefined* p_data, MechU8* p_palette)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		push dword ptr [ebp+0x8]
		push offset g_unk0x10036da5
		call FUN_10036dad
		add esp, 0x8
		mov esi, eax
		mov edi, dword ptr [ebp+0xc]
		mov ecx, 0x300
jmp_10036fda:
		lodsb
		shr al, 0x2
		stosb
		loop jmp_10036fda
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the IFF image's BMHD width and height, packed as width << 16 | height.
#ifdef COMPAT_MODE
MechU32 FUN_10036fe7(undefined* p_data)
{
	STUB(0x10036fe7);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10036fe7
__declspec(naked) MechU32 FUN_10036fe7(undefined* p_data)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		push dword ptr [ebp+0x8]
		push offset g_unk0x10036da1
		call FUN_10036dad
		add esp, 0x8
		mov esi, eax
		lodsw
		_emit 0x86 /* xchg ah, al: the inline assembler encodes the operands the other way */
		_emit 0xc4
		shl eax, 0x10
		lodsw
		_emit 0x86 /* xchg ah, al: the inline assembler encodes the operands the other way */
		_emit 0xc4
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_10037014(PixelView* p_view, undefined* p_data)
{
	STUB(0x10037014);
}
#else
// FUNCTION: MW2SHELL 0x10037014
__declspec(naked) void FUN_10037014(PixelView* p_view, undefined* p_data)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0xc
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov dword ptr [ebp-0xc], 0x0
		mov edi, dword ptr [ebp+0x8]
		mov edi, dword ptr [edi]
		mov edi, dword ptr [edi]
		mov esi, dword ptr [ebp+0xc]
		movzx ebx, word ptr [esi+0xa]
		sub bx, word ptr [esi+0x6]
		mov dword ptr [ebp-0x8], ebx
		mov ebx, 0x0
		movzx eax, word ptr [esi+0x42]
		mov dword ptr [ebp-0x4], eax
		add esi, 0x80
jmp_1003704f:
		mov edi, offset g_unk0x10068d45
		mov edx, edi
		add edx, dword ptr [ebp-0x4]
jmp_10037059:
		lodsb
		mov ah, al
		and ah, 0xc0
		xor ah, 0xc0
		_emit 0x75 /* jnz jmp_1003706e */
		_emit 0x0a
		and eax, 0x3f
		mov ecx, eax
		lodsb
		rep stosb
		_emit 0xeb /* jmp jmp_1003706f */
		_emit 0x01
jmp_1003706e:
		stosb
jmp_1003706f:
		cmp edi, edx
		_emit 0x7c /* jl jmp_10037059 */
		_emit 0xe6
		push dword ptr [ebp-0x4]
		push offset g_unk0x10068d45
		push ebx
		push dword ptr [ebp+0x8]
		call FUN_10036c9e
		add esp, 0x10
		inc ebx
		cmp ebx, dword ptr [ebp-0x8]
		_emit 0x7e /* jle jmp_1003704f */
		_emit 0xc2
		mov eax, dword ptr [ebp-0xc]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_10037096(undefined* p_data, MechS32 p_size, PaletteColor* p_palette)
{
	STUB(0x10037096);
}
#else
// FUNCTION: MW2SHELL 0x10037096
__declspec(naked) void FUN_10037096(undefined* p_data, MechS32 p_size, PaletteColor* p_palette)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		add esi, dword ptr [ebp+0xc]
		sub esi, 0x300
		mov edi, dword ptr [ebp+0x10]
		mov ecx, 0x300
jmp_100370b4:
		lodsb
		shr al, 0x2
		stosb
		loop jmp_100370b4
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

#ifdef COMPAT_MODE
MechS32 FUN_100370c1(undefined* p_data)
{
	STUB(0x100370c1);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100370c1
__declspec(naked) MechS32 FUN_100370c1(undefined* p_data)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov ax, word ptr [esi+0x8]
		sub ax, word ptr [esi+0x4]
		inc ax
		shl eax, 0x10
		mov ax, word ptr [esi+0xa]
		sub ax, word ptr [esi+0x6]
		inc ax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Internal assembly helper: initializes the code tables using ECX and EDI.
#ifdef COMPAT_MODE
void FUN_100370e8(void)
{
	STUB(0x100370e8);
}
#else
// FUNCTION: MW2SHELL 0x100370e8
__declspec(naked) void FUN_100370e8(void)
{
	__asm {
		mov ebx, 0x0
		mov eax, ecx
		add eax, 0x2
		mov dword ptr [edi], eax
		mov eax, ecx
		shl eax, 0x1
		mov dword ptr [edi+0x4], eax
	jmp_100370fb:
		cmp ebx, ecx
		_emit 0x7d /* jge jmp_1003711a */
		_emit 0x1b
		mov byte ptr [ebx+edi+0x102e], bl
		mov byte ptr [ebx+edi+0x202e], bl
		mov word ptr [edi+ebx*2+0x302e], 0xffff
		inc ebx
		_emit 0xeb /* jmp jmp_100370fb */
		_emit 0xe1
	jmp_1003711a:
		cmp ebx, 0x1000
		_emit 0x7d /* jge jmp_1003712f */
		_emit 0x0d
		mov word ptr [edi+ebx*2+0x302e], 0xfffe
		inc ebx
		_emit 0xeb /* jmp jmp_1003711a */
		_emit 0xeb
	jmp_1003712f:
		ret
	}
}
#endif

// Internal assembly helper: reads a byte from ESI, tracking the run in EDI.
#ifdef COMPAT_MODE
MechU32 FUN_10037130(void)
{
	STUB(0x10037130);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10037130
__declspec(naked) MechU32 FUN_10037130(void)
{
	__asm {
		cmp dword ptr [edi+0x10], 0x0
		_emit 0x75 /* jnz jmp_1003713f */
		_emit 0x09
		lodsb
		and eax, 0xff
		mov dword ptr [edi+0x10], eax
	jmp_1003713f:
		lodsb
		and eax, 0xff
		dec dword ptr [edi+0x10]
		ret
	}
}
#endif

// Internal assembly helper: returns the next EDX bits of the LZW code stream in EDI.
#ifdef COMPAT_MODE
MechU32 FUN_10037149(void)
{
	STUB(0x10037149);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10037149
__declspec(naked) MechU32 FUN_10037149(void)
{
	__asm {
		cmp dword ptr [edi+0x18], 0x0
		_emit 0x75 /* jnz jmp_1003715e */
		_emit 0x0f
		call FUN_10037130
		mov dword ptr [edi+0x14], eax
		mov dword ptr [edi+0x18], 0x8
	jmp_1003715e:
		mov eax, edx
		cmp dword ptr [edi+0x18], eax
		_emit 0x7d /* jge jmp_10037176 */
		_emit 0x11
		call FUN_10037130
		mov ecx, dword ptr [edi+0x18]
		shl eax, cl
		or dword ptr [edi+0x14], eax
		add dword ptr [edi+0x18], 0x8
	jmp_10037176:
		mov ebx, edx
		movzx eax, byte ptr [g_unk0x1006a045+ebx]
		mov ebx, dword ptr [edi+0x14]
		and ebx, eax
		push ebx
		sub dword ptr [edi+0x18], edx
		mov ecx, edx
		shr dword ptr [edi+0x14], cl
		pop eax
		ret
	}
}
#endif

// Internal assembly helper: installs a code and grows the code table when full.
#ifdef COMPAT_MODE
void FUN_1003718f(void)
{
	STUB(0x1003718f);
}
#else
// FUNCTION: MW2SHELL 0x1003718f
__declspec(naked) void FUN_1003718f(void)
{
	__asm {
		push ebx
		mov ebx, dword ptr [edi]
		mov word ptr [edi+ebx*2+0x302e], cx
		pop ebx
		push ebx
		mov al, byte ptr [ebx+edi+0x102e]
		mov ebx, dword ptr [edi]
		mov byte ptr [ebx+edi+0x202e], al
		mov ebx, ecx
		mov al, byte ptr [ebx+edi+0x102e]
		mov ebx, dword ptr [edi]
		mov byte ptr [ebx+edi+0x102e], al
		pop ebx
		inc dword ptr [edi]
		mov eax, dword ptr [edi]
		cmp eax, dword ptr [edi+0x4]
		_emit 0x75 /* jnz jmp_100371d4 */
		_emit 0x0c
		cmp dword ptr [edi+0x1c], 0xc
		_emit 0x7d /* jge jmp_100371d4 */
		_emit 0x06
		inc dword ptr [edi+0x1c]
		shl dword ptr [edi+0x4], 0x1
	jmp_100371d4:
		ret
	}
}
#endif

// Internal assembly helper: appends the pixel in AL to the row buffer and, when the row is
// full, draws it and moves to the next row (in GIF interlace order when enabled).
#ifdef COMPAT_MODE
void FUN_100371d5(void)
{
	STUB(0x100371d5);
}
#else
// FUNCTION: MW2SHELL 0x100371d5
__declspec(naked) void FUN_100371d5(void)
{
	__asm {
		mov ebx, dword ptr [edi+0x8]
		mov byte ptr [g_unk0x10068d45+ebx], al
		inc dword ptr [edi+0x8]
		dec dword ptr [edi+0x20]
		cmp dword ptr [edi+0x20], 0x0
		_emit 0x75 /* jnz jmp_10037251 */
		_emit 0x67
		push dword ptr [edi+0x24]
		push offset g_unk0x10068d45
		push dword ptr [edi+0xc]
		push dword ptr [g_unk0x1006a058]
		call FUN_10036c9e
		add esp, 0x10
		mov dword ptr [edi+0x8], 0x0
		mov eax, dword ptr [edi+0x24]
		mov dword ptr [edi+0x20], eax
		cmp byte ptr [edi+0x2c], 0x0
		_emit 0x74 /* jz jmp_1003723f */
		_emit 0x29
		movzx ebx, byte ptr [edi+0x2d]
		movzx eax, byte ptr [g_unk0x1006a04e+ebx]
		add dword ptr [edi+0xc], eax
		mov eax, dword ptr [edi+0xc]
		cmp eax, dword ptr [edi+0x28]
		_emit 0x7c /* jl jmp_1003723d */
		_emit 0x11
		inc byte ptr [edi+0x2d]
		movzx ebx, byte ptr [edi+0x2d]
		movzx eax, byte ptr [g_unk0x1006a053+ebx]
		mov dword ptr [edi+0xc], eax
	jmp_1003723d:
		_emit 0xeb /* jmp jmp_10037251 */
		_emit 0x12
	jmp_1003723f:
		inc dword ptr [edi+0xc]
		mov eax, dword ptr [edi+0xc]
		cmp eax, dword ptr [edi+0x28]
		_emit 0x7c /* jl jmp_10037251 */
		_emit 0x07
		mov dword ptr [edi+0xc], 0x0
	jmp_10037251:
		ret
	}
}
#endif

// Decodes the LZW image data of a GIF into the view, using p_state as the decoder state.
// Returns the background color index.
#ifdef COMPAT_MODE
MechS32 FUN_10037252(PixelView* p_view, undefined* p_data, undefined* p_state)
{
	STUB(0x10037252);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10037252
__declspec(naked) MechS32 FUN_10037252(PixelView* p_view, undefined* p_data, undefined* p_state)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x18
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov edi, dword ptr [ebp+0x8]
		mov dword ptr [g_unk0x1006a058], edi
		mov edi, dword ptr [ebp+0x10]
		mov eax, 0x0
		mov ecx, 0x2e
		mov edx, ecx
		and ecx, 0x3
		rep stosb
		mov ecx, edx
		shr ecx, 0x2
		rep stosd
		mov edi, dword ptr [ebp+0x10]
		mov esi, dword ptr [ebp+0xc]
		movzx eax, byte ptr [esi+0xb]
		mov dword ptr [ebp-0x18], eax
		mov al, byte ptr [esi+0xa]
		mov cl, al
		and cl, 0x7
		inc cl
		mov ebx, 0x1
		shl ebx, cl
		add esi, 0xd
		test al, 0x80
		_emit 0x74 /* jz jmp_100372ad */
		_emit 0x05
		imul ebx, ebx, 0x3
		add esi, ebx
	jmp_100372ad:
		movzx eax, word ptr [esi+0x5]
		mov dword ptr [edi+0x24], eax
		movzx eax, word ptr [esi+0x7]
		mov dword ptr [edi+0x28], eax
		movzx eax, byte ptr [esi+0x9]
		mov byte ptr [edi+0x2c], al
		and byte ptr [edi+0x2c], 0x40
		add esi, 0xa
		test al, 0x80
		_emit 0x74 /* jz jmp_100372e0 */
		_emit 0x13
		mov cl, al
		and cl, 0x7
		inc cl
		mov ebx, 0x1
		shl ebx, cl
		imul ebx, ebx, 0x3
		add esi, ebx
	jmp_100372e0:
		mov dword ptr [edi+0x10], 0x0
		lodsb
		movzx ecx, al
		mov edx, 0x8
		push ecx
		push edx
		mov eax, 0x1
		shl eax, cl
		mov dword ptr [ebp-0x4], eax
		inc eax
		mov dword ptr [ebp-0x8], eax
		inc ecx
		mov dword ptr [edi+0x1c], ecx
		mov ecx, dword ptr [ebp-0x4]
		call FUN_100370e8
		mov dword ptr [ebp-0x10], 0xffff
		mov dword ptr [ebp-0x14], 0x0
		mov byte ptr [edi+0x2d], 0x0
		mov eax, dword ptr [edi+0x24]
		mov dword ptr [edi+0x20], eax
		mov dword ptr [edi+0x8], 0x0
		mov dword ptr [edi+0xc], 0x0
		pop edx
		pop ecx
	jmp_10037334:
		push ecx
		push edx
		mov edx, dword ptr [edi+0x1c]
		cmp edx, 0x8
		_emit 0x7f /* jg jmp_10037347 */
		_emit 0x09
		push edx
		call FUN_10037149
		pop edx
		_emit 0xeb /* jmp jmp_10037364 */
		_emit 0x1d
	jmp_10037347:
		push edx
		mov edx, 0x8
		call FUN_10037149
		pop edx
		push eax
		push edx
		sub edx, 0x8
		call FUN_10037149
		pop edx
		shl eax, 0x8
		pop ebx
		or eax, ebx
	jmp_10037364:
		mov dword ptr [ebp-0xc], eax
		pop edx
		pop ecx
		cmp eax, dword ptr [ebp-0x4]
		_emit 0x75 /* jnz jmp_1003738c */
		_emit 0x1e
		push ecx
		push edx
		mov ecx, dword ptr [ebp-0x4]
		call FUN_100370e8
		pop edx
		pop ecx
		mov eax, ecx
		inc eax
		mov dword ptr [edi+0x1c], eax
		mov dword ptr [ebp-0x10], 0xffff
		jmp jmp_10037457
	jmp_1003738c:
		cmp eax, dword ptr [ebp-0x8]
		_emit 0x75 /* jnz jmp_100373b8 */
		_emit 0x27
	jmp_10037391:
		cmp dword ptr [edi+0x10], 0x0
		_emit 0x74 /* jz jmp_1003739d */
		_emit 0x06
		lodsb
		dec dword ptr [edi+0x10]
		_emit 0xeb /* jmp jmp_10037391 */
		_emit 0xf4
	jmp_1003739d:
		lodsb
		and eax, 0xff
		mov dword ptr [edi+0x10], eax
		cmp dword ptr [edi+0x10], 0x0
		_emit 0x75 /* jnz jmp_10037391 */
		_emit 0xe5
		mov dword ptr [ebp-0x14], 0xffff
		jmp jmp_10037457
	jmp_100373b8:
		mov ebx, dword ptr [ebp-0xc]
		cmp word ptr [edi+ebx*2+0x302e], -0x2
		_emit 0x74 /* jz jmp_100373dd */
		_emit 0x17
		cmp dword ptr [ebp-0x10], 0xffff
		_emit 0x74 /* jz jmp_100373db */
		_emit 0x0c
		push ecx
		push edx
		mov ecx, dword ptr [ebp-0x10]
		call FUN_1003718f
		pop edx
		pop ecx
	jmp_100373db:
		_emit 0xeb /* jmp jmp_100373ec */
		_emit 0x0f
	jmp_100373dd:
		push ecx
		push edx
		mov ebx, dword ptr [ebp-0x10]
		mov ecx, dword ptr [ebp-0x10]
		call FUN_1003718f
		pop edx
		pop ecx
	jmp_100373ec:
		push ecx
		push edx
		mov ebx, dword ptr [ebp-0xc]
		push esi
		mov ecx, 0x0
		mov esi, edi
		add esi, 0x2e
	jmp_100373fc:
		mov al, byte ptr [ebx+edi+0x202e]
		mov byte ptr [esi], al
		inc esi
		inc ecx
		movzx ebx, word ptr [edi+ebx*2+0x302e]
		cmp ebx, 0xffff
		_emit 0x75 /* jnz jmp_100373fc */
		_emit 0xe5
		cmp edx, 0x1
		_emit 0x75 /* jnz jmp_1003743d */
		_emit 0x21
	jmp_1003741c:
		dec esi
		mov al, byte ptr [esi]
		and eax, 0x1
		push ecx
		call FUN_100371d5
		pop ecx
		mov al, byte ptr [esi]
		and eax, 0xff
		shr eax, 0x1
		push ecx
		call FUN_100371d5
		pop ecx
		loop jmp_1003741c
		_emit 0xeb /* jmp jmp_1003744e */
		_emit 0x11
	jmp_1003743d:
		dec esi
		mov al, byte ptr [esi]
		and eax, 0xff
		push ecx
		call FUN_100371d5
		pop ecx
		loop jmp_1003743d
	jmp_1003744e:
		pop esi
		pop edx
		pop ecx
		mov eax, dword ptr [ebp-0xc]
		mov dword ptr [ebp-0x10], eax
	jmp_10037457:
		cmp dword ptr [ebp-0x14], 0x0
		_emit 0x75 /* jnz jmp_10037462 */
		_emit 0x05
		jmp jmp_10037334
	jmp_10037462:
		mov eax, dword ptr [ebp-0x18]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Copies the GIF's global color table, and then its local one, into p_palette as 6-bit
// components.
#ifdef COMPAT_MODE
void FUN_1003746b(undefined* p_data, undefined* p_palette)
{
	STUB(0x1003746b);
}
#else
// FUNCTION: MW2SHELL 0x1003746b
__declspec(naked) void FUN_1003746b(undefined* p_data, undefined* p_palette)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		mov al, byte ptr [esi+0xa]
		add esi, 0xd
		test al, 0x80
		_emit 0x74 /* jz jmp_1003749f */
		_emit 0x1d
		mov cl, al
		and cl, 0x7
		inc cl
		mov ebx, 0x1
		shl ebx, cl
		imul ebx, ebx, 0x3
		mov edi, dword ptr [ebp+0xc]
		mov ecx, ebx
	jmp_10037498:
		lodsb
		shr al, 0x2
		stosb
		loop jmp_10037498
	jmp_1003749f:
		mov al, byte ptr [esi+0x9]
		test al, 0x80
		_emit 0x74 /* jz jmp_100374c6 */
		_emit 0x20
		mov cl, al
		and cl, 0x7
		inc cl
		mov ebx, 0x1
		shl ebx, cl
		imul ebx, ebx, 0x3
		add esi, 0xa
		mov edi, dword ptr [ebp+0xc]
		mov ecx, ebx
	jmp_100374bf:
		lodsb
		shr al, 0x2
		stosb
		loop jmp_100374bf
	jmp_100374c6:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the two dimensions packed into a dword from the image header.
#ifdef COMPAT_MODE
MechU32 FUN_100374cc(undefined* p_data)
{
	STUB(0x100374cc);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100374cc
__declspec(naked) MechU32 FUN_100374cc(undefined* p_data)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov al, byte ptr [esi+0xa]
		mov cl, al
		and cl, 0x7
		inc cl
		mov ebx, 0x1
		shl ebx, cl
		add esi, 0xd
		test al, 0x80
		_emit 0x74 /* jz jmp_100374f3 */
		_emit 0x05
		imul ebx, ebx, 0x3
		add esi, ebx
	jmp_100374f3:
		mov ax, word ptr [esi+0x5]
		shl eax, 0x10
		mov ax, word ptr [esi+0x7]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the first dword in an entry selected from the offset table at data + 8.
#ifdef COMPAT_MODE
MechS32 FUN_10037504(void* p_data, MechS32 p_index)
{
	STUB(0x10037504);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10037504
__declspec(naked) MechS32 FUN_10037504(void* p_data, MechS32 p_index)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		add esi, 0x8
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		add esi, eax
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the second dword in an entry selected from the offset table at data + 8.
#ifdef COMPAT_MODE
MechS32 FUN_10037526(void* p_data, MechS32 p_index)
{
	STUB(0x10037526);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10037526
__declspec(naked) MechS32 FUN_10037526(void* p_data, MechS32 p_index)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		add esi, 0x8
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		add esi, eax
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0x4]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the entry's inclusive horizontal and vertical spans packed into a dword.
#ifdef COMPAT_MODE
MechU32 FUN_10037549(void* p_data, MechS32 p_index)
{
	STUB(0x10037549);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10037549
__declspec(naked) MechU32 FUN_10037549(void* p_data, MechS32 p_index)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		add esi, 0x8
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		add esi, eax
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0x10]
		sub eax, dword ptr [esi+0x8]
		inc eax
		mov ebx, dword ptr [esi+0x14]
		sub ebx, dword ptr [esi+0xc]
		inc ebx
		shl eax, 0x10
		mov ax, bx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the entry's two coordinate fields at +8 and +c packed into a dword.
#ifdef COMPAT_MODE
MechU32 FUN_1003757d(void* p_data, MechS32 p_index)
{
	STUB(0x1003757d);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x1003757d
__declspec(naked) MechU32 FUN_1003757d(void* p_data, MechS32 p_index)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		add esi, 0x8
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		add esi, eax
		mov esi, dword ptr [esi]
		add esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0x8]
		shl eax, 0x10
		mov ax, word ptr [esi+0xc]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Applies a list of three-byte palette updates from an entry.
#ifdef COMPAT_MODE
void FUN_100375a7(undefined* p_data, MechS32 p_index, undefined* p_palette)
{
	STUB(0x100375a7);
}
#else
// FUNCTION: MW2SHELL 0x100375a7
__declspec(naked) void FUN_100375a7(undefined* p_data, MechS32 p_index, undefined* p_palette)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		add esi, 0x8
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		add esi, eax
		add esi, 0x4
		mov esi, dword ptr [esi]
		cmp esi, 0x0
		_emit 0x74 /* jz jmp_100375ec */
		_emit 0x23
		add esi, dword ptr [ebp+0x8]
		lodsd
		mov ecx, eax
		mov edi, dword ptr [ebp+0x10]
	jmp_100375d2:
		lodsb
		and eax, 0xff
		mov ebx, eax
		shl ebx, 0x1
		add ebx, eax
		lodsb
		mov byte ptr [ebx+edi], al
		inc ebx
		lodsw
		mov word ptr [ebx+edi], ax
		dec ecx
		_emit 0x75 /* jnz jmp_100375d2 */
		_emit 0xe6
	jmp_100375ec:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Copies the entry's dword array into the supplied destination, if present.
#ifdef COMPAT_MODE
MechS32 FUN_100375f2(undefined* p_data, MechS32 p_index, undefined4* p_destination)
{
	STUB(0x100375f2);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100375f2
__declspec(naked) MechS32 FUN_100375f2(undefined* p_data, MechS32 p_index, undefined4* p_destination)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		add esi, 0x8
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		add esi, eax
		add esi, 0x4
		mov esi, dword ptr [esi]
		cmp esi, 0x0
		_emit 0x75 /* jnz jmp_1003761b */
		_emit 0x07
		mov eax, 0x0
		_emit 0xeb /* jmp jmp_10037634 */
		_emit 0x19
	jmp_1003761b:
		add esi, dword ptr [ebp+0x8]
		lodsd
		mov ebx, eax
		mov edi, dword ptr [ebp+0x10]
		or edi, edi
		_emit 0x74 /* jz jmp_10037634 */
		_emit 0x0c
		mov ecx, ebx
		mov eax, 0x0
	jmp_1003762f:
		movsd
		loop jmp_1003762f
		mov eax, ebx
	jmp_10037634:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Writes the entry's dword array back from the supplied source, if present.
#ifdef COMPAT_MODE
MechS32 FUN_1003763a(undefined* p_data, MechS32 p_index, undefined4* p_source)
{
	STUB(0x1003763a);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x1003763a
__declspec(naked) MechS32 FUN_1003763a(undefined* p_data, MechS32 p_index, undefined4* p_source)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov esi, dword ptr [ebp+0x8]
		add esi, 0x8
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		add esi, eax
		add esi, 0x4
		mov esi, dword ptr [esi]
		cmp esi, 0x0
		_emit 0x75 /* jnz jmp_10037663 */
		_emit 0x07
		mov eax, 0x0
		_emit 0xeb /* jmp jmp_1003767e */
		_emit 0x1b
	jmp_10037663:
		add esi, dword ptr [ebp+0x8]
		lodsd
		mov ebx, eax
		mov edi, esi
		mov esi, dword ptr [ebp+0x10]
		or esi, esi
		_emit 0x74 /* jz jmp_1003767e */
		_emit 0x0c
		mov ecx, ebx
		mov eax, 0x0
	jmp_10037679:
		movsd
		loop jmp_10037679
		mov eax, ebx
	jmp_1003767e:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Returns the dword at +4 in the data header.
#ifdef COMPAT_MODE
MechS32 FUN_10037684(void* p_data)
{
	STUB(0x10037684);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10037684
__declspec(naked) MechS32 FUN_10037684(void* p_data)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0x4]
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Counts the distinct entries of the data's offset table, storing the index of each first
// occurrence in p_indices when given.
#ifdef COMPAT_MODE
MechS32 FUN_10037697(void* p_data, MechS32* p_indices)
{
	STUB(0x10037697);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10037697
__declspec(naked) MechS32 FUN_10037697(void* p_data, MechS32* p_indices)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x4
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov ecx, dword ptr [esi+0x4]
		dec ecx
		add esi, 0x8
		mov dword ptr [ebp-0x4], esi
		mov ebx, 0x1
		mov edx, dword ptr [ebp+0xc]
		cmp edx, 0x0
		_emit 0x74 /* jz jmp_100376c4 */
		_emit 0x09
		mov dword ptr [edx], 0x0
		add edx, 0x4
	jmp_100376c4:
		cmp ecx, 0x0
		_emit 0x74 /* jz jmp_100376f1 */
		_emit 0x28
	jmp_100376c9:
		add esi, 0x8
		mov eax, dword ptr [esi]
		mov edi, dword ptr [ebp-0x4]
	jmp_100376d1:
		cmp eax, dword ptr [edi]
		_emit 0x74 /* jz jmp_100376ef */
		_emit 0x1a
		add edi, 0x8
		cmp edi, esi
		_emit 0x7c /* jl jmp_100376d1 */
		_emit 0xf5
		cmp edx, 0x0
		_emit 0x74 /* jz jmp_100376ee */
		_emit 0x0d
		mov eax, esi
		sub eax, dword ptr [ebp-0x4]
		shr eax, 0x3
		mov dword ptr [edx], eax
		add edx, 0x4
	jmp_100376ee:
		inc ebx
	jmp_100376ef:
		loop jmp_100376c9
	jmp_100376f1:
		mov eax, ebx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Counts the distinct first dwords among the data's 8-byte entries (the count at +4, the
// entries from +0xc). When p_out isn't NULL, stores the index of each entry that starts a new
// value there. Returns the count.
#ifdef COMPAT_MODE
MechS32 FUN_100376f9(undefined* p_data, MechU32* p_out)
{
	STUB(0x100376f9);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100376f9
__declspec(naked) MechS32 FUN_100376f9(undefined* p_data, MechU32* p_out)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x4
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov ecx, dword ptr [esi+0x4]
		dec ecx
		add esi, 0xc
		mov dword ptr [ebp-0x4], esi
		mov ebx, 0x1
		mov edx, dword ptr [ebp+0xc]
		cmp edx, 0x0
		_emit 0x74 /* je jmp_10037726 */
		_emit 0x09
		mov dword ptr [edx], 0x0
		add edx, 0x4
jmp_10037726:
		cmp ecx, 0x0
		_emit 0x74 /* je jmp_10037753 */
		_emit 0x28
jmp_1003772b:
		add esi, 0x8
		mov eax, dword ptr [esi]
		mov edi, dword ptr [ebp-0x4]
jmp_10037733:
		cmp eax, dword ptr [edi]
		_emit 0x74 /* je jmp_10037751 */
		_emit 0x1a
		add edi, 0x8
		cmp edi, esi
		_emit 0x7c /* jl jmp_10037733 */
		_emit 0xf5
		cmp edx, 0x0
		_emit 0x74 /* je jmp_10037750 */
		_emit 0x0d
		mov eax, esi
		sub eax, dword ptr [ebp-0x4]
		shr eax, 0x3
		mov dword ptr [edx], eax
		add edx, 0x4
jmp_10037750:
		inc ebx
jmp_10037751:
		loop jmp_1003772b
jmp_10037753:
		mov eax, ebx
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// The tap masks of maximal-length LFSRs, for 2 to 32 bits. The original keeps the table in
// .text, between FUN_100376f9 and FUN_100377d7.
// GLOBAL: MW2SHELL 0x1003775b
MechU32 g_unk0x1003775b[0x1f] = {0x3,        0x6,        0xc,       0x14,      0x30,      0x60,      0xb8,
								 0x110,      0x240,      0x500,     0xca0,     0x1b00,    0x3500,    0x6000,
								 0xb400,     0x12000,    0x20400,   0x72000,   0x90000,   0x140000,  0x300000,
								 0x420000,   0xd80000,   0x1200000, 0x3880000, 0x7200000, 0x9000000, 0x14000000,
								 0x32800000, 0x48000000, 0xa3000000};

// Dissolves p_src into p_dest: copies up to p_count pixels, in the order of a maximal-length
// LFSR over the pixel indices (taps from g_unk0x1003775b), starting from p_state (0 starts a
// new dissolve). Returns the LFSR state to continue from.
// Not 100%: the tap lookup indexes from g_unk0x1003775b - 8 (the bit count starts at 2). In the
// original that address is inside FUN_100376f9 and has no symbol; here it falls in whichever
// global precedes the table, so reccmp names the two sides differently.
#ifdef COMPAT_MODE
MechU32 FUN_100377d7(PixelView* p_src, PixelView* p_dest, MechS32 p_count, MechU32 p_state)
{
	STUB(0x100377d7);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100377d7
__declspec(naked) MechU32 FUN_100377d7(PixelView* p_src, PixelView* p_dest, MechS32 p_count, MechU32 p_state)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x70
		push es
		push ebx
		push esi
		push edi
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x44], eax
		_emit 0x7e /* jle jmp_10037853 */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10037853 */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x4c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_10037807 */
		_emit 0x05
		mov eax, 0x0
jmp_10037807:
		mov dword ptr [ebp-0x34], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x50], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_1003781a */
		_emit 0x05
		mov eax, 0x0
jmp_1003781a:
		mov dword ptr [ebp-0x38], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x44]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_1003782a */
		_emit 0x02
		mov eax, edx
jmp_1003782a:
		mov dword ptr [ebp-0x3c], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_10037839 */
		_emit 0x02
		mov eax, edx
jmp_10037839:
		mov dword ptr [ebp-0x40], eax
		mov eax, dword ptr [ebp-0x3c]
		cmp eax, dword ptr [ebp-0x34]
		_emit 0x7c /* jl jmp_1003785e */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x40]
		cmp eax, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_1003785e */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x48], eax
		_emit 0xeb /* jmp jmp_10037869 */
		_emit 0x16
jmp_10037853:
		mov eax, 0xffffffff
		pop edi
		pop esi
		pop ebx
		pop es
		leave
		ret
jmp_1003785e:
		mov eax, 0xfffffffe
		pop edi
		pop esi
		pop ebx
		pop es
		leave
		ret
jmp_10037869:
		mov esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0xc]
		sub eax, dword ptr [esi+0x4]
		inc eax
		mov dword ptr [ebp-0x24], eax
		mov eax, dword ptr [esi+0x10]
		sub eax, dword ptr [esi+0x8]
		inc eax
		mov dword ptr [ebp-0x28], eax
		mov eax, dword ptr [ebp-0x50]
		imul dword ptr [ebp-0x44]
		add eax, dword ptr [ebp-0x48]
		add eax, dword ptr [ebp-0x4c]
		mov edi, eax
		xor ebx, ebx
		mov ecx, dword ptr [ebp-0x28]
jmp_10037893:
		mov dword ptr [g_unk0x10068845+ebx*0x4], edi
		inc ebx
		add edi, dword ptr [ebp-0x44]
		loop jmp_10037893
		mov esi, dword ptr [ebp+0xc]
		mov ebx, dword ptr [esi]
		mov eax, dword ptr [ebx+0x4]
		inc eax
		mov dword ptr [ebp-0x68], eax
		_emit 0x7e /* jle jmp_10037912 */
		_emit 0x64
		mov eax, dword ptr [ebx+0x8]
		inc eax
		mov ecx, eax
		_emit 0x7e /* jle jmp_10037912 */
		_emit 0x5c
		mov eax, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x6c], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_100378c6 */
		_emit 0x05
		mov eax, 0x0
jmp_100378c6:
		mov dword ptr [ebp-0x54], eax
		mov eax, dword ptr [esi+0x8]
		mov dword ptr [ebp-0x70], eax
		cmp eax, 0x0
		_emit 0x7f /* jg jmp_100378d9 */
		_emit 0x05
		mov eax, 0x0
jmp_100378d9:
		mov dword ptr [ebp-0x58], eax
		mov eax, dword ptr [esi+0xc]
		mov edx, dword ptr [ebp-0x68]
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100378e9 */
		_emit 0x02
		mov eax, edx
jmp_100378e9:
		mov dword ptr [ebp-0x5c], eax
		mov eax, dword ptr [esi+0x10]
		mov edx, ecx
		dec edx
		cmp eax, edx
		_emit 0x7c /* jl jmp_100378f8 */
		_emit 0x02
		mov eax, edx
jmp_100378f8:
		mov dword ptr [ebp-0x60], eax
		mov eax, dword ptr [ebp-0x5c]
		cmp eax, dword ptr [ebp-0x54]
		_emit 0x7c /* jl jmp_1003791d */
		_emit 0x1a
		mov eax, dword ptr [ebp-0x60]
		cmp eax, dword ptr [ebp-0x58]
		_emit 0x7c /* jl jmp_1003791d */
		_emit 0x12
		mov eax, dword ptr [ebx]
		mov dword ptr [ebp-0x64], eax
		_emit 0xeb /* jmp jmp_10037928 */
		_emit 0x16
jmp_10037912:
		mov eax, 0xffffffff
		pop edi
		pop esi
		pop ebx
		pop es
		leave
		ret
jmp_1003791d:
		mov eax, 0xfffffffe
		pop edi
		pop esi
		pop ebx
		pop es
		leave
		ret
jmp_10037928:
		mov esi, dword ptr [ebp+0xc]
		mov eax, dword ptr [esi+0xc]
		sub eax, dword ptr [esi+0x4]
		inc eax
		mov dword ptr [ebp-0x2c], eax
		mov eax, dword ptr [esi+0x10]
		sub eax, dword ptr [esi+0x8]
		inc eax
		mov dword ptr [ebp-0x30], eax
		mov eax, dword ptr [ebp-0x70]
		imul dword ptr [ebp-0x68]
		add eax, dword ptr [ebp-0x64]
		add eax, dword ptr [ebp-0x6c]
		mov edi, eax
		xor ebx, ebx
		mov ecx, dword ptr [ebp-0x30]
jmp_10037952:
		mov dword ptr [g_unk0x10069445+ebx*0x4], edi
		inc ebx
		add edi, dword ptr [ebp-0x68]
		loop jmp_10037952
		mov eax, dword ptr [ebp-0x28]
		cmp eax, dword ptr [ebp-0x30]
		_emit 0x7c /* jl jmp_1003796a */
		_emit 0x03
		mov eax, dword ptr [ebp-0x30]
jmp_1003796a:
		mov dword ptr [ebp-0x4], eax
		mov eax, dword ptr [ebp-0x24]
		mov ebx, dword ptr [ebp-0x2c]
		cmp ebx, eax
		_emit 0x7f /* jg jmp_10037979 */
		_emit 0x02
		mov eax, ebx
jmp_10037979:
		mov dword ptr [ebp-0x8], eax
		xor ebx, ebx
		mov dword ptr [ebp-0x20], ebx
		mov eax, dword ptr [ebp-0x4]
jmp_10037984:
		inc ebx
		stc
		rcl dword ptr [ebp-0x20], 0x1
		shr eax, 0x1
		_emit 0x75 /* jne jmp_10037984 */
		_emit 0xf7
		mov dword ptr [ebp-0x1c], ebx
		mov eax, dword ptr [ebp-0x8]
jmp_10037993:
		inc ebx
		shr eax, 0x1
		_emit 0x75 /* jne jmp_10037993 */
		_emit 0xfb
		mov eax, dword ptr [g_unk0x1003775b+ebx*0x4-0x8]
		mov dword ptr [ebp-0x10], eax
		cmp dword ptr [ebp+0x14], 0x0
		_emit 0x75 /* jne jmp_100379bb */
		_emit 0x13
		mov dword ptr [ebp+0x14], 0x1
		mov ebx, 0x0
		mov esi, 0x0
		_emit 0xeb /* jmp jmp_100379e9 */
		_emit 0x2e
jmp_100379bb:
		mov esi, dword ptr [ebp+0x14]
		shr esi, 0x1
		_emit 0x73 /* jae jmp_100379c5 */
		_emit 0x03
		xor esi, dword ptr [ebp-0x10]
jmp_100379c5:
		mov dword ptr [ebp+0x14], esi
		cmp esi, 0x1
		_emit 0x75 /* jne jmp_100379d4 */
		_emit 0x07
		mov dword ptr [ebp+0x10], 0x0
jmp_100379d4:
		mov ecx, dword ptr [ebp-0x1c]
		shr esi, cl
		cmp esi, dword ptr [ebp-0x8]
		_emit 0x73 /* jae jmp_100379bb */
		_emit 0xdd
		mov ebx, dword ptr [ebp+0x14]
		and ebx, dword ptr [ebp-0x20]
		cmp ebx, dword ptr [ebp-0x4]
		_emit 0x73 /* jae jmp_100379bb */
		_emit 0xd2
jmp_100379e9:
		dec dword ptr [ebp+0x10]
		mov eax, esi
		add eax, dword ptr [ebp-0x6c]
		cmp eax, dword ptr [ebp-0x54]
		_emit 0x7c /* jl jmp_100379bb */
		_emit 0xc5
		cmp eax, dword ptr [ebp-0x5c]
		_emit 0x7f /* jg jmp_100379bb */
		_emit 0xc0
		mov eax, esi
		add eax, dword ptr [ebp-0x4c]
		cmp eax, dword ptr [ebp-0x34]
		_emit 0x7c /* jl jmp_100379bb */
		_emit 0xb6
		cmp eax, dword ptr [ebp-0x3c]
		_emit 0x7f /* jg jmp_100379bb */
		_emit 0xb1
		mov eax, ebx
		add eax, dword ptr [ebp-0x70]
		cmp eax, dword ptr [ebp-0x58]
		_emit 0x7c /* jl jmp_100379bb */
		_emit 0xa7
		cmp eax, dword ptr [ebp-0x60]
		_emit 0x7f /* jg jmp_100379bb */
		_emit 0xa2
		mov eax, ebx
		add eax, dword ptr [ebp-0x50]
		cmp eax, dword ptr [ebp-0x38]
		_emit 0x7c /* jl jmp_100379bb */
		_emit 0x98
		cmp eax, dword ptr [ebp-0x40]
		_emit 0x7f /* jg jmp_100379bb */
		_emit 0x93
		mov edi, dword ptr [g_unk0x10069445+ebx*0x4]
		add edi, esi
		mov eax, dword ptr [g_unk0x10068845+ebx*0x4]
		add esi, eax
		movsb
		cmp dword ptr [ebp+0x10], 0x0
		jge jmp_100379bb
		mov eax, dword ptr [ebp+0x14]
		pop edi
		pop esi
		pop ebx
		pop es
		leave
		ret
	}
}
#endif

// Fades the colors used in the view toward p_palette in p_steps steps, through the
// callbacks in g_unk0x100687cc.
#ifdef COMPAT_MODE
void FUN_10037a4e(PixelView* p_view, undefined* p_palette, MechS32 p_steps)
{
	STUB(0x10037a4e);
}
#else
// FUNCTION: MW2SHELL 0x10037a4e
__declspec(naked) void FUN_10037a4e(PixelView* p_view, undefined* p_palette, MechS32 p_steps)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0x10
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov edi, offset g_unk0x10069a45
		mov eax, 0x0
		mov ecx, 0x40
		rep stosd
		mov esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0x4]
		inc eax
		mov ecx, dword ptr [esi+0x8]
		inc ecx
		imul ecx, eax
		mov esi, dword ptr [esi]
		mov edi, offset g_unk0x10069045
		mov dword ptr [ebp-0x4], 0xffffffff
	jmp_10037a88:
		mov eax, 0x0
		lodsb
		cmp byte ptr [g_unk0x10069a45+eax], 0x0
		_emit 0x75 /* jnz jmp_10037abb */
		_emit 0x24
		or byte ptr [g_unk0x10069a45+eax], 0x1
		stosb
		inc dword ptr [ebp-0x4]
		mov ebx, eax
		shl ebx, 0x1
		add ebx, eax
		add ebx, offset g_unk0x10068d45
		push ecx
		push ebx
		push eax
		call dword ptr [g_unk0x100687cc+0x20]
		add esp, 0x8
		pop ecx
	jmp_10037abb:
		dec ecx
		_emit 0x75 /* jnz jmp_10037a88 */
		_emit 0xca
		mov dword ptr [ebp-0x8], 0x0
		mov ecx, dword ptr [ebp-0x4]
		mov ebx, 0x0
		mov esi, dword ptr [ebp+0xc]
	jmp_10037ad0:
		movzx eax, byte ptr [g_unk0x10069045+ecx]
		mov ebx, eax
		shl ebx, 0x1
		add ebx, eax
		mov edx, 0x2
	jmp_10037ae2:
		mov ah, 0xff
		mov al, byte ptr [g_unk0x10068d45+ebx]
		sub al, byte ptr [ebx+esi]
		_emit 0x7d /* jge jmp_10037af3 */
		_emit 0x04
		mov ah, 0x1
		neg al
	jmp_10037af3:
		mov byte ptr [g_unk0x10069a45+ebx], ah
		mov byte ptr [g_unk0x10069145+ebx], al
		cmp al, byte ptr [ebp-0x8]
		_emit 0x7e /* jle jmp_10037b07 */
		_emit 0x03
		mov byte ptr [ebp-0x8], al
	jmp_10037b07:
		inc ebx
		dec edx
		_emit 0x79 /* jns jmp_10037ae2 */
		_emit 0xd7
		dec ecx
		_emit 0x79 /* jns jmp_10037ad0 */
		_emit 0xc2
		mov ecx, dword ptr [ebp-0x4]
		mov edi, offset g_unk0x10069d45
		mov al, byte ptr [ebp-0x8]
		shr al, 0x1
		mov ah, al
		shr ecx, 0x1
		rep stosw
		adc ecx, 0x0
		rep stosb
		movzx esi, byte ptr [ebp-0x8]
		cmp esi, 0x0
		jz jmp_10037bcc
		mov eax, dword ptr [ebp+0x10]
		mov edx, 0x0
		shld edx, eax, 0x10
		shl eax, 0x10
		div esi
		mov dword ptr [ebp-0xc], eax
		mov dword ptr [ebp-0x10], 0x8000
	jmp_10037b4f:
		mov ecx, dword ptr [ebp-0x4]
	jmp_10037b52:
		movzx eax, byte ptr [g_unk0x10069045+ecx]
		mov ebx, eax
		shl ebx, 0x1
		add ebx, eax
		mov edx, 0x2
	jmp_10037b64:
		mov al, byte ptr [g_unk0x10069d45+edx+ebx]
		add al, byte ptr [g_unk0x10069145+edx+ebx]
		cmp al, byte ptr [ebp-0x8]
		_emit 0x7c /* jl jmp_10037b88 */
		_emit 0x11
		sub al, byte ptr [ebp-0x8]
		mov ah, byte ptr [g_unk0x10069a45+edx+ebx]
		add byte ptr [g_unk0x10068d45+edx+ebx], ah
	jmp_10037b88:
		mov byte ptr [g_unk0x10069d45+edx+ebx], al
		dec edx
		_emit 0x79 /* jns jmp_10037b64 */
		_emit 0xd2
		push ecx
		mov eax, ebx
		add eax, offset g_unk0x10068d45
		push eax
		movzx eax, byte ptr [g_unk0x10069045+ecx]
		push eax
		call dword ptr [g_unk0x100687cc+0x24]
		add esp, 0x8
		pop ecx
		dec ecx
		_emit 0x79 /* jns jmp_10037b52 */
		_emit 0xa2
		mov eax, dword ptr [ebp-0xc]
		add dword ptr [ebp-0x10], eax
		cmp word ptr [ebp-0xe], 0x1
		_emit 0x7c /* jl jmp_10037bc9 */
		_emit 0x0c
	jmp_10037bbd:
		call dword ptr [g_unk0x100687cc+0x14]
		dec word ptr [ebp-0xe]
		_emit 0x75 /* jnz jmp_10037bbd */
		_emit 0xf4
	jmp_10037bc9:
		dec esi
		_emit 0x75 /* jnz jmp_10037b4f */
		_emit 0x83
	jmp_10037bcc:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif

// Counts the distinct colors in the view, storing each one in p_colors when given.
#ifdef COMPAT_MODE
MechS32 FUN_10037bd2(PixelView* p_view, MechU32* p_colors)
{
	STUB(0x10037bd2);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10037bd2
__declspec(naked) MechS32 FUN_10037bd2(PixelView* p_view, MechU32* p_colors)
{
	__asm {
		push ebp
		mov ebp, esp
		add esp, -0xc
		push ebx
		push esi
		push edi
		push es
		cld
		push ds
		pop es
		mov edi, offset g_unk0x10069a45
		mov eax, 0x0
		mov ecx, 0x40
		rep stosd
		mov esi, dword ptr [ebp+0x8]
		mov ecx, dword ptr [esi+0xc]
		sub ecx, dword ptr [esi+0x4]
		mov dword ptr [ebp-0x8], ecx
		mov ebx, dword ptr [esi+0x10]
		sub ebx, dword ptr [esi+0x8]
		mov esi, dword ptr [esi]
		mov eax, dword ptr [esi+0x4]
		inc eax
		mov dword ptr [ebp-0xc], eax
		mov esi, dword ptr [ebp+0x8]
		mov eax, dword ptr [esi+0x8]
		imul dword ptr [ebp-0xc]
		add eax, dword ptr [esi+0x4]
		mov esi, dword ptr [esi]
		mov esi, dword ptr [esi]
		add esi, eax
		mov edi, dword ptr [ebp+0xc]
		mov dword ptr [ebp-0x4], 0xffffffff
	jmp_10037c27:
		mov eax, 0x0
		movzx eax, byte ptr [ecx+esi]
		cmp byte ptr [g_unk0x10069a45+eax], 0x0
		_emit 0x75 /* jnz jmp_10037c49 */
		_emit 0x10
		or byte ptr [g_unk0x10069a45+eax], 0x1
		inc dword ptr [ebp-0x4]
		cmp edi, 0x0
		_emit 0x74 /* jz jmp_10037c49 */
		_emit 0x01
		stosd
	jmp_10037c49:
		dec ecx
		_emit 0x79 /* jns jmp_10037c27 */
		_emit 0xdb
		mov ecx, dword ptr [ebp-0x8]
		add esi, dword ptr [ebp-0xc]
		dec ebx
		_emit 0x79 /* jns jmp_10037c27 */
		_emit 0xd2
		mov eax, dword ptr [ebp-0x4]
		inc eax
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
	}
}
#endif
