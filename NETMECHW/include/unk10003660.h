#ifndef UNK10003660_H
#define UNK10003660_H

#include "types.h"
#include "unk1000aa90.h"

#include <windows.h>

class ChatLog;
class IronLantern0x160;
class SessionList;

// The functions and globals of unk10003660.cpp that other units use.
extern ChatLog g_chatLog;
extern CRITICAL_SECTION g_unk0x1001ca78;
extern IronLantern0x160 g_unk0x1001ca90;
extern DWORD g_unk0x1001ce14;
extern MechU8 g_unk0x1001ce18[8];
extern DWORD g_unk0x1001ce20;
extern HINSTANCE g_hInstance;
extern DWORD g_unk0x1001ee54;
extern CRITICAL_SECTION g_unk0x1001ee60;
extern GUID g_unk0x100230f8;
extern HWND g_unk0x10023110;
extern SessionList* g_sessionList;
extern HANDLE g_unk0x10023128;
extern HANDLE g_unk0x1002312c;
extern HANDLE g_unk0x10023130;
extern HWND g_unk0x10023160;

void FUN_100042ab();
void FUN_100057ba();
void FUN_100057fa(NetPlayer p_player, MechS32 p_index);
void FUN_10005923(MechS32 p_index);
MechS32 FUN_10005a55();
MechS32 FUN_10005a9a();
MechS32 FUN_10005cdc(MechChar* p_name, DWORD* p_size);
void FUN_10005e36();

// VC++ 2.2 constructs unk10003660.cpp's global objects (g_unk0x1001ca90, then g_chatLog) in one
// function at the start of the object, and destroys them through a function registered with
// atexit by two more at its end.

// SYNTHETIC: NETMECHW 0x10003660
// $$1000

// SYNTHETIC: NETMECHW 0x10005f9d
// $$2000

// SYNTHETIC: NETMECHW 0x10005fc1
// $$3000

// SYNTHETIC: NETMECHW 0x10005fde
// $$4000

#endif // UNK10003660_H
