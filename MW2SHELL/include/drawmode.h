#ifndef DRAWMODE_H
#define DRAWMODE_H

#include "decomp.h"
#include "types.h"

#pragma pack(1)
// SIZE 0x24
struct DrawMode {
	undefined m_unk0x00[0x18 - 0x00]; // 0x00
	void (*m_unk0x18)();              // 0x18
	undefined m_unk0x1c[0x24 - 0x1c]; // 0x1c
};
#pragma pack()

#endif // DRAWMODE_H
