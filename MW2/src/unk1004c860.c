/* Hand-written assembly: FUN_1004c860 is a C function whose body is an __asm block. */
#include "unk1004c860.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Returns (p_a * p_b + (p_c << 16)) / p_d, with a 64-bit intermediate.
// FUNCTION: MW2 0x1004c860
MechS32 FUN_1004c860(MechS32 p_a, MechS32 p_b, MechS32 p_c, MechS32 p_d)
{
	__asm {
		mov eax, p_a
		mov edx, p_b
		mov ebx, p_c
		mov ecx, p_d
		imul edx
		mov edi, edx
		mov esi, eax
		mov eax, ebx
		cdq
		shld edx, eax, 16
		shl eax, 16
		add eax, esi
		adc edx, edi
		idiv ecx
	}
}
