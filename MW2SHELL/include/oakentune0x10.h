#ifndef OAKENTUNE0X10_H
#define OAKENTUNE0X10_H

#include "decomp.h"
#include "types.h"

// SIZE 0x10
// Per-video sound object held by the FMV slot table (CopperFinch0x4c::m_unk0x04). The size is
// a floor from the destructor's field accesses (+0x0c) and is not yet proven by a match, so
// there is no DECOMP_SIZE_ASSERT until one pins it down.
class OakenTune0x10 {
public:
	~OakenTune0x10();

private:
	undefined4 m_unk0x00; // 0x00
	undefined4 m_unk0x04; // 0x04 — Miles sample handle
	void* m_unk0x08;      // 0x08 — heap buffer
	void* m_unk0x0c;      // 0x0c — heap buffer
};

#endif // OAKENTUNE0X10_H
