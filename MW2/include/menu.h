#ifndef MENU_H
#define MENU_H

#include "decomp.h"
#include "point.h"
#include "rendertarget.h"
#include "types.h"

// An in-mission menu's definition, one of the eleven in g_menuDefinitions.
typedef struct MenuDefinition {
	RenderTarget* m_target;                  // 0x00 — where the menu draws
	MechU32 m_flags;                         // 0x04 — 1: takes navigation keys, 0x20: clears its target
	undefined4* m_pageStack;                 // 0x08 — the open pages, up to 8
	MechS32 m_pageDepth;                     // 0x0c
	MechS32 m_backgroundId;                  // 0x10 — a SHP resource, -1: none
	void* m_background;                      // 0x14
	RenderTarget* m_backgroundTarget;        // 0x18
	MechS32 m_unk0x1c;                       // 0x1c — a SHP resource, -1: none
	void* m_unk0x20;                         // 0x20
	undefined4 m_unk0x24[(0x2c - 0x24) / 4]; // 0x24
	MechS32 m_fontId;                        // 0x2c — a FONT resource
	void* m_font;                            // 0x30
	undefined4 m_unk0x34[(0x3c - 0x34) / 4]; // 0x34
	MechS32 m_unk0x3c;                       // 0x3c — lines, for the line spacing
	Point m_unk0x40;                         // 0x40 — the text origin, in pixels
	Point m_unk0x48;                         // 0x48 — m_y: half the line spacing
	Point m_unk0x50;                         // 0x50 — m_y: the line spacing
	Point m_unk0x58;                         // 0x58 — m_y: the line spacing
	Point m_unk0x60;                         // 0x60 — m_y: the line spacing
	undefined4 m_rootPage;                   // 0x68
} MenuDefinition;

// A registered menu: RegisterMenu adds one per menu ID to g_menuSlots.
// SIZE 0x18
typedef struct MenuSlot {
	MechS32 m_id;                 // 0x00 — the index in g_menuDefinitions
	MechS32 m_state;              // 0x04 — 1: open
	MechS32 m_requested;          // 0x08 — the state UpdateMenus moves it to
	MenuDefinition* m_definition; // 0x0c — while open
	undefined4 m_unk0x10;         // 0x10
	struct MenuSlot* m_next;      // 0x14
} MenuSlot;

// The functions and globals of menu.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_menuKey;

	MechS32 RegisterMenu(MechS32 p_id);
	void FreeMenus(void);
	void FirstMenu(void);
	void UpdateMenuKey(void);
	MenuDefinition* GetOpenMenu(void);
	void UpdateMenus(void);
	MechS32 GetMenuSlotState(MechS32 p_id);
	void RequestMenuClose(MechS32 p_id);

#ifdef __cplusplus
}
#endif

#endif // MENU_H
