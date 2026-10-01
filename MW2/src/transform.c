/* Hand-written assembly: FUN_1000da0c is a C function with an __asm body, and FUN_1000d7c0 has an
   __asm block. */
#include "transform.h"

#include "clock.h"
#include "decomp.h"
#include "loadres.h"
#include "types.h"
#include "unk100696c0.h"

// Transforms the point (*p_x, *p_y, *p_z) by p_matrix: its 2.29 rotation, then its translation.
// The products are an __asm block.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1000d650
void FUN_1000d650(Matrix* p_matrix, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	MechS32 rz;
	MechS32 x;
	MechS32 rx;
	MechS32 y;
	MechS32 z;
	MechS32 ry;

	x = *p_x;
	y = *p_y;
	z = *p_z;
	__asm {
		mov edi, p_matrix
		mov eax, dword ptr [edi]
		imul x
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [edi + 0x4]
		imul y
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [edi + 0x8]
		imul z
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 29
		adc eax, dword ptr [edi + 0x24]
		mov rx, eax
		mov eax, dword ptr [edi + 0xc]
		imul x
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [edi + 0x10]
		imul y
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [edi + 0x14]
		imul z
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 29
		adc eax, dword ptr [edi + 0x28]
		mov ry, eax
		mov eax, dword ptr [edi + 0x18]
		imul x
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [edi + 0x1c]
		imul y
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [edi + 0x20]
		imul z
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 29
		adc eax, dword ptr [edi + 0x2c]
		mov rz, eax
	}

	*p_x = rx;
	*p_y = ry;
	*p_z = rz;
}

// Rotates the point (*p_x, *p_y, *p_z) by p_matrix's 2.29 rotation. The products are an __asm
// block.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1000d708
void FUN_1000d708(Matrix* p_matrix, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	MechS32 rz;
	MechS32 x;
	MechS32 rx;
	MechS32 y;
	MechS32 z;
	MechS32 ry;

	x = *p_x;
	y = *p_y;
	z = *p_z;
	__asm {
		mov edi, p_matrix
		mov eax, dword ptr [edi]
		imul x
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [edi + 0x4]
		imul y
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [edi + 0x8]
		imul z
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 29
		adc eax, 0
		mov rx, eax
		mov eax, dword ptr [edi + 0xc]
		imul x
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [edi + 0x10]
		imul y
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [edi + 0x14]
		imul z
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 29
		adc eax, 0
		mov ry, eax
		mov eax, dword ptr [edi + 0x18]
		imul x
		mov ecx, edx
		mov ebx, eax
		mov eax, dword ptr [edi + 0x1c]
		imul y
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [edi + 0x20]
		imul z
		add eax, ebx
		adc edx, ecx
		shrd eax, edx, 29
		adc eax, 0
		mov rz, eax
	}

	*p_x = rx;
	*p_y = ry;
	*p_z = rz;
}

// Recomputes column p_column of p_matrix's 2.29 rotation as the cross product of the other two,
// which makes it orthogonal to them again. The products are an __asm block.
// Stack-slot permutation: the column values and x, y and z.
// FUNCTION: MW2 0x1000d7c0
void FUN_1000d7c0(Matrix* p_matrix, MechS32 p_column)
{
	MechS32 a2;
	MechS32 b2;
	MechS32 a0;
	MechS32 x;
	MechS32 b0;
	MechS32 y;
	MechS32 z;
	MechS32 a1;
	MechS32 b1;

	switch (p_column) {
	case 0:
		a0 = p_matrix->m_rows[0][1];
		a1 = p_matrix->m_rows[1][1];
		a2 = p_matrix->m_rows[2][1];
		b0 = p_matrix->m_rows[0][2];
		b1 = p_matrix->m_rows[1][2];
		b2 = p_matrix->m_rows[2][2];
		break;
	case 1:
		a0 = p_matrix->m_rows[0][2];
		a1 = p_matrix->m_rows[1][2];
		a2 = p_matrix->m_rows[2][2];
		b0 = p_matrix->m_rows[0][0];
		b1 = p_matrix->m_rows[1][0];
		b2 = p_matrix->m_rows[2][0];
		break;
	case 2:
		a0 = p_matrix->m_rows[0][0];
		a1 = p_matrix->m_rows[1][0];
		a2 = p_matrix->m_rows[2][0];
		b0 = p_matrix->m_rows[0][1];
		b1 = p_matrix->m_rows[1][1];
		b2 = p_matrix->m_rows[2][1];
		break;
	}

	__asm {
		mov eax, a1
		mov edx, b2
		imul edx
		mov edi, edx
		mov esi, eax
		mov eax, b1
		mov edx, a2
		imul edx
		sub esi, eax
		sbb edi, edx
		shrd esi, edi, 29
		adc esi, 0
		mov x, esi
		mov eax, a2
		mov edx, b0
		imul edx
		mov edi, edx
		mov esi, eax
		mov eax, b2
		mov edx, a0
		imul edx
		sub esi, eax
		sbb edi, edx
		shrd esi, edi, 29
		adc esi, 0
		mov y, esi
		mov eax, a0
		mov edx, b1
		imul edx
		mov edi, edx
		mov esi, eax
		mov eax, b0
		mov edx, a1
		imul edx
		sub esi, eax
		sbb edi, edx
		shrd esi, edi, 29
		adc esi, 0
		mov z, esi
	}

	switch (p_column)
	{
	case 0:
		p_matrix->m_rows[0][0] = x;
		p_matrix->m_rows[1][0] = y;
		p_matrix->m_rows[2][0] = z;
		break;
	case 1:
		p_matrix->m_rows[0][1] = x;
		p_matrix->m_rows[1][1] = y;
		p_matrix->m_rows[2][1] = z;
		break;
	case 2:
		p_matrix->m_rows[0][2] = x;
		p_matrix->m_rows[1][2] = y;
		p_matrix->m_rows[2][2] = z;
		break;
	}
}

