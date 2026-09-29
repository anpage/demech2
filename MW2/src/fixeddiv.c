/* Hand-written assembly: FixedDiv16 is a C function whose body is an __asm block, like
   FixedMul16. */
#include "fixeddiv.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Divides two 16.16 fixed-point values (no check for a zero divisor).
// FUNCTION: MW2 0x10002c90
MechS32 FixedDiv16(MechS32 p_a, MechS32 p_b)
{
	__asm {
		mov eax, p_a
		mov ebx, p_b
		cdq
		shld edx, eax, 16
		shl eax, 16
		idiv ebx
	}
}
