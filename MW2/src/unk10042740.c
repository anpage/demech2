/* Hand-written assembly: FUN_10042740 is a C function whose body is an __asm block. Its portable
   C (PORTABLE_C) is tested against the assembly by tests/asmequiv. */
#include "unk10042740.h"

#include "compat.h"
#include "portable.h"
#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Returns (p_a << p_shift) / p_b, divided by 4 and rounded, plus p_c.
// FUNCTION: MW2 0x10042740
MechS32 FUN_10042740(MechS32 p_a, MechS32 p_b, MechS32 p_shift, MechS32 p_c)
{
#ifdef PORTABLE_C
	/* shld/shl take the count modulo 32; the rounding and the sum wrap. */
	MechU32 quotient = (MechU32) ((MechS64) p_a * ((MechS64) 1 << (p_shift & 31)) / p_b);

	return PortableS32((MechU32) PortableSar32(PortableS32(quotient + 2), 2) + (MechU32) p_c);
#else
	__asm {
		mov eax, p_a
		mov esi, p_b
		mov ecx, p_shift
		mov ebx, p_c
		cdq
		shld edx, eax, cl
		shl eax, cl
		idiv esi
		add eax, 2
		sar eax, 2
		add eax, ebx
	}
#endif
}
