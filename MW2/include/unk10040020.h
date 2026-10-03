#ifndef UNK10040020_H
#define UNK10040020_H

#include "mech.h"
#include "rendertarget.h"
#include "types.h"

// The functions and globals of unk10040020.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern PANE g_unk0x100a5cf8[16];
	extern MechS32 g_unk0x100a5eb8;

	void FUN_10040020(void);
	void FUN_10040511(Mech* p_mech, PANE* p_target);
	void FUN_100407b6(Mech* p_mech, PANE* p_target);

#ifdef __cplusplus
}
#endif

#endif // UNK10040020_H
