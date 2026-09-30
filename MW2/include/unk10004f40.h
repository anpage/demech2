#ifndef UNK10004F40_H
#define UNK10004F40_H

#include "decomp.h"
#include "types.h"

struct CobaltHarbor0x88;
struct Point;

// The functions and globals of unk10004f40.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a116c;
	extern MechS32 g_unk0x100a1170;
	extern MechChar g_unk0x10179e90[0x30];

	MechChar* FUN_10004f40(MechS32 p_ticks);
	MechChar* FUN_10004ff5(MechS32 p_seconds);
	MechS32 FUN_1000507d(const MechChar* p_text, void* p_font);
	void FUN_100050d1(struct CobaltHarbor0x88* p_panel, struct Point* p_pos, void* p_font, MechU8 p_priority);
	void FUN_100056f0(struct CobaltHarbor0x88* p_panel);
	void FUN_10005add(struct CobaltHarbor0x88* p_panel);
	void FUN_100060b6(struct CobaltHarbor0x88* p_panel);
	void FUN_10006189(struct CobaltHarbor0x88* p_panel);
	void FUN_10006291(struct CobaltHarbor0x88* p_panel);
	void FUN_100063cd(struct CobaltHarbor0x88* p_panel);
	void FUN_10006484(struct CobaltHarbor0x88* p_panel);
	void FUN_100065c3(struct CobaltHarbor0x88* p_panel);
	void FUN_1000667c(struct CobaltHarbor0x88* p_panel);

#ifdef __cplusplus
}
#endif

#endif // UNK10004F40_H
