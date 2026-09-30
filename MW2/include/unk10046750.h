#ifndef UNK10046750_H
#define UNK10046750_H

#include "duskmoth.h"
#include "emberfern.h"
#include "path.h"
#include "quartzreel.h"
#include "resourceref.h"
#include "types.h"

// The functions and globals of unk10046750.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_pathCount;
	extern MechS32 g_unk0x100a6d6c;
	extern MechS32 g_unk0x100a6d70;
	extern MechS32 g_unk0x100a6d74;
	extern Path g_paths[0x40];
	extern QuartzReel0x14* g_unk0x101079e0[0x780];

	void FUN_10046750(void);
	MechS32 FUN_100472fe(MechS32 p_mode, MechS32 p_value);
	MechS32 LoadAnimFile(ResourceRef* p_ref);
	MechS32 FUN_10047462(void);
	MechS32 FUN_10047477(void);
	MechS32 FUN_1004748c(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_1004771e(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_100479ec(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_10047d10(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	void FUN_10049155(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices);

#ifdef __cplusplus
}
#endif

#endif // UNK10046750_H
