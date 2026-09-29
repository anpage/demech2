#ifndef DRAWMODE_H
#define DRAWMODE_H

#include "decomp.h"
#include "types.h"

// The present half of a draw mode (g_currentDrawMode).
typedef struct DrawMode {
	MechU32 m_index;          // 0x00
	MechS32 m_extensionIndex; // 0x04
	MechS32 m_initialized;    // 0x08
	MechU32 m_profileTime;    // 0x0c
	void* m_begin;            // 0x10
	void* m_end;              // 0x14
	void (*m_blitFlip)(void); // 0x18
} DrawMode;

#endif // DRAWMODE_H
