#ifndef BRIGHTNESSMENU_H
#define BRIGHTNESSMENU_H

#include "decomp.h"
#include "types.h"

// The functions of brightnessmenu.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	MechS32 GetBrightnessFraction(void);
	void PreviewBrightnessFraction(undefined4 p_unk0x00, MechS32 p_value);
	void SetBrightnessFraction(undefined4 p_unk0x00, MechS32 p_value);
	void RestoreBrightness(void);

#ifdef __cplusplus
}
#endif

#endif // BRIGHTNESSMENU_H
