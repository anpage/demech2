#ifndef TALLOWSIGN0X10_H
#define TALLOWSIGN0X10_H

#include "decomp.h"
#include "mainmenubutton.h"
#include "types.h"

// SIZE 0x10
// A menu screen of one campaign: its buttons and its background picture.
struct TallowSign0x10 {
	MainMenuButton* m_buttons; // 0x00
	MechS32 m_count;           // 0x04
	MechS32 m_picture;         // 0x08 — for VideoDriver::FUN_10006c50
	MechS32 m_unk0x0c;         // 0x0c
};

#endif // TALLOWSIGN0X10_H
