#ifndef MECHBAY_H
#define MECHBAY_H

#include "mechchassis.h"
#include "screenfield.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <windows.h>

// The functions and globals of mechbay.cpp that other units use.
extern MechChassis g_unk0x10061560[];
extern MechS32 g_unk0x10061774;

void FUN_100078cd(ScreenField* p_tabs);
void FUN_100079f8(ScreenField* p_tabs);
void FUN_10007ac8(ScreenField* p_tabs);
ScreenField* FUN_1000b5ed(ScreenField* p_tabs, MechS32 p_x, MechS32 p_y);
void FUN_1000d0d4(TMPackDataBase* p_database, MechS32 p_campaign, WPARAM p_wParam);

#endif // MECHBAY_H
