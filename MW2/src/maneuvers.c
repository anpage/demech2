#include "maneuvers.h"

#include "ai.h"
#include "aiweapons.h"
#include "approxlen.h"
#include "clock.h"
#include "collision.h"
#include "decomp.h"
#include "fixeddiv.h"
#include "fixedmul.h"
#include "geocache.h"
#include "hud.h"
#include "mech.h"
#include "mw2log.h"
#include "object.h"
#include "players.h"
#include "playersteering.h"
#include "point.h"
#include "random.h"
#include "ray.h"
#include "rendertarget.h"
#include "shape.h"
#include "silverbrook.h"
#include "simmain.h"
#include "transform.h"
#include "types.h"
#include "view.h"
#include "weapondata.h"
#include "weapons.h"
#include "weaponslot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Sixteen directions around a mech, as (x, z) steps: FUN_10015b9f's probe rays.
// GLOBAL: MW2 0x100a2900
Point g_unk0x100a2900[16] = {
	{0, 1},
	{2, 1},
	{1, 1},
	{1, 2},
	{1, 0},
	{1, -2},
	{1, -1},
	{2, -1},
	{0, -1},
	{-2, -1},
	{-1, -1},
	{-1, -2},
	{-1, 0},
	{-1, 2},
	{-1, 1},
	{-2, 1}
};

// The maneuver tables: the maneuvers a mech chooses among and those that may follow each.

// GLOBAL: MW2 0x100a2980
AmberGlade0x12 g_unk0x100a2980[13] = {
	{{0, 6, 0, 1, 2, 3, 4, 4, 0}},
	{{1, 5, 0, 1, 2, 3, 4, 0, 0}},
	{{2, 3, 7, 8, 4, 0, 0, 0, 0}},
	{{3, 2, 7, 8, 0, 0, 0, 0, 0}},
	{{4, 3, 5, 5, 8, 0, 0, 0, 0}},
	{{5, 2, 7, 8, 0, 0, 0, 0, 0}},
	{{6, 1, 6, 0, 0, 0, 0, 0, 0}},
	{{7, 3, 0, 1, 8, 0, 0, 0, 0}},
	{{8, 5, 0, 1, 2, 3, 4, 0, 0}},
	{{9, 3, 0, 8, 7, 0, 0, 0, 0}},
	{{10, 3, 7, 7, 4, 0, 0, 0, 0}},
	{{11, 2, 3, 1, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0}},
};

// GLOBAL: MW2 0x100a2a70
AmberGlade0x12 g_unk0x100a2a70 = {{0, 1, 0, 0, 0, 0, 0, 0, 0}};

// GLOBAL: MW2 0x100a2a88
AmberGlade0x12 g_unk0x100a2a88 = {{12, 1, 12, 0, 0, 0, 0, 0, 0}};

// GLOBAL: MW2 0x100a2aa0
AmberGlade0x12 g_unk0x100a2aa0 = {{1, 1, 1, 0, 0, 0, 0, 0, 0}};

// The table of a mech whose class sets m_unk0xe4 (FUN_10013d81).
// GLOBAL: MW2 0x100a2ab8
AmberGlade0x12 g_unk0x100a2ab8[13] = {
	{{0, 7, 0, 1, 2, 3, 4, 4, 4}},
	{{1, 6, 0, 1, 2, 3, 4, 4, 0}},
	{{2, 4, 7, 8, 4, 4, 0, 0, 0}},
	{{3, 4, 7, 8, 4, 4, 0, 0, 0}},
	{{4, 3, 5, 5, 8, 0, 0, 0, 0}},
	{{5, 2, 7, 8, 0, 0, 0, 0, 0}},
	{{6, 1, 6, 0, 0, 0, 0, 0, 0}},
	{{7, 5, 0, 1, 2, 8, 4, 0, 0}},
	{{8, 6, 0, 1, 2, 3, 4, 4, 0}},
	{{9, 3, 0, 8, 7, 0, 0, 0, 0}},
	{{10, 4, 7, 7, 4, 4, 0, 0, 0}},
	{{11, 1, 3, 0, 0, 0, 0, 0, 0}},
	{{0, 0, 0, 0, 0, 0, 0, 0, 0}},
};

// Set once FUN_100139e9 has filled g_unk0x101748e0.
// GLOBAL: MW2 0x100a2ba4
MechS32 g_unk0x100a2ba4 = 0;

// Set from the world stream's planet record when positive.

// GLOBAL: MW2 0x100a2bdc
MechS32 g_unk0x100a2bdc = 100000;

// GLOBAL: MW2 0x100a2be0
MechS32 g_unk0x100a2be0 = 0x2000;

// The maneuver table of each player type (m_unk0x00, 1 to 8), and in entry 8 the alternative to
// the first.
// GLOBAL: MW2 0x101748e0
SilverBrook0x08 g_unk0x101748e0[9];

