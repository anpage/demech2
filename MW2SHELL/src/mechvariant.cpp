#include "mechvariant.h"

#include "audiosample.h"
#include "brasslantern0x414.h"
#include "campaignmission.h"
#include "cedarknot0x10.h"
#include "decomp.h"
#include "emberglyph0x3e.h"
#include "granitemast0x18.h"
#include "hazelstar0x80.h"
#include "hollowreed0x110.h"
#include "linenpacket0x218.h"
#include "mechbay.h"
#include "menulist0x10d.h"
#include "mousestate.h"
#include "sableroster0x24.h"
#include "shellmain.h"
#include "simhandoff.h"
#include "tallowsign0x10.h"
#include "tinwhistle0x3c.h"
#include "tmpackdatabase.h"
#include "types.h"
#include "unk10010a30.h"
#include "unk1003bf90.h"
#include "unk1006e150.h"
#include "unk100711f8.h"
#include "video.h"
#include "videodriver.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The two stars of a custom battle: the player's and the enemy's.

// SIZE 0x08
// A formation: its label and its simulator option.
struct AshCord0x08 {
	MechChar* m_unk0x00; // 0x00
	MechChar* m_unk0x04; // 0x04
};

// SIZE 0x10
// A mech's place in a formation on the star screen: the star's mech it shows, where its video
// plays and which label position its name takes.
struct CopperPost0x10 {
	MechS32 m_mech;  // 0x00
	MechS32 m_left;  // 0x04
	MechS32 m_top;   // 0x08
	MechS32 m_label; // 0x0c
};

// SIZE 0x30
// A formation's three positions.
struct CopperFormation0x30 {
	CopperPost0x10 m_posts[3]; // 0x00
};

DECOMP_SIZE_ASSERT(SableRoster0x24, 0x24)
DECOMP_SIZE_ASSERT(HazelStar0x80, 0x80)
DECOMP_SIZE_ASSERT(AshCord0x08, 0x08)
DECOMP_SIZE_ASSERT(CopperPost0x10, 0x10)
DECOMP_SIZE_ASSERT(CopperFormation0x30, 0x30)
DECOMP_SIZE_ASSERT(GraniteMast0x18, 0x18)
DECOMP_SIZE_ASSERT(TallowSign0x10, 0x10)
DECOMP_SIZE_ASSERT(MainMenuButton, 0x1c)

MechS32 FUN_10017460(
	MechS32 p_index,
	const char* p_name,
	undefined4 p_unk0x08,
	undefined4 p_unk0x0c,
	MechU32 p_unk0x10,
	MechU32 p_unk0x14
);

// The formations of the Wolf and Inner Sphere stars (Jade Falcon's differ): six formations of
// three mechs.
// GLOBAL: MW2SHELL 0x1005b4c0
CopperFormation0x30 g_unk0x1005b4c0[6] = {
	{{{2, 430, 346, 2}, {1, 333, 353, 1}, {0, 204, 359, 0}}},
	{{{2, 295, 330, 0}, {1, 332, 352, 1}, {0, 385, 395, 2}}},
	{{{2, 245, 341, 0}, {0, 343, 355, 1}, {1, 418, 363, 2}}},
	{{{2, 390, 332, 2}, {1, 350, 355, 1}, {0, 277, 374, 0}}},
	{{{0, 390, 332, 1}, {2, 204, 358, 0}, {1, 385, 395, 2}}},
	{{{2, 295, 330, 0}, {1, 430, 346, 2}, {0, 288, 376, 1}}},
};

// GLOBAL: MW2SHELL 0x1005b5e0
CopperFormation0x30 g_unk0x1005b5e0[6] = {
	{{{2, 425, 345, 2}, {1, 325, 354, 1}, {0, 181, 362, 0}}},
	{{{2, 297, 329, 0}, {1, 325, 354, 1}, {0, 377, 408, 2}}},
	{{{2, 251, 341, 0}, {0, 330, 350, 1}, {1, 410, 366, 2}}},
	{{{2, 373, 338, 2}, {1, 338, 357, 1}, {0, 258, 385, 0}}},
	{{{0, 373, 338, 1}, {2, 181, 362, 0}, {1, 377, 408, 2}}},
	{{{2, 297, 329, 0}, {1, 425, 345, 2}, {0, 258, 385, 1}}},
};

