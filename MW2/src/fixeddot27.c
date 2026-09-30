/* Hand-written assembly: FixedDot27 is a C function whose body is an __asm block. */
#include "fixeddot27.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Returns p_a * p_b + p_c * p_d + p_e * p_f, shifted right by 27 and rounded.
// FUNCTION: MW2 0x100426c0
MechS32 FixedDot27(MechS32 p_a, MechS32 p_b, MechS32 p_c, MechS32 p_d, MechS32 p_e, MechS32 p_f)
{
	__asm {
		mov eax, p_a
		mov edx, p_b
		mov ecx, p_c
		mov ebx, p_d
		mov esi, p_e
		mov edi, p_f
		push esi
		push edi
		imul edx
		mov esi, eax
		mov edi, edx
		mov eax, ecx
		imul ebx
		add esi, eax
		adc edi, edx
		pop eax
		pop ebx
		imul ebx
		add esi, eax
		adc edi, edx
		shrd esi, edi, 27
		adc esi, 0
		mov eax, esi
	}
}
