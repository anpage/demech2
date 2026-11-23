/* Voice messages: the lines lancemates, the cockpit and the damage reports speak, queued by
   priority and played one at a time, or shown as text without digital sound. */
#include "speech.h"

#include "audio.h"
#include "audioengine.h"
#include "clock.h"
#include "decomp.h"
#include "fixedmul.h"
#include "loadres.h"
#include "mss.h"
#include "mw2prj.h"
#include "simmain.h"
#include "soundfx.h"
#include "speechentry.h"
#include "speechline.h"
#include "timedoverlays.h"
#include "types.h"

#include <string.h>
#include <windows.h>

// The lancemates a message addresses.
// GLOBAL: MW2 0x100a96c8
// GLOBAL: MW2GBL 0x100ae860
// GLOBAL: MW2MATROX 0x100bbe98
// GLOBAL: MW23DFX 0x1024eb40
// GLOBAL: MW2ATI 0x10182e50
// GLOBAL: MW2S3 0x100732f0
// GLOBAL: MW2POWERVR 0x101adeb8
SpeechLine g_slotSpeech[3] = {
	{2, "All points", NULL},
	{0x108, "Point 2", NULL},
	{0x109, "Point 3", NULL},
};

// GLOBAL: MW2 0x100a96ec
MechS32 g_unk0x100a96ec = 0;

// What a lancemate reports.
// GLOBAL: MW2 0x100a96f0
// GLOBAL: MW2GBL 0x100ae888
// GLOBAL: MW2MATROX 0x100bbec0
// GLOBAL: MW23DFX 0x1024ec58
// GLOBAL: MW2ATI 0x10182f68
// GLOBAL: MW2S3 0x10073318
// GLOBAL: MW2POWERVR 0x101adfd0
SpeechLine g_lancemateSpeech[11] = {
	{0, "cannot do this", NULL},
	{0xa, "attacking your target", NULL},
	{0xb4, "joining formation", NULL},
	{0x10a, "patrolling your target", NULL},
	{0x71, "running awayyyyyy", NULL},
	{0x22, "reports target destroyed", NULL},
	{0x23, "destroyed", NULL},
	{0x36, "critical hit", NULL},
	{0xc, "ejecting", NULL},
	{0xd, "reports system shutdown", NULL},
	{0x2a, "reports task complete. Rejoining formation", NULL},
};

// GLOBAL: MW2 0x100a9774
MechS32 g_unk0x100a9774 = 0;

// GLOBAL: MW2 0x100a9778
// GLOBAL: MW2GBL 0x100ae910
// GLOBAL: MW2MATROX 0x100bbf48
// GLOBAL: MW23DFX 0x1024ed40
// GLOBAL: MW2ATI 0x10183050
// GLOBAL: MW2S3 0x100733a0
// GLOBAL: MW2POWERVR 0x101ae0b8
SpeechLine g_formationSpeech[7] = {
	{0xb, "Formation change to", NULL},
	{0x4f, "echelon left", NULL},
	{0x51, "echelon right", NULL},
	{0x52, "line abreast", NULL},
	{0x53, "line astern", NULL},
	{0x56, "vee form", NULL},
	{0x57, "wedge", NULL},
};

// GLOBAL: MW2 0x100a97cc
MechS32 g_unk0x100a97cc = 0;

// GLOBAL: MW2 0x100a97d0
// GLOBAL: MW2GBL 0x100ae968
// GLOBAL: MW2MATROX 0x100bbfa0
// GLOBAL: MW23DFX 0x1024edb8
// GLOBAL: MW2ATI 0x101830c8
// GLOBAL: MW2S3 0x100733f8
// GLOBAL: MW2POWERVR 0x101ae130
SpeechLine g_engageSpeech[3] = {
	{0x54, "disengaged", NULL},
	{0x50, "engaged", NULL},
	{0x54, "disengaged", NULL},
};

// GLOBAL: MW2 0x100a97f4
MechS32 g_unk0x100a97f4 = 0;

