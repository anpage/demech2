#ifndef DEBUG_H
#define DEBUG_H

#include "types.h"

#include <windows.h>

// Where DebugPrint sends its output
enum DebugMode {
	c_debugModeNone = 0,
	c_debugModeMono = 1,       // monochrome (MDA) text screen at 0xb0000
	c_debugModeOutput = 2,     // OutputDebugString
	c_debugModeMessageBox = 3, // MessageBox, when a window is set
	c_debugModeLog = 4         // appended to the log file
};

void MonoScroll(void);
void MonoClearLastLine(void);
void MonoPrintLine(const MechChar* p_text);
void MonoPrint(const MechChar* p_text);
void OpenDebugLog(void);
void WriteDebugLog(const MechChar* p_text);
MechS32 SetDebugMode(MechS32 p_mode);
void SetDebugWindow(HWND p_hWnd);
void SetDebugMessageBoxType(UINT p_type);
void SetDebugCaption(const MechChar* p_format, ...);
void SetDebugLogName(const MechChar* p_name);
void DebugPrint(const MechChar* p_format, ...);

#endif // DEBUG_H
