#include "decomp.h"
#include "types.h"

#include <stdarg.h>
#include <stdio.h>

// GLOBAL: MW2 0x100ea3f0
MechChar g_unk0x100ea3f0[0x100];

void FUN_1003a432(MechChar* p_message);

// STUB: MW2 0x10050780
void FirstResource(void)
{
	STUB(0x10050780);
}

// STUB: MW2 0x1005082f
void CloseResourceFile(void)
{
	STUB(0x1005082f);
}

// STUB: MW2 0x10050848
void CachePreloads(void)
{
	STUB(0x10050848);
}

// FUNCTION: MW2 0x10050958
void DebugLog(const MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_unk0x100ea3f0, sizeof(g_unk0x100ea3f0), p_format, args);
	va_end(args);
	FUN_1003a432(g_unk0x100ea3f0);
}
