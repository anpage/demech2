#ifndef SHELLMAIN_H
#define SHELLMAIN_H

#include "tmpackdatabase.h"
#include "types.h"

#include <windows.h>

// The functions and globals of shellmain.cpp that other units use.
MechS32 PumpMessage();
void EnableShellMenu(HMENU p_menu);
void DisableShellMenu(HMENU p_menu);
BOOL CALLBACK OkDialogProc(HWND p_hDlg, UINT p_msg, WPARAM p_wParam, LPARAM);
void RegisterScreenFunction(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32));
void UnregisterScreenFunction(void (*p_callback)(TMPackDataBase*, MechS32*, MechU8*, char**, MechS32));
void RegisterMenuFunction(void (*p_callback)(MechS32));
void UnregisterMenuFunction(void (*p_callback)(MechS32));

#endif // SHELLMAIN_H
