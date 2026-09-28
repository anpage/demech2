/* Hand-written assembly (originally a MASM object), transcribed like blit.c: every
   routine is a __declspec(naked) function whose body is one __asm block, with the original's
   short jumps, its xchg/test encodings and its alignment padding (cs: mov eax, eax, which the
   inline assembler can't prefix) as _emit bytes. The object follows the compiled code
   that ends at 0x1002a966; its routines fill polygons given as arrays of six-dword vertices
   (x and y first) and share the working variables in g_unk0x100666a0.

   FUN_1002cd5d loads its per-mode span routines from a table of its own labels, which the
   original keeps inside the routine (0x1002d2a0). The inline assembler has no data directives,
   so the table is split out as g_unk0x1002d2a0, 16 filler bytes keep its place, and the code
   after it is split at the table's entries into the span routines FUN_1002d2b0, FUN_1002d457,
   FUN_1002d5c3 and FUN_1002d724. Jumps between the pieces are _emit bytes: their displacements
   hold because the pieces are laid out back to back as in the original. */
#include "compat.h"
#include "decomp.h"
#include "pixelview.h"
#include "types.h"

#pragma warning(disable : 4102) /* the labels mark the targets of the _emit short jumps */
#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// The routines' working variables: the clipped view, the vertex list and its edges' walks.
// FUN_1002cd3d fills the 0x40 dwords at +0xd0.
// GLOBAL: MW2SHELL 0x100666a0
undefined4 g_unk0x100666a0[0x74] = {0};

void FUN_1002d2b0(void);
void FUN_1002d457(void);
void FUN_1002d5c3(void);
void FUN_1002d724(void);

// FUN_1002cd5d's span routines, one per mode. The original keeps the table inside FUN_1002cd5d,
// between the routine and the span routines.
// GLOBAL: MW2SHELL 0x1002d2a0
void (*g_unk0x1002d2a0[4])(void) = {FUN_1002d724, FUN_1002d457, FUN_1002d5c3, FUN_1002d2b0};

// Fills a polygon in one color (the third dword of the first vertex, 16.16), clipped to the view.
#ifdef COMPAT_MODE
void FUN_1002a968(PixelView* p_view, MechS32 p_count, MechS32* p_vertices)
{
	STUB(0x1002a968);
}
#else
// FUNCTION: MW2SHELL 0x1002a968
__declspec(naked) void FUN_1002a968(PixelView* p_view, MechS32 p_count, MechS32* p_vertices)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov ecx, dword ptr [ebx+0x4]
		inc ecx
		mov dword ptr [g_unk0x100666a0+0xc], ecx
		mov eax, dword ptr [esi+0xc]
		mov ecx, dword ptr [ebx+0x4]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002a98a */
		_emit 0x02
		mov ecx, eax
jmp_1002a98a:
		mov eax, dword ptr [esi+0x4]
		mov edi, 0x0
		cmp edi, eax
		_emit 0x7f /* jg jmp_1002a998 */
		_emit 0x02
		mov edi, eax
jmp_1002a998:
		sub ecx, edi
		jl jmp_1002ac9b
		mov dword ptr [g_unk0x100666a0], ecx
		mov eax, dword ptr [esi+0x10]
		mov ecx, dword ptr [ebx+0x8]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002a9b2 */
		_emit 0x02
		mov ecx, eax
jmp_1002a9b2:
		mov edx, dword ptr [esi+0x8]
		mov eax, 0x0
		cmp eax, edx
		_emit 0x7f /* jg jmp_1002a9c0 */
		_emit 0x02
		mov eax, edx
jmp_1002a9c0:
		sub ecx, eax
		jl jmp_1002ac9b
		mov dword ptr [g_unk0x100666a0+0x4], ecx
		mul dword ptr [g_unk0x100666a0+0xc]
		add eax, edi
		add eax, dword ptr [ebx]
		mov dword ptr [g_unk0x100666a0+0x8], eax
		push ds
		pop es
		mov ebx, dword ptr [ebp+0x10]
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		mov edx, eax
		shl eax, 0x1
		add eax, edx
		add eax, ebx
		mov dword ptr [g_unk0x100666a0+0x24], ebx
		mov dword ptr [g_unk0x100666a0+0x28], eax
		mov eax, dword ptr [ebx+0x8]
		add eax, 0x8000
		shr eax, 0x10
		mov ah, al
		mov edx, eax
		shl eax, 0x10
		or eax, edx
		mov dword ptr [g_unk0x100666a0+0xa8], eax
		mov dword ptr [g_unk0x100666a0+0x20], 0x0
		mov esi, 0x7fff
		mov edi, 0xffff8000
		mov ecx, 0xf
jmp_1002aa2d:
		mov edx, 0x0
		mov eax, dword ptr [ebx]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0]
		sub eax, dword ptr [ebx]
		shld edx, eax, 0x1
		or dword ptr [g_unk0x100666a0+0x20], edx
		mov eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		cmp eax, esi
		_emit 0x7f /* jg jmp_1002aa6b */
		_emit 0x08
		mov esi, eax
		mov dword ptr [g_unk0x100666a0+0x2c], ebx
jmp_1002aa6b:
		cmp eax, edi
		_emit 0x7c /* jl jmp_1002aa71 */
		_emit 0x02
		mov edi, eax
jmp_1002aa71:
		and ecx, edx
		add ebx, 0x18
		cmp ebx, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x75 /* jne jmp_1002aa2d */
		_emit 0xaf
		or ecx, ecx
		jne jmp_1002ac9b
		mov eax, dword ptr [g_unk0x100666a0+0x2c]
		mov dword ptr [g_unk0x100666a0+0x38], eax
		mov dword ptr [g_unk0x100666a0+0x3c], eax
		mov dword ptr [g_unk0x100666a0+0x4c], esi
		cmp edi, esi
		je jmp_1002ac9b
jmp_1002aaa3:
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002aac5 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002aac5:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002aadb */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002aaa3 */
		_emit 0xc8
jmp_1002aadb:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002aaa3 */
		_emit 0xc4
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x50], edx
jmp_1002ab10:
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002ab2f */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002ab2f:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002ab45 */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002ab10 */
		_emit 0xcb
jmp_1002ab45:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002ab10 */
		_emit 0xc7
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x54], edx
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		sub edi, dword ptr [g_unk0x100666a0+0x4]
		_emit 0x7f /* jg jmp_1002ab8f */
		_emit 0x02
		add eax, edi
jmp_1002ab8f:
		mov dword ptr [g_unk0x100666a0+0x48], eax
		mov eax, 0x0
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		_emit 0x7e /* jle jmp_1002abfd */
		_emit 0x5c
		sub dword ptr [g_unk0x100666a0+0x48], eax
		mov ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x4c], ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x30]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x40], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x70]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x50], eax
		mov ecx, 0x0
		mov ebx, dword ptr [g_unk0x100666a0+0x34]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x44], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x74]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x54], eax
jmp_1002abfd:
		mov eax, dword ptr [g_unk0x100666a0+0x4c]
		mul dword ptr [g_unk0x100666a0+0xc]
		mov edi, dword ptr [g_unk0x100666a0+0x8]
		add edi, eax
		mov eax, dword ptr [g_unk0x100666a0+0x50]
		mov ebx, dword ptr [g_unk0x100666a0+0x54]
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		jne jmp_1002acb4
jmp_1002ac28:
		push eax
		push edi
		mov edx, ebx
		cmp edx, eax
		_emit 0x7f /* jg jmp_1002ac31 */
		_emit 0x01
		xchg edx, eax
jmp_1002ac31:
		sar eax, 0x10
		sar edx, 0x10
		mov ecx, edx
		sub ecx, eax
		inc ecx
		add edi, eax
		mov eax, dword ptr [g_unk0x100666a0+0xa8]
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_1002ac61 */
		_emit 0x19
		mov edx, ecx
		mov ecx, edi
		neg ecx
		and ecx, 0x3
		sub edx, ecx
		rep stosb
		mov ecx, edx
		shr ecx, 0x2
		rep stosd
		and edx, 0x3
		mov ecx, edx
jmp_1002ac61:
		rep stosb
		pop edi
		pop eax
		add edi, dword ptr [g_unk0x100666a0+0xc]
		dec dword ptr [g_unk0x100666a0+0x48]
		_emit 0x78 /* js jmp_1002ac9b */
		_emit 0x28
		_emit 0x74 /* je jmp_1002aca1 */
		_emit 0x2c
		dec dword ptr [g_unk0x100666a0+0x40]
		je jmp_1002ad5b
		add eax, dword ptr [g_unk0x100666a0+0x70]
jmp_1002ac87:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002adcf
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		_emit 0xeb /* jmp jmp_1002ac28 */
		_emit 0x8d
jmp_1002ac9b:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1002aca1:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		jmp jmp_1002ac28
		mov eax, eax
jmp_1002acb4:
		push eax
		push edi
		mov edx, ebx
		cmp edx, eax
		_emit 0x7f /* jg jmp_1002acbd */
		_emit 0x01
		xchg edx, eax
jmp_1002acbd:
		sar eax, 0x10
		sar edx, 0x10
		cmp eax, dword ptr [g_unk0x100666a0]
		_emit 0x7f /* jg jmp_1002ad09 */
		_emit 0x3e
		cmp edx, 0x0
		_emit 0x7c /* jl jmp_1002ad09 */
		_emit 0x39
		mov ecx, edx
		sub ecx, eax
		inc ecx
		add edi, eax
		sub eax, 0x0
		_emit 0x7c /* jl jmp_1002ad51 */
		_emit 0x75
jmp_1002acdc:
		sub edx, dword ptr [g_unk0x100666a0]
		_emit 0x7f /* jg jmp_1002ad57 */
		_emit 0x73
jmp_1002ace4:
		mov eax, dword ptr [g_unk0x100666a0+0xa8]
		cmp ecx, 0x4
		_emit 0x7e /* jle jmp_1002ad07 */
		_emit 0x19
		mov edx, ecx
		mov ecx, edi
		neg ecx
		and ecx, 0x3
		sub edx, ecx
		rep stosb
		mov ecx, edx
		shr ecx, 0x2
		rep stosd
		and edx, 0x3
		mov ecx, edx
jmp_1002ad07:
		rep stosb
jmp_1002ad09:
		pop edi
		pop eax
		add edi, dword ptr [g_unk0x100666a0+0xc]
		dec dword ptr [g_unk0x100666a0+0x48]
		_emit 0x78 /* js jmp_1002ac9b */
		_emit 0x82
		_emit 0x74 /* je jmp_1002ad40 */
		_emit 0x25
		dec dword ptr [g_unk0x100666a0+0x40]
		_emit 0x74 /* je jmp_1002ad5b */
		_emit 0x38
		add eax, dword ptr [g_unk0x100666a0+0x70]
jmp_1002ad29:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002adcf
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		jmp jmp_1002acb4
jmp_1002ad40:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		jmp jmp_1002acb4
jmp_1002ad51:
		sub edi, eax
		add ecx, eax
		_emit 0xeb /* jmp jmp_1002acdc */
		_emit 0x85
jmp_1002ad57:
		sub ecx, edx
		_emit 0xeb /* jmp jmp_1002ace4 */
		_emit 0x89
jmp_1002ad5b:
		push ebx
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002ad7e */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002ad7e:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov eax, dword ptr [ebx]
		shl eax, 0x10
		add eax, 0x8000
		pop ebx
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		je jmp_1002ac87
		jmp jmp_1002ad29
jmp_1002adcf:
		push eax
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002adef */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002adef:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov ebx, dword ptr [ebx]
		shl ebx, 0x10
		add ebx, 0x8000
		pop eax
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		je jmp_1002ac28
		jmp jmp_1002acb4
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_1002ae41(PixelView* p_view, MechS32 p_count, MechS32* p_vertices)
{
	STUB(0x1002ae41);
}
#else
// FUNCTION: MW2SHELL 0x1002ae41
__declspec(naked) void FUN_1002ae41(PixelView* p_view, MechS32 p_count, MechS32* p_vertices)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov ecx, dword ptr [ebx+0x4]
		inc ecx
		mov dword ptr [g_unk0x100666a0+0xc], ecx
		mov eax, dword ptr [esi+0xc]
		mov ecx, dword ptr [ebx+0x4]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002ae63 */
		_emit 0x02
		mov ecx, eax