// GLOBAL: MW2 0x100a97f8
// GLOBAL: MW2GBL 0x100ae990
// GLOBAL: MW2MATROX 0x100bbfc8
// GLOBAL: MW23DFX 0x1024f140
// GLOBAL: MW2ATI 0x10183450
// GLOBAL: MW2S3 0x10073420
// GLOBAL: MW2POWERVR 0x101ae4b8
SpeechLine g_cockpitSpeech[34] = {
	{0x40, "Heat level critical", NULL},
	{0x4b, "Thermal threshold has been exceeded. Shutdown sequence initiated", NULL},
	{0x4e, "Shutdown sequence overridden", NULL},
	{0xf, "Shutdown sequence initiated", NULL},
	{0x10, "Self-destruct sequence initiated", NULL},
	{0x11, "Target is currently engaged", NULL},
	{0x12, "Chain fire", NULL},
	{0x13, "Group fire", NULL},
	{0x30, "Internal ammo explosion detected", NULL},
	{0x3a, "NARC missle probe", NULL},
	{0x3b, "Anti-missle suite", NULL},
	{0x36, "Critical hit : ", NULL},
	{0xc, "Ejecting", NULL},
	{0xe, "Shutting down", NULL},
	{0x7a, "Enemy target of opportunity identified", NULL},
	{0x7b, "Friendly target identified", NULL},
	{0x80, "Neutral target identified", NULL},
	{0x42, "External camera engaged", NULL},
	{0x43, "Satellite link established", NULL},
	{0x44, "Inspection successful", NULL},
	{0x45, "Target is beyond inspection radius", NULL},
	{0x46, "Enemy mech destroyed", NULL},
	{0x47, "Enemy vehicle destroyed", NULL},
	{0x48, "Enemy aircraft destroyed", NULL},
	{0x49, "Enemy target destroyed", NULL},
	{0x49, "Enemy target destroyed", NULL},
	{0x4a, "Enemy power-up detected", NULL},
	{0x4c, "Image enhancement", NULL},
	{0x4d, "Light amplification", NULL},
	{0x41, "Autopilot", NULL},
	{0x3c, "ECM suite", NULL},
	{0x3d, "MASC", NULL},
	{0x3f, "Atmosphere hostile : ejection aborted", NULL},
	{0x3e, "Alotted mission time has been exceeded", NULL},
};

// The damaged parts "Critical hit : " names.
// GLOBAL: MW2 0x100a9990
// GLOBAL: MW2GBL 0x100aeb28
// GLOBAL: MW2MATROX 0x100bc160
// GLOBAL: MW23DFX 0x1024f4e8
// GLOBAL: MW2ATI 0x101837f8
// GLOBAL: MW2S3 0x100735b8
// GLOBAL: MW2POWERVR 0x101ae860
SpeechLine g_damageSpeech[39] = {
	{0x14, "weapon destroyed", NULL},
	{0x30, "internal ammo explosion detected", NULL},
	{0x3a, "NARC missle probe", NULL},
	{0x3b, "anti-missle suite", NULL},
	{0x15, "jump-jet exhaust port", NULL},
	{0x16, "heat-sink", NULL},
	{0x17, "life-support", NULL},
	{0x18, "engine", NULL},
	{0x19, "gyro", NULL},
	{0x1a, "sensors", NULL},
	{0x1b, "leg", NULL},
	{0x1c, "hip", NULL},
	{0x1d, "arm", NULL},
	{0x1e, "torso", NULL},
	{0x1f, "left leg", NULL},
	{0x20, "right leg", NULL},
	{0x21, "left arm", NULL},
	{0x24, "right arm", NULL},
	{0x25, "left torso", NULL},
	{0x26, "right torso", NULL},
	{0x27, "center torso", NULL},
	{0x28, "head", NULL},
	{0x29, "LRM", NULL},
	{0x2b, "SRM", NULL},
	{0x2c, "streak SRM", NULL},
	{0x2d, "machine gun", NULL},
	{0x2e, "gauss rifle", NULL},
	{0x2f, "LBX auto cannon", NULL},
	{0x31, "Ultra auto cannon", NULL},
	{0x32, "particle cannon", NULL},
	{0x33, "ER large laser", NULL},
	{0x34, "ER medium laser", NULL},
	{0x35, "ER small laser", NULL},
	{0x37, "large pulse laser", NULL},
	{0x38, "medium pulse laser", NULL},
	{0x39, "small pulse laser", NULL},
	{0x41, "autopilot", NULL},
	{0x3c, "ECM suite", NULL},
	{0x3d, "MASC", NULL},
};

// GLOBAL: MW2 0x100a9b64
// GLOBAL: MW2GBL 0x100aecfc
// GLOBAL: MW2MATROX 0x100bc334
// GLOBAL: MW23DFX 0x1024f6bc
// GLOBAL: MW2ATI 0x101839cc
// GLOBAL: MW2S3 0x1007378c
// GLOBAL: MW2POWERVR 0x101aea34
SpeechEntry* g_speechQueue = NULL;

