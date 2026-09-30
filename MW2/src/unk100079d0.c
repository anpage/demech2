#include "unk100079d0.h"

#include "decomp.h"
#include "mech.h"
#include "mechsection.h"
#include "network.h"
#include "object.h"
#include "players.h"
#include "silvertern.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "types.h"
#include "weaponslot.h"

// A game-key toggle (FUN_1005e9b0's setting 0x3c).
// GLOBAL: MW2 0x100a1590
MechS32 g_unk0x100a1590 = 0;

// GLOBAL: MW2 0x100a1598
MechS32 g_unk0x100a1598 = 4;

// GLOBAL: MW2 0x100a159c
MechS32 g_unk0x100a159c = 0;

// The kill count the cockpit shows in a network game.
// GLOBAL: MW2 0x100a15a0
MechS32 g_unk0x100a15a0 = 0;

// Turns the mech's player towards the mech's heading offset (m_unk0x0c) unless the autopilot
// steers it.
// FUNCTION: MW2 0x10007cb5
void FUN_10007cb5(Mech* p_mech)
{
	MechS32 heading;

	if (p_mech->m_unk0xbc == 1) {
		return;
	}

	heading = (p_mech->m_player->m_heading + 0x1680000 + p_mech->m_unk0x04.m_value) % 0x1680000;
	p_mech->m_player->m_targetInfo.m_heading = heading;
}

// Destroys p_mech on behalf of player p_killer, unless g_unk0x100a2c10 is clear: an intact
// section holding an ammunition bin with ammunition left blows up first (FUN_10008c0f) instead.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10007d06
void FUN_10007d06(MechS32 p_killer, Mech* p_mech)
{
	WeaponSlot* weapon;
	MechS32 i;
	MechS32 j;
	MechS32 k;
	MechSection* section;
	SilverTern0x14* bin;

	if (!g_unk0x100a2c10) {
		return;
	}

	if (p_mech->m_unk0x5c) {
		for (i = 0; i < 8; i++) {
			section = &p_mech->m_sections[i];
			if (section->m_unk0x26 & 0x2000) {
				continue;
			}

			for (j = 0; j < section->m_unk0x24; j++) {
				if (section->m_slots[j] > 10000) {
					bin = p_mech->m_unk0x5c;
					for (k = 0; k < p_mech->m_unk0xc8; k++) {
						if (section->m_slots[j] == bin->m_id) {
							weapon = &p_mech->m_weapons[bin->m_weapon];
							if (weapon && weapon->m_ammo > 0) {
								FUN_10008c0f(p_killer, p_mech, i + 1, j, 0);
								return;
							}
						}

						bin++;
					}
				}
			}
		}
	}

	if (p_mech->m_player->m_index == g_localPlayerId) {
		PlayCockpitSound(0, -1);
	}

	FUN_1000832b(p_killer, p_mech);
}

// Destroys p_mech, on behalf of player p_killer (-2: its player left the game).
// STUB: MW2 0x1000832b
void FUN_1000832b(MechS32 p_killer, Mech* p_mech)
{
	STUB(0x1000832b);
}

// Calls FUN_10008c0f once for each of section p_section's m_unk0x24.
// Stack-slot permutation of i, section and count; the loop test compares with i in eax in the
// original (operand order).
// FUNCTION: MW2 0x10008938
void FUN_10008938(MechS32 p_attacker, Mech* p_mech, MechU32 p_section)
{
	MechS32 i;
	MechSection* section;
	MechS32 count;

	section = p_mech->m_sections + p_section - 1;
	count = section->m_unk0x24;
	for (i = 0; i < count; i++) {
		FUN_10008c0f(p_attacker, p_mech, p_section, 0, 1);
	}
}

