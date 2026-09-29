#ifndef UNK1006E970_H
#define UNK1006E970_H

#include "decomp.h"
#include "types.h"
#include "unk1003a530.h"

// The bounding box of a shape's selected model (ScarletOrchid0x4c::m_unk0x44), from the
// transformed vertices. FUN_1006ea90 allocates it inside a larger 0x78-byte block.
// SIZE 0x18
typedef struct CinderBox0x18 {
	MechS32 m_minX; // 0x00
	MechS32 m_maxX; // 0x04
	MechS32 m_minY; // 0x08
	MechS32 m_maxY; // 0x0c
	MechS32 m_minZ; // 0x10
	MechS32 m_maxZ; // 0x14
} CinderBox0x18;

// The functions and globals of unk1006e970.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	CinderBox0x18* FUN_1006e970(void);
	void FUN_1006e9e6(ScarletOrchid0x4c* p_shape);
	CinderBox0x18* FUN_1006ea90(ScarletOrchid0x4c* p_shape);
	void FUN_1006eb02(ScarletOrchid0x4c* p_shape);
	void FUN_1006eb80(
		ScarletOrchid0x4c* p_shape,
		MechS32* p_minX,
		MechS32* p_maxX,
		MechS32* p_minY,
		MechS32* p_maxY,
		MechS32* p_minZ,
		MechS32* p_maxZ
	);
	void FUN_1006ed30(ScarletOrchid0x4c* p_shape);
	MechS32 FUN_1006edc3(ScarletOrchid0x4c* p_shape);

#ifdef __cplusplus
}
#endif

#endif // UNK1006E970_H
