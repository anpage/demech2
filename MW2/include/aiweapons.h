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

#ifdef __cplusplus
}
#endif

#endif // AIWEAPONS_H