// Runs p_player's current maneuver (m_unk0x170) against p_target for a tick, and ends it when it
// is done, when the mech must jump or turn away (m_unk0x174) or when its time is up; with no
// maneuver, chooses and starts one.
// Stack-slot permutation: done, mech and leader.
// FUNCTION: MW2 0x10013430
void FUN_10013430(Player* p_player, MechU16 p_target)
{
	MechS32 done;
	Mech* mech;
	Player* leader;

	done = FALSE;
	mech = p_player->m_mech;
	if (p_player->m_unk0x170 != -1) {
		switch (p_player->m_unk0x170) {
		case 0:
			FUN_100155e1(p_player);
			FUN_100149e7(p_player, p_target);
			break;
		case 1:
			if (p_player->m_unk0x17c <= g_currentClock) {
				FUN_10054778(p_player);
				leader = g_players[p_player->m_ai.m_goal & 0xff];
				if (p_player->m_unk0x180) {
					leader->m_unk0x1a2[p_player->m_unk0x180 / 2]--;
				}

				p_player->m_unk0x17c = g_currentClock + 543;
				if (p_player->m_unk0x184 == -1) {
					p_player->m_unk0x180 = FUN_100166b1(p_player);
					p_player->m_unk0x184 = 0;
				}

				FUN_10014723(p_player, p_player->m_ai.m_goal, p_player->m_unk0x180, 15000);
				leader->m_unk0x1a2[p_player->m_unk0x180 / 2]++;
			}

			FUN_100155e1(p_player);
			FUN_10014aa8(p_player, p_target);
			break;
		case 2:
			FUN_100155e1(p_player);
			if (FUN_10014c3d(p_player, p_target)) {
				done = TRUE;
			}
			break;
		case 3:
			FUN_100155e1(p_player);
			if (FUN_10014c3d(p_player, p_target)) {
				done = TRUE;
			}

			if (!FUN_10015709(p_player)) {
				p_player->m_steering->m_turn += p_player->m_unk0x180 * 0x1c20000;
			}

			if (p_player->m_unk0x17c <= g_currentClock) {
				p_player->m_unk0x180 = -p_player->m_unk0x180;
				p_player->m_unk0x17c = g_currentClock + 543;
			}
			break;
		case 4:
			if (FUN_100150c1(p_player, p_target)) {
				done = TRUE;
			}
			break;
		case 11:
			if (FUN_1001512e(p_player, p_target)) {
				done = TRUE;
			}
			break;
		case 5:
			if (FUN_10015172(p_player, p_target) || p_player->m_mech->m_unk0xc0 <= 0) {
				done = TRUE;
			}
			break;
		case 6:
			if (FUN_10014d4e(p_player, p_target)) {
				done = TRUE;
			}
			break;
		case 7:
			FUN_100155e1(p_player);
			if (FUN_10014e5e(p_player, p_target)) {
				done = TRUE;
			}
			break;
		case 8:
			FUN_100155e1(p_player);
			if (FUN_10014df1(p_player, p_target)) {
				done = TRUE;
			}
			break;
		case 9:
			if (FUN_10014f23(p_player, p_target)) {
				done = TRUE;
			}
			break;
		case 10:
			FUN_10015342(p_player, p_target);
			break;
		case 12:
			FUN_100155e1(p_player);
			if (FUN_100153e6(p_player, p_target)) {
				done = TRUE;
			}
			break;
		}

		if (mech->m_unk0x88 && FUN_10015fa8(mech) && !p_player->m_unk0x196 && p_player->m_unk0x170 != 6 &&
			p_player->m_ai.m_state != c_aiStateFlee) {
			if (RandomIntBelow(4) && p_player->m_unk0x172 != 5 && FUN_10016222(p_player, 40)) {
				p_player->m_unk0x174 = 4;
				p_player->m_unk0x184 = 1;
			}
			else {
				if (RandomIntBelow(2)) {
					p_player->m_unk0x174 = 6;
				}
				else {
					p_player->m_unk0x174 = -2;
				}

				if (!p_player->m_skillFlag5) {
					p_player->m_unk0x174 = -2;
				}
			}
		}

		if (p_player->m_unk0x00 == 1 && FUN_10016880(mech)) {
			done = TRUE;
		}

		if (p_player->m_unk0x178 && p_player->m_unk0x178 <= g_currentClock) {
			done = TRUE;
		}

		if (p_player->m_unk0x174) {
			done = TRUE;
		}

		if (done) {
			FUN_1001450e(p_player);
		}
	}
	else {
		p_player->m_unk0x170 = FUN_10013d81(p_player);
		if (p_player->m_unk0x170 != -1) {
			FUN_10014149(p_player);
		}
	}
}

// Resets p_player's maneuver state and sets the maneuvers its skill (m_unk0x159, 1 to 4) allows,
// and fills the maneuver tables the first time.
// FUNCTION: MW2 0x100139e9
void FUN_100139e9(Player* p_player)
{
	MechS16 i;

	p_player->m_unk0x170 = -1;
	p_player->m_unk0x172 = -1;
	p_player->m_unk0x174 = 0;
	p_player->m_unk0x188 = NULL;
	p_player->m_unk0x190 = 0;
	p_player->m_unk0x192 = 1;
	p_player->m_unk0x18c = 0;
	memset(p_player->m_unk0x1a2, 0, sizeof(p_player->m_unk0x1a2));
	if (p_player->m_unk0x159 < 1 || p_player->m_unk0x159 > 4) {
		p_player->m_unk0x159 = 1;
	}

	if (p_player->m_unk0x159 <= 3) {
		p_player->m_skillFlag0 = 1;
	}
	else {
		p_player->m_skillFlag0 = 0;
	}

	if (p_player->m_unk0x159 <= 3) {
		p_player->m_skillFlag1 = 1;
	}
	else {
		p_player->m_skillFlag1 = 0;
	}

	if (p_player->m_unk0x159 <= 2) {
		p_player->m_skillFlag2 = 1;
	}
	else {
		p_player->m_skillFlag2 = 0;
	}

	if (p_player->m_unk0x00 == 1 && p_player->m_unk0x159 <= 5) {
		p_player->m_skillFlag3 = 1;
	}
	else {
		p_player->m_skillFlag3 = 0;
	}

	if (p_player->m_unk0x159 <= 4) {
		p_player->m_skillFlag4 = 1;
	}
	else {
		p_player->m_skillFlag4 = 0;
	}

	if (p_player->m_unk0x00 == 1 && p_player->m_unk0x159 <= 1) {
		p_player->m_skillFlag5 = 1;
	}
	else {
		p_player->m_skillFlag5 = 0;
	}

	if (p_player->m_unk0x159 <= 5) {
		p_player->m_skillFlag6 = 1;
	}
	else {
		p_player->m_skillFlag6 = 0;
	}

	if (!g_unk0x100a2ba4) {
		for (i = 0; i < 8; i++) {
			switch (i + 1) {
			case 1:
				g_unk0x101748e0[i].m_count = 13;
				g_unk0x101748e0[i].m_unk0x02 = 7;
				g_unk0x101748e0[i].m_entries = g_unk0x100a2980;
				g_unk0x101748e0[8] = g_unk0x101748e0[i];
				g_unk0x101748e0[8].m_entries = g_unk0x100a2ab8;
				break;
			case 8:
				g_unk0x101748e0[i].m_count = 1;
				g_unk0x101748e0[i].m_unk0x02 = 0;
				g_unk0x101748e0[i].m_entries = &g_unk0x100a2aa0;
				break;
			case 5:
				g_unk0x101748e0[i].m_count = 1;
				g_unk0x101748e0[i].m_unk0x02 = 0;
				g_unk0x101748e0[i].m_entries = &g_unk0x100a2a88;
				break;
			default:
				g_unk0x101748e0[i].m_count = 1;
				g_unk0x101748e0[i].m_unk0x02 = 0;
				g_unk0x101748e0[i].m_entries = &g_unk0x100a2a70;
				break;
			}
		}

		g_unk0x100a2ba4 = 1;
	}
}

