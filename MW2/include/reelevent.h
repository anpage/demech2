#ifndef REELEVENT_H
#define REELEVENT_H

#include "types.h"

// A frame's events in an animation (QuartzReel0x14::m_events): flags that mark the frames an
// animation jumps between (see FUN_10046750), and the values they compare with the player's
// animation state.
// SIZE 0x08
typedef struct ReelEvent {
	MechU32 m_flags;    // 0x00 — 1: a jump target (m_values[0]), 2: a loop start
	MechS8 m_values[4]; // 0x04
} ReelEvent;

#endif // REELEVENT_H
