#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include "types.h"

#include <stdio.h>
#include <windows.h>

// The functions and globals of logwindow.c that other units use.
#ifdef __cplusplus
extern "C"
{
#endif

	extern MechS32 g_debugLogMode;
	extern HWND g_debugLogWindow;
	extern UINT g_debugLogBoxType;
	extern FILE* g_debugLogFile;
	extern MechChar g_debugLogPath[0x100];
	extern MechChar g_debugLogBoxTitle[0x50];

	void FUN_1003a1c0(void);
	void FUN_1003a1e2(void);
	void FUN_1003a21c(MechChar* p_text);
	void FUN_1003a299(MechChar* p_text);
	void FUN_1003a2e6(void);
	void FUN_1003a308(MechChar* p_text);
	MechS32 FUN_1003a37c(MechS32 p_mode);
	void FUN_1003a3b9(HWND p_window);
	void FUN_1003a3cc(UINT p_type);
	void FUN_1003a3df(const MechChar* p_format, ...);
	void FUN_1003a411(const MechChar* p_path);
	void FUN_1003a432(const MechChar* p_format, ...);

#ifdef __cplusplus
}
#endif

#endif // LOGWINDOW_H
