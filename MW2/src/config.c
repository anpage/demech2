#include "config.h"

#include "anim2d.h"
#include "approxlen.h"
#include "cobaltharbor.h"
#include "cockpit.h"
#include "decomp.h"
#include "environment.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "gamekeys.h"
#include "garnetframe.h"
#include "loadres.h"
#include "mech.h"
#include "network.h"
#include "players.h"
#include "point.h"
#include "quartzreel.h"
#include "random.h"
#include "recttransition.h"
#include "render.h"
#include "rendertarget.h"
#include "resource.h"
#include "resourcefile.h"
#include "resourceref.h"
#include "screenscale.h"
#include "screenshot.h"
#include "silvertern.h"
#include "simmain.h"
#include "soundconfig.h"
#include "soundfx.h"
#include "speech.h"
#include "staticmem.h"
#include "types.h"
#include "unk10004f40.h"
#include "unk100079d0.h"
#include "unk10021460.h"
#include "unk10033280.h"
#include "unk10040020.h"
#include "unk10040b30.h"
#include "unk10046750.h"
#include "unk1004d020.h"
#include "unk100509a0.h"
#include "unk100563d0.h"
#include "unk100737e0.h"
#include "unk100746c0.h"
#include "unk1007b930.h"
#include "weapondata.h"
#include "weapons.h"
#include "weaponslot.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

DECOMP_SIZE_ASSERT(DifficultyCfg, 0x17)
DECOMP_SIZE_ASSERT(QuartzReel0x14, 0x14)
DECOMP_SIZE_ASSERT(GarnetFrame0x8, 0x8)

enum FilePermission {
	c_permissionWrite = 0x80 // _S_IWRITE (sys/stat.h)
};

// The 26 cockpit panels' rectangles, in 320x200 screen coordinates (FUN_1006fba3 scales them to the
// screen).
// GLOBAL: MW2 0x100adf58
RenderTarget g_unk0x100adf58[26] = {
	{&g_mainPixelBuffer, 13, 10, 80, 60},     {&g_mainPixelBuffer, 260, 145, 312, 197},
	{&g_mainPixelBuffer, 258, 145, 309, 175}, {&g_mainPixelBuffer, 214, 15, 256, 23},
	{&g_mainPixelBuffer, 214, 25, 256, 33},   {&g_mainPixelBuffer, 214, 35, 256, 43},
	{&g_mainPixelBuffer, 214, 45, 256, 53},   {&g_mainPixelBuffer, 214, 55, 256, 63},
	{&g_mainPixelBuffer, 268, 15, 319, 23},   {&g_mainPixelBuffer, 268, 25, 319, 33},
	{&g_mainPixelBuffer, 268, 35, 319, 43},   {&g_mainPixelBuffer, 268, 45, 319, 53},
	{&g_mainPixelBuffer, 268, 55, 319, 63},   {&g_mainPixelBuffer, 10, 145, 62, 179},
	{&g_mainPixelBuffer, 10, 183, 70, 199},   {&g_mainPixelBuffer, 30, 70, 300, 170},
	{&g_mainPixelBuffer, 30, 70, 300, 170},   {&g_mainPixelBuffer, 55, 190, 120, 199},
	{&g_mainPixelBuffer, 275, 130, 317, 195}, {&g_mainPixelBuffer, 250, 188, 273, 199},
	{&g_mainPixelBuffer, 110, 175, 169, 189}, {&g_mainPixelBuffer, 170, 175, 209, 189},
	{&g_mainPixelBuffer, 210, 175, 249, 189}, {&g_mainPixelBuffer, 1, 74, 35, 111},
	{&g_mainPixelBuffer, 115, 1, 205, 35},    {&g_mainPixelBuffer, 10, 136, 70, 145},
};

// The panels' text positions (16.16 fractions of their rectangles).
// GLOBAL: MW2 0x100ae160
Point g_unk0x100ae160[26] = {{0, 0},           {0, 0},
							 {0, 0},           {0x11ec, 0x2148},
							 {0x11ec, 0x2148}, {0x11ec, 0x2148},
							 {0x11ec, 0x2148}, {0x11ec, 0x2148},
							 {0x11ec, 0x2148}, {0x11ec, 0x2148},
							 {0x11ec, 0x2148}, {0x11ec, 0x2148},
							 {0x11ec, 0x2148}, {0, 0},
							 {0, 0},           {0, 0},
							 {0, 0},           {0, 0},
							 {0, 0xe666},      {0, 0},
							 {0, 0xa666},      {0, 0xa666},
							 {0, 0xa666},      {0, 0},
							 {0, 0},           {0, 0}};

// The transitions of panels 13 and 2.

// GLOBAL: MW2 0x100ae230
RectTransitionState g_unk0x100ae230 = {0, 0, 0};

// GLOBAL: MW2 0x100ae240
RectTransitionState g_unk0x100ae240 = {0, 0, 0};

// GLOBAL: MW2 0x100ae250
RenderTarget g_unk0x100ae250 = {NULL, 0x8000, 0x8000, 0x8000, 0x8000};

// GLOBAL: MW2 0x100ae268
RenderTarget g_unk0x100ae268 = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100ae280
RenderTarget g_unk0x100ae280 = {NULL, 0, 0, 0, 0};

// GLOBAL: MW2 0x100ae298
RectTransitionDef g_unk0x100ae298 = {0xb5, &g_unk0x100ae250, &g_unk0x100ae268, &g_unk0x100ae280};

// GLOBAL: MW2 0x100ae2a8
RectTransition g_unk0x100ae2a8 = {&g_unk0x100ae230, &g_unk0x100ae298};