// GLOBAL: MW2SHELL 0x1005b700
MechChar g_unk0x1005b700[] = "awosc%s";

// GLOBAL: MW2SHELL 0x1005b708
MechChar g_unk0x1005b708[] = "ajfsc%s";

// GLOBAL: MW2SHELL 0x1005b710
MechChar g_unk0x1005b710[] = "aiasc%s";

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

// Where the name labels go, by position: left edges and tops, for the Wolf and Inner Sphere
// screens and for the Jade Falcon one.
// GLOBAL: MW2SHELL 0x1005b850
MechS32 g_unk0x1005b850[4] = {81, 313, 508, 0};

// GLOBAL: MW2SHELL 0x1005b860
MechS32 g_unk0x1005b860[4] = {166, 133, 189, 0};

// GLOBAL: MW2SHELL 0x1005b870
MechS32 g_unk0x1005b870[4] = {47, 280, 495, 0};

// GLOBAL: MW2SHELL 0x1005b880
MechS32 g_unk0x1005b880[3] = {172, 127, 217};

// The formation, mission, star size, tonnage limit and star mass lines.
// GLOBAL: MW2SHELL 0x1005b88c
EmberGlyph0x3e* g_unk0x1005b88c = NULL;

// GLOBAL: MW2SHELL 0x1005b890
EmberGlyph0x3e* g_unk0x1005b890 = NULL;

// GLOBAL: MW2SHELL 0x1005b894
EmberGlyph0x3e* g_unk0x1005b894 = NULL;

// GLOBAL: MW2SHELL 0x1005b898
EmberGlyph0x3e* g_unk0x1005b898 = NULL;

// GLOBAL: MW2SHELL 0x1005b89c
EmberGlyph0x3e* g_unk0x1005b89c = NULL;

// GLOBAL: MW2SHELL 0x1005b8a0
MechS32 g_unk0x1005b8a0 = 0;

// Per position: the name, type and mass glyphs.
// GLOBAL: MW2SHELL 0x10079438
EmberGlyph0x3e* g_unk0x10079438[3][3];

// The mass of each mech of the star.
// GLOBAL: MW2SHELL 0x10079460
MechS32 g_unk0x10079460[3];

// GLOBAL: MW2SHELL 0x1007946c
CopperFormation0x30* g_unk0x1007946c;

// GLOBAL: MW2SHELL 0x10079470
AudioSample* g_unk0x10079470;

// GLOBAL: MW2SHELL 0x10079478
MechChar g_unk0x10079478[0x100];

// GLOBAL: MW2SHELL 0x10079578
MechS32* g_unk0x10079578;

// GLOBAL: MW2SHELL 0x1007957c
MechS32* g_unk0x1007957c;

// GLOBAL: MW2SHELL 0x10079580
AudioSample* g_unk0x10079580;

// GLOBAL: MW2SHELL 0x10079584
MenuList0x10d* g_unk0x10079584;

// GLOBAL: MW2SHELL 0x10079588
MechChar g_unk0x10079588[0x100];

// GLOBAL: MW2SHELL 0x10079688
MechChar* g_unk0x10079688;

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