// GLOBAL: MW2 0x100a9b68
// GLOBAL: MW2GBL 0x100aed00
// GLOBAL: MW2MATROX 0x100bc338
// GLOBAL: MW23DFX 0x1024f6c0
// GLOBAL: MW2ATI 0x101839d0
// GLOBAL: MW2S3 0x10073790
// GLOBAL: MW2POWERVR 0x101aea38
HSAMPLE g_speechSample = NULL;

// GLOBAL: MW2 0x100a9b6c
// GLOBAL: MW2GBL 0x100aed04
// GLOBAL: MW2MATROX 0x100bc33c
// GLOBAL: MW23DFX 0x1024f6c4
// GLOBAL: MW2ATI 0x101839d4
// GLOBAL: MW2S3 0x10073794
// GLOBAL: MW2POWERVR 0x101aea3c
MechS32 g_speechLocked = 0;

// GLOBAL: MW2 0x100bea10
// GLOBAL: MW2GBL 0x100b9d80
// GLOBAL: MW2MATROX 0x10125430
// GLOBAL: MW23DFX 0x1007c890
// GLOBAL: MW2ATI 0x10071890
// GLOBAL: MW2S3 0x1007d848
// GLOBAL: MW2POWERVR 0x1006f8d8
SpeechEntry g_speechEntries[8];

// Queues p_line, followed by p_suffix, by priority (-1: the sound's own). Returns whether it
// was queued.
// FUNCTION: MW2 0x10059390
// FUNCTION: MW2GBL 0x10041b30
// FUNCTION: MW2MATROX 0x1008b510
// FUNCTION: MW23DFX 0x10031150
// FUNCTION: MW2ATI 0x1002ff40
// FUNCTION: MW2S3 0x10037100
// FUNCTION: MW2POWERVR 0x1002a9c0
MechS32 QueueSpeech(SpeechLine* p_line, SpeechLine* p_suffix, MechS32 p_priority)
{
	SpeechEntry* entry;
	MechS32 i;
	SpeechEntry* prev;
	SpeechEntry* cur;

	entry = NULL;
	if (g_speechLocked) {
		return FALSE;
	}

	if (!p_line) {
		if (!p_suffix) {
			return FALSE;
		}
		else {
			p_line = p_suffix;
			p_suffix = NULL;
		}
	}

	if (p_line->m_id < 0 || p_line->m_data) {
		p_line->m_id = 0;
	}

	if (p_line->m_text && !*p_line->m_text) {
		p_line->m_text = NULL;
	}

	if (!p_line->m_id && !p_line->m_text && !p_line->m_data) {
		p_line = p_suffix;
	}

	if (!p_line) {
		return FALSE;
	}

	if (p_suffix) {
		if (p_suffix->m_id < 0) {
			p_suffix->m_id = 0;
		}

		if (p_suffix->m_data || !p_suffix->m_id) {
			p_suffix = NULL;
		}
		else {
			if (p_suffix->m_text && !*p_suffix->m_text) {
				p_suffix->m_text = NULL;
			}

			if (!p_suffix->m_id && !p_suffix->m_text) {
				p_suffix = NULL;
			}
		}
	}

	for (i = 0; i < 8 && !entry; i++) {
		if (!g_speechEntries[i].m_used) {
			entry = &g_speechEntries[i];
		}
	}

	if (entry) {
		memset(entry, 0, sizeof(SpeechEntry));
		entry->m_used = 1;
		entry->m_id = p_line->m_id;
		entry->m_data = p_line->m_data;
		if (p_suffix) {
			entry->m_suffixId = p_suffix->m_id;
			entry->m_suffixData = p_suffix->m_data;
		}

		if (p_line->m_text) {
			strncat(entry->m_text, p_line->m_text, sizeof(entry->m_text));
		}

		if (p_suffix && p_suffix->m_text) {
			strncat(entry->m_text, " ", sizeof(entry->m_text) - strlen(entry->m_text));
			strncat(entry->m_text, p_suffix->m_text, sizeof(entry->m_text) - strlen(entry->m_text));
		}

		if (entry->m_text[0]) {
			strncat(entry->m_text, ".", sizeof(entry->m_text) - strlen(entry->m_text));
		}

		if (p_priority == -1 && entry->m_id) {
			entry->m_priority = g_soundInfo[entry->m_id].m_priority;
		}
		else {
			entry->m_priority = p_priority;
		}

		if (g_speechQueue) {
			prev = g_speechQueue;
			cur = prev->m_next;
			i = 0;
			while (cur) {
				if (cur->m_data) {
					if (cur->m_data == entry->m_data) {
						return FALSE;
					}
				}
				else {
					if (cur->m_id && cur->m_id == entry->m_id &&
						(!cur->m_suffixId || entry->m_suffixId == cur->m_suffixId) &&
						(g_soundInfo[entry->m_id].m_maxCount == -1 || ++i >= g_soundInfo[entry->m_id].m_maxCount)) {
						return FALSE;
					}

					if (cur->m_priority < entry->m_priority) {
						prev->m_next = entry;
						entry->m_next = cur;
						break;
					}
				}

				prev = cur;
				cur = cur->m_next;
			}

			prev->m_next = entry;
		}
		else if (StartSpeech(entry)) {
			g_speechQueue = entry;
		}
		else {
			entry->m_used = 0;
			entry = NULL;
		}
	}

	return entry != NULL;
}

