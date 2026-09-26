#include "brasslantern0x414.h"
#include "decomp.h"
#include "emberglyph0x3e.h"
#include "slatetab0x2c.h"
#include "tmpackdatabase.h"
#include "types.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

// The mech bay: the variant being edited, its engine, weapons and armor, and the fields that
// show them.

// An engine: its rating, its weight (in 1/100 t) and its maker. Ids from 10000 up are XL
// engines at half the weight.
// SIZE 0x0c
struct CobaltSpur0x0c {
	MechS32 m_rating; // 0x00
	MechS32 m_weight; // 0x04
	MechChar* m_name; // 0x08
};

// A weapon; the weight (in 1/100 t) is at 0x18. The table holds negative values in the fields
// up to 0x14, so those are signed.
// SIZE 0x28
struct TinLattice0x28 {
	MechS32 m_unk0x00;    // 0x00
	MechS32 m_unk0x04;    // 0x04
	MechS32 m_unk0x08;    // 0x08
	MechS32 m_unk0x0c;    // 0x0c
	MechS32 m_unk0x10;    // 0x10
	MechS32 m_unk0x14;    // 0x14
	MechS32 m_unk0x18;    // 0x18
	undefined4 m_unk0x1c; // 0x1c
	MechS32 m_unk0x20;    // 0x20 — rounds per ton of ammo, 0 for weapons without ammo
	MechChar* m_name;     // 0x24
};

// SIZE 0xf50
// The variant being edited. 0x10007e90 copies an m_unk0x4a0 entry through the struct base
// (0x1005c640 + 0x4a0), which places the start. The size is a bound, not a proven size: the
// engine table follows at 0x1005d590.
struct QuartzHelm0xf50 {
	// SIZE 0x08
	struct Slot {
		MechS32 m_unk0x00; // 0x00 — item id, -1 when free
		MechS32 m_unk0x04; // 0x04
	};

	// SIZE 0x10
	// One location's armor. The tab callbacks index the array through its base at 0x710.
	struct Armor {
		undefined4 m_unk0x00; // 0x00
		MechS32 m_unk0x04;    // 0x04
		MechS32 m_unk0x08;    // 0x08 — front armor
		MechS32 m_unk0x0c;    // 0x0c — rear armor, negative when the location has none
	};

	undefined4 m_unk0x00;                // 0x00
	undefined m_unk0x04[0x100 - 0x04];   // 0x04
	MechChar m_unk0x100[0x40];           // 0x100 — variant name
	undefined m_unk0x140[0x200 - 0x140]; // 0x140
	MechS32 m_unk0x200;                  // 0x200 — maximum weight
	MechS32 m_unk0x204;                  // 0x204 — total weight
	MechS32 m_unk0x208;                  // 0x208 — weight left
	MechS32 m_unk0x20c;                  // 0x20c — engine id
	MechS32 m_unk0x210;                  // 0x210 — engine rating
	MechS32 m_unk0x214;                  // 0x214 — engine weight
	MechS32 m_unk0x218;                  // 0x218
	MechS32 m_unk0x21c;                  // 0x21c
	MechS32 m_unk0x220;                  // 0x220
	MechS32 m_unk0x224;                  // 0x224
	MechS32 m_unk0x228;                  // 0x228
	MechS32 m_unk0x22c;                  // 0x22c
	MechS32 m_unk0x230;                  // 0x230
	MechS32 m_unk0x234;                  // 0x234
	MechS32 m_unk0x238;                  // 0x238
	undefined4 m_unk0x23c;               // 0x23c
	MechS32 m_unk0x240;                  // 0x240
	MechS32 m_unk0x244;                  // 0x244
	undefined4 m_unk0x248;               // 0x248
	MechS32 m_unk0x24c;                  // 0x24c
	MechS32 m_unk0x250;                  // 0x250
	undefined4 m_unk0x254;               // 0x254
	MechS32 m_unk0x258;                  // 0x258
	MechS32 m_unk0x25c;                  // 0x25c
	MechS32 m_unk0x260;                  // 0x260
	undefined4 m_unk0x264;               // 0x264
	MechS32 m_unk0x268;                  // 0x268
	undefined4 m_unk0x26c;               // 0x26c
	MechS32 m_unk0x270;                  // 0x270
	undefined4 m_unk0x274;               // 0x274
	MechS32 m_unk0x278;                  // 0x278
	MechS32 m_unk0x27c;                  // 0x27c
	MechS32 m_unk0x280;                  // 0x280
	MechS32 m_unk0x284[25];              // 0x284 — ids, -1 past the last
	MechS32 m_unk0x2e8[10];              // 0x2e8 — ids, -1 past the last
	MechS32 m_unk0x310;                  // 0x310 — highlighted item id, -1 for none
	MechS32 m_unk0x314;                  // 0x314 — selected location
	MechS32 m_unk0x318[8][12];           // 0x318 — item ids per location and slot, 0 when free
	undefined4 m_unk0x498;               // 0x498
	MechS32 m_unk0x49c;                  // 0x49c — used entries of m_unk0x4a0
	Slot m_unk0x4a0[78];                 // 0x4a0
	Armor m_unk0x710[8];                 // 0x710
	undefined m_unk0x790[0xf50 - 0x790]; // 0x790
};

DECOMP_SIZE_ASSERT(SlateTab0x2c, 0x2c)
DECOMP_SIZE_ASSERT(CobaltSpur0x0c, 0x0c)
DECOMP_SIZE_ASSERT(TinLattice0x28, 0x28)
DECOMP_SIZE_ASSERT(QuartzHelm0xf50::Slot, 0x08)
DECOMP_SIZE_ASSERT(QuartzHelm0xf50::Armor, 0x10)
DECOMP_SIZE_ASSERT(QuartzHelm0xf50, 0xf50)

