#ifndef MENU_H
#define MENU_H

#include "decomp.h"
#include "types.h"

// The functions and globals of menu.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	void RegisterMenu(MechS32 p_unk0x00);
	void FreeMenus(void);
	void FirstMenu(void);
	void UpdateMenuKey(void);
	void UpdateMenus(void);
	undefined4 FUN_1003da65(undefined4 p_unk0x00);

#ifdef __cplusplus
}
#endif

#endif // MENU_H
