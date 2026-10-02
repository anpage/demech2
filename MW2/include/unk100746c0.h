#ifndef UNK100746C0_H
#define UNK100746C0_H

#include "cobaltharbor.h"
#include "rendertarget.h"
#include "types.h"

// The functions and globals of unk100746c0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_100746c0(CobaltHarbor0x88* p_panel);
	void FUN_100747c9(CobaltHarbor0x88* p_panel);
	void FUN_100747d4(CobaltHarbor0x88* p_panel, undefined4 p_unk0x08);
	void FUN_100747e8(CobaltHarbor0x88* p_panel, MechS32 p_unk0x0c);
	void FUN_100747fc(CobaltHarbor0x88* p_panel, const MechChar* p_name);
	void FUN_10074823(CobaltHarbor0x88* p_panel, Pane* p_target);
	void FUN_10074879(CobaltHarbor0x88* p_panel, Point* p_unk0x34);
	void FUN_1007488d(CobaltHarbor0x88* p_panel, struct RectTransition* p_transition);
	void FUN_100748a1(CobaltHarbor0x88* p_panel, MechS32 p_x, MechS32 p_y, MechS32 p_width, MechS32 p_height);
	void FUN_100748d4(CobaltHarbor0x88* p_panel, MechS32 p_unk0x06);
	void FUN_100748e9(CobaltHarbor0x88* p_panel);
	void FUN_100748fd(CobaltHarbor0x88* p_panel);

#ifdef __cplusplus
}
#endif

#endif // UNK100746C0_H
