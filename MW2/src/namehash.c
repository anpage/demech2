/* Hand-written assembly: FUN_100074e0 is a C function whose body is an __asm block. */
#include "namehash.h"

#include "types.h"

#pragma warning(disable : 4035) /* no return value: the result is left in eax */

// Hashes a name, ignoring case: each character is added and the low word rotated left.
// FUNCTION: MW2 0x100074e0
MechU32 FUN_100074e0(const MechChar* p_name)
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
