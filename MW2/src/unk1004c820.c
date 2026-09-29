/* Hand-written assembly: FUN_1004c820 is a C function whose body is an __asm block. */
#include "unk1004c820.h"

#include "types.h"

// Multiplies p_a by p_b (unsigned) into *p_low; *p_scaled gets the product shifted right until it
// fits in 16 bits, and *p_shift is increased by the shift.
// FUNCTION: MW2 0x1004c820
void FUN_1004c820(MechS32* p_low, MechS32* p_scaled, MechS16* p_shift, MechU32 p_a, MechU32 p_b)
{
	__asm {
		mov esi, p_low
		mov edi, p_scaled
		mov edx, p_shift
		mov ebx, p_a
		mov eax, p_b
		push edx
		mul ebx
		mov [esi], eax
		pop ebx
		xor ecx, ecx
		bsr ecx, eax
		sub cx, 15
		jle done
		add word ptr [ebx], cx
		shrd eax, edx, cl
done:
		mov [edi], eax
	}
}
