#ifndef STARMECH_H
#define STARMECH_H

#include "decomp.h"
#include "types.h"

// SIZE 0x24
// One mech of a star: a negative m_unk0x00 leaves the slot empty.
struct StarMech {
	MechS32 m_unk0x00;    // 0x00
	char m_unk0x04[0x10]; // 0x04 — variant file name
	char m_unk0x14[0x10]; // 0x14 — pilot name
};

#endif // STARMECH_H
