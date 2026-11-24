#include "debugprint.h"

#include "logwindow.h"
#include "simmain.h"
#include "types.h"

#include <stdarg.h>
#include <stdio.h>
#include <windows.h>

// GLOBAL: MW2 0x100ea3f0
// GLOBAL: MW2GBL 0x100e98e0
// GLOBAL: MW2PREMERCS 0x10212e30
// GLOBAL: MW2MATROX 0x1012c200
// GLOBAL: MW23DFX 0x102442a0
// GLOBAL: MW2ATI 0x10177f60
// GLOBAL: MW2S3 0x1008ac30
// GLOBAL: MW2POWERVR 0x101a2c20
MechChar g_debugPrintBuffer[0x100];

// FUNCTION: MW2 0x10050900
// FUNCTION: MW2GBL 0x1003c010
// FUNCTION: MW2PREMERCS 0x100271a0
// FUNCTION: MW2MATROX 0x10085be0
// FUNCTION: MW23DFX 0x10008460
void ShowMessage(const MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_debugPrintBuffer, sizeof(g_debugPrintBuffer), p_format, args);
	va_end(args);
	OutputDebugString(g_debugPrintBuffer);
#ifdef MW2_GHOST_BEAR_TITLES
	MessageBox(g_gameWindow, g_debugPrintBuffer, "Ghost Bear's Legacy Message", MB_ICONASTERISK);
#else
	MessageBox(g_gameWindow, g_debugPrintBuffer, "MechWarrior2 Message", MB_ICONASTERISK);
#endif
}

// FUNCTION: MW2 0x10050958
// FUNCTION: MW2GBL 0x1003c068
// FUNCTION: MW2PREMERCS 0x100271f8
// FUNCTION: MW2MATROX 0x10085c38
// FUNCTION: MW23DFX 0x100084b0
// FUNCTION: MW2ATI 0x10008ca0
// FUNCTION: MW2S3 0x10035140
// FUNCTION: MW2POWERVR 0x10007a10
void DebugPrint(const MechChar* p_format, ...)
{
	va_list args;

	va_start(args, p_format);
	_vsnprintf(g_debugPrintBuffer, sizeof(g_debugPrintBuffer), p_format, args);
	va_end(args);
	DebugPrintInternal(g_debugPrintBuffer);
}
