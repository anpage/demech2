/* Hand-written assembly: FUN_100696c0 (sine), FUN_1006975b (arcsine) and FUN_100698de
   (arctangent) are C functions with __asm bodies over clock.c's tables. */
#include "unk100696c0.h"

#include "clock.h"
#include "decomp.h"
#include "types.h"

#pragma warning(disable : 4035) /* FUN_100698de leaves its result in eax */

// Returns the sine of p_angle (16.16 degrees) from the table, as 16.16: the angle is scaled
// to 1024 steps to the circle (0x5b05b05b is 2^37 / 360), and the table's quarter wave is
// interpolated, mirrored and negated by quadrant.
// FUNCTION: MW2 0x100696c0
MechS32 FUN_100696c0(MechS32 p_angle)
{
	MechS32 result;

	__asm {
		mov eax, p_angle
		mov edx, 0x5b05b05b
		imul edx
		shrd eax, edx, 29
		adc eax, 0
		mov p_angle, eax
		mov ebx, eax
		mov esi, eax
		mov edx, eax
		shr ebx, 14
		and ebx, 0x3fc
		test esi, 0x1000000
		jz rising
		not edx
		xor ebx, 0x3fc
rising:
		mov ecx, g_sinTable[ebx]
		mov eax, g_sinTable[ebx + 4]
		sub eax, ecx
		and edx, 0xffff
		mul edx
		shrd eax, edx, 16
		adc ecx, eax
		test esi, 0x2000000
		jz positive
		neg ecx
positive:
		mov result, ecx
	}

	return result;
}

// Returns the cosine of p_angle (16.16 degrees): the sine 90 degrees on.
// FUNCTION: MW2 0x1006973a
MechS32 FUN_1006973a(MechS32 p_angle)
{
	return FUN_100696c0(p_angle + 0x5a0000);
}

// Returns the arcsine of p_sine (2.29 fixed point, as the table holds it) in 16.16 degrees: a
// binary search of the table's quarter wave, interpolated between the two entries found.
// Stack-slot permutation: result and table.
// FUNCTION: MW2 0x1006975b
MechS32 FUN_1006975b(MechS32 p_sine)
{
	MechS32 negative;
	MechS32 result;
	MechS32* table;

	negative = 0;
	table = g_sinTable;

	if (p_sine == 0) {
		return 0;
	}

	if (p_sine < 0) {
		negative++;
		p_sine = -p_sine;
	}

	if (p_sine >= 0x20000000) {
		result = 0x5a0000;
	}
	else {
		__asm {
			mov ebx, table
			xor ecx, ecx
			mov eax, p_sine
			cmp eax, [ebx + 0x200]
			jb below_128
			add ebx, 0x200
			or ecx, 0x800000
below_128:
			cmp eax, [ebx + 0x100]
			jb below_64
			add ebx, 0x100
			or ecx, 0x400000
below_64:
			cmp eax, [ebx + 0x80]
			jb below_32
			add ebx, 0x80
			or ecx, 0x200000
below_32:
			cmp eax, [ebx + 0x40]
			jb below_16
			add ebx, 0x40
			or ecx, 0x100000
below_16:
			cmp eax, [ebx + 0x20]
			jb below_8
			add ebx, 0x20
			or ecx, 0x80000
below_8:
			cmp eax, [ebx + 0x10]
			jb below_4
			add ebx, 0x10
			or ecx, 0x40000
below_4:
			cmp eax, [ebx + 0x8]
			jb below_2
			add ebx, 0x8
			or ecx, 0x20000
below_2:
			cmp eax, [ebx + 0x4]
			jb below_1
			add ebx, 0x4
			or ecx, 0x10000
below_1:
			sub eax, [ebx]
			jz scale
			mov esi, [ebx + 0x4]
			sub esi, [ebx]
			jz fraction
			cmp eax, esi
			jb fraction
			add ecx, 0x10000
			jmp scale
fraction:
			cdq
			shld edx, eax, 16
			shl eax, 16
			idiv esi
			mov cx, ax
scale:
			mov eax, 0x5a000000
			imul ecx
			mov result, edx
		}
	}

	return negative ? -result : result;
}

// FUNCTION: MW2 0x100698b9
MechS32 FUN_100698b9(MechS32 p_unk0x00)
{
	return 0x5a0000 - FUN_1006975b(p_unk0x00);
}

// Returns the bearing of (p_x, p_z) in 16.16 degrees: the arctangent of the smaller over the
// larger component from the table, folded into its octant. The result is also left in dx:ax.
// FUNCTION: MW2 0x100698de
MechS32 FUN_100698de(MechS32 p_x, MechS32 p_z)
{
	__asm {
		xor cl, cl
		mov edx, p_x
		or edx, edx
		jns x_positive
		neg edx
		or cl, 1
x_positive:
		mov ebx, p_z
		or ebx, ebx
		jns z_positive
		neg ebx
		or cl, 2
z_positive:
		cmp edx, ebx
		jl ordered
		jz diagonal
		xchg edx, ebx
		or cl, 4
		jmp short ordered
diagonal:
		mov eax, 0x2d0000
		jmp short octant
ordered:
		or edx, edx
		jnz divide
		mov eax, 0
		jmp short octant
divide:
		xor eax, eax
		shrd eax, edx, 8
		shr edx, 8
		div ebx
		mov ebx, eax
		shr ebx, 14
		and ebx, 0x3fc
		mov edi, g_atanTable[ebx]
		mov esi, g_atanTable[ebx + 4]
		sub esi, edi
		and eax, 0xffff
		mul esi
		shr eax, 16
		adc eax, edi
octant:
		test cl, 4
		jz below_diagonal
		neg eax
		add eax, 0x5a0000
below_diagonal:
		test cl, 2
		jz z_forward
		neg eax
		add eax, 0xb40000
z_forward:
		test cl, 1
		jz x_forward
		neg eax
x_forward:
		shld edx, eax, 16
	}
}
