#include "types.h"

#include <stdarg.h>
#include <stdio.h>

// GLOBAL: MW2SHELL 0x10096760
MechChar g_unk0x10096760[0x100];

void DebugPrintInternal(MechChar* p_message);

// FUNCTION: MW2SHELL 0x10015ce8
void DebugPrint(const MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_unk0x10096760, sizeof(g_unk0x10096760), p_format, args);
	va_end(args);
	DebugPrintInternal(g_unk0x10096760);
}
