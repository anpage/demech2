#ifndef WEAPONS_H
#define WEAPONS_H

#include "decomp.h"
#include "types.h"
#include "weapondef.h"

struct AmberWillow0x7c;
struct Mech;
struct Player;
struct Ray;
struct ScarletOrchid0x4c;
struct WeaponSlot;

// The functions and globals of weapons.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern WeaponDef g_weaponDefs[31];
	extern struct ScarletOrchid0x4c* g_unk0x100a6d34;
	extern MechS32 g_unk0x101099c0[10];
	extern MechS32 g_unk0x101099f0[10];

	void FUN_10044740(struct Mech* p_mech);
	void UpdateWeaponFireState(struct Mech* p_mech);
	MechS32 SpawnShot(struct Player* p_player, struct WeaponSlot* p_slot);
	void FUN_10045449(struct Mech* p_mech, MechS32 p_wrap);
	void FUN_10045567(struct Mech* p_mech);
	void FUN_1004567b(struct Mech* p_mech);
	MechS32 FUN_100457d3(struct Mech* p_mech, MechS32 p_group);
	MechS32 FUN_10045919(struct Mech* p_mech);
	MechS32 FUN_1004597b(struct Mech* p_mech);
	void FUN_10045a5b(void);
	void FUN_10045b14(struct Mech* p_mech, MechS32 p_index, MechS32 p_group);
	void FUN_10045b56(MechS32 p_group);
	void FUN_10045b9c(void);
	void FUN_10045bc8(struct Mech* p_mech);
	void FUN_10045cd8(void);
	void FUN_10045e25(struct Mech* p_mech);
	void FUN_10045eac(struct Mech* p_mech);
	struct ScarletOrchid0x4c* FUN_10046269(struct Player* p_player);
	MechS32 FUN_1004635c(struct Player* p_player);
	void GetMechAimDirection(struct Player* p_player, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_100463e5(struct Player* p_player, struct Ray* p_ray);
	void FUN_10046466(struct Player* p_player, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_100464f3(struct Player* p_player, MechS32* p_x, MechS32* p_y, MechS32* p_z);
	void FUN_10046519(struct Player* p_player, struct AmberWillow0x7c* p_obj);
	void SpawnLaunchFx(
		struct Player* p_player,
		struct AmberWillow0x7c* p_obj,
		MechS32 p_dx,
		MechS32 p_dy,
		MechS32 p_dz,
		MechS32 p_spread
	);

#ifdef __cplusplus
}
#endif

#endif // WEAPONS_H