// Picks p_player's next maneuver: one an order asked for, an AI player's escape, attack or chase
// when it applies, or else one its maneuver table lets follow the previous one (any one at
// first), redrawn until its conditions hold.
// Stack-slot permutation: mech, table, choice and index.
// FUNCTION: MW2 0x10013d81
MechS32 FUN_10013d81(Player* p_player)
{
	Mech* mech;
	SilverBrook0x08* table;
	MechS16 choice;
	MechS16 index;

	choice = -1;
	table = &g_unk0x101748e0[p_player->m_unk0x00 - 1];
	mech = p_player->m_mech;
	if (mech->m_unk0xe4 == 1) {
		table = &g_unk0x101748e0[8];
	}

	do {
		FUN_1005372c(p_player, p_player->m_ai.m_target);
		if (p_player->m_unk0x174) {
			if (p_player->m_unk0x174 == -2) {
				FUN_10054684(p_player, 4, p_player->m_ai.m_goal, 0);
				p_player->m_ai.m_flags = 1;
			}
			else {
				choice = p_player->m_unk0x174;
			}

			p_player->m_unk0x174 = 0;
			break;
		}

		if (p_player->m_unk0x00 == 1) {
			if (FUN_10016880(mech)) {
				choice = 10;
				break;
			}

			if (!(p_player->m_ai.m_goal & 0x200)) {
				choice = 0;
				break;
			}

			if (FUN_10016222(p_player, 0x14) && p_player->m_skillFlag1 && FUN_10015520(p_player) &&
				FUN_10015e74(p_player, p_player->m_targetInfo.m_position.m_y)) {
				if (!p_player->m_unk0x172) {
					choice = 9;
				}
				else {
					choice = 0;
				}

				if (!choice && RandomIntBelow(2)) {
					choice = 8;
				}
				break;
			}
		}

		if (p_player->m_unk0x172 == -1) {
			index = RandomIntBelow(table->m_count - table->m_unk0x02);
			choice = table->m_entries[index].m_list[0];
		}
		else {
			index = FUN_100140e4(table, p_player->m_unk0x172);
			if (index == -1) {
				index = 0;
			}

			choice = table->m_entries[index].m_list[2 + RandomIntBelow(table->m_entries[index].m_list[1])];
		}

		if (p_player->m_unk0x172 == 4 && p_player->m_unk0x184 == 1) {
			choice = 5;
			break;
		}

		if (choice == 4 && (!FUN_10016222(p_player, 0x14) || !p_player->m_skillFlag1)) {
			choice = -1;
		}

		if (choice == 4 && FUN_1005432f(p_player) > mech->m_unk0xe0) {
			choice = 0;
		}

		if (choice == 3 && mech->m_unk0xe0 < 0xa0000) {
			choice = 2;
		}

		if (choice == 5 && p_player->m_unk0x78) {
			choice = 0;
		}
	} while (choice == -1);

	return choice;
}

// Returns the index of maneuver p_id in p_table, or -1.
// The original loads the index before m_entries (index order).
// FUNCTION: MW2 0x100140e4
MechS16 FUN_100140e4(SilverBrook0x08* p_table, MechS16 p_id)
{
	MechS16 i;

	for (i = 0; i < p_table->m_count; i++) {
		if (p_table->m_entries[i].m_list[0] == p_id) {
			return i;
		}
	}

	return -1;
}

// Starts p_player's maneuver (m_unk0x170): resets its state and sets it up, with the time it
// ends (m_unk0x178).
// FUNCTION: MW2 0x10014149
void FUN_10014149(Player* p_player)
{
	p_player->m_ai.m_goal = p_player->m_ai.m_target;
	p_player->m_unk0x17c = 0;
	p_player->m_unk0x19a = p_player->m_unk0x180 = p_player->m_unk0x184 = 0;
	p_player->m_unk0x196 = 0;
	p_player->m_unk0x178 = g_currentClock + 0x235a;
	switch (p_player->m_unk0x170) {
	case 0:
		p_player->m_unk0x178 = (RandomIntBelow(5) + 8) * 181 + g_currentClock;
		break;
	case 1:
		p_player->m_unk0x178 = g_currentClock + 0xe24;
		p_player->m_unk0x184 = -1;
		break;
	case 3:
		if (RandomIntBelow(2)) {
			p_player->m_unk0x180 = 1;
		}
		else {
			p_player->m_unk0x180 = -1;
		}
		break;
	case 4:
		p_player->m_steering->m_throttle = 0;
		p_player->m_steering->m_turn = 0;
		p_player->m_unk0x196 = 1;
		FUN_100156f2(p_player, 1);
		p_player->m_unk0x178 = g_currentClock + 0x389;
		break;
	case 11:
		p_player->m_steering->m_turn = 0;
		p_player->m_unk0x196 = 1;
		if (!p_player->m_unk0x180 || p_player->m_unk0x180 == 2) {
			p_player->m_steering->m_unk0x1e = 1;
		}
		else {
			p_player->m_steering->m_unk0x20 = 1;
		}

		FUN_100156f2(p_player, 1);
		p_player->m_unk0x178 = g_currentClock + 0x16a;
		break;
	case 5:
		FUN_10016093(p_player);
		p_player->m_unk0x196 = 1;
		break;
	case 8:
		p_player->m_unk0x178 = (RandomIntBelow(11) + 10) * 181 + g_currentClock;
		p_player->m_steering->m_unk0x2f = 1;
		break;
	case 7:
		if (p_player->m_targetInfo.m_distance < 4000) {
			FUN_10014723(p_player, p_player->m_index | 0x200, RandomIntBelow(2) ? 4 : 12, 10000);
		}
		else {
			FUN_10014723(p_player, p_player->m_index | 0x200, RandomIntBelow(2) ? 3 : 15, 10000);
		}
		break;
	case 9:
		p_player->m_unk0x180 = 0;
		p_player->m_steering->m_turn = 0;
		p_player->m_unk0x196 = 1;
		break;
	case 10:
		if (!p_player->m_mech->m_unk0xa4 || p_player->m_unk0x172 != 10 || p_player->m_unk0x7c != -1) {
			p_player->m_steering->m_unk0x2f = 1;
			p_player->m_unk0x178 = g_currentClock + 0x5a8;
		}
		else {
			p_player->m_unk0x178 = g_currentClock + 0x2d4;
		}
		break;
	case 12:
		p_player->m_unk0x178 = g_currentClock + 0x46b4;
		FUN_10054851(p_player);
		break;
	default:
		break;
	}
}

// Ends p_player's maneuver (m_unk0x170): undoes what it set up, records it as the previous one
// and makes the goal the target again.
// FUNCTION: MW2 0x1001450e
void FUN_1001450e(Player* p_player)
{
	switch (p_player->m_unk0x170) {
	case 1:
		FUN_10054778(p_player);
		if (p_player->m_ai.m_goal & 0x200) {
			g_players[p_player->m_ai.m_goal & 0xff]->m_unk0x1a2[p_player->m_unk0x180 / 2]--;
		}
		break;
	case 4:
		FUN_100156f2(p_player, 0);
		break;
	case 11:
		p_player->m_steering->m_unk0x20 = 0;
		p_player->m_steering->m_unk0x21 = 0;
		p_player->m_steering->m_unk0x1e = 0;
		p_player->m_steering->m_unk0x1f = 0;
		FUN_100156f2(p_player, 0);
		break;
	case 5:
		FUN_100156f2(p_player, 0);
		p_player->m_steering->m_unk0x20 = 0;
		p_player->m_steering->m_unk0x21 = 0;
		break;
	case 8:
	case 10:
		p_player->m_steering->m_unk0x2f = 0;
		break;
	case 7:
		FUN_10054778(p_player);
		break;
	case 9:
		FUN_100156f2(p_player, 0);
		break;
	case 12:
		FUN_10054778(p_player);
		break;
	case 2:
	case 3:
	case 6:
		break;
	}

	FUN_100156f2(p_player, 0);
	p_player->m_unk0x172 = p_player->m_unk0x170;
	p_player->m_unk0x170 = -1;
	p_player->m_unk0x178 = p_player->m_unk0x17c = 0;
	p_player->m_ai.m_target = p_player->m_ai.m_goal;
	p_player->m_unk0x180 = p_player->m_unk0x184 = 0;
	FUN_1005372c(p_player, p_player->m_ai.m_goal);
}

