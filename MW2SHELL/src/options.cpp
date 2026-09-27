#include "audiosample.h"
#include "audiosubsystem.h"
#include "decomp.h"
#include "hollowreed0x110.h"
#include "slatetab0x2c.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "videodriver.h"

#include <stdio.h>

extern AudioSubsystem* g_pAudioSubsystem;
extern TMPackDataBase* g_pDatabaseMw2;
extern HollowReed0x110* g_unk0x100711f8;
extern VideoDriver* g_pVideoDriver;

extern void* AllocateAllowNew(MechS32 p_size);
// SIZE 0x18
class RandomName0x18 {
public:
	RandomName0x18(MechChar* p_image, MechS32 p_width, MechS32 p_height);

private:
	undefined m_unk0x00[0x18]; // 0x00
};

extern void FUN_100078cd(SlateTab0x2c* p_clickables);
void CalledWhenCombatVarsOptionClicked(MechS32);
extern SlateTab0x2c ClickableThing_ARRAY_10070da8[15];
extern MechS32 g_effectsVolume;
extern MechS32 g_midiVolume;

// GLOBAL: MW2SHELL 0x10070d90
void* DAT_10070d90 = NULL;

// GLOBAL: MW2SHELL 0x1007116c
MechChar s_amwlogo1_1007116c[0x10] = "amwlogo1";

// GLOBAL: MW2SHELL 0x10071680
MechS32 g_unk0x10071680 = 0x10000;

// GLOBAL: MW2SHELL 0x1007168c
MechS32 g_unk0x1007168c = 1;
// GLOBAL: MW2SHELL 0x10071690
MechS32 g_unk0x10071690 = 1;
// GLOBAL: MW2SHELL 0x10071694
MechS32 g_unk0x10071694 = 1;
// GLOBAL: MW2SHELL 0x10071698
MechS32 g_unk0x10071698 = 1;
// GLOBAL: MW2SHELL 0x1007169c
MechS32 g_unk0x1007169c = 1;
// GLOBAL: MW2SHELL 0x100716a4
MechS32 g_unk0x100716a4 = 0;

// GLOBAL: MW2SHELL 0x10092c18
AudioSample* DAT_10092c18;
// GLOBAL: MW2SHELL 0x10092c30
undefined g_unk0x10092c30[0x300];
// GLOBAL: MW2SHELL 0x10092f30
void* DAT_10092f30;

// GLOBAL: MW2SHELL 0x100716b8
undefined g_unk0x100716b8[0x17] = {0, 0, 1, 1, 1, 1, 0, 0, 0, 1};

// STUB: MW2SHELL 0x100109a0
void FUN_100109a0(void (*)(MechS32))
{
	STUB(0x100109a0);
}

DECOMP_SIZE_ASSERT(RandomName0x18, 0x18)

// STUB: MW2SHELL 0x1001603a
RandomName0x18::RandomName0x18(MechChar*, MechS32, MechS32)
{
	STUB(0x1001603a);
}

// STUB: MW2SHELL 0x1004338a
EmberGlyph0x3e* FUN_1004338a(SlateTab0x2c*)
{
	STUB(0x1004338a);
	return NULL;
}

// STUB: MW2SHELL 0x100433dc
EmberGlyph0x3e* FUN_100433dc(SlateTab0x2c*)
{
	STUB(0x100433dc);
	return NULL;
}

// STUB: MW2SHELL 0x1004343c
EmberGlyph0x3e* FUN_1004343c(SlateTab0x2c*)
{
	STUB(0x1004343c);
	return NULL;
}

// STUB: MW2SHELL 0x1004349f
EmberGlyph0x3e* FUN_1004349f(SlateTab0x2c*)
{
	STUB(0x1004349f);
	return NULL;
}

// STUB: MW2SHELL 0x10043517
EmberGlyph0x3e* FUN_10043517(SlateTab0x2c*)
{
	STUB(0x10043517);
	return NULL;
}

// STUB: MW2SHELL 0x10043589
EmberGlyph0x3e* FUN_10043589(SlateTab0x2c*)
{
	STUB(0x10043589);
	return NULL;
}

// STUB: MW2SHELL 0x100435e9
EmberGlyph0x3e* FUN_100435e9(SlateTab0x2c*)
{
	STUB(0x100435e9);
	return NULL;
}

// STUB: MW2SHELL 0x10043651
void FUN_10043651(SlateTab0x2c*)
{
	STUB(0x10043651);
}

// FUNCTION: MW2SHELL 0x10043688
void FUN_10043688(SlateTab0x2c* p_toggle)
{
	MechS32* value;

	value = (MechS32*) p_toggle->m_unk0x24;
	if (*value) {
		*value = 0;
	}
	else {
		*value = 1;
	}
}

// STUB: MW2SHELL 0x100436c7
void FUN_100436c7(SlateTab0x2c*)
{
	STUB(0x100436c7);
}

// STUB: MW2SHELL 0x10043703
void FUN_10043703(SlateTab0x2c*)
{
	STUB(0x10043703);
}

// STUB: MW2SHELL 0x10043758
EmberGlyph0x3e* FUN_10043758(SlateTab0x2c*)
{
	STUB(0x10043758);
	return NULL;
}

// STUB: MW2SHELL 0x10043790
EmberGlyph0x3e* FUN_10043790(SlateTab0x2c*)
{
	STUB(0x10043790);
	return NULL;
}

