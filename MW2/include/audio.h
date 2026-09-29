#ifndef AUDIO_H
#define AUDIO_H

#include "types.h"

// The functions and globals of audio.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void StartMissionMusic(void);
	void LoopCdMusic(void);
	void FirstAudio(void);
	void DoAudio(void);
	void ShutdownAudio(void);
	void FUN_10007040(void);
	void FUN_10007064(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_H