// Multiplies two 2.29 fixed-point values. The body is an __asm block.
// FUNCTION: MW2 0x1000d9a8
MechS32 FUN_1000d9a8(MechS32 p_a, MechS32 p_b)
{
	MechS32 result;

	__asm {
		mov eax, p_a
		imul p_b
		shrd eax, edx, 29
		adc eax, 0
		mov result, eax
	}

	return result;
}

// The dot product of two vectors of 2.29 fixed-point values, with a 64-bit sum. The body is an
// __asm block.
// FUNCTION: MW2 0x1000d9ce
MechS32 FUN_1000d9ce(MechS32 p_ax, MechS32 p_ay, MechS32 p_az, MechS32 p_bx, MechS32 p_by, MechS32 p_bz)
{
	MechS32 result;

	__asm {
		mov eax, p_ax
		imul p_bx
		mov esi, eax
		mov edi, edx
		mov eax, p_ay
		imul p_by
		add esi, eax
		adc edi, edx
		mov eax, p_az
		imul p_bz
		add esi, eax
		adc edi, edx
		shrd esi, edi, 29
		adc esi, 0
		mov result, esi
	}

	return result;
}

// Multiplies the rotations of p_unk0x00 and p_unk0x04 (2.29 fixed point) into p_unk0x08. The
// products are an __asm block.
// Stack-slot permutation of the locals the __asm block names.
// FUNCTION: MW2 0x1000da0c
void FUN_1000da0c(Matrix* p_unk0x00, Matrix* p_unk0x04, Matrix* p_unk0x08)
{
	MechS32 m00;
	MechS32 m01;
	MechS32 m02;
	MechS32 m10;
	MechS32 m11;
	MechS32 m12;
	MechS32 m20;
	MechS32 m21;
	MechS32 m22;

	__asm {
		mov esi, p_unk0x00
		mov edi, p_unk0x04
		mov eax, dword ptr [esi]
		imul dword ptr [edi]
		mov ebx, eax
		mov ecx, edx
		mov eax, dword ptr [esi + 4]
		imul dword ptr [edi + 0xc]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 8]
		imul dword ptr [edi + 0x18]
		add ebx, eax
		adc ecx, edx
		shrd ebx, ecx, 0x1d
		adc ebx, 0
		mov m00, ebx
		mov eax, dword ptr [esi]
		imul dword ptr [edi + 4]
		mov ebx, eax
		mov ecx, edx
		mov eax, dword ptr [esi + 4]
		imul dword ptr [edi + 0x10]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 8]
		imul dword ptr [edi + 0x1c]
		add ebx, eax
		adc ecx, edx
		shrd ebx, ecx, 0x1d
		adc ebx, 0
		mov m01, ebx
		mov eax, dword ptr [esi]
		imul dword ptr [edi + 8]
		mov ebx, eax
		mov ecx, edx
		mov eax, dword ptr [esi + 4]
		imul dword ptr [edi + 0x14]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 8]
		imul dword ptr [edi + 0x20]
		add ebx, eax
		adc ecx, edx
		shrd ebx, ecx, 0x1d
		adc ebx, 0
		mov m02, ebx
		mov eax, dword ptr [esi + 0xc]
		imul dword ptr [edi]
		mov ebx, eax
		mov ecx, edx
		mov eax, dword ptr [esi + 0x10]
		imul dword ptr [edi + 0xc]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x14]
		imul dword ptr [edi + 0x18]
		add ebx, eax
		adc ecx, edx
		shrd ebx, ecx, 0x1d
		adc ebx, 0
		mov m10, ebx
		mov eax, dword ptr [esi + 0xc]
		imul dword ptr [edi + 4]
		mov ebx, eax
		mov ecx, edx
		mov eax, dword ptr [esi + 0x10]
		imul dword ptr [edi + 0x10]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x14]
		imul dword ptr [edi + 0x1c]
		add ebx, eax
		adc ecx, edx
		shrd ebx, ecx, 0x1d
		adc ebx, 0
		mov m11, ebx
		mov eax, dword ptr [esi + 0xc]
		imul dword ptr [edi + 8]
		mov ebx, eax
		mov ecx, edx
		mov eax, dword ptr [esi + 0x10]
		imul dword ptr [edi + 0x14]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x14]
		imul dword ptr [edi + 0x20]
		add ebx, eax
		adc ecx, edx
		shrd ebx, ecx, 0x1d
		adc ebx, 0
		mov m12, ebx
		mov eax, dword ptr [esi + 0x18]
		imul dword ptr [edi]
		mov ebx, eax
		mov ecx, edx
		mov eax, dword ptr [esi + 0x1c]
		imul dword ptr [edi + 0xc]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x20]
		imul dword ptr [edi + 0x18]
		add ebx, eax
		adc ecx, edx
		shrd ebx, ecx, 0x1d
		adc ebx, 0
		mov m20, ebx
		mov eax, dword ptr [esi + 0x18]
		imul dword ptr [edi + 4]
		mov ebx, eax
		mov ecx, edx
		mov eax, dword ptr [esi + 0x1c]
		imul dword ptr [edi + 0x10]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x20]
		imul dword ptr [edi + 0x1c]
		add ebx, eax
		adc ecx, edx
		shrd ebx, ecx, 0x1d
		adc ebx, 0
		mov m21, ebx
		mov eax, dword ptr [esi + 0x18]
		imul dword ptr [edi + 8]
		mov ebx, eax
		mov ecx, edx
		mov eax, dword ptr [esi + 0x1c]
		imul dword ptr [edi + 0x14]
		add ebx, eax
		adc ecx, edx
		mov eax, dword ptr [esi + 0x20]
		imul dword ptr [edi + 0x20]
		add ebx, eax
		adc ecx, edx
		shrd ebx, ecx, 0x1d
		adc ebx, 0
		mov m22, ebx
		mov edi, p_unk0x08
		mov eax, m00
		mov dword ptr [edi], eax
		mov eax, m01
		mov dword ptr [edi + 4], eax
		mov eax, m10
		mov dword ptr [edi + 0xc], eax
		mov eax, m11
		mov dword ptr [edi + 0x10], eax
		mov eax, m20
		mov dword ptr [edi + 0x18], eax
		mov eax, m21
		mov dword ptr [edi + 0x1c], eax
		mov eax, m02
		mov dword ptr [edi + 8], eax
		mov eax, m12
		mov dword ptr [edi + 0x14], eax
		mov eax, m22
		mov dword ptr [edi + 0x20], eax
	}
}

