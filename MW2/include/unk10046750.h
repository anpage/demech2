#ifndef UNK10046750_H
#define UNK10046750_H

#include "path.h"
#include "types.h"

// The functions and globals of unk10046750.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_pathCount;
	extern MechS32 g_unk0x100a6d70;
	extern Path g_paths[0x40];

	void FUN_10046750(void);
	MechS32 FUN_100472fe(MechS32 p_mode, MechS32 p_value);
	MechS32 FUN_10047462(void);
	MechS32 FUN_10047477(void);

#ifdef __cplusplus
}
#endif

#endif // UNK10046750_H
