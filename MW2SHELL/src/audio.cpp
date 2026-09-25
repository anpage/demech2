#include "audiosample.h"
#include "audiosubsystem.h"
#include "midisequence.h"

#include <windows.h>

DECOMP_SIZE_ASSERT(AudioSubsystem, 0x15)

extern HANDLE g_hPrimaryHeap;
extern MechS32 g_fAudio;
extern MechS32 g_fDigitalAudio;

// GLOBAL: MW2SHELL 0x10071684
MechS32 g_midiVolume = 0x10000;

// FUNCTION: MW2SHELL 0x1003ceb0
AudioSubsystem::AudioSubsystem()
{
	m_currentMidiSequence = NULL;
	m_midiDriver = NULL;
	m_unk0x05 = 0;

	if (g_fAudio || g_fDigitalAudio) {
		AIL_startup();
		m_milesStarted = 1;
	}
	else {
		m_milesStarted = 0;
	}

	if (g_fAudio) {
		if (AIL_midiOutOpen(&m_midiDriver, NULL, -1) && AIL_midiOutOpen(&m_midiDriver, NULL, 0)) {
			m_midiDriver = NULL;
		}
	}
}

// FUNCTION: MW2SHELL 0x1003cf5d
AudioSubsystem::~AudioSubsystem()
{
	if (m_milesStarted == 1) {
		AIL_shutdown();
	}
}

// FUNCTION: MW2SHELL 0x1003d12a
MechS32 AudioSubsystem::GetMidiVolume()
{
	return g_midiVolume;
}

// FUNCTION: MW2SHELL 0x1003d198
MidiSequence::MidiSequence(AudioSubsystem* p_subsystem, void* p_data, undefined4 p_size)
{
	MechS32 sequenceNum;

	m_subsystem = p_subsystem;
	m_data = p_data;
	m_size = p_size;
	m_sequence = NULL;

	if (m_subsystem->m_midiDriver == NULL) {
		return;
	}

	m_sequence = AIL_allocate_sequence_handle(m_subsystem->m_midiDriver);
	if (m_sequence == NULL) {
		return;
	}

	sequenceNum = 0;
	if (AIL_init_sequence(m_sequence, m_data, sequenceNum) <= 0) {
		m_sequence = NULL;
		return;
	}

	SetLoopCount(0);
	SetVolume(50);
}

// FUNCTION: MW2SHELL 0x1003d25c
void MidiSequence::SetLoopCount(MechS32 p_loopCount)
{
	m_loopCount = p_loopCount;
	if (m_sequence) {
		AIL_set_sequence_loop_count(m_sequence, m_loopCount);
	}
}

// FUNCTION: MW2SHELL 0x1003d29e
MidiSequence::~MidiSequence()
{
	if (m_sequence) {
		Stop();
		m_subsystem->m_currentMidiSequence = NULL;
		AIL_release_sequence_handle(m_sequence);
		HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_data);
	}
}

// FUNCTION: MW2SHELL 0x1003d2f7
void MidiSequence::Start()
{
	if (m_sequence) {
		AIL_start_sequence(m_sequence);
		SetLoopCount(m_loopCount);
		m_subsystem->m_currentMidiSequence = this;
	}
}

// FUNCTION: MW2SHELL 0x1003d341
void MidiSequence::Stop()
{
	if (m_sequence) {
		AIL_stop_sequence(m_sequence);
	}
}

// FUNCTION: MW2SHELL 0x1003d371
void MidiSequence::SetVolume(MechS32 p_volume)
{
	m_volume = p_volume;
	if (m_sequence) {
		AIL_set_sequence_volume(m_sequence, (MechS32) (((m_subsystem->GetMidiVolume() * m_volume) >> 16) * 1.27), 0);
	}
}

// FUNCTION: MW2SHELL 0x1003d3f4
void MidiSequence::ApplyCurrentVolume()
{
	SetVolume(m_volume);
}

// FUNCTION: MW2SHELL 0x1003d50f
AudioSample::~AudioSample()
{
	if (m_sample) {
		AIL_end_sample(m_sample);
		AIL_release_sample_handle(m_sample);
		HeapFree(g_hPrimaryHeap, HEAP_NO_SERIALIZE, m_data);
	}
}
