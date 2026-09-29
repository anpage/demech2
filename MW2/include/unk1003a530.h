#ifndef UNK1003A530_H
#define UNK1003A530_H

#include "decomp.h"
#include "types.h"
#include "unk10036230.h"

struct AmberWillow0x7c;

/* A shape hung on a scene object (AmberWillow0x7c::m_unk0x6c): a list of models by level of
   detail and the one in use. FUN_1003a8c1 allocates it. */
typedef struct ScarletOrchid0x4c ScarletOrchid0x4c;

// SIZE 0x4c
struct ScarletOrchid0x4c {
	MechU16 m_unk0x00;                   // 0x00
	MechU16 m_unk0x02;                   // 0x02
	undefined m_unk0x04[0x08 - 0x04];    // 0x04
	struct ScarletOrchid0x4c* m_unk0x08; // 0x08 — the next shape in its list
	undefined m_unk0x0c[0x14 - 0x0c];    // 0x0c
	MechU16 m_unk0x14;                   // 0x14
	MechU16 m_unk0x16;                   // 0x16
	struct AmberWillow0x7c* m_unk0x18;   // 0x18
	GraniteLattice0x18* m_unk0x1c;       // 0x1c
	GraniteLattice0x18* m_unk0x20;       // 0x20
	MechS32 m_unk0x24;                   // 0x24
	undefined m_unk0x28[0x48 - 0x28];    // 0x28
	undefined4 m_unk0x48;                // 0x48
};

// The functions and globals of unk1003a530.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1003acbe(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x04);
	MechU32 FUN_1003acf7(ScarletOrchid0x4c* p_shape);
	void FUN_1003ad2d(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x02);
	void FUN_1003ad4c(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x16);
	void FUN_1003ad62(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x14);
	MechU32 FUN_1003ad78(ScarletOrchid0x4c* p_shape);
	MechU32 FUN_1003ad93(ScarletOrchid0x4c* p_shape);
	MechU32 FUN_1003adae(ScarletOrchid0x4c* p_shape);
	struct AmberWillow0x7c* FUN_1003b6e5(ScarletOrchid0x4c* p_shape);
	void FUN_1003b6fb(ScarletOrchid0x4c* p_shape, struct AmberWillow0x7c* p_unk0x18);

#ifdef __cplusplus
}
#endif

#endif // UNK1003A530_H
