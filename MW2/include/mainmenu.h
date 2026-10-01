#ifndef MAINMENU_H
#define MAINMENU_H

#include "menu.h"
#include "menuchoices.h"
#include "menupage.h"
#include "types.h"

// The globals of mainmenu.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechChar g_unk0x100a1ad8[];
	extern MechChar g_unk0x100a1af0[];
	extern MenuChoices g_unk0x100a1ba0;
	extern MenuChoices g_unk0x100a1c30;
	extern MechS32 g_unk0x100a1cc0[8];
	extern MenuDefinition g_mainMenu;
	extern MenuPage* g_mainMenuPageStack[8];

#ifdef __cplusplus
}
#endif

#endif // MAINMENU_H
