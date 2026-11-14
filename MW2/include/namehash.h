#ifndef NAMEHASH_H
#define NAMEHASH_H

#include "types.h"

// The functions and globals of namehash.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef MW2_3DFX
	// The 3Dfx edition expands HashName inline in its caller (its /Ox implies /Ob1), so the
	// header defines it there, with namehash.c's assembly; the other targets call namehash.c's.
#pragma warning(disable : 4035) /* no return value: the result is left in eax */
	__inline MechU32 HashName(const MechChar* p_name)
	{
		__asm {
			mov edx, p_name
			xor eax, eax
			jmp check
next:
			xor ebx, ebx
			mov bl, [edx]
			or bl, 0x20
			add eax, ebx
			inc edx
			rol ax, 1
check:
			cmp byte ptr [edx], 0
			jne next
		}
	}
#pragma warning(default : 4035)
#else
MechU32 HashName(const MechChar* p_name);
#endif

#ifdef __cplusplus
}
#endif

#endif // NAMEHASH_H
