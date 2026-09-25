/* Hand-written assembly: FixedMul16 is a C function whose body is an __asm block (the /Od
   frame saves esi/edi, which the body never touches). */
#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// FUNCTION: MW2 0x10003580
MechS32 FixedMul16(MechS32 p_a, MechS32 p_b)
{
	__asm {
		mov eax, p_a
		mov ebx, p_b
		imul ebx
		shrd eax, edx, 16
		adc eax, 0
	}
}
