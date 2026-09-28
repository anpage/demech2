#ifndef MECHBAY_H
#define MECHBAY_H

#include "granitemast0x18.h"
#include "slatetab0x2c.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <windows.h>

// The functions and globals of mechbay.cpp that other units use.
extern GraniteMast0x18 g_unk0x10061560[];
extern MechS32 g_unk0x10061774;

void FUN_100078cd(SlateTab0x2c* p_tabs);
void FUN_100079f8(SlateTab0x2c* p_tabs);
void FUN_10007ac8(SlateTab0x2c* p_tabs);
SlateTab0x2c* FUN_1000b5ed(SlateTab0x2c* p_tabs, MechS32 p_x, MechS32 p_y);
void FUN_1000d0d4(TMPackDataBase* p_database, MechS32 p_campaign, WPARAM p_wParam);

#endif // MECHBAY_H
