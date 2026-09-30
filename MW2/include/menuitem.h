#ifndef MENUITEM_H
#define MENUITEM_H

#include "decomp.h"
#include "types.h"

struct MenuItem;

// Returns the text a menu item shows (FUN_100664cb: an AI player's goal).
typedef MechChar* (*MenuItemTextFn)(undefined4 p_unk0x00, struct MenuItem* p_item);

// An item of a menu page.
// SIZE 0x14
typedef struct MenuItem {
	undefined4 m_unk0x00;      // 0x00 — set to 1 for an AI player that FUN_10065f50 marks
	undefined4 m_unk0x04;      // 0x04
	MenuItemTextFn* m_unk0x08; // 0x08
	MechU32 m_unk0x0c;         // 0x0c — an AI slot (FUN_10054ccc)
	undefined4 m_unk0x10;      // 0x10
} MenuItem;

#endif // MENUITEM_H
