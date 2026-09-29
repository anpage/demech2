#include "transform.h"

#include "decomp.h"
#include "loadres.h"
#include "types.h"

// STUB: MW2 0x1000d650
void FUN_1000d650(Matrix* p_matrix, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	STUB(0x1000d650);
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

// STUB: MW2 0x1000da0c
void FUN_1000da0c(Matrix* p_unk0x00, Matrix* p_unk0x04, Matrix* p_unk0x08)
{
	STUB(0x1000da0c);
}

// STUB: MW2 0x1000dbba
void FUN_1000dbba(Matrix* p_unk0x00, Matrix* p_unk0x04, Matrix* p_unk0x08)
{
	STUB(0x1000dbba);
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

// STUB: MW2 0x1000e2ea
void FUN_1000e2ea(Matrix* p_matrix, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c)
{
	STUB(0x1000e2ea);
}