// Places a nav point for p_player where FUN_100147d0 puts it, and makes it the player's target.
// The only diff is a stack-slot permutation of x, y, z and nav.
// FUNCTION: MW2 0x10014723
void FUN_10014723(Player* p_player, MechU32 p_unk0x04, MechS16 p_unk0x08, MechS16 p_unk0x0c)
{
	MechS32 z;
	MechS32 y;
	MechS32 x;
	MechS32 nav;

	FUN_100147d0(p_unk0x04, p_unk0x08, &x, &z, &y, p_unk0x0c);
	nav = FUN_1005ec80(p_player->m_index, x, y, z);
	if (nav != -1) {
		g_navTable[nav].m_flags |= 1;
		g_navTable[nav].m_owner = p_player->m_index | 0x200;
		FUN_10054a30(p_player, 0x100);
	}
}

// Finds the point p_unk0x14 units out in direction p_unk0x04 (of g_unk0x100a2900's 16) from
// target p_unk0x00, a player (0x200) or a game thing (0x400), in world coordinates; for a player
// *p_y takes its heading. A game thing without an object is offset from its position.
// The empty else arms give the original's jmp to the next statement after each offset. The only
// other diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100147d0
void FUN_100147d0(MechU32 p_unk0x00, MechS16 p_unk0x04, MechS32* p_x, MechS32* p_z, MechS32* p_y, MechS16 p_unk0x14)
{
	MechU32 index;
	MechS32 thing;
	Matrix* matrix;
	struct SceneObject* obj;
	MechS32 y;

	y = 0;
	index = p_unk0x00 & 0xff;
	switch (p_unk0x00 & 0xf00) {
	case 0x200:
		*p_y = g_players[index]->m_heading;
		obj = g_players[index]->m_obj;
		break;
	case 0x400:
		thing = g_gameThings[index].m_unk0x04;
		obj = FUN_10020bdd(thing);
		if (!obj) {
			FUN_10020c6f(thing, p_x, p_y, p_z);
			*p_y = 0;
			if (g_unk0x100a2900[p_unk0x04].m_x) {
				*p_x += p_unk0x14 / g_unk0x100a2900[p_unk0x04].m_x;
			}
			else {
			}

			if (g_unk0x100a2900[p_unk0x04].m_y) {
				*p_z += p_unk0x14 / g_unk0x100a2900[p_unk0x04].m_y;
			}
			else {
			}

			return;
		}
		break;
	default:
		break;
	}

	if (g_unk0x100a2900[p_unk0x04].m_x) {
		*p_x = p_unk0x14 / g_unk0x100a2900[p_unk0x04].m_x;
	}
	else {
		*p_x = 0;
	}

	if (g_unk0x100a2900[p_unk0x04].m_y) {
		*p_z = p_unk0x14 / g_unk0x100a2900[p_unk0x04].m_y;
	}
	else {
		*p_z = 0;
	}

	matrix = FUN_10001e01(obj);
	FUN_1000d650(matrix, p_x, &y, p_z);
}

// Whether p_turn (16.16 degrees) is a sharp turn, past 5 degrees either way; a stopped player
// then creeps forward.
// FUNCTION: MW2 0x1001498c
MechS32 FUN_1001498c(Player* p_player, MechS32 p_turn)
{
	MechS32 sharp;

	sharp = 0;
	if (p_turn > 0x50000 || p_turn < -0x50000) {
		sharp = 1;
		if (!p_player->m_steering->m_throttle) {
			p_player->m_steering->m_throttle = 0x66;
		}
	}

	return sharp;
}

// FUNCTION: MW2 0x100149e7
void FUN_100149e7(Player* p_player, MechS16 p_target)
{
	MechS32 range;
	MechS32 heading;

	FUN_1005372c(p_player, p_target);
	if (!FUN_10015709(p_player)) {
		range = 15000;
		p_player->m_steering->m_throttle = FUN_10053811(p_player, range);
		heading = FUN_1005398f(p_player);
		FUN_1001498c(p_player, heading);
	}
	else {
		heading = FUN_1005432f(p_player);
	}

	if (p_player->m_targetInfo.m_distance < 4500) {
		p_player->m_unk0x174 = 10;
	}

	FUN_1004b5a0(p_player, heading);
	FUN_100160eb(p_player);
}

// Turns p_player toward its goal and closes on its target; once stopped and turned more than 5
// degrees away, turns in place (FUN_1001498c) until the target is more than 4500 away.
// FUNCTION: MW2 0x10014aa8
void FUN_10014aa8(Player* p_player, MechS16 p_target)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_player->m_ai.m_goal);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	if (!p_player->m_unk0x184) {
		FUN_1005372c(p_player, p_player->m_ai.m_target);
		if (!FUN_10015709(p_player)) {
			FUN_1005398f(p_player);
			p_player->m_steering->m_throttle = FUN_10053811(p_player, 4000);
		}

		if (!p_player->m_steering->m_throttle && abs(heading) > 0x50000) {
			p_player->m_unk0x184 = 1;
		}

		FUN_100160eb(p_player);
	}

	if (p_player->m_unk0x184 == 1) {
		if (FUN_10015709(p_player)) {
			heading = FUN_1005432f(p_player);
		}
		else {
			heading = FUN_1005398f(p_player);
		}

		if (!FUN_1001498c(p_player, heading)) {
			p_player->m_steering->m_throttle = 0;
		}
		else {
			FUN_100160eb(p_player);
		}

		FUN_1005372c(p_player, p_player->m_ai.m_target);
		if (p_player->m_targetInfo.m_distance > 4500) {
			p_player->m_unk0x184 = 0;
		}
	}
}

// Closes on p_target. Whether it is within 8000, or more when closing fast (FUN_10016093, per
// 500000).
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10014c3d
MechS32 FUN_10014c3d(Player* p_player, MechS16 p_target)
{
	MechDouble scale;
	MechS32 heading;
	MechS32 result;

	result = FALSE;
	FUN_1005372c(p_player, p_target);
	if (!FUN_10015709(p_player)) {
		heading = FUN_1005398f(p_player);
		p_player->m_steering->m_throttle = FUN_10053811(p_player, 5500);
	}
	else {
		heading = FUN_1005432f(p_player);
	}

	FUN_1004b5a0(p_player, heading);
	scale = FixedDiv16(FUN_10016093(p_player) << 16, 500000) / 65536.0;
	if (scale < 1.0) {
		scale = 1.0;
	}

	if (p_player->m_targetInfo.m_distance <= scale * 8000.0) {
		result = TRUE;
	}

	FUN_100160eb(p_player);
	return result;
}