// Moves to the next queued message once the current one has finished.
// FUNCTION: MW2 0x10059827
// FUNCTION: MW2GBL 0x10041fb9
// FUNCTION: MW2MATROX 0x1008b9a7
// FUNCTION: MW23DFX 0x100313d0
// FUNCTION: MW2ATI 0x100301c0
// FUNCTION: MW2S3 0x10037380
// FUNCTION: MW2POWERVR 0x1002ac40
void AdvanceSpeechQueue(void)
{
	MechS32 done;
	MechS32 started;

	if (g_speechQueue) {
		done = 0;
		if (g_speechQueue->m_deadline) {
			if (g_speechQueue->m_deadline > g_currentClock) {
				done = 1;
			}
		}
		else if (!g_speechSample) {
			done = 1;
		}

		if (done) {
			started = 0;
			do {
				g_speechQueue->m_used = 0;
				g_speechQueue = g_speechQueue->m_next;
				if (g_speechQueue && StartSpeech(g_speechQueue)) {
					started = 1;
				}
			} while (g_speechQueue && !started);
		}
	}
}

// Plays a queued message: the line and its suffix as sounds, or its text.
// FUNCTION: MW2 0x100598f6
// FUNCTION: MW2GBL 0x10042088
// FUNCTION: MW2MATROX 0x1008ba76
// FUNCTION: MW23DFX 0x10031440
// FUNCTION: MW2ATI 0x10030230
// FUNCTION: MW2S3 0x100373f0
// FUNCTION: MW2POWERVR 0x1002acb0
MechS32 StartSpeech(SpeechEntry* p_entry)
{
	MechS32 id;
	MechS32 suffix;
	MechS32 slot;

	slot = -1;
	g_speechSample = NULL;
	if (!g_audioEngine || !(g_soundConfig.m_simFlags & 2)) {
		if (p_entry->m_text && p_entry->m_text[0]) {
			ShowInGameMessage(p_entry->m_text, 0, 0x712, 0x32);
			p_entry->m_deadline = g_currentClock + 0x712;
			return TRUE;
		}

		return FALSE;
	}

	id = p_entry->m_id;
	suffix = p_entry->m_suffixId;
	p_entry->m_deadline = 0;
	if (!id && suffix) {
		id = suffix;
		suffix = 0;
	}

	if (!id) {
		if (p_entry->m_data) {
			id = -1;
		}
		else {
			return FALSE;
		}
	}

	if (suffix) {
		if (LoadCachedResource(0, suffix, g_resTagSnds, 0)) {
			if (!LoadCachedResource(0, id, g_resTagSnds, 0)) {
				UnlockCachedResource(suffix, g_resTagSnds);
			}
			else {
				slot = PlaySample(
					0,
					0,
					id,
					NULL,
					100,
					g_soundConfig.m_voiceVolume,
					0x40,
					11025,
					(MechS32*) &g_speechSample,
					0x150
				);
				if (slot >= 0) {
					g_speechSample = g_audioEngine->m_samples[slot];
					AIL_set_sample_user_data(g_speechSample, 1, id);
					AIL_set_sample_user_data(g_speechSample, 2, slot);
					AIL_set_sample_user_data(g_speechSample, 3, suffix);
				}
			}
		}
	}
	else if (id) {
		slot = PlaySample(
			0,
			0,
			id,
			p_entry->m_data,
			100,
			g_soundConfig.m_voiceVolume,
			0x40,
			11025,
			(MechS32*) &g_speechSample,
			0x150
		);
		if (slot >= 0) {
			g_speechSample = g_audioEngine->m_samples[slot];
		}
	}
	else if (p_entry->m_text && p_entry->m_text[0]) {
		ShowInGameMessage(p_entry->m_text, 0, 0x712, 0x32);
	}

	return g_speechSample != NULL;
}

