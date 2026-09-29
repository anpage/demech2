#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "cobaltharbor.h"
#include "decomp.h"
#include "types.h"

// The functions and globals of environment.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100ba5fc;
	extern MechS32 g_unk0x100ba600;
	extern MechS32 g_unk0x100ba604;
	extern CobaltHarbor0x88* g_unk0x100c3280;

	void FirstEnvironment(void);
	void FUN_1007d6bb(void);
	void FUN_1007d7e3(MechS32 p_index);
	MechS32 FUN_1007d875(void);
	void FUN_1007d88a(undefined4 p_unk0x00, MechS32 p_state);
	void FUN_1007d931(void);
	void FUN_1007d97c(void);

#ifdef __cplusplus
}
#endif

#endif // ENVIRONMENT_H