// Composes p_unk0x04 with p_unk0x00 into p_unk0x08: the rotations' product, and p_unk0x04's
// translation transformed by p_unk0x00.
// FUNCTION: MW2 0x1000dbba
void FUN_1000dbba(Matrix* p_unk0x00, Matrix* p_unk0x04, Matrix* p_unk0x08)
{
	MechS32 z;
	MechS32 y;
	MechS32 x;
	Matrix result;

	FUN_1000da0c(p_unk0x00, p_unk0x04, &result);
	x = p_unk0x04->m_rows[3][0];
	y = p_unk0x04->m_rows[3][1];
	z = p_unk0x04->m_rows[3][2];
	FUN_1000d650(p_unk0x00, &x, &y, &z);
	result.m_rows[3][0] = x;
	result.m_rows[3][1] = y;
	result.m_rows[3][2] = z;
	MemCopy(p_unk0x08, &result, sizeof(Matrix));
}

// Transposes the rotation of p_src into p_dst.
// FUNCTION: MW2 0x1000dc33
void FUN_1000dc33(Matrix* p_src, Matrix* p_dst)
{
	MechS32 temp;

	p_dst->m_rows[0][0] = p_src->m_rows[0][0];
	p_dst->m_rows[1][1] = p_src->m_rows[1][1];
	p_dst->m_rows[2][2] = p_src->m_rows[2][2];
	temp = p_src->m_rows[0][1];
	p_dst->m_rows[0][1] = p_src->m_rows[1][0];
	p_dst->m_rows[1][0] = temp;
	temp = p_src->m_rows[2][0];
	p_dst->m_rows[2][0] = p_src->m_rows[0][2];
	p_dst->m_rows[0][2] = temp;
	temp = p_src->m_rows[2][1];
	p_dst->m_rows[2][1] = p_src->m_rows[1][2];
	p_dst->m_rows[1][2] = temp;
}

