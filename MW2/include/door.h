#ifndef DOOR_H
#define DOOR_H

#include "decomp.h"
#include "types.h"

struct Mech;
struct Player;

// The functions of door.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_100680a0(struct Player* p_player);
	void FUN_1006831a(struct Mech* p_mech);
	void FUN_1006844e(struct Mech* p_mech);
	void FUN_10068758(struct Mech* p_mech);
	MechS32 FUN_10068772(MechS32 p_index, struct Player* p_player);

#ifdef __cplusplus
}
#endif

#endif // DOOR_H
