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
   inline assembler encodes identically. `loop` has no rel32 form, so FUN_10037096's loop
   keeps a plain label jump. */
#include "compat.h"
#include "decomp.h"
#include "pixelview.h"
#include "types.h"

#pragma warning(disable : 4102) /* the labels mark the targets of the _emit short jumps */
#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// 13 dwords that FUN_10032279 copies in from its argument.
// GLOBAL: MW2SHELL 0x100687cc
undefined4 g_unk0x100687cc[0xd] = {0};

// FUN_10032250 copies a name into it and returns it. The next variable the assembly
// references starts at 0x1006880d, so the buffer is 0xd bytes.
// GLOBAL: MW2SHELL 0x10068800
MechChar g_unk0x10068800[0xd] = "MCGA.DLL";

// RLE scanline buffer: FUN_10037014 decodes one scanline (up to the ushort width at data
// header +0x42) into it before blitting it through FUN_10036c9e. Only the base address is
// ever referenced, so no bound below 0x400 is provable.
// GLOBAL: MW2SHELL 0x10068d45
undefined g_unk0x10068d45[0x400] = {0};

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

// STUB: MW2SHELL 0x10032449
void FUN_10032449(
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
}

// STUB: MW2SHELL 0x10032f84
void FUN_10032f84(PixelView* p_view, undefined4 p_unk0x04, undefined4 p_unk0x08, MechS32 p_left, MechS32 p_top)
{
	STUB(0x10032f84);
}

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

// STUB: MW2SHELL 0x10036c9e
void FUN_10036c9e(PixelView* p_view, MechS32 p_index, undefined* p_data, MechS32 p_count)
{
	STUB(0x10036c9e);
}

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
void FUN_10037096(undefined* p_data, MechS32 p_size, undefined* p_palette)
{
	STUB(0x10037096);
}
#else
// FUNCTION: MW2SHELL 0x10037096
__declspec(naked) void FUN_10037096(undefined* p_data, MechS32 p_size, undefined* p_palette)
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
