#include "decomp.h"
#include "hazelstar0x80.h"
#include "linenpacket0x218.h"
#include "sableroster0x24.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <string.h>

// The two stars of a custom battle: the player's and the enemy's.

// SIZE 0x08
// A formation: its label and its simulator option.
struct AshCord0x08 {
	MechChar* m_unk0x00; // 0x00
	MechChar* m_unk0x04; // 0x04
};

DECOMP_SIZE_ASSERT(SableRoster0x24, 0x24)
DECOMP_SIZE_ASSERT(HazelStar0x80, 0x80)
DECOMP_SIZE_ASSERT(AshCord0x08, 0x08)

extern LinenPacket0x218 g_unk0x10090288;

void FUN_1002ea62(MechS32 p_count, SableRoster0x24* p_mechs, MechS32 p_enemyCount, SableRoster0x24* p_enemies);

// GLOBAL: MW2SHELL 0x1005b718
HazelStar0x80 g_unk0x1005b718 =
	{0, 0, 3, 3, 100, {{12, "tbr00std", "MechWarrior"}, {0, "drw00std", "Friend 1"}, {1, "frm00std", "Friend 2"}}};

// GLOBAL: MW2SHELL 0x1005b798
HazelStar0x80 g_unk0x1005b798 =
	{0, 0, 3, 3, 100, {{12, "tbr00std", "Enemy 1"}, {0, "drw00std", "Enemy 2"}, {1, "frm00std", "Enemy 3"}}};

// GLOBAL: MW2SHELL 0x1005b818
HazelStar0x80* g_unk0x1005b818 = &g_unk0x1005b718;

// GLOBAL: MW2SHELL 0x1005b820
AshCord0x08 g_unk0x1005b820[6] = {
	{"~Echelon Left", "echelonl"},
	{"~Echelon Right", "echelonr"},
	{"~Line Abreast", "lineabreast"},
	{"~Line Astern", "lineastern"},
	{"~V-Form", "vform"},
	{"~Wedge", "wedge"},
};

// Saves both stars and which one is selected in the mw2prm.cfg record, for FUN_10002d8d to
// restore.
// FUNCTION: MW2SHELL 0x10002d30
void FUN_10002d30()
{
	if (g_unk0x1005b818 == &g_unk0x1005b718) {
		g_unk0x10090288.m_unk0x0c = TRUE;
	}
	else {
		g_unk0x10090288.m_unk0x0c = FALSE;
	}

	g_unk0x10090288.m_unk0x10 = g_unk0x1005b718;
	g_unk0x10090288.m_unk0x90 = g_unk0x1005b798;
}

// FUNCTION: MW2SHELL 0x10002d8d
void FUN_10002d8d()
{
	if (g_unk0x10090288.m_unk0x0c) {
		g_unk0x1005b818 = &g_unk0x1005b718;
	}
	else {
		g_unk0x1005b818 = &g_unk0x1005b798;
	}

	g_unk0x1005b718 = g_unk0x10090288.m_unk0x10;
	g_unk0x1005b798 = g_unk0x10090288.m_unk0x90;
}

// FUNCTION: MW2SHELL 0x10003013
MechChar* FUN_10003013(MechS32 p_index)
{
	if (p_index < 0) {
		p_index = g_unk0x1005b818->m_unk0x04;
	}

	if (p_index < g_unk0x1005b818->m_unk0x08 && p_index < g_unk0x1005b818->m_unk0x0c) {
		return g_unk0x1005b818->m_unk0x14[p_index].m_unk0x04;
	}
	else {
		return "";
	}
}

// FUNCTION: MW2SHELL 0x1000307c
MechS32 FUN_1000307c(MechS32 p_index)
{
	if (p_index < 0) {
		p_index = g_unk0x1005b818->m_unk0x04;
	}

	if (p_index < g_unk0x1005b818->m_unk0x08 && p_index < g_unk0x1005b818->m_unk0x0c) {
		return g_unk0x1005b818->m_unk0x14[p_index].m_unk0x00;
	}
	else {
		return -1;
	}
}

// Returns a star's formation: the selected star's for a negative p_star, else the player's (0) or
// the enemy's.
// FUNCTION: MW2SHELL 0x100030e5
MechS32 FUN_100030e5(MechS32 p_star)
{
	if (p_star < 0) {
		return g_unk0x1005b818->m_unk0x00;
	}
	else if (p_star) {
		return g_unk0x1005b798.m_unk0x00;
	}
	else {
		return g_unk0x1005b718.m_unk0x00;
	}
}

// FUNCTION: MW2SHELL 0x1000312e
HazelStar0x80* FUN_1000312e(MechS32 p_star)
{
	if (p_star < 0) {
		return g_unk0x1005b818;
	}
	else if (p_star) {
		return &g_unk0x1005b798;
	}
	else {
		return &g_unk0x1005b718;
	}
}

// STUB: MW2SHELL 0x10003175
void FUN_10003175(MechS32, MechS32, MechS32, MechS32, MechS32)
{
	STUB(0x10003175);
}

// Adds the formations to the simulator's command line and writes the stars' .bwd files.
// FUNCTION: MW2SHELL 0x10003221
void FUN_10003221()
{
	strcat(g_unk0x10090288.m_unk0x118, " -of=");
	strcat(g_unk0x10090288.m_unk0x118, g_unk0x1005b820[g_unk0x1005b718.m_unk0x00].m_unk0x04);
	if (g_unk0x1005b798.m_unk0x0c) {
		strcat(g_unk0x10090288.m_unk0x118, " -oe=");
		strcat(g_unk0x10090288.m_unk0x118, g_unk0x1005b820[g_unk0x1005b798.m_unk0x00].m_unk0x04);
	}

	FUN_1002ea62(
		g_unk0x1005b718.m_unk0x0c,
		g_unk0x1005b718.m_unk0x14,
		g_unk0x1005b798.m_unk0x0c,
		g_unk0x1005b798.m_unk0x14
	);
}

// STUB: MW2SHELL 0x10003d3a
void FUN_10003d3a(TMPackDataBase* p_database, MechS32 p_campaign)
{
	STUB(0x10003d3a);
}
