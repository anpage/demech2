#include "menu.h"

#include "decomp.h"
#include "inputmap.h"
#include "loadres.h"
#include "render.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "simmain.h"
#include "types.h"

#include <stddef.h>
#include <windows.h>

// GLOBAL: MW2 0x10109c78
MechS32 g_openMenuCount;

// GLOBAL: MW2 0x10109c7c
MenuSlot* g_menuSlotsTail;

// GLOBAL: MW2 0x10109c80
MenuSlot* g_menuSlots;

// The key the open menu acts on this frame (0: none).
// GLOBAL: MW2 0x10109c84
MechS32 g_menuKey;

MechS32 ActivateMenu(MenuSlot* p_slot);
void DeactivateMenu(MenuSlot* p_slot);
void RequestMenuClose(MechS32 p_id);
MechS32 DrawAndRunMenu(MenuDefinition* p_menu);
void RunMenuItems(MenuDefinition* p_menu);
MenuSlot* FindMenuSlot(MechS32 p_id);
MechS32 PushMenuPage(MenuDefinition* p_menu, undefined4 p_page);
void ClearMenuPages(MenuDefinition* p_menu);
MechS32 IsMenuPageStackEmpty(MenuDefinition* p_menu);

// Adds a slot for a menu ID at the end of g_menuSlots. Returns whether it could.
// FUNCTION: MW2 0x1003c3e0
MechS32 RegisterMenu(MechS32 p_id)
{
	MechS32 result;
	MenuSlot* slot;

	result = 0;
	slot = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(MenuSlot));
	if (slot == NULL) {
		return result;
	}

	memset(slot, 0, sizeof(MenuSlot));
	slot->m_id = p_id;
	if (g_menuSlots == NULL) {
		g_menuSlots = slot;
	}
	else {
		g_menuSlotsTail->m_next = slot;
	}
	g_menuSlotsTail = slot;
	result = 1;

	return result;
}

// Asks for an in-mission menu to open (4 is the mission menu), if no menu is open already.
// Returns whether it will.
// FUNCTION: MW2 0x1003c46b
MechS32 RequestMenu(MechS32 p_id)
{
	MenuSlot* slot;
	MechS32 result;

	result = 0;
	if (g_openMenuCount == 0) {
		slot = FindMenuSlot(p_id);
		if (slot) {
			slot->m_requested = 1;
			result = 1;
		}
	}

	return result;
}

// Asks for a closed menu to open or an open one to close. Returns whether it asked to open it.
// FUNCTION: MW2 0x1003c4bf
MechS32 ToggleMenu(MechS32 p_id)
{
	MechS32 result;

	result = 0;
	switch (GetMenuSlotState(p_id)) {
	case 1:
		RequestMenuClose(p_id);
		break;
	case 0:
		RequestMenu(p_id);
		result = 1;
		break;
	default:
		break;
	}

	return result;
}

// Stack-slot permutation: slot and freed.
// FUNCTION: MW2 0x1003c53c
void FreeMenus(void)
{
	MenuSlot* slot;
	MenuSlot* freed;

	slot = g_menuSlots;
	while (slot) {
		DeactivateMenu(slot);
		freed = slot;
		slot = slot->m_next;
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, freed);
	}

	g_menuSlots = NULL;
	g_menuSlotsTail = NULL;
}

// STUB: MW2 0x1003c5a2
void FUN_1003c5a2(MenuDefinition* p_menu)
{
	STUB(0x1003c5a2);
}

// Loads a menu's background shapes and font.
// FUNCTION: MW2 0x1003c844
void LoadMenuResources(MenuDefinition* p_menu)
{
	if (p_menu->m_backgroundId != -1) {
		p_menu->m_background =
			FUN_1001a19f(g_unk0x100a8740, p_menu->m_backgroundId + g_unk0x100e9614, g_unk0x100a8680, 0);
	}
	else {
		p_menu->m_background = NULL;
	}

	if (p_menu->m_unk0x1c != -1) {
		p_menu->m_unk0x20 = FUN_1001a19f(g_unk0x100a8740, p_menu->m_unk0x1c + g_unk0x100e9614, g_unk0x100a8680, 0);
	}
	else {
		p_menu->m_unk0x20 = NULL;
	}

	p_menu->m_font = FUN_1001a19f(g_unk0x100a8740, p_menu->m_fontId + g_unk0x100e9614, g_unk0x100a8684, 0);
}

// Opens a menu. Menus with flag 1 take the controls, so this calls DisableGameplayInput;
// DeactivateMenu calls EnableGameplayInput again.
// STUB: MW2 0x1003c902
MechS32 ActivateMenu(MenuSlot* p_slot)
{
	STUB(0x1003c902);
	return 0;
}

