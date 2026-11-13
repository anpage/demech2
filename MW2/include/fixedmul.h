#ifndef FIXEDMUL_H
#define FIXEDMUL_H

#include "types.h"

// The functions and globals of fixedmul.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MW2_3DFX
	// The 3Dfx edition expands FixedMul16 inline in every caller (its /Ox implies /Ob1), so the
	// header defines it there, with fixedmul.c's assembly; the other targets call fixedmul.c's.
#pragma warning(disable : 4035) /* no return value: the result is left in eax */
	__inline MechS32 FixedMul16(MechS32 p_a, MechS32 p_b)
	{
		__asm {
			mov eax, p_a
			mov ebx, p_b
			imul ebx
			shrd eax, edx, 16
			adc eax, 0
		}
	}
#pragma warning(default : 4035)
#else
MechS32 FixedMul16(MechS32 p_a, MechS32 p_b);
#endif

#ifdef __cplusplus
}
#endif

#endif // FIXEDMUL_H