jmp_1002ae63:
		mov eax, dword ptr [esi+0x4]
		mov edi, 0x0
		cmp edi, eax
		_emit 0x7f /* jg jmp_1002ae71 */
		_emit 0x02
		mov edi, eax
jmp_1002ae71:
		sub ecx, edi
		jl jmp_1002b30a
		mov dword ptr [g_unk0x100666a0], ecx
		mov eax, dword ptr [esi+0x10]
		mov ecx, dword ptr [ebx+0x8]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002ae8b */
		_emit 0x02
		mov ecx, eax
jmp_1002ae8b:
		mov edx, dword ptr [esi+0x8]
		mov eax, 0x0
		cmp eax, edx
		_emit 0x7f /* jg jmp_1002ae99 */
		_emit 0x02
		mov eax, edx
jmp_1002ae99:
		sub ecx, eax
		jl jmp_1002b30a
		mov dword ptr [g_unk0x100666a0+0x4], ecx
		mul dword ptr [g_unk0x100666a0+0xc]
		add eax, edi
		add eax, dword ptr [ebx]
		mov dword ptr [g_unk0x100666a0+0x8], eax
		push ds
		pop es
		mov ebx, dword ptr [ebp+0x10]
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		mov edx, eax
		shl eax, 0x1
		add eax, edx
		add eax, ebx
		mov dword ptr [g_unk0x100666a0+0x24], ebx
		mov dword ptr [g_unk0x100666a0+0x28], eax
		mov dword ptr [g_unk0x100666a0+0x20], 0x0
		mov esi, 0x7fff
		mov edi, 0xffff8000
		mov ecx, 0xf
jmp_1002aeed:
		mov edx, 0x0
		mov eax, dword ptr [ebx]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0]
		sub eax, dword ptr [ebx]
		shld edx, eax, 0x1
		or dword ptr [g_unk0x100666a0+0x20], edx
		mov eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		cmp eax, esi
		_emit 0x7f /* jg jmp_1002af2b */
		_emit 0x08
		mov esi, eax
		mov dword ptr [g_unk0x100666a0+0x2c], ebx
jmp_1002af2b:
		cmp eax, edi
		_emit 0x7c /* jl jmp_1002af31 */
		_emit 0x02
		mov edi, eax
jmp_1002af31:
		and ecx, edx
		add ebx, 0x18
		cmp ebx, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x75 /* jne jmp_1002aeed */
		_emit 0xaf
		or ecx, ecx
		jne jmp_1002b30a
		mov eax, dword ptr [g_unk0x100666a0+0x2c]
		mov dword ptr [g_unk0x100666a0+0x38], eax
		mov dword ptr [g_unk0x100666a0+0x3c], eax
		mov dword ptr [g_unk0x100666a0+0x4c], esi
		cmp edi, esi
		je jmp_1002b30a
jmp_1002af63:
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002af85 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002af85:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002af9b */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002af63 */
		_emit 0xc8
jmp_1002af9b:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002af63 */
		_emit 0xc4
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x78], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x50], edx
		mov edx, dword ptr [ebx+0x8]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x58], edx
jmp_1002affe:
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002b01d */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002b01d:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002b033 */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002affe */
		_emit 0xcb
jmp_1002b033:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002affe */
		_emit 0xc7
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x7c], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x54], edx
		mov edx, dword ptr [ebx+0x8]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x5c], edx
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		sub edi, dword ptr [g_unk0x100666a0+0x4]
		_emit 0x7f /* jg jmp_1002b0ab */
		_emit 0x02
		add eax, edi
jmp_1002b0ab:
		mov dword ptr [g_unk0x100666a0+0x48], eax
		mov eax, 0x0
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		_emit 0x7e /* jle jmp_1002b13b */
		_emit 0x7e
		sub dword ptr [g_unk0x100666a0+0x48], eax
		mov ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x4c], ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x30]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x40], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x70]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x50], eax
		mov eax, dword ptr [g_unk0x100666a0+0x78]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x58], eax
		mov ecx, 0x0
		mov ebx, dword ptr [g_unk0x100666a0+0x34]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x44], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x74]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x54], eax
		mov eax, dword ptr [g_unk0x100666a0+0x7c]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x5c], eax
jmp_1002b13b:
		mov eax, dword ptr [g_unk0x100666a0+0x4c]
		mul dword ptr [g_unk0x100666a0+0xc]
		mov edi, dword ptr [g_unk0x100666a0+0x8]
		add edi, eax
		mov eax, dword ptr [g_unk0x100666a0+0x50]
		mov ebx, dword ptr [g_unk0x100666a0+0x54]
		mov ecx, dword ptr [g_unk0x100666a0+0x58]
		mov edx, dword ptr [g_unk0x100666a0+0x5c]
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		jne jmp_1002b330
		mov eax, eax
jmp_1002b174:
		push eax
		push ebx
		push ecx
		push edx
		push edi
		cmp ebx, eax
		_emit 0x7f /* jg jmp_1002b180 */
		_emit 0x03
		xchg ebx, eax
		_emit 0x87 /* xchg edx, ecx: the inline assembler encodes the operands the other way */
		_emit 0xca
jmp_1002b180:
		sar eax, 0x10
		sar ebx, 0x10
		add edi, eax
		sub ebx, eax
		mov esi, ebx
		_emit 0x74 /* je jmp_1002b1a4 */
		_emit 0x16
		sub edx, ecx
		xor eax, eax
		shrd eax, edx, 0x10
		shl ebx, 0x10
		sar edx, 0x10
		idiv ebx
		shld edx, eax, 0x10
		mov ebx, esi
jmp_1002b1a4:
		add ebx, edi
		mov dword ptr [g_unk0x100666a0+0x1c], ebx
		inc esi
		mov ebx, ecx
		shr ecx, 0x10
		shl ebx, 0x10
		shl eax, 0x10
		mov ch, cl
		add ebx, eax
		adc ch, dl
		test edi, 0x1
		_emit 0x74 /* je jmp_1002b1d6 */
		_emit 0x10
		mov byte ptr [edi], cl
		dec esi
		je jmp_1002b2c0
		inc edi
		mov cl, ch
		add ebx, eax
		adc ch, dl
jmp_1002b1d6:
		cmp esi, 0x1
		je jmp_1002b2b8
		push esi
		shr esi, 0x1
		dec esi
		cmp esi, 0x5
		_emit 0x7c /* jl jmp_1002b254 */
		_emit 0x6c
jmp_1002b1e8:
		mov word ptr [edi], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0x2], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0x4], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0x6], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0x8], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0xa], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		add edi, 0xc
		sub esi, 0x6
		_emit 0x78 /* js jmp_1002b2af */
		_emit 0x60
		cmp esi, 0x5
		_emit 0x7d /* jge jmp_1002b1e8 */
		_emit 0x94
jmp_1002b254:
		mov word ptr [edi], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002b2af */
		_emit 0x49
		mov word ptr [edi+0x2], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002b2af */
		_emit 0x36
		mov word ptr [edi+0x4], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002b2af */
		_emit 0x23
		mov word ptr [edi+0x6], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002b2af */
		_emit 0x10
		mov word ptr [edi+0x8], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
jmp_1002b2af:
		pop esi
		test esi, 0x1
		_emit 0x74 /* je jmp_1002b2c0 */
		_emit 0x08
jmp_1002b2b8:
		mov edi, dword ptr [g_unk0x100666a0+0x1c]
		mov byte ptr [edi], cl
jmp_1002b2c0:
		pop edi
		pop edx
		pop ecx
		pop ebx
		pop eax
		add edi, dword ptr [g_unk0x100666a0+0xc]
		dec dword ptr [g_unk0x100666a0+0x48]
		_emit 0x78 /* js jmp_1002b30a */
		_emit 0x37
		_emit 0x74 /* je jmp_1002b310 */
		_emit 0x3b
		dec dword ptr [g_unk0x100666a0+0x40]
		je jmp_1002b551
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
jmp_1002b2ed:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002b5ef
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002b174
jmp_1002b30a:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1002b310:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002b174
		_emit 0x2e /* mov eax, eax with a segment prefix: MASM alignment padding */
		_emit 0x8b
		_emit 0xc0
jmp_1002b330:
		push eax
		push ebx
		push ecx
		push edx
		push edi
		cmp ebx, eax
		_emit 0x7f /* jg jmp_1002b33c */
		_emit 0x03
		xchg ebx, eax
		_emit 0x87 /* xchg edx, ecx: the inline assembler encodes the operands the other way */
		_emit 0xca
jmp_1002b33c:
		sar eax, 0x10
		cmp eax, dword ptr [g_unk0x100666a0]
		jg jmp_1002b4cb
		sar ebx, 0x10
		cmp ebx, 0x0
		jl jmp_1002b4cb
		mov dword ptr [g_unk0x100666a0+0x90], eax
		mov dword ptr [g_unk0x100666a0+0x94], ebx
		add edi, eax
		sub ebx, eax
		mov esi, ebx
		inc esi
		cmp esi, 0x1
		_emit 0x74 /* je jmp_1002b383 */
		_emit 0x15
		sub edx, ecx
		xor eax, eax
		shrd eax, edx, 0x10
		shl ebx, 0x10
		sar edx, 0x10
		idiv ebx
		mov dword ptr [g_unk0x100666a0+0x98], eax
jmp_1002b383:
		mov eax, 0x0
		sub eax, dword ptr [g_unk0x100666a0+0x90]
		jg jmp_1002b532
jmp_1002b394:
		mov eax, dword ptr [g_unk0x100666a0+0x94]
		sub eax, dword ptr [g_unk0x100666a0]
		jg jmp_1002b54a
jmp_1002b3a5:
		mov ebx, ecx
		shr ecx, 0x10
		mov eax, edi
		add eax, esi
		dec eax
		mov dword ptr [g_unk0x100666a0+0x1c], eax
		mov eax, dword ptr [g_unk0x100666a0+0x98]
		shld edx, eax, 0x10
		shl ebx, 0x10
		shl eax, 0x10
		mov ch, cl
		add ebx, eax
		adc ch, dl
		test edi, 0x1
		_emit 0x74 /* je jmp_1002b3e1 */
		_emit 0x10
		mov byte ptr [edi], cl
		dec esi
		je jmp_1002b4cb
		inc edi
		mov cl, ch
		add ebx, eax
		adc ch, dl
jmp_1002b3e1:
		cmp esi, 0x1
		je jmp_1002b4c3
		push esi
		shr esi, 0x1
		dec esi
		cmp esi, 0x5
		_emit 0x7c /* jl jmp_1002b45f */
		_emit 0x6c
jmp_1002b3f3:
		mov word ptr [edi], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0x2], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0x4], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0x6], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0x8], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		mov word ptr [edi+0xa], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		add edi, 0xc
		sub esi, 0x6
		_emit 0x78 /* js jmp_1002b4ba */
		_emit 0x60
		cmp esi, 0x5
		_emit 0x7d /* jge jmp_1002b3f3 */
		_emit 0x94
jmp_1002b45f:
		mov word ptr [edi], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002b4ba */
		_emit 0x49
		mov word ptr [edi+0x2], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002b4ba */
		_emit 0x36
		mov word ptr [edi+0x4], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002b4ba */
		_emit 0x23
		mov word ptr [edi+0x6], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002b4ba */
		_emit 0x10
		mov word ptr [edi+0x8], cx
		mov cl, ch
		add ebx, eax
		adc cl, dl
		mov ch, cl
		add ebx, eax
		adc ch, dl
jmp_1002b4ba:
		pop esi
		test esi, 0x1
		_emit 0x74 /* je jmp_1002b4cb */
		_emit 0x08
jmp_1002b4c3:
		mov edi, dword ptr [g_unk0x100666a0+0x1c]
		mov byte ptr [edi], cl
jmp_1002b4cb:
		pop edi
		pop edx
		pop ecx
		pop ebx
		pop eax
		add edi, dword ptr [g_unk0x100666a0+0xc]
		dec dword ptr [g_unk0x100666a0+0x48]
		js jmp_1002b30a
		_emit 0x74 /* je jmp_1002b515 */
		_emit 0x31
		dec dword ptr [g_unk0x100666a0+0x40]
		_emit 0x74 /* je jmp_1002b551 */
		_emit 0x65
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
jmp_1002b4f8:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002b5ef
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002b330
jmp_1002b515:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002b330
jmp_1002b532:
		add edi, eax
		sub esi, eax
		shl eax, 0x10
		imul dword ptr [g_unk0x100666a0+0x98]
		shrd eax, edx, 0x10
		add ecx, eax
		jmp jmp_1002b394
