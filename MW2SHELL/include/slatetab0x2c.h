#ifndef SLATETAB0X2C_H
#define SLATETAB0X2C_H

#include "decomp.h"
#include "types.h"

class EmberGlyph0x3e;

// SIZE 0x2c
// One labelled field of a shell screen. Screens keep them in tables ended by m_left == -1; the
// callback at 0x1c draws the field and returns its glyph. Its data at 0x24 depends on the callback:
// a MechS32* or MechChar** to the value shown, a string, a flag, or an index.
struct SlateTab0x2c {
	MechS32 m_left;                              // 0x00
	MechS32 m_top;                               // 0x04 — negative: packed row (bits 4-11) and offset (bits 0-3)
	MechS32 m_width;                             // 0x08
	MechS32 m_height;                            // 0x0c
	undefined4 m_unk0x10;                        // 0x10
	undefined* m_unk0x14;                        // 0x14 — color map for the glyph
	EmberGlyph0x3e* m_glyph;                     // 0x18
	EmberGlyph0x3e* (*m_unk0x1c)(SlateTab0x2c*); // 0x1c
	undefined4 m_unk0x20;                        // 0x20
	void* m_unk0x24;                             // 0x24 — the callback's data, see above
	SlateTab0x2c* m_unk0x28;                     // 0x28 — the table to switch to
};

#endif // SLATETAB0X2C_H