// Sets a mech of the selected star: its pilot name and its variant file. A three-letter variant
// name is the mech's standard variant. Returns 0 when the mech is too heavy for the star.
// FUNCTION: MW2SHELL 0x10002de7
MechS32 FUN_10002de7(MechS32 p_index, MechChar* p_variant, MechChar* p_name)
{
	MechS32 type;

	if (p_index < 0) {
		p_index = g_unk0x1005b818->m_unk0x04;
	}
	else {
		g_unk0x1005b818->m_unk0x04 = p_index;
	}

	if (p_name) {
		strcpy(g_unk0x1005b818->m_unk0x14[p_index].m_unk0x14, p_name);
	}

	if (p_variant) {
		for (type = 0; g_unk0x10061560[type].m_unk0x04; type++) {
			if (!_strnicmp(p_variant, g_unk0x10061560[type].m_unk0x04, 3)) {
				break;
			}
		}

		if (!g_unk0x10061560[type].m_unk0x04) {
			if (p_index && g_unk0x1005b818->m_unk0x0c == p_index + 1) {
				g_unk0x1005b818->m_unk0x0c--;
				return -1;
			}
			else if (g_unk0x1005b818->m_unk0x0c <= p_index) {
				return 1;
			}
			else {
				return 0;
			}
		}

		if (g_unk0x10061560[type].m_unk0x10 > g_unk0x1005b818->m_unk0x10) {
			return 0;
		}

		strncpy(g_unk0x1005b818->m_unk0x14[p_index].m_unk0x04, p_variant, 8);
		g_unk0x1005b818->m_unk0x14[p_index].m_unk0x04[8] = '\0';
		if (strlen(p_variant) == 3) {
			strcat(g_unk0x1005b818->m_unk0x14[p_index].m_unk0x04, "00std");
		}
		g_unk0x1005b818->m_unk0x14[p_index].m_unk0x00 = type;
	}

	if (g_unk0x1005b818->m_unk0x0c == p_index && g_unk0x1005b818->m_unk0x08 > g_unk0x1005b818->m_unk0x0c) {
		g_unk0x1005b818->m_unk0x0c++;
	}

	return -1;
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

// Selects a star (negative arguments leave the setting alone) and sets its formation, its size,
// its mech count and its tonnage limit.
// FUNCTION: MW2SHELL 0x10003175
void FUN_10003175(MechS32 p_star, MechS32 p_formation, MechS32 p_size, MechS32 p_count, MechS32 p_tonnage)
{
	if (p_star >= 0) {
		if (p_star == 0) {
			g_unk0x1005b818 = &g_unk0x1005b718;
		}
		else {
			g_unk0x1005b818 = &g_unk0x1005b798;
		}
	}

	if (p_formation >= 0) {
		g_unk0x1005b818->m_unk0x00 = p_formation;
	}

	if (p_size >= 0) {
		g_unk0x1005b818->m_unk0x08 = p_size < 3 ? p_size : 3;
	}

	if (p_count >= 0) {
		g_unk0x1005b818->m_unk0x0c = p_count < 3 ? p_count : 3;
	}

	if (p_tonnage >= 0) {
		g_unk0x1005b818->m_unk0x10 = p_tonnage;
	}
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

#define STAR_LAYOUT(formation, i) g_unk0x1007946c[formation].m_posts[i]

// Returns the mech whose video is under a point, or -1.
// Stack-slot permutation: mech, left, right, i, top and bottom.
// FUNCTION: MW2SHELL 0x10003320
MechS32 FUN_10003320(MechS32 p_x, MechS32 p_y)
{
	MechS32 mech;
	MechS32 left;
	MechS32 right;
	MechS32 i;
	MechS32 top;
	MechS32 bottom;

	for (i = 2; i >= 0; i--) {
		left = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, i).m_left - 0x32;
		right = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, i).m_left + 0x32;
		top = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, i).m_top - 100;
		bottom = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, i).m_top;
		if (left <= p_x && right >= p_x && top <= p_y && bottom >= p_y) {
			mech = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, i).m_mech;
			if (g_unk0x1005b818->m_unk0x14[mech].m_unk0x00 >= 0) {
				return mech;
			}
		}
	}

	return -1;
}

// Returns as much of p_text as fits in p_width pixels of p_font.
// Stack-slot permutation: out and width. The original compares p_width against width; the
// declaration order doesn't flip it.
// FUNCTION: MW2SHELL 0x1000345a
MechChar* FUN_1000345a(MechChar* p_text, BrassLantern0x414* p_font, MechS32 p_width)
{
	MechChar* out = g_unk0x10079588;
	MechS32 width = 0;

	if (p_text == NULL) {
		return p_text;
	}

	while (*p_text) {
		width += p_font->FUN_10005424(*p_text);
		if (width > p_width) {
			break;
		}
		*out++ = *p_text++;
	}
	*out = '\0';

	return g_unk0x10079588;
}

