#ifndef ARTILLERY_H
#define ARTILLERY_H

#include "decomp.h"
#include "types.h"

struct Mech;
struct Player;

// The functions of artillery.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10059fc0(struct Player* p_player);
	void FUN_1005a203(struct Mech* p_mech);
	void FUN_1005a2ea(struct Mech* p_mech);
	void FUN_1005a61d(struct Mech* p_mech);
	MechS32 FUN_1005a637(MechS32 p_index, struct Player* p_player);

#ifdef __cplusplus
}
#endif

#endif // ARTILLERY_H
