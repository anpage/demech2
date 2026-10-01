#ifndef UNK100025F0_H
#define UNK100025F0_H

#include "types.h"

#include <windows.h>

// The functions and globals of unk100025f0.cpp that other units use.
void FUN_100025f0(HWND p_dialog, MechS32 p_force);
BOOL CALLBACK FUN_1000268a(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam);
void FUN_10002e0e(HWND p_dialog);
void FUN_10002ec3(HWND p_dialog);
void FUN_10003154(HWND p_listBox);
void CopyPlayerMech(MechChar* p_mechFile);
void RegisterLobbyHotKeys(HWND p_hWnd);
void UnregisterLobbyHotKeys(HWND p_hWnd);

#endif // UNK100025F0_H