extern BrassLantern0x414* g_unk0x1007120c;

void FUN_10016d27(MechS32 p_index);
void FUN_10016f82(MechS32 p_index, MechS32 p_left, MechS32 p_top);
void FUN_10017698(MechS32 p_index, MechS32 p_frame);

// GLOBAL: MW2SHELL 0x1005c4f0
MechChar* g_unk0x1005c4f0[8] =
	{"Head", "Right Torso", "Center Torso", "Left Torso", "Right Arm", "Left Arm", "Right Leg", "Left Leg"};

// GLOBAL: MW2SHELL 0x1005c640
QuartzHelm0xf50 g_unk0x1005c640 = {0};

// GLOBAL: MW2SHELL 0x1005d590
CobaltSpur0x0c g_unk0x1005d590[] = {
	{10, 50, "Omni"},      {15, 50, "GM"},          {20, 50, "Pitban"},    {25, 50, "Omni"},
	{30, 100, "Nissan"},   {35, 100, "VOX"},        {40, 100, "GM"},       {45, 100, "GM"},
	{50, 150, "DAV"},      {55, 150, "VOX"},        {60, 150, "Leenex"},   {65, 200, "Nissan"},
	{70, 200, "Omni"},     {75, 200, "GM"},         {80, 250, "VOX"},      {85, 250, "DAV"},
	{90, 300, "DAV"},      {95, 300, "Nissan"},     {100, 300, "Hermes"},  {105, 350, "DAV"},
	{110, 350, "GM"},      {115, 400, "GM"},        {120, 400, "GM"},      {125, 400, "Vlar"},
	{130, 450, "Magna"},   {135, 450, "Hermes"},    {140, 500, "Leenex"},  {145, 500, "Omni"},
	{150, 550, "GM"},      {155, 550, "GM"},        {160, 600, "LTV"},     {165, 600, "VOX"},
	{170, 600, "DAV"},     {175, 700, "Omni"},      {180, 700, "GM"},      {185, 750, "GM"},
	{190, 750, "DAV"},     {195, 800, "Nissan"},    {200, 850, "Nissan"},  {205, 850, "Vlar"},
	{210, 900, "GM"},      {215, 850, "Core Tex"},  {220, 1000, "DAV"},    {225, 1000, "VOX"},
	{230, 1050, "Leenex"}, {235, 1100, "GM"},       {240, 1150, "Pitban"}, {245, 1200, "Magna"},
	{250, 1250, "Magna"},  {255, 1300, "Strand"},   {260, 1350, "Magna"},  {265, 1400, "Vlar"},
	{270, 1450, "GM"},     {275, 1550, "Core Tex"}, {280, 1600, "VOX"},    {285, 1650, "Pitban"},
	{290, 1750, "Omni"},   {295, 1800, "GM"},       {300, 1900, "Vlar"},   {305, 1950, "GM"},
	{310, 2050, "Magna"},  {315, 2150, "GM"},       {320, 2250, "Pitban"}, {325, 2350, "VOX"},
	{330, 2400, "VOX"},    {335, 2550, "Leenex"},   {340, 2700, "VOX"},    {345, 2850, "Vlar"},
	{350, 2950, "Magna"},  {355, 3150, "LTV"},      {360, 3300, "Hermes"}, {365, 3450, "Hermes"},
	{370, 3650, "Magna"},  {375, 3850, "GM"},       {380, 4100, "GM"},     {385, 4350, "LTV"},
	{390, 4600, "Magna"},  {395, 4900, "Hermes"},   {400, 5250, "LTV"},    {-1, 0, NULL},
};

// GLOBAL: MW2SHELL 0x1005d950
TinLattice0x28 g_unk0x1005d950[] = {
	{6, -1, -1, 7, 14, 1000, 500, 4, 6, "LRM 20"},
	{5, -1, -1, 7, 14, 1000, 350, 2, 8, "LRM 15"},
	{4, -1, -1, 7, 14, 1000, 250, 1, 12, "LRM 10"},
	{2, -1, -1, 7, 14, 1000, 100, 1, 24, "LRM 5"},
	{4, -2, -1, 3, 6, 497, 150, 1, 15, "SRM 6"},
	{3, -2, -1, 3, 6, 497, 100, 1, 25, "SRM 4"},
	{2, -2, -1, 3, 6, 497, 50, 1, 50, "SRM 2"},
	{4, -2, -1, 4, 8, 497, 300, 2, 15, "Streak SRM-6"},
	{3, -2, -1, 4, 8, 497, 200, 1, 25, "Streak SRM-4"},
	{2, -2, -1, 4, 8, 497, 100, 1, 50, "Streak SRM-2"},
	{0, 2, -1, 1, 2, 175, 25, 1, 200, "Machine Gun"},
	{1, 15, 2, 7, 15, 1820, 1200, 6, 8, "Gauss Rifle"},
	{1, 2, 4, 10, 20, 800, 500, 3, 45, "LB 2-X AC"},
	{1, 5, 3, 8, 15, 700, 700, 4, 20, "LB 5-X AC"},
	{2, 10, -1, 6, 12, 600, 1000, 5, 10, "LB 10-X AC"},
	{6, 20, -1, 4, 8, 450, 1200, 9, 5, "LB 20-X AC"},
	{1, 2, 2, 9, 18, 700, 500, 2, 45, "Ultra AC/2"},
	{1, 5, -1, 7, 14, 600, 700, 3, 20, "Ultra AC/5"},
	{3, 10, -1, 6, 12, 500, 1000, 4, 10, "Ultra AC/10"},
	{7, 20, -1, 4, 8, 400, 1200, 8, 5, "Ultra AC/20"},
	{3, 2, -1, 1, 2, 3, 50, 1, 0, "Flamer"},
	{15, 15, -1, 7, 14, 746, 600, 2, 0, "ER PPC"},
	{12, 10, -1, 8, 15, 1019, 400, 1, 0, "ER Laser (Large)"},
	{5, 7, -1, 5, 10, 510, 100, 1, 0, "ER Laser (Medium)"},
	{2, 5, -1, 2, 4, 255, 50, 1, 0, "ER Laser (Small)"},
	{10, 10, -1, 6, 14, 815, 600, 2, 0, "Pulse Laser (Large)"},
	{4, 7, -1, 4, 8, 408, 200, 1, 0, "Pulse Laser (Medium)"},
	{2, 3, -1, 2, 4, 204, 100, 1, 0, "Pulse Laser (Small)"},
	{0, 0, -1, 4, 8, 12, 200, 1, 0, "Narc Missile Beacon"},
	{1, 0, -1, -1, -1, -1, 50, 1, 0, "Anti-Missile System"},
	{0, 0, -1, -1, -1, -1, 0, 1, 0, "Nuke"},
};