// GLOBAL: MW2 0x100ae2b0
RenderTarget g_unk0x100ae2b0 = {NULL, 0x8000, 0x8000, 0x8000, 0x8000};

// GLOBAL: MW2 0x100ae2c8
RenderTarget g_unk0x100ae2c8 = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100ae2e0
RenderTarget g_unk0x100ae2e0 = {NULL, 0, 0, 0, 0};

// GLOBAL: MW2 0x100ae2f8
RectTransitionDef g_unk0x100ae2f8 = {0xb5, &g_unk0x100ae2b0, &g_unk0x100ae2c8, &g_unk0x100ae2e0};

// GLOBAL: MW2 0x100ae308
RectTransition g_unk0x100ae308 = {&g_unk0x100ae240, &g_unk0x100ae2f8};

// The panels' transitions.
// GLOBAL: MW2 0x100ae310
RectTransition* g_unk0x100ae310[26] = {
	NULL,
	NULL,
	&g_unk0x100ae308,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&g_unk0x100ae2a8,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

// Set when FUN_1007005a should run FUN_10007cb5 on its next frame.
// GLOBAL: MW2 0x100ae37c
MechS32 g_unk0x100ae37c = 0;

// GLOBAL: MW2 0x100ae380
MechS32 g_unk0x100ae380 = 0;

// The panels' m_unk0x08 values.
// GLOBAL: MW2 0x100ae388
undefined4 g_unk0x100ae388[26] = {0x16a, 0xb5,  0xb5,  0x21f, 0x23d, 0x25b, 0x279, 0x297, 0x32e,
								  0x310, 0x2f2, 0x2d4, 0x2b5, 0xb5,  0xb5,  0xb5,  0xb5,  0xb5,
								  0xb5,  0xb5,  0xb5,  0xb5,  0xb5,  0x0,   0x0,   0x0};

// The local mech's state (Mech::m_unk0xa0) when FUN_100705dd last ran.
// GLOBAL: MW2 0x100ae3f0
MechS32 g_unk0x100ae3f0 = 0;

// GLOBAL: MW2 0x100ae3f4
MechS32 g_unk0x100ae3f4 = 0;

// GLOBAL: MW2 0x100ae3f8
MechS32 g_unk0x100ae3f8 = 0;

// GLOBAL: MW2 0x100ae3fc
MechS32 g_unk0x100ae3fc = 0;

// The game directory (the MECHWARRIOR environment variable).
// GLOBAL: MW2 0x100ae400
MechChar g_gameDir[256] = {0};

// The number of the next screenshot FUN_100715a2 saves.
// GLOBAL: MW2 0x100ae500
MechS32 g_screenshotCount = 0;

// The path BuildGamePath returns.
// GLOBAL: MW2 0x100bef58
MechChar g_gamePath[0x50];

// GLOBAL: MW2 0x100c3358
MechS32 g_unk0x100c3358;

// Loads eight sounds ahead of their use.
// FUNCTION: MW2 0x1006f480
void FUN_1006f480(void)
{
	MechS32 ids[8];
	MechU32 i;

	ids[0] = 0xbd;
	ids[1] = 0xf7;
	ids[2] = 0xdb;
	ids[3] = 0xf0;
	ids[4] = 0xf6;
	ids[5] = 0xcf;
	ids[6] = 0xce;
	ids[7] = 0xfe;
	for (i = 0; i < 8; i++) {
		FUN_10050862(ids[i], g_unk0x100a8674);
	}
}

// Lays out p_mech's weapons on the weapon panels: the left panels (3 to 7) take the weapons on
// hardpoints 5, 3 and 7, the right ones (8 to 12) those on 4, 1 and 6, and the weapons on 2 and 0
// fill the rest, alternating. Then reorders the weapons to match, left at even indices and right
// at odd ones, renumbering their bins, and clears the slots left empty.
// The locals are a stack-slot permutation, and right > left takes its operands in the other
// order.
// FUNCTION: MW2 0x1006f4fa
void FUN_1006f4fa(Mech* p_mech)
{
	MechS32 rightStart;
	WeaponSlot* slot;
	MechS32 panel;
	MechS32 i;
	WeaponSlot* dst;
	WeaponSlot* weapons;
	MechS32 left;
	MechS32 j;
	MechS32 index;
	MechS32 alternate;
	SilverTern0x14* bin;
	MechS32 right;

	alternate = TRUE;
	for (i = 0; i < 10; i++) {
		p_mech->m_weapons[i].m_index = i;
	}

	slot = p_mech->m_weapons;
	rightStart = 8;
	for (i = 0; i < 10; i++) {
		if (slot->m_hardpoint == 4 || slot->m_hardpoint == 1 || slot->m_hardpoint == 6) {
			rightStart++;
		}

		if (i != 10) {
			slot++;
		}
	}

	slot = p_mech->m_weapons;
	left = 3;
	for (i = 0; i < 10; i++) {
		if (slot->m_hardpoint == 5 || slot->m_hardpoint == 3 || slot->m_hardpoint == 7) {
			g_unk0x100c3280[left]->m_setName(g_unk0x100c3280[left], g_weaponDefs[slot->m_type].m_name);
			g_unk0x100c3280[left]->m_setUnk0x0c(g_unk0x100c3280[left], i);
			g_unk0x100c3280[left]->m_unk0x78 = FUN_100334d3;
			g_unk0x100c3280[left]->m_unk0x7c = FUN_10033280;
			left++;
		}

		if (left == 8) {
			left = rightStart;
		}

		if (i != 10) {
			slot++;
		}
	}

	slot = p_mech->m_weapons;
	right = 8;
	for (i = 0; i < 10; i++) {
		if (slot->m_hardpoint == 4 || slot->m_hardpoint == 1 || slot->m_hardpoint == 6) {
			g_unk0x100c3280[right]->m_setName(g_unk0x100c3280[right], g_weaponDefs[slot->m_type].m_name);
			g_unk0x100c3280[right]->m_setUnk0x0c(g_unk0x100c3280[right], i);
			g_unk0x100c3280[right]->m_unk0x78 = FUN_100334d3;
			g_unk0x100c3280[right]->m_unk0x7c = FUN_10033280;
			right++;
		}

		if (right > 12) {
			right = left;
		}

		if (i != 10) {
			slot++;
		}
	}

	slot = p_mech->m_weapons;
	panel = left;
	for (i = 0; i < 10; i++) {
		if (slot->m_hardpoint == 2 || slot->m_hardpoint == 0) {
			if (alternate && right > left) {
				panel = right;
				if (panel == 13) {
					panel = left;
					left++;
				}
				else {
					right++;
				}

				alternate = FALSE;
			}
			else {
				panel = left;
				if (panel == 8) {
					panel = right;
					right++;
				}
				else {
					left++;
				}

				alternate = TRUE;
			}

			g_unk0x100c3280[panel]->m_setName(g_unk0x100c3280[panel], g_weaponDefs[slot->m_type].m_name);
			g_unk0x100c3280[panel]->m_setUnk0x0c(g_unk0x100c3280[panel], i);
			g_unk0x100c3280[panel]->m_unk0x78 = FUN_100334d3;
			g_unk0x100c3280[panel]->m_unk0x7c = FUN_10033280;
			panel++;
		}

		if (i != 10) {
			slot++;
		}
	}

	weapons = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 10 * sizeof(WeaponSlot));
	dst = weapons;
	for (i = 3; i <= 7; i++) {
		if (g_unk0x100c3280[i]->m_unk0x0c != -1) {
			slot = &p_mech->m_weapons[g_unk0x100c3280[i]->m_unk0x0c];
			*dst = *slot;
			if (i != 7) {
				dst += 2;
			}
		}
	}

	dst = weapons + 1;
	for (i = 8; i <= 12; i++) {
		if (g_unk0x100c3280[i]->m_unk0x0c != -1) {
			slot = &p_mech->m_weapons[g_unk0x100c3280[i]->m_unk0x0c];
			*dst = *slot;
			if (i != 12) {
				dst += 2;
			}
		}
	}

	index = 0;
	for (i = 3; i <= 7; i++) {
		if (g_unk0x100c3280[i]->m_unk0x0c != -1) {
			slot = &p_mech->m_weapons[g_unk0x100c3280[i]->m_unk0x0c];
			for (j = 0; j < slot->m_binCount; j++) {
				bin = &((SilverTern0x14*) p_mech->m_unk0x5c)[slot->m_bins[j]];
				bin->m_weapon = index;
			}
		}

		g_unk0x100c3280[i]->m_unk0x0c = index;
		index += 2;
	}

	index = 1;
	for (i = 8; i <= 12; i++) {
		if (g_unk0x100c3280[i]->m_unk0x0c != -1) {
			slot = &p_mech->m_weapons[g_unk0x100c3280[i]->m_unk0x0c];
			for (j = 0; j < slot->m_binCount; j++) {
				bin = &((SilverTern0x14*) p_mech->m_unk0x5c)[slot->m_bins[j]];
				bin->m_weapon = index;
			}

			g_unk0x100c3280[i]->m_unk0x0c = index;
			index += 2;
		}
	}

	memcpy(p_mech->m_weapons, weapons, 10 * sizeof(WeaponSlot));
	HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, weapons);
	for (i = 0; i < 10; i++) {
		slot = &p_mech->m_weapons[i];
		if (!slot->m_type && !slot->m_ammo) {
			slot->m_type = -1;
		}
	}
}

