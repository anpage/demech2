#ifndef UNK10010460_H
#define UNK10010460_H

#include "types.h"

#include <dplay.h>
#include <windows.h>

// The functions and globals of unk10010460.cpp that other units use.
MechS32 FUN_10010460(HWND p_dialog);
void FUN_10010475(HWND p_dialog);
BOOL CALLBACK FUN_100105ee(HWND p_dialog, UINT p_message, WPARAM p_wParam, LPARAM p_lParam);
MechS32 FUN_10010920(DPID* p_ids);
char CdCheck();

#endif // UNK10010460_H
