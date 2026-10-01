#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "decomp.h"
#include "types.h"

/* A 2.29 fixed-point transform: three rotation rows and a translation row
   (FUN_1000dd4d sets the identity, 0x20000000 on the diagonal). */
typedef struct Matrix Matrix;

// SIZE 0x30
struct Matrix {
	MechS32 m_rows[4][3]; // 0x00
};

// The functions and globals of transform.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 FUN_1000d9a8(MechS32 p_a, MechS32 p_b);
	MechS32 FUN_1000d9ce(MechS32 p_ax, MechS32 p_ay, MechS32 p_az, MechS32 p_bx, MechS32 p_by, MechS32 p_bz);
	void FUN_1000d650(Matrix* p_matrix, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_1000d708(Matrix* p_matrix, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_1000da0c(Matrix* p_unk0x00, Matrix* p_unk0x04, Matrix* p_unk0x08);
	void FUN_1000dbba(Matrix* p_unk0x00, Matrix* p_unk0x04, Matrix* p_unk0x08);
	void FUN_1000dc33(Matrix* p_src, Matrix* p_dst);
	void FUN_1000dcbd(Matrix* p_src, Matrix* p_dst);
	void FUN_1000dd4d(Matrix* p_matrix);
	void FUN_1000dddf(Matrix* p_src, Matrix* p_dst);
	void FUN_1000ddfc(Matrix* p_src, Matrix* p_dst);
	void FUN_1000de3b(
		Matrix* p_matrix,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		MechS32 p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_unk0x14,
		MechS32 p_unk0x18,
		MechU32 p_flags
	);
	void FUN_1000e2b9(
		Matrix* p_matrix,
		MechS32 p_unk0x04,
		MechS32 p_unk0x08,
		MechS32 p_unk0x0c,
		MechS32 p_unk0x10,
		MechS32 p_unk0x14,
		MechS32 p_unk0x18
	);
	void FUN_1000e2ea(Matrix* p_matrix, undefined4* p_unk0x04, undefined4* p_unk0x08, undefined4* p_unk0x0c);

#ifdef __cplusplus
}
#endif

#endif // TRANSFORM_H
