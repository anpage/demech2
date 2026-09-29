/* Hand-written assembly: FUN_10004ec0 is a C function whose body is an __asm block. */
#include "unk10004ec0.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Returns 1 if p_x^2 + p_y^2 + p_z^2 (64-bit) is at most p_radius^2.
// FUNCTION: MW2 0x10004ec0
MechS32 FUN_10004ec0(MechS32 p_x, MechS32 p_y, MechS32 p_z, MechS32 p_radius)
{
	__asm {
		mov eax, p_x
		mov edi, p_y
		mov ecx, p_z
		mov ebx, p_radius
		imul eax
		mov esi, eax
		mov eax, edi
		mov edi, edx
		imul eax
		add esi, eax
		adc edi, edx
		mov eax, ecx
		imul eax
		add esi, eax
		adc edi, edx
		mov eax, ebx
		imul eax
		sub esi, eax
		sbb edi, edx
		jnc not_below
		mov eax, 1
		jmp done
not_below:
		cmp esi, 0
		jne outside
		mov eax, 1
		jmp done
outside:
		mov eax, 0
done:
	}
}
