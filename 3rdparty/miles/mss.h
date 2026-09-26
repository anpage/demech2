/* Miles Sound System (WAIL32.DLL) declarations: only what the game calls, with the types its
   matched callers prove. There is no SDK in the tree; add functions here as callers are
   decompiled. The import library is generated from wail32.def. */
#ifndef MSS_H
#define MSS_H

#include <windows.h>
// windows.h first: mmsystem.h depends on it
#include <mmsystem.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define AILCALL __stdcall
#define AILIMPORT __declspec(dllimport)

	typedef struct _MDI_DRIVER* HMDIDRIVER;
	typedef struct _SEQUENCE* HSEQUENCE;
	typedef struct _SAMPLE* HSAMPLE;
	typedef struct _DIG_DRIVER* HDIGDRIVER;

#define SMP_PLAYING 4

	AILIMPORT void AILCALL AIL_startup(void);
	AILIMPORT void AILCALL AIL_shutdown(void);

	AILIMPORT int AILCALL AIL_midiOutOpen(HMDIDRIVER* p_driver, LPHMIDIOUT* p_midiOut, int p_deviceId);
	AILIMPORT int AILCALL AIL_active_sequence_count(HMDIDRIVER p_driver);

	AILIMPORT int AILCALL
	AIL_waveOutOpen(HDIGDRIVER* p_driver, LPHWAVEOUT* p_waveOut, int p_deviceId, LPWAVEFORMAT p_format);
	AILIMPORT void AILCALL AIL_waveOutClose(HDIGDRIVER p_driver);
	AILIMPORT int AILCALL AIL_active_sample_count(HDIGDRIVER p_driver);

	AILIMPORT HSEQUENCE AILCALL AIL_allocate_sequence_handle(HMDIDRIVER p_driver);
	AILIMPORT void AILCALL AIL_release_sequence_handle(HSEQUENCE p_sequence);
	AILIMPORT int AILCALL AIL_init_sequence(HSEQUENCE p_sequence, void* p_start, int p_sequenceNum);
	AILIMPORT void AILCALL AIL_start_sequence(HSEQUENCE p_sequence);
	AILIMPORT void AILCALL AIL_stop_sequence(HSEQUENCE p_sequence);
	AILIMPORT void AILCALL AIL_set_sequence_volume(HSEQUENCE p_sequence, int p_volume, int p_milliseconds);
	AILIMPORT void AILCALL AIL_set_sequence_loop_count(HSEQUENCE p_sequence, int p_loopCount);

	AILIMPORT HSAMPLE AILCALL AIL_allocate_sample_handle(HDIGDRIVER p_driver);
	AILIMPORT void AILCALL AIL_release_sample_handle(HSAMPLE p_sample);
	AILIMPORT void AILCALL AIL_init_sample(HSAMPLE p_sample);
	AILIMPORT int AILCALL AIL_set_sample_file(HSAMPLE p_sample, void* p_fileImage, int p_block);
	AILIMPORT void AILCALL AIL_start_sample(HSAMPLE p_sample);
	AILIMPORT void AILCALL AIL_stop_sample(HSAMPLE p_sample);
	AILIMPORT void AILCALL AIL_end_sample(HSAMPLE p_sample);
	AILIMPORT unsigned int AILCALL AIL_sample_status(HSAMPLE p_sample);
	AILIMPORT void AILCALL AIL_set_sample_volume(HSAMPLE p_sample, int p_volume);
	AILIMPORT void AILCALL AIL_set_sample_loop_count(HSAMPLE p_sample, int p_loopCount);

#ifdef __cplusplus
}
#endif

#endif /* MSS_H */
