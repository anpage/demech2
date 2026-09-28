#ifndef OAKENTUNE0X10_H
#define OAKENTUNE0X10_H

#include "decomp.h"
#include "types.h"

class AudioSubsystem;

// Per-video sound object held by the FMV slot table (CopperFinch0x4c::m_unk0x04).
// The matched streaming methods establish a 0x14-byte floor, not the full size.
class OakenTune0x10 {
public:
	OakenTune0x10(AudioSubsystem* p_subsystem, MechS32 p_stereo, MechS32 p_wide, MechS32 p_size);
	~OakenTune0x10();
	undefined4 FUN_1003dad5();
	void* FUN_1003db31();
	void FUN_1003db95(void* p_buffer, MechU32 p_size);

private:
	AudioSubsystem* m_unk0x00; // 0x00
	undefined4 m_unk0x04;      // 0x04 — Miles sample handle
	void* m_unk0x08;           // 0x08 — heap buffer
	void* m_unk0x0c;           // 0x0c — heap buffer
	MechS32 m_unk0x10;         // 0x10 — ready buffer index, -1 until queried
};

#endif // OAKENTUNE0X10_H
