#ifndef UNK10021460_H
#define UNK10021460_H

#include "decomp.h"
#include "types.h"

// The functions and globals of unk10021460.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10021460(void);
	void FUN_1002152b(void);
	void FUN_100215a4(void);
	void FUN_1002161d(void);
	void FUN_1002187c(void);
	MechS32 FUN_100219f5(
		MechS32 p_delay,
		MechS32 p_bearing,
		MechS32 p_id,
		MechU32 p_volume,
		MechS32 p_pan,
		MechS32 p_flags
	);
	void FUN_10021c49(void);

#ifdef __cplusplus
}
#endif

#endif // UNK10021460_H
