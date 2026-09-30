/* Hand-written assembly: FUN_100349f0 is a C function whose body is an __asm block, like
   MulDiv64. */
#include "unk100349f0.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Interpolates linearly: the y at p_x on the line through (p_x0, p_y0) and (p_x1, p_y1), measured
// from whichever end has the larger x.
// FUNCTION: MW2 0x100349f0
MechS32 FUN_100349f0(MechS32 p_x0, MechS32 p_x1, MechS32 p_x, MechS32 p_y0, MechS32 p_y1)
{
	__asm {
		mov ebx, p_x0
		mov ecx, p_x1
		mov eax, p_x
		mov edi, p_y0
		mov edx, p_y1
		cmp ecx, ebx
		jg jmp_10034a1e
		sub ecx, ebx
		sub eax, ebx
		sub edx, edi
		imul edx
		idiv ecx
		add eax, edi
		jmp jmp_10034a2c
jmp_10034a1e:
		sub ebx, ecx
		sub eax, ecx
		mov ecx, edx
		sub edi, ecx
		imul edi
		idiv ebx
		add eax, ecx
jmp_10034a2c:
	}
}
