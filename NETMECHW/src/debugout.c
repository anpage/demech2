#include "debugout.h"

#include "decomp.h"
#include "types.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// NETMECHW.DLL's copy of the shell's debugout.c, the same source but where noted.

// Where DebugPrintInternal sends messages: 1 the monochrome display (text memory at 0xb0000),
// 2 OutputDebugString, 3 a message box, 4 the log file and the monochrome display (the shell's
// copy writes only the log); anything else drops them.
// GLOBAL: NETMECHW 0x10025228
MechS32 g_debugOutputMode = 0;

// GLOBAL: NETMECHW 0x1002522c
HWND g_debugWindow = NULL;

// GLOBAL: NETMECHW 0x10025230
UINT g_debugMessageBoxType = MB_ICONASTERISK;

// GLOBAL: NETMECHW 0x10025234
FILE* g_debugLogFile = NULL;

// GLOBAL: NETMECHW 0x10025238
MechChar g_debugLogName[0x100] = "debug.log";

// GLOBAL: NETMECHW 0x10025338
MechChar g_debugMessageTitle[0x50] = "DEBUG Message";

// FUNCTION: NETMECHW 0x10013800
void ScrollMonoDisplay(void)
{
	// The ranges overlap (dst 0xb0000 < src 0xb00a0), so this is memmove: /Oi would expand
	// memcpy inline even at this constant size.
	memmove((void*) 0xb0000, (void*) 0xb00a0, 0xf00);
}

// FUNCTION: NETMECHW 0x10013822
void ClearMonoLastLine(void)
{
	MechS32 pixelAddress = 0xb0f00;

	while (pixelAddress < 0xb0fa0) {
		*(MechS32*) pixelAddress = *(MechS32*) pixelAddress & 0xff00ff00;
		pixelAddress += 4;
	}
}

// Stack-slot permutation (VC++ 2.2): source, destination and index.
// FUNCTION: NETMECHW 0x1001385c
void PrintMonoLine(MechChar* p_message)
{
	MechChar* source = p_message;
	MechChar* destination = (MechChar*) 0xb0f00;
	MechS32 length = strlen(p_message);
	MechS32 index;

	if (length > 0x50) {
		length = 0x50;
	}

	ScrollMonoDisplay();
	ClearMonoLastLine();
	for (index = 0; index < length; index++) {
		*destination = *source;
		source++;
		destination++;
		destination++;
	}
}

// FUNCTION: NETMECHW 0x100138d9
void PrintMono(MechChar* p_message)
{
	MechS32 length = strlen(p_message);
	MechS32 offset = 0;

	while (offset < length) {
		PrintMonoLine(p_message + offset);
		offset += 0x50;
	}
}

// FUNCTION: NETMECHW 0x10013926
void CreateDebugLog(void)
{
	g_debugLogFile = fopen(g_debugLogName, "wt");
}

// FUNCTION: NETMECHW 0x10013948
void AppendDebugLog(MechChar* p_message)
{
	if (g_debugLogFile == NULL) {
		g_debugLogFile = fopen(g_debugLogName, "at");
	}
	if (g_debugLogFile != NULL) {
		fputs(p_message, g_debugLogFile);
		fflush(g_debugLogFile);
		fclose(g_debugLogFile);
		g_debugLogFile = NULL;
	}
}

// FUNCTION: NETMECHW 0x100139bc
MechS32 SetDebugOutputMode(MechS32 p_mode)
{
	if (p_mode >= 0 && p_mode <= 4) {
		g_debugOutputMode = p_mode;

		return 0;
	}
	else {
		return 2;
	}
}

// FUNCTION: NETMECHW 0x100139f9
void SetDebugWindow(HWND p_hWnd)
{
	g_debugWindow = p_hWnd;
}

// FUNCTION: NETMECHW 0x10013a0c
void SetDebugMessageBoxType(UINT p_type)
{
	g_debugMessageBoxType = p_type;
}

// FUNCTION: NETMECHW 0x10013a1f
void SetDebugMessageTitle(MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_debugMessageTitle, 0x50, p_format, args);
	va_end(args);
}

// FUNCTION: NETMECHW 0x10013a51
void SetDebugLogName(MechChar* p_fileName)
{
	strncpy(g_debugLogName, p_fileName, 0x100);
}

// Stack-slot permutation (VC++ 2.2): message and args. The jump table's entries move with
// the longer encodings the permutation gives the recompiled cases.
// FUNCTION: NETMECHW 0x10013a72
void DebugPrintInternal(MechChar* p_message, ...)
{
	MechChar message[0x100];
	va_list args;

	va_start(args, p_message);
	_vsnprintf(message, sizeof(message), p_message, args);
	va_end(args);

	switch (g_debugOutputMode) {
	case 1:
		PrintMono(message);
		break;
	case 2:
		OutputDebugString(message);
		break;
	case 4:
		PrintMono(message);
		AppendDebugLog(message);
		break;
	case 3:
		if (g_debugWindow != NULL) {
			MessageBox(g_debugWindow, message, g_debugMessageTitle, g_debugMessageBoxType);
		}
		break;
	default:
		break;
	}
}
