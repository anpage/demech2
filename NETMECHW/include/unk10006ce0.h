#ifndef UNK10006CE0_H
#define UNK10006CE0_H

#include "types.h"

#include <dplay.h>
#include <windows.h>

// The functions and globals of unk10006ce0.cpp that other units use.
MechS32 HostSession(HWND p_dialog);
MechS32 JoinSession(HWND p_dialog);
BOOL FAR PASCAL AddSessionPlayer(DPID p_id, LPSTR p_friendlyName, LPSTR p_formalName, DWORD p_flags, LPVOID p_context);
void FUN_10007b30(HWND p_hWnd);
void FUN_10007bce(HWND p_hWnd);

#endif // UNK10006CE0_H