// Scales the panels' rectangles, text positions and transition rectangles to the screen.
// Stack-slot permutation: rect, transition, i and target.
// FUNCTION: MW2 0x1006fba3
void FUN_1006fba3(void)
{
	RenderTarget* rect;
	RectTransition* transition;
	MechS32 i;
	RenderTarget* target;

	for (i = 0; i < 26; i++) {
		target = &g_unk0x100adf58[i];
		FUN_10056ce9(target, target);
		ScaleRectToScreen(&g_mainPixelBuffer, target, target);
		FUN_10056bc1(target, &g_unk0x100ae160[i], &g_unk0x100ae160[i]);
		transition = g_unk0x100ae310[i];
		if (transition) {
			rect = transition->m_def->m_first;
			rect->m_buffer = &g_mainPixelBuffer;
			FUN_1005699f(target, rect, rect);
			rect = transition->m_def->m_second;
			rect->m_buffer = &g_mainPixelBuffer;
			FUN_1005699f(target, rect, rect);
			rect = transition->m_def->m_out;
			rect->m_buffer = &g_mainPixelBuffer;
		}
	}
}

// Creates the 26 cockpit panels, all enabled but the second and (in network games) the
// fourteenth, lays out the local mech's weapon panels (FUN_1006f4fa) and installs the panels'
// handlers.
// FUNCTION: MW2 0x1006fca5
void FUN_1006fca5(void)
{
	Mech* mech;
	MechS32 i;

	for (i = 0; i < 26; i++) {
		g_unk0x100c32f0[i] = TRUE;
	}

	g_unk0x100c32f0[1] = FALSE;
	if (!g_difficulty->m_unk0x09) {
		g_unk0x100c32f0[13] = FALSE;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	for (i = 0; i < 26; i++) {
		g_unk0x100c3280[i] = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(CobaltHarbor0x88));
		FUN_100746c0(g_unk0x100c3280[i]);
		g_unk0x100c3280[i]->m_setUnk0x08(g_unk0x100c3280[i], g_unk0x100ae388[i]);
		g_unk0x100c3280[i]->m_setTarget(g_unk0x100c3280[i], &g_unk0x100adf58[i]);
		g_unk0x100c3280[i]->m_setUnk0x34(g_unk0x100c3280[i], &g_unk0x100ae160[i]);
		g_unk0x100c3280[i]->m_setTransition(g_unk0x100c3280[i], g_unk0x100ae310[i]);
		if (g_unk0x100c32f0[i]) {
			g_unk0x100c3280[i]->m_enable(g_unk0x100c3280[i]);
		}
		else {
			g_unk0x100c3280[i]->m_disable(g_unk0x100c3280[i]);
		}
	}

	FUN_1006f4fa(mech);
	g_unk0x100c3280[2]->m_unk0x7c = FUN_100509c8;
	g_unk0x100c3280[2]->m_unk0x84 = FUN_10050e20;
	g_unk0x100c3280[2]->m_unk0x78 = FUN_10050ebe;
	g_unk0x100c3280[2]->m_unk0x80 = FUN_10050fd6;
	g_unk0x100c3280[13]->m_unk0x7c = FUN_1007c126;
	g_unk0x100c3280[13]->m_unk0x84 = FUN_1007c6df;
	g_unk0x100c3280[14]->m_unk0x7c = FUN_1007b930;
	g_unk0x100c3280[13]->m_unk0x78 = FUN_1007c71e;
	g_unk0x100c3280[13]->m_unk0x80 = FUN_1007c81c;
	g_unk0x100c3280[15]->m_unk0x7c = FUN_100056f0;
	g_unk0x100c3280[16]->m_unk0x7c = FUN_10005add;
	g_unk0x100c3280[17]->m_unk0x7c = FUN_10006189;
	g_unk0x100c3280[18]->m_unk0x7c = FUN_10006291;
	g_unk0x100c3280[19]->m_unk0x7c = FUN_100063cd;
	g_unk0x100c3280[20]->m_unk0x7c = FUN_10006484;
	g_unk0x100c3280[21]->m_unk0x7c = FUN_100065c3;
	g_unk0x100c3280[22]->m_unk0x7c = FUN_1000667c;
	g_unk0x100c3280[25]->m_unk0x7c = FUN_100060b6;
	FUN_1004d020();
	FUN_10040020();
	g_unk0x100a2434 = &mech->m_unk0xd0;
	g_unk0x100a2438 = &mech->m_unk0x04.m_value;
	FUN_10040f91();
	if (g_difficulty->m_unk0x09) {
		FUN_1003dce8();
	}
}

