#ifndef UNK10011120_H
#define UNK10011120_H

#include "types.h"

#include <windows.h>

// The functions and globals of unk10011120.cpp that other units use.
extern MechChar g_unk0x10023948[4];
extern HWND g_unk0x1001f2a4;

void FUN_10011120(HWND p_dialog, MechS32 p_force);
BOOL CALLBACK FUN_100113ec(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam);
void FUN_10011b0b(HWND p_hWnd);
void FUN_10011bad(HWND p_hWnd);

#endif // UNK10011120_H
