#include "debugout.h"

#include "decomp.h"
#include "types.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// GLOBAL: MW2SHELL 0x10064b70
MechS32 g_unk0x10064b70 = 0;

// GLOBAL: MW2SHELL 0x10064b74
HWND g_unk0x10064b74 = NULL;

// GLOBAL: MW2SHELL 0x10064b78
UINT g_unk0x10064b78 = MB_ICONASTERISK;

// GLOBAL: MW2SHELL 0x10064b7c
FILE* g_unk0x10064b7c = NULL;

// GLOBAL: MW2SHELL 0x10064b80
MechChar g_unk0x10064b80[0x100] = "debug.log";

// GLOBAL: MW2SHELL 0x10064c80
MechChar g_unk0x10064c80[0x50] = "DEBUG Message";

// FUNCTION: MW2SHELL 0x10017710
void FUN_10017710(void)
{
	// The ranges overlap (dst 0xb0000 < src 0xb00a0), so this is memmove: /Oi would expand
	// memcpy inline even at this constant size.
	memmove((void*) 0xb0000, (void*) 0xb00a0, 0xf00);
}

// FUNCTION: MW2SHELL 0x10017732
void FUN_10017732(void)
{
	MechS32 pixelAddress = 0xb0f00;

	while (pixelAddress < 0xb0fa0) {
		*(MechS32*) pixelAddress = *(MechS32*) pixelAddress & 0xff00ff00;
		pixelAddress += 4;
	}
}

// Stack-slot permutation: original source/destination/length/index are at
// [ebp-0xc]/[ebp-4]/[ebp-0x10]/[ebp-8]; VC++ assigns
// [ebp-4]/[ebp-0x10]/[ebp-8]/[ebp-0xc] here.
// FUNCTION: MW2SHELL 0x1001776c
void FUN_1001776c(MechChar* p_message)
{
	MechChar* source = p_message;
	MechChar* destination = (MechChar*) 0xb0f00;
	MechS32 length = strlen(p_message);
	MechS32 index;

	if (length > 0x50) {
		length = 0x50;
	}

	FUN_10017710();
	FUN_10017732();
	for (index = 0; index < length; index++) {
		*destination = *source;
		source++;
		destination++;
		destination++;
	}
}

// Stack-slot permutation: original length/offset are at [ebp-8]/[ebp-4];
// VC++ assigns [ebp-4]/[ebp-8] here.
// FUNCTION: MW2SHELL 0x100177e9
void FUN_100177e9(MechChar* p_message)
{
	MechS32 length = strlen(p_message);
	MechS32 offset = 0;

	while (offset < length) {
		FUN_1001776c(p_message + offset);
		offset += 0x50;
	}
}

// FUNCTION: MW2SHELL 0x10017836
void FUN_10017836(void)
{
	g_unk0x10064b7c = fopen(g_unk0x10064b80, "wt");
}

// FUNCTION: MW2SHELL 0x10017858
void FUN_10017858(MechChar* p_message)
{
	if (g_unk0x10064b7c == NULL) {
		g_unk0x10064b7c = fopen(g_unk0x10064b80, "at");
	}
	if (g_unk0x10064b7c != NULL) {
		fputs(p_message, g_unk0x10064b7c);
		fflush(g_unk0x10064b7c);
		fclose(g_unk0x10064b7c);
		g_unk0x10064b7c = NULL;
	}
}

// FUNCTION: MW2SHELL 0x100178cc
MechS32 FUN_100178cc(MechS32 p_mode)
{
	if (p_mode >= 0 && p_mode <= 4) {
		g_unk0x10064b70 = p_mode;

		return 0;
	}
	else {
		return 2;
	}
}

// FUNCTION: MW2SHELL 0x10017909
void FUN_10017909(HWND p_hWnd)
{
	g_unk0x10064b74 = p_hWnd;
}

// FUNCTION: MW2SHELL 0x1001791c
void FUN_1001791c(UINT p_type)
{
	g_unk0x10064b78 = p_type;
}

// FUNCTION: MW2SHELL 0x1001792f
void FUN_1001792f(MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_unk0x10064c80, 0x50, p_format, args);
	va_end(args);
}

// FUNCTION: MW2SHELL 0x10017961
void FUN_10017961(MechChar* p_fileName)
{
	strncpy(g_unk0x10064b80, p_fileName, 0x100);
}

// Stack-slot permutation: original message is at [ebp-0x100] and args at [ebp-0x104];
// VC++ assigns them [ebp-0x104] and [ebp-4] here.
// FUNCTION: MW2SHELL 0x10017982
void DebugPrintInternal(MechChar* p_message, ...)
{
	MechChar message[0x100];
	va_list args;

	va_start(args, p_message);
	_vsnprintf(message, sizeof(message), p_message, args);
	va_end(args);

	switch (g_unk0x10064b70) {
	case 1:
		FUN_100177e9(message);
		break;
	case 2:
		OutputDebugString(message);
		break;
	case 4:
		FUN_10017858(message);
		break;
	case 3:
		if (g_unk0x10064b74 != NULL) {
			MessageBox(g_unk0x10064b74, message, g_unk0x10064c80, g_unk0x10064b78);
		}
		break;
	}
}
