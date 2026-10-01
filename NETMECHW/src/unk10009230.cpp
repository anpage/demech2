#include "unk10009230.h"

#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk100089d0.h"
#include "unk1000a650.h"

#include <commctrl.h>
#include <stdio.h>
#include <windows.h>

// The host's options pane: the mission lists, the option check boxes, and the values.

// The time limit shown in control 0x3fb, in units of 16 of m_settings.m_unk0x4b.
// GLOBAL: NETMECHW 0x10023518
double g_unk0x10023518 = 1.0;

// The list box of the missions (control 0x41c).
// GLOBAL: NETMECHW 0x1001efac
HWND g_unk0x1001efac;

// The briefing of the selected mission (control 0x41b).
// GLOBAL: NETMECHW 0x1001efb0
HWND g_unk0x1001efb0;

// The combo box of m_settings.m_unk0x4a (control 0x42a).
// GLOBAL: NETMECHW 0x1001efb4
HWND g_unk0x1001efb4;

// The up-down control of control 0x3fb (control 0x428).
// GLOBAL: NETMECHW 0x1001efb8
HWND g_unk0x1001efb8;

// GLOBAL: NETMECHW 0x1001efbc
HWND g_unk0x1001efbc;

// The combo box of m_settings.m_unk0x4c (control 0x429).
// GLOBAL: NETMECHW 0x1001efc0
HWND g_unk0x1001efc0;

// GLOBAL: NETMECHW 0x1001efc4
HWND g_unk0x1001efc4;

// GLOBAL: NETMECHW 0x1001efc8
HWND g_unk0x1001efc8;

// The up-down control of m_settings.m_unk0x49 (control 0x427).
// GLOBAL: NETMECHW 0x1001efcc
HWND g_unk0x1001efcc;

// Sets up the options pane p_dialog: fills the mission lists and shows the game options.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x10009230
void FUN_10009230(HWND p_dialog)
{
	CopperField0x4d::Options flags;
	MechChar text[8];
	MechS32 i;
	LRESULT index;
	LRESULT data;

	EnterCriticalSection(&g_unk0x1001ee60);
	flags = g_unk0x1001ca90.m_settings.m_options.m_bits;

	g_unk0x1001efac = GetDlgItem(p_dialog, 0x41c);
	g_unk0x1001efc8 = GetDlgItem(p_dialog, 0x41e);
	g_unk0x1001efb0 = GetDlgItem(p_dialog, 0x41b);
	FUN_1000a650(g_unk0x1001efac, "MISSIONT");
	FUN_1000a650(g_unk0x1001efc8, "MISSIONF");

	SendMessage(g_unk0x1001efac, LB_SETCURSEL, 0, 0);
	index = SendMessage(g_unk0x1001efac, LB_GETCURSEL, 0, 0);
	data = SendMessage(g_unk0x1001efac, LB_GETITEMDATA, index, 0);
	FUN_1000a73b(g_unk0x1001efb0, (MechChar*) data);

	CheckDlgButton(p_dialog, 0x421, flags.m_option0);
	CheckDlgButton(p_dialog, 0x422, flags.m_option1);
	CheckDlgButton(p_dialog, 0x423, flags.m_option2);
	CheckDlgButton(p_dialog, 0x424, flags.m_option3);
	CheckDlgButton(p_dialog, 0x425, flags.m_option4);
	CheckDlgButton(p_dialog, 0x426, flags.m_option5);

	g_unk0x1001efc0 = GetDlgItem(p_dialog, 0x429);
	for (i = 0; i < 4; i++) {
		index = SendMessage(g_unk0x1001efc0, CB_ADDSTRING, 0, (LPARAM) LoadResString(g_unk0x100234f8[i]));
		SendMessage(g_unk0x1001efc0, CB_SETITEMDATA, i, i);
	}

	g_unk0x1001efb4 = GetDlgItem(p_dialog, 0x42a);
	for (i = 0; i < 3; i++) {
		index = SendMessage(g_unk0x1001efb4, CB_ADDSTRING, 0, (LPARAM) LoadResString(g_unk0x10023508[i]));
		SendMessage(g_unk0x1001efb4, CB_SETITEMDATA, i, i);
	}

	g_unk0x1001efbc = GetDlgItem(p_dialog, 0x3f8);
	g_unk0x1001efcc = GetDlgItem(p_dialog, 0x427);
	SendMessage(g_unk0x1001efcc, UDM_SETBUDDY, (WPARAM) GetDlgItem(p_dialog, 0x3f8), 0);
	SendMessage(g_unk0x1001efcc, UDM_SETRANGE, 0, MAKELONG(100, 25));
	SendMessage(g_unk0x1001efcc, UDM_SETPOS, 0, 1);
	SetDlgItemInt(p_dialog, 0x3f8, g_unk0x1001ca90.m_settings.m_unk0x49, FALSE);

	g_unk0x1001efc4 = GetDlgItem(p_dialog, 0x3fb);
	g_unk0x1001efb8 = GetDlgItem(p_dialog, 0x428);
	SendMessage(g_unk0x1001efb8, UDM_SETBUDDY, (WPARAM) g_unk0x1001efc4, 0);
	SendMessage(g_unk0x1001efb8, UDM_SETRANGE, 0, MAKELONG(4, 0));
	SendMessage(g_unk0x1001efb8, UDM_SETPOS, 0, 1);

	g_unk0x10023518 = (MechU32) g_unk0x1001ca90.m_settings.m_unk0x4b / 16.0;
	sprintf(text, "%4.2f", g_unk0x10023518);
	SetDlgItemText(p_dialog, 0x3fb, text);

	LeaveCriticalSection(&g_unk0x1001ee60);
}

