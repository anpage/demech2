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
	void PlayCockpitSound(undefined4 p_unk0x00, undefined4 p_unk0x04);

#ifdef __cplusplus
}
#endif

#endif // SPEECH_H
