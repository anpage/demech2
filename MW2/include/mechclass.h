#ifndef MECHCLASS_H
#define MECHCLASS_H

struct Mech;
struct Player;

#include "decomp.h"
#include "types.h"

// The functions and globals of mechclass.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100be00c;
	extern MechS32 g_unk0x100a2be4;
	extern MechS32 g_unk0x100a2be8;
	extern MechS32 g_unk0x100a2bec;
	extern MechS32 g_unk0x100a2bf0;
	extern MechS32 g_unk0x100a2bf8;
	extern MechS32 g_localMechLost;
	extern MechS32 g_unk0x100a2c08;
	extern MechS32 g_unk0x100a2c10;
	extern MechS32 g_unk0x100a2c18;

	void FUN_10016ad0(struct Player* p_player);
	void FUN_10016edf(struct Mech* p_mech);
	void FUN_100180cd(struct Mech* p_mech);
	void FUN_10019368(struct Mech* p_mech);
	void FUN_1001975a(struct Mech* p_mech);
	void FUN_1001978e(struct Mech* p_mech);
	MechS32 FUN_100197ca(MechS32 p_index, struct Player* p_player);
	void FUN_10019881(struct Mech* p_mech);
	MechS32 FUN_10019a0a(void);
	MechS32 GetLastSelectedWeapon(struct Player* p_player);
	void FUN_10019a61(struct Player* p_player, MechS32 p_weapon);
	MechS32 GetMechHeight(struct Player* p_player);

#ifdef __cplusplus
}
#endif

#endif // MECHCLASS_H
