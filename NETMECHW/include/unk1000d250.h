#ifndef UNK1000D250_H
#define UNK1000D250_H

#include "types.h"

#include <windows.h>

// The functions and globals of unk1000d250.cpp that other units use.
extern MechS32 g_unk0x10023708[16];

void FUN_1000d250(HWND p_dialog, MechS32 p_force);
BOOL CALLBACK FUN_1000d31f(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam);
void FUN_1000d91f(HWND p_dialog, MechS32 p_force);
BOOL CALLBACK FUN_1000dae1(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam);
void FUN_1000e2e2(HWND p_hWnd);
void FUN_1000e396(HWND p_hWnd);

#endif // UNK1000D250_H
