#ifndef SIMHANDOFFSTATE_H
#define SIMHANDOFFSTATE_H

#include "customstar.h"
#include "decomp.h"
#include "types.h"

// SIZE 0x218
// The shell's state across a mission, saved to mw2prm.cfg before the simulator runs and read
// back after it.
struct SimHandoffState {
	undefined4 m_unk0x00;       // 0x00 — message to post on return
	undefined4 m_unk0x04;       // 0x04 — campaign
	undefined4 m_unk0x08;       // 0x08 — pilot chosen
	MechS32 m_unk0x0c;          // 0x0c — TRUE when the player's star was selected
	CustomStar m_unk0x10;       // 0x10 — the player's star
	CustomStar m_unk0x90;       // 0x90 — the enemy's star
	undefined4 m_unk0x110;      // 0x110 — mission, an index into g_unk0x1006a220 (missionui.cpp)
	MechS32 m_unk0x114;         // 0x114 — pilot index, -1 for none
	MechChar m_unk0x118[0x100]; // 0x118 — the simulator's command line
};

#endif // SIMHANDOFFSTATE_H
