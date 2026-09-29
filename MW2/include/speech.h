#ifndef SPEECH_H
#define SPEECH_H

#include "decomp.h"
#include "types.h"

// The functions and globals of speech.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void AdvanceSpeechQueue(void);
	void FUN_10059ed2(MechS32 p_formation);
	void PlayCockpitSound(undefined4 p_unk0x00, undefined4 p_unk0x04);
	void FUN_10059e63(MechS32 p_message, MechS32 p_slot);

#ifdef __cplusplus
}
#endif

#endif // SPEECH_H
