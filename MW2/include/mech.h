#ifndef MECH_H
#define MECH_H

#include "decomp.h"
#include "types.h"

// A player's mech. Only the members matched code reaches are laid out.
typedef struct Mech {
	undefined m_unk0x00[0x9c]; // 0x00
	MechS32 m_unk0x9c;         // 0x9c
} Mech;

#endif // MECH_H
