#ifndef SHELLMAIN_H
#define SHELLMAIN_H

#include "tmpackdatabase.h"
#include "types.h"

#include <windows.h>

// The functions and globals of shellmain.cpp that other units use.
MechS32 FUN_1000fe0d();
void FUN_1001023c(HMENU p_menu);
void FUN_10010320(HMENU p_menu);
BOOL CALLBACK FUN_1001067f(HWND p_hDlg, UINT p_msg, WPARAM p_wParam, LPARAM);
void FUN_100108e5(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32));
void FUN_100108fd(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32));
void FUN_100109a0(void (*p_callback)(MechS32));
void FUN_100109b8(void (*p_callback)(MechS32));

#endif // SHELLMAIN_H
