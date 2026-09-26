#ifndef COPPERFINCH0X4C_H
#define COPPERFINCH0X4C_H

#include "decomp.h"
#include "oakentune0x10.h"
#include "smackw32.h"
#include "types.h"

// SIZE 0x4c
// One FMV playback slot; the shell keeps 32 of them at 0x100641a8. Flags at 0x1c: 0x80 places the
// video by its bottom center, 0x1000 disables the menu for this video, 0x80000000 marks the slot in use.
struct CopperFinch0x4c {
	Smack* m_unk0x00;                 // 0x00 — Smacker handle
	OakenTune0x10* m_unk0x04;         // 0x04 — per-video sound object
	undefined4 m_unk0x08;             // 0x08
	undefined4 m_unk0x0c;             // 0x0c
	undefined4 m_unk0x10;             // 0x10
	void* m_unk0x14;                  // 0x14 — Miles-locked buffer
	void* m_unk0x18;                  // 0x18 — Miles-locked buffer
	MechS32 m_unk0x1c;                // 0x1c — flags, see above
	MechS32 m_left;                   // 0x20
	MechS32 m_top;                    // 0x24
	MechS32 m_width;                  // 0x28
	MechS32 m_height;                 // 0x2c
	undefined4 m_unk0x30;             // 0x30
	undefined4 m_unk0x34;             // 0x34
	MechS32 m_unk0x38;                // 0x38
	MechS32 m_unk0x3c;                // 0x3c — current frame
	MechS32 m_unk0x40;                // 0x40 — frame count
	undefined m_unk0x44[0x4c - 0x44]; // 0x44
};

#endif // COPPERFINCH0X4C_H
