/* Hand-written assembly: FUN_100349c0 is a C function whose body is an __asm block, like
   MulDiv64. */
#include "unk100349c0.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Divides the 64-bit p_a * p_d - p_b * p_c by p_divisor.
// FUNCTION: MW2 0x100349c0
MechS32 FUN_100349c0(MechS32 p_a, MechS32 p_b, MechS32 p_c, MechS32 p_d, MechS32 p_divisor)
{
	__asm {
		mov ebx, p_a
		mov eax, p_b
		mov edx, p_c
		mov ecx, p_d
		mov edi, p_divisor
		push edi
		imul edx
		mov edi, edx
		mov esi, eax
		mov eax, ebx
		imul ecx
		sub eax, esi
		sbb edx, edi
		pop edi
		idiv edi
	}
}
