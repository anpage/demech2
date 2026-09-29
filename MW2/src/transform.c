#include "transform.h"

#include "decomp.h"
#include "loadres.h"
#include "types.h"

// STUB: MW2 0x1000d650
void FUN_1000d650(Matrix* p_matrix, MechS32* p_x, MechS32* p_y, MechS32* p_z)
{
	STUB(0x1000d650);
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

// STUB: MW2 0x1000dcbd
void FUN_1000dcbd(Matrix* p_unk0x00, Matrix* p_unk0x04)
{
	STUB(0x1000dcbd);
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
