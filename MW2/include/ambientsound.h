#ifndef AMBIENTSOUND_H
#define AMBIENTSOUND_H

#include "decomp.h"
#include "types.h"

struct AmberWillow0x7c;

// A sound that loops on an object while the eyepoint is in range (FUN_1007ed1e).
typedef struct AmbientSound {
	MechS32 m_range;               // 0x00
	MechS32 m_slot;                // 0x04 — the sample slot, 8-15, or -1
	void* m_data;                  // 0x08 — the loaded resource
	undefined4 m_unk0x0c;          // 0x0c
	struct AmberWillow0x7c* m_obj; // 0x10
	undefined4 m_unk0x14;          // 0x14
	MechS32 m_skip;                // 0x18 — skips one update
	MechS16 m_id;                  // 0x1c — the sound resource
} AmbientSound;

#endif // AMBIENTSOUND_H