// Closes on p_target's shape until it is its target's again. Whether it is within 2000 (then it
// sets steering flag 0x43).
// FUNCTION: MW2 0x10014d4e
MechS32 FUN_10014d4e(Player* p_player, MechS16 p_target)
{
	MechS32 result;

	result = FALSE;
	FUN_1005372c(p_player, p_target);
	if (!FUN_10015709(p_player) || FUN_1001627f(p_target) == p_player->m_unk0x188) {
		FUN_1005398f(p_player);
		p_player->m_steering->m_throttle = FUN_10053811(p_player, 0);
	}

	if (p_player->m_targetInfo.m_unk0x04 <= 2000) {
		p_player->m_steering->m_unk0x43 = 1;
		result = TRUE;
	}

	return result;
}

// The only diff is a stack-slot permutation of heading and result.
// FUNCTION: MW2 0x10014df1
MechS32 FUN_10014df1(Player* p_player, MechS16 p_target)
{
	MechS32 heading;
	MechS32 result;

	FUN_1005372c(p_player, p_target);
	heading = FUN_1005398f(p_player);
	FUN_1004b5a0(p_player, heading);
	result = p_player->m_mech->m_unk0xa4;
	p_player->m_steering->m_throttle = 0x400;
	FUN_100160eb(p_player);
	return result;
}

// Turns p_player toward its goal and closes on its target. Whether it is within 3000.
// FUNCTION: MW2 0x10014e5e
MechS32 FUN_10014e5e(Player* p_player, MechS16 p_target)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_player->m_ai.m_goal);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	FUN_1005372c(p_player, p_player->m_ai.m_target);
	if (!FUN_10015709(p_player)) {
		p_player->m_steering->m_throttle = FUN_10053811(p_player, 3000);
		FUN_1005398f(p_player);
	}

	FUN_100160eb(p_player);
	return p_player->m_targetInfo.m_distance <= 3000;
}

// Turns p_player toward p_target and runs an attack in three steps (m_unk0x180): steer at it
// while it can fire and has a line to it, then for a second, then FUN_100155e1. Whether that ended.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10014f23
MechS32 FUN_10014f23(Player* p_player, MechS16 p_target)
{
	MechS32 heading;
	MechS32 result;

	result = FALSE;
	FUN_1005372c(p_player, p_target);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	switch (p_player->m_unk0x180) {
	case 0:
		if (FUN_10016222(p_player, 40) && FUN_10015520(p_player)) {
			FUN_100156f2(p_player, 1);
		}
		else {
			FUN_100156f2(p_player, 0);
			p_player->m_unk0x180 = 1;
			if (FUN_10016222(p_player, 20) && !RandomIntBelow(4)) {
				p_player->m_unk0x174 = 5;
			}

			p_player->m_unk0x17c = g_currentClock + 181;
		}
		break;
	case 1:
		if (p_player->m_unk0x17c < g_currentClock) {
			p_player->m_unk0x180 = 2;
			FUN_100156f2(p_player, 0);
		}
		else {
			FUN_100156f2(p_player, 1);
		}

		FUN_1005398f(p_player);
		break;
	case 2:
		if (FUN_100155e1(p_player)) {
			result = TRUE;
		}
		break;
	}

	return result;
}

// Turns p_player toward p_target. Whether the player is at least 2000 above it.
// FUNCTION: MW2 0x100150c1
MechS32 FUN_100150c1(Player* p_player, MechS16 p_target)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_target);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	if (p_player->m_position.m_y >= p_player->m_targetInfo.m_position.m_y + 2000) {
		return 1;
	}
	else {
		return 0;
	}
}

// Turns p_player toward p_target.
// FUNCTION: MW2 0x1001512e
MechS32 FUN_1001512e(Player* p_player, MechS16 p_target)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_target);
	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	return 0;
}

// Turns p_player toward p_target and circles it (steering flags 0x20/0x21) while more than 1000
// away, counting the passes it makes closer in (m_unk0x180). Whether it has weapons ready.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10015172
MechS32 FUN_10015172(Player* p_player, MechS16 p_target)
{
	MechS32 rate;
	MechS32 result;
	MechS32 turn;
	MechS16 side;
	Mech* mech;
	Mech* targetMech;

	FUN_1005372c(p_player, p_target);
	turn = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, turn);
	if (p_player->m_unk0x78 || p_player->m_mech->m_unk0xa4) {
		result = TRUE;
	}
	else {
		result = FALSE;
	}

	if (p_player->m_unk0x180 == 2) {
		return result;
	}

	turn = abs(turn);
	targetMech = g_players[p_target & 0xff]->m_mech;
	mech = p_player->m_mech;
	if (turn > mech->m_unk0xe0 && turn < mech->m_unk0xe0 * 3) {
		side = 1;
	}
	else {
		side = 0;
	}

	if (p_player->m_targetInfo.m_distance > 1000) {
		rate = FUN_10016093(p_player);
		if (rate > (p_player->m_targetInfo.m_distance <= 6000 ? 9 : 36)) {
			FUN_10016057(p_player, side);
		}
		else {
			FUN_10016057(p_player, !side);
		}

		FUN_100156f2(p_player, 1);
		if (!FUN_10016222(p_player, 40)) {
			result = TRUE;
		}
	}
	else {
		if (p_player->m_steering->m_unk0x1d) {
			p_player->m_unk0x180++;
		}

		FUN_100156f2(p_player, 0);
		p_player->m_steering->m_unk0x20 = 0;
		p_player->m_steering->m_unk0x21 = 0;
	}

	return result;
}

// FUNCTION: MW2 0x10015342
void FUN_10015342(Player* p_player, MechS16 p_target)
{
	MechS32 heading;

	FUN_1005372c(p_player, p_target);
	if (!p_player->m_steering->m_unk0x2f) {
		if (!FUN_10015709(p_player)) {
			heading = FUN_1005398f(p_player);
		}
		else {
			heading = FUN_1005432f(p_player);
		}
	}
	else {
		heading = FUN_1005398f(p_player);
	}

	FUN_1004b5a0(p_player, heading);
	p_player->m_steering->m_throttle = 0x400;
	FUN_100160eb(p_player);
}

