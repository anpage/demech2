#ifndef UNK10016AD0_H
#define UNK10016AD0_H

struct Mech;
struct Player;

#include "decomp.h"
#include "types.h"

// The functions and globals of unk10016ad0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10016ad0(struct Player* p_player);
	void FUN_1001975a(struct Mech* p_mech);
	void FUN_1001978e(struct Mech* p_mech);
	MechS32 FUN_100197ca(undefined4 p_unk0x00, struct Player* p_player);
	void FUN_10019881(struct Mech* p_mech);
	MechS32 FUN_10019a0a(void);
	MechS32 FUN_10019a3c(struct Player* p_player);
	void FUN_10019a61(struct Player* p_player, MechS32 p_weapon);
	MechS32 FUN_10019a84(struct Player* p_player);

#ifdef __cplusplus
}
#endif

#endif // UNK10016AD0_H