// A click on a pilot name edits it.
// Stack-slot permutation: i, mech, label, left and top. The original loads left and top ahead of
// p_x and p_y in the bounds test; the declaration order doesn't flip it.
// FUNCTION: MW2SHELL 0x100034de
void FUN_100034de(MechS32 p_x, MechS32 p_y, MechS32 p_campaign)
{
	MechS32 i;
	MechS32 mech;
	MechS32 label;
	MechS32 left;
	MechS32 top;

	for (i = 0; i < 3; i++) {
		mech = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, i).m_mech;
		if (mech > g_unk0x1005b818->m_unk0x0c) {
			continue;
		}
		if (mech == 0 && p_campaign != 2) {
			continue;
		}

		label = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, i).m_label;
		left = g_unk0x1007957c[label];
		top = g_unk0x10079578[label];
		if (left <= p_x && left + 100 > p_x && top <= p_y && top + 10 > p_y) {
			if (g_unk0x10079438[i][0]) {
				delete g_unk0x10079438[i][0];
			}

			FUN_10044451(g_unk0x1007120c, left, top, g_unk0x1005b818->m_unk0x14[mech].m_unk0x14, NULL, 0xf, 100);
			g_unk0x10079438[i][0] =
				g_unk0x10071210->FUN_10005522(left, top, g_unk0x1005b818->m_unk0x14[mech].m_unk0x14, NULL);
			return;
		}
	}
}

// Shows the mech at a formation position: its video, pilot name, type and mass, or hides the
// position when the star has fewer mechs.
// Stack-slot permutation: label, type and mech.
// FUNCTION: MW2SHELL 0x10003690
void FUN_10003690(MechS32 p_formation, MechS32 p_position, MenuList0x10d* p_menu)
{
	MechS32 label;
	MechS32 type;
	MechS32 left;
	MechChar name[0x10];
	MechS32 top;
	MechS32 mech;

	if (g_unk0x10079438[p_position][0]) {
		delete g_unk0x10079438[p_position][0];
	}
	if (g_unk0x10079438[p_position][1]) {
		delete g_unk0x10079438[p_position][1];
	}
	if (g_unk0x10079438[p_position][2]) {
		delete g_unk0x10079438[p_position][2];
	}
	g_unk0x10079438[p_position][0] = NULL;
	g_unk0x10079438[p_position][1] = NULL;
	g_unk0x10079438[p_position][2] = NULL;

	label = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, p_position).m_label;
	mech = STAR_LAYOUT(p_formation, p_position).m_mech;
	p_menu->FUN_10048d65(label + 6);
	if (mech >= g_unk0x1005b818->m_unk0x0c) {
		FUN_10016cc0(p_position + 10, 0x40000000, 0x40000000);
		FUN_10016cc0(label + 1, 0x20, 0x20);
		return;
	}

	FUN_10016d27(label + 1);
	p_menu->FUN_10048cc1(label + 6);

	type = g_unk0x1005b818->m_unk0x14[mech].m_unk0x00;
	left = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, p_position).m_left;
	top = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, p_position).m_top;
	sprintf(name, g_unk0x10079688, g_unk0x10061560[type].m_unk0x00);
	FUN_10017460(p_position + 10, name, left, top, 0x88, 0);

	left = g_unk0x1007957c[label];
	top = g_unk0x10079578[label];
	strcpy(
		g_unk0x1005b818->m_unk0x14[mech].m_unk0x14,
		FUN_1000345a(g_unk0x1005b818->m_unk0x14[mech].m_unk0x14, g_unk0x10071210, 0x5c)
	);
	g_unk0x10079438[p_position][0] =
		g_unk0x10071210->FUN_10005522(left, top, g_unk0x1005b818->m_unk0x14[mech].m_unk0x14, NULL);
	g_unk0x10079438[p_position][1] =
		g_unk0x10071210->FUN_10005522(left, top + 0xc, g_unk0x10061560[type].m_unk0x0c, NULL);

	g_unk0x10079460[mech] = g_unk0x10061560[type].m_unk0x10;
	sprintf(name, "%d.00 T", g_unk0x10061560[type].m_unk0x10);
	g_unk0x10079438[p_position][2] = g_unk0x10071210->FUN_10005522(left, top + 0x18, name, NULL);
}

