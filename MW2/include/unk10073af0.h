#ifndef UNK10073AF0_H
#define UNK10073AF0_H

#include "decomp.h"
#include "menu.h"
#include "menucontrol.h"
#include "menupage.h"
#include "point.h"
#include "types.h"

// The functions of unk10073af0.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MenuDefinition g_dorcsMenu;
	extern MenuPage* g_dorcsMenuPageStack[8];

	void FUN_10073af0(MenuDefinition* p_menu, MenuControl* p_control, MechS32 p_index, Point p_pos, MenuPage* p_page);
	void FUN_10073ba6(MenuDefinition* p_menu, MenuControl* p_control, MechS32 p_index, Point p_pos, MenuPage* p_page);
	void* ReadVfxBin(MechChar* p_name);
	void FUN_10073cb5(void);
	void UpdateDorcs(void);
	void ShowDorcs(void);

#ifdef __cplusplus
}
#endif

#endif // UNK10073AF0_H
