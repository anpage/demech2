#ifndef UNK100089D0_H
#define UNK100089D0_H

#include "types.h"

#include <windows.h>

// The functions and globals of unk100089d0.cpp that other units use.
extern UINT g_unk0x100234f8[4];
extern UINT g_unk0x10023508[3];

void FUN_100089d0(HWND p_dialog, MechS32 p_force);
BOOL CALLBACK FUN_10008d5d(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam);
void FUN_10009183(HWND p_hWnd);
void FUN_100091e3(HWND p_hWnd);

#endif // UNK100089D0_H
