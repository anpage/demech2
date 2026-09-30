#include "unk100079d0.h"

#include "ai.h"
#include "clock.h"
#include "config.h"
#include "decomp.h"
#include "mech.h"
#include "mechsection.h"
#include "network.h"
#include "object.h"
#include "objective.h"
#include "players.h"
#include "playersteering.h"
#include "random.h"
#include "rendertarget.h"
#include "silvertern.h"
#include "simmain.h"
#include "soundfx.h"
#include "speech.h"
#include "types.h"
#include "unk10013430.h"
#include "weaponslot.h"

// A game-key toggle (FUN_1005e9b0's setting 0x3c).
// GLOBAL: MW2 0x100a1590
MechS32 g_unk0x100a1590 = 0;

// The armor per damage level of other players' sections (the local player's: g_unk0x100a1598).
// GLOBAL: MW2 0x100a1594
MechS32 g_unk0x100a1594 = 4;

// GLOBAL: MW2 0x100a1598
MechS32 g_unk0x100a1598 = 4;

// GLOBAL: MW2 0x100a159c
MechS32 g_unk0x100a159c = 0;

// The kill count the cockpit shows in a network game.
// GLOBAL: MW2 0x100a15a0
MechS32 g_unk0x100a15a0 = 0;

// Set while the local player has been warned of critical heat (CalculateHeat).
// GLOBAL: MW2 0x100a15a4
MechS32 g_unk0x100a15a4 = 0;

// When the warning was given.
// GLOBAL: MW2 0x100bdff0
static MechS32 g_unk0x100bdff0;

// Runs the autopilot (m_unk0xbc): mode 1 follows the nav points in order, skipping the ones
// already reached and marking each one it reaches (turning off after the last); then the AI
// steers, the throttle saved while FUN_10015709 has it.
// Stack-slot permutation: index and first.
// FUNCTION: MW2 0x100079d0
void FUN_100079d0(Mech* p_mech)
{
	MechS32 index;
	MechS32 first;

	if (p_mech->m_unk0xbc == 0) {
		return;
	}

	if (p_mech->m_unk0xbc == 1) {
		if (!(p_mech->m_player->m_targetInfo.m_target & 0x100) || (p_mech->m_player->m_targetInfo.m_target & 0x1000)) {
			FUN_100602b2(p_mech->m_player, 0, 0);
			index = p_mech->m_player->m_targetInfo.m_target & 0xff;
			first = index;
			while (g_navTable[index].m_flags & 0x20) {
				FUN_100602b2(p_mech->m_player, 1, 0);
				index = p_mech->m_player->m_targetInfo.m_target & 0xff;
				if (index == first) {
					p_mech->m_player->m_targetInfo.m_target |= 0x1000;
					return;
				}
			}
		}

		if (p_mech->m_player->m_targetInfo.m_target & 0x100) {
			index = p_mech->m_player->m_targetInfo.m_target & 0xff;
			first = index;
			if (g_navTable[index].m_radius > p_mech->m_player->m_targetInfo.m_distance &&
				p_mech->m_player->m_index == g_localPlayerId) {
				if (!(g_navTable[index].m_flags & 0x20)) {
					g_navTable[index].m_flags |= 0x20;
					g_navTable[index].m_unk0x26 |= 1 << p_mech->m_player->m_team;
					if (g_navTable[index].m_flags & 0x40) {
						FUN_1001cdd1();
					}

					FUN_1007eb23(0xe7, 100, 0x40, 5, 0x50);
				}

				FUN_100602b2(p_mech->m_player, 1, 0);
				if ((p_mech->m_player->m_targetInfo.m_target & 0xff) < first) {
					p_mech->m_unk0xbc = 0;
					p_mech->m_player->m_targetInfo.m_target |= 0x1000;
					p_mech->m_player->m_steering->m_throttle = 0;
					p_mech->m_player->m_steering->m_unk0x24 = 1;
					return;
				}
			}
		}
	}

	if (!FUN_10015709(p_mech->m_player)) {
		FUN_1005398f(p_mech->m_player);
		if (p_mech->m_player->m_unk0x184 == 1) {
			p_mech->m_player->m_steering->m_throttle = p_mech->m_player->m_unk0x180;
			p_mech->m_player->m_unk0x184 = 0;
		}

		p_mech->m_player->m_unk0x180 = p_mech->m_player->m_steering->m_throttle;
	}
	else {
		p_mech->m_player->m_unk0x184 = 1;
	}
}

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

