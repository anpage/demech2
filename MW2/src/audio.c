/* The mission's sound: the digital effects, the MIDI or CD music the mission's "MUS" resource
   names, and the warning tone when an AI player engages the local player. */
#include "audio.h"

#include "cdaudio.h"
#include "clock.h"
#include "config.h"
#include "decomp.h"
#include "loadres.h"
#include "mss.h"
#include "players.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "types.h"
#include "unk10013370.h"
#include "unk10021460.h"

#include <stdio.h>
#include <windows.h>

// The CD track the mission plays, or -1.
// GLOBAL: MW2 0x100a1490
MechS32 g_cdTrack = -1;

// The MIDI sequence the mission plays, or -1.
// GLOBAL: MW2 0x100a1494
MechS32 g_midiSequence = -1;

// GLOBAL: MW2 0x100a1498
SoundConfig g_soundConfig = {0x10000, 0x10000, 0x10000, 0x10000, 11, 1, 1, 1, 1, 1, 9, "mcga.dll"};

// GLOBAL: MW2 0x100a14d4
SoundConfig* g_mw2SndCfgData = NULL;

// GLOBAL: MW2 0x100a14d8
MechS32 g_audioPaused = 0;

// GLOBAL: MW2 0x100a14dc
MechS32 g_musicStarted = 0;

// GLOBAL: MW2 0x100bcda8
static PCMWAVEFORMAT g_waveFormat;

// GLOBAL: MW2 0x10179e80
MechS32 g_nextEngageCheck;

// FUNCTION: MW2 0x10006c5c
void StartMissionMusic(void)
{
	MechChar* music;

	if (!g_musicStarted) {
		g_musicStarted = 1;
		if (g_unk0x100e9340 <= 0 || (music = FUN_1001a19f(0, g_unk0x100e9340, g_unk0x100a86d0, 0)) == NULL) {
			return;
		}

		sscanf(music, "%d %d", &g_cdTrack, &g_midiSequence);
		FUN_1001a163(g_unk0x100e9340, g_unk0x100a86d0);
	}

	if (!FUN_1005b696() || GetCdStatus() == 1 || !FUN_1005b6ab(g_cdTrack)) {
		g_cdTrack = -1;
	}
	else if (g_soundConfig.m_unk0x10 & 8) {
		FUN_1005b22f(g_cdTrack);
		if (FUN_1005ad0a() != 3) {
			g_cdTrack = -1;
		}
		else {
			g_midiSequence = -1;
		}
	}

	g_midiSequence = -1;
}

// FUNCTION: MW2 0x10006d73
void PauseMusic(void)
{
	if (!g_audioPaused) {
		if (g_midiSequence != -1 && (g_soundConfig.m_unk0x10 & 4)) {
			PauseMidiSequences();
		}

		if (g_cdTrack != -1 && (g_soundConfig.m_unk0x10 & 8)) {
			CdAudioTogglePaused();
		}

		g_audioPaused = 1;
	}
}

// FUNCTION: MW2 0x10006dd3
void ResumeMusic(void)
{
	if (g_audioPaused) {
		if (g_midiSequence != -1 && (g_soundConfig.m_unk0x10 & 4)) {
			ResumeMidiSequences();
		}

		if (g_cdTrack != -1 && (g_soundConfig.m_unk0x10 & 8)) {
			CdAudioTogglePaused();
		}

		g_audioPaused = 0;
	}
}

// FUNCTION: MW2 0x10006e33
void StopMusic(void)
{
	if (g_cdTrack != -1) {
		FUN_1005ae7b();
	}

	if (g_midiSequence != -1) {
		StopMidiSequences();
	}
}

// FUNCTION: MW2 0x10006e62
void LoopCdMusic(void)
{
	if (!g_audioPaused && (g_soundConfig.m_unk0x10 & 8)) {
		if (FUN_1005b00b() && g_cdTrack != -1) {
			FUN_1005b22f(g_cdTrack);
			FUN_1005ad0a();
		}
	}
}

// FUNCTION: MW2 0x10006eb4
MechS32 FirstAudio(void)
{
	InitializeDigitalAudio(8);
	InitializeMidi();
	StartCdAudio();
	FUN_10013370();
	g_nextEngageCheck = g_currentClock + 0x389;
	return 1;
}

// Serves the samples and, every five seconds, sounds a warning while an AI player within
// 150000 attacks the local player.
// Operand order: g_nextEngageCheck <= g_currentClock and i < g_playerCount load the other
// operand first in the original.
// FUNCTION: MW2 0x10006ef1
void DoAudio(void)
{
	Player* player;
	MechS32 i;

	AIL_serve();
	ServeSamples();
	if (g_nextEngageCheck <= g_currentClock) {
		g_nextEngageCheck = g_currentClock + 0x389;
		if (!(g_players[g_localPlayerId]->m_flags & 0x16)) {
			for (i = 0; i < g_playerCount; i++) {
				player = g_players[i];
				if (player->m_index == g_localPlayerId) {
					continue;
				}

				if ((player->m_aiGoal & 0xff) == g_localPlayerId &&
					(player->m_aiState == 2 || player->m_aiState == 3) && player->m_targetInfo.m_unk0x04 <= 150000) {
					FUN_1007eb23(0x100, 100, 0x40, 5, 0x32);
					break;
				}
			}
		}
	}
}

// FUNCTION: MW2 0x10006ffa
void ShutdownAudio(void)
{
	StopMusic();
	DeInitCdAudio();
	ShutdownMidi();
	ShutdownDigitalAudio();
	SaveSndCfg("mw2snd.cfg", g_mw2SndCfgData);
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, g_mw2SndCfgData);
}

// FUNCTION: MW2 0x10007040
void PauseAudio(void)
{
	StopSamples(0);
	FUN_10059d8b();
	PauseMusic();
	FUN_10021c49();
}

// FUNCTION: MW2 0x10007064
void ResumeAudio(void)
{
	ResumeMusic();
	ResumeSpeech();
}

// Opens the digital driver at 11025 Hz, 8-bit stereo.
// FUNCTION: MW2 0x10007079
HDIGDRIVER OpenDigitalDriver(void)
{
	HDIGDRIVER driver;

	g_waveFormat.wf.wFormatTag = WAVE_FORMAT_PCM;
	g_waveFormat.wf.nChannels = 2;
	g_waveFormat.wf.nSamplesPerSec = 11025;
	g_waveFormat.wf.nAvgBytesPerSec = 22050;
	g_waveFormat.wf.nBlockAlign = 2;
	g_waveFormat.wBitsPerSample = 8;
	if (AIL_waveOutOpen(&driver, NULL, 0, (LPWAVEFORMAT) &g_waveFormat)) {
		return NULL;
	}
	else {
		return driver;
	}
}

// Opens the MIDI mapper, or MIDI device 0.
// FUNCTION: MW2 0x100070ee
HMDIDRIVER OpenMidiDriver(void)
{
	HMDIDRIVER driver;

	if (AIL_midiOutOpen(&driver, NULL, -1) && AIL_midiOutOpen(&driver, NULL, 0)) {
		return NULL;
	}
	else {
		return driver;
	}
}
