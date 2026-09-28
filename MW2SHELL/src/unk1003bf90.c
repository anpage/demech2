#include "unk1003bf90.h"

#include "decomp.h"
#include "types.h"

#include <windows.h>

// The shell window's state. The original keeps these in this object's data, between joystick.c's
// and unk1003bfb0.c's.

// The window class name.
// GLOBAL: MW2SHELL 0x1006a9c0
char g_unk0x1006a9c0[0x10] = "MECHWARRIOR 2";

// GLOBAL: MW2SHELL 0x1006a9d0
MechS32 g_fWindowActive = 1;

// Read by the draw mode unit (unk10010a30.c) when it restyles the window.
// GLOBAL: MW2SHELL 0x1006a9d8
MechS32 g_unk0x1006a9d8 = 0;

// GLOBAL: MW2SHELL 0x1006a9dc
MechU32 g_fQuickTips = 1;

// GLOBAL: MW2SHELL 0x1006a9e0
MechS32 g_unk0x1006a9e0 = 1;

// GLOBAL: MW2SHELL 0x1006a9e4
MechS32 g_menuVisible = 0;

// GLOBAL: MW2SHELL 0x1006a9e8
MechS32 g_fHelpRegistered = 0;

// GLOBAL: MW2SHELL 0x1006a9ec
MechS32 g_menuDialogOpen = 0;

// GLOBAL: MW2SHELL 0x1006a9f0
MechS32 g_unk0x1006a9f0 = 0;

// GLOBAL: MW2SHELL 0x1006a9f4
HANDLE g_hPrimaryHeap = NULL;

// FUNCTION: MW2SHELL 0x1003bf90
undefined4 FUN_1003bf90(MechS32 p_unk0x00)
{
	return 0;
}