// Accumulates the mech's heat each frame (m_deltaHeat less its cooling, doubled while shut down)
// in m_unk0x98, 16.16 percent: overheating (bit 4, above 80) shuts the mech down after 6 seconds
// (state 3), and above 100 the ammunition may explode (with bit 8) or the mech is destroyed after
// 25 seconds; the local player hears the warnings. Cooling below 65 ends the shutdown.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10007e86
void CalculateHeat(Mech* p_mech)
{
	MechS32 delta;
	MechS32 cooling;
	MechS32 heat;
	MechS32 shift;

	shift = 0;
	if ((p_mech->m_player->m_flags & 2) || (p_mech->m_player->m_flags & 4)) {
		return;
	}

	if (g_isNetworkGame && p_mech->m_player->m_index != g_localPlayerId) {
		return;
	}

	if (!g_difficulty->m_heatTracking && p_mech->m_player->m_index == g_localPlayerId) {
		p_mech->m_unk0x98 = 0;
		p_mech->m_deltaHeat = 0;
		return;
	}

	if (p_mech->m_unk0xa0 == 3) {
		shift = 1;
	}

	cooling = p_mech->m_unk0x9c * g_deltaTime << shift;
	delta = p_mech->m_deltaHeat - cooling;
	p_mech->m_unk0x98 += delta;
	if (p_mech->m_unk0x98 < 0) {
		p_mech->m_unk0x98 = 0;
	}

	heat = p_mech->m_unk0x98 >> 16;
	if ((p_mech->m_unk0x10c & 4) && !(p_mech->m_unk0x10c & 8) && p_mech->m_unk0xa0 != 3 &&
		g_currentClock - p_mech->m_unk0x8c > 1086) {
		p_mech->m_unk0xa0 = 3;
		p_mech->m_unk0x8c = g_currentClock;
		if (p_mech->m_player->m_index == g_localPlayerId) {
			PlayCockpitSound(13, -1);
		}
	}

	if (heat > 100) {
		if (p_mech->m_player->m_index == g_localPlayerId && g_difficulty->m_unk0x01) {
			return;
		}

		if (p_mech->m_unk0x10c & 8) {
			if (g_deltaTime && RandomIntBelow(4000 / g_deltaTime) < 3) {
				FUN_10007d06(p_mech->m_player->m_index, p_mech);
			}
		}
		else if (g_currentClock - p_mech->m_unk0x8c > 4525) {
			FUN_1000832b(p_mech->m_player->m_index, p_mech);
		}
	}
	else if (heat > 80.0) {
		if (!(p_mech->m_unk0x10c & 4)) {
			p_mech->m_unk0x8c = g_currentClock;
			p_mech->m_unk0x10c |= 4;
			if (!(p_mech->m_unk0x10c & 0x1000) && p_mech->m_player->m_index == g_localPlayerId) {
				p_mech->m_unk0x10c |= 0x1000;
				PlayCockpitSound(3, -1);
				FUN_1007eb23(0xe9, 100, 0x40, 5, 0x50);
			}
		}
	}
	else if (heat > 65.0) {
		if (p_mech->m_player->m_index == g_localPlayerId && !g_unk0x100a15a4 && delta > 0) {
			g_unk0x100bdff0 = g_currentClock;
			g_unk0x100a15a4 = 1;
			PlayCockpitSound(0, -1);
		}
	}
	else if (p_mech->m_unk0x10c & 4) {
		if (p_mech->m_unk0xa0 == 3 && g_currentClock - p_mech->m_unk0x8c > 724) {
			p_mech->m_unk0x10c &= ~4;
			p_mech->m_unk0xa0 = 0;
		}

		if (p_mech->m_unk0x10c & 8) {
			p_mech->m_unk0x10c &= ~8;
			p_mech->m_unk0x10c &= ~4;
		}
	}
	else {
		if (g_unk0x100a15a4 && g_currentClock - g_unk0x100bdff0 > 724 && p_mech->m_player->m_index == g_localPlayerId) {
			g_unk0x100a15a4 = 0;
		}

		p_mech->m_unk0x10c &= ~0x1000;
	}
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

// Deals p_damage (16.16) to section p_section of the mech, on behalf of player p_attacker: to its
// rear armor with bit 0x8000 (sections 2 to 4), and past the armor to the internal structure,
// which may destroy the section or hit its critical slots. Section 3 stands for a random one of
// 2 to 4. The local player takes none while invulnerable, and in a network game each machine
// only damages its own mech.
// The original ends in an explicit return (the jmp to the epilogue). The only diff is a stack-slot
// permutation of the locals.
// FUNCTION: MW2 0x1000991b
void ApplyDamageToMech(MechS32 p_attacker, Mech* p_mech, MechS32 p_damage, MechS32 p_section)
{
	MechS32 side;
	MechU32 levels;
	MechS32 rear;
	MechS32 i;
	MechS32 level;
	MechS32 front;
	MechS32 roll;
	MechSection* section;

	side = 0;
	front = TRUE;
	rear = FALSE;
	level = 0;
	roll = 0;
	if (p_mech->m_player->m_index == g_localPlayerId && g_difficulty->m_unk0x01) {
		return;
	}

	if (!g_unk0x100a2c10) {
		return;
	}

	if (g_isNetworkGame && p_mech->m_player->m_index != g_localPlayerId) {
		return;
	}

	if (p_damage <= 0) {
		return;
	}

	if (p_section & 0x8000) {
		rear = TRUE;
		p_section &= ~0x8000;
	}

	if (p_section < 1 || p_section > 8) {
		return;
	}

	if (p_section == 3) {
		p_section = RandomIntBelow(3);
		switch (p_section) {
		case 0:
			p_section = 2;
			break;
		case 1:
			p_section = 3;
			break;
		case 2:
			p_section = 4;
			break;
		}
	}

	section = p_mech->m_sections + p_section - 1;
	if (rear && (p_section == 2 || p_section == 4 || p_section == 3)) {
		side = 1;
		front = FALSE;
		levels = (section->m_unk0x26 & 0xf0) >> 4;
	}
	else {
		levels = section->m_unk0x26 & 0xf;
	}

	section->m_armor[side] -= p_damage;
	section->m_unk0x26 |= 0x8000;
	if (section->m_armor[side] <= 0) {
		if (!(section->m_unk0x26 & 0x4000) && p_mech->m_player->m_index == g_localPlayerId && p_mech->m_unk0xa0 == 2) {
			FUN_1007eb23(0xec, 100, 0x40, 5, 0x50);
		}

		section->m_unk0x26 |= 0x4000;
		section->m_unk0x08 += section->m_armor[side];
		section->m_armor[side] = 0;
		if (p_mech->m_player->m_index == g_localPlayerId && (p_section == 1 || p_section == 3) &&
			p_mech->m_unk0xa0 != 4 && p_damage > 0x20000) {
			g_unk0x100ae380 = 1;
		}

		if (section->m_unk0x08 <= 0) {
			FUN_1000899d(p_attacker, p_mech, p_section);
			return;
		}
		else {
			for (i = 0; i < RandomIntBelow(5); i++) {
				roll = RandomIntBelow(100);
				if (roll < 20 && section->m_unk0x24 > 0) {
					if (roll == 12) {
						FUN_1000899d(p_attacker, p_mech, p_section);
						return;
					}
					else {
						FUN_10008c0f(p_attacker, p_mech, p_section, RandomIntBelow(section->m_unk0x24), 0);
					}
				}
			}
		}
	}

	if (levels) {
		if (p_mech->m_player->m_index == g_localPlayerId) {
			level =
				15 - ((section->m_unk0x08 + section->m_armor[side] / g_unk0x100a1598) * 3) / (MechS32) (levels << 16);
		}
		else {
			level =
				15 - ((section->m_unk0x08 + section->m_armor[side] / g_unk0x100a1594) * 3) / (MechS32) (levels << 16);
		}
	}

	FUN_10002246(p_mech->m_player->m_obj, level, p_section);
	return;
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