// Closes a menu and frees its resources.
// FUNCTION: MW2 0x1003c9d2
void DeactivateMenu(MenuSlot* p_slot)
{
	MenuDefinition* menu;

	menu = p_slot->m_definition;
	if (menu) {
		if (menu->m_background) {
			FUN_1001a163(menu->m_backgroundId + g_unk0x100e9614, g_unk0x100a8680);
			menu->m_background = NULL;
		}

		if (menu->m_unk0x20) {
			FUN_1001a163(menu->m_unk0x1c + g_unk0x100e9614, g_unk0x100a8680);
			menu->m_unk0x20 = NULL;
		}

		if (menu->m_font) {
			FUN_1001a163(menu->m_fontId + g_unk0x100e9614, g_unk0x100a8684);
			menu->m_font = NULL;
		}

		if (menu->m_flags & 1) {
			EnableGameplayInput();
		}
	}

	p_slot->m_definition = NULL;
	g_openMenuCount--;
}

// Asks for a menu to close.
// FUNCTION: MW2 0x1003caab
void RequestMenuClose(MechS32 p_id)
{
	MenuSlot* slot;

	slot = FindMenuSlot(p_id);
	if (slot) {
		slot->m_requested = 0;
	}
}

// First draws the menus' render targets to the main pixel buffer and loads their layout; drops
// the definitions whose pages fail their init callbacks.
// STUB: MW2 0x1003cadc
void FirstMenu(void)
{
	STUB(0x1003cadc);
}

// Works out g_menuKey for the open menu from the key code, or from the menu bindings in menus
// that take navigation keys (repeating while they are held), and takes the key code it uses.
// Stack-slot permutation: menu and flags.
// FUNCTION: MW2 0x1003cc20
void UpdateMenuKey(void)
{
	MenuDefinition* menu;
	MechU32 flags;

	g_menuKey = 0;
	menu = GetOpenMenu();
	if (menu) {
		flags = menu->m_flags;
		if (g_keyCode) {
			if (g_keyCode == 0x1b) {
				g_menuKey = g_keyCode;
			}
			else if (g_keyCode >= '0' && g_keyCode <= '9') {
				g_menuKey = g_keyCode;
			}
			else if (flags & 1) {
				switch (g_keyCode) {
				case 0x09:
				case 0x0d:
				case 0x20:
				case 0xc6:
				case 0xc7:
				case 0xc8:
				case 0xc9:
				case 0x209:
					g_menuKey = g_keyCode;
					break;
				default:
					break;
				}
			}

			if (g_keyCode == g_menuKey) {
				g_keyCode = 0;
			}
		}
		else if (flags & 1) {
			if (g_sinkMenuEnter) {
				g_menuKey = 0x0d;
			}
			else if (g_sinkMenuAbort) {
				g_menuKey = 0x1b;
			}

			if (g_menuKey == 0 && GetTicks(g_menuRepeatTimer) > 90) {
				if (g_sinkMenuItem < -0x2000) {
					g_menuKey = 0xc6;
				}
				else if (g_sinkMenuItem > 0x2000) {
					g_menuKey = 0xc7;
				}
			}

			if (g_menuKey == 0 && GetTicks(g_menuRepeatTimer) > 45) {
				if (g_sinkMenuValue < -0x2000) {
					g_menuKey = 0xc9;
				}
				else if (g_sinkMenuValue > 0x2000) {
					g_menuKey = 0x20;
				}
			}
		}

		if (g_menuKey && (flags & 1)) {
			ResetTicks(g_menuRepeatTimer);
			g_sinkMenuItemReset = 1;
			g_sinkMenuValueReset = 1;
		}
	}
}

// Opens and closes in-mission menus as requested, then draws and runs the open one. Called from
// SimMain after UpdateMenuKey and HandleGameKeys, so a menu closed this frame still counts as open
// (g_openMenuCount) when that frame's game keys run.
// Stack-slot permutation: slot, done and requested. The original compares state against
// requested in the other operand order.
// FUNCTION: MW2 0x1003ce91
void UpdateMenus(void)
{
	MechS32 requested;
	MechS32 state;
	MenuSlot* slot;
	MechS32 done;

	slot = g_menuSlots;
	done = 0;
	while (slot && !done) {
		state = slot->m_state;
		requested = slot->m_requested;
		if (state != requested) {
			if (requested == 1) {
				if (ActivateMenu(slot)) {
					state = 1;
				}
			}
			else if (requested == 0) {
				DeactivateMenu(slot);
				state = 0;
			}
		}

		slot->m_state = state;
		slot->m_requested = requested;
		if (state == 1) {
			if (!(slot->m_definition->m_flags & 2) || !g_unk0x10176ebc) {
				if (!DrawAndRunMenu(slot->m_definition)) {
					slot->m_requested = 0;
				}
			}
			done = 1;
		}

		slot = slot->m_next;
	}
}

