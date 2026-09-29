#ifndef UNK10036230_H
#define UNK10036230_H

#include "decomp.h"
#include "transform.h"
#include "types.h"

struct ScarletOrchid0x4c;

/* One level of detail of a shape's model (the list at ScarletOrchid0x4c::m_unk0x1c): the
   header is followed by the 0x2c-byte vertices and the 0x24-byte faces (FUN_1003a5f3 allocates
   it, FUN_10039a30 transforms it). */
typedef struct GraniteLattice0x18 GraniteLattice0x18;

// SIZE 0x18
struct GraniteLattice0x18 {
	undefined m_unk0x00[0x10 - 0x00]; // 0x00
	undefined4 m_unk0x10;             // 0x10
	undefined m_unk0x14[0x18 - 0x14]; // 0x14
};

// The functions and globals of unk10036230.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10039a30(GraniteLattice0x18* p_model, Matrix* p_matrix);
	void FUN_10039b94(struct ScarletOrchid0x4c* p_shape, Matrix* p_matrix);

#ifdef __cplusplus
}
#endif

#endif // UNK10036230_H