// GLOBAL: MW2SHELL 0x1005de28
SlateTab0x2c* g_unk0x1005de28 = NULL;

// GLOBAL: MW2SHELL 0x1005de30
MechS32 g_unk0x1005de30 = 0;

// GLOBAL: MW2SHELL 0x1005de38
MechS32 g_unk0x1005de38[8] = {75, 46, 86, 126, 10, 162, 39, 109};

// GLOBAL: MW2SHELL 0x1005de58
MechS32 g_unk0x1005de58[8] = {171, 206, 206, 206, 204, 204, 310, 310};

// GLOBAL: MW2SHELL 0x10079b50
MechChar g_szTempBuffer[0x100];

// GLOBAL: MW2SHELL 0x1007c960
undefined g_unk0x1007c960[0x100];

// GLOBAL: MW2SHELL 0x1007ca60
undefined g_unk0x1007ca60[0x100];

// FUNCTION: MW2SHELL 0x10007850
MechS32 FUN_10007850(MechS32 p_value)
{
	MechS32 remainder;

	remainder = p_value % 100;
	return remainder ? p_value + 100 - remainder : p_value;
}

// FUNCTION: MW2SHELL 0x1000788c
MechS32 FUN_1000788c(MechS32 p_value)
{
	return p_value - p_value % 100;
}

// FUNCTION: MW2SHELL 0x100078ae
MechS32 FUN_100078ae(MechS32 p_value)
{
	return FUN_1000788c(p_value + 50);
}

// Stack-slot permutation: top and height.
// FUNCTION: MW2SHELL 0x100078cd
void FUN_100078cd(SlateTab0x2c* p_tabs)
{
	MechS32 top;
	MechS32 height;

	top = 0;
	height = 0;
	if (p_tabs == NULL) {
		return;
	}

	while (p_tabs->m_left != -1) {
		if (p_tabs->m_top < 0) {
			p_tabs->m_top = ((p_tabs->m_top & 0xff0) >> 4) * height + (p_tabs->m_top & 0xf) + top;
		}

		if (p_tabs->m_unk0x1c != NULL) {
			p_tabs->m_glyph = p_tabs->m_unk0x1c(p_tabs);
		}
		else {
			p_tabs->m_glyph = NULL;
		}

		if (p_tabs->m_glyph != NULL) {
			if (p_tabs->m_width == -1) {
				p_tabs->m_width = p_tabs->m_glyph->m_width;
			}
			if (p_tabs->m_height == -1) {
				p_tabs->m_height = p_tabs->m_glyph->m_height;
			}
		}

		if (p_tabs->m_width == -1) {
			p_tabs->m_width = 0;
		}
		if (p_tabs->m_height == -1) {
			p_tabs->m_height = height;
		}

		top = p_tabs->m_top;
		height = p_tabs->m_height;
		p_tabs++;
	}
}

// FUNCTION: MW2SHELL 0x100079f8
void FUN_100079f8(SlateTab0x2c* p_tabs)
{
	SlateTab0x2c* tab;

	if (p_tabs == NULL) {
		return;
	}

	for (tab = p_tabs; tab->m_left != -1; tab++) {
		if (tab->m_glyph != NULL) {
			delete tab->m_glyph;
			tab->m_glyph = NULL;
		}
	}

	for (tab = p_tabs; tab->m_left != -1; tab++) {
		if (tab->m_unk0x1c != NULL) {
			tab->m_glyph = tab->m_unk0x1c(tab);
		}
	}
}

// FUNCTION: MW2SHELL 0x10007ac8
void FUN_10007ac8(SlateTab0x2c* p_tabs)
{
	if (p_tabs == NULL) {
		return;
	}

	while (p_tabs->m_left != -1) {
		if (p_tabs->m_glyph != NULL) {
			delete p_tabs->m_glyph;
			p_tabs->m_glyph = NULL;
		}

		p_tabs++;
	}
}

// The original loads the index j before i in m_unk0x318[i][j]; declaring j first didn't flip it.
// FUNCTION: MW2SHELL 0x10007b4d
MechS32 FUN_10007b4d(MechS32 p_id)
{
	MechS32 count;
	MechS32 j;
	MechS32 i;

	count = 0;
	if (p_id <= 0) {
		return count;
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 12; j++) {
			if (g_unk0x1005c640.m_unk0x318[i][j] == p_id) {
				g_unk0x1005c640.m_unk0x318[i][j] = 0;
				count++;
			}
		}
	}

	return count;
}