// Lays out the local mech's weapon panels again and resets every panel to its settings.
// FUNCTION: MW2 0x1006ff7b
void FUN_1006ff7b(void)
{
	Mech* mech;
	MechS32 i;

	mech = g_players[g_localPlayerId]->m_mech;
	FUN_1006f4fa(mech);
	for (i = 0; i < 26; i++) {
		g_unk0x100c3280[i]->m_setUnk0x08(g_unk0x100c3280[i], g_unk0x100ae388[i]);
		g_unk0x100c3280[i]->m_setTransition(g_unk0x100c3280[i], g_unk0x100ae310[i]);
		g_unk0x100c3280[i]->m_setUnk0x06(g_unk0x100c3280[i], 0);
		if (g_unk0x100c32f0[i]) {
			g_unk0x100c3280[i]->m_enable(g_unk0x100c3280[i]);
		}
	}
}

// Updates the cockpit for the local mech's frame: the heading and the torso twist the panels show,
// the target's bearing relative to both and its distance, a one-time warning sound, and the
// panels' handlers for the view mode (Mech::m_unk0xa0: 1, 2 or the rest).
// Stack-slot permutation: pitch, twistBearing, i, distance and bearing.
// FUNCTION: MW2 0x1007005a
void FUN_1007005a(Mech* p_mech)
{
	MechS32 pitch;
	MechS32 twistBearing;
	MechS32 i;
	MechS32 distance;
	MechS32 bearing;

	if (p_mech->m_player->m_index != g_localPlayerId) {
		return;
	}

	g_unk0x100c3358 = p_mech->m_unk0xa0;
	if (g_unk0x100c3358 != 2 && g_unk0x100c3280[16]->m_unk0x7c) {
		g_unk0x100c3280[16]->m_unk0x7c(g_unk0x100c3280[16]);
	}

	if (g_unk0x100c3358 == 4 || g_unk0x100c3358 == 5) {
		return;
	}

	if (!g_unk0x100a5f18) {
		return;
	}

	g_unk0x100c3270 = ((p_mech->m_player->m_heading >> 16) % 360 % 360 + 360) % 360;
	g_unk0x100c326c = (p_mech->m_unk0x04.m_value >> 16) % 360 % 360;
	pitch = (p_mech->m_player->m_targetInfo.m_unk0x18 + p_mech->m_unk0x14.m_value) % 0x1680000;
	bearing = (p_mech->m_player->m_targetInfo.m_heading >> 16) % 360 - g_unk0x100c3270;
	if (bearing > 180) {
		bearing -= 360;
	}
	else if (bearing < -180) {
		bearing += 360;
	}

	twistBearing = bearing - g_unk0x100c326c;
	if (twistBearing > 180) {
		twistBearing -= 360;
	}
	else if (twistBearing < -180) {
		twistBearing += 360;
	}

	distance = ApproximateVectorLength(
		p_mech->m_player->m_targetInfo.m_position.m_x - p_mech->m_player->m_position.m_x,
		p_mech->m_player->m_targetInfo.m_position.m_y - p_mech->m_player->m_position.m_y,
		p_mech->m_player->m_targetInfo.m_position.m_z - p_mech->m_player->m_position.m_z
	);
	if (g_unk0x100ae37c) {
		if (g_unk0x100c3358 == 2) {
			FUN_10007cb5(p_mech);
		}

		g_unk0x100ae37c = 0;
	}

	if (g_unk0x100aa298 && g_unk0x100c3358 != 3 && (p_mech->m_unk0x10c & 4) && !(p_mech->m_unk0x10c & 8)) {
		FUN_1007eb23(0xcd, 100, 0x40, 5, 0x32);
		PlayCockpitSound(2, -1);
		p_mech->m_unk0x10c |= 8;
		g_unk0x100aa298 = 0;
	}
	else {
		g_unk0x100aa298 = 0;
	}

	FUN_100705dd(p_mech);
	switch (g_unk0x100c3358) {
	case 2:
		if (g_difficulty->m_unk0x09) {
			FUN_1003dd82();
		}

		for (i = 0; i < 26; i++) {
			if (g_unk0x100c3280[i]->m_unk0x7c) {
				g_unk0x100c3280[i]->m_unk0x7c(g_unk0x100c3280[i]);
			}
		}

		FUN_10040bfd(p_mech, g_unk0x100c3270, g_unk0x100c326c, bearing, twistBearing, pitch, distance, g_unk0x100a241c);
		FUN_10021b2a(p_mech->m_unk0x44.m_value);
		break;
	case 1:
		if (g_difficulty->m_unk0x09) {
			FUN_1003f66d();
		}

		for (i = 0; i < 26; i++) {
			if (g_unk0x100c3280[i]->m_unk0x78) {
				g_unk0x100c3280[i]->m_unk0x78(g_unk0x100c3280[i]);
			}
		}

		FUN_10021a07();
		break;
	default:
		if (g_difficulty->m_unk0x09) {
			FUN_1003fa05();
		}

		for (i = 0; i < 26; i++) {
			if (g_unk0x100c3280[i]->m_unk0x80) {
				g_unk0x100c3280[i]->m_unk0x80(g_unk0x100c3280[i]);
			}
		}

		FUN_10021be2();
		break;
	}

	if (!g_unk0x100a2420) {
		FUN_10021c49();
	}
}

