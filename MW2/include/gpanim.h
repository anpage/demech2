#ifndef GPANIM_H
#define GPANIM_H

#include "fixedfloat.h"
#include "players.h"
#include "types.h"

// The functions and globals of gpanim.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FirstGPAnim(void);
	void StartMotion(Player* p_player);
	void StopMotion(Player* p_player);
	MechScalar* GetEyepointOffset(Player* p_player, MechScalar* p_offset);
	void UpdateMotion(Player* p_player);
	void UpdateMotionSounds(Player* p_player, MechS32 (*p_sounds)[4], MechScalar* p_offset);
	void ResetMotion(Player* p_player);

#ifdef __cplusplus
}
#endif

#endif // GPANIM_H
