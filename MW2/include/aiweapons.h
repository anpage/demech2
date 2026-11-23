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
	MechS32 DecideAIFire(struct Player* p_player);
#ifdef MW2_PILOT_TABLE
	MechS32 GroupAIWeapons(struct Player* p_player, MechS32 p_group, MechS32 p_unused);
#endif

#ifdef __cplusplus
}
#endif

#endif // AIWEAPONS_H
