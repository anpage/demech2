#ifndef ASMEQUIV_POSIX_WINDOWS_H
#define ASMEQUIV_POSIX_WINDOWS_H

// The little of <windows.h> the units under test need, for the standalone build on platforms
// without it (stubs.c defines the functions; nothing calls them).

#include <stddef.h>

typedef int BOOL;
typedef unsigned long DWORD;
typedef size_t SIZE_T;
typedef void* HANDLE;
typedef void* HWND;
typedef void* LPVOID;
typedef const void* LPCVOID;

#define TRUE 1
#define FALSE 0
#define __stdcall
#define __declspec(p_attribute)
#define HEAP_NO_SERIALIZE 0x1
#define HEAP_ZERO_MEMORY 0x8

LPVOID HeapAlloc(HANDLE p_heap, DWORD p_flags, SIZE_T p_size);
BOOL HeapFree(HANDLE p_heap, DWORD p_flags, LPVOID p_block);
SIZE_T HeapSize(HANDLE p_heap, DWORD p_flags, LPCVOID p_block);

// MSVC's <string.h>
int _strcmpi(const char* p_a, const char* p_b);

#endif // ASMEQUIV_POSIX_WINDOWS_H