// Turns p_player toward its goal and closes on its target, giving up the goal (FUN_10054778,
// FUN_10054851) now and then while far from the target, and handing the target to FUN_10054a30
// within 3000.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100153e6
MechS32 FUN_100153e6(Player* p_player, MechS16 p_target)
{
	MechS32 heading;
	MechS32 result;

	result = 0;
	FUN_1005372c(p_player, p_player->m_ai.m_goal);
	if (p_player->m_unk0x17c < g_currentClock) {
		if (p_player->m_targetInfo.m_distance > 15000.0) {
			FUN_10054778(p_player);
			FUN_10054851(p_player);
		}

		p_player->m_unk0x17c = g_currentClock + 0x5a8;
	}

	heading = FUN_1005432f(p_player);
	FUN_1004b5a0(p_player, heading);
	FUN_1005372c(p_player, p_player->m_ai.m_target);
	if (!FUN_10015709(p_player)) {
		p_player->m_steering->m_throttle = FUN_10053811(p_player, 3000);
		FUN_1005398f(p_player);
	}

	FUN_100160eb(p_player);
	if (p_player->m_targetInfo.m_distance <= 3000) {
		FUN_10054a30(p_player, p_player->m_ai.m_target);
	}

	return result;
}

// Whether p_player, targeting a player it has a clear line to, is more than 800 below it with
// m_unk0xc0 not negative.
// The only diff is a stack-slot permutation of index and below.
// FUNCTION: MW2 0x10015520
MechS32 FUN_10015520(Player* p_player)
{
	MechS16 index;
	MechS32 below;

	below = 0;
	index = p_player->m_targetInfo.m_target & 0xff;
	switch (p_player->m_targetInfo.m_target & 0xf00) {
	default:
		break;
	case 0x200:
		if (!FUN_10015e74(p_player, p_player->m_position.m_y)) {
			if (p_player->m_targetInfo.m_position.m_y - 800 > p_player->m_position.m_y &&
				p_player->m_mech->m_unk0xc0 >= 0) {
				below = 1;
			}
			else {
				below = 0;
			}
		}
		break;
	}

	return below;
}

// Near the ground, sets the player's m_unk0x1d while its mech falls faster than 85% of the fall
// damage speed and clears it once it is slower than 75%, and logs a fall faster than that speed.
// Returns the player's m_unk0x78.
// Stack-slot permutation: line, value and mech.
// FUNCTION: MW2 0x100155e1
MechS32 FUN_100155e1(Player* p_player)
{
	MechChar line[80];
	MechS16 value;
	Mech* mech;

	mech = p_player->m_mech;
	if (mech->m_unk0xc0 < 0 || !mech->m_unk0xec) {
		return p_player->m_unk0x78;
	}

	value = p_player->m_steering->m_unk0x1d;
	if (p_player->m_position.m_y < 20000) {
		if (mech->m_unk0xf8 < -0x102762 * 0.85) {
			value = 1;
		}
		else if (mech->m_unk0xf8 > -0x102762 * 0.75) {
			value = 0;
		}
	}

	FUN_100156f2(p_player, value);
	if (mech->m_unk0xf8 < -0x102762) {
		sprintf(
			line,
			"%6ld : %2d Mech %2d has exceded fall damage speed.\n",
			g_currentClock,
			p_player->m_team,
			p_player->m_index
		);
		WriteToMw2Log(line);
	}

	return p_player->m_unk0x78;
}

// FUNCTION: MW2 0x100156f2
void FUN_100156f2(Player* p_player, MechS8 p_value)
{
	p_player->m_steering->m_unk0x1d = p_value;
}

// Steers p_player's mech around what lies ahead: casts up to four probe rays (FUN_10015b9f) on
// the avoiding side m_unk0x190, as long as the mech's speed (m_unk0x192), and turns away from what
// they hit, slowing down if the first one hits. Returns whether it steered. Between checks (every
// 10 ticks for the local player, 90 or 181 for others) it returns whether it is avoiding a shape.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10015709
MechS32 FUN_10015709(Player* p_player)
{
	MechS32 length;
	Shape* hit;
	Ray ray;
	Mech* mech;
	MechS16 i;
	MechS32 turn;
	MechS32 throttle;

	turn = 0;
	mech = p_player->m_mech;
	if (!mech->m_unk0x88 || (!p_player->m_unk0x190 && !p_player->m_steering->m_throttle) ||
		p_player->m_steering->m_unk0x2f) {
		return FALSE;
	}

	if (p_player->m_unk0x18c > g_currentClock) {
		return p_player->m_unk0x188 ? TRUE : FALSE;
	}

	if (!p_player->m_unk0x190) {
		length = ApproximateVectorLength(mech->m_unk0xf4, 0, mech->m_unk0xfc);
		p_player->m_unk0x192 = FixedDiv16(length, 0x7a120);
		if (p_player->m_unk0x00 != 1) {
			p_player->m_unk0x192 >>= 1;
		}

		if (p_player->m_unk0x192 < 0.3 * 0x10000) {
			p_player->m_unk0x192 = 0x4ccc;
		}
	}

	for (i = 0; i < 4; i++) {
		FUN_10015b9f(p_player, &ray, p_player->m_unk0x190, i, FixedMul16(p_player->m_unk0x192, 0x13880000) >> 16, 0);
		if (TestSegmentCollision(&ray, &hit, p_player->m_index)) {
			if (FUN_10015b40(hit)) {
				break;
			}

			if (!p_player->m_unk0x190) {
				p_player->m_unk0x190 = FUN_10015d2a(
					p_player,
					hit,
					p_player->m_position.m_x,
					p_player->m_position.m_y,
					p_player->m_position.m_z
				);
				p_player->m_unk0x188 = hit;
			}

			turn += (MechS32) (p_player->m_unk0x190 * (0.2 * 0x10000000) / 4);
		}
		else {
			if (i == 0 && abs(FUN_1005432f(p_player)) <= 0x10000) {
				p_player->m_unk0x188 = NULL;
				p_player->m_unk0x190 = 0;
			}

			if (i == 0 && p_player->m_unk0x190) {
				FUN_10015b9f(
					p_player,
					&ray,
					-p_player->m_unk0x190,
					1,
					FixedMul16(p_player->m_unk0x192, 0x13880000) >> 16,
					1
				);
				if (TestSegmentCollision(&ray, &hit, p_player->m_index)) {
					if (FUN_10015b40(hit)) {
						break;
					}

					turn += (MechS32) (p_player->m_unk0x190 * (0.1 * 0x10000000));
				}
			}

			break;
		}
	}

	if (turn) {
		turn = FUN_10015e34(turn, 0x3333333);
		p_player->m_steering->m_turn = turn;
		if (p_player->m_index == g_localPlayerId) {
			throttle = p_player->m_unk0x180;
		}
		else {
			throttle = 0x400;
		}

		if (i == 0) {
			p_player->m_steering->m_throttle = throttle;
		}
		else {
			p_player->m_steering->m_throttle = 0x100;
		}

		if (p_player->m_unk0x00 == 6) {
			p_player->m_steering->m_throttle = 0;
			p_player->m_steering->m_turn = 0;
		}
	}

	if (p_player->m_index == g_localPlayerId) {
		p_player->m_unk0x18c = g_currentClock + 10;
	}
	else {
		p_player->m_unk0x18c = g_currentClock + (p_player->m_unk0x188 ? 90 : 181);
	}

	return turn ? TRUE : FALSE;
}

