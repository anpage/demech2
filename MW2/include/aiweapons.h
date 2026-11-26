#ifndef AIWEAPONS_H
#define AIWEAPONS_H

#include "fixedfloat.h"
#include "types.h"

struct Mech;
struct Player;

// The functions and globals of aiweapons.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 RunAIWeapons(struct Player* p_player, MechScalar p_heading);
#ifdef MW2_AI_LEAD_TARGET
	MechS32 RunAIWeaponsLeading(struct Player* p_player, MechScalar p_heading);
	MechS32 RunAIWeaponsDirect(struct Player* p_player, MechScalar p_heading);
	MechS32 DecideAIFire(struct Player* p_player, MechS32* p_guided);
	MechS32 DecideAIGroupFire(struct Player* p_player, MechS32* p_guided);
	void RollAILock(struct Player* p_player);
	void LeadAITarget(struct Player* p_player, MechScalar* p_heading, MechScalar* p_pitch);
#else
MechS32 DecideAIFire(struct Player* p_player);
#endif
#ifdef MW2_PILOT_TABLE
	MechS32 GroupAIWeapons(struct Player* p_player, MechS32 p_group, MechS32 p_unused);
#endif

#ifdef __cplusplus
}
#endif

#endif // AIWEAPONS_H