// FUNCTION: MW2 0x10059b7b
// FUNCTION: MW2GBL 0x1004230d
// FUNCTION: MW2MATROX 0x1008bcfb
// FUNCTION: MW23DFX 0x100315f0
// FUNCTION: MW2ATI 0x100303e0
// FUNCTION: MW2S3 0x100375a0
// FUNCTION: MW2POWERVR 0x1002ae60
void PlayVoiceVolumeTest(void)
{
	if (g_soundConfig.m_simFlags & 2) {
		PlaySample(0, 0, 0x36, NULL, 100, g_soundConfig.m_voiceVolume, 0x40, 11025, (MechS32*) -1, 0x250);
	}
	else {
		PlaySample(0, 0, 0x54, NULL, 100, 0x10000, 0x40, 11025, (MechS32*) -1, 0x250);
	}
}

// Stops the queue taking messages and drops the queued ones; p_keep keeps sound data and
// messages of priority 0x50 and above.
// FUNCTION: MW2 0x10059be3
// FUNCTION: MW2GBL 0x10042375
// FUNCTION: MW2MATROX 0x1008bd63
// FUNCTION: MW23DFX 0x10031640
// FUNCTION: MW2ATI 0x10030430
// FUNCTION: MW2S3 0x100375f0
// FUNCTION: MW2POWERVR 0x1002aeb0
void FlushSpeechQueue(MechS32 p_keep)
{
	SpeechEntry* prev;
	MechS32 remove;
	SpeechEntry* cur;

	if (g_speechLocked) {
		return;
	}

	g_speechLocked = 1;
	if (!g_speechQueue) {
		return;
	}

	if (g_speechQueue->m_id > 0 && g_speechQueue->m_priority < 0x50) {
		if (g_speechSample) {
			AIL_end_sample(g_speechSample);
		}

		g_speechQueue = FreeSpeechEntry(g_speechQueue);
		if (!g_speechQueue) {
			return;
		}
	}

	prev = g_speechQueue;
	cur = g_speechQueue->m_next;
	while (cur) {
		remove = 1;
		if (p_keep && (cur->m_data || (cur->m_id > 0 && cur->m_priority >= 0x50))) {
			remove = 0;
		}

		if (remove) {
			prev->m_next = FreeSpeechEntry(cur);
			cur = prev->m_next;
		}
		else {
			prev = cur;
			cur = cur->m_next;
		}
	}
}

// Frees an entry's sound data. Returns the next entry.
// FUNCTION: MW2 0x10059d15
// FUNCTION: MW2GBL 0x100424a7
// FUNCTION: MW2MATROX 0x1008be95
// FUNCTION: MW23DFX 0x10031700
// FUNCTION: MW2ATI 0x100304f0
// FUNCTION: MW2S3 0x100376b0
// FUNCTION: MW2POWERVR 0x1002af70
SpeechEntry* FreeSpeechEntry(SpeechEntry* p_entry)
{
	SpeechEntry* next;

	next = p_entry->m_next;
	p_entry->m_used = 0;
	p_entry->m_next = NULL;
	if (p_entry->m_data) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_entry->m_data);
	}

	if (p_entry->m_suffixData) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, p_entry->m_suffixData);
	}

	return next;
}

// Drops the messages after the current one and stops it.
// FUNCTION: MW2 0x10059d8b
// FUNCTION: MW2GBL 0x1004251d
// FUNCTION: MW2MATROX 0x1008bf0b
// FUNCTION: MW23DFX 0x10031760
// FUNCTION: MW2ATI 0x10030550
// FUNCTION: MW2S3 0x10037700
// FUNCTION: MW2POWERVR 0x1002afd0
void StopSpeech(void)
{
	SpeechEntry* entry;

	entry = g_speechQueue;
	if (entry) {
		entry = entry->m_next;
		while (entry) {
			entry = FreeSpeechEntry(entry);
		}
	}

	if (g_speechSample) {
		AIL_stop_sample(g_speechSample);
	}
}