// FUNCTION: MW2SHELL 0x10007bee
MechS32 FUN_10007bee(MechS32 p_location, MechS32 p_id, MechS32 p_count)
{
	MechS32 i;

	if (p_id <= 0) {
		return FALSE;
	}

	for (i = 0; p_count && i < 12; i++) {
		if (g_unk0x1005c640.m_unk0x318[p_location][i] == 0) {
			g_unk0x1005c640.m_unk0x318[p_location][i] = p_id;
			p_count--;
		}
	}

	if (p_count) {
		for (i = 0; i < 12; i++) {
			if (g_unk0x1005c640.m_unk0x318[p_location][i] == p_id) {
				g_unk0x1005c640.m_unk0x318[p_location][i] = 0;
			}
		}

		return FALSE;
	}

	return TRUE;
}

// FUNCTION: MW2SHELL 0x10007cd4
void FUN_10007cd4(MechS32 p_id)
{
	MechS32 j;
	MechS32 i;

	if (p_id <= 0) {
		return;
	}

	j = 0;
	for (i = 0; i < 78; i++) {
		if (g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 == p_id) {
			g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 = -1;
			g_unk0x1005c640.m_unk0x49c--;
		}

		if (g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 != -1) {
			if (g_unk0x1005c640.m_unk0x4a0[j].m_unk0x00 == -1) {
				g_unk0x1005c640.m_unk0x4a0[j] = g_unk0x1005c640.m_unk0x4a0[i];
				g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 = -1;
			}
			j++;
		}
	}
}

// FUNCTION: MW2SHELL 0x10007d9e
void FUN_10007d9e(MechS32 p_id, MechS32 p_unk0x04)
{
	if (p_id <= 0) {
		return;
	}

	if (g_unk0x1005c640.m_unk0x49c < 78) {
		g_unk0x1005c640.m_unk0x4a0[g_unk0x1005c640.m_unk0x49c].m_unk0x00 = p_id;
		g_unk0x1005c640.m_unk0x4a0[g_unk0x1005c640.m_unk0x49c].m_unk0x04 = p_unk0x04;
		g_unk0x1005c640.m_unk0x49c++;
	}
}

// FUNCTION: MW2SHELL 0x10007df0
void FUN_10007df0(MechS32 p_id)
{
	MechS32 count;

	if (p_id <= 0) {
		return;
	}

	count = FUN_10007b4d(p_id);
	if (count) {
		FUN_10007d9e(p_id, count);
	}
}

// FUNCTION: MW2SHELL 0x10007e3b
MechS32 FUN_10007e3b(MechS32 p_location, MechS32 p_id, MechS32 p_count)
{
	if (p_id <= 0) {
		return FALSE;
	}

	if (FUN_10007bee(p_location, p_id, p_count)) {
		FUN_10007cd4(p_id);
		return TRUE;
	}

	return FALSE;
}

// Stack-slot permutation: id, group, index, i and j.
// FUNCTION: MW2SHELL 0x10007e90
void FUN_10007e90(MechS32 p_id)
{
	MechS32 id;
	MechS32 group;
	MechS32 index;
	MechS32 i;
	MechS32 j;

	group = p_id / 100;
	index = p_id % 100;
	if (p_id <= 0) {
		return;
	}

	for (i = 0; i < 8; i++) {
		for (j = 0; j < 12; j++) {
			id = g_unk0x1005c640.m_unk0x318[i][j];
			if (id == p_id) {
				id = 0;
			}
			else if (id > 0 && (id < 5000 || id >= 10000)) {
				if (id / 100 == group && id % 100 > index) {
					id--;
				}
				else if (p_id < 5000 && id >= 10000) {
					if ((id - 10000) / 100 == p_id) {
						id = 0;
					}
					else if ((id - 10000) / 10000 == group && id / 100 % 100 > index) {
						id -= 100;
					}
				}
			}

			g_unk0x1005c640.m_unk0x318[i][j] = id;
		}
	}

	for (i = 0, j = 0; i < 78; i++) {
		id = g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00;
		if (id == p_id) {
			id = -1;
			g_unk0x1005c640.m_unk0x49c--;
		}
		else if (id > 0 && (id < 5000 || id >= 10000)) {
			if (id / 100 == group && id % 100 > index) {
				id--;
			}
			else if (p_id < 5000 && id >= 10000) {
				if ((id - 10000) / 100 == p_id) {
					id = -1;
					g_unk0x1005c640.m_unk0x49c--;
				}
				else if ((id - 10000) / 10000 == group && id / 100 % 100 > index) {
					id -= 100;
				}
			}
		}

		g_unk0x1005c640.m_unk0x4a0[i].m_unk0x00 = id;
		if (id != -1) {
			g_unk0x1005c640.m_unk0x4a0[j] = g_unk0x1005c640.m_unk0x4a0[i];
			j++;
		}
	}

	while (j < 78) {
		g_unk0x1005c640.m_unk0x4a0[j].m_unk0x00 = -1;
		j++;
	}
}

// FUNCTION: MW2SHELL 0x1000819f
MechS32 FUN_1000819f(MechS32 p_id)
{
	MechS32 i;

	p_id -= p_id % 100;
	p_id++;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] == -1) {
			if (p_id % 100 > 10) {
				return 0;
			}

			g_unk0x1005c640.m_unk0x284[i] = p_id;
			return p_id;
		}

		if (g_unk0x1005c640.m_unk0x284[i] / 100 == p_id / 100) {
			p_id++;
		}
	}

	return 0;
}

// Stack-slot permutation: count and i.
// FUNCTION: MW2SHELL 0x10008254
MechS32 FUN_10008254(MechS32 p_id)
{
	MechS32 count;
	MechS32 i;

	count = 0;
	p_id = (p_id * 100 + 10000) / 100;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] == -1) {
			break;
		}

		if (g_unk0x1005c640.m_unk0x284[i] / 100 == p_id) {
			count++;
		}
	}

	return count;
}

