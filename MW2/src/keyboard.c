/* The keyboard object is shared with MW2SHELL.DLL (KeyboardReadKeyCode is byte-identical);
   both targets keep their own copy until one source matches both. */
#include "decomp.h"
#include "types.h"

#include <windows.h>

// GLOBAL: MW2 0x100be5d0
undefined4 g_unk0x100be5d0;

// GLOBAL: MW2 0x100be5cc
undefined4 g_unk0x100be5cc;

// GLOBAL: MW2 0x10109a30
undefined2 g_unk0x10109a30[64];

// FUNCTION: MW2 0x100428a9
MechS32 KeyboardReadKeyCode(undefined2* p_keyCode)
{
	if (g_unk0x100be5cc == g_unk0x100be5d0) {
		*p_keyCode = 0;
	}
	else {
		*p_keyCode = g_unk0x10109a30[g_unk0x100be5cc];
		g_unk0x100be5cc++;
		if (g_unk0x100be5cc == 64) {
			g_unk0x100be5cc = 0;
		}
	}

	return 0;
}

// STUB: MW2 0x10042966
void HandleInput(UINT p_msg, WPARAM p_wParam, LPARAM p_lParam)
{
	STUB(0x10042966);
}

// STUB: MW2 0x10042d55
void FUN_10042d55(void)
{
	STUB(0x10042d55);
}
