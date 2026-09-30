/* Menu actions (the eject menu) and the "dorcs" sequence's data files. */
#include "unk10073af0.h"

#include "config.h"
#include "gamekeys.h"
#include "menu.h"
#include "menupage.h"
#include "mss.h"
#include "players.h"
#include "simmain.h"
#include "soundfx.h"
#include "types.h"
#include "unk100079d0.h"

#include <stdio.h>

// A menu item's action: ejects the local player (game key 0x3b) without its sound.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10073af0
void FUN_10073af0(
	undefined4 p_unk0x00,
	undefined4 p_unk0x04,
	MechS32 p_index,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	MenuPage* p_page
)
{
	MechS32 saved;
	MechS32 digit;
	MechS32 selected;

	if (!p_page) {
		return;
	}

	digit = g_menuKey - '1' == p_index;
	selected = p_page->m_selected == p_index;
	if ((selected && g_menuKey == '\r') || digit) {
		g_difficulty->m_unk0x01 = 0;
		saved = g_unk0x100ba624;
		g_unk0x100ba624 = 0;
		FUN_1005c78a(0x3b);
		g_unk0x100ba624 = saved;
		FreeMenus();
	}
}

// A menu item's action: ejects the local player's mech.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10073ba6
void FUN_10073ba6(
	undefined4 p_unk0x00,
	undefined4 p_unk0x04,
	MechS32 p_index,
	undefined4 p_unk0x0c,
	undefined4 p_unk0x10,
	MenuPage* p_page
)
{
	MechS32 digit;
	MechS32 selected;
	Player* player;

	if (!p_page) {
		return;
	}

	digit = g_menuKey - '1' == p_index;
	selected = p_page->m_selected == p_index;
	if ((selected && g_menuKey == '\r') || digit) {
		g_unk0x100b1350 = 1;
		g_difficulty->m_unk0x01 = 0;
		player = g_players[g_localPlayerId];
		EjectPlayer(player->m_mech, 0);
		FreeMenus();
	}
}

// Reads vfx/<p_name>.bin whole.
// FUNCTION: MW2 0x10073c62
void* ReadVfxBin(MechChar* p_name)
{
	void* data;
	MechChar path[256];

	sprintf(path, "%s/%s.%s", "vfx", p_name, "bin");
	data = FILE_read(path, NULL);
	return data;
}

// FUNCTION: MW2 0x10073cb5
void FUN_10073cb5(void)
{
	RequestMenuClose(6);
	RequestMenuClose(2);
	RequestMenuClose(4);
	RequestMenuClose(7);
	RequestMenuClose(8);
	RequestMenuClose(5);
}
