#ifndef GRANITEMAST0X18_H
#define GRANITEMAST0X18_H

#include "decomp.h"
#include "types.h"

// SIZE 0x18
// A mech the bay can load. The table ends with a zeroed entry; only the first 15 are offered.
struct GraniteMast0x18 {
	MechChar* m_unk0x00; // 0x00 — code of the mech's video, "awomp%s"
	MechChar* m_unk0x04; // 0x04 — prefix of its variant files
	MechChar* m_unk0x08; // 0x08
	MechChar* m_unk0x0c; // 0x0c — name
	MechS32 m_unk0x10;   // 0x10 — tonnage
	MechS32 m_unk0x14;   // 0x14 — database item of the name sample, -1 for none
};

#endif // GRANITEMAST0X18_H
