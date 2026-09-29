/* Hand-written assembly: FUN_1004c800 is a C function whose body is an __asm block. */
#include "unk1004c800.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// p_a * p_b / (p_b + p_c) >> 12.
// FUNCTION: MW2 0x1004c800
MechS32 FUN_1004c800(MechS32 p_a, MechS32 p_b, MechS32 p_c)
{
	__asm {
		mov eax, p_a
		mov ebx, p_b
		mov ecx, p_c
		imul ebx
		add ebx, ecx
		idiv ebx
		shr eax, 12
	}
}
