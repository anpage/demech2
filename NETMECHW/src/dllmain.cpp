#include "unk10003660.h"

#include <windows.h>

// FUNCTION: NETMECHW 0x10001000
extern "C" BOOL WINAPI DllMain(HINSTANCE p_hInstance, DWORD p_reason, LPVOID)
{
	switch (p_reason) {
	case DLL_PROCESS_ATTACH:
		g_hInstance = p_hInstance;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