// FUNCTION: MW2 0x10059deb
// FUNCTION: MW2GBL 0x1004257d
// FUNCTION: MW2MATROX 0x1008bf6b
// FUNCTION: MW23DFX 0x10031790
// FUNCTION: MW2ATI 0x10030580
// FUNCTION: MW2S3 0x10037730
// FUNCTION: MW2POWERVR 0x1002b000
void ResumeSpeech(void)
{
	MechS32 slot;
	MechS32 volume;

	if (g_speechSample) {
		volume = FixedMul16(g_soundConfig.m_voiceVolume, 100);
		slot = AIL_sample_user_data(g_speechSample, 2);
		if (slot >= 0) {
			g_audioEngine->m_volumes[slot] = volume;
		}

		AIL_set_sample_volume(g_speechSample, volume);
		AIL_resume_sample(g_speechSample);
	}
}

// A lancemate's report: p_message of g_lancemateSpeech, addressed by p_slot (-1: none).
// FUNCTION: MW2 0x10059e63
// FUNCTION: MW2GBL 0x100425f5
// FUNCTION: MW2MATROX 0x1008bfe3
// FUNCTION: MW23DFX 0x10031810
// FUNCTION: MW2ATI 0x10030600
// FUNCTION: MW2S3 0x100377b0
// FUNCTION: MW2POWERVR 0x1002b080
void SayLancemateReport(MechS32 p_message, MechS32 p_slot)
{
	SpeechLine* slot;
	SpeechLine* message;

	if (p_message == -1 || p_message >= 11) {
		return;
	}

	if (p_slot != -1) {
		slot = &g_slotSpeech[p_slot];
	}
	else {
		slot = NULL;
	}

	message = &g_lancemateSpeech[p_message];
	QueueSpeech(slot, message, -1);
}

// FUNCTION: MW2 0x10059ed2
// FUNCTION: MW2GBL 0x10042664
// FUNCTION: MW2MATROX 0x1008c052
// FUNCTION: MW23DFX 0x10031850
// FUNCTION: MW2ATI 0x10030640
// FUNCTION: MW2S3 0x100377f0
// FUNCTION: MW2POWERVR 0x1002b0c0
void SayFormation(MechS32 p_formation)
{
	if (p_formation >= 0 && p_formation < 6) {
		QueueSpeech(g_formationSpeech, &g_formationSpeech[p_formation + 1], -1);
	}
}

// FUNCTION: MW2 0x10059f0f
// FUNCTION: MW2GBL 0x100426a1
// FUNCTION: MW2MATROX 0x1008c08f
// FUNCTION: MW23DFX 0x10031870
// FUNCTION: MW2ATI 0x10030660
// FUNCTION: MW2S3 0x10037810
// FUNCTION: MW2POWERVR 0x1002b0e0
void PlayCockpitSound(MechS32 p_message, MechS32 p_engage)
{
	SpeechLine* suffix;

	if (!p_engage) {
		return;
	}

	if (p_engage == -1) {
		suffix = NULL;
	}
	else {
		suffix = &g_engageSpeech[p_engage];
	}

	QueueSpeech(&g_cockpitSpeech[p_message], suffix, -1);
}

// "Critical hit : " and the part of g_damageSpeech.
// FUNCTION: MW2 0x10059f6e
// FUNCTION: MW2GBL 0x10042700
// FUNCTION: MW2MATROX 0x1008c0ee
// FUNCTION: MW23DFX 0x100318a0
// FUNCTION: MW2ATI 0x10030690
// FUNCTION: MW2S3 0x10037840
// FUNCTION: MW2POWERVR 0x1002b110
void SayCriticalHit(MechS32 p_part)
{
	QueueSpeech(&g_cockpitSpeech[11], &g_damageSpeech[p_part], -1);
}

// FUNCTION: MW2 0x10059f9c
// FUNCTION: MW2GBL 0x1004272e
// FUNCTION: MW2MATROX 0x1008c11c
// FUNCTION: MW23DFX 0x100318c0
// FUNCTION: MW2ATI 0x100306b0
// FUNCTION: MW2S3 0x10037860
// FUNCTION: MW2POWERVR 0x1002b130
MechS32 QueueSpeechLine(SpeechLine* p_line)
{
	return QueueSpeech(p_line, NULL, 0x50);
}