// Whether p_shape is solid ground to stand on: a flat enough face or a shape of type 0x50.
// FUNCTION: MW2 0x10015b40
MechS32 FUN_10015b40(Shape* p_shape)
{
	if (FUN_10034db8(p_shape) && g_segmentNormalY >= 0xc41b) {
		return 1;
	}

	return (p_shape->m_unk0x02 & 0xf0) == 0x50;
}

// Builds a probe ray for p_player in p_ray: p_length along direction p_step (mirrored for a
// negative p_side) in the mech's frame, from its position or, with p_fromEdge, from its side.
// Stack-slot permutation of the locals.
// FUNCTION: MW2 0x10015b9f
void FUN_10015b9f(Player* p_player, Ray* p_ray, MechS32 p_side, MechS16 p_step, MechS32 p_length, MechS32 p_fromEdge)
{
	Matrix* matrix;
	MechS32 x;
	MechS32 y;
	MechS32 dx;
	MechS32 z;
	MechS32 dz;
	MechS32 dy;
	Mech* mech;

	dy = 0;
	mech = p_player->m_mech;
	matrix = FUN_10001e01(p_player->m_obj);
	dx = g_unk0x100a2900[p_side >= 0 ? p_step : (0x10 - p_step) % 16].m_x;
	dz = g_unk0x100a2900[p_side >= 0 ? p_step : (0x10 - p_step) % 16].m_y;
	if (dx) {
		dx = p_length / dx;
	}

	if (dz) {
		dz = p_length / dz;
	}

	FUN_1000d650(matrix, &dx, &dy, &dz);
	if (p_fromEdge) {
		if (p_side >= 0) {
			x = mech->m_radius - 1;
		}
		else {
			x = -mech->m_radius + 1;
		}

		z = 0;
		FUN_1000d650(matrix, &x, &dy, &z);
		y = p_player->m_position.m_y;
	}
	else {
		y = p_player->m_position.m_y;
		x = p_player->m_position.m_x;
		z = p_player->m_position.m_z;
	}

	BuildRayFromSegment(p_ray, x, y, z, dx, y, dz);
}

// Which side of p_player the point (p_x, p_y, p_z) is, seen from p_shape: 1 or -1.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10015d2a
MechS16 FUN_10015d2a(Player* p_player, Shape* p_shape, MechS32 p_x, MechS32 p_y, MechS32 p_z)
{
	MechS32 dz;
	MechS32 unused;
	MechU32 distance;
	MechS32 heading;
	MechS32 pitch;
	MechS32 dx;
	MechS32 dy;

	dx = p_shape->m_unk0x34 - p_x;
	dy = p_shape->m_unk0x38 - p_y;
	dz = p_shape->m_unk0x3c - p_z;
	FUN_10060197(dx, dy, dz, &heading, &pitch, &distance, &unused);
	heading -= p_player->m_heading;
	if (heading > 0xb40000) {
		heading -= 0x1680000;
	}
	else if (heading < -0xb40000) {
		heading += 0x1680000;
	}

	return heading >= 0 ? -1 : 1;
}

// The heading from p_player to p_target (16.16 degrees, 0 to 360), keeping its target.
// FUNCTION: MW2 0x10015dd6
MechS32 FUN_10015dd6(Player* p_player, MechS16 p_target)
{
	MechS32 target;
	MechS32 heading;

	target = p_player->m_targetInfo.m_target;
	FUN_1005372c(p_player, p_target);
	heading = (FUN_1005432f(p_player) + 0x1680000) % 0x1680000;
	FUN_1005372c(p_player, target);
	return heading;
}

// Clamps p_value to +/- p_limit.
// FUNCTION: MW2 0x10015e34
MechS32 FUN_10015e34(MechS32 p_value, MechS32 p_limit)
{
	if (p_value > p_limit) {
		p_value = p_limit;
	}
	else if (p_value < -p_limit) {
		p_value = -p_limit;
	}

	return p_value;
}

// Whether the segment from p_player at height p_y to its target is clear, or hits the target.
// The only diff is a stack-slot permutation of ray, hit, target and flags.
// FUNCTION: MW2 0x10015e74
MechS32 FUN_10015e74(Player* p_player, MechS32 p_y)
{
	Ray ray;
	Shape* hit;
	MechU32 target;
	MechU16 flags;

	hit = NULL;
	BuildRayFromSegment(
		&ray,
		p_player->m_position.m_x,
		p_y,
		p_player->m_position.m_z,
		p_player->m_targetInfo.m_position.m_x,
		p_player->m_targetInfo.m_position.m_y,
		p_player->m_targetInfo.m_position.m_z
	);
	BuildRayFixed(&ray);
	if (TestSegmentCollision(&ray, &hit, p_player->m_index) && hit) {
		flags = hit->m_unk0x02;
		target = p_player->m_targetInfo.m_target;
		if ((flags & 0x100) && (target & 0x200)) {
			return hit->m_unk0x14 == (target & 0xff);
		}
		else if ((flags & 0x200) && (target & 0x400)) {
			return hit->m_unk0x14 == (target & 0xff);
		}
		else {
			return 0;
		}
	}

	return 1;
}

// Whether p_mech has weapons but none left that can fire: every one is out of ammunition.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10015fa8
MechS32 FUN_10015fa8(Mech* p_mech)
{
	MechS16 i;
	WeaponSlot* slot;
	MechS32 found;

	found = FALSE;
	for (i = 0; i < p_mech->m_weaponCount && !found; i++) {
		slot = &p_mech->m_weapons[i];
		if (slot->m_state != c_weaponEmpty && slot->m_ammo) {
			found = TRUE;
			break;
		}
	}

	return !found && p_mech->m_weaponCount;
}

// FUNCTION: MW2 0x10016057
void FUN_10016057(Player* p_player, MechS16 p_value)
{
	p_player->m_steering->m_unk0x20 = p_value;
	if (!p_value) {
		p_player->m_steering->m_unk0x21 = 1;
	}
	else {
		p_player->m_steering->m_unk0x21 = 0;
	}
}

// How fast p_player closes on its target since the last call, per tick.
// FUNCTION: MW2 0x10016093
MechS32 FUN_10016093(Player* p_player)
{
	MechS32 rate;

	if (!g_deltaTime) {
		return 0;
	}

	rate = (p_player->m_unk0x19a - p_player->m_targetInfo.m_distance) / g_deltaTime;
	p_player->m_unk0x19a = p_player->m_targetInfo.m_distance;
	return rate;
}

