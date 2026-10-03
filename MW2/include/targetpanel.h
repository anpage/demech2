#ifndef TARGETPANEL_H
#define TARGETPANEL_H

#include "types.h"

struct CobaltHarbor0x88;

// The functions and globals of targetpanel.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100ba4bc;
	extern MechChar g_unk0x100c26a0[8];

	void FUN_1007b930(struct CobaltHarbor0x88* p_panel);
	void FUN_1007c126(struct CobaltHarbor0x88* p_panel);
	void FUN_1007c6df(struct CobaltHarbor0x88* p_panel);
	void FUN_1007c71e(struct CobaltHarbor0x88* p_panel);
	void FUN_1007c81c(struct CobaltHarbor0x88* p_panel);

#ifdef __cplusplus
}
#endif

#endif // TARGETPANEL_H
