#ifndef MENUPAGE_H
#define MENUPAGE_H

#include "decomp.h"
#include "menuitem.h"
#include "types.h"

// A page of a menu: its items, some of which the page's callbacks copy from templates at the
// end of the table.
typedef struct MenuPage {
	undefined4 m_unk0x00[0x08 / 4]; // 0x00
	MechU32 m_unk0x08;              // 0x08 — an AI slot (FUN_10054ccc)
	MechS32 m_itemCount;            // 0x0c
	MechS32 m_selected;             // 0x10 — the highlighted item
	undefined4 m_unk0x14;           // 0x14
	MenuItem m_items[8];            // 0x18
} MenuPage;

#endif // MENUPAGE_H
