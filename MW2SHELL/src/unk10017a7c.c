/* Hand-written assembly (originally a MASM object), transcribed like unk10032250.c. The object
   starts at 0x10017a7c, right after debugout.c, with a 0x2000-byte table and a large block of
   code that patches itself at run time (0x10019a7c onward, through 0x100286a5); only the
   routines after it that touch neither are transcribed here. MASM pads between some routines
   with cs: mov eax, eax, which the inline assembler can't prefix, so the padding is _emit bytes
   at the end of the routine before it.

   Not yet transcribed: the table, the code block, FUN_100286c3 (returns the code block's
   bounds), FUN_100286eb and FUN_100287e0 (both call through the table). */
#include "compat.h"
#include "decomp.h"
#include "types.h"

#pragma warning(disable : 4102) /* the labels mark the targets of the _emit short jumps */
#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// The pair FUN_100286a6 sets.
// GLOBAL: MW2SHELL 0x10064cd8
undefined4 g_unk0x10064cd8 = 0x8000;

// GLOBAL: MW2SHELL 0x10064cdc
undefined4 g_unk0x10064cdc = 0;

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
