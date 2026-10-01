#include "unk10011120.h"

#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk100089d0.h"
#include "unk1000a650.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The mission whose briefing g_unk0x1001f2a4 shows.
// GLOBAL: NETMECHW 0x10023948
MechChar g_unk0x10023948[4] = "";

// The briefing's edit control.
// GLOBAL: NETMECHW 0x1001f2a4
HWND g_unk0x1001f2a4;

// GLOBAL: NETMECHW 0x1001f2a8
HWND g_unk0x1001f2a8;

// GLOBAL: NETMECHW 0x1001f2b0
HWND g_unk0x1001f2b0;

// GLOBAL: NETMECHW 0x1001f2b4
HWND g_unk0x1001f2b4;

// GLOBAL: NETMECHW 0x1001f2b8
HWND g_unk0x1001f2b8;

// Shows the game options in a client's dialog p_dialog, if they changed since they were last
// shown, p_force is set or the local player is the host: the briefing of the mission, if it
// changed, the option check boxes and values.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10011120
void FUN_10011120(HWND p_dialog, MechS32 p_force)
{
	MechS32 unused0;
	MechS32 unused1;
	MechS32 unused2;
	CopperField0x4d::Options flags;
	MechChar code[4];
	MechChar text[256];

	unused0 = 0;
	unused1 = 5;
	if (p_force || g_unk0x1001ca90.FUN_100035c0() || g_unk0x1001ca90.m_isHost) {
		EnterCriticalSection(&g_unk0x1001ee60);
		if (p_force) {
			strcpy(g_unk0x10023948, "");
		}

		flags = g_unk0x1001ca90.m_settings.m_options.m_bits;

		if (g_unk0x1001ca90.m_settings.m_unk0x44[0] &&
			_strnicmp(g_unk0x1001ca90.m_settings.m_unk0x44, g_unk0x10023948, 4)) {
			memcpy(g_unk0x10023948, g_unk0x1001ca90.m_settings.m_unk0x44, sizeof(g_unk0x10023948));
			memcpy(code, g_unk0x1001ca90.m_settings.m_unk0x44, sizeof(code));
			FUN_1000a73b(g_unk0x1001f2a4, code);
		}

		CheckDlgButton(p_dialog, 0x421, flags.m_option0);
		CheckDlgButton(p_dialog, 0x422, flags.m_option1);
		CheckDlgButton(p_dialog, 0x423, flags.m_option2);
		CheckDlgButton(p_dialog, 0x424, flags.m_option3);
		CheckDlgButton(p_dialog, 0x425, flags.m_option4);
		CheckDlgButton(p_dialog, 0x426, flags.m_option5);

		g_unk0x1001f2b4 = GetDlgItem(p_dialog, 0x429);
		SetDlgItemText(p_dialog, 0x429, LoadResString(g_unk0x100234f8[g_unk0x1001ca90.m_settings.m_unk0x4c]));
		g_unk0x1001f2a8 = GetDlgItem(p_dialog, 0x42a);
		SetDlgItemText(p_dialog, 0x42a, LoadResString(g_unk0x10023508[g_unk0x1001ca90.m_settings.m_unk0x4a]));
		g_unk0x1001f2b0 = GetDlgItem(p_dialog, 0x433);
		SetDlgItemInt(p_dialog, 0x433, g_unk0x1001ca90.m_settings.m_unk0x49, FALSE);
		g_unk0x1001f2b8 = GetDlgItem(p_dialog, 0x3fb);
		sprintf(text, "%4.2f", g_unk0x1001ca90.m_settings.m_unk0x4b / 16.0);
		SetDlgItemText(p_dialog, 0x3fb, text);

		g_unk0x1001ca90.FUN_10003620();
		LeaveCriticalSection(&g_unk0x1001ee60);
	}
}

// FUNCTION: NETMECHW 0x10011b0b
void FUN_10011b0b(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'C');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');

	if (g_unk0x1001ca90.m_settings.m_unk0x00 == 2) {
		RegisterHotKey(p_hWnd, 6, MOD_ALT, 'T');
	}

	if (g_unk0x1001ca90.m_isHost) {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	}
	else {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'L');
	}

	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x10011bad
void FUN_10011bad(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);

	if (g_unk0x1001ca90.m_settings.m_unk0x00 == 2) {
		UnregisterHotKey(p_hWnd, 6);
	}

	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