// Shuts the cockpit panels down: FUN_1003fad9 outside network games (DifficultyCfg::m_unk0x09),
// each panel's m_unk0x4c hook, then every 2D animation.
// FUNCTION: MW2 0x100704c1
void FUN_100704c1(void)
{
	MechS32 i;

	if (g_difficulty->m_unk0x09) {
		FUN_1003fad9();
	}

	for (i = 0; i < 26; i++) {
		if (g_unk0x100c3280[i]->m_unk0x4c) {
			g_unk0x100c3280[i]->m_unk0x4c(g_unk0x100c3280[i]);
		}
	}

	FreeAnim2ds(-1);
}

// Knocks the cockpit panels about when the local player's mech is hit: each panel has a two
// (p_heavy: five) in ten chance of stepping its m_unk0x06.
// FUNCTION: MW2 0x1007053d
void FUN_1007053d(Mech* p_mech, MechS32 p_heavy)
{
	MechS32 chance;
	MechS32 i;

	if (p_mech->m_player->m_index != g_localPlayerId) {
		return;
	}

	if (p_heavy) {
		chance = 5;
	}
	else {
		chance = 2;
	}

	for (i = 0; i < 26; i++) {
		if (RandomIntBelow(10) < chance) {
			g_unk0x100c3280[i]->m_setUnk0x06(g_unk0x100c3280[i], g_unk0x100c3280[i]->m_unk0x06 + 1);
		}
	}
}

// Plays the cockpit's warning sounds for the local mech as its state and flags change.
// FUNCTION: MW2 0x100705dd
void FUN_100705dd(Mech* p_mech)
{
	if (g_unk0x100ae380) {
		g_unk0x100ae3fc++;
		FUN_1004ca29(g_unk0x100ae3fc * 3);
		g_unk0x100ae380 = 0;
	}

	if (g_unk0x100a2420) {
		if (p_mech->m_unk0x10c & 0x80) {
			if (!g_unk0x100ae3f4 && p_mech->m_weapons[p_mech->m_selectedWeapon].m_state == 1 &&
				p_mech->m_unk0xa0 == 2) {
				g_unk0x100ae3f4 = 1;
				FUN_1007eb23(0xfe, 100, 0x5f, 5, 0x32);
			}
		}
		else {
			g_unk0x100ae3f4 = 0;
			if (p_mech->m_unk0x10c & 0x40) {
				if (!g_unk0x100ae3f8 && p_mech->m_unk0xa0 == 2) {
					g_unk0x100ae3f8 = 1;
					FUN_1007eb23(0xcf, 100, 0x1f, 5, 0x32);
				}
			}
			else {
				g_unk0x100ae3f8 = 0;
			}
		}

		if (p_mech->m_unk0xa0 != g_unk0x100ae3f0) {
			switch (p_mech->m_unk0xa0) {
			case 2:
				FUN_1007eb23(0xce, 100, 0x2f, 5, 0x32);
				break;
			case 0:
			case 4:
				FUN_1007eb23(0xf6, 100, 0x40, 5, 0x32);
				break;
			default:
				break;
			}
		}
	}

	g_unk0x100ae3f0 = p_mech->m_unk0xa0;
}

