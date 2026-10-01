#include "unk1000e410.h"

#include "decomp.h"
#include "types.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000aa90.h"
#include "unk1000d250.h"

#include <windows.h>

// The ready check boxes of the player slots of g_unk0x10023708.
// GLOBAL: NETMECHW 0x10023758
MechS32 g_unk0x10023758[8] = {0x7d1, 0x7d2, 0x7da, 0x7db, 0x7dc, 0x7dd, 0x7de, 0x7df};

// Shows the players' names and ready check boxes in the slots of the dialog p_dialog, if the
// options changed since they were last shown, p_force is set or the local player is the host.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x1000e410
void FUN_1000e410(HWND p_dialog, MechS32 p_force)
{
	MechS32 count;
	MechS32 unused0;
	MechS32 unused1;
	MechS32 i;
	MechS32 id;
	NetPlayer player;

	unused0 = 0;
	unused1 = 5;
	if (p_force || g_unk0x1001ca90.FUN_100035c0() || g_unk0x1001ca90.m_isHost) {
		EnterCriticalSection(&g_unk0x1001ee60);
		if (g_unk0x1001ca90.m_settings.m_unk0x44[0]) {
			count = i = 0;
			EnterCriticalSection(&g_unk0x1001ca78);
			while (i < 8) {
				if (FUN_1000afa8(i, &player)) {
					id = g_unk0x10023708[count];
					SetDlgItemText(p_dialog, id, player.m_name);

					if ((1 << i) & g_unk0x1001ca90.m_settings.m_unk0x02) {
						CheckDlgButton(p_dialog, g_unk0x10023758[count], 1);
					}
					else {
						CheckDlgButton(p_dialog, g_unk0x10023758[count], 0);
					}

					count++;
				}

				i++;
			}

			LeaveCriticalSection(&g_unk0x1001ca78);
			while (count < 8) {
				id = g_unk0x10023708[count];
				SetDlgItemText(p_dialog, id, "");
				CheckDlgButton(p_dialog, g_unk0x10023758[count], 0);
				count++;
			}
		}

		g_unk0x1001ca90.FUN_10003620();
		LeaveCriticalSection(&g_unk0x1001ee60);
	}
}

// STUB: NETMECHW 0x1000e58d
BOOL CALLBACK FUN_1000e58d(HWND, UINT, WPARAM, LPARAM)
{
	STUB(0x1000e58d);
	return FALSE;
}

// FUNCTION: NETMECHW 0x1000ef3d
void FUN_1000ef3d(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'U');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');

	if (g_unk0x1001ca90.m_settings.m_unk0x00 == 2) {
		RegisterHotKey(p_hWnd, 6, MOD_ALT, 'T');
	}

	RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	RegisterHotKey(p_hWnd, 7, MOD_ALT, 'A');
	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x1000efcd
void FUN_1000efcd(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);

	if (g_unk0x1001ca90.m_settings.m_unk0x00 == 2) {
		UnregisterHotKey(p_hWnd, 6);
	}

	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 7);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