jmp_1002b54a:
		sub esi, eax
		jmp jmp_1002b3a5
jmp_1002b551:
		push ebx
		push edx
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002b575 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002b575:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x78], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov ecx, dword ptr [ebx+0x8]
		add ecx, 0x8000
		mov eax, dword ptr [ebx]
		shl eax, 0x10
		add eax, 0x8000
		pop edx
		pop ebx
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		je jmp_1002b2ed
		jmp jmp_1002b4f8
jmp_1002b5ef:
		push eax
		push ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002b610 */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002b610:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x7c], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov edx, dword ptr [ebx+0x8]
		add edx, 0x8000
		mov ebx, dword ptr [ebx]
		shl ebx, 0x10
		add ebx, 0x8000
		pop ecx
		pop eax
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		je jmp_1002b174
		jmp jmp_1002b330
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_1002b68b(PixelView* p_view, MechS32 p_count, MechS32* p_vertices, undefined4 p_unk0x0c)
{
	STUB(0x1002b68b);
}
#else
// FUNCTION: MW2SHELL 0x1002b68b
__declspec(naked) void FUN_1002b68b(PixelView* p_view, MechS32 p_count, MechS32* p_vertices, undefined4 p_unk0x0c)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov ecx, dword ptr [ebx+0x4]
		inc ecx
		mov dword ptr [g_unk0x100666a0+0xc], ecx
		mov eax, dword ptr [esi+0xc]
		mov ecx, dword ptr [ebx+0x4]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002b6ad */
		_emit 0x02
		mov ecx, eax
jmp_1002b6ad:
		mov eax, dword ptr [esi+0x4]
		mov edi, 0x0
		cmp edi, eax
		_emit 0x7f /* jg jmp_1002b6bb */
		_emit 0x02
		mov edi, eax
jmp_1002b6bb:
		sub ecx, edi
		jl jmp_1002bb8a
		mov dword ptr [g_unk0x100666a0], ecx
		mov eax, dword ptr [esi+0x10]
		mov ecx, dword ptr [ebx+0x8]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002b6d5 */
		_emit 0x02
		mov ecx, eax
jmp_1002b6d5:
		mov edx, dword ptr [esi+0x8]
		mov eax, 0x0
		cmp eax, edx
		_emit 0x7f /* jg jmp_1002b6e3 */
		_emit 0x02
		mov eax, edx
jmp_1002b6e3:
		sub ecx, eax
		jl jmp_1002bb8a
		mov dword ptr [g_unk0x100666a0+0x4], ecx
		mul dword ptr [g_unk0x100666a0+0xc]
		add eax, edi
		add eax, dword ptr [ebx]
		mov dword ptr [g_unk0x100666a0+0x8], eax
		push ds
		pop es
		mov dword ptr [g_unk0x100666a0+0xb8], ebp
		mov ebx, dword ptr [ebp+0x14]
		mov eax, dword ptr [ebp+0x10]
		shl eax, 0x3
		mov edx, eax
		shl eax, 0x1
		add eax, edx
		add eax, ebx
		mov dword ptr [g_unk0x100666a0+0x24], ebx
		mov dword ptr [g_unk0x100666a0+0x28], eax
		mov dword ptr [g_unk0x100666a0+0x20], 0x0
		mov esi, 0x7fff
		mov edi, 0xffff8000
		mov ecx, 0xf
jmp_1002b73d:
		mov edx, 0x0
		mov eax, dword ptr [ebx]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0]
		sub eax, dword ptr [ebx]
		shld edx, eax, 0x1
		or dword ptr [g_unk0x100666a0+0x20], edx
		mov eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		cmp eax, esi
		_emit 0x7f /* jg jmp_1002b77b */
		_emit 0x08
		mov esi, eax
		mov dword ptr [g_unk0x100666a0+0x2c], ebx
jmp_1002b77b:
		cmp eax, edi
		_emit 0x7c /* jl jmp_1002b781 */
		_emit 0x02
		mov edi, eax
jmp_1002b781:
		and ecx, edx
		add ebx, 0x18
		cmp ebx, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x75 /* jne jmp_1002b73d */
		_emit 0xaf
		or ecx, ecx
		jne jmp_1002bb8a
		mov eax, dword ptr [g_unk0x100666a0+0x2c]
		mov dword ptr [g_unk0x100666a0+0x38], eax
		mov dword ptr [g_unk0x100666a0+0x3c], eax
		mov dword ptr [g_unk0x100666a0+0x4c], esi
		cmp edi, esi
		je jmp_1002bb8a
jmp_1002b7b3:
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002b7d5 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002b7d5:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002b7eb */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002b7b3 */
		_emit 0xc8
jmp_1002b7eb:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002b7b3 */
		_emit 0xc4
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x78], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x50], edx
		mov edx, dword ptr [ebx+0x8]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x58], edx
jmp_1002b84e:
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002b86d */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002b86d:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002b883 */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002b84e */
		_emit 0xcb
jmp_1002b883:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002b84e */
		_emit 0xc7
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x7c], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x54], edx
		mov edx, dword ptr [ebx+0x8]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x5c], edx
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		sub edi, dword ptr [g_unk0x100666a0+0x4]
		_emit 0x7f /* jg jmp_1002b8fb */
		_emit 0x02
		add eax, edi
jmp_1002b8fb:
		mov dword ptr [g_unk0x100666a0+0x48], eax
		mov esi, dword ptr [ebp+0xc]
		mov edi, 0x0
		mov eax, 0x0
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		jle jmp_1002b9a0
		sub dword ptr [g_unk0x100666a0+0x48], eax
		test eax, 0x1
		_emit 0x74 /* je jmp_1002b928 */
		_emit 0x02
		_emit 0x87 /* xchg edi, esi: the inline assembler encodes the operands the other way */
		_emit 0xf7
jmp_1002b928:
		mov ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x4c], ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x30]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x40], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x70]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x50], eax
		mov eax, dword ptr [g_unk0x100666a0+0x78]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x58], eax
		mov ecx, 0x0
		mov ebx, dword ptr [g_unk0x100666a0+0x34]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x44], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x74]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x54], eax
		mov eax, dword ptr [g_unk0x100666a0+0x7c]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x5c], eax
jmp_1002b9a0:
		mov dword ptr [g_unk0x100666a0+0xb0], esi
		mov dword ptr [g_unk0x100666a0+0xb4], edi
		mov eax, dword ptr [g_unk0x100666a0+0x4c]
		mul dword ptr [g_unk0x100666a0+0xc]
		mov edi, dword ptr [g_unk0x100666a0+0x8]
		add edi, eax
		mov eax, dword ptr [g_unk0x100666a0+0x50]
		mov ebx, dword ptr [g_unk0x100666a0+0x54]
		mov ecx, dword ptr [g_unk0x100666a0+0x58]
		mov edx, dword ptr [g_unk0x100666a0+0x5c]
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		jne jmp_1002bbb0
		nop
jmp_1002b9e4:
		push eax
		push ebx
		push ecx
		push edx
		push edi
		cmp ebx, eax
		_emit 0x7f /* jg jmp_1002b9f0 */
		_emit 0x03
		xchg ebx, eax
		_emit 0x87 /* xchg edx, ecx: the inline assembler encodes the operands the other way */
		_emit 0xca
jmp_1002b9f0:
		sar eax, 0x10
		sar ebx, 0x10
		add edi, eax
		sub ebx, eax
		mov esi, ebx
		_emit 0x74 /* je jmp_1002ba1c */
		_emit 0x1e
		sub edx, ecx
		sar ebx, 0x1
		cmp ebx, 0x1
		adc ebx, 0x0
		xor eax, eax
		shrd eax, edx, 0x10
		shl ebx, 0x10
		sar edx, 0x10
		idiv ebx
		shld edx, eax, 0x10
		mov ebx, esi
jmp_1002ba1c:
		shl eax, 0x10
		add ebx, edi
		mov dword ptr [g_unk0x100666a0+0x1c], ebx
		inc esi
		mov ebx, ecx
		mov ebp, ecx
		add ebx, dword ptr [g_unk0x100666a0+0xb0]
		add ebp, dword ptr [g_unk0x100666a0+0xb4]
		mov ecx, ebx
		shr ecx, 0x10
		mov dh, cl
		mov ecx, ebp
		shr ecx, 0x8
		mov cl, dh
		shl ebx, 0x10
		shl ebp, 0x10
		test edi, 0x1
		_emit 0x74 /* je jmp_1002ba66 */
		_emit 0x12
		mov byte ptr [edi], cl
		dec esi
		je jmp_1002bb24
		inc edi
		_emit 0x86 /* xchg ch, cl: the inline assembler encodes the operands the other way */
		_emit 0xcd
		_emit 0x87 /* xchg ebp, ebx: the inline assembler encodes the operands the other way */
		_emit 0xdd
		add ebp, eax
		adc ch, dl
jmp_1002ba66:
		cmp esi, 0x1
		je jmp_1002bb1c
		push esi
		shr esi, 0x1
		dec esi
		cmp esi, 0x5
		_emit 0x7c /* jl jmp_1002bacc */
		_emit 0x54
jmp_1002ba78:
		mov word ptr [edi], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0x2], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0x4], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0x6], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0x8], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0xa], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add edi, 0xc
		sub esi, 0x6
		_emit 0x78 /* js jmp_1002bb13 */
		_emit 0x4c
		cmp esi, 0x5
		_emit 0x7d /* jge jmp_1002ba78 */
		_emit 0xac
jmp_1002bacc:
		mov word ptr [edi], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002bb13 */
		_emit 0x39
		mov word ptr [edi+0x2], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002bb13 */
		_emit 0x2a
		mov word ptr [edi+0x4], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002bb13 */
		_emit 0x1b
		mov word ptr [edi+0x6], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002bb13 */
		_emit 0x0c
		mov word ptr [edi+0x8], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
jmp_1002bb13:
		pop esi
		test esi, 0x1
		_emit 0x74 /* je jmp_1002bb24 */
		_emit 0x08
jmp_1002bb1c:
		mov edi, dword ptr [g_unk0x100666a0+0x1c]
		mov byte ptr [edi], cl
jmp_1002bb24:
		mov eax, dword ptr [g_unk0x100666a0+0xb0]
		mov ebx, dword ptr [g_unk0x100666a0+0xb4]
		mov dword ptr [g_unk0x100666a0+0xb4], eax
		mov dword ptr [g_unk0x100666a0+0xb0], ebx
		mov ebp, dword ptr [g_unk0x100666a0+0xb8]
		pop edi
		pop edx
		pop ecx
		pop ebx
		pop eax
		add edi, dword ptr [g_unk0x100666a0+0xc]
		dec dword ptr [g_unk0x100666a0+0x48]
		_emit 0x78 /* js jmp_1002bb8a */
		_emit 0x37
		_emit 0x74 /* je jmp_1002bb90 */
		_emit 0x3b
		dec dword ptr [g_unk0x100666a0+0x40]
		je jmp_1002bdff
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
jmp_1002bb6d:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002be9d
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002b9e4
jmp_1002bb8a:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1002bb90:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002b9e4
		_emit 0x2e /* mov eax, eax with a segment prefix: MASM alignment padding */
		_emit 0x8b
		_emit 0xc0
jmp_1002bbb0:
		push eax
		push ebx
		push ecx
		push edx
		push edi
		cmp ebx, eax
		_emit 0x7f /* jg jmp_1002bbbc */
		_emit 0x03
		xchg ebx, eax
		_emit 0x87 /* xchg edx, ecx: the inline assembler encodes the operands the other way */
		_emit 0xca
