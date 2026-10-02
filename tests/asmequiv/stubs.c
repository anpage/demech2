// What the units under test reference beyond the routines: clock.c's tables, which the cases
// fill, and empty stand-ins for the functions and globals the units' other functions use. None
// of them is called.

#include "clock.h"
#include "error.h"
#include "gamekeys.h"
#include "prjfile.h"
#include "simmain.h"
#include "timedoverlays.h"
#include "types.h"

#include <stddef.h>

MechS32 g_sinTable[0x102];
MechS32 g_atanTable[0x102];
MechS32 g_missionTimerStopped;
HANDLE g_primaryHeap;

MechS32 FUN_1007c930(void)
{
	return 0;
}

MechS32 FUN_1007c9e3(void)
{
	return 0;
}

MechS32 FUN_1007ca8e(void)
{
	return 0;
}

void Error(MechS32 p_code, const char* p_format, ...)
{
	(void) p_code;
	(void) p_format;
}

MechS32 GetPrjResourceSize(MechS32 p_file, const MechChar* p_type, MechU16 p_id)
{
	(void) p_file;
	(void) p_type;
	(void) p_id;
	return 0;
}

MechS32 ReadPrjResource(MechS32 p_file, const MechChar* p_type, MechU16 p_id, void* p_buffer)
{
	(void) p_file;
	(void) p_type;
	(void) p_id;
	(void) p_buffer;
	return 0;
}

MechS32 ShowInGameMessage(MechChar* p_text, MechS32 p_font, MechS32 p_duration, MechS32 p_priority)
{
	(void) p_text;
	(void) p_font;
	(void) p_duration;
	(void) p_priority;
	return 0;
}

#ifndef _WIN32
LPVOID HeapAlloc(HANDLE p_heap, DWORD p_flags, SIZE_T p_size)
{
	(void) p_heap;
	(void) p_flags;
	(void) p_size;
	return NULL;
}

BOOL HeapFree(HANDLE p_heap, DWORD p_flags, LPVOID p_block)
{
	(void) p_heap;
	(void) p_flags;
	(void) p_block;
	return FALSE;
}

SIZE_T HeapSize(HANDLE p_heap, DWORD p_flags, LPCVOID p_block)
{
	(void) p_heap;
	(void) p_flags;
	(void) p_block;
	return 0;
}
#endif
