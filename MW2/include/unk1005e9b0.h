#ifndef UNK1005E9B0_H
#define UNK1005E9B0_H

#include "menu.h"
#include "menupage.h"
#include "types.h"

// The functions of unk1005e9b0.c.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MenuDefinition g_systemsMenu;
	extern MenuPage* g_systemsMenuPageStack[8];

	MechS32 FUN_1005e9b0(MechS32 p_id);
	void FUN_1005eb10(MechS32 p_id, MechS32 p_value);

#ifdef __cplusplus
}
#endif

#endif // UNK1005E9B0_H