jmp_1002bbbc:
		sar eax, 0x10
		cmp eax, dword ptr [g_unk0x100666a0]
		jg jmp_1002bd55
		sar ebx, 0x10
		cmp ebx, 0x0
		jl jmp_1002bd55
		mov dword ptr [g_unk0x100666a0+0x90], eax
		mov dword ptr [g_unk0x100666a0+0x94], ebx
		add edi, eax
		sub ebx, eax
		mov esi, ebx
		inc esi
		cmp esi, 0x1
		_emit 0x74 /* je jmp_1002bc0b */
		_emit 0x1d
		sub edx, ecx
		sar ebx, 0x1
		cmp ebx, 0x1
		adc ebx, 0x0
		xor eax, eax
		shrd eax, edx, 0x10
		shl ebx, 0x10
		sar edx, 0x10
		idiv ebx
		mov dword ptr [g_unk0x100666a0+0x98], eax
jmp_1002bc0b:
		mov eax, 0x0
		sub eax, dword ptr [g_unk0x100666a0+0x90]
		jg jmp_1002bdd8
		mov dword ptr [g_unk0x100666a0+0xbc], 0x0
jmp_1002bc26:
		mov eax, dword ptr [g_unk0x100666a0+0x94]
		sub eax, dword ptr [g_unk0x100666a0]
		jg jmp_1002bdf8
jmp_1002bc37:
		mov eax, edi
		add eax, esi
		dec eax
		mov dword ptr [g_unk0x100666a0+0x1c], eax
		mov eax, dword ptr [g_unk0x100666a0+0x98]
		shld edx, eax, 0x10
		shl eax, 0x10
		mov ebx, ecx
		mov ebp, ecx
		add ebx, dword ptr [g_unk0x100666a0+0xb0]
		add ebp, dword ptr [g_unk0x100666a0+0xb4]
		mov ecx, ebx
		shr ecx, 0x10
		mov dh, cl
		mov ecx, ebp
		shr ecx, 0x8
		mov cl, dh
		shl ebx, 0x10
		shl ebp, 0x10
		test dword ptr [g_unk0x100666a0+0xbc], 0x1
		_emit 0x75 /* jne jmp_1002bc8f */
		_emit 0x12
		test edi, 0x1
		_emit 0x74 /* je jmp_1002bc97 */
		_emit 0x12
		mov byte ptr [edi], cl
		dec esi
		je jmp_1002bd55
		inc edi
jmp_1002bc8f:
		_emit 0x86 /* xchg ch, cl: the inline assembler encodes the operands the other way */
		_emit 0xcd
		_emit 0x87 /* xchg ebp, ebx: the inline assembler encodes the operands the other way */
		_emit 0xdd
		add ebp, eax
		adc ch, dl
jmp_1002bc97:
		cmp esi, 0x1
		je jmp_1002bd4d
		push esi
		shr esi, 0x1
		dec esi
		cmp esi, 0x5
		_emit 0x7c /* jl jmp_1002bcfd */
		_emit 0x54
jmp_1002bca9:
		mov word ptr [edi], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0x2], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0x4], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0x6], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0x8], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		mov word ptr [edi+0xa], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add edi, 0xc
		sub esi, 0x6
		_emit 0x78 /* js jmp_1002bd44 */
		_emit 0x4c
		cmp esi, 0x5
		_emit 0x7d /* jge jmp_1002bca9 */
		_emit 0xac
jmp_1002bcfd:
		mov word ptr [edi], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002bd44 */
		_emit 0x39
		mov word ptr [edi+0x2], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002bd44 */
		_emit 0x2a
		mov word ptr [edi+0x4], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002bd44 */
		_emit 0x1b
		mov word ptr [edi+0x6], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002bd44 */
		_emit 0x0c
		mov word ptr [edi+0x8], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
jmp_1002bd44:
		pop esi
		test esi, 0x1
		_emit 0x74 /* je jmp_1002bd55 */
		_emit 0x08
jmp_1002bd4d:
		mov edi, dword ptr [g_unk0x100666a0+0x1c]
		mov byte ptr [edi], cl
jmp_1002bd55:
		mov eax, dword ptr [g_unk0x100666a0+0xb0]
		mov ebx, dword ptr [g_unk0x100666a0+0xb4]
		mov dword ptr [g_unk0x100666a0+0xb4], eax
		mov dword ptr [g_unk0x100666a0+0xb0], ebx
		mov ebp, dword ptr [g_unk0x100666a0+0xb8]
		pop edi
		pop edx
		pop ecx
		pop ebx
		pop eax
		add edi, dword ptr [g_unk0x100666a0+0xc]
		dec dword ptr [g_unk0x100666a0+0x48]
		js jmp_1002bb8a
		_emit 0x74 /* je jmp_1002bdbb */
		_emit 0x31
		dec dword ptr [g_unk0x100666a0+0x40]
		_emit 0x74 /* je jmp_1002bdff */
		_emit 0x6d
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
jmp_1002bd9e:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002be9d
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002bbb0
jmp_1002bdbb:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002bbb0
jmp_1002bdd8:
		mov dword ptr [g_unk0x100666a0+0xbc], eax
		add edi, eax
		sub esi, eax
		and eax, 0xfffffffe
		shl eax, 0xf
		imul dword ptr [g_unk0x100666a0+0x98]
		shrd eax, edx, 0x10
		add ecx, eax
		jmp jmp_1002bc26
jmp_1002bdf8:
		sub esi, eax
		jmp jmp_1002bc37
jmp_1002bdff:
		push ebx
		push edx
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002be23 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002be23:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x78], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov ecx, dword ptr [ebx+0x8]
		add ecx, 0x8000
		mov eax, dword ptr [ebx]
		shl eax, 0x10
		add eax, 0x8000
		pop edx
		pop ebx
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		je jmp_1002bb6d
		jmp jmp_1002bd9e
jmp_1002be9d:
		push eax
		push ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002bebe */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002bebe:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x7c], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov edx, dword ptr [ebx+0x8]
		add edx, 0x8000
		mov ebx, dword ptr [ebx]
		shl ebx, 0x10
		add ebx, 0x8000
		pop ecx
		pop eax
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		je jmp_1002b9e4
		jmp jmp_1002bbb0
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_1002bf39(PixelView* p_view, MechS32 p_count, MechS32* p_vertices, undefined4 p_unk0x0c)
{
	STUB(0x1002bf39);
}
#else
// FUNCTION: MW2SHELL 0x1002bf39
__declspec(naked) void FUN_1002bf39(PixelView* p_view, MechS32 p_count, MechS32* p_vertices, undefined4 p_unk0x0c)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov ecx, dword ptr [ebx+0x4]
		inc ecx
		mov dword ptr [g_unk0x100666a0+0xc], ecx
		mov eax, dword ptr [esi+0xc]
		mov ecx, dword ptr [ebx+0x4]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002bf5b */
		_emit 0x02
		mov ecx, eax
jmp_1002bf5b:
		mov eax, dword ptr [esi+0x4]
		mov edi, 0x0
		cmp edi, eax
		_emit 0x7f /* jg jmp_1002bf69 */
		_emit 0x02
		mov edi, eax
jmp_1002bf69:
		sub ecx, edi
		jl jmp_1002c394
		mov dword ptr [g_unk0x100666a0], ecx
		mov eax, dword ptr [esi+0x10]
		mov ecx, dword ptr [ebx+0x8]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002bf83 */
		_emit 0x02
		mov ecx, eax
jmp_1002bf83:
		mov edx, dword ptr [esi+0x8]
		mov eax, 0x0
		cmp eax, edx
		_emit 0x7f /* jg jmp_1002bf91 */
		_emit 0x02
		mov eax, edx
jmp_1002bf91:
		sub ecx, eax
		jl jmp_1002c394
		mov dword ptr [g_unk0x100666a0+0x4], ecx
		mul dword ptr [g_unk0x100666a0+0xc]
		add eax, edi
		add eax, dword ptr [ebx]
		mov dword ptr [g_unk0x100666a0+0x8], eax
		push ds
		pop es
		mov ebx, dword ptr [ebp+0x10]
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		mov edx, eax
		shl eax, 0x1
		add eax, edx
		add eax, ebx
		mov dword ptr [g_unk0x100666a0+0x24], ebx
		mov dword ptr [g_unk0x100666a0+0x28], eax
		mov esi, 0x7fff
		mov edi, 0xffff8000
		mov ecx, 0xf
jmp_1002bfdb:
		mov edx, 0x0
		mov eax, dword ptr [ebx]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0]
		sub eax, dword ptr [ebx]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		cmp eax, esi
		_emit 0x7f /* jg jmp_1002c013 */
		_emit 0x08
		mov esi, eax
		mov dword ptr [g_unk0x100666a0+0x2c], ebx
jmp_1002c013:
		cmp eax, edi
		_emit 0x7c /* jl jmp_1002c019 */
		_emit 0x02
		mov edi, eax
jmp_1002c019:
		and ecx, edx
		add ebx, 0x18
		cmp ebx, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x75 /* jne jmp_1002bfdb */
		_emit 0xb5
		or ecx, ecx
		jne jmp_1002c394
		mov eax, dword ptr [g_unk0x100666a0+0x2c]
		mov dword ptr [g_unk0x100666a0+0x38], eax
		mov dword ptr [g_unk0x100666a0+0x3c], eax
		mov dword ptr [g_unk0x100666a0+0x4c], esi
		cmp edi, esi
		je jmp_1002c394
jmp_1002c04b:
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002c06d */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002c06d:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002c083 */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002c04b */
		_emit 0xc8
jmp_1002c083:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002c04b */
		_emit 0xc4
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x50], edx
jmp_1002c0b8:
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002c0d7 */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002c0d7:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002c0ed */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002c0b8 */
		_emit 0xcb
jmp_1002c0ed:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002c0b8 */
		_emit 0xc7
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x54], edx
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		sub edi, dword ptr [g_unk0x100666a0+0x4]
		_emit 0x7f /* jg jmp_1002c137 */
		_emit 0x02
		add eax, edi
jmp_1002c137:
		mov dword ptr [g_unk0x100666a0+0x48], eax
		mov eax, 0x0
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		_emit 0x7e /* jle jmp_1002c1a5 */
		_emit 0x5c
		sub dword ptr [g_unk0x100666a0+0x48], eax
		mov ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x4c], ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x30]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x40], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x70]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x50], eax
		mov ecx, 0x0
		mov ebx, dword ptr [g_unk0x100666a0+0x34]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x44], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x74]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x54], eax
jmp_1002c1a5:
		mov eax, dword ptr [g_unk0x100666a0+0x4c]
		mul dword ptr [g_unk0x100666a0+0xc]
		add eax, dword ptr [g_unk0x100666a0+0x8]
		mov dword ptr [g_unk0x100666a0+0x18], eax
		mov eax, dword ptr [g_unk0x100666a0+0x50]
		mov ebx, dword ptr [g_unk0x100666a0+0x54]
jmp_1002c1c6:
		push eax
		push ebx
		cmp ebx, eax
		_emit 0x7f /* jg jmp_1002c1cd */
		_emit 0x01
		xchg ebx, eax
jmp_1002c1cd:
		sar eax, 0x10
		cmp eax, dword ptr [g_unk0x100666a0]
		jg jmp_1002c357
		sar ebx, 0x10
		cmp ebx, 0x0
		jl jmp_1002c357
		mov dword ptr [g_unk0x100666a0+0x90], eax
		mov dword ptr [g_unk0x100666a0+0x94], ebx
		sub ebx, eax
		_emit 0x74 /* je jmp_1002c219 */
		_emit 0x22
		mov ecx, 0x0
		sub ecx, dword ptr [g_unk0x100666a0+0x90]
		jg jmp_1002c3ab
jmp_1002c208:
		mov eax, dword ptr [g_unk0x100666a0+0x94]
		sub eax, dword ptr [g_unk0x100666a0]
		jg jmp_1002c3b6
jmp_1002c219:
		mov eax, dword ptr [g_unk0x100666a0+0x90]
		mov edi, dword ptr [g_unk0x100666a0+0x18]
		add edi, eax
		mov ebx, dword ptr [g_unk0x100666a0+0x94]
		sub ebx, eax
		mov eax, edi
		add eax, ebx
		mov dword ptr [g_unk0x100666a0+0x1c], eax
		xor eax, eax
		mov esi, dword ptr [ebp+0x14]
		inc ebx
		test edi, 0x1
		_emit 0x74 /* je jmp_1002c254 */
		_emit 0x0f
		mov al, byte ptr [edi]
		mov dl, byte ptr [eax+esi]
		mov byte ptr [edi], dl
		dec ebx
		je jmp_1002c357
		inc edi
