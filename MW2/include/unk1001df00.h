#ifndef UNK1001DF00_H
#define UNK1001DF00_H

#include "decomp.h"
#include "ray.h"
#include "types.h"

struct DuskMoth0x24;
#include "unk10036230.h"

// A quadtree node: its bounds, its four children (m_unk0x18 == 0) and m_unk0x18 entries
// after the header (undefined4 each). FUN_1001e429 allocates it; ScarletOrchid0x4c::m_unk0x44 holds the root
// when the shape's m_unk0x24 is 5.
// SIZE 0x2c
typedef struct AzureThicket0x2c {
	MechS32 m_unk0x00;                      // 0x00
	MechS32 m_unk0x04;                      // 0x04
	MechS32 m_unk0x08;                      // 0x08
	MechS32 m_unk0x0c;                      // 0x0c
	MechS32 m_unk0x10;                      // 0x10
	MechS32 m_unk0x14;                      // 0x14
	MechS32 m_unk0x18;                      // 0x18
	struct AzureThicket0x2c* m_children[4]; // 0x1c
} AzureThicket0x2c;

// The functions and globals of unk1001df00.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	AzureThicket0x2c* FUN_1001e429(
		undefined4 p_unk0x00,
		undefined4 p_unk0x04,
		undefined4 p_unk0x08,
		undefined4 p_unk0x0c,
		undefined4 p_unk0x10,
		undefined4 p_unk0x14,
		MechS32 p_unk0x18
	);
	void FUN_1001e50d(AzureThicket0x2c* p_node);
	MechS32 FUN_1001e57a(
		struct DuskMoth0x24* p_face,
		GraniteLattice0x18* p_model,
		MechS32 p_minX,
		MechS32 p_maxX,
		MechS32 p_minZ,
		MechS32 p_maxZ,
		MechS32* p_minY,
		MechS32* p_maxY
	);
	MechS32 FUN_1001e6dc(AzureThicket0x2c* p_node, GraniteLattice0x18* p_model, MechS32 p_x, MechS32 p_y, MechS32 p_z);
	MechS32 FUN_1001e90f(AzureThicket0x2c* p_node, GraniteLattice0x18* p_model, Ray* p_ray);
	MechS32 FUN_1001eb25(AzureThicket0x2c* p_node, GraniteLattice0x18* p_model, Ray* p_ray);
	MechS32 FUN_1001ebfa(
		AzureThicket0x2c* p_node,
		GraniteLattice0x18* p_model,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z,
		MechS32* p_top
	);
	void FUN_1001edfa(void);
	MechS32 FUN_1001ee0f(AzureThicket0x2c* p_node);

#ifdef __cplusplus
}
#endif

#endif // UNK1001DF00_H