// FUNCTION: MW2 0x1007079d
void FUN_1007079d(RenderTarget* p_target, MechS32 p_index, MechS32 p_x, MechS32 p_y)
{
	DrawAnim2d(p_target, p_index, p_x, p_y);
}

// Loads seven values from resource p_ref.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100707c0
MechS32 FUN_100707c0(
	ResourceRef* p_ref,
	MechS32* p_unk0x04,
	MechS32* p_unk0x08,
	MechS32* p_unk0x0c,
	MechS32* p_unk0x10,
	MechS32* p_unk0x14,
	MechS32* p_unk0x18,
	MechS32* p_unk0x1c
)
{
	MechS32 size;
	MechS32* data;
	MechS32* cursor;
	FILE* file;

	data = FUN_10073922(p_ref, g_unk0x100a86a8, g_unk0x100a8710, 5, &size, NULL);
	if (!data) {
		file = fopen("symlog.txt", "a");
		if (file) {
			fprintf(file, "Couldn't load ID=%s Type=%s\n", p_ref->m_name, g_unk0x100a86a8);
			fclose(file);
		}

		return FALSE;
	}

	cursor = data;
	*p_unk0x04 = *cursor;
	cursor++;
	*p_unk0x08 = *cursor;
	cursor++;
	*p_unk0x0c = *cursor;
	cursor++;
	*p_unk0x10 = *cursor;
	cursor++;
	*p_unk0x14 = *cursor;
	cursor++;
	*p_unk0x18 = *cursor;
	cursor++;
	*p_unk0x1c = *cursor;
	if (p_ref->m_id == -1) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
	}
	else {
		FUN_1001a163(p_ref->m_id, g_unk0x100a86a8);
	}

	return TRUE;
}

// Loads the animation file p_ref: up to 32 animations, numbered from the current base
// (FUN_10047462), into g_unk0x101079e0.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x100708f4
MechS32 FUN_100708f4(ResourceRef* p_ref)
{
	MechS32 ids[32];
	MechS32 size;
	MechS32 unk0x08;
	MechU8* frames;
	MechS32 base;
	MechS32 offset;
	MechS32 count;
	MechS32 frameCount;
	MechS32 i;
	MechU8* data;
	MechS32 stride;
	MechU8* end;
	MechS32 index;
	FILE* file;

	offset = 0;
	stride = sizeof(MechS32);
	data = FUN_10073922(p_ref, g_unk0x100a86a4, g_unk0x100a870c, 2, &size, &g_staticPoolTags[6]);
	if (!data) {
		file = fopen("symlog.txt", "a");
		if (file) {
			fprintf(file, "Couldn't load ID=%s Type=%s\n", p_ref->m_name, g_unk0x100a86a4);
		}

		fclose(file);
		return FALSE;
	}

	count = *(MechS32*) data;
	frameCount = *(MechS32*) (data + 4);
	offset = stride * 2;
	base = FUN_10047462();
	for (i = 0; i < count; i++) {
		index = *(MechS32*) (data + offset);
		offset += stride;
		unk0x08 = *(MechS32*) (data + offset);
		offset += stride;
		if (index >= 0x20) {
			return FALSE;
		}

		index += base;
		if (index >= 0x780) {
			return FALSE;
		}

		frames = data + offset;
		offset += frameCount * sizeof(MechS32);
		g_unk0x101079e0[index] = StaticPoolAlloc(sizeof(QuartzReel0x14), g_staticPoolTags[5]);
		if (!g_unk0x101079e0[index]) {
			return FALSE;
		}

		g_unk0x101079e0[index]->m_amounts = (MechS32*) frames;
		g_unk0x101079e0[index]->m_kind = unk0x08;
		g_unk0x101079e0[index]->m_frameCount = frameCount;
		if (p_ref->m_id == -1) {
			g_unk0x101079e0[index]->m_unk0x00 = 1;
		}
		else {
			g_unk0x101079e0[index]->m_unk0x00 = 0;
		}

		ids[i] = index;
	}

	end = data + offset;
	for (i = 0; i < count; i++) {
		g_unk0x101079e0[ids[i]]->m_events = (ReelEvent*) end;
	}

	return TRUE;
}

