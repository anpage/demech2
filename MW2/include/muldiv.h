#ifndef MULDIV_H
#define MULDIV_H

#include "types.h"

// The functions and globals of muldiv.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MW2_3DFX
	// The 3Dfx edition expands MulDiv64 inline in every caller (its /Ox implies /Ob1), so the
	// header defines it there, with muldiv.c's assembly; the other targets call muldiv.c's.
#pragma warning(disable : 4035) /* no return value: the result is left in eax */
	__inline MechS32 MulDiv64(MechS32 p_a, MechS32 p_b, MechS32 p_c)
	{
		__asm {
			mov eax, p_a
			mov edx, p_b
			mov ecx, p_c
			imul edx
			idiv ecx
		}
	}
#pragma warning(default : 4035)
#else
MechS32 MulDiv64(MechS32 p_a, MechS32 p_b, MechS32 p_c);
#endif

#ifdef __cplusplus
}
#endif

#endif // MULDIV_H
