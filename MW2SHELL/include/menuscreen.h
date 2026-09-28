#ifndef MENUSCREEN_H
#define MENUSCREEN_H

#include "decomp.h"
#include "mainmenubutton.h"
#include "types.h"

// SIZE 0x10
// A menu screen of one campaign: its buttons and its background picture.
struct MenuScreen {
	MainMenuButton* m_buttons; // 0x00
	MechS32 m_count;           // 0x04
	MechS32 m_picture;         // 0x08 — for VideoDriver::LoadBackground
	MechS32 m_unk0x0c;         // 0x0c
};

#endif // MENUSCREEN_H
