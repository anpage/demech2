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

   The inline assembler also encodes `xchg r32, r32` and `test r8, r8` with the two registers
   the other way round in the ModRM byte (87 da for `xchg ebx, edx`, where the original has
   87 d3), so those few instructions are _emit bytes too.

   Not yet transcribed: FUN_10036904 indexes the fixed-point sine table at 0x10035af0, which
   lives in .text; FUN_10036def, FUN_10036fb6 and FUN_10036fe7 push the addresses of the IFF
   chunk tags that follow FUN_10036c9e; FUN_100376f9 jumps through a table of its own labels.
   Each needs a symbol inside .text. */
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

// 13 dwords that FUN_10032279 copies in from its argument.
// GLOBAL: MW2SHELL 0x100687cc
undefined4 g_unk0x100687cc[0xd] = {0};

// FUN_10032250 copies a name into it and returns it. The next variable the assembly
// references starts at 0x1006880d, so the buffer is 0xd bytes.
// GLOBAL: MW2SHELL 0x10068800
MechChar g_unk0x10068800[0xd] = "MCGA.DLL";

// A dword table that FUN_100376f9 fills and reads. The next variable starts at 0x10068d45.
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

// A second dword table of FUN_100376f9. The next variable starts at 0x10069a45.
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

// Looks up the 16.16 sine and cosine of p_angle (in tenths of a degree) in the fixed-point
// table at 0x10035af0, which the source doesn't reproduce yet.
// STUB: MW2SHELL 0x10036904
void FUN_10036904(MechS32 p_angle, MechS32* p_sin, MechS32* p_cos)
{
	STUB(0x10036904);
}

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

// Returns the dword at +0x8 of the font data (BrassLantern0x414 keeps it as the line height).
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
// offset (BrassLantern0x414 sums it as the character's width).
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

// Copies p_count pixels into row p_index of the view, clipped. The IFF chunk tags that
// FUN_10036def, FUN_10036fb6 and FUN_10036fe7 look up follow the routine.
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
									/* "BMHD" */
		_emit 0x42
		_emit 0x4d
		_emit 0x48
		_emit 0x44
									/* "CMAP" */
		_emit 0x43
		_emit 0x4d
		_emit 0x41
		_emit 0x50
									/* "BODY" */
		_emit 0x42
		_emit 0x4f
		_emit 0x44
		_emit 0x59
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
		_emit 0x75 /* jnz jmp_10036dc5 */
		_emit 0x03
		inc esi
		_emit 0xeb /* jmp jmp_10036dbd */
		_emit 0xf8
jmp_10036dc5:
		mov ecx, 0x2
		mov edi, dword ptr [ebp+0x8]
		mov eax, esi
		repe cmpsw
		_emit 0x74 /* jz jmp_10036de6 */
		_emit 0x12
		mov esi, eax
		add esi, 0x6
		lodsw
		xchg ah, al
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
