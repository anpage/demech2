#ifndef UNK1003BF90_H
#define UNK1003BF90_H

#include "decomp.h"
#include "types.h"

#include <windows.h>

// The functions and globals of unk1003bf90.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern char g_windowClassName[0x10];
	extern MechS32 g_fWindowActive;
	extern MechS32 g_unk0x1006a9d8;
	extern MechU32 g_fQuickTips;
	extern MechS32 g_showDialog;
	extern MechS32 g_menuVisible;
	extern MechS32 g_fHelpRegistered;
	extern MechS32 g_menuDialogOpen;
	extern MechS32 g_littleMovies;
	extern HANDLE g_hPrimaryHeap;

	undefined4 FUN_1003bf90(MechS32 p_unk0x00);

#ifdef __cplusplus
}
#endif

#endif // UNK1003BF90_H
