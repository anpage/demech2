/* Hand-written assembly: FUN_10019ad0 is a C function whose body is an __asm block. */
#include "unk10019ad0.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// p_a * p_b >> 29, rounded.
// FUNCTION: MW2 0x10019ad0
MechS32 FUN_10019ad0(MechS32 p_a, MechS32 p_b)
{
	__asm {
		mov eax, p_a
		mov edx, p_b
		imul edx
		shrd eax, edx, 29
		adc eax, 0
	}
}
