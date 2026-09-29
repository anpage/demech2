#include "menu.h"

#include "decomp.h"
#include "input.h"
#include "simmain.h"
#include "types.h"

#include <stddef.h>

// The key the open menu acts on this frame (0: none).
// GLOBAL: MW2 0x10109c84
MechS32 g_menuKey;

// STUB: MW2 0x1003c3e0
void RegisterMenu(MechS32 p_unk0x00)
{
	STUB(0x1003c3e0);
}

// STUB: MW2 0x1003c53c
void FreeMenus(void)
{
	STUB(0x1003c53c);
}

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

// STUB: MW2 0x1003ce91
void UpdateMenus(void)
{
	STUB(0x1003ce91);
}

// STUB: MW2 0x1003da0d
MenuDefinition* GetOpenMenu(void)
{
	STUB(0x1003da0d);
	return NULL;
}

// STUB: MW2 0x1003da65
undefined4 FUN_1003da65(undefined4 p_unk0x00)
{
	STUB(0x1003da65);
	return 0;
}
