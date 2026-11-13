#ifndef FIXEDDIV_H
#define FIXEDDIV_H

#include "types.h"

// The functions and globals of fixeddiv.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MW2_3DFX
	// The 3Dfx edition expands FixedDiv16 inline in every caller (its /Ox implies /Ob1), so the
	// header defines it there, with fixeddiv.c's assembly; the other targets call fixeddiv.c's.
#pragma warning(disable : 4035) /* no return value: the result is left in eax */
	__inline MechS32 FixedDiv16(MechS32 p_a, MechS32 p_b)
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
#pragma warning(default : 4035)
#else
MechS32 FixedDiv16(MechS32 p_a, MechS32 p_b);
#endif

#ifdef __cplusplus
}
#endif

#endif // FIXEDDIV_H
