/* Hand-written assembly: IntegrateMidpoint is a C function whose body is an __asm block. */
#include "integrate.h"

#include "types.h"

// Advances p_velocity by p_acceleration * p_time and p_position by the midpoint velocity
// times p_time (16.16 fixed point, with a 64-bit product).
// FUNCTION: MW2 0x10004e90
void IntegrateMidpoint(MechS32* p_position, MechS32* p_velocity, MechS32 p_acceleration, MechS32 p_time)
{
	__asm {
		mov esi, p_position
		mov ebx, p_velocity
		mov eax, p_acceleration
		mov ecx, p_time
		imul ecx
		mov edi, [ebx]
		add [ebx], eax
		sar eax, 1
		add eax, edi
		imul ecx
		shrd eax, edx, 16
		add [esi], eax
	}
}
