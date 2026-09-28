#ifndef FMVSLOT_H
#define FMVSLOT_H

#include "decomp.h"
#include "smackw32.h"
#include "types.h"
#include "videosound.h"

// SIZE 0x4c
// One FMV playback slot; the shell keeps 32 of them at 0x100641a8. Flags at 0x1c: 0x80 places the
// video by its bottom center, 0x1000 disables the menu for this video, 0x80000000 marks the slot in use.
struct FmvSlot {
	Smack* m_unk0x00;                 // 0x00 — Smacker handle
	VideoSound* m_unk0x04;            // 0x04 — per-video sound object
	void* m_unk0x08;                  // 0x08 — sound buffer being filled
	MechS32 m_unk0x0c;                // 0x0c — its size
	undefined4 m_unk0x10;             // 0x10 — set while the sound track has data to stream
	void* m_unk0x14;                  // 0x14 — Miles-locked buffer
	void* m_unk0x18;                  // 0x18 — Miles-locked buffer
	MechS32 m_unk0x1c;                // 0x1c — flags, see above
	MechS32 m_left;                   // 0x20
	MechS32 m_top;                    // 0x24
	MechS32 m_width;                  // 0x28
	MechS32 m_height;                 // 0x2c
	MechS32 m_unk0x30;                // 0x30 — m_left when last drawn
	MechS32 m_unk0x34;                // 0x34 — m_top when last drawn
	MechS32 m_unk0x38;                // 0x38 — m_unk0x3c when last drawn
	MechS32 m_unk0x3c;                // 0x3c — current frame
	MechS32 m_unk0x40;                // 0x40 — frame count
	undefined m_unk0x44[0x4c - 0x44]; // 0x44
};

#endif // FMVSLOT_H