jmp_1002c254:
		cmp ebx, 0x1
		je jmp_1002c34a
		push ebx
		shr ebx, 0x1
		dec ebx
		cmp ebx, 0x5
		_emit 0x7c /* jl jmp_1002c2dd */
		_emit 0x77
jmp_1002c266:
		mov cx, word ptr [edi]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi], dx
		mov cx, word ptr [edi+0x2]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi+0x2], dx
		mov cx, word ptr [edi+0x4]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi+0x4], dx
		mov cx, word ptr [edi+0x6]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi+0x6], dx
		mov cx, word ptr [edi+0x8]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi+0x8], dx
		mov cx, word ptr [edi+0xa]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi+0xa], dx
		add edi, 0xc
		sub ebx, 0x6
		_emit 0x78 /* js jmp_1002c341 */
		_emit 0x69
		cmp ebx, 0x5
		_emit 0x7d /* jge jmp_1002c266 */
		_emit 0x89
jmp_1002c2dd:
		mov cx, word ptr [edi]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi], dx
		dec ebx
		_emit 0x78 /* js jmp_1002c341 */
		_emit 0x51
		mov cx, word ptr [edi+0x2]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi+0x2], dx
		dec ebx
		_emit 0x78 /* js jmp_1002c341 */
		_emit 0x3c
		mov cx, word ptr [edi+0x4]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi+0x4], dx
		dec ebx
		_emit 0x78 /* js jmp_1002c341 */
		_emit 0x27
		mov cx, word ptr [edi+0x6]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi+0x6], dx
		dec ebx
		_emit 0x78 /* js jmp_1002c341 */
		_emit 0x12
		mov cx, word ptr [edi+0x8]
		mov al, cl
		mov dl, byte ptr [eax+esi]
		mov al, ch
		mov dh, byte ptr [eax+esi]
		mov word ptr [edi+0x8], dx
jmp_1002c341:
		pop ebx
		test ebx, 0x1
		_emit 0x74 /* je jmp_1002c357 */
		_emit 0x0d
jmp_1002c34a:
		mov edi, dword ptr [g_unk0x100666a0+0x1c]
		mov al, byte ptr [edi]
		mov dl, byte ptr [eax+esi]
		mov byte ptr [edi], dl
jmp_1002c357:
		mov edi, dword ptr [g_unk0x100666a0+0xc]
		add dword ptr [g_unk0x100666a0+0x18], edi
		pop ebx
		pop eax
		dec dword ptr [g_unk0x100666a0+0x48]
		_emit 0x78 /* js jmp_1002c394 */
		_emit 0x27
		_emit 0x74 /* je jmp_1002c39a */
		_emit 0x2b
		dec dword ptr [g_unk0x100666a0+0x40]
		_emit 0x74 /* je jmp_1002c3c1 */
		_emit 0x4a
		add eax, dword ptr [g_unk0x100666a0+0x70]
jmp_1002c37d:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002c428
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		jmp jmp_1002c1c6
jmp_1002c394:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1002c39a:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		jmp jmp_1002c1c6
jmp_1002c3ab:
		add dword ptr [g_unk0x100666a0+0x90], ecx
		jmp jmp_1002c208
jmp_1002c3b6:
		sub dword ptr [g_unk0x100666a0+0x94], eax
		jmp jmp_1002c219
jmp_1002c3c1:
		push ebx
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002c3e4 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002c3e4:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov eax, dword ptr [ebx]
		shl eax, 0x10
		add eax, 0x8000
		pop ebx
		jmp jmp_1002c37d
jmp_1002c428:
		push eax
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov edi, ebx
		add edi, 0x18
		cmp edi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002c448 */
		_emit 0x06
		mov edi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002c448:
		mov dword ptr [g_unk0x100666a0+0x3c], edi
		mov ecx, dword ptr [edi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [edi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov ebx, dword ptr [ebx]
		shl ebx, 0x10
		add ebx, 0x8000
		pop eax
		jmp jmp_1002c1c6
	}
}
#endif

#ifdef COMPAT_MODE
void FUN_1002c48d(PixelView* p_view, MechS32 p_count, MechS32* p_vertices, undefined4 p_unk0x0c)
{
	STUB(0x1002c48d);
}
#else
// FUNCTION: MW2SHELL 0x1002c48d
__declspec(naked) void FUN_1002c48d(PixelView* p_view, MechS32 p_count, MechS32* p_vertices, undefined4 p_unk0x0c)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov ecx, dword ptr [ebx+0x4]
		inc ecx
		mov dword ptr [g_unk0x100666a0+0xc], ecx
		mov eax, dword ptr [esi+0xc]
		mov ecx, dword ptr [ebx+0x4]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002c4af */
		_emit 0x02
		mov ecx, eax
jmp_1002c4af:
		mov eax, dword ptr [esi+0x4]
		mov edi, 0x0
		cmp edi, eax
		_emit 0x7f /* jg jmp_1002c4bd */
		_emit 0x02
		mov edi, eax
jmp_1002c4bd:
		sub ecx, edi
		jl jmp_1002c98e
		mov dword ptr [g_unk0x100666a0], ecx
		mov eax, dword ptr [esi+0x10]
		mov ecx, dword ptr [ebx+0x8]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002c4d7 */
		_emit 0x02
		mov ecx, eax
jmp_1002c4d7:
		mov edx, dword ptr [esi+0x8]
		mov eax, 0x0
		cmp eax, edx
		_emit 0x7f /* jg jmp_1002c4e5 */
		_emit 0x02
		mov eax, edx
jmp_1002c4e5:
		sub ecx, eax
		jl jmp_1002c98e
		mov dword ptr [g_unk0x100666a0+0x4], ecx
		mul dword ptr [g_unk0x100666a0+0xc]
		add eax, edi
		add eax, dword ptr [ebx]
		mov dword ptr [g_unk0x100666a0+0x8], eax
		push ds
		pop es
		mov dword ptr [g_unk0x100666a0+0xb8], ebp
		mov ebx, dword ptr [ebp+0x14]
		mov eax, dword ptr [ebp+0x10]
		shl eax, 0x3
		mov edx, eax
		shl eax, 0x1
		add eax, edx
		add eax, ebx
		mov dword ptr [g_unk0x100666a0+0x24], ebx
		mov dword ptr [g_unk0x100666a0+0x28], eax
		mov dword ptr [g_unk0x100666a0+0x20], 0x0
		mov esi, 0x7fff
		mov edi, 0xffff8000
		mov ecx, 0xf
jmp_1002c53f:
		mov edx, 0x0
		mov eax, dword ptr [ebx]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0]
		sub eax, dword ptr [ebx]
		shld edx, eax, 0x1
		or dword ptr [g_unk0x100666a0+0x20], edx
		mov eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		cmp eax, esi
		_emit 0x7f /* jg jmp_1002c57d */
		_emit 0x08
		mov esi, eax
		mov dword ptr [g_unk0x100666a0+0x2c], ebx
jmp_1002c57d:
		cmp eax, edi
		_emit 0x7c /* jl jmp_1002c583 */
		_emit 0x02
		mov edi, eax
jmp_1002c583:
		and ecx, edx
		add ebx, 0x18
		cmp ebx, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x75 /* jne jmp_1002c53f */
		_emit 0xaf
		or ecx, ecx
		jne jmp_1002c98e
		mov eax, dword ptr [g_unk0x100666a0+0x2c]
		mov dword ptr [g_unk0x100666a0+0x38], eax
		mov dword ptr [g_unk0x100666a0+0x3c], eax
		mov dword ptr [g_unk0x100666a0+0x4c], esi
		cmp edi, esi
		je jmp_1002c98e
jmp_1002c5b5:
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002c5d7 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002c5d7:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002c5ed */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002c5b5 */
		_emit 0xc8
jmp_1002c5ed:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002c5b5 */
		_emit 0xc4
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x78], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x50], edx
		mov edx, dword ptr [ebx+0x8]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x58], edx
jmp_1002c650:
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002c66f */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002c66f:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002c685 */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002c650 */
		_emit 0xcb
jmp_1002c685:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002c650 */
		_emit 0xc7
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x7c], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x54], edx
		mov edx, dword ptr [ebx+0x8]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x5c], edx
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		sub edi, dword ptr [g_unk0x100666a0+0x4]
		_emit 0x7f /* jg jmp_1002c6fd */
		_emit 0x02
		add eax, edi
jmp_1002c6fd:
		mov dword ptr [g_unk0x100666a0+0x48], eax
		mov esi, dword ptr [ebp+0xc]
		mov edi, 0x0
		mov eax, 0x0
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		jle jmp_1002c7a2
		sub dword ptr [g_unk0x100666a0+0x48], eax
		test eax, 0x1
		_emit 0x74 /* je jmp_1002c72a */
		_emit 0x02
		_emit 0x87 /* xchg edi, esi: the inline assembler encodes the operands the other way */
		_emit 0xf7
jmp_1002c72a:
		mov ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x4c], ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x30]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x40], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x70]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x50], eax
		mov eax, dword ptr [g_unk0x100666a0+0x78]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x58], eax
		mov ecx, 0x0
		mov ebx, dword ptr [g_unk0x100666a0+0x34]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x44], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x74]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x54], eax
		mov eax, dword ptr [g_unk0x100666a0+0x7c]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x5c], eax
jmp_1002c7a2:
		mov dword ptr [g_unk0x100666a0+0xb0], esi
		mov dword ptr [g_unk0x100666a0+0xb4], edi
		mov eax, dword ptr [g_unk0x100666a0+0x4c]
		mul dword ptr [g_unk0x100666a0+0xc]
		mov edi, dword ptr [g_unk0x100666a0+0x8]
		add edi, eax
		mov eax, dword ptr [g_unk0x100666a0+0x50]
		mov ebx, dword ptr [g_unk0x100666a0+0x54]
		mov ecx, dword ptr [g_unk0x100666a0+0x58]
		mov edx, dword ptr [g_unk0x100666a0+0x5c]
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		jne jmp_1002c9b4
		_emit 0x2e /* mov eax, eax with a segment prefix: MASM alignment padding */
		_emit 0x8b
		_emit 0xc0
jmp_1002c7e8:
		push eax
		push ebx
		push ecx
		push edx
		push edi
		cmp ebx, eax
		_emit 0x7f /* jg jmp_1002c7f4 */
		_emit 0x03
		xchg ebx, eax
		_emit 0x87 /* xchg edx, ecx: the inline assembler encodes the operands the other way */
		_emit 0xca
jmp_1002c7f4:
		sar eax, 0x10
		sar ebx, 0x10
		add edi, eax
		sub ebx, eax
		mov esi, ebx
		_emit 0x74 /* je jmp_1002c820 */
		_emit 0x1e
		sub edx, ecx
		sar ebx, 0x1
		cmp ebx, 0x1
		adc ebx, 0x0
		xor eax, eax
		shrd eax, edx, 0x10
		shl ebx, 0x10
		sar edx, 0x10
		idiv ebx
		shld edx, eax, 0x10
		mov ebx, esi
jmp_1002c820:
		shl eax, 0x10
		add ebx, edi
		mov dword ptr [g_unk0x100666a0+0x1c], ebx
		inc esi
		mov ebx, ecx
		mov ebp, ecx
		add ebx, dword ptr [g_unk0x100666a0+0xb0]
		add ebp, dword ptr [g_unk0x100666a0+0xb4]
		mov ecx, ebx
		shr ecx, 0x10
		mov dh, cl
		mov ecx, ebp
		shr ecx, 0x8
		mov cl, dh
		shl ebx, 0x10
		shl ebp, 0x10
		test edi, 0x1
		_emit 0x74 /* je jmp_1002c86a */
		_emit 0x12
		add byte ptr [edi], cl
		dec esi
		je jmp_1002c928
		inc edi
		_emit 0x86 /* xchg ch, cl: the inline assembler encodes the operands the other way */
		_emit 0xcd
		_emit 0x87 /* xchg ebp, ebx: the inline assembler encodes the operands the other way */
		_emit 0xdd
		add ebp, eax
		adc ch, dl
