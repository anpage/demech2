#ifndef SHELLMAIN_H
#define SHELLMAIN_H

#include <windows.h>

// Shell globals shared by C and C++ units (g_hPrimaryHeap is defined in unk1003bf90.c).
#ifdef __cplusplus
extern "C"
{
#endif

	extern HANDLE g_hPrimaryHeap;

#ifdef __cplusplus
}
#endif

#endif // SHELLMAIN_H