// FUNCTION: MW2SHELL 0x100082de
void FUN_100082de(MechS32 p_id)
{
	MechS32 j;
	MechS32 i;

	j = 0;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] == p_id) {
			g_unk0x1005c640.m_unk0x284[i] = -1;
		}

		if (g_unk0x1005c640.m_unk0x284[i] != -1) {
			if (g_unk0x1005c640.m_unk0x284[j] == -1) {
				g_unk0x1005c640.m_unk0x284[j] = g_unk0x1005c640.m_unk0x284[i];
				g_unk0x1005c640.m_unk0x284[i] = -1;
			}

			if (g_unk0x1005c640.m_unk0x284[j] / 100 == p_id / 100 && g_unk0x1005c640.m_unk0x284[j] % 100 > p_id % 100) {
				g_unk0x1005c640.m_unk0x284[j]--;
			}

			j++;
		}
	}
}

// FUNCTION: MW2SHELL 0x100083d6
MechS32 FUN_100083d6(MechS32 p_id)
{
	MechS32 i;

	p_id -= p_id % 100;
	p_id++;
	for (i = 0; i < 10; i++) {
		if (g_unk0x1005c640.m_unk0x2e8[i] == -1) {
			g_unk0x1005c640.m_unk0x2e8[i] = p_id;
			return p_id;
		}

		if (g_unk0x1005c640.m_unk0x2e8[i] / 100 == p_id / 100) {
			p_id++;
		}
	}

	return 0;
}

// FUNCTION: MW2SHELL 0x10008470
void FUN_10008470(MechS32 p_id)
{
	MechS32 j;
	MechS32 i;

	j = 0;
	for (i = 0; i < 10; i++) {
		if (g_unk0x1005c640.m_unk0x2e8[i] == p_id) {
			g_unk0x1005c640.m_unk0x2e8[i] = -1;
		}

		if (g_unk0x1005c640.m_unk0x2e8[i] != -1) {
			if (g_unk0x1005c640.m_unk0x2e8[j] == -1) {
				g_unk0x1005c640.m_unk0x2e8[j] = g_unk0x1005c640.m_unk0x2e8[i];
				g_unk0x1005c640.m_unk0x2e8[i] = -1;
			}

			if (g_unk0x1005c640.m_unk0x2e8[j] / 100 == p_id / 100 && g_unk0x1005c640.m_unk0x2e8[j] % 100 > p_id % 100) {
				g_unk0x1005c640.m_unk0x2e8[j]--;
			}

			j++;
		}
	}

	p_id = p_id * 100 + 10001;
	j = 0;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] / 100 == p_id / 100) {
			g_unk0x1005c640.m_unk0x284[i] = -1;
		}

		if (g_unk0x1005c640.m_unk0x284[i] != -1) {
			if (g_unk0x1005c640.m_unk0x284[j] == -1) {
				g_unk0x1005c640.m_unk0x284[j] = g_unk0x1005c640.m_unk0x284[i];
				g_unk0x1005c640.m_unk0x284[i] = -1;
			}

			if (g_unk0x1005c640.m_unk0x284[j] / 10000 == p_id / 10000 &&
				g_unk0x1005c640.m_unk0x284[j] / 100 % 100 > p_id / 100 % 100) {
				g_unk0x1005c640.m_unk0x284[j] -= 100;
			}

			j++;
		}
	}
}

// FUNCTION: MW2SHELL 0x10008686
void FUN_10008686()
{
	g_unk0x1005c640.m_unk0x220 = FUN_10007850((MechS32) (g_unk0x1005c640.m_unk0x210 / 100.0 * 100.0));
	g_unk0x1005c640.m_unk0x244 = g_unk0x1005c640.m_unk0x200 / (g_unk0x1005c640.m_unk0x240 ? 20 : 10);
	g_unk0x1005c640.m_unk0x224 = 300;
	g_unk0x1005c640.m_unk0x204 = g_unk0x1005c640.m_unk0x238 + g_unk0x1005c640.m_unk0x22c + g_unk0x1005c640.m_unk0x270 +
								 g_unk0x1005c640.m_unk0x260 + g_unk0x1005c640.m_unk0x250 + g_unk0x1005c640.m_unk0x220 +
								 g_unk0x1005c640.m_unk0x244 + g_unk0x1005c640.m_unk0x224 + g_unk0x1005c640.m_unk0x268 +
								 g_unk0x1005c640.m_unk0x214;
	g_unk0x1005c640.m_unk0x208 = g_unk0x1005c640.m_unk0x200 - g_unk0x1005c640.m_unk0x204;
}

// FUNCTION: MW2SHELL 0x10008739
void FUN_10008739()
{
	g_unk0x1005c640.m_unk0x278 = g_unk0x1005c640.m_unk0x210 * 100 / g_unk0x1005c640.m_unk0x200;
	g_unk0x1005c640.m_unk0x27c = (g_unk0x1005c640.m_unk0x278 * 3 + 2) / 2;
}

// FUNCTION: MW2SHELL 0x10008776
void FUN_10008776()
{
}

// FUNCTION: MW2SHELL 0x10008786
void FUN_10008786()
{
	MechS32 count;

	count = g_unk0x1005c640.m_unk0x230;
	g_unk0x1005c640.m_unk0x230 = g_unk0x1005c640.m_unk0x22c / 100 + 10 - g_unk0x1005c640.m_unk0x210 / 25;
	if (g_unk0x1005c640.m_unk0x230 < 0) {
		g_unk0x1005c640.m_unk0x230 = 0;
	}

	if (count >= 0 && g_unk0x1005c640.m_unk0x230 != count) {
		while (g_unk0x1005c640.m_unk0x230 < count) {
			FUN_10007e90(count + 6000);
			count--;
		}

		while (count < g_unk0x1005c640.m_unk0x230) {
			count++;
			FUN_10007d9e(count + 6000, g_unk0x1005c640.m_unk0x228);
		}
	}
}

