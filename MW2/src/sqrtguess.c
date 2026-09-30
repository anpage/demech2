/* Hand-written assembly: FixedSqrtGuess is a C function whose body is an __asm block. */
#include "sqrtguess.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Estimates the square root of a 16.16 fixed-point value from its highest set bit, for
// FixedSqrt16 to refine.
// FUNCTION: MW2 0x10016a90
MechU32 FixedSqrtGuess(MechU32 p_value)
{
	__asm {
		mov eax, p_value
		bsr ecx, eax
		sub ecx, 15
		jle small
		shr ecx, 1
		shr eax, cl
		jmp done
small:
		bsr cx, ax
		neg cx
		add cx, 16
		shr cx, 1
		shl eax, cl
done:
	}
}