// Redraws the formation, mission, star size, tonnage limit and star mass lines.
// Stack-slot permutation: i and mass.
// FUNCTION: MW2SHELL 0x10003a4e
void FUN_10003a4e(MechS32 p_campaign)
{
	MechS32 i;
	MechS32 mass;

	if (g_unk0x1005b88c) {
		delete g_unk0x1005b88c;
	}
	g_unk0x1005b88c =
		g_unk0x10071214->FUN_1000544e(0x140, 4, g_unk0x1005b820[g_unk0x1005b818->m_unk0x00].m_unk0x00, NULL);

	if (g_unk0x1005b890) {
		delete g_unk0x1005b890;
	}
	if (p_campaign == 2) {
		strcpy(g_unk0x10079478, "~Mission: Trial of Grievance");
	}
	else {
		sprintf(g_unk0x10079478, "~Mission: %s", g_campaignMissions[p_campaign][g_pCurrentPilot->m_mission].m_title);
	}
	g_unk0x1005b890 = g_unk0x10071210->FUN_1000544e(0x140, 0x23, g_unk0x10079478, NULL);

	if (g_unk0x1005b894) {
		delete g_unk0x1005b894;
	}
	sprintf(g_unk0x10079478, "~Maximum 'Mechs in current Star: %d", g_unk0x1005b818->m_unk0x08);
	g_unk0x1005b894 = g_unk0x10071210->FUN_1000544e(0x140, 0x32, g_unk0x10079478, NULL);

	if (g_unk0x1005b898) {
		delete g_unk0x1005b898;
	}
	sprintf(g_unk0x10079478, "~Keshik Defined Maximum Tonnage (KDMT) per 'Mech: %d.00 T", g_unk0x1005b818->m_unk0x10);
	g_unk0x1005b898 = g_unk0x10071210->FUN_1000544e(0x140, 0x41, g_unk0x10079478, NULL);

	if (g_unk0x1005b89c) {
		delete g_unk0x1005b89c;
	}
	mass = 0;
	for (i = 0; i < g_unk0x1005b818->m_unk0x0c; i++) {
		mass += g_unk0x10079460[i];
	}
	sprintf(g_unk0x10079478, "~Current Total Mass of the Star: %d.00 T", mass);
	g_unk0x1005b89c = g_unk0x10071210->FUN_1000544e(0x140, 0x50, g_unk0x10079478, NULL);
}

void FUN_100043c2(TMPackDataBase*, MechS32* p_campaign, MechU8*, char**, MechS32 p_msg);

