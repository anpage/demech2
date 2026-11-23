#ifndef GBL_AMS_H
#define GBL_AMS_H

#include "types.h"

struct Player;
struct WeaponDef;
struct WeaponSlot;

// The functions of gbl/ams.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void ToggleAms(void);
	MechS32 AmsFrame(struct Player* p_player, struct WeaponDef* p_def, struct WeaponSlot* p_slot);
	MechS32 AmsLaunch(struct Player* p_player, struct WeaponDef* p_def, struct WeaponSlot* p_slot);
	MechS32 AmsIntercept(struct Player* p_player, struct WeaponDef* p_def, struct WeaponSlot* p_slot);

#ifdef __cplusplus
}
#endif

#endif // GBL_AMS_H
