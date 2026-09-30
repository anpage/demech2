#ifndef UNK100758A0_H
#define UNK100758A0_H

#include "types.h"

struct Mech;
struct Player;
struct ScarletOrchid0x4c;

// The functions and globals of unk100758a0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 FUN_100758a0(
		struct Mech* p_mech,
		struct ScarletOrchid0x4c** p_hit,
		struct Player** p_player,
		MechS32 p_dx,
		MechS32 p_dy,
		MechS32 p_dz,
		MechS32* p_x,
		MechS32* p_y,
		MechS32* p_z
	);
	MechS32 FUN_10075d7b(struct Mech* p_mech, MechS32* p_x, MechS32* p_y, MechS32* p_z, struct Player** p_hit);
	MechS32 FUN_10076295(
		struct Mech* p_mech,
		MechS32* p_x,
		MechS32* p_y,
		MechS32* p_z,
		struct ScarletOrchid0x4c** p_hit
	);
	void FUN_100765f8(struct Mech* p_mech, struct Mech* p_other);
	void FUN_1007669e(struct Mech* p_mech, struct Mech* p_other, MechS32 p_damage);
	void FUN_100768a8(struct Mech* p_mech, struct ScarletOrchid0x4c* p_shape);
	void FUN_10076a23(struct Mech* p_mech);

#ifdef __cplusplus
}
#endif

#endif // UNK100758A0_H
