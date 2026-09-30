#include "unk1004b5a0.h"

#include "decomp.h"
#include "mech.h"
#include "players.h"
#include "playersteering.h"
#include "random.h"
#include "simmain.h"
#include "soundfx.h"
#include "types.h"
#include "unk1006ca60.h"
#include "weapondef.h"
#include "weapons.h"
#include "weaponslot.h"

// STUB: MW2 0x1004b5a0
void FUN_1004b5a0(Player* p_player, MechS32 p_heading)
{
	STUB(0x1004b5a0);
}

// Decides whether p_player's AI fires its selected weapon: in range of its target, cool enough, the
// weapon ready and loaded, and by chance per recycle time; otherwise it selects the next weapon.
// Firing weapon types 0 and 4 also steers; a guided weapon's volley may lock on.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x1004b724
MechS32 FUN_1004b724(Player* p_player)
{
	MechS32 fire;
	MechS32 type;
	Mech* mech;
	MechS32 i;
	WeaponSlot* slot;
	WeaponDef* def;

	mech = p_player->m_mech;
	for (i = 0, fire = FALSE; i < mech->m_weaponCount && !fire; i++) {
		fire = TRUE;
		slot = &mech->m_weapons[mech->m_selectedWeapon];
		type = slot->m_type;
		def = &g_weaponDefs[type];
		switch (FUN_1006cd45(p_player, def)) {
		case 1:
		case 2:
			break;
		default:
			fire = FALSE;
			break;
		}

		if (fire) {
			fire = FALSE;
			if ((def->m_heat + mech->m_unk0x98) >> 16 < 65.0 && FUN_10045919(mech) == 1 && slot->m_ammo &&
				!RandomIntBelow(def->m_recycle / 90 + 1)) {
				fire = TRUE;
			}
		}

		if (!fire) {
			FUN_10045449(mech, 1);
		}
	}

	if (fire) {
		if (type == 0) {
			p_player->m_steering->m_unk0x00 += 0x1e000;
		}
		else if (type == 4) {
			p_player->m_steering->m_unk0x00 += 0x3c000;
		}
		else if (type == 21) {
		}

		if (g_weaponDefs[type].m_unk0x18) {
			if (p_player->m_unk0x158 <= 4 && !RandomIntBelow(p_player->m_unk0x158 + 1)) {
				mech->m_unk0x10c |= 0x80;
			}

			if (p_player->m_aiGoal == (g_localPlayerId | 0x200)) {
				FUN_1007eb23(0x6f, 100, 0x40, 5, 0x32);
			}
		}
	}

	return fire;
}
