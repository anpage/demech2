/* Hand-written assembly: FixedDivU16 is a C function whose body is an __asm block, like
   FixedDiv16. */
#include "fixeddivu.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Divides two 16.16 fixed-point values like FixedDiv16, but with an unsigned divide.
// FUNCTION: MW2 0x10044700
MechS32 FixedDivU16(MechS32 p_a, MechS32 p_b)
{
	__asm {
		mov eax, p_a
		mov ebx, p_b
		cdq
		shld edx, eax, 16
		shl eax, 16
		div ebx
	}
}
