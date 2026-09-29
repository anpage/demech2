#ifndef UNK100509A0_H
#define UNK100509A0_H

#include "cobaltharbor.h"
#include "slateheron.h"
#include "types.h"

// The functions and globals of unk100509a0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a88e8;

	void FUN_100509a0(void);
	void FUN_100509c8(CobaltHarbor0x88* p_panel);
	void FUN_10050dc3(SlateHeron0x68* p_saved);
	void FUN_10050e20(CobaltHarbor0x88* p_panel);
	void FUN_10050e6c(CobaltHarbor0x88* p_panel, MechS32 p_color, MechS32 p_unk0x08);
	void FUN_10050ebe(CobaltHarbor0x88* p_panel);
	void FUN_10050fd6(CobaltHarbor0x88* p_panel);

#ifdef __cplusplus
}
#endif

#endif // UNK100509A0_H
