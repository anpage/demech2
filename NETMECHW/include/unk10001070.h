#ifndef UNK10001070_H
#define UNK10001070_H

#include "types.h"

#include <windows.h>

// The functions and globals of unk10001070.cpp that other units use.
MechChar* LoadResString(UINT p_id);
void CenterWindow(HWND p_hWnd);
void SetBusyCursor(MechS32 p_busy);
void FUN_1000120a(HWND p_hWnd, HWND p_child);
void TrimLeadingSpace(MechChar* p_string);
MechS32 IsBlankString(MechChar* p_string);

#endif // UNK10001070_H
