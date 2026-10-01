#ifndef UNK10065F50_H
#define UNK10065F50_H

#include "decomp.h"
#include "menu.h"
#include "menuchoices.h"
#include "menucontrol.h"
#include "menupage.h"
#include "types.h"

// The functions and globals of unk10065f50.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_unk0x100acaf0[8];

	MechS32 FUN_10065f50(MenuDefinition* p_menu, MenuPage* p_page);
	MechS32 FUN_100660c2(MenuDefinition* p_menu, MenuPage* p_page);
	MechS32 FUN_100661ef(MechS32 p_index);
	MechS32 FUN_10066223(MechS32 p_arg);
	void FUN_10066241(MechS32 p_formation, MechS32 p_value);
	MechS32 FUN_10066272(MechS32 p_index);
	void FUN_100662df(MenuPage* p_page, MenuControl* p_control);
	void FUN_10066314(MenuPage* p_page, MenuControl* p_control);
	void FUN_10066369(MechS32 p_index, MechS32 p_value);
	void FUN_100663a4(MechS32 p_index, MechS32 p_value);
	void FUN_100663df(MechS32 p_index, MechS32 p_value);
	void FUN_1006641a(MechS32 p_index, MechS32 p_value);
	void FUN_10066455(MechS32 p_index, MechS32 p_value);
	void FUN_10066490(MechS32 p_index, MechS32 p_value);
	MechChar* FUN_100664cb(
		MenuDefinition* p_menu,
		MenuControl* p_control,
		MechS32 p_index,
		Point p_pos,
		MenuPage* p_page
	);

#ifdef __cplusplus
}
#endif

#endif // UNK10065F50_H
