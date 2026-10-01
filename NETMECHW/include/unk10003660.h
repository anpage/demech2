#ifndef UNK10003660_H
#define UNK10003660_H

#include "types.h"

#include <windows.h>

class ChatLog;
class IronLantern0x160;
class SessionList;

// The functions and globals of unk10003660.cpp that other units use.
extern ChatLog g_chatLog;
extern CRITICAL_SECTION g_unk0x1001ca78;
extern IronLantern0x160 g_unk0x1001ca90;
extern MechU8 g_unk0x1001ce18[8];
extern HINSTANCE g_hInstance;
extern GUID g_unk0x100230f8;
extern SessionList* g_sessionList;
extern HWND g_unk0x10023160;

MechS32 FUN_10005cdc(MechChar* p_name, DWORD* p_size);

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