// Draws the open in-mission menu and acts on g_menuKey. Returns FALSE when the menu should close.
// Stack-slot permutation: target and backgroundTarget.
// FUNCTION: MW2 0x1003cf96
MechS32 DrawAndRunMenu(MenuDefinition* p_menu)
{
	MechS32 result;
	RenderTarget* target;
	RenderTarget* backgroundTarget;

	result = FALSE;
	target = p_menu->m_target;
	if (target == NULL) {
		return result;
	}

	backgroundTarget = p_menu->m_backgroundTarget;
	if (backgroundTarget == NULL) {
		return result;
	}

	LoadMenuResources(p_menu);
	if (p_menu->m_flags & 0x20) {
		FillRenderTargetRect(target, 0);
	}

	if (p_menu->m_background) {
		DrawShapeFrame(backgroundTarget, p_menu->m_background, 0, 0, 0);
	}

	if (p_menu->m_flags & 4) {
		FUN_100570e9(target, 1);
	}

	RunMenuItems(p_menu);
	if (!IsMenuPageStackEmpty(p_menu)) {
		result = TRUE;
	}
	else {
		result = FALSE;
	}

	return result;
}

// Acts on a menu key for the selected item: Enter depends on the item's type (0 opens a submenu,
// action 3; 2 goes back, action 4; 5 and 6 close, action 5); Escape closes (action 5); Down (0xc7)
// and Tab move the selection +1 through *p_move; Up (0xc6) and Shift+Tab (0x209) move it -1.
// FUNCTION: MW2 0x1003d083
void ApplyMenuKey(MechS32 p_key, MechS32 p_itemType, MechS32* p_action, MechS32* p_move)
{
	switch (p_key) {
	case 0x09:
	case 0xc7:
		*p_move = 1;
		break;
	case 0xc6:
	case 0x209:
		*p_move = -1;
		break;
	case 0x0d:
		switch (p_itemType) {
		case 0:
			*p_action = 3;
			break;
		case 2:
			*p_action = 4;
			break;
		case 5:
		case 6:
			*p_action = 5;
			break;
		default:
			break;
		}
		break;
	case 0x1b:
		*p_action = 5;
		break;
	default:
		break;
	}
}

// STUB: MW2 0x1003d1a7
void RunMenuItems(MenuDefinition* p_menu)
{
	STUB(0x1003d1a7);
}

// Finds the g_menuSlots entry for a menu ID, or NULL.
// FUNCTION: MW2 0x1003d85b
MenuSlot* FindMenuSlot(MechS32 p_id)
{
	MenuSlot* slot;

	slot = g_menuSlots;
	while (slot) {
		if (slot->m_id == p_id) {
			break;
		}
		slot = slot->m_next;
	}

	return slot;
}

// Returns the definition of an open menu, or NULL.
// FUNCTION: MW2 0x1003d8a4
MenuDefinition* FindMenuDefinition(MechS32 p_id)
{
	MenuSlot* slot;
	MenuDefinition* result;

	result = NULL;
	slot = g_menuSlots;
	while (slot) {
		if (slot->m_id == p_id) {
			break;
		}
		slot = slot->m_next;
	}

	if (slot) {
		result = slot->m_definition;
	}

	return result;
}

// FUNCTION: MW2 0x1003d907
undefined4 PopMenuPage(MenuDefinition* p_menu)
{
	undefined4 page;

	page = 0;
	if (p_menu->m_pageDepth > 0) {
		p_menu->m_pageDepth--;
		page = p_menu->m_pageStack[p_menu->m_pageDepth];
	}

	return page;
}

// FUNCTION: MW2 0x1003d949
undefined4 PeekMenuPage(MenuDefinition* p_menu)
{
	undefined4 page;

	page = 0;
	if (p_menu->m_pageDepth > 0) {
		page = p_menu->m_pageStack[p_menu->m_pageDepth - 1];
	}

	return page;
}

// FUNCTION: MW2 0x1003d986
MechS32 PushMenuPage(MenuDefinition* p_menu, undefined4 p_page)
{
	MechS32 result;

	result = 0;
	if (p_menu->m_pageDepth < 8) {
		p_menu->m_pageStack[p_menu->m_pageDepth] = p_page;
		p_menu->m_pageDepth++;
		result = 1;
	}

	return result;
}

// FUNCTION: MW2 0x1003d9cf
void ClearMenuPages(MenuDefinition* p_menu)
{
	p_menu->m_pageDepth = 0;
}

// FUNCTION: MW2 0x1003d9e4
MechS32 IsMenuPageStackEmpty(MenuDefinition* p_menu)
{
	return p_menu->m_pageDepth == 0;
}

// Returns the open in-mission menu's definition, or NULL if none is open.
// FUNCTION: MW2 0x1003da0d
MenuDefinition* GetOpenMenu(void)
{
	MenuSlot* slot;
	MenuDefinition* result;

	slot = g_menuSlots;
	result = NULL;
	while (slot) {
		if (slot->m_state == 1) {
			result = slot->m_definition;
			break;
		}
		slot = slot->m_next;
	}

	return result;
}

// Returns a menu's state (1: open), or 0 if it isn't registered.
// Stack-slot permutation: slot and result.
// FUNCTION: MW2 0x1003da65
MechS32 GetMenuSlotState(MechS32 p_id)
{
	MenuSlot* slot;
	MechS32 result;

	result = 0;
	slot = FindMenuSlot(p_id);
	if (slot) {
		result = slot->m_state;
	}

	return result;
}
