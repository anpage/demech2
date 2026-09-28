#ifndef SOUNDCONFIG_H
#define SOUNDCONFIG_H

#include "decomp.h"
#include "types.h"

// SIZE 0x3c
// The sound settings, read from and written to MW2SND.CFG as one block. The options screen's
// sliders and rows point at the members.
struct SoundConfig {
	MechS32 m_unk0x00;                   // 0x00
	MechS32 m_effectsVolume;             // 0x04
	MechS32 m_unk0x08;                   // 0x08 — the third volume slider
	MechS32 m_midiVolume;                // 0x0c
	undefined4 m_unk0x10;                // 0x10
	MechS32 m_unk0x14;                   // 0x14
	MechS32 m_unk0x18;                   // 0x18
	MechS32 m_unk0x1c;                   // 0x1c
	MechS32 m_unk0x20;                   // 0x20
	MechS32 m_unk0x24;                   // 0x24
	MechS32 m_displayBrightness;         // 0x28
	MechChar m_videoDriver[0x3c - 0x2c]; // 0x2c — "" or "vesa480.dll"
};

#endif // SOUNDCONFIG_H
