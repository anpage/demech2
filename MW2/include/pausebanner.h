#ifndef PAUSEBANNER_H
#define PAUSEBANNER_H

#include "types.h"

// The functions and globals of pausebanner.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100a15d0;
	extern MechS32 g_unk0x100a15d4;

	void DrawPausedBanner(void);
	void PlayPauseSound(void);
	void PlayResumeSound(void);
	void FUN_10009f35(void);
	void FUN_10009f61(void);
	void FUN_10009f8d(MechU16 p_key);

#ifdef __cplusplus
}
#endif

#endif // PAUSEBANNER_H
