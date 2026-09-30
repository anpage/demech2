/* Hand-written assembly: FixedDiv29 is a C function whose body is an __asm block, like
   FixedDiv16. */
#include "fixeddiv29.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Divides p_a, shifted left by 29, by p_b.
// FUNCTION: MW2 0x10019ab0
MechS32 FixedDiv29(MechS32 p_a, MechS32 p_b)
{
	__asm {
		mov eax, p_a
		mov ebx, p_b
		cdq
		shld edx, eax, 29
		shl eax, 29
		idiv ebx
	}
}
