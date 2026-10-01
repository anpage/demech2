#ifndef UNK1000E410_H
#define UNK1000E410_H

#include "types.h"

#include <windows.h>

// The functions and globals of unk1000e410.cpp that other units use.
extern MechS32 g_unk0x10023758[8];

void FUN_1000e410(HWND p_dialog, MechS32 p_force);
BOOL CALLBACK FUN_1000e58d(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam);
void FUN_1000ef3d(HWND p_hWnd);
void FUN_1000efcd(HWND p_hWnd);

#endif // UNK1000E410_H