// Opens the star screen of a campaign (2: a trial of grievance).
// FUNCTION: MW2SHELL 0x10003d3a
void FUN_10003d3a(TMPackDataBase* p_database, MechS32 p_campaign)
{
	void* audioData;
	MechS32 i;
	MechS32 audioSize;

	g_pVideoDriver->FUN_10006c50(p_database, g_unk0x1006fea0[p_campaign].m_picture);
	g_unk0x10079584 = new MenuList0x10d(g_pVideoDriver, g_unk0x1007120c, 0, g_unk0x1006fea0[p_campaign].m_buttons, 9);

	switch (p_campaign) {
	case 0:
		FUN_10017460(0, "awogrid", 0x8c, 0x136, 0x4a, 0);
		FUN_10017460(1, "awostr1", 0x49, 0x9e, 0x44, 0);
		FUN_10017460(2, "awostr2", 0x131, 0x7d, 0x44, 0);
		FUN_10017460(3, "awostr3", 500, 0xb5, 0x44, 0);
		FUN_10017460(4, "wwobkg", 0xdb, 0x1ad, 2, 0);
		FUN_10017460(5, "wwodsgn", 0x197, 0x19a, 0x4a, 0);
		FUN_10017460(6, "wwocn", 0x122, 0x1b1, 100, 0);
		FUN_10017460(7, "wwocp", 0xf0, 0x1b3, 100, 0);
		FUN_10017460(8, "wwovn", 300, 0x1ad, 100, 0);
		FUN_10017460(9, "wwovp", 0xdc, 0x1b0, 100, 0);
		g_unk0x1007946c = g_unk0x1005b4c0;
		g_unk0x10079688 = g_unk0x1005b700;
		g_unk0x1007957c = g_unk0x1005b850;
		g_unk0x10079578 = g_unk0x1005b860;
		break;
	case 1:
		FUN_10017460(0, "ajfgrid", 0x6c, 0x132, 0x4a, 0);
		FUN_10017460(1, "ajfstr1", 0x27, 0xa4, 0x44, 0);
		FUN_10017460(2, "ajfstr2", 0x110, 0x77, 0x44, 0);
		FUN_10017460(3, "ajfstr3", 0x1e7, 0xd1, 0x44, 0);
		FUN_10017460(4, "wjfbkg", 0xd8, 0x1b2, 2, 0);
		FUN_10017460(5, "wjfdsgn", 0x171, 0x1a0, 0x48, 0);
		FUN_10017460(6, "wjfcn", 0x11e, 0x1b6, 100, 0);
		FUN_10017460(7, "wjfcp", 0xf2, 0x1b6, 100, 0);
		FUN_10017460(8, "wjfvn", 0x128, 0x1b2, 100, 0);
		FUN_10017460(9, "wjfvp", 0xdc, 0x1b5, 100, 0);
		g_unk0x1007946c = g_unk0x1005b5e0;
		g_unk0x10079688 = g_unk0x1005b708;
		g_unk0x1007957c = g_unk0x1005b870;
		g_unk0x10079578 = g_unk0x1005b880;
		break;
	case 2:
		FUN_10017460(0, "aiagrid", 0x6c, 0x132, 0x4a, 0);
		FUN_10017460(1, "aiastr1", 0x49, 0x9e, 0x44, 0);
		FUN_10017460(2, "aiastr1", 0x131, 0x7d, 0x44, 0);
		FUN_10017460(3, "aiastr1", 500, 0xb5, 0x44, 0);
		FUN_10017460(4, "wiabkg2", 0xdb, 0x19e, 4, 0);
		FUN_10017460(5, "wiadsgn", 0x19f, 0x1a1, 0x24, 0);
		FUN_10017460(6, "wiacn", 0x109, 0x1ae, 0x24, 0);
		FUN_10017460(7, "wiacp", 0xee, 0x1b5, 0x24, 0);
		FUN_10017460(8, "wiavn", 0x12e, 0x1af, 0x24, 0);
		FUN_10017460(9, "wiavp", 0xdf, 0x1b5, 0x24, 0);
		g_unk0x1007946c = g_unk0x1005b4c0;
		g_unk0x10079688 = g_unk0x1005b710;
		g_unk0x1007957c = g_unk0x1005b850;
		g_unk0x10079578 = g_unk0x1005b860;
		break;
	}

	for (i = 0; i < 3; i++) {
		g_unk0x10079438[i][0] = NULL;
		g_unk0x10079438[i][1] = NULL;
		g_unk0x10079438[i][2] = NULL;
		FUN_10003690(g_unk0x1005b818->m_unk0x00, i, g_unk0x10079584);
	}

	g_unk0x1005b88c = NULL;
	g_unk0x1005b890 = NULL;
	g_unk0x1005b894 = NULL;
	g_unk0x1005b898 = NULL;
	g_unk0x1005b89c = NULL;

	g_pDatabaseMw2->GetDBItem(101, &audioData, &audioSize);
	g_unk0x10079580 = new AudioSample(g_pAudioSubsystem, audioData, audioSize);
	g_unk0x10079580->SetVolume(0x32);
	g_pDatabaseMw2->GetDBItem(102, &audioData, &audioSize);
	g_unk0x10079470 = new AudioSample(g_pAudioSubsystem, audioData, audioSize);
	g_unk0x10079470->SetVolume(0x32);

	FUN_10003a4e(p_campaign);
	FUN_100108e5(FUN_100043c2);
}