jmp_1002c86a:
		cmp esi, 0x1
		je jmp_1002c920
		push esi
		shr esi, 0x1
		dec esi
		cmp esi, 0x5
		_emit 0x7c /* jl jmp_1002c8d0 */
		_emit 0x54
jmp_1002c87c:
		add word ptr [edi], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0x2], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0x4], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0x6], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0x8], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0xa], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add edi, 0xc
		sub esi, 0x6
		_emit 0x78 /* js jmp_1002c917 */
		_emit 0x4c
		cmp esi, 0x5
		_emit 0x7d /* jge jmp_1002c87c */
		_emit 0xac
jmp_1002c8d0:
		add word ptr [edi], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002c917 */
		_emit 0x39
		add word ptr [edi+0x2], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002c917 */
		_emit 0x2a
		add word ptr [edi+0x4], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002c917 */
		_emit 0x1b
		add word ptr [edi+0x6], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002c917 */
		_emit 0x0c
		add word ptr [edi+0x8], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
jmp_1002c917:
		pop esi
		test esi, 0x1
		_emit 0x74 /* je jmp_1002c928 */
		_emit 0x08
jmp_1002c920:
		mov edi, dword ptr [g_unk0x100666a0+0x1c]
		add byte ptr [edi], cl
jmp_1002c928:
		mov eax, dword ptr [g_unk0x100666a0+0xb0]
		mov ebx, dword ptr [g_unk0x100666a0+0xb4]
		mov dword ptr [g_unk0x100666a0+0xb4], eax
		mov dword ptr [g_unk0x100666a0+0xb0], ebx
		mov ebp, dword ptr [g_unk0x100666a0+0xb8]
		pop edi
		pop edx
		pop ecx
		pop ebx
		pop eax
		add edi, dword ptr [g_unk0x100666a0+0xc]
		dec dword ptr [g_unk0x100666a0+0x48]
		_emit 0x78 /* js jmp_1002c98e */
		_emit 0x37
		_emit 0x74 /* je jmp_1002c994 */
		_emit 0x3b
		dec dword ptr [g_unk0x100666a0+0x40]
		je jmp_1002cc03
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
jmp_1002c971:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002cca1
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002c7e8
jmp_1002c98e:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1002c994:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002c7e8
		_emit 0x2e /* mov eax, eax with a segment prefix: MASM alignment padding */
		_emit 0x8b
		_emit 0xc0
jmp_1002c9b4:
		push eax
		push ebx
		push ecx
		push edx
		push edi
		cmp ebx, eax
		_emit 0x7f /* jg jmp_1002c9c0 */
		_emit 0x03
		xchg ebx, eax
		_emit 0x87 /* xchg edx, ecx: the inline assembler encodes the operands the other way */
		_emit 0xca
jmp_1002c9c0:
		sar eax, 0x10
		cmp eax, dword ptr [g_unk0x100666a0]
		jg jmp_1002cb59
		sar ebx, 0x10
		cmp ebx, 0x0
		jl jmp_1002cb59
		mov dword ptr [g_unk0x100666a0+0x90], eax
		mov dword ptr [g_unk0x100666a0+0x94], ebx
		add edi, eax
		sub ebx, eax
		mov esi, ebx
		inc esi
		cmp esi, 0x1
		_emit 0x74 /* je jmp_1002ca0f */
		_emit 0x1d
		sub edx, ecx
		sar ebx, 0x1
		cmp ebx, 0x1
		adc ebx, 0x0
		xor eax, eax
		shrd eax, edx, 0x10
		shl ebx, 0x10
		sar edx, 0x10
		idiv ebx
		mov dword ptr [g_unk0x100666a0+0x98], eax
jmp_1002ca0f:
		mov eax, 0x0
		sub eax, dword ptr [g_unk0x100666a0+0x90]
		jg jmp_1002cbdc
		mov dword ptr [g_unk0x100666a0+0xbc], 0x0
jmp_1002ca2a:
		mov eax, dword ptr [g_unk0x100666a0+0x94]
		sub eax, dword ptr [g_unk0x100666a0]
		jg jmp_1002cbfc
jmp_1002ca3b:
		mov eax, edi
		add eax, esi
		dec eax
		mov dword ptr [g_unk0x100666a0+0x1c], eax
		mov eax, dword ptr [g_unk0x100666a0+0x98]
		shld edx, eax, 0x10
		shl eax, 0x10
		mov ebx, ecx
		mov ebp, ecx
		add ebx, dword ptr [g_unk0x100666a0+0xb0]
		add ebp, dword ptr [g_unk0x100666a0+0xb4]
		mov ecx, ebx
		shr ecx, 0x10
		mov dh, cl
		mov ecx, ebp
		shr ecx, 0x8
		mov cl, dh
		shl ebx, 0x10
		shl ebp, 0x10
		test dword ptr [g_unk0x100666a0+0xbc], 0x1
		_emit 0x75 /* jne jmp_1002ca93 */
		_emit 0x12
		test edi, 0x1
		_emit 0x74 /* je jmp_1002ca9b */
		_emit 0x12
		add byte ptr [edi], cl
		dec esi
		je jmp_1002cb59
		inc edi
jmp_1002ca93:
		_emit 0x86 /* xchg ch, cl: the inline assembler encodes the operands the other way */
		_emit 0xcd
		_emit 0x87 /* xchg ebp, ebx: the inline assembler encodes the operands the other way */
		_emit 0xdd
		add ebp, eax
		adc ch, dl
jmp_1002ca9b:
		cmp esi, 0x1
		je jmp_1002cb51
		push esi
		shr esi, 0x1
		dec esi
		cmp esi, 0x5
		_emit 0x7c /* jl jmp_1002cb01 */
		_emit 0x54
jmp_1002caad:
		add word ptr [edi], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0x2], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0x4], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0x6], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0x8], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add word ptr [edi+0xa], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		add edi, 0xc
		sub esi, 0x6
		_emit 0x78 /* js jmp_1002cb48 */
		_emit 0x4c
		cmp esi, 0x5
		_emit 0x7d /* jge jmp_1002caad */
		_emit 0xac
jmp_1002cb01:
		add word ptr [edi], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002cb48 */
		_emit 0x39
		add word ptr [edi+0x2], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002cb48 */
		_emit 0x2a
		add word ptr [edi+0x4], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002cb48 */
		_emit 0x1b
		add word ptr [edi+0x6], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
		dec esi
		_emit 0x78 /* js jmp_1002cb48 */
		_emit 0x0c
		add word ptr [edi+0x8], cx
		add ebx, eax
		adc cl, dl
		add ebp, eax
		adc ch, dl
jmp_1002cb48:
		pop esi
		test esi, 0x1
		_emit 0x74 /* je jmp_1002cb59 */
		_emit 0x08
jmp_1002cb51:
		mov edi, dword ptr [g_unk0x100666a0+0x1c]
		add byte ptr [edi], cl
jmp_1002cb59:
		mov eax, dword ptr [g_unk0x100666a0+0xb0]
		mov ebx, dword ptr [g_unk0x100666a0+0xb4]
		mov dword ptr [g_unk0x100666a0+0xb4], eax
		mov dword ptr [g_unk0x100666a0+0xb0], ebx
		mov ebp, dword ptr [g_unk0x100666a0+0xb8]
		pop edi
		pop edx
		pop ecx
		pop ebx
		pop eax
		add edi, dword ptr [g_unk0x100666a0+0xc]
		dec dword ptr [g_unk0x100666a0+0x48]
		js jmp_1002c98e
		_emit 0x74 /* je jmp_1002cbbf */
		_emit 0x31
		dec dword ptr [g_unk0x100666a0+0x40]
		_emit 0x74 /* je jmp_1002cc03 */
		_emit 0x6d
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
jmp_1002cba2:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002cca1
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002c9b4
jmp_1002cbbf:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x78]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x7c]
		jmp jmp_1002c9b4
jmp_1002cbdc:
		mov dword ptr [g_unk0x100666a0+0xbc], eax
		add edi, eax
		sub esi, eax
		and eax, 0xfffffffe
		shl eax, 0xf
		imul dword ptr [g_unk0x100666a0+0x98]
		shrd eax, edx, 0x10
		add ecx, eax
		jmp jmp_1002ca2a
jmp_1002cbfc:
		sub esi, eax
		jmp jmp_1002ca3b
jmp_1002cc03:
		push ebx
		push edx
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002cc27 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002cc27:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x78], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov ecx, dword ptr [ebx+0x8]
		add ecx, 0x8000
		mov eax, dword ptr [ebx]
		shl eax, 0x10
		add eax, 0x8000
		pop edx
		pop ebx
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		je jmp_1002c971
		jmp jmp_1002cba2
jmp_1002cca1:
		push eax
		push ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002ccc2 */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002ccc2:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi+0x8]
		sub edx, dword ptr [ebx+0x8]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x7c], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov edx, dword ptr [ebx+0x8]
		add edx, 0x8000
		mov ebx, dword ptr [ebx]
		shl ebx, 0x10
		add ebx, 0x8000
		pop ecx
		pop eax
		cmp dword ptr [g_unk0x100666a0+0x20], 0x0
		je jmp_1002c7e8
		jmp jmp_1002c9b4
	}
}
#endif

// Copies a 0x40-dword table into the working variables.
#ifdef COMPAT_MODE
void FUN_1002cd3d(undefined4* p_table)
{
	STUB(0x1002cd3d);
}
#else
// FUNCTION: MW2SHELL 0x1002cd3d
__declspec(naked) void FUN_1002cd3d(undefined4* p_table)
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
		lea edi, [g_unk0x100666a0+0xd0]
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

// Fills a textured polygon, clipped to the view: p_unk0x10 points at the texture (its pixels and
// width minus one), and p_mode picks the span routine from g_unk0x1002d2a0. It ends by jumping
// to that routine; the routines jump back into it for each scan line.
#ifdef COMPAT_MODE
void FUN_1002cd5d(PixelView* p_view, MechS32 p_count, MechS32* p_vertices, undefined4* p_unk0x10, MechS32 p_mode)
{
	STUB(0x1002cd5d);
}
#else
// FUNCTION: MW2SHELL 0x1002cd5d
__declspec(naked) void FUN_1002cd5d(
	PixelView* p_view,
	MechS32 p_count,
	MechS32* p_vertices,
	undefined4* p_unk0x10,
	MechS32 p_mode
)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov esi, dword ptr [ebp+0x8]
		mov ebx, dword ptr [esi]
		mov ecx, dword ptr [ebx+0x4]
		inc ecx
		mov dword ptr [g_unk0x100666a0+0xc], ecx
		mov eax, dword ptr [esi+0xc]
		mov ecx, dword ptr [ebx+0x4]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002cd7f */
		_emit 0x02
		mov ecx, eax
jmp_1002cd7f:
		mov eax, dword ptr [esi+0x4]
		mov edi, 0x0
		cmp edi, eax
		_emit 0x7f /* jg jmp_1002cd8d */
		_emit 0x02
		mov edi, eax
jmp_1002cd8d:
		sub ecx, edi
		_emit 0x0f /* jl 0x1002d8aa, outside this routine */
		_emit 0x8c
		_emit 0x15
		_emit 0x0b
		_emit 0x00
		_emit 0x00
		mov dword ptr [g_unk0x100666a0], ecx
		mov eax, dword ptr [esi+0x10]
		mov ecx, dword ptr [ebx+0x8]
		cmp ecx, eax
		_emit 0x7c /* jl jmp_1002cda7 */
		_emit 0x02
		mov ecx, eax
jmp_1002cda7:
		mov edx, dword ptr [esi+0x8]
		mov eax, 0x0
		cmp eax, edx
		_emit 0x7f /* jg jmp_1002cdb5 */
		_emit 0x02
		mov eax, edx
