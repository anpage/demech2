#ifndef UNK10006060_H
#define UNK10006060_H

#include "types.h"

#include <windows.h>

// The functions and globals of unk10006060.cpp that other units use.
MechChar* FUN_10006060(MechChar* p_name);
MechS32 FUN_100060dc(MechChar* p_name);
BOOL FAR PASCAL AddServiceProvider(LPGUID p_guid, LPSTR p_name, DWORD p_major, DWORD p_minor, LPVOID p_context);
MechS32 FUN_1000633a(HWND p_dialog);
void FUN_100064e4(HWND p_dialog);
void FUN_10006a68(HWND p_hWnd);
void FUN_10006ac8(HWND p_hWnd);

#endif // UNK10006060_H
