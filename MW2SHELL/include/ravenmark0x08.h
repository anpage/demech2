#ifndef RAVENMARK0X08_H
#define RAVENMARK0X08_H

#include "decomp.h"
#include "types.h"

// SIZE 0x08
// A star formation: the positions of its three mechs and its name.
struct RavenMark0x08 {
	MechS32* m_unk0x00;  // 0x00 — three x, y pairs
	MechChar* m_unk0x04; // 0x04 — name
};

#endif // RAVENMARK0X08_H
