#include "unk1000d250.h"

#include "decomp.h"
#include "types.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000aa90.h"

#include <windows.h>

// The controls of the player slots in the lobby's dialogs: eight for a free-for-all or team 0,
// then eight for team 1.
// GLOBAL: NETMECHW 0x10023708
MechS32 g_unk0x10023708[16] =
	{0x3f9, 0x3fc, 0x3fd, 0x40d, 0x3ff, 0x401, 0x403, 0x40e, 0x405, 0x407, 0x409, 0x410, 0x40a, 0x40b, 0x40c, 0x412};

// Shows the players' names in the slots of the dialog p_dialog, if the options changed since they
// were last shown or p_force is set.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000d250
void FUN_1000d250(HWND p_dialog, MechS32 p_force)
{
	MechS32 unused;
	MechS32 count;
	MechU8 i;
	MechS32 id;
	NetPlayer player;

	count = 0;
	unused = 0;
	i = 0;
	if (g_unk0x1001ca90.FUN_100035c0() || p_force) {
		while (i < 8) {
			if (FUN_1000afa8(i, &player)) {
				id = g_unk0x10023708[count++];
				SetDlgItemText(p_dialog, id, player.m_name);
			}

			i++;
		}

		while (count < 8) {
			id = g_unk0x10023708[count];
			SetDlgItemText(p_dialog, id, "");
			count++;
		}
	}

	g_unk0x1001ca90.FUN_10003620();
}

// STUB: NETMECHW 0x1000d31f
BOOL CALLBACK FUN_1000d31f(HWND, UINT, WPARAM, LPARAM)
{
	STUB(0x1000d31f);
	return FALSE;
}

// Shows the players' names in the slots of the dialog p_dialog by team, if the options changed
// since they were last shown or p_force is set, and disables control 0x42c when the local
// player is alone on their team.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000d91f
void FUN_1000d91f(HWND p_dialog, MechS32 p_force)
{
	MechS32 count1;
	MechS32 count0;
	MechU8 i;
	MechS32 id;
	NetPlayer player;

	count0 = 0;
	count1 = 0;
	i = 0;
	if (g_unk0x1001ca90.FUN_100035c0() || p_force) {
		EnterCriticalSection(&g_unk0x1001ca78);
		while (i < 8) {
			if (FUN_1000afa8(i, &player)) {
				if (player.m_team == 1) {
					id = g_unk0x10023708[8 + count1++];
				}
				else {
					id = g_unk0x10023708[count0++];
				}

				SetDlgItemText(p_dialog, id, player.m_name);
			}

			i++;
		}

		LeaveCriticalSection(&g_unk0x1001ca78);
		FUN_1000aeff(g_unk0x1001ca90.m_playerId, &player);

		if (player.m_team == 0 && count0 == 1) {
			EnableWindow(GetDlgItem(p_dialog, 0x42c), FALSE);
		}
		else if (player.m_team == 1 && count1 == 1) {
			EnableWindow(GetDlgItem(p_dialog, 0x42c), FALSE);
		}
		else {
			EnableWindow(GetDlgItem(p_dialog, 0x42c), TRUE);
		}

		while (count0 < 8) {
			id = g_unk0x10023708[count0];
			SetDlgItemText(p_dialog, id, "");
			count0++;
		}

		while (count1 < 8) {
			id = g_unk0x10023708[8 + count1];
			SetDlgItemText(p_dialog, id, "");
			count1++;
		}
	}

	g_unk0x1001ca90.FUN_10003620();
}

// STUB: NETMECHW 0x1000dae1
BOOL CALLBACK FUN_1000dae1(HWND, UINT, WPARAM, LPARAM)
{
	STUB(0x1000dae1);
	return FALSE;
}

// FUNCTION: NETMECHW 0x1000e2e2
void FUN_1000e2e2(HWND p_hWnd)
{
	if (g_unk0x10023138[5] == p_hWnd) {
		RegisterHotKey(p_hWnd, 500, MOD_ALT, 'A');
		RegisterHotKey(p_hWnd, 6, MOD_ALT, 'T');
	}

	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'C');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');

	if (g_unk0x1001ca90.m_isHost) {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	}
	else {
		RegisterHotKey(p_hWnd, 1, MOD_ALT, 'L');
	}

	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x1000e396
void FUN_1000e396(HWND p_hWnd)
{
	if (g_unk0x10023138[5] == p_hWnd) {
		UnregisterHotKey(p_hWnd, 500);
		UnregisterHotKey(p_hWnd, 6);
	}

	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);
	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
