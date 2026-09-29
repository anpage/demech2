#ifndef AI_H
#define AI_H

#include "types.h"

// The functions and globals of ai.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FirstAI(void);
	MechS32 FUN_1005212a(MechS32 p_team);
	void FUN_10054f50(MechS32 p_unk0x00, MechS32 p_unk0x04);

#ifdef __cplusplus
}
#endif

#endif // AI_H
