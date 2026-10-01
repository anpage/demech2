#ifndef UNK10046750_H
#define UNK10046750_H

#include "copperwren.h"
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
	extern MechS32 g_unk0x1010b530;
	extern MechU8 g_unk0x1010b53c;
	extern MechU8 g_unk0x1010b5b8;
	extern CopperWren0x20* g_unk0x1010b550[20];
	extern MechS32 g_unk0x1010b5b0;
	extern Path g_paths[0x40];
	extern QuartzReel0x14* g_unk0x101079e0[0x780];

	MechS32 FUN_10046750(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_100472fe(MechS32 p_mode, MechS32 p_value);
	MechS32 LoadAnimFile(ResourceRef* p_ref);
	MechS32 FUN_10047462(void);
	MechS32 FUN_10047477(void);
	MechS32 FUN_1004748c(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_1004771e(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_100479ec(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_10047d10(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	MechS32 FUN_10047f60(MechS32 p_event, MechChar* p_data, MechS32 p_clock, MechS32 p_period);
	CopperWren0x20* FUN_10048c50(EmberFern0x2c* p_vertex);
	CopperWren0x20* FUN_10048d46(EmberFern0x2c* p_a, EmberFern0x2c* p_b);
	CopperWren0x20* FUN_10048ebe(CopperWren0x20* p_vertex);
	MechS32 FUN_10048faf(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices);
	void FUN_10049155(DuskMoth0x24* p_face, EmberFern0x2c* p_vertices);

#ifdef __cplusplus
}
#endif

#endif // UNK10046750_H
