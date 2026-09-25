/* The keyboard object is shared with MW2.DLL (KeyboardReadKeyCode is byte-identical); both
   targets keep their own copy until one source matches both. */
#include "decomp.h"
#include "types.h"

#include <windows.h>

// GLOBAL: MW2SHELL 0x10079690
undefined4 g_unk0x10079690;

// GLOBAL: MW2SHELL 0x1007968c
undefined4 g_unk0x1007968c;

// GLOBAL: MW2SHELL 0x10096eb0
undefined2 g_unk0x10096eb0[64];

// FUNCTION: MW2SHELL 0x10004a60
MechS32 GetKeyboardDeviceCount(void)
{
	return 1;
}

// FUNCTION: MW2SHELL 0x10004b99
MechS32 KeyboardReadKeyCode(undefined2* p_keyCode)
{
	if (g_unk0x1007968c == g_unk0x10079690) {
		*p_keyCode = 0;
	}
	else {
		*p_keyCode = g_unk0x10096eb0[g_unk0x1007968c];
		g_unk0x1007968c++;
		if (g_unk0x1007968c == 64) {
			g_unk0x1007968c = 0;
		}
	}

	return 0;
}

// STUB: MW2SHELL 0x10004c56
void HandleKeyboardMessages(UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	STUB(0x10004c56);
}
