#ifndef SAGELARK_H
#define SAGELARK_H

#include "decomp.h"
#include "point.h"
#include "types.h"

// A text readout of the cockpit (g_unk0x100aabd4): a font, a label and the text formatted after
// it, at a place in 16.16 fractions of the screen.
// SIZE 0x1c
typedef struct SageLark0x1c {
	MechS32 m_unk0x00; // 0x00 — its font, from g_artResolution
	MechS32 m_unk0x04; // 0x04
	MechS32 m_unk0x08; // 0x08 — the value it last formatted
	MechChar* m_label; // 0x0c
	MechChar* m_text;  // 0x10
	Point m_position;  // 0x14
} SageLark0x1c;

#endif // SAGELARK_H
