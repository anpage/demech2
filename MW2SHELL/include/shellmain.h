#ifndef SHELLMAIN_H
#define SHELLMAIN_H

#include <windows.h>

// Globals defined in shellmain.cpp and shared with other units, C and C++ alike.
#ifdef __cplusplus
extern "C"
{
#endif

	extern HANDLE g_hPrimaryHeap;

#ifdef __cplusplus
}
#endif

#endif // SHELLMAIN_H