// Loads the cockpit layout resource p_ref: the gauge positions (g_unk0x100a5ee8), three values
// (g_unk0x10109c30) and fifteen rectangles in percent of the screen (g_unk0x100a5cf8), any of
// them out of range cleared.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10070bda
MechS32 FUN_10070bda(ResourceRef* p_ref)
{
	MechS32 size;
	MechS32 value;
	MechS32 i;
	MechS32* data;
	MechS32* cursor;
	MechS32 right;
	MechS32 bottom;
	MechS32 left;
	MechS32 top;
	FILE* file;

	data = FUN_10073922(p_ref, g_unk0x100a86ac, g_unk0x100a8714, 4, &size, NULL);
	if (!data) {
		file = fopen("symlog.txt", "a");
		if (file) {
			fprintf(file, "Couldn't load ID=%s Type=%s\n", p_ref->m_name, g_unk0x100a86ac);
		}

		fclose(file);
		return FALSE;
	}

	cursor = data;
	for (i = 0; i < 5; i++) {
		g_unk0x100a5ee8[i].m_x = *cursor;
		cursor++;
		g_unk0x100a5ee8[i].m_y = *cursor;
		cursor++;
	}

	for (i = 0; i < 3; i++) {
		value = *cursor;
		cursor++;
		g_unk0x10109c30[i] = value;
	}

	for (i = 0; i < 15; i++) {
		left = *cursor;
		cursor++;
		top = *cursor;
		cursor++;
		right = *cursor;
		cursor++;
		bottom = *cursor;
		cursor++;
		if (left < 0 || left > 100 || top < 0 || top > 100 || right < 0 || right > 100 || bottom < 0 || bottom > 100) {
			left = top = right = bottom = 0;
		}

		g_unk0x100a5cf8[i].m_left = left;
		g_unk0x100a5cf8[i].m_top = top;
		g_unk0x100a5cf8[i].m_right = right;
		g_unk0x100a5cf8[i].m_bottom = bottom;
	}

	if (p_ref->m_id == -1) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
	}
	else {
		FUN_1001a163(p_ref->m_id, g_unk0x100a86ac);
	}

	return TRUE;
}

// Loads the cockpit layout resource p_ref: five rectangles into p_gauges, fifteen into p_panels
// and a point.
// The only diff is a stack-slot permutation of the locals.
// FUNCTION: MW2 0x10070e22
MechS32 FUN_10070e22(ResourceRef* p_ref, RenderTarget* p_gauges, RenderTarget* p_panels, Point* p_point)
{
	MechS32 size;
	RenderTarget* target;
	MechS32 i;
	GarnetFrame0x8* frame;
	void* data;
	MechS16* value;
	FILE* file;

	if (!p_gauges || !p_panels || !p_point) {
		return FALSE;
	}

	data = FUN_10073922(p_ref, g_unk0x100a86b0, g_unk0x100a8718, 3, &size, NULL);
	if (!data) {
		file = fopen("symlog.txt", "a");
		if (file) {
			fprintf(file, "Couldn't load ID=%s Type=%s\n", p_ref->m_name, g_unk0x100a86b0);
		}

		fclose(file);
		return FALSE;
	}

	target = p_gauges;
	frame = data;
	for (i = 0; i < 5; i++) {
		target->m_left = frame->m_x;
		target->m_top = frame->m_y;
		target->m_right = frame->m_x + frame->m_width - 1;
		target->m_bottom = frame->m_y + frame->m_height - 1;
		frame++;
		target++;
	}

	target = p_panels;
	for (i = 0; i < 15; i++) {
		target->m_left = frame->m_x;
		target->m_top = frame->m_y;
		target->m_right = frame->m_x + frame->m_width - 1;
		target->m_bottom = frame->m_y + frame->m_height - 1;
		frame++;
		target++;
	}

	value = (MechS16*) frame;
	p_point->m_x = *value;
	value++;
	p_point->m_y = *value;
	value++;
	if (p_ref->m_id == -1) {
		HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, data);
	}
	else {
		FUN_1001a163(p_ref->m_id, g_unk0x100a86b0);
	}

	return TRUE;
}

// Writes the screen as a picture: a 0x20-byte header from TABL resource 15, the palette, then
// the main pixel buffer. Returns whether it could.
// Stack-slot permutation: header, file and pixels.
// FUNCTION: MW2 0x10071026
MechS32 FUN_10071026(MechChar* p_path, void* p_palette)
{
	void* header;
	MechS32 file;
	undefined* pixels;

	header = NULL;
	file = open(p_path, _O_BINARY | _O_CREAT | _O_WRONLY, c_permissionWrite);
	if (file == -1) {
		return FALSE;
	}

	header = FUN_1001a19f(g_unk0x100a8740, 15, g_unk0x100a8698, 1);
	if (header == NULL) {
		close(file);
		return FALSE;
	}

	write(file, header, 0x20);
	write(file, p_palette, 0x300);
	pixels = g_mainPixelBuffer.m_pixels;
	write(file, pixels, g_screenPixelCount);
	close(file);
	FUN_1001a163(15, g_unk0x100a8698);
	return TRUE;
}

// Reads a whole file into memory from the heap, or from a static pool when p_poolTag is given.
// Returns the open file, or -1 (logging the path to symlog.txt when it can't be opened).
// Stack-slot permutation: log and file.
// FUNCTION: MW2 0x10071108
MechS32 LoadFile(MechChar* p_path, MechS32* p_size, void** p_data, MechU32* p_poolTag)
{
	FILE* log;
	MechS32 file;

	file = open(p_path, _O_BINARY);
	if (file == -1) {
		log = fopen("symlog.txt", "a");
		if (log) {
			fprintf(log, "Couldn't load ID=%s\n", p_path);
		}
		fclose(log);
		return -1;
	}

	*p_size = filelength(file);
	if (p_poolTag == NULL) {
		*p_data = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, *p_size);
	}
	else {
		*p_data = StaticPoolAlloc(*p_size, *p_poolTag);
	}

	if (*p_data == NULL) {
		close(file);
		return -1;
	}

	if (read(file, *p_data, *p_size) != *p_size) {
		if (p_poolTag == NULL) {
			HeapFree(g_primaryHeap, HEAP_NO_SERIALIZE, *p_data);
		}
		close(file);
		return -1;
	}

	return file;
}