// Inverts a rigid transform: the transposed rotation, and the negated translation rotated by it.
// FUNCTION: MW2 0x1000dcbd
void FUN_1000dcbd(Matrix* p_src, Matrix* p_dst)
{
	MechS32 z;
	MechS32 y;
	MechS32 x;

	x = -p_src->m_rows[3][0];
	y = -p_src->m_rows[3][1];
	z = -p_src->m_rows[3][2];
	FUN_1000dc33(p_src, p_dst);
	p_dst->m_rows[3][0] = 0;
	p_dst->m_rows[3][1] = 0;
	p_dst->m_rows[3][2] = 0;
	FUN_1000d650(p_dst, &x, &y, &z);
	p_dst->m_rows[3][0] = x;
	p_dst->m_rows[3][1] = y;
	p_dst->m_rows[3][2] = z;
}

// FUNCTION: MW2 0x1000dd4d
void FUN_1000dd4d(Matrix* p_matrix)
{
	p_matrix->m_rows[0][0] = p_matrix->m_rows[1][1] = p_matrix->m_rows[2][2] = 0x20000000;
	p_matrix->m_rows[1][0] = p_matrix->m_rows[2][0] = p_matrix->m_rows[3][0] = 0;
	p_matrix->m_rows[0][1] = p_matrix->m_rows[2][1] = p_matrix->m_rows[3][1] = 0;
	p_matrix->m_rows[0][2] = p_matrix->m_rows[1][2] = p_matrix->m_rows[3][2] = 0;
}

// FUNCTION: MW2 0x1000dddf
void FUN_1000dddf(Matrix* p_src, Matrix* p_dst)
{
	MemCopy(p_dst, p_src, sizeof(Matrix));
}

// FUNCTION: MW2 0x1000ddfc
void FUN_1000ddfc(Matrix* p_src, Matrix* p_dst)
{
	MemCopy(p_dst, p_src, sizeof(Matrix));
	p_dst->m_rows[3][0] = p_dst->m_rows[3][1] = p_dst->m_rows[3][2] = 0;
}

// STUB: MW2 0x1000de3b
void FUN_1000de3b(
	Matrix* p_matrix,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18,
	MechU32 p_unk0x1c
)
{
	STUB(0x1000de3b);
}

// FUNCTION: MW2 0x1000e2b9
void FUN_1000e2b9(
	Matrix* p_matrix,
	MechS32 p_unk0x04,
	MechS32 p_unk0x08,
	MechS32 p_unk0x0c,
	MechS32 p_unk0x10,
	MechS32 p_unk0x14,
	MechS32 p_unk0x18
)
{
	FUN_1000de3b(p_matrix, p_unk0x04, p_unk0x08, p_unk0x0c, p_unk0x10, p_unk0x14, p_unk0x18, 0);
}

// Reads the three rotation angles (16.16 degrees) back out of p_matrix. Near straight up or down
// (the pitch sine within 0.001 of 1), the angle comes from the other rows; with nothing to go on,
// the yaw is taken as 90 degrees and the roll from the first row.
// Stack-slot permutation: pitch, roll, length and yaw.
// FUNCTION: MW2 0x1000e2ea
void FUN_1000e2ea(Matrix* p_matrix, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c)
{
	MechS32 pitch;
	MechS32 roll;
	MechS32 length;
	MechS32 yaw;

	if (p_matrix->m_rows[1][2] > 0x1ff7ced9 || p_matrix->m_rows[1][2] < -0x1ff7ced9) {
		length = FUN_1007caf7(p_matrix->m_rows[0][2], p_matrix->m_rows[2][2]);
		if (length > 2000000) {
			pitch = FUN_100698b9(length);
			if (p_matrix->m_rows[1][2] < 0) {
				pitch = -pitch;
			}
		}
		else {
			if (p_matrix->m_rows[1][2] >= 0) {
				yaw = -0x5a0000;
			}
			else {
				yaw = 0x5a0000;
			}

			pitch = 0;
			roll = FUN_100698de(p_matrix->m_rows[0][1], p_matrix->m_rows[0][0]);
			goto done;
		}
	}
	else {
		pitch = -FUN_1006975b(p_matrix->m_rows[1][2]);
	}

	yaw = FUN_100698de(p_matrix->m_rows[0][2], p_matrix->m_rows[2][2]);
	roll = FUN_100698de(p_matrix->m_rows[1][0], p_matrix->m_rows[1][1]);

done:
	*p_unk0x08 = yaw;
	*p_unk0x04 = pitch;
	*p_unk0x0c = roll;
}
