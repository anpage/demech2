/* Normalizes a rotation matrix. Not a unit of its own: 1.1 has these in clock.c's object, the
   Matrox edition at the end of transform.c's, so clock.c includes this file in 1.1's build and
   transform.c in the Matrox edition's. */
#include "clock.h"
#include "decomp.h"
#include "transform.h"
#include "types.h"

#include <math.h>

// Normalizes the rows and columns of the rotation (2.29 fixed point).
// FUNCTION: MW2 0x1007cbf1
// FUNCTION: MW2MATROX 0x10002479
void NormalizeRotation(Matrix* p_matrix)
{
#ifdef MW2_MATROX
	ScaleVectorToLength(&p_matrix->m_rows[0][0], &p_matrix->m_rows[0][1], &p_matrix->m_rows[0][2]);
	ScaleVectorToLength(&p_matrix->m_rows[1][0], &p_matrix->m_rows[1][1], &p_matrix->m_rows[1][2]);
	ScaleVectorToLength(&p_matrix->m_rows[2][0], &p_matrix->m_rows[2][1], &p_matrix->m_rows[2][2]);
	ScaleVectorToLength(&p_matrix->m_rows[0][0], &p_matrix->m_rows[1][0], &p_matrix->m_rows[2][0]);
	ScaleVectorToLength(&p_matrix->m_rows[0][1], &p_matrix->m_rows[1][1], &p_matrix->m_rows[2][1]);
	ScaleVectorToLength(&p_matrix->m_rows[0][2], &p_matrix->m_rows[1][2], &p_matrix->m_rows[2][2]);
#else
	ScaleVectorToLength(0x20000000, &p_matrix->m_rows[0][0], &p_matrix->m_rows[0][1], &p_matrix->m_rows[0][2]);
	ScaleVectorToLength(0x20000000, &p_matrix->m_rows[1][0], &p_matrix->m_rows[1][1], &p_matrix->m_rows[1][2]);
	ScaleVectorToLength(0x20000000, &p_matrix->m_rows[2][0], &p_matrix->m_rows[2][1], &p_matrix->m_rows[2][2]);
	ScaleVectorToLength(0x20000000, &p_matrix->m_rows[0][0], &p_matrix->m_rows[1][0], &p_matrix->m_rows[2][0]);
	ScaleVectorToLength(0x20000000, &p_matrix->m_rows[0][1], &p_matrix->m_rows[1][1], &p_matrix->m_rows[2][1]);
	ScaleVectorToLength(0x20000000, &p_matrix->m_rows[0][2], &p_matrix->m_rows[1][2], &p_matrix->m_rows[2][2]);
#endif
}

// Scales (*p_x, *p_y, *p_z) to length p_length (the Matrox edition always to 1).
// FUNCTION: MW2 0x1007ccc2
// FUNCTION: MW2MATROX 0x1000252c
void ScaleVectorToLength(SCALE_VECTOR_PARAMS)
{
	MechDouble scale;
	MechDouble x;
	MechDouble y;
	MechDouble z;

	x = *p_x;
	y = *p_y;
	z = *p_z;
#ifdef MW2_MATROX
	*p_x = (scale = 1.0 / sqrt(x * x + y * y + z * z)) * x;
	*p_y = y * scale;
	*p_z = z * scale;
#else
	*p_x = (MechS32) ((scale = p_length / sqrt(x * x + y * y + z * z)) * x);
	*p_y = (MechS32) (y * scale);
	*p_z = (MechS32) (z * scale);
#endif
}
