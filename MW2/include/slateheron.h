#ifndef SLATEHERON_H
#define SLATEHERON_H

#include "decomp.h"
#include "types.h"

// SIZE 0x68
// Rendering settings (g_unk0x100a6cc8) the map view saves and replaces as one block.
typedef struct SlateHeron0x68 {
	undefined4 m_unk0x00[(0x54 - 0x00) / 4]; // 0x00
	void (*m_frameDrawCallback)(void);       // 0x54
	void (*m_unk0x58)();                     // 0x58
	void (*m_unk0x5c)();                     // 0x5c
	undefined4 m_unk0x60[(0x68 - 0x60) / 4]; // 0x60
} SlateHeron0x68;

#endif // SLATEHERON_H