// Takes the value of the control p_control of the options pane p_dialog into the game options;
// one of the option check boxes takes them all.
// FUNCTION: NETMECHW 0x10009615
MechS32 FUN_10009615(HWND p_dialog, MechS32 p_control)
{
	CopperField0x4d::Options flags;
	BOOL translated;
	LRESULT index;

	EnterCriticalSection(&g_unk0x1001ee60);
	switch (p_control) {
	case 0x3fb:
		g_unk0x1001ca90.m_settings.m_unk0x4b = (MechU8) (g_unk0x10023518 * 16.0);
		break;
	case 0x421:
	case 0x422:
	case 0x423:
	case 0x424:
	case 0x425:
	case 0x426:
		flags.m_option0 = IsDlgButtonChecked(p_dialog, 0x421);
		flags.m_option1 = IsDlgButtonChecked(p_dialog, 0x422);
		flags.m_option2 = IsDlgButtonChecked(p_dialog, 0x423);
		flags.m_option3 = IsDlgButtonChecked(p_dialog, 0x424);
		flags.m_option4 = IsDlgButtonChecked(p_dialog, 0x425);
		flags.m_option5 = IsDlgButtonChecked(p_dialog, 0x426);
		flags.m_option6 = 0;
		flags.m_option7 = 0;
		g_unk0x1001ca90.m_settings.m_options.m_bits = flags;
		break;
	case 0x3f8:
		g_unk0x1001ca90.m_settings.m_unk0x49 = GetDlgItemInt(p_dialog, 0x3f8, &translated, FALSE);
		break;
	case 0x42a:
		index = SendMessage(g_unk0x1001efb4, CB_GETCURSEL, 0, 0);
		g_unk0x1001ca90.m_settings.m_unk0x4a = (MechU8) SendMessage(g_unk0x1001efb4, CB_GETITEMDATA, index, 0);
		break;
	case 0x429:
		index = SendMessage(g_unk0x1001efc0, CB_GETCURSEL, 0, 0);
		g_unk0x1001ca90.m_settings.m_unk0x4c = (MechU8) SendMessage(g_unk0x1001efc0, CB_GETITEMDATA, index, 0);
		break;
	}

	LeaveCriticalSection(&g_unk0x1001ee60);
	return TRUE;
}

// Takes all the values of the options pane p_dialog into the game options.
// FUNCTION: NETMECHW 0x10009838
MechS32 FUN_10009838(HWND p_dialog)
{
	FUN_10009615(p_dialog, 0x3fb);
	FUN_10009615(p_dialog, 0x421);
	FUN_10009615(p_dialog, 0x3f8);
	FUN_10009615(p_dialog, 0x42a);
	FUN_10009615(p_dialog, 0x429);
	return TRUE;
}

// STUB: NETMECHW 0x100098a2
BOOL CALLBACK FUN_100098a2(HWND, UINT, WPARAM, LPARAM)
{
	STUB(0x100098a2);
	return FALSE;
}

// FUNCTION: NETMECHW 0x1000a4f6
void FUN_1000a4f6(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 300, MOD_ALT, 'T');
	RegisterHotKey(p_hWnd, 301, MOD_ALT, 'F');
	RegisterHotKey(p_hWnd, 302, MOD_ALT, 'O');
	RegisterHotKey(p_hWnd, 303, MOD_ALT, 'A');
	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'C');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');

	if (g_unk0x1001ca90.m_unk0x00 == 2) {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'N');
	}
	else {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	}

	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x1000a5c4
void FUN_1000a5c4(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 300);
	UnregisterHotKey(p_hWnd, 301);
	UnregisterHotKey(p_hWnd, 302);
	UnregisterHotKey(p_hWnd, 303);
	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);
	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