// STUB: MW2SHELL 0x1004381b
void FUN_1004381b(SlateTab0x2c*)
{
	STUB(0x1004381b);
}

// STUB: MW2SHELL 0x10043926
void FUN_10043926()
{
	STUB(0x10043926);
}

// FUNCTION: MW2SHELL 0x10043979
void FUN_10043979()
{
	FILE* file;

	file = fopen("MW2DIF.CFG", "rb");
	if (file != NULL) {
		fread(g_unk0x100716b8, 0x17, 1, file);
		fclose(file);
	}
}

// FUNCTION: MW2SHELL 0x100439cc
void FUN_100439cc()
{
	FILE* file;

	file = fopen("MW2DIF.CFG", "wb");
	if (file != NULL) {
		fwrite(g_unk0x100716b8, 0x17, 1, file);
		fclose(file);
	}
}

// Stack-slot permutation: original paletteSize is at [ebp-0x10] and audioSize at
// [ebp-0x14]; VC++ assigns them [ebp-0x14] and [ebp-0x10] here.
// FUNCTION: MW2SHELL 0x10043a72
void FUN_10043a72()
{
	MechS32 paletteSize;
	void* audioData;
	MechS32 audioSize;
	g_pVideoDriver->GetPalette(g_unk0x10092c30);
	g_pDatabaseMw2->GetDBItem(8, &DAT_10092f30, &paletteSize);
	g_pDatabaseMw2->GetDBItem(0x66, &audioData, &audioSize);
	DAT_10092c18 = new AudioSample(g_pAudioSubsystem, audioData, audioSize);
	DAT_10092c18->SetVolume(0x32);
	g_pVideoDriver->LoadPalette(3);
	*(MechS32*) ((MechU8*) g_pVideoDriver + 0x3a6) = 0;
	g_pVideoDriver->FUN_100071ad(0x177, 0x7c, 0x102, 0x160);
	DAT_10070d90 = NULL;
	DAT_10070d90 = new RandomName0x18(s_amwlogo1_1007116c, 0x78, 4);
	g_unk0x100711f8->FUN_100440ed();
	FUN_10043926();
	FUN_10043979();
	FUN_100078cd(ClickableThing_ARRAY_10070da8);
	FUN_100109a0(CalledWhenCombatVarsOptionClicked);
}

// STUB: MW2SHELL 0x10043c1f
void CalledWhenCombatVarsOptionClicked(MechS32)
{
	STUB(0x10043c1f);
}

// Callbacks and value pointers come from the original 15-entry options table.
#define OPTION_ROW(x, y, width, draw, click, value) {x, y, width, -1, 0, NULL, NULL, draw, click, value, NULL}
#define OPTION_BAR(x, y, width, height, draw, click, value)                                                            \
	{x, y, width, height, 0, NULL, NULL, draw, click, value, NULL}
// GLOBAL: MW2SHELL 0x10070da8
SlateTab0x2c ClickableThing_ARRAY_10070da8[15] = {
	OPTION_ROW(0x189, 0xdb, 100, FUN_1004338a, FUN_10043651, g_unk0x100716b8 + 5),
	OPTION_ROW(0x189, 0xef, 100, FUN_1004343c, FUN_100436c7, g_unk0x100716b8 + 4),
	OPTION_ROW(0x189, 0x115, 100, FUN_100433dc, FUN_10043688, &g_unk0x1007168c),
	OPTION_ROW(0x189, 0x129, 100, FUN_100433dc, FUN_10043688, &g_unk0x10071690),
	OPTION_ROW(0x189, 0x13d, 100, FUN_10043589, FUN_10043688, &g_unk0x10071694),
	OPTION_ROW(0x189, 0x151, 100, FUN_10043589, FUN_10043688, &g_unk0x10071698),
	OPTION_ROW(0x189, 0x165, 100, FUN_100433dc, FUN_10043688, &g_unk0x1007169c),
	OPTION_ROW(0x189, 0x179, 100, FUN_100435e9, FUN_10043703, &g_unk0x100716a4),
	OPTION_ROW(0x189, 0x1a0, 100, FUN_1004349f, FUN_100436c7, g_unk0x100716b8 + 1),
	OPTION_ROW(0x189, 0x1b4, 100, FUN_1004349f, FUN_100436c7, g_unk0x100716b8),
	OPTION_ROW(0x189, 0x1c8, 100, FUN_10043517, FUN_100436c7, g_unk0x100716b8 + 3),
	OPTION_BAR(0x14f, 0x80, 0x11d, 0x4e, FUN_10043758, NULL, NULL),
	OPTION_BAR(0x14f, 0x80, 0x11d, 0x15, FUN_10043790, FUN_1004381b, &g_midiVolume),
	OPTION_BAR(0x14f, 0x98, 0x11d, 0x15, FUN_10043790, FUN_1004381b, &g_effectsVolume),
	OPTION_BAR(0x14f, 0xb0, 0x11d, 0x15, FUN_10043790, FUN_1004381b, &g_unk0x10071680),
};
#undef OPTION_ROW
#undef OPTION_BAR

// STUB: MW2SHELL 0x10043e25
MechS32 ShowDialog(const char* p_text, MechS32 p_unk0x04)
{
	STUB(0x10043e25);
	return 0;
}
