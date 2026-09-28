/* Hand-written assembly (originally a MASM object), transcribed like blit.c. The object
   starts at 0x10017a7c, right after debugout.c, with a 0x2000-byte table and a large block of
   code that patches itself at run time (0x10019a7c onward, through 0x100286a5); only the
   routines after it that touch neither are transcribed here. MASM pads between some routines
   with cs: mov eax, eax, which the inline assembler can't prefix, so the padding is _emit bytes
   at the end of the routine before it.

   Not yet transcribed: the table and the code block (declared as data below, so the routines
   can reference them) and FUN_100287e0 (calls through the table).

   The untranscribed code is believed to be dead, so its absence should not affect the
   recompiled DLL: the original's relocation table holds no absolute reference into
   0x10017a7c-0x1002900f from outside it (no function pointer or `offset` operand names any
   of it), Ghidra finds no call or jump into the block or to the routines here from any
   other function, and .text is read-only with no VirtualProtect import, so the block could
   not patch itself in a Win32 process anyway. It reads as a DOS-era renderer left in the
   link. */
#include "compat.h"
#include "decomp.h"
#include "pixelview.h"
#include "types.h"

#pragma warning(disable : 4102) /* the labels mark the targets of the _emit short jumps */
#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// The table of routines FUN_100286eb dispatches to: 0x800 entries, mostly null, pointing into
// the code block. It lives in .text in the original.
// GLOBAL: MW2SHELL 0x10017a7c
undefined4 g_unk0x10017a7c[0x800];

// The start of the self-modifying code block (through 0x100286a5), declared as data.
// GLOBAL: MW2SHELL 0x10019a7c
undefined g_unk0x10019a7c[1];

// The pair FUN_100286a6 sets.
// GLOBAL: MW2SHELL 0x10064cd8
undefined4 g_unk0x10064cd8 = 0x8000;

// GLOBAL: MW2SHELL 0x10064cdc
undefined4 g_unk0x10064cdc = 0;

// The code block's working variables. FUN_100286eb sets the view (+0x8, +0x10), the vertex
// list's bounds (+0x14, +0x18) and its arguments (+0x98, +0x100 to +0x108).
// GLOBAL: MW2SHELL 0x10064ce0
undefined4 g_unk0x10064ce0[0x43] = {0};

#ifdef COMPAT_MODE
void FUN_100286a6(undefined4 p_unk0x00, undefined4 p_unk0x04)
{
	STUB(0x100286a6);
}
#else
// FUNCTION: MW2SHELL 0x100286a6
__declspec(naked) void FUN_100286a6(undefined4 p_unk0x00, undefined4 p_unk0x04)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		push es
		mov eax, dword ptr [ebp+0x8]
		mov dword ptr [g_unk0x10064cd8], eax
		mov eax, dword ptr [ebp+0xc]
		mov dword ptr [g_unk0x10064cdc], eax
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
// size (it ends where FUN_100286a6 starts).
#ifdef COMPAT_MODE
MechS32 FUN_100286c3(undefined4* p_start, undefined4* p_segment)
{
	STUB(0x100286c3);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100286c3
__declspec(naked) MechS32 FUN_100286c3(undefined4* p_start, undefined4* p_segment)
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
		mov ebx, offset g_unk0x10019a7c
		mov dword ptr [eax], ebx
		mov eax, offset FUN_100286a6
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
void FUN_100286eb(
	PixelView* p_view,
	MechS32* p_vertices,
	MechS32 p_count,
	MechS32 p_index,
	undefined4 p_unk0x18,
	undefined4 p_unk0x1c,
	undefined4 p_unk0x20,
	undefined4 p_unk0x24
)
{
	STUB(0x100286eb);
}
#else
// FUNCTION: MW2SHELL 0x100286eb
__declspec(naked) void FUN_100286eb(
	PixelView* p_view,
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
		mov dword ptr [g_unk0x10064ce0+0x8], eax
		mov eax, dword ptr [edi]
		mov dword ptr [g_unk0x10064ce0+0x10], eax
		mov ebx, dword ptr [ebp+0xc]
		mov eax, dword ptr [ebp+0x10]
		shl eax, 0x3
		mov edx, eax
		shl eax, 0x1
		add eax, edx
		add eax, ebx
		mov dword ptr [g_unk0x10064ce0+0x14], ebx
		mov dword ptr [g_unk0x10064ce0+0x18], eax
		mov eax, dword ptr [ebp+0x1c]
		mov dword ptr [g_unk0x10064ce0+0x100], eax
		mov eax, dword ptr [ebp+0x20]
		mov dword ptr [g_unk0x10064ce0+0x104], eax
		mov eax, dword ptr [ebp+0x24]
		mov dword ptr [g_unk0x10064ce0+0x108], eax
		mov eax, dword ptr [ebp+0x18]
		mov dword ptr [g_unk0x10064ce0+0x98], eax
		mov ebx, dword ptr [ebp+0x14]
		mov eax, dword ptr [g_unk0x10017a7c+ebx*0x4]
		cmp eax, 0x0
		_emit 0x74 /* je jmp_10028756 */
		_emit 0x02
		call eax
jmp_10028756:
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
MechS32 FUN_1002875c(MechS32 p_a, MechS32 p_b)
{
	STUB(0x1002875c);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x1002875c
__declspec(naked) MechS32 FUN_1002875c(MechS32 p_a, MechS32 p_b)
{
	__asm {
		push ebp
		mov ebp, esp
		push ebx
		sub ecx, ecx
		mov eax, dword ptr [ebp+0x8]
		and eax, eax
		_emit 0x79 /* jns jmp_1002876c */
		_emit 0x03
		inc ecx
		neg eax
jmp_1002876c:
		mov edx, eax
		sar edx, 0x2
		shl eax, 0x1e
		mov ebx, dword ptr [ebp+0xc]
		and ebx, ebx
		_emit 0x79 /* jns jmp_1002877e */
		_emit 0x03
		dec ecx
		neg ebx
jmp_1002877e:
		div ebx
		shr ebx, 0x1
		adc ebx, 0x0
		dec ebx
		cmp ebx, edx
		adc eax, 0x0
		and ecx, ecx
		_emit 0x74 /* je jmp_10028791 */
		_emit 0x02
		neg eax
jmp_10028791:
		pop ebx
		leave
		ret
	}
}
#endif

// The rounded reciprocal of p_value: 2^46 / p_value.
#ifdef COMPAT_MODE
MechS32 FUN_10028794(MechS32 p_value)
{
	STUB(0x10028794);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x10028794
__declspec(naked) MechS32 FUN_10028794(MechS32 p_value)
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
		_emit 0x79 /* jns jmp_100287ab */
		_emit 0x03
		dec ecx
		neg ebx
jmp_100287ab:
		div ebx
		shr ebx, 0x1
		adc ebx, 0x0
		dec ebx
		cmp ebx, edx
		adc eax, 0x0
		and ecx, ecx
		_emit 0x74 /* je jmp_100287be */
		_emit 0x02
		neg eax
jmp_100287be:
		pop ebx
		leave
		ret
		_emit 0x2e /* mov eax, eax with a segment prefix: MASM alignment padding */
		_emit 0x8b
		_emit 0xc0
	}
}
#endif

// Multiplies two 2.30 fixed-point values, rounded.
#ifdef COMPAT_MODE
MechS32 FUN_100287c4(MechS32 p_a, MechS32 p_b)
{
	STUB(0x100287c4);
	return 0;
}
#else
// FUNCTION: MW2SHELL 0x100287c4
__declspec(naked) MechS32 FUN_100287c4(MechS32 p_a, MechS32 p_b)
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