// FUNCTION: MW2SHELL 0x1000884c
void FUN_1000884c()
{
	g_unk0x1005c640.m_unk0x210 = g_unk0x1005d590[g_unk0x1005c640.m_unk0x20c % 10000].m_rating;
	g_unk0x1005c640.m_unk0x214 = g_unk0x1005d590[g_unk0x1005c640.m_unk0x20c % 10000].m_weight;
	if (g_unk0x1005c640.m_unk0x20c >= 10000) {
		g_unk0x1005c640.m_unk0x214 /= 2;
	}

	if (g_unk0x1005c640.m_unk0x20c >= 10000) {
		g_unk0x1005c640.m_unk0x218 = 7;
	}
	else {
		g_unk0x1005c640.m_unk0x218 = 0;
	}

	g_unk0x1005c640.m_unk0x21c = g_unk0x1005c640.m_unk0x200 - g_unk0x1005c640.m_unk0x214;
}

// FUNCTION: MW2SHELL 0x100088ec
void FUN_100088ec()
{
	FUN_10008739();
	while (g_unk0x1005c640.m_unk0x280 > g_unk0x1005c640.m_unk0x278) {
		FUN_10007e90(g_unk0x1005c640.m_unk0x280 + 7000);
		g_unk0x1005c640.m_unk0x280--;
	}

	if (g_unk0x1005c640.m_unk0x200 <= 5500) {
		g_unk0x1005c640.m_unk0x234 = 50;
	}
	else if (g_unk0x1005c640.m_unk0x200 <= 8500) {
		g_unk0x1005c640.m_unk0x234 = 100;
	}
	else {
		g_unk0x1005c640.m_unk0x234 = 200;
	}

	g_unk0x1005c640.m_unk0x238 = g_unk0x1005c640.m_unk0x280 * g_unk0x1005c640.m_unk0x234;
}

// FUNCTION: MW2SHELL 0x10008989
void FUN_10008989()
{
	FUN_10008686();
	FUN_10008739();
	FUN_10008776();
}

// FUNCTION: MW2SHELL 0x100089a8
void FUN_100089a8()
{
	MechS32 i;

	g_unk0x1005c640.m_unk0x260 = 0;
	for (i = 0; i < 10; i++) {
		if (g_unk0x1005c640.m_unk0x2e8[i] != -1) {
			g_unk0x1005c640.m_unk0x260 += g_unk0x1005d950[g_unk0x1005c640.m_unk0x2e8[i] / 100].m_unk0x18;
		}
	}
}

// FUNCTION: MW2SHELL 0x10008a16
void FUN_10008a16()
{
	MechS32 i;

	if (!g_unk0x1005c640.m_unk0x24c) {
		g_unk0x1005c640.m_unk0x258 = (g_unk0x1005c640.m_unk0x250 * 16 + 49) / 100;
	}
	else {
		g_unk0x1005c640.m_unk0x258 = ((MechS32) (g_unk0x1005c640.m_unk0x250 * 16 * 1.2) + 49) / 100;
	}

	g_unk0x1005c640.m_unk0x25c = 0;
	for (i = 0; i < 8; i++) {
		g_unk0x1005c640.m_unk0x25c += g_unk0x1005c640.m_unk0x710[i].m_unk0x08;
		if (g_unk0x1005c640.m_unk0x710[i].m_unk0x0c >= 0) {
			g_unk0x1005c640.m_unk0x25c += g_unk0x1005c640.m_unk0x710[i].m_unk0x0c;
		}
	}

	while (g_unk0x1005c640.m_unk0x25c > g_unk0x1005c640.m_unk0x258) {
		if (g_unk0x1005c640.m_unk0x710[g_unk0x1005de30].m_unk0x08 > 0) {
			g_unk0x1005c640.m_unk0x710[g_unk0x1005de30].m_unk0x08--;
			g_unk0x1005c640.m_unk0x25c--;
		}

		if (g_unk0x1005c640.m_unk0x25c <= g_unk0x1005c640.m_unk0x258) {
			break;
		}

		if (g_unk0x1005c640.m_unk0x710[g_unk0x1005de30].m_unk0x0c > 0) {
			g_unk0x1005c640.m_unk0x710[g_unk0x1005de30].m_unk0x0c--;
			g_unk0x1005c640.m_unk0x25c--;
		}

		g_unk0x1005de30++;
		if (g_unk0x1005de30 >= 8) {
			g_unk0x1005de30 = 0;
		}
	}
}

// FUNCTION: MW2SHELL 0x10008b76
void FUN_10008b76()
{
	MechS32 i;

	g_unk0x1005c640.m_unk0x268 = 0;
	for (i = 0; i < 25; i++) {
		if (g_unk0x1005c640.m_unk0x284[i] == -1) {
			break;
		}

		g_unk0x1005c640.m_unk0x268 += 100;
	}
}

// FUNCTION: MW2SHELL 0x10008bce
EmberGlyph0x3e* FUN_10008bce(SlateTab0x2c* p_tab)
{
	// An empty test of the unused parameter: the original compiles it to a cmp with a zero-length je.
	if (p_tab) {
	}

	FUN_10017698(12, g_unk0x1005c640.m_unk0x314);
	FUN_10016f82(
		12,
		g_unk0x1005de38[g_unk0x1005c640.m_unk0x314] + 0xd8,
		g_unk0x1005de58[g_unk0x1005c640.m_unk0x314] + 0x34
	);
	FUN_10016d27(12);
	return NULL;
}

