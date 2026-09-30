/* Hand-written assembly: FUN_10013340 is a C function whose body is an __asm block. */
#include "unk10013340.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Returns p_a * p_b / p_c / 2^14, the product kept in 64 bits and shifted by 8 before the division
// and the quotient by 6 after it.
// FUNCTION: MW2 0x10013340
MechS32 FUN_10013340(MechS32 p_a, MechS32 p_b, MechS32 p_c)
{
	__asm {
		mov eax, p_a
		mov edx, p_b
		mov ecx, p_c
		imul edx
		shrd eax, edx, 8
		sar edx, 8
		idiv ecx
		sar eax, 6
	}
}
