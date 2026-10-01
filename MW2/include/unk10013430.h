#ifndef UNK10013430_H
#define UNK10013430_H

#include "silverbrook.h"
#include "types.h"

struct Ray;

struct Mech;
struct Player;
struct ScarletOrchid0x4c;
struct WeaponSlot;

// The functions and globals of unk10013430.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10013430(struct Player* p_player, MechU16 p_target);
	void FUN_100139e9(struct Player* p_player);
	MechS32 FUN_10013d81(struct Player* p_player);
	MechS16 FUN_100140e4(SilverBrook0x08* p_table, MechS16 p_id);
	void FUN_10014149(struct Player* p_player);
	void FUN_1001450e(struct Player* p_player);
	void FUN_10014723(struct Player* p_player, MechU32 p_unk0x04, MechS16 p_unk0x08, MechS16 p_unk0x0c);
	void FUN_100147d0(
		MechU32 p_unk0x00,
		MechS16 p_unk0x04,
		MechS32* p_x,
		MechS32* p_z,
		MechS32* p_y,
		MechS16 p_unk0x14
	);
	MechS32 FUN_1001498c(struct Player* p_player, MechS32 p_turn);
	void FUN_100149e7(struct Player* p_player, MechS16 p_target);
	void FUN_10014aa8(struct Player* p_player);
	MechS32 FUN_10014c3d(struct Player* p_player, MechS16 p_target);
	MechS32 FUN_10014d4e(struct Player* p_player, MechS16 p_target);
	MechS32 FUN_10014df1(struct Player* p_player, MechS16 p_target);
	MechS32 FUN_10014e5e(struct Player* p_player);
	MechS32 FUN_10014f23(struct Player* p_player, MechS16 p_target);
	MechS32 FUN_100150c1(struct Player* p_player, MechS16 p_target);
	MechS32 FUN_1001512e(struct Player* p_player, MechS16 p_target);
	MechS32 FUN_10015172(struct Player* p_player, MechS16 p_target);
	void FUN_10015342(struct Player* p_player, MechS16 p_target);
	MechS32 FUN_100153e6(struct Player* p_player);
	MechS32 FUN_10015520(struct Player* p_player);
	MechS32 FUN_100155e1(struct Player* p_player);
	void FUN_100156f2(struct Player* p_player, MechS8 p_value);
	MechS32 FUN_10015709(struct Player* p_player);
	MechS32 FUN_10015b40(struct ScarletOrchid0x4c* p_shape);
	void FUN_10015b9f(
		struct Player* p_player,
		struct Ray* p_ray,
		MechS32 p_side,
		MechS16 p_step,
		MechS32 p_length,
		MechS32 p_fromEdge
	);
	MechS16 FUN_10015d2a(
		struct Player* p_player,
		struct ScarletOrchid0x4c* p_shape,
		MechS32 p_x,
		MechS32 p_y,
		MechS32 p_z
	);
	MechS32 FUN_10015dd6(struct Player* p_player, MechS16 p_target);
	MechS32 FUN_10015e34(MechS32 p_value, MechS32 p_limit);
	MechS32 FUN_10015e74(struct Player* p_player, MechS32 p_y);
	MechS32 FUN_10015fa8(struct Mech* p_mech);
	void FUN_10016057(struct Player* p_player, MechS16 p_value);
	MechS32 FUN_10016093(struct Player* p_player);
	void FUN_100160eb(struct Player* p_player);
	MechS32 FUN_10016222(struct Player* p_player, MechS32 p_limit);
	struct ScarletOrchid0x4c* FUN_1001627f(MechS16 p_target);
	void FUN_1001632c(struct WeaponSlot* p_slot, struct Mech* p_mech);
	MechS32 FUN_100166b1(struct Player* p_player);
	MechS32 FUN_10016880(struct Mech* p_mech);

#ifdef __cplusplus
}
#endif

#endif // UNK10013430_H
