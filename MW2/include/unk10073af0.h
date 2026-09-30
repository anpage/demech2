#ifndef UNK10073AF0_H
#define UNK10073AF0_H

#include "decomp.h"
#include "menupage.h"
#include "types.h"

// The functions of unk10073af0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void FUN_10073af0(
		undefined4 p_unk0x00,
		undefined4 p_unk0x04,
		MechS32 p_index,
		undefined4 p_unk0x0c,
		undefined4 p_unk0x10,
		MenuPage* p_page
	);
	void FUN_10073ba6(
		undefined4 p_unk0x00,
		undefined4 p_unk0x04,
		MechS32 p_index,
		undefined4 p_unk0x0c,
		undefined4 p_unk0x10,
		MenuPage* p_page
	);
	void* ReadVfxBin(MechChar* p_name);
	void FUN_10073cb5(void);

#ifdef __cplusplus
}
#endif

#endif // UNK10073AF0_H
