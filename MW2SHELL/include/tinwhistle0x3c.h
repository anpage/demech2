#ifndef TINWHISTLE0X3C_H
#define TINWHISTLE0X3C_H

#include "decomp.h"
#include "types.h"

// SIZE 0x3c
// One pilot career record; the roster file MW2REG.CFG holds 20 of them.
struct TinWhistle0x3c {
	undefined4 m_unk0x00;     // 0x00
	undefined4 m_unk0x04;     // 0x04
	undefined4 m_unk0x08;     // 0x08
	undefined4 m_unk0x0c;     // 0x0c
	undefined4 m_unk0x10;     // 0x10
	undefined4 m_unk0x14;     // 0x14
	undefined4 m_unk0x18;     // 0x18
	undefined4 m_unk0x1c;     // 0x1c
	undefined4 m_unk0x20;     // 0x20
	undefined4 m_unk0x24;     // 0x24
	MechChar m_unk0x28[0x10]; // 0x28
	undefined4 m_unk0x38;     // 0x38
};

#endif // TINWHISTLE0X3C_H