// Reads a game file into memory. Returns the file (closed), or -1.
// Stack-slot permutation: file and data.
// FUNCTION: MW2 0x10071251
MechS32 FUN_10071251(MechChar* p_name, void** p_data)
{
	MechS32 size;
	MechS32 file;
	void* data;

	*p_data = NULL;
	file = LoadFile(BuildGamePath(p_name), &size, &data, NULL);
	if (file != -1) {
		close(file);
		*p_data = data;
	}

	return file;
}

// FUNCTION: MW2 0x100712b0
MechS32 FUN_100712b0(MechChar* p_name, void* p_data)
{
	MechS32 file;
	MechS32 result;

	file = open(BuildGamePath(p_name), _O_BINARY | _O_CREAT | _O_WRONLY, c_permissionWrite);
	if (file != -1) {
		write(file, p_data, 0xd6);
		close(file);
		result = 0;
	}
	else {
		result = -1;
	}

	return result;
}

// Reads the difficulty settings into a new block, then overrides some of them in network games.
// Returns 1, or -1 if there is no block.
// Stack-slot permutation: file and data.
// FUNCTION: MW2 0x10071326
MechS32 LoadDifficultyCfg(MechChar* p_name, DifficultyCfg** p_cfg)
{
	MechS32 size;
	MechS32 file;
	void* data;

	*p_cfg = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(DifficultyCfg));
	file = LoadFile(BuildGamePath(p_name), &size, &data, NULL);
	if (file != -1) {
		memcpy(*p_cfg, data, size);
	}
	else if (*p_cfg == NULL) {
		return -1;
	}

	close(file);
	if (g_isNetworkGame) {
		(*p_cfg)->m_unk0x05 = 2;
		(*p_cfg)->m_invulnerable = 0;
	}
	else {
		(*p_cfg)->m_unk0x0f = 0;
		(*p_cfg)->m_unk0x0b = 0;
		(*p_cfg)->m_unk0x13 = 0;
		(*p_cfg)->m_unk0x09 = 1;
	}

	if (g_isNetworkGame > 1) {
		(*p_cfg)->m_heatTracking = 1;
		(*p_cfg)->m_unlimitedAmmo = 0;
		(*p_cfg)->m_splashDamage = 1;
		(*p_cfg)->m_collisionDamage = 1;
	}

	return 1;
}

// FUNCTION: MW2 0x10071440
MechS32 SaveDifficultyCfg(MechChar* p_name, DifficultyCfg* p_cfg)
{
	MechS32 file;
	MechS32 result;

	file = open(BuildGamePath(p_name), _O_BINARY | _O_CREAT | _O_WRONLY, c_permissionWrite);
	if (file != -1) {
		write(file, p_cfg, sizeof(DifficultyCfg));
		close(file);
		result = 0;
	}
	else {
		result = -1;
	}

	return result;
}

// Reads the sound settings, or allocates cleared ones. Returns 1, or -1 if it couldn't read them.
// Stack-slot permutation: file and data.
// FUNCTION: MW2 0x100714b3
MechS32 LoadSndCfg(MechChar* p_name, SoundConfig** p_cfg)
{
	MechS32 size;
	MechS32 file;
	void* data;

	file = LoadFile(BuildGamePath(p_name), &size, &data, NULL);
	if (file != -1) {
		*p_cfg = data;
	}
	else {
		*p_cfg = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(SoundConfig));
		return -1;
	}

	close(file);
	return 1;
}

// FUNCTION: MW2 0x1007152f
MechS32 SaveSndCfg(MechChar* p_name, SoundConfig* p_cfg)
{
	MechS32 file;
	MechS32 result;

	file = open(BuildGamePath(p_name), _O_BINARY | _O_CREAT | _O_WRONLY, c_permissionWrite);
	if (file != -1) {
		write(file, p_cfg, sizeof(SoundConfig));
		close(file);
		result = 0;
	}
	else {
		result = -1;
	}

	return result;
}

// Saves the screen as the next of mw2NNNN.gif, up to 1000 of them.
// FUNCTION: MW2 0x100715a2
void FUN_100715a2(void)
{
	MechS32 count;
	RenderTarget target;
	MechChar name[16];

	target.m_buffer = &g_mainPixelBuffer;
	target.m_left = 0;
	target.m_top = 0;
	target.m_right = g_screenWidthMinus1;
	target.m_bottom = g_screenHeightMinus1;
	if (g_screenshotCount < 1000) {
		count = g_screenshotCount++;
		sprintf(name, "mw2%04d.gif", count);
		ScreenshotBegin(name);
		ScreenshotWritePalette();
		ScreenshotWriteImage(&target);
		ScreenshotEnd();
	}
}

// Returns the path of a game file: in g_gameDir unless the name has a directory already.
// FUNCTION: MW2 0x1007162a
MechChar* BuildGamePath(MechChar* p_name)
{
	MechS32 i;

	for (i = 0; i < 0x50; i++) {
		g_gamePath[i] = 0;
	}

	if (g_gameDir[0] && !strchr(p_name, '\\') && !strchr(p_name, '/')) {
		sprintf(g_gamePath, "%s\\%s", g_gameDir, p_name);
	}
	else {
		strcpy(g_gamePath, p_name);
	}

	return g_gamePath;
}

// Returns the path of a file in g_gameDir.
// FUNCTION: MW2 0x100716ec
MechChar* FUN_100716ec(MechChar* p_name)
{
	MechS32 i;

	for (i = 0; i < 0x50; i++) {
		g_gamePath[i] = 0;
	}

	if (g_gameDir[0]) {
		sprintf(g_gamePath, "%s\\%s", g_gameDir, p_name);
	}
	else {
		strcpy(g_gamePath, p_name);
	}

	return g_gamePath;
}
