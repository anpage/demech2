#ifndef GPANIM_H
#define GPANIM_H

#include "players.h"
#include "types.h"

// The functions and globals of gpanim.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FirstGPAnim(void);
	void FUN_1000365a(Player* p_player);
	void FUN_1000369e(Player* p_player);
	MechS32* FUN_100036c3(Player* p_player, MechS32* p_offset);
	void FUN_100038c2(Player* p_player, MechS32 (*p_sounds)[4], MechS32* p_offset);
	void FUN_10003a10(Player* p_player);

#ifdef __cplusplus
}
#endif

#endif // GPANIM_H
