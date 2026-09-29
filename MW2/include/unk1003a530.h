#ifndef UNK1003A530_H
#define UNK1003A530_H

#include "decomp.h"
#include "types.h"
#include "unk10036230.h"

struct AmberWillow0x7c;
struct DuskMoth0x24;
struct EmberFern0x2c;

/* A shape hung on a scene object (AmberWillow0x7c::m_unk0x6c): a list of models by level of
   detail and the one in use. FUN_1003a8c1 allocates it. */
typedef struct ScarletOrchid0x4c ScarletOrchid0x4c;

// SIZE 0x4c
struct ScarletOrchid0x4c {
	MechU16 m_unk0x00;                   // 0x00
	MechU16 m_unk0x02;                   // 0x02
	struct ScarletOrchid0x4c* m_unk0x04; // 0x04 — the previous shape in its list
	struct ScarletOrchid0x4c* m_unk0x08; // 0x08 — the next shape in its list
	struct ScarletOrchid0x4c* m_unk0x0c; // 0x0c
	struct ScarletOrchid0x4c* m_unk0x10; // 0x10
	MechU16 m_unk0x14;                   // 0x14
	MechU16 m_unk0x16;                   // 0x16
	struct AmberWillow0x7c* m_unk0x18;   // 0x18
	GraniteLattice0x18* m_unk0x1c;       // 0x1c
	GraniteLattice0x18* m_unk0x20;       // 0x20
	MechS32 m_unk0x24;                   // 0x24
	undefined4 m_unk0x28;                // 0x28
	undefined4 m_unk0x2c;                // 0x2c
	undefined4 m_unk0x30;                // 0x30
	MechS32 m_unk0x34;                   // 0x34
	MechS32 m_unk0x38;                   // 0x38
	MechS32 m_unk0x3c;                   // 0x3c
	MechS32 m_unk0x40;                   // 0x40 — a radius (debris.c)
	void* m_unk0x44;                     // 0x44 — FUN_1006e9e6's bounding box when m_unk0x24 is 0
	undefined4 m_unk0x48;                // 0x48
};

// The functions and globals of unk1003a530.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechU32 FUN_1003a530(void);
	MechU32 FUN_1003a545(MechU32 p_flags);
	void FUN_1003a56b(ScarletOrchid0x4c* p_shape);
	void FUN_1003a589(ScarletOrchid0x4c* p_shape, GraniteLattice0x18* p_model);
	void FUN_1003a59d(ScarletOrchid0x4c* p_shape);
	GraniteLattice0x18* FUN_1003a5f3(
		ScarletOrchid0x4c* p_shape,
		MechS32 p_key,
		MechS32 p_vertexCount,
		MechS32 p_faceCount,
		MechS32 p_extra,
		void** p_extraData
	);
	void FUN_1003a7a2(ScarletOrchid0x4c* p_shape, MechS32 p_key);
	void FUN_1003a7f9(ScarletOrchid0x4c* p_shape, MechS32 p_key);
	MechS32 FUN_1003a827(ScarletOrchid0x4c* p_shape);
	void FUN_1003a859(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x14);
	MechU32 FUN_1003a889(ScarletOrchid0x4c* p_shape);
	ScarletOrchid0x4c* FUN_1003a8c1(MechS32 p_vertexCount, MechS32 p_faceCount, MechS32 p_extra, void** p_extraData);
	void FUN_1003aa1d(
		ScarletOrchid0x4c* p_shape,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z,
		undefined4 p_unk0x18,
		undefined4 p_unk0x1c
	);
	struct DuskMoth0x24* FUN_1003aab5(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x00, MechU8* p_indices);
	void FUN_1003ab34(ScarletOrchid0x4c* p_shape, struct DuskMoth0x24* p_face, MechU32 p_index);
	void FUN_1003ab79(GraniteLattice0x18* p_model);
	void FUN_1003aba5(ScarletOrchid0x4c* p_shape);
	void FUN_1003ac5f(ScarletOrchid0x4c* p_shape);
	void FUN_1003acbe(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x04);
	MechU32 FUN_1003acf7(ScarletOrchid0x4c* p_shape);
	void FUN_1003ad2d(ScarletOrchid0x4c* p_shape, MechS32 p_unk0x02);
	void FUN_1003ad4c(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x16);
	void FUN_1003ad62(ScarletOrchid0x4c* p_shape, MechU16 p_unk0x14);
	MechU32 FUN_1003ad78(ScarletOrchid0x4c* p_shape);
	MechU32 FUN_1003ad93(ScarletOrchid0x4c* p_shape);
	MechU32 FUN_1003adae(ScarletOrchid0x4c* p_shape);
	MechS32 FUN_1003adc9(ScarletOrchid0x4c* p_shape, MechS32* p_unk0x34, MechS32* p_unk0x38, MechS32* p_unk0x3c);
	void FUN_1003ae1e(ScarletOrchid0x4c* p_shape);
	void FUN_1003ae96(ScarletOrchid0x4c* p_shape);
	void FUN_1003b0e4(struct DuskMoth0x24* p_face, struct EmberFern0x2c* p_vertices);
	void FUN_1003b43d(ScarletOrchid0x4c* p_shape, MechS32* p_vertexCount, MechS32* p_faceCount);
	void FUN_1003b48f(ScarletOrchid0x4c* p_shape, void (*p_fn)(ScarletOrchid0x4c*));
	void FUN_1003b4dd(ScarletOrchid0x4c* p_shape, MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_1003b55a(ScarletOrchid0x4c* p_shape, MechS32 p_index, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_1003b5d6(
		ScarletOrchid0x4c* p_shape,
		MechS32 p_index,
		MechU32* p_unk0x00,
		MechU32* p_count,
		MechU32* p_indices,
		MechS32 p_max
	);
	void FUN_1003b696(ScarletOrchid0x4c* p_shape, MechS32 p_index, MechS32 p_unk0x00);
	struct AmberWillow0x7c* FUN_1003b6e5(ScarletOrchid0x4c* p_shape);
	void FUN_1003b6fb(ScarletOrchid0x4c* p_shape, struct AmberWillow0x7c* p_unk0x18);
	MechU32 FUN_1003b70f(ScarletOrchid0x4c* p_shape);
	void FUN_1003b72f(ScarletOrchid0x4c* p_shape, MechU32 p_flags);
	void FUN_1003b75e(ScarletOrchid0x4c* p_shape);
	void FUN_1003b78b(ScarletOrchid0x4c* p_shape);
	MechS32 FUN_1003b7cc(ScarletOrchid0x4c* p_shape);

#ifdef __cplusplus
}
#endif

#endif // UNK1003A530_H
