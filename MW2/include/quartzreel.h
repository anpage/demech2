#ifndef QUARTZREEL_H
#define QUARTZREEL_H

#include "types.h"

// An animation of a loaded animation file (FUN_100708f4), in g_unk0x101079e0: its frame data
// lies between m_unk0x0c and the end of the file's animation records, m_unk0x10.
// SIZE 0x14
typedef struct QuartzReel0x14 {
	MechS32 m_unk0x00; // 0x00 — the file was looked up by name (ResourceRef::m_id -1)
	MechS32 m_unk0x04; // 0x04
	MechS32 m_unk0x08; // 0x08
	MechU8* m_unk0x0c; // 0x0c
	MechU8* m_unk0x10; // 0x10
} QuartzReel0x14;

#endif // QUARTZREEL_H
