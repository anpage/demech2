#ifndef UNK10013430_H
#define UNK10013430_H

#include "types.h"

struct Mech;
struct Player;

// The functions and globals of unk10013430.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10013430(struct Player* p_player, MechU16 p_target);
	void FUN_100139e9(struct Player* p_player);
	void FUN_1001450e(struct Player* p_player);
	void FUN_100155e1(struct Player* p_player);
	MechS32 FUN_10015709(struct Player* p_player);
	MechS32 FUN_10015fa8(struct Mech* p_mech);

#ifdef __cplusplus
}
#endif

#endif // UNK10013430_H