// FUNCTION: MW2SHELL 0x10008c30
EmberGlyph0x3e* FUN_10008c30(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%d.%d%d T", value / 100, value / 10 % 10, value % 10);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008cb0
EmberGlyph0x3e* FUN_10008cb0(SlateTab0x2c* p_tab)
{
	MechS32 value;
	undefined* colors;

	value = *(MechS32*) p_tab->m_unk0x24;
	colors = p_tab->m_unk0x14;
	if (value > g_unk0x1005c640.m_unk0x200) {
		colors = g_unk0x1007c960;
	}

	sprintf(g_szTempBuffer, "%d.%d%d T", value / 100, value / 10 % 10, value % 10);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, colors);
}

// FUNCTION: MW2SHELL 0x10008d4c
EmberGlyph0x3e* FUN_10008d4c(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%d.%d%d T", value / 100, value / 10 % 10, value % 10);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008dcc
EmberGlyph0x3e* FUN_10008dcc(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, value >= 10000 ? "%dXL" : "%d", g_unk0x1005d590[value % 10000].m_rating);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008e4f
EmberGlyph0x3e* FUN_10008e4f(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value >= 10000 ? "XL" : "Std"), p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008eaa
EmberGlyph0x3e* FUN_10008eaa(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%s", g_unk0x1005d590[value % 10000].m_name);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008f14
EmberGlyph0x3e* FUN_10008f14(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%1.1f kph", value * 10.8);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008f7d
EmberGlyph0x3e* FUN_10008f7d(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%d m", value * 30);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10008fdd
EmberGlyph0x3e* FUN_10008fdd(SlateTab0x2c* p_tab)
{
	MechS32 value;
	MechS32 count;

	value = *(MechS32*) p_tab->m_unk0x24;
	count = (g_unk0x1005c640.m_unk0x22c + 50) / 100 + 10;
	if (value == 1) {
		sprintf(g_szTempBuffer, "%d", count);
	}
	else {
		sprintf(g_szTempBuffer, "%d (%d)", count, count * 2);
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

extern BrassLantern0x414* g_unk0x10071214;

void FUN_10016cc0(MechS32 p_index, MechS32 p_mask, MechS32 p_value);

// FUNCTION: MW2SHELL 0x10009076
EmberGlyph0x3e* FUN_10009076(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value == 1 ? "Single" : "Double"), p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100090ce
EmberGlyph0x3e* FUN_100090ce(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	sprintf(g_szTempBuffer, "%d", value);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10009126
EmberGlyph0x3e* FUN_10009126(SlateTab0x2c* p_tab)
{
	MechChar* text;

	if (p_tab->m_unk0x24 == NULL) {
		return NULL;
	}

	text = *(MechChar**) p_tab->m_unk0x24;
	if (text == NULL) {
		return NULL;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000918c
EmberGlyph0x3e* FUN_1000918c(SlateTab0x2c* p_tab)
{
	MechChar* text;

	text = (MechChar*) p_tab->m_unk0x24;
	if (text == NULL) {
		return NULL;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100091dc
EmberGlyph0x3e* FUN_100091dc(SlateTab0x2c* p_tab)
{
	MechChar* text;

	text = (MechChar*) p_tab->m_unk0x24;
	if (text == NULL) {
		return NULL;
	}

	return g_unk0x10071214->FUN_10005522(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000922c
EmberGlyph0x3e* FUN_1000922c(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value ? "Endo-S" : "Std"), p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10009284
EmberGlyph0x3e* FUN_10009284(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value ? "Ferro-F" : "Std"), p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100092dc
EmberGlyph0x3e* FUN_100092dc(SlateTab0x2c* p_tab)
{
	QuartzHelm0xf50::Armor* armor;

	armor = &g_unk0x1005c640.m_unk0x710[g_unk0x1005c640.m_unk0x314];
	if (p_tab->m_unk0x24) {
		if (armor->m_unk0x0c >= 0) {
			sprintf(g_szTempBuffer, "~%d", armor->m_unk0x0c);
		}
		else {
			strcpy(g_szTempBuffer, "~--");
		}
	}
	else {
		sprintf(g_szTempBuffer, "~%d", armor->m_unk0x08);
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100093a7
EmberGlyph0x3e* FUN_100093a7(SlateTab0x2c* p_tab)
{
	QuartzHelm0xf50::Armor* armor;

	armor = &g_unk0x1005c640.m_unk0x710[(MechS32) p_tab->m_unk0x24];
	if (armor->m_unk0x0c >= 0) {
		sprintf(g_szTempBuffer, "~%d/%d", armor->m_unk0x08, armor->m_unk0x0c);
	}
	else {
		sprintf(g_szTempBuffer, "~%d", armor->m_unk0x08);
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000943f
EmberGlyph0x3e* FUN_1000943f(SlateTab0x2c* p_tab)
{
	MechS32 location;

	location = (MechS32) p_tab->m_unk0x24;
	if (location == -1) {
		return NULL;
	}

	sprintf(g_szTempBuffer, "%s (%d)", g_unk0x1005c4f0[location], g_unk0x1005c640.m_unk0x710[location].m_unk0x04);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x100094ba
EmberGlyph0x3e* FUN_100094ba(SlateTab0x2c* p_tab)
{
	MechS32 location;

	location = *(MechS32*) p_tab->m_unk0x24;
	if (location == -1) {
		return NULL;
	}

	sprintf(g_szTempBuffer, "%s (%d)", g_unk0x1005c4f0[location], g_unk0x1005c640.m_unk0x710[location].m_unk0x04);
	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10009537
EmberGlyph0x3e* FUN_10009537(SlateTab0x2c* p_tab)
{
	MechS32 value;
	MechS32 count;
	undefined* colors;

	value = *(MechS32*) p_tab->m_unk0x24;
	colors = p_tab->m_unk0x14;
	if (value == -1) {
		strcpy(g_szTempBuffer, "-");
	}
	else {
		if (g_unk0x1005d950[value / 100].m_unk0x20) {
			count = FUN_10008254(value);
			sprintf(
				g_szTempBuffer,
				"%s #%d (ammo %dT/%d)",
				g_unk0x1005d950[value / 100].m_name,
				value % 100,
				count,
				g_unk0x1005d950[value / 100].m_unk0x20 * count
			);
		}
		else {
			sprintf(g_szTempBuffer, "%s #%d", g_unk0x1005d950[value / 100].m_name, value % 100);
		}

		if (g_unk0x1005c640.m_unk0x310 == value) {
			colors = g_unk0x1007ca60;
		}
	}

	if (p_tab->m_left >= 0x1b4) {
		return g_unk0x1007120c->FUN_1000544e(p_tab->m_left, p_tab->m_top, g_szTempBuffer, colors);
	}
	else {
		return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_szTempBuffer, colors);
	}
}

// FUNCTION: MW2SHELL 0x100096bd
EmberGlyph0x3e* FUN_100096bd(SlateTab0x2c* p_tab)
{
	MechS32 index;
	undefined* colors;

	index = (MechS32) p_tab->m_unk0x24;
	colors = p_tab->m_unk0x14;
	if (index == -1) {
		return NULL;
	}

	if (g_unk0x1005c640.m_unk0x310 == index * 100) {
		colors = g_unk0x1007ca60;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_unk0x1005d950[index].m_name, colors);
}

// FUNCTION: MW2SHELL 0x1000973c
EmberGlyph0x3e* FUN_1000973c(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	if (value == -1) {
		return NULL;
	}

	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, g_unk0x1005d950[value / 100].m_name, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x10009d50
EmberGlyph0x3e* FUN_10009d50(SlateTab0x2c* p_tab)
{
	MechS32 location;

	location = *(MechS32*) p_tab->m_unk0x24;
	if (location == -1) {
		return NULL;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, g_unk0x1005c4f0[location], p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000a161
EmberGlyph0x3e* FUN_1000a161(SlateTab0x2c* p_tab)
{
	if (g_unk0x1005c640.m_unk0x4a0[(MechS32) p_tab->m_unk0x24].m_unk0x00 <= 0) {
		return NULL;
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, "More...", p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000a1b0
EmberGlyph0x3e* FUN_1000a1b0(SlateTab0x2c* p_tab)
{
	MechS32 value;

	value = *(MechS32*) p_tab->m_unk0x24;
	return g_unk0x1007120c
		->FUN_10005522(p_tab->m_left, p_tab->m_top, (MechChar*) (value ? "Yes" : "No"), p_tab->m_unk0x14);
}

// Stack-slot permutation: id and text.
// FUNCTION: MW2SHELL 0x1000a208
EmberGlyph0x3e* FUN_1000a208(SlateTab0x2c* p_tab)
{
	MechS32 id;
	MechChar* text;

	id = (MechS32) p_tab->m_unk0x24;
	switch (id) {
	case 5000:
		text = "MASC";
		break;
	case 5401:
		text = "Right Lower Arm Actuator";
		break;
	case 5402:
		text = "Left Lower Arm Actuator";
		break;
	case 5451:
		text = "Right Hand Actuator";
		break;
	case 5452:
		text = "Left Hand Actuator";
		break;
	default:
		text = "";
	}

	return g_unk0x1007120c->FUN_10005522(p_tab->m_left, p_tab->m_top, text, p_tab->m_unk0x14);
}

// FUNCTION: MW2SHELL 0x1000a2eb
void FUN_1000a2eb(SlateTab0x2c* p_tab)
{
	SlateTab0x2c* tabs;

	tabs = p_tab->m_unk0x28;
	FUN_10016cc0(10, 0x20, 0x20);
	FUN_10016cc0(11, 0x20, 0x20);
	FUN_10016cc0(12, 0x20, 0x20);
	FUN_10016cc0(13, 0x20, 0x20);
	FUN_10016d27(14);
	FUN_10007ac8(g_unk0x1005de28);
	g_unk0x1005de28 = tabs;
	FUN_100078cd(g_unk0x1005de28);
}

// FUNCTION: MW2SHELL 0x1000a36d
void FUN_1000a36d(SlateTab0x2c* p_tab)
{
	g_unk0x1005c640.m_unk0x310 = -1;
	FUN_1000a2eb(p_tab);
	FUN_10016d27(11);
	FUN_10016d27(13);
	FUN_10016cc0(14, 0x20, 0x20);
}

// FUNCTION: MW2SHELL 0x1000a3b5
void FUN_1000a3b5(SlateTab0x2c* p_tab)
{
	FUN_1000a2eb(p_tab);
	FUN_10016d27(10);
	FUN_10016d27(12);
	FUN_10016cc0(14, 0x20, 0x20);
}

// FUNCTION: MW2SHELL 0x1000a3f3
void FUN_1000a3f3(SlateTab0x2c* p_tab)
{
	FUN_1000a2eb(p_tab);
	FUN_10016d27(10);
	FUN_10016d27(11);
	FUN_10016d27(12);
	FUN_10016cc0(14, 0x20, 0x20);
}

// STUB: MW2SHELL 0x1000d0d4
void FUN_1000d0d4(TMPackDataBase* p_database, MechS32 p_campaign, WPARAM p_wParam)
{
	STUB(0x1000d0d4);
}
