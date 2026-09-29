#ifndef MENU_H
#define MENU_H

#include "decomp.h"
#include "types.h"

// An in-mission menu's definition (only its flags are known).
typedef struct MenuDefinition {
	undefined4 m_unk0x00; // 0x00
	MechU32 m_flags;      // 0x04 — 1: takes navigation keys
} MenuDefinition;

// The functions and globals of menu.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_menuKey;

	void RegisterMenu(MechS32 p_unk0x00);
	void FreeMenus(void);
	void FirstMenu(void);
	void UpdateMenuKey(void);
	MenuDefinition* GetOpenMenu(void);
	void UpdateMenus(void);
	undefined4 FUN_1003da65(undefined4 p_unk0x00);

#ifdef __cplusplus
}
#endif

#endif // MENU_H
