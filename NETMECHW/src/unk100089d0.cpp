#include "unk100089d0.h"

#include "decomp.h"
#include "types.h"
#include "unk10001070.h"
#include "unk10003660.h"
#include "unk10006b20.h"
#include "unk1000a650.h"
#include "unk1000aa90.h"
#include "unk1000d250.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// GLOBAL: NETMECHW 0x1001ef90
HWND g_unk0x1001ef90;

// GLOBAL: NETMECHW 0x1001ef98
HWND g_unk0x1001ef98;

// GLOBAL: NETMECHW 0x1001ef9c
HWND g_unk0x1001ef9c;

// GLOBAL: NETMECHW 0x1001efa0
HWND g_unk0x1001efa0;

// Shows the game options in the host's dialog p_dialog, if they changed since they were last
// shown or p_force is set: the mission's name, the players' names, the option check boxes and
// values.
// Matches except for the stack slots of the locals, which VC++ 2.2 permutes.
// FUNCTION: NETMECHW 0x100089d0
void FUN_100089d0(HWND p_dialog, MechS32 p_force)
{
	MechS32 unused0;
	MechS32 unused1;
	MechS32 unused2;
	CopperField0x4d::Options flags;
	MechChar code[4];
	MechChar name[12];
	MechChar* missionName;
	MechS32 length;
	MechS32 i;
	MechS32 count;
	NetPlayer player;
	MechS32 id;
	MechChar text[256];

	unused0 = 0;
	unused1 = 5;
	if (p_force || g_unk0x1001ca90.FUN_100035c0()) {
		EnterCriticalSection(&g_unk0x1001ee60);
		flags = g_unk0x1001ca90.m_settings.m_options.m_bits;

		if (g_unk0x1001ca90.m_settings.m_unk0x44[0]) {
			memcpy(code, g_unk0x1001ca90.m_settings.m_unk0x44, sizeof(code));
			sprintf(name, "%.4sNAME", code);
			missionName = LoadMissionText(name);
			length = strlen(missionName);
			missionName[length - 2] = '\0';
			SetDlgItemText(p_dialog, 0x41b, missionName);
			free(missionName);
		}

		count = i = 0;
		EnterCriticalSection(&g_unk0x1001ca78);
		while (i < 8) {
			if (FUN_1000afa8(i, &player)) {
				id = g_unk0x10023708[count++];
				SetDlgItemText(p_dialog, id, player.m_name);
			}

			i++;
		}

		LeaveCriticalSection(&g_unk0x1001ca78);
		while (count < 8) {
			id = g_unk0x10023708[count];
			SetDlgItemText(p_dialog, id, "");
			count++;
		}

		CheckDlgButton(p_dialog, 0x421, flags.m_option0);
		CheckDlgButton(p_dialog, 0x422, flags.m_option1);
		CheckDlgButton(p_dialog, 0x423, flags.m_option2);
		CheckDlgButton(p_dialog, 0x424, flags.m_option3);
		CheckDlgButton(p_dialog, 0x425, flags.m_option4);
		CheckDlgButton(p_dialog, 0x426, flags.m_option5);

		g_unk0x1001ef9c = GetDlgItem(p_dialog, 0x429);
		SetDlgItemText(p_dialog, 0x429, LoadResString(g_unk0x100234f8[g_unk0x1001ca90.m_settings.m_unk0x4c]));
		g_unk0x1001ef90 = GetDlgItem(p_dialog, 0x42a);
		SetDlgItemText(p_dialog, 0x42a, LoadResString(g_unk0x10023508[g_unk0x1001ca90.m_settings.m_unk0x4a]));
		g_unk0x1001ef98 = GetDlgItem(p_dialog, 0x433);
		SetDlgItemInt(p_dialog, 0x433, g_unk0x1001ca90.m_settings.m_unk0x49, FALSE);
		g_unk0x1001efa0 = GetDlgItem(p_dialog, 0x3fb);
		sprintf(text, "%4.2f", g_unk0x1001ca90.m_settings.m_unk0x4b / 16.0);
		SetDlgItemText(p_dialog, 0x3fb, text);

		g_unk0x1001ca90.FUN_10003620();
		LeaveCriticalSection(&g_unk0x1001ee60);
	}
}

// The string IDs of the values of m_settings.m_unk0x4c.
// GLOBAL: NETMECHW 0x100234f8
UINT g_unk0x100234f8[4] = {0x71, 0x72, 0x73, 0x74};

// The string IDs of the values of m_settings.m_unk0x4a.
// GLOBAL: NETMECHW 0x10023508
UINT g_unk0x10023508[3] = {0x6e, 0x6f, 0x70};

// FUNCTION: NETMECHW 0x10009183
void FUN_10009183(HWND p_hWnd)
{
	RegisterHotKey(p_hWnd, 4, MOD_ALT, 'C');
	RegisterHotKey(p_hWnd, 5, MOD_ALT, 'S');
	RegisterHotKey(p_hWnd, 1, MOD_ALT, 'B');
	RegisterHotKey(p_hWnd, 2, MOD_ALT, 'H');
	RegisterHotKey(p_hWnd, 3, MOD_ALT, 'Q');
}

// FUNCTION: NETMECHW 0x100091e3
void FUN_100091e3(HWND p_hWnd)
{
	UnregisterHotKey(p_hWnd, 4);
	UnregisterHotKey(p_hWnd, 5);
	UnregisterHotKey(p_hWnd, 1);
	UnregisterHotKey(p_hWnd, 2);
	UnregisterHotKey(p_hWnd, 3);
}
