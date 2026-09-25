#include "debug.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Debug output. DebugPrint formats a message and routes it according to g_debugMode:
// the monochrome text screen (a second, MDA display at 0xb0000, as in the shell's
// debugout.c), OutputDebugString, a message box, or the log file. The mode defaults to
// none, and nothing in MECH2.EXE changes it.

MechS32 g_debugMode = c_debugModeNone;
HWND g_debugWindow = NULL;
UINT g_debugMessageBoxType = MB_ICONINFORMATION;
FILE* g_debugLogFile = NULL;
MechChar g_debugLogName[0x100] = "debug.log";
MechChar g_debugCaption[0x50] = "DEBUG Message";

// The monochrome screen is 80x25 characters of two bytes each (character, attribute).

void MonoScroll(void)
{
	// The ranges overlap: scrolls the screen up by one line
	memmove((void*) 0xb0000, (void*) 0xb00a0, 0xf00);
}

void MonoClearLastLine(void)
{
	MechU32* cell;

	// Clears the characters and keeps the attributes
	for (cell = (MechU32*) 0xb0f00; cell < (MechU32*) 0xb0fa0; cell++) {
		*cell &= 0xff00ff00;
	}
}

void MonoPrintLine(const MechChar* p_text)
{
	const MechChar* src;
	MechChar* dst;
	MechS32 i;
	MechS32 length;

	src = p_text;
	dst = (MechChar*) 0xb0f00;
	length = strlen(p_text);
	if (length > 80) {
		length = 80;
	}

	MonoScroll();
	MonoClearLastLine();
	for (i = 0; i < length; i++) {
		*dst = *src++;
		dst += 2;
	}
}

void MonoPrint(const MechChar* p_text)
{
	MechS32 i;

	for (i = 0; i < (MechS32) strlen(p_text); i += 80) {
		MonoPrintLine(p_text + i);
	}
}

void OpenDebugLog(void)
{
	g_debugLogFile = fopen(g_debugLogName, "wt");
}

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

MechS32 SetDebugMode(MechS32 p_mode)
{
	if (p_mode < c_debugModeNone || p_mode > c_debugModeLog) {
		return 2;
	}

	g_debugMode = p_mode;
	return 0;
}

void SetDebugWindow(HWND p_hWnd)
{
	g_debugWindow = p_hWnd;
}

void SetDebugMessageBoxType(UINT p_type)
{
	g_debugMessageBoxType = p_type;
}

void SetDebugCaption(const MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_debugCaption, sizeof(g_debugCaption), p_format, args);
	va_end(args);
}

void SetDebugLogName(const MechChar* p_name)
{
	strncpy(g_debugLogName, p_name, sizeof(g_debugLogName));
}

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
	}
}
