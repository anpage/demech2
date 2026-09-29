#ifndef TWILIGHTGROVE_H
#define TWILIGHTGROVE_H

#include "decomp.h"
#include "types.h"

// The transform the next block opens with (ApplyBlockXform): scale, rotation and
// translation, three values each.
// SIZE 0x24
typedef struct TwilightGrove0x24 {
	MechS32 m_unk0x00; // 0x00
	MechS32 m_unk0x04; // 0x04
	MechS32 m_unk0x08; // 0x08
	MechS32 m_unk0x0c; // 0x0c
	MechS32 m_unk0x10; // 0x10
	MechS32 m_unk0x14; // 0x14
	MechS32 m_unk0x18; // 0x18
	MechS32 m_unk0x1c; // 0x1c
	MechS32 m_unk0x20; // 0x20
} TwilightGrove0x24;

#endif // TWILIGHTGROVE_H
