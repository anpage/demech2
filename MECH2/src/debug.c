#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Debug output. DebugPrint formats a message and routes it according to g_debugMode:
// the monochrome text screen (a second, MDA display at 0xb0000, as in the shell's
// debugout.c), OutputDebugString, a message box, or the log file. The mode defaults to
// none, and nothing in MECH2.EXE changes it.

// GLOBAL: MECH2 0x0040d7b8
MechS32 g_debugMode = c_debugModeNone;
// GLOBAL: MECH2 0x0040d7bc
HWND g_debugWindow = NULL;
// GLOBAL: MECH2 0x0040d7c0
UINT g_debugMessageBoxType = MB_ICONINFORMATION;
// GLOBAL: MECH2 0x0040d7c4
FILE* g_debugLogFile = NULL;
// GLOBAL: MECH2 0x0040d7c8
MechChar g_debugLogName[0x100] = "debug.log";
// GLOBAL: MECH2 0x0040d8c8
MechChar g_debugCaption[0x50] = "DEBUG Message";

// The monochrome screen is 80x25 characters of two bytes each (character, attribute).

// FUNCTION: MECH2 0x00401e80
void MonoScroll(void)
{
	// The ranges overlap: scrolls the screen up by one line
	memmove((void*) 0xb0000, (void*) 0xb00a0, 0xf00);
}

// FUNCTION: MECH2 0x00401ea2
void MonoClearLastLine(void)
{
	MechS32 cell;

	// Clears the characters and keeps the attributes
	cell = 0xb0f00;
	while (cell < 0xb0fa0) {
		*(MechU32*) cell = *(MechS32*) cell & 0xff00ff00;
		cell += 4;
	}
}

// Stack-slot permutation: dst and i.
// FUNCTION: MECH2 0x00401edc
void MonoPrintLine(const MechChar* p_text)
{
	const MechChar* src;
	MechChar* dst;
	MechS32 length;
	MechS32 i;

	src = p_text;
	dst = (MechChar*) 0xb0f00;
	length = strlen(p_text);
	if (length > 80) {
		length = 80;
	}

	MonoScroll();
	MonoClearLastLine();
	for (i = 0; i < length; i++) {
		*dst = *src;
		src++;
		dst++;
		dst++;
	}
}

// FUNCTION: MECH2 0x00401f59
void MonoPrint(const MechChar* p_text)
{
	MechS32 length;
	MechS32 i;

	length = strlen(p_text);
	i = 0;
	while (i < length) {
		MonoPrintLine(p_text + i);
		i += 80;
	}
}

// FUNCTION: MECH2 0x00401fa6
void OpenDebugLog(void)
{
	g_debugLogFile = fopen(g_debugLogName, "wt");
}

// FUNCTION: MECH2 0x00401fc8
void WriteDebugLog(const MechChar* p_text)
{
	if (g_debugLogFile == NULL) {
		g_debugLogFile = fopen(g_debugLogName, "at");
	}

	if (g_debugLogFile != NULL) {
		fputs(p_text, g_debugLogFile);
		fflush(g_debugLogFile);
		fclose(g_debugLogFile);
		g_debugLogFile = NULL;
	}
}

// FUNCTION: MECH2 0x0040203c
MechS32 SetDebugMode(MechS32 p_mode)
{
	if (p_mode >= c_debugModeNone && p_mode <= c_debugModeLog) {
		g_debugMode = p_mode;
		return 0;
	}
	else {
		return 2;
	}
}

// FUNCTION: MECH2 0x00402079
void SetDebugWindow(HWND p_hWnd)
{
	g_debugWindow = p_hWnd;
}

// FUNCTION: MECH2 0x0040208c
void SetDebugMessageBoxType(UINT p_type)
{
	g_debugMessageBoxType = p_type;
}

// FUNCTION: MECH2 0x0040209f
void SetDebugCaption(const MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_debugCaption, sizeof(g_debugCaption), p_format, args);
	va_end(args);
}

// FUNCTION: MECH2 0x004020d1
void SetDebugLogName(const MechChar* p_name)
{
	strncpy(g_debugLogName, p_name, sizeof(g_debugLogName));
}

// Stack-slot permutation: buffer and args.
// FUNCTION: MECH2 0x004020f2
void DebugPrint(const MechChar* p_format, ...)
{
	MechChar buffer[0x100];
	va_list args;

	va_start(args, p_format);
	_vsnprintf(buffer, sizeof(buffer), p_format, args);
	va_end(args);

	switch (g_debugMode) {
	case c_debugModeMono:
		MonoPrint(buffer);
		break;
	case c_debugModeOutput:
		OutputDebugString(buffer);
		break;
	case c_debugModeLog:
		WriteDebugLog(buffer);
		break;
	case c_debugModeMessageBox:
		if (g_debugWindow != NULL) {
			MessageBox(g_debugWindow, buffer, g_debugCaption, g_debugMessageBoxType);
		}
		break;
	default:
		break;
	}
}