// The star screen's buttons: 0 exits, 1 opens the mech lab, 2 and 3 cycle the formation, 4 and 5
// add and remove a mech, 6 to 8 change the mech at a label position. A double click on a mech
// changes it too.
// Stack-slot permutation: i, button and mech.
// FUNCTION: MW2SHELL 0x100043c2
void FUN_100043c2(TMPackDataBase*, MechS32* p_campaign, MechU8*, char**, MechS32 p_msg)
{
	MechS32 i;
	MechS32 button;
	MechS32 mech;

	// The original skips the frame's work with a goto: the jump it compiles to leaves a stub
	// after the function's end.
	if (p_msg != 0x404) {
		goto done;
	}

	if (g_fQuickTips && !g_unk0x1005b8a0) {
		FUN_1001661b();
		g_pVideoDriver->DrawShell();
		DialogBoxParam(g_pModule, MAKEINTRESOURCE(0x7c), g_pWnd, (DLGPROC) FUN_1001067f, 0);
		g_unk0x1005b8a0 = 1;
	}

	button = g_unk0x10079584->FUN_100489e9(g_pMouseState->m_x, g_pMouseState->m_y);
	if (*p_campaign == 2) {
		FUN_10016cc0(5, 0x20, 0x20);
	}
	else {
		FUN_10016cc0(5, 1, 1);
	}
	FUN_10016cc0(6, 0x20, 0x20);
	FUN_10016cc0(7, 0x20, 0x20);
	FUN_10016cc0(8, 0x20, 0x20);
	FUN_10016cc0(9, 0x20, 0x20);

	switch (button) {
	case 0:
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		p_msg = 0x411;
		break;
	case 1:
		if (*p_campaign == 2) {
			FUN_10016d27(5);
		}
		else {
			FUN_10016cc0(5, 1, 0);
		}
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		g_unk0x10079470->Start();
		g_unk0x10061774 = 0;
		p_msg = 0x40f;
		break;
	case 2:
		if (g_pMouseState->m_leftDown == 1) {
			FUN_10016d27(6);
		}
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		g_unk0x10079580->Start();
		g_unk0x1005b818->m_unk0x00++;
		if (g_unk0x1005b818->m_unk0x00 >= 6) {
			g_unk0x1005b818->m_unk0x00 = 0;
		}
		for (i = 0; i < 3; i++) {
			FUN_10003690(g_unk0x1005b818->m_unk0x00, i, g_unk0x10079584);
		}
		FUN_10003a4e(*p_campaign);
		break;
	case 3:
		if (g_pMouseState->m_leftDown == 1) {
			FUN_10016d27(7);
		}
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		g_unk0x10079580->Start();
		g_unk0x1005b818->m_unk0x00--;
		if (g_unk0x1005b818->m_unk0x00 < 0) {
			g_unk0x1005b818->m_unk0x00 = 5;
		}
		for (i = 0; i < 3; i++) {
			FUN_10003690(g_unk0x1005b818->m_unk0x00, i, g_unk0x10079584);
		}
		FUN_10003a4e(*p_campaign);
		break;
	case 4:
		if (g_pMouseState->m_leftDown == 1) {
			FUN_10016d27(8);
		}
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		g_unk0x10079580->Start();
		if (g_unk0x1005b818->m_unk0x0c < g_unk0x1005b818->m_unk0x08) {
			g_unk0x1005b818->m_unk0x0c++;
		}
		for (i = 0; i < 3; i++) {
			FUN_10003690(g_unk0x1005b818->m_unk0x00, i, g_unk0x10079584);
		}
		FUN_10003a4e(*p_campaign);
		break;
	case 5:
		if (g_pMouseState->m_leftDown == 1) {
			FUN_10016d27(9);
		}
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		g_unk0x10079580->Start();
		if (g_unk0x1005b818->m_unk0x0c > 1) {
			g_unk0x1005b818->m_unk0x0c--;
		}
		for (i = 0; i < 3; i++) {
			FUN_10003690(g_unk0x1005b818->m_unk0x00, i, g_unk0x10079584);
		}
		FUN_10003a4e(*p_campaign);
		break;
	case 6:
	case 7:
	case 8:
		if (g_pMouseState->GetLeftPressed() != 1) {
			break;
		}
		g_unk0x10079580->Start();
		for (i = 0; i < 3; i++) {
			if (STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, i).m_label == button - 6) {
				g_unk0x1005b818->m_unk0x04 = STAR_LAYOUT(g_unk0x1005b818->m_unk0x00, i).m_mech;
				g_unk0x10061774 = 1;
				p_msg = 0x40f;
			}
		}
		break;
	}

	if (g_pMouseState->GetLeftPressed() == 1) {
		FUN_100034de(g_pMouseState->m_x, g_pMouseState->m_y, *p_campaign);
	}

	if (g_pMouseState->GetDoubleClicked() && (mech = FUN_10003320(g_pMouseState->m_x, g_pMouseState->m_y)) >= 0) {
		g_unk0x1005b818->m_unk0x04 = mech;
		g_unk0x10061774 = 1;
		p_msg = 0x40f;
	}

done:
	if (p_msg != 0x404) {
		delete g_unk0x10079584;
		delete g_unk0x10079470;
		delete g_unk0x10079580;
		FUN_10016f45();
		g_pVideoDriver->FUN_100077b4(TRUE);
		g_unk0x1005b8a0 = 0;
		PostMessage(g_pWnd, p_msg, 0x413, 0);
		FUN_100108fd(FUN_100043c2);
	}
}

#undef STAR_LAYOUT
