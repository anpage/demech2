#include "unk10033280.h"

#include "clock.h"
#include "cobaltharbor.h"
#include "loadres.h"
#include "mech.h"
#include "players.h"
#include "point.h"
#include "rendertarget.h"
#include "screenscale.h"
#include "simmain.h"
#include "types.h"

#include <stdio.h>

// Draws a weapon panel: the name of the local mech's weapon p_panel->m_unk0x0c with its ammo,
// in the color of its state (and the weapon group's, while it is ready), and frames the panel
// in that color if the weapon is selected.
// The selected-weapon comparison loads its operands in the opposite order (one attempt at
// swapping them didn't flip it), and stack-slot permutation: color, font, mech, text and weapon.
// FUNCTION: MW2 0x10033280
void FUN_10033280(CobaltHarbor0x88* p_panel)
{
	Mech* mech;
	MechS32 color;
	void* font;
	MechChar text[64];
	WeaponSlot* weapon;

	if (!p_panel->m_enabled || p_panel->m_unk0x0c < 0) {
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	weapon = &mech->m_weapons[p_panel->m_unk0x0c];
	if (weapon->m_type < 0) {
		return;
	}

	switch (weapon->m_state) {
	case 1:
		switch (weapon->m_group) {
		case 0:
			color = 0xe;
			break;
		case 1:
			color = 0xfe;
			break;
		case 2:
			color = 3;
			break;
		}
		break;
	case 0:
		color = 0xb;
		break;
	case -1:
		color = 8;
		break;
	case 2:
		color = 0xe;
		break;
	default:
		color = 0xb;
		break;
	}

	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (font) {
		g_unk0x100e9350[0xe] = color;
		if (weapon->m_ammo < 0) {
			sprintf(text, "%s", p_panel->m_name);
			FUN_10064f0b(
				p_panel->m_target,
				p_panel->m_unk0x34->m_x,
				p_panel->m_unk0x34->m_y,
				font,
				text,
				g_unk0x100e9350
			);
		}
		else {
			sprintf(text, "%s %d", p_panel->m_name, weapon->m_ammo);
			FUN_10064f0b(
				p_panel->m_target,
				p_panel->m_unk0x34->m_x,
				p_panel->m_unk0x34->m_y,
				font,
				text,
				g_unk0x100e9350
			);
		}

		g_unk0x100e9350[0xe] = 0xe;
		FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
	}

	if (p_panel->m_unk0x0c == mech->m_selectedWeapon) {
		FUN_100570e9(p_panel->m_target, color);
	}
}

// Draws a panel's name once the clock passes p_panel->m_unk0x08, if the local mech has the
// weapon p_panel->m_unk0x0c.
// Stack-slot permutation: clock, font, mech and weapon.
// FUNCTION: MW2 0x100334d3
void FUN_100334d3(CobaltHarbor0x88* p_panel)
{
	Mech* mech;
	WeaponSlot* weapon;
	MechS32 clock;
	void* font;

	if (!p_panel->m_enabled || p_panel->m_unk0x0c < 0) {
		return;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	weapon = &mech->m_weapons[p_panel->m_unk0x0c];
	if (weapon->m_type < 0) {
		return;
	}

	clock = g_currentClock;
	font = FUN_1001a19f(g_unk0x100a8740, g_unk0x100e9614 + 1, g_unk0x100a8684, 0);
	if (!font) {
		return;
	}

	if (p_panel->m_enabled && p_panel->m_unk0x08 < clock) {
		FUN_10064f0b(p_panel->m_target, 0, 0, font, p_panel->m_name, g_unk0x100e9350);
	}

	FUN_1001a163(g_unk0x100e9614 + 1, g_unk0x100a8684);
}