jmp_1002cdb5:
		sub ecx, eax
		_emit 0x0f /* jl 0x1002d8aa, outside this routine */
		_emit 0x8c
		_emit 0xed
		_emit 0x0a
		_emit 0x00
		_emit 0x00
		mov dword ptr [g_unk0x100666a0+0x4], ecx
		mul dword ptr [g_unk0x100666a0+0xc]
		add eax, edi
		add eax, dword ptr [ebx]
		mov dword ptr [g_unk0x100666a0+0x8], eax
		mov ebx, dword ptr [ebp+0x14]
		mov eax, dword ptr [ebx]
		mov dword ptr [g_unk0x100666a0+0x14], eax
		mov ecx, dword ptr [ebx+0x4]
		inc ecx
		mov dword ptr [g_unk0x100666a0+0x10], ecx
		push ds
		pop es
		mov ebx, dword ptr [ebp+0x10]
		mov eax, dword ptr [ebp+0xc]
		shl eax, 0x3
		mov edx, eax
		shl eax, 0x1
		add eax, edx
		add eax, ebx
		mov dword ptr [g_unk0x100666a0+0x24], ebx
		mov dword ptr [g_unk0x100666a0+0x28], eax
		mov esi, 0x7fff
		mov edi, 0xffff8000
		mov ecx, 0xf
jmp_1002ce13:
		mov edx, 0x0
		mov eax, dword ptr [ebx]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0]
		sub eax, dword ptr [ebx]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [ebx+0x4]
		shld edx, eax, 0x1
		mov eax, dword ptr [ebx+0x4]
		cmp eax, esi
		_emit 0x7f /* jg jmp_1002ce4b */
		_emit 0x08
		mov esi, eax
		mov dword ptr [g_unk0x100666a0+0x2c], ebx
jmp_1002ce4b:
		cmp eax, edi
		_emit 0x7c /* jl jmp_1002ce51 */
		_emit 0x02
		mov edi, eax
jmp_1002ce51:
		and ecx, edx
		add ebx, 0x18
		cmp ebx, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x75 /* jne jmp_1002ce13 */
		_emit 0xb5
		or ecx, ecx
		_emit 0x0f /* jne 0x1002d8aa, outside this routine */
		_emit 0x85
		_emit 0x44
		_emit 0x0a
		_emit 0x00
		_emit 0x00
		mov eax, dword ptr [g_unk0x100666a0+0x2c]
		mov dword ptr [g_unk0x100666a0+0x38], eax
		mov dword ptr [g_unk0x100666a0+0x3c], eax
		mov dword ptr [g_unk0x100666a0+0x4c], esi
		cmp edi, esi
		_emit 0x0f /* je 0x1002d8aa, outside this routine */
		_emit 0x84
		_emit 0x27
		_emit 0x0a
		_emit 0x00
		_emit 0x00
jmp_1002ce83:
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002cea5 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002cea5:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002cebb */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002ce83 */
		_emit 0xc8
jmp_1002cebb:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002ce83 */
		_emit 0xc4
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi+0xc]
		sub edx, dword ptr [ebx+0xc]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x80], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi+0x10]
		sub edx, dword ptr [ebx+0x10]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x88], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x50], edx
		mov edx, dword ptr [ebx+0xc]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x60], edx
		mov edx, dword ptr [ebx+0x10]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x68], edx
jmp_1002cf4c:
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov esi, ebx
		add esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002cf6b */
		_emit 0x06
		mov esi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002cf6b:
		mov dword ptr [g_unk0x100666a0+0x3c], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		cmp edx, 0x0
		_emit 0x7d /* jge jmp_1002cf81 */
		_emit 0x05
		cmp ecx, 0x0
		_emit 0x7e /* jle jmp_1002cf4c */
		_emit 0xcb
jmp_1002cf81:
		sub ecx, edx
		_emit 0x74 /* je jmp_1002cf4c */
		_emit 0xc7
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [esi+0xc]
		sub edx, dword ptr [ebx+0xc]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x84], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [esi+0x10]
		sub edx, dword ptr [ebx+0x10]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x8c], eax
		mov edx, dword ptr [ebx]
		shl edx, 0x10
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x54], edx
		mov edx, dword ptr [ebx+0xc]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x64], edx
		mov edx, dword ptr [ebx+0x10]
		add edx, 0x8000
		mov dword ptr [g_unk0x100666a0+0x6c], edx
		mov eax, dword ptr [g_unk0x100666a0+0x4]
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		sub edi, dword ptr [g_unk0x100666a0+0x4]
		_emit 0x7f /* jg jmp_1002d027 */
		_emit 0x02
		add eax, edi
jmp_1002d027:
		mov dword ptr [g_unk0x100666a0+0x48], eax
		mov eax, 0x0
		sub eax, dword ptr [g_unk0x100666a0+0x4c]
		jle jmp_1002d0dd
		sub dword ptr [g_unk0x100666a0+0x48], eax
		mov ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x4c], ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x30]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x40], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x70]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x50], eax
		mov eax, dword ptr [g_unk0x100666a0+0x80]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x60], eax
		mov eax, dword ptr [g_unk0x100666a0+0x88]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x68], eax
		mov ecx, 0x0
		mov ebx, dword ptr [g_unk0x100666a0+0x34]
		sub ecx, dword ptr [ebx+0x4]
		sub dword ptr [g_unk0x100666a0+0x44], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x74]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x54], eax
		mov eax, dword ptr [g_unk0x100666a0+0x84]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x64], eax
		mov eax, dword ptr [g_unk0x100666a0+0x8c]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0x6c], eax
jmp_1002d0dd:
		mov eax, dword ptr [g_unk0x100666a0+0x4c]
		mul dword ptr [g_unk0x100666a0+0xc]
		add eax, dword ptr [g_unk0x100666a0+0x8]
		mov dword ptr [g_unk0x100666a0+0x18], eax
		mov eax, dword ptr [ebp+0x18]
		mov eax, dword ptr [g_unk0x1002d2a0+eax*0x4]
		mov dword ptr [g_unk0x100666a0+0xac], eax
		mov eax, dword ptr [g_unk0x100666a0+0x50]
		mov ebx, dword ptr [g_unk0x100666a0+0x54]
		mov ecx, dword ptr [g_unk0x100666a0+0x60]
		mov edx, dword ptr [g_unk0x100666a0+0x64]
		mov esi, dword ptr [g_unk0x100666a0+0x68]
		mov edi, dword ptr [g_unk0x100666a0+0x6c]
		push eax
		push ebx
		push ecx
		push edx
		push esi
		push edi
		cmp ebx, eax
		_emit 0x7f /* jg jmp_1002d134 */
		_emit 0x05
		xchg ebx, eax
		_emit 0x87 /* xchg edx, ecx: the inline assembler encodes the operands the other way */
		_emit 0xca
		_emit 0x87 /* xchg edi, esi: the inline assembler encodes the operands the other way */
		_emit 0xf7
jmp_1002d134:
		sar eax, 0x10
		cmp eax, dword ptr [g_unk0x100666a0]
		_emit 0x0f /* jg 0x1002d84d, outside this routine */
		_emit 0x8f
		_emit 0x0a
		_emit 0x07
		_emit 0x00
		_emit 0x00
		sar ebx, 0x10
		cmp ebx, 0x0
		_emit 0x0f /* jl 0x1002d84d, outside this routine */
		_emit 0x8c
		_emit 0xfe
		_emit 0x06
		_emit 0x00
		_emit 0x00
		mov dword ptr [g_unk0x100666a0+0x90], eax
		mov dword ptr [g_unk0x100666a0+0x94], ebx
		mov dword ptr [g_unk0x100666a0+0xa4], ecx
		sub ebx, eax
		je jmp_1002d236
		push ebx
		sub edx, ecx
		xor eax, eax
		shrd eax, edx, 0x10
		shl ebx, 0x10
		sar edx, 0x10
		idiv ebx
		mov dword ptr [g_unk0x100666a0+0x9c], eax
		shld edx, eax, 0x10
		pop ebx
		and eax, 0xffff
		and edx, 0xffff
		mov ecx, 0x1
		test edx, 0x8000
		_emit 0x74 /* je jmp_1002d1a9 */
		_emit 0x0e
		or edx, 0xffff0000
		neg ecx
		cmp eax, 0x1
		sbb edx, -0x1
jmp_1002d1a9:
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
		mov dword ptr [g_unk0x100666a0+0xa0], eax
		shld edx, eax, 0x10
		and eax, 0xffff
		and edx, 0xffff
		mov ecx, dword ptr [g_unk0x100666a0+0x10]
		test edx, 0x8000
		_emit 0x74 /* je jmp_1002d1e9 */
		_emit 0x08
		neg ecx
		cmp eax, 0x1
		sbb edx, -0x1
jmp_1002d1e9:
		mov eax, dword ptr [g_unk0x100666a0+0x10]
		imul dx
		cwde
		pop edx
		pop ebx
		add edx, eax
		mov dword ptr [g_unk0x100666a0+0xc0], edx
		add edx, ecx
		mov dword ptr [g_unk0x100666a0+0xc4], edx
		add ebx, eax
		mov dword ptr [g_unk0x100666a0+0xc8], ebx
		add ebx, ecx
		mov dword ptr [g_unk0x100666a0+0xcc], ebx
		mov ecx, 0x0
		sub ecx, dword ptr [g_unk0x100666a0+0x90]
		_emit 0x0f /* jg 0x1002d8d9, outside this routine */
		_emit 0x8f
		_emit 0xb4
		_emit 0x06
		_emit 0x00
		_emit 0x00
		mov eax, dword ptr [g_unk0x100666a0+0x94]
		sub eax, dword ptr [g_unk0x100666a0]
		_emit 0x0f /* jg 0x1002d905, outside this routine */
		_emit 0x8f
		_emit 0xcf
		_emit 0x06
		_emit 0x00
		_emit 0x00
jmp_1002d236:
		mov ecx, esi
		shr esi, 0x10
		mov eax, esi
		mul dword ptr [g_unk0x100666a0+0x10]
		add eax, dword ptr [g_unk0x100666a0+0x14]
		mov esi, dword ptr [g_unk0x100666a0+0xa4]
		shr esi, 0x10
		add esi, eax
		mov eax, dword ptr [g_unk0x100666a0+0x90]
		mov edi, dword ptr [g_unk0x100666a0+0x18]
		add edi, eax
		mov ebx, dword ptr [g_unk0x100666a0+0x94]
		sub ebx, eax
		push ebp
		mov edx, dword ptr [g_unk0x100666a0+0xa4]
		mov eax, dword ptr [g_unk0x100666a0+0x9c]
		or eax, eax
		_emit 0x79 /* jns jmp_1002d27d */
		_emit 0x04
		neg eax
		not edx
jmp_1002d27d:
		shl eax, 0x10
		shl edx, 0x10
		mov ebp, dword ptr [g_unk0x100666a0+0xa0]
		or ebp, ebp
		_emit 0x79 /* jns jmp_1002d291 */
		_emit 0x04
		neg ebp
		not ecx
jmp_1002d291:
		shl ebp, 0x10
		shl ecx, 0x10
		push ebx
		xor ebx, ebx
		jmp dword ptr [g_unk0x100666a0+0xac]
	}
}
#endif

// The 16 bytes where the original keeps g_unk0x1002d2a0, so that the span routines' jumps into
// FUN_1002cd5d and back keep their displacements.
#ifndef COMPAT_MODE
__declspec(naked) void FUN_1002d2a0(void)
{
	__asm {
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
		_emit 0x00
	}
}
#endif

// A span routine of FUN_1002cd5d (a g_unk0x1002d2a0 entry); register-based, entered by jmp.
#ifdef COMPAT_MODE
void FUN_1002d2b0(void)
{
	STUB(0x1002d2b0);
}
#else
// FUNCTION: MW2SHELL 0x1002d2b0
__declspec(naked) void FUN_1002d2b0(void)
{
	__asm {
		cmp dword ptr [esp], 0x5
		jl jmp_1002d396
jmp_1002d2ba:
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d2c9 */
		_emit 0x02
		mov byte ptr [edi], bl
jmp_1002d2c9:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d2ea */
		_emit 0x03
		mov byte ptr [edi+0x1], bl
jmp_1002d2ea:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d30b */
		_emit 0x03
		mov byte ptr [edi+0x2], bl
jmp_1002d30b:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d32c */
		_emit 0x03
		mov byte ptr [edi+0x3], bl
jmp_1002d32c:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d34d */
		_emit 0x03
		mov byte ptr [edi+0x4], bl
jmp_1002d34d:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d36e */
		_emit 0x03
		mov byte ptr [edi+0x5], bl
jmp_1002d36e:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		add edi, 0x6
		sub dword ptr [esp], 0x6
		js jmp_1002d452
		cmp dword ptr [esp], 0x5
		jge jmp_1002d2ba
jmp_1002d396:
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d3a5 */
		_emit 0x02
		mov byte ptr [edi], bl
jmp_1002d3a5:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		js jmp_1002d452
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d3cf */
		_emit 0x03
		mov byte ptr [edi+0x1], bl
jmp_1002d3cf:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d452 */
		_emit 0x6d
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d3f5 */
		_emit 0x03
		mov byte ptr [edi+0x2], bl
jmp_1002d3f5:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d452 */
		_emit 0x47
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d41b */
		_emit 0x03
		mov byte ptr [edi+0x3], bl
jmp_1002d41b:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d452 */
		_emit 0x21
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d441 */
		_emit 0x03
		mov byte ptr [edi+0x4], bl
jmp_1002d441:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
jmp_1002d452:
		_emit 0xe9 /* jmp 0x1002d849, outside this routine */
		_emit 0xf2
		_emit 0x03
		_emit 0x00
		_emit 0x00
	}
}
#endif

