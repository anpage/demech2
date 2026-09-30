#ifndef UNK100335D0_H
#define UNK100335D0_H

#include "types.h"

struct AmberWillow0x7c;
struct CopperVale0x20;
struct Eyepoint;
struct IvoryDelta0xc;
struct ScarletOrchid0x4c;

// An entry of the depth-sorted draw lists (g_unk0x100c269c, g_unk0x100c2280): a polygon, or a
// shape whose polygons are queued once the list is sorted (IvoryDelta0xc::m_count bit 15).
// SIZE 0x8
typedef struct AmberDune0x8 {
	struct IvoryDelta0xc* m_poly; // 0x00
	MechS32 m_depth;              // 0x04
} AmberDune0x8;

// The functions and globals of unk100335d0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a54b0;
	extern MechS32 g_unk0x100a54b4;
	extern MechS32 g_unk0x100a54b8;
	extern MechS32 g_unk0x1010b5a0;
	extern MechS32 g_unk0x1010b5a4;
	extern AmberDune0x8* g_unk0x1010b5c4;
	extern MechU32 g_unk0x1010b5c8;
	extern MechS32 g_unk0x1010b5cc;

	MechS32 FUN_100335d0(struct ScarletOrchid0x4c* p_shape, MechS32 p_depth);
	void FUN_1003378e(AmberDune0x8* p_first, AmberDune0x8* p_last);
	void FUN_100338bb(struct ScarletOrchid0x4c* p_root);
	void FUN_10033a06(void);
	void FUN_10033b9e(struct AmberWillow0x7c* p_root);
	void FUN_10033c4b(struct AmberWillow0x7c* p_object);
	void FUN_10033d0f(struct ScarletOrchid0x4c* p_root, struct Eyepoint* p_eyepoint);
	void FUN_10033d32(struct IvoryDelta0xc* p_poly);
	MechS32 FUN_10033e92(struct IvoryDelta0xc* p_poly, MechU32* p_points);
	MechS32 FUN_10034499(MechS32 p_a0, MechS32 p_a1, MechS32 p_edge, MechS32 p_isX, MechS32 p_z0, MechS32 p_z1);
	void FUN_10034571(
		struct CopperVale0x20* p_a,
		struct CopperVale0x20* p_b,
		MechS32 p_x,
		struct CopperVale0x20* p_out
	);
	void FUN_10034779(
		struct CopperVale0x20* p_a,
		struct CopperVale0x20* p_b,
		MechS32 p_y,
		struct CopperVale0x20* p_out
	);

#ifdef __cplusplus
}
#endif

#endif // UNK100335D0_H
