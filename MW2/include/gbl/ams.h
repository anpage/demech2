#ifndef GBL_AMS_H
#define GBL_AMS_H

#include "types.h"

struct Player;
struct WeaponDef;
struct WeaponSlot;

// The functions of gbl/ams.c that other units use: the AMS's weapon functions (g_weaponDefs).
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_1002e997(struct Player* p_player, struct WeaponDef* p_def, struct WeaponSlot* p_slot);
	MechS32 FUN_1002eb5f(struct Player* p_player, struct WeaponDef* p_def, struct WeaponSlot* p_slot);

#ifdef __cplusplus
}
#endif

#endif // GBL_AMS_H