// A span routine of FUN_1002cd5d (a g_unk0x1002d2a0 entry); register-based, entered by jmp.
#ifdef COMPAT_MODE
void FUN_1002d457(void)
{
	STUB(0x1002d457);
}
#else
// FUNCTION: MW2SHELL 0x1002d457
__declspec(naked) void FUN_1002d457(void)
{
	__asm {
		cmp dword ptr [esp], 0x5
		jl jmp_1002d51f
jmp_1002d461:
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi+0x1], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi+0x2], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi+0x3], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi+0x4], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi+0x5], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		add edi, 0x6
		sub dword ptr [esp], 0x6
		js jmp_1002d5be
		cmp dword ptr [esp], 0x5
		jge jmp_1002d461
jmp_1002d51f:
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d5be */
		_emit 0x7f
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi+0x1], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d5be */
		_emit 0x5e
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi+0x2], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d5be */
		_emit 0x3d
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi+0x3], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d5be */
		_emit 0x1c
		mov bl, byte ptr [esi]
		mov bl, byte ptr [g_unk0x100666a0+ebx+0xd0]
		mov byte ptr [edi+0x4], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
jmp_1002d5be:
		_emit 0xe9 /* jmp 0x1002d849, outside this routine */
		_emit 0x86
		_emit 0x02
		_emit 0x00
		_emit 0x00
	}
}
#endif

// A span routine of FUN_1002cd5d (a g_unk0x1002d2a0 entry); register-based, entered by jmp.
#ifdef COMPAT_MODE
void FUN_1002d5c3(void)
{
	STUB(0x1002d5c3);
}
#else
// FUNCTION: MW2SHELL 0x1002d5c3
__declspec(naked) void FUN_1002d5c3(void)
{
	__asm {
		cmp dword ptr [esp], 0x5
		jl jmp_1002d685
jmp_1002d5cd:
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d5d6 */
		_emit 0x02
		mov byte ptr [edi], bl
jmp_1002d5d6:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d5f1 */
		_emit 0x03
		mov byte ptr [edi+0x1], bl
jmp_1002d5f1:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d60c */
		_emit 0x03
		mov byte ptr [edi+0x2], bl
jmp_1002d60c:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d627 */
		_emit 0x03
		mov byte ptr [edi+0x3], bl
jmp_1002d627:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d642 */
		_emit 0x03
		mov byte ptr [edi+0x4], bl
jmp_1002d642:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d65d */
		_emit 0x03
		mov byte ptr [edi+0x5], bl
jmp_1002d65d:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		add edi, 0x6
		sub dword ptr [esp], 0x6
		js jmp_1002d71f
		cmp dword ptr [esp], 0x5
		jge jmp_1002d5cd
jmp_1002d685:
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d68e */
		_emit 0x02
		mov byte ptr [edi], bl
jmp_1002d68e:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d71f */
		_emit 0x7b
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d6ae */
		_emit 0x03
		mov byte ptr [edi+0x1], bl
jmp_1002d6ae:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d71f */
		_emit 0x5b
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d6ce */
		_emit 0x03
		mov byte ptr [edi+0x2], bl
jmp_1002d6ce:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d71f */
		_emit 0x3b
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d6ee */
		_emit 0x03
		mov byte ptr [edi+0x3], bl
jmp_1002d6ee:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d71f */
		_emit 0x1b
		mov bl, byte ptr [esi]
		cmp bl, 0xff
		_emit 0x74 /* je jmp_1002d70e */
		_emit 0x03
		mov byte ptr [edi+0x4], bl
jmp_1002d70e:
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
jmp_1002d71f:
		_emit 0xe9 /* jmp 0x1002d849, outside this routine */
		_emit 0x25
		_emit 0x01
		_emit 0x00
		_emit 0x00
	}
}
#endif

// A span routine of FUN_1002cd5d (a g_unk0x1002d2a0 entry); register-based, entered by jmp.
// It also holds the scan-line stepping the other span routines jump to.
#ifdef COMPAT_MODE
void FUN_1002d724(void)
{
	STUB(0x1002d724);
}
#else
// FUNCTION: MW2SHELL 0x1002d724
__declspec(naked) void FUN_1002d724(void)
{
	__asm {
		cmp dword ptr [esp], 0x5
		jl jmp_1002d7c8
jmp_1002d72e:
		mov bl, byte ptr [esi]
		mov byte ptr [edi], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov byte ptr [edi+0x1], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov byte ptr [edi+0x2], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov byte ptr [edi+0x3], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov byte ptr [edi+0x4], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		mov bl, byte ptr [esi]
		mov byte ptr [edi+0x5], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		add edi, 0x6
		sub dword ptr [esp], 0x6
		js jmp_1002d849
		cmp dword ptr [esp], 0x5
		jge jmp_1002d72e
jmp_1002d7c8:
		mov bl, byte ptr [esi]
		mov byte ptr [edi], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d849 */
		_emit 0x67
		mov bl, byte ptr [esi]
		mov byte ptr [edi+0x1], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d849 */
		_emit 0x4c
		mov bl, byte ptr [esi]
		mov byte ptr [edi+0x2], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d849 */
		_emit 0x31
		mov bl, byte ptr [esi]
		mov byte ptr [edi+0x3], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
		dec dword ptr [esp]
		_emit 0x78 /* js jmp_1002d849 */
		_emit 0x16
		mov bl, byte ptr [esi]
		mov byte ptr [edi+0x4], bl
		xor ebx, ebx
		add edx, eax
		adc ebx, ebx
		add ecx, ebp
		adc ebx, ebx
		add esi, dword ptr [g_unk0x100666a0+ebx*0x4+0xc0]
jmp_1002d849:
		add esp, 0x4
		pop ebp
		mov edi, dword ptr [g_unk0x100666a0+0xc]
		add dword ptr [g_unk0x100666a0+0x18], edi
		pop edi
		pop esi
		pop edx
		pop ecx
		pop ebx
		pop eax
		dec dword ptr [g_unk0x100666a0+0x48]
		_emit 0x78 /* js jmp_1002d8aa */
		_emit 0x43
		_emit 0x74 /* je jmp_1002d8b0 */
		_emit 0x47
		dec dword ptr [g_unk0x100666a0+0x40]
		je jmp_1002d910
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x80]
		add esi, dword ptr [g_unk0x100666a0+0x88]
jmp_1002d887:
		dec dword ptr [g_unk0x100666a0+0x44]
		je jmp_1002d9c9
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x84]
		add edi, dword ptr [g_unk0x100666a0+0x8c]
		_emit 0xe9 /* jmp 0x1002d125, outside this routine */
		_emit 0x7b
		_emit 0xf8
		_emit 0xff
		_emit 0xff
jmp_1002d8aa:
		pop es
		pop edi
		pop esi
		pop ebx
		leave
		ret
jmp_1002d8b0:
		add eax, dword ptr [g_unk0x100666a0+0x70]
		add ecx, dword ptr [g_unk0x100666a0+0x80]
		add esi, dword ptr [g_unk0x100666a0+0x88]
		add ebx, dword ptr [g_unk0x100666a0+0x74]
		add edx, dword ptr [g_unk0x100666a0+0x84]
		add edi, dword ptr [g_unk0x100666a0+0x8c]
		_emit 0xe9 /* jmp 0x1002d125, outside this routine */
		_emit 0x4c
		_emit 0xf8
		_emit 0xff
		_emit 0xff
		add dword ptr [g_unk0x100666a0+0x90], ecx
		shl ecx, 0x10
		mov eax, dword ptr [g_unk0x100666a0+0x9c]
		imul ecx
		shrd eax, edx, 0x10
		add dword ptr [g_unk0x100666a0+0xa4], eax
		mov eax, dword ptr [g_unk0x100666a0+0xa0]
		imul ecx
		shrd eax, edx, 0x10
		add esi, eax
		_emit 0xe9 /* jmp 0x1002d225, outside this routine */
		_emit 0x20
		_emit 0xf9
		_emit 0xff
		_emit 0xff
		sub dword ptr [g_unk0x100666a0+0x94], eax
		_emit 0xe9 /* jmp 0x1002d236, outside this routine */
		_emit 0x26
		_emit 0xf9
		_emit 0xff
		_emit 0xff
jmp_1002d910:
		push ebx
		push edx
		mov ebx, dword ptr [g_unk0x100666a0+0x38]
		mov dword ptr [g_unk0x100666a0+0x30], ebx
		mov esi, ebx
		sub esi, 0x18
		cmp esi, dword ptr [g_unk0x100666a0+0x24]
		_emit 0x7d /* jge jmp_1002d934 */
		_emit 0x09
		mov esi, dword ptr [g_unk0x100666a0+0x28]
		sub esi, 0x18
jmp_1002d934:
		mov dword ptr [g_unk0x100666a0+0x38], esi
		mov ecx, dword ptr [esi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x40], ecx
		mov edx, dword ptr [esi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x70], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi+0xc]
		sub edx, dword ptr [ebx+0xc]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x80], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x40]
		mov edx, dword ptr [esi+0x10]
		sub edx, dword ptr [ebx+0x10]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x88], eax
		mov eax, dword ptr [ebx]
		shl eax, 0x10
		add eax, 0x8000
		mov ecx, dword ptr [ebx+0xc]
		add ecx, 0x8000
		mov esi, dword ptr [ebx+0x10]
		add esi, 0x8000
		pop edx
		pop ebx
		jmp jmp_1002d887
jmp_1002d9c9:
		push eax
		push ecx
		mov ebx, dword ptr [g_unk0x100666a0+0x3c]
		mov dword ptr [g_unk0x100666a0+0x34], ebx
		mov edi, ebx
		add edi, 0x18
		cmp edi, dword ptr [g_unk0x100666a0+0x28]
		_emit 0x7c /* jl jmp_1002d9ea */
		_emit 0x06
		mov edi, dword ptr [g_unk0x100666a0+0x24]
jmp_1002d9ea:
		mov dword ptr [g_unk0x100666a0+0x3c], edi
		mov ecx, dword ptr [edi+0x4]
		mov edx, dword ptr [ebx+0x4]
		sub ecx, edx
		cmp ecx, 0x1
		adc ecx, 0x0
		mov dword ptr [g_unk0x100666a0+0x44], ecx
		mov edx, dword ptr [edi]
		sub edx, dword ptr [ebx]
		shl edx, 0x10
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x74], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [edi+0xc]
		sub edx, dword ptr [ebx+0xc]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x84], eax
		mov ecx, dword ptr [g_unk0x100666a0+0x44]
		mov edx, dword ptr [edi+0x10]
		sub edx, dword ptr [ebx+0x10]
		xor eax, eax
		shrd eax, edx, 0x10
		shl ecx, 0x10
		sar edx, 0x10
		idiv ecx
		mov dword ptr [g_unk0x100666a0+0x8c], eax
		mov edx, dword ptr [ebx+0xc]
		add edx, 0x8000
		mov edi, dword ptr [ebx+0x10]
		add edi, 0x8000
		mov ebx, dword ptr [ebx]
		shl ebx, 0x10
		add ebx, 0x8000
		pop ecx
		pop eax
		_emit 0xe9 /* jmp 0x1002d125, outside this routine */
		_emit 0xa5
		_emit 0xf6
		_emit 0xff
		_emit 0xff
	}
}
#endif
