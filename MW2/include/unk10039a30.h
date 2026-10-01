#ifndef UNK10039A30_H
#define UNK10039A30_H

#include "decomp.h"
#include "transform.h"
#include "types.h"

struct Ray;
struct ScarletOrchid0x4c;

/* One level of detail of a shape's model (the list at ScarletOrchid0x4c::m_unk0x1c): the
   header is followed by the 0x2c-byte vertices and the 0x24-byte faces (FUN_1003a5f3 allocates
   it, FUN_10039a30 transforms it). */
typedef struct GraniteLattice0x18 GraniteLattice0x18;

// SIZE 0x18
struct GraniteLattice0x18 {
	MechS32 m_unk0x00;                // 0x00 — the list is sorted by it, ascending
	MechS16 m_unk0x04;                // 0x04 — vertices
	MechS16 m_unk0x06;                // 0x06 — faces
	MechU32 m_unk0x08;                // 0x08 — offset of the faces
	GraniteLattice0x18* m_unk0x0c;    // 0x0c — the next model in the list
	undefined4 m_unk0x10;             // 0x10
	MechU16 m_unk0x14;                // 0x14
	undefined m_unk0x16[0x18 - 0x16]; // 0x16
};

// The functions of unk10039a30.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10039a30(GraniteLattice0x18* p_model, Matrix* p_matrix);
	void FUN_10039b94(struct ScarletOrchid0x4c* p_shape, Matrix* p_matrix);
	void FUN_10039c36(struct ScarletOrchid0x4c* p_shape, Matrix* p_matrix);
	MechS32 FUN_10039c96(
		MechS32 p_normalX,
		MechS32 p_normalY,
		MechS32 p_normalZ,
		MechS32 p_unk0x0c,
		MechS32 p_dx,
		MechS32 p_dz
	);
	MechS32 FUN_10039ccc(struct ScarletOrchid0x4c* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_10039dda(
		MechS32 p_x0,
		MechS32 p_y0,
		MechS32 p_z0,
		MechS32 p_x1,
		MechS32 p_y1,
		MechS32 p_z1,
		MechS32 p_x2,
		MechS32 p_y2,
		MechS32 p_z2,
		MechS32* p_nx,
		MechS32* p_ny,
		MechS32* p_nz
	);
	MechS32 FUN_1003a096(struct ScarletOrchid0x4c* p_shape, struct Ray* p_ray);

#ifdef __cplusplus
}
#endif

#endif // UNK10039A30_H
