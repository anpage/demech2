#ifndef AUDIOSUBSYSTEM_H
#define AUDIOSUBSYSTEM_H

#include "decomp.h"
#include "mss.h"
#include "types.h"

class MidiSequence;

#pragma pack(1)
// SIZE 0x15
class AudioSubsystem {
public:
	AudioSubsystem();
	~AudioSubsystem();

	MechS32 GetMidiVolume();

	friend class MidiSequence;

private:
	MechU8 m_milesStarted;               // 0x00
	HMDIDRIVER m_midiDriver;             // 0x01
	undefined4 m_unk0x05;                // 0x05
	undefined4 m_unk0x09;                // 0x09
	MidiSequence* m_currentMidiSequence; // 0x0d
	undefined4 m_unk0x11;                // 0x11
};
#pragma pack()

#endif // AUDIOSUBSYSTEM_H