// Destroys section p_section of p_mech, on behalf of player p_attacker.
// Destroying section 3 takes sections 1, 2 and 4 to 6 with it; section 2 takes 5, and 4 takes 6.
// Of sections 7 and 8, the first to go only marks the mech (0x20); the second destroys 1 and 3.
// FUNCTION: MW2 0x1000899d
void FUN_1000899d(MechS32 p_attacker, Mech* p_mech, MechU32 p_section)
{
	MechSection* section;

	if (!p_section) {
		return;
	}

	section = p_mech->m_sections + p_section - 1;
	if (section->m_unk0x26 & 0x2000) {
		return;
	}

	section->m_unk0x08 = 0;
	section->m_unk0x26 |= 0x2000;
	FUN_10008938(p_attacker, p_mech, p_section);
	switch (p_section) {
	case 2:
		FUN_1000899d(p_attacker, p_mech, 5);
		section->m_unk0x26 &= ~0x2000;
		return;
	case 4:
		FUN_1000899d(p_attacker, p_mech, 6);
		section->m_unk0x26 &= ~0x2000;
		return;
	case 7:
	case 8:
		if (!(p_mech->m_unk0x10c & 0x20)) {
			FUN_10002314(p_mech->m_player->m_obj, p_section);
			p_mech->m_unk0xb0 = 0;
			p_mech->m_unk0x10c |= 0x20;
			return;
		}
		else {
			FUN_1000899d(p_attacker, p_mech, 1);
			FUN_1000899d(p_attacker, p_mech, 3);
		}
		break;
	case 3:
		FUN_1000899d(p_attacker, p_mech, 5);
		FUN_1000899d(p_attacker, p_mech, 2);
		FUN_1000899d(p_attacker, p_mech, 4);
		FUN_1000899d(p_attacker, p_mech, 6);
		FUN_1000899d(p_attacker, p_mech, 1);
		break;
	case 5:
	case 6:
		FUN_10002314(p_mech->m_player->m_obj, p_section);
		return;
	}

	FUN_10002314(p_mech->m_player->m_obj, p_section);
	if (p_mech->m_unk0xa0 != 4 && p_mech->m_unk0xa0 != 5 &&
		(p_mech->m_player->m_index == g_localPlayerId || !g_isNetworkGame)) {
		FUN_1000832b(p_attacker, p_mech);
	}
}

// STUB: MW2 0x10008c0f
void FUN_10008c0f(MechS32 p_attacker, Mech* p_mech, MechU32 p_section, undefined4 p_unk0x0c, undefined4 p_unk0x10)
{
	STUB(0x10008c0f);
}

// Deals p_damage (16.16) to section p_section of the mech, on behalf of player p_attacker.
// STUB: MW2 0x1000991b
void ApplyDamageToMech(MechS32 p_attacker, Mech* p_mech, MechS32 p_damage, MechU32 p_section)
{
	STUB(0x1000991b);
}

// Ejects from p_mech, unless it is already shutting down or ejecting: the local player (p_eject)
// ejects with a sound, or hears that the ejection system is disabled, and the mech is destroyed.
// FUNCTION: MW2 0x10009d2a
void EjectPlayer(Mech* p_mech, MechS32 p_eject)
{
	if (p_mech->m_unk0xa0 == 4 || p_mech->m_unk0xa0 == 5) {
		return;
	}

	if (p_eject && p_mech->m_player->m_index == g_localPlayerId) {
		if (!g_unk0x100ba624) {
			p_mech->m_unk0xa0 = 5;
			FUN_1007eb23(0xc5, 100, 0x40, 5, 0x32);
		}
		else {
			PlayCockpitSound(0x20, -1);
		}
	}

	FUN_1000832b(p_mech->m_player->m_index, p_mech);
	return;
}

// Toggles the local player's object between FUN_100018ca and FUN_10001926.
// FUNCTION: MW2 0x10009dd2
void FUN_10009dd2(void)
{
	if (!g_unk0x100a159c) {
		FUN_100018ca(g_players[g_localPlayerId]->m_obj);
	}
	else {
		FUN_10001926(g_players[g_localPlayerId]->m_obj);
	}

	g_unk0x100a159c = !g_unk0x100a159c;
}