// FUNCTION: MW2 0x100160eb
void FUN_100160eb(Player* p_player)
{
	MechS32 turn;
	Mech* mech;

	mech = p_player->m_mech;
	if (!p_player->m_skillFlag0 || p_player->m_unk0x190 || p_player->m_unk0x170 == 10) {
		return;
	}

	FUN_1005372c(p_player, p_player->m_ai.m_target);
	turn = abs(FUN_1005432f(p_player));
	if (turn >= mech->m_unk0xe0 || (turn <= -mech->m_unk0xe0 && p_player->m_steering->m_throttle)) {
		if (FUN_10016222(p_player, 20) && p_player->m_unk0x78 && !p_player->m_steering->m_unk0x1d) {
			FUN_100156f2(p_player, 1);
		}
	}
	else if (!p_player->m_unk0x196 && p_player->m_steering->m_unk0x1d && p_player->m_unk0x78) {
		FUN_100156f2(p_player, 0);
	}
}

// Whether p_player's mech can fire: m_unk0xc0 at least 6, m_unk0xec set and the heat
// (m_unk0x98, 16.16) under p_limit.
// FUNCTION: MW2 0x10016222
MechS32 FUN_10016222(Player* p_player, MechS32 p_limit)
{
	Mech* mech;

	mech = p_player->m_mech;
	return mech->m_unk0xc0 >= 6 && mech->m_unk0xec && mech->m_unk0x98 >> 16 < p_limit;
}

// The shape of the player or game thing an AI target id names, or NULL.
// The only diff is a stack-slot permutation of index, id and obj.
// FUNCTION: MW2 0x1001627f
Shape* FUN_1001627f(MechS16 p_target)
{
	MechS16 index;
	MechS32 id;
	SceneObject* obj;

	obj = NULL;
	index = p_target & 0xff;
	switch (p_target & 0xf00) {
	case 0x200:
		obj = g_players[index]->m_obj;
		break;
	case 0x400:
		id = g_gameThings[index].m_unk0x04;
		obj = FUN_10020bdd(id);
		break;
	}

	return obj ? obj->m_unk0x6c : NULL;
}

// When p_mech's player fires the weapon in p_slot at another player, the target may dodge: an
// AI player that sees the shot coming (a guided weapon or type 21, and the target inside its
// sights within 3 units and 15 degrees) sidesteps along a clear path (state 11), or else braces
// (state 4).
// Stack-slot permutation; pitch < range and bearing < maxAngle compare in the other operand order.
// FUNCTION: MW2 0x1001632c
void FUN_1001632c(WeaponSlot* p_slot, Mech* p_mech)
{
	MechS32 maxAngle;
	MechS32 index;
	MechS32 pitch;
	MechS32 kind;
	Player* target;
	MechU32 id;
	MechS32 bearing;
	MechS32 x;
	MechS32 y;
	MechS32 range;
	MechS32 sx;
	MechS32 z;
	MechS32 sy;
	Shape* hit;
	MechS32 heading;
	Ray ray;
	MechS16 side;
	MechS16 j;
	MechS16 step;

	id = 0;
	target = NULL;
	if (!p_mech->m_player->m_skillFlag2 || !RandomIntBelow(3)) {
		return;
	}

	if (!g_weaponDefs[p_slot->m_type].m_unk0x18 && p_slot->m_type != 21) {
		return;
	}

	if (p_mech->m_player->m_unk0x10 == 2) {
		id = p_mech->m_player->m_targetInfo.m_target;
	}
	else if (p_mech->m_player->m_ai.m_goal & 0x200) {
		id = p_mech->m_player->m_ai.m_goal;
	}
	else {
		id = p_mech->m_player->m_targetInfo.m_target;
	}

	kind = id & 0xf00;
	index = id & 0xff;
	if (!index || kind != 0x200 || g_players[index]->m_unk0x174 == 4) {
		return;
	}

	if (p_mech->m_player->m_unk0x10 == 2) {
		target = g_players[index];
	}

	if (!target && FUN_10041998(p_mech, &sx, &sy)) {
		x = g_players[index]->m_position.m_x;
		y = g_players[index]->m_position.m_y;
		z = g_players[index]->m_position.m_z;
		if (FUN_1004c11d(&x, &y, &z)) {
			range = 0x30000;
			maxAngle = 15;
			pitch = p_mech->m_player->m_targetInfo.m_unk0x18 / 0xf00;
			bearing = (FUN_1005432f(p_mech->m_player) >> 16) % 360;
			if (pitch < range && -range < pitch && bearing < maxAngle && -maxAngle < bearing) {
				target = g_players[index];
			}
		}
	}

	if (target && FUN_10016222(target, 0x41)) {
		if (RandomIntBelow(3)) {
			heading = FUN_10015dd6(target, p_mech->m_player->m_index);
			step = FixedDiv16(heading, 0x5a0000) >> 16;
			if (RandomIntBelow(2)) {
				side = -1;
			}
			else {
				side = 1;
			}

			step = ((step + 1) % 4) * 4;
			for (j = 0; j < 2; j++) {
				FUN_10015b9f(target, &ray, side, step, 5000, 0);
				if (!TestSegmentCollision(&ray, &hit, target->m_index)) {
					target->m_unk0x174 = 11;
					target->m_unk0x180 = step;
					break;
				}

				side = -side;
			}
		}

		if (target->m_unk0x174 == 0 && target->m_skillFlag1) {
			target->m_unk0x174 = 4;
		}
	}
}

// Picks p_player's place around its goal player (eight places, 45 degrees apart): the one it is
// nearest, moved off the front and back, or else the first free one on its side. Returns twice
// the place.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100166b1
MechS16 FUN_100166b1(Player* p_player)
{
	MechS32 angle;
	MechS32 i;
	MechS32 place;
	MechS32 found;
	Player* leader;

	leader = g_players[p_player->m_ai.m_goal & 0xff];
	angle = FixedDiv16(FUN_10015dd6(leader, p_player->m_index | 0x200), 0x2d0000);
	place = (angle + 0x8000) >> 16;
	if (place >= 8) {
		place = 0;
	}

	if (place == 0 || place == 1 || place == 7) {
		if (place == 0) {
			if (RandomIntBelow(2)) {
				place = 2;
			}
			else {
				place = 6;
			}

			if (p_player->m_unk0x1a2[place]) {
				if (place == 2) {
					place = 6;
				}
				else {
					place = 2;
				}
			}
		}

		if (place == 1) {
			place = 3;
		}

		if (place == 7) {
			place = 5;
		}
	}
	else {
		found = FALSE;
		if (place > 4) {
			for (i = 6; i > 3 && !found; i--) {
				if (!leader->m_unk0x1a2[i]) {
					found = TRUE;
				}
			}

			i++;
		}
		else {
			for (i = 2; i < 5 && !found; i++) {
				if (!leader->m_unk0x1a2[i]) {
					found = TRUE;
				}
			}

			i--;
		}

		place = i;
	}

	return place * 2;
}

// FUNCTION: MW2 0x10016880
MechS32 FUN_10016880(Mech* p_mech)
{
	return p_mech->m_unk0xa4 && p_mech->m_player->m_steering->m_unk0x2f != 1 && !p_mech->m_player->m_unk0x196 &&
		   p_mech->m_player->m_unk0x170 != 10;
}
