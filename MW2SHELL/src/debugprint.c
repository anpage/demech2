#include "types.h"

#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

// GLOBAL: MW2SHELL 0x10096760
MechChar g_unk0x10096760[0x100];

extern HWND g_pWnd;

void DebugPrintInternal(MechChar* p_message);

// FUNCTION: MW2SHELL 0x10015c90
void FUN_10015c90(const MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_unk0x10096760, sizeof(g_unk0x10096760), p_format, args);
	va_end(args);
	OutputDebugString(g_unk0x10096760);
	MessageBox(g_pWnd, g_unk0x10096760, "MechWarrior2 Message", MB_ICONASTERISK);
}

// FUNCTION: MW2SHELL 0x10015ce8
void DebugPrint(const MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_unk0x10096760, sizeof(g_unk0x10096760), p_format, args);
	va_end(args);
	DebugPrintInternal(g_unk0x10096760);
}
