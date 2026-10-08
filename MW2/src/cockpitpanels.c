/* The cockpit panels: their layout, set-up, per-frame update and warnings. Not a unit of its own:
   1.1 has these in config.c's object, ahead of its other functions, and the Matrox edition at the
   end of staticmem.c's, so config.c includes this file in 1.1's build and staticmem.c in the
   Matrox edition's. */
#include "ammobin.h"
#include "anim2d.h"
#include "approxlen.h"
#include "bargauges.h"
#include "cockpit.h"
#include "cockpitframe.h"
#include "cockpitpanel.h"
#include "config.h"
#include "damagepanel.h"
#include "decomp.h"
#include "environment.h"
#include "eyepoint.h"
#include "fadepal.h"
#include "fixedfloat.h"
#include "gamekeys.h"
#include "hud.h"
#include "loadres.h"
#include "mech.h"
#include "mechdamage.h"
#include "mechviewpanel.h"
#include "midi.h"
#include "mw2prj.h"
#include "network.h"
#include "objectanim.h"
#include "palette.h"
#include "players.h"
#include "point.h"
#include "poolsizes.h"
#include "random.h"
#include "recttransition.h"
#include "reel.h"
#include "render.h"
#include "resource.h"
#include "resourcename.h"
#include "resourceref.h"
#include "screenscale.h"
#include "screenshot.h"
#include "simmain.h"
#include "soundconfig.h"
#include "soundfx.h"
#include "speech.h"
#include "staticmem.h"
#include "statuspanels.h"
#include "targeting.h"
#include "targetpanel.h"
#include "types.h"
#include "weapondata.h"
#include "weaponpanel.h"
#include "weapons.h"
#include "weaponslot.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

// The 26 cockpit panels' rectangles, in 320x200 screen coordinates (ScaleCockpitLayout scales them to the
// screen).
// GLOBAL: MW2 0x100adf58
// GLOBAL: MW2MATROX 0x100b1478
PANE g_cockpitPanelPanes[c_panelCount] = {
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
// GLOBAL: MW2MATROX 0x100b1680
Point g_cockpitPanelTextOrigins[c_panelCount] = {{0, 0},           {0, 0},
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
// GLOBAL: MW2MATROX 0x100b1750
RectTransitionState g_targetTransitionState = {0, 0, 0};

// GLOBAL: MW2 0x100ae240
// GLOBAL: MW2MATROX 0x100b1760
RectTransitionState g_mechViewTransitionState = {0, 0, 0};

// GLOBAL: MW2 0x100ae250
// GLOBAL: MW2MATROX 0x100b1770
PANE g_targetTransitionFirst = {NULL, 0x8000, 0x8000, 0x8000, 0x8000};

// GLOBAL: MW2 0x100ae268
// GLOBAL: MW2MATROX 0x100b1788
PANE g_targetTransitionSecond = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100ae280
// GLOBAL: MW2MATROX 0x100b17a0
PANE g_targetTransitionRect = {NULL, 0, 0, 0, 0};

// GLOBAL: MW2 0x100ae298
// GLOBAL: MW2MATROX 0x100b17b8
RectTransitionDef g_targetTransitionDef =
	{0xb5, &g_targetTransitionFirst, &g_targetTransitionSecond, &g_targetTransitionRect};

// GLOBAL: MW2 0x100ae2a8
// GLOBAL: MW2MATROX 0x100b17c8
RectTransition g_targetTransition = {&g_targetTransitionState, &g_targetTransitionDef};

// GLOBAL: MW2 0x100ae2b0
// GLOBAL: MW2MATROX 0x100b17d0
PANE g_mechViewTransitionFirst = {NULL, 0x8000, 0x8000, 0x8000, 0x8000};

// GLOBAL: MW2 0x100ae2c8
// GLOBAL: MW2MATROX 0x100b17e8
PANE g_mechViewTransitionSecond = {NULL, 0, 0, 0x10000, 0x10000};

// GLOBAL: MW2 0x100ae2e0
// GLOBAL: MW2MATROX 0x100b1800
PANE g_mechViewTransitionRect = {NULL, 0, 0, 0, 0};

// GLOBAL: MW2 0x100ae2f8
// GLOBAL: MW2MATROX 0x100b1818
RectTransitionDef g_mechViewTransitionDef =
	{0xb5, &g_mechViewTransitionFirst, &g_mechViewTransitionSecond, &g_mechViewTransitionRect};

// GLOBAL: MW2 0x100ae308
// GLOBAL: MW2MATROX 0x100b1828
RectTransition g_mechViewTransition = {&g_mechViewTransitionState, &g_mechViewTransitionDef};

// The panels' transitions.
// GLOBAL: MW2 0x100ae310
// GLOBAL: MW2MATROX 0x100b1830
RectTransition* g_cockpitPanelTransitions[c_panelCount] = {
	NULL,
	NULL,
	&g_mechViewTransition,
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
	&g_targetTransition,
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

// Set when UpdateCockpit should run PunchInAutoHeading on its next frame.
// GLOBAL: MW2 0x100ae37c
// GLOBAL: MW2MATROX 0x100b189c
MechS32 g_punchInAutoHeadingRequested = 0;

// GLOBAL: MW2 0x100ae380
// GLOBAL: MW2MATROX 0x100b18a0
MechS32 g_hitFadePending = 0;

// The clock times the panels light up at on startup (CockpitPanel::m_lightUpTime): the weapon panels in turn.
// GLOBAL: MW2 0x100ae388
// GLOBAL: MW2MATROX 0x100b18a8
undefined4 g_cockpitPanelLightUpTimes[c_panelCount] = {0x16a, 0xb5,  0xb5,  0x21f, 0x23d, 0x25b, 0x279, 0x297, 0x32e,
													   0x310, 0x2f2, 0x2d4, 0x2b5, 0xb5,  0xb5,  0xb5,  0xb5,  0xb5,
													   0xb5,  0xb5,  0xb5,  0xb5,  0xb5,  0x0,   0x0,   0x0};

// The local mech's state (Mech::m_powerState) when PlayCockpitWarnings last ran.
// GLOBAL: MW2 0x100ae3f0
// GLOBAL: MW2MATROX 0x100b1910
MechS32 g_lastWarningPowerState = 0;

// GLOBAL: MW2 0x100ae3f4
// GLOBAL: MW2MATROX 0x100b1914
MechS32 g_lockedTonePlayed = 0;

// GLOBAL: MW2 0x100ae3f8
// GLOBAL: MW2MATROX 0x100b1918
MechS32 g_lockingTonePlayed = 0;

// GLOBAL: MW2 0x100ae3fc
// GLOBAL: MW2MATROX 0x100b191c
MechS32 g_hitFadeCount = 0;

// The local player's heading and torso twist, in whole degrees (UpdateCockpit).

// GLOBAL: MW2 0x100c326c
// GLOBAL: MW2MATROX 0x1012f964
MechS32 g_torsoTwistDegrees;

// GLOBAL: MW2 0x100c3270
// GLOBAL: MW2MATROX 0x1012f960
MechS32 g_headingDegrees;

// The 26 cockpit panels InitCockpitPanels allocates.
// GLOBAL: MW2 0x100c3280
// GLOBAL: MW2MATROX 0x1012f970
CockpitPanel* g_cockpitPanels[c_panelCount];

// Which of the panels are enabled when they are set up.
// GLOBAL: MW2 0x100c32f0
// GLOBAL: MW2MATROX 0x1012f9e0
MechS32 g_cockpitPanelEnabled[c_panelCount];

// GLOBAL: MW2 0x100c3358
// GLOBAL: MW2MATROX 0x1012f9d8
MechS32 g_cockpitPowerState;

// Loads eight sounds ahead of their use.
// FUNCTION: MW2 0x1006f480
// FUNCTION: MW2MATROX 0x100794d0
void PreloadCockpitSounds(void)
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
		PreloadResource(ids[i], g_resourceTypeTags[c_resTagSnds]);
	}
}

// Lays out p_mech's weapons on the weapon panels: the left panels (3 to 7) take the weapons on
// hardpoints 5, 3 and 7, the right ones (8 to 12) those on 4, 1 and 6, and the weapons on 2 and 0
// fill the rest, alternating. Then reorders the weapons to match, left at even indices and right
// at odd ones, renumbering their bins, and clears the slots left empty.
// The locals are a stack-slot permutation, and right > left takes its operands in the other
// order.
// FUNCTION: MW2 0x1006f4fa
// FUNCTION: MW2MATROX 0x1007954a
void LayoutWeaponPanels(Mech* p_mech)
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
	AmmoBin* bin;
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
			g_cockpitPanels[left]->m_setName(g_cockpitPanels[left], g_weaponDefs[slot->m_type].m_name);
			g_cockpitPanels[left]->m_setWeapon(g_cockpitPanels[left], i);
			g_cockpitPanels[left]->m_drawStartup = DrawWeaponPanelStartup;
			g_cockpitPanels[left]->m_draw = DrawWeaponPanel;
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
			g_cockpitPanels[right]->m_setName(g_cockpitPanels[right], g_weaponDefs[slot->m_type].m_name);
			g_cockpitPanels[right]->m_setWeapon(g_cockpitPanels[right], i);
			g_cockpitPanels[right]->m_drawStartup = DrawWeaponPanelStartup;
			g_cockpitPanels[right]->m_draw = DrawWeaponPanel;
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

			g_cockpitPanels[panel]->m_setName(g_cockpitPanels[panel], g_weaponDefs[slot->m_type].m_name);
			g_cockpitPanels[panel]->m_setWeapon(g_cockpitPanels[panel], i);
			g_cockpitPanels[panel]->m_drawStartup = DrawWeaponPanelStartup;
			g_cockpitPanels[panel]->m_draw = DrawWeaponPanel;
			panel++;
		}

		if (i != 10) {
			slot++;
		}
	}

	weapons = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, 10 * sizeof(WeaponSlot));
	dst = weapons;
	for (i = 3; i <= 7; i++) {
		if (g_cockpitPanels[i]->m_weapon != -1) {
			slot = &p_mech->m_weapons[g_cockpitPanels[i]->m_weapon];
			*dst = *slot;
			if (i != 7) {
				dst += 2;
			}
		}
	}

	dst = weapons + 1;
	for (i = 8; i <= 12; i++) {
		if (g_cockpitPanels[i]->m_weapon != -1) {
			slot = &p_mech->m_weapons[g_cockpitPanels[i]->m_weapon];
			*dst = *slot;
			if (i != 12) {
				dst += 2;
			}
		}
	}

	index = 0;
	for (i = 3; i <= 7; i++) {
		if (g_cockpitPanels[i]->m_weapon != -1) {
			slot = &p_mech->m_weapons[g_cockpitPanels[i]->m_weapon];
			for (j = 0; j < slot->m_binCount; j++) {
				bin = &((AmmoBin*) p_mech->m_ammoBins)[slot->m_bins[j]];
				bin->m_weapon = index;
			}
		}

		g_cockpitPanels[i]->m_weapon = index;
		index += 2;
	}

	index = 1;
	for (i = 8; i <= 12; i++) {
		if (g_cockpitPanels[i]->m_weapon != -1) {
			slot = &p_mech->m_weapons[g_cockpitPanels[i]->m_weapon];
			for (j = 0; j < slot->m_binCount; j++) {
				bin = &((AmmoBin*) p_mech->m_ammoBins)[slot->m_bins[j]];
				bin->m_weapon = index;
			}

			g_cockpitPanels[i]->m_weapon = index;
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
// FUNCTION: MW2MATROX 0x10079bfb
void ScaleCockpitLayout(void)
{
	PANE* rect;
	RectTransition* transition;
	MechS32 i;
	PANE* target;

	for (i = 0; i < c_panelCount; i++) {
		target = &g_cockpitPanelPanes[i];
		ScaleRectFromLowRes(target, target);
		ScaleRectToScreen(&g_mainPixelBuffer, target, target);
		ScalePointToFrame(target, &g_cockpitPanelTextOrigins[i], &g_cockpitPanelTextOrigins[i]);
		transition = g_cockpitPanelTransitions[i];
		if (transition) {
			rect = transition->m_def->m_first;
			rect->m_window = &g_mainPixelBuffer;
			ScaleRectToFrame(target, rect, rect);
			rect = transition->m_def->m_second;
			rect->m_window = &g_mainPixelBuffer;
			ScaleRectToFrame(target, rect, rect);
			rect = transition->m_def->m_out;
			rect->m_window = &g_mainPixelBuffer;
		}
	}
}

// Creates the 26 cockpit panels, all enabled but the second and (in network games) the
// fourteenth, lays out the local mech's weapon panels (LayoutWeaponPanels) and installs the panels'
// handlers.
// FUNCTION: MW2 0x1006fca5
// FUNCTION: MW2MATROX 0x10079cfd
void InitCockpitPanels(void)
{
	Mech* mech;
	MechS32 i;

	for (i = 0; i < c_panelCount; i++) {
		g_cockpitPanelEnabled[i] = TRUE;
	}

	g_cockpitPanelEnabled[1] = FALSE;
	if (!g_difficulty->m_radar) {
		g_cockpitPanelEnabled[c_panelTarget] = FALSE;
	}

	mech = g_players[g_localPlayerId]->m_mech;
	for (i = 0; i < c_panelCount; i++) {
		g_cockpitPanels[i] = HeapAlloc(g_primaryHeap, HEAP_NO_SERIALIZE, sizeof(CockpitPanel));
		InitCockpitPanel(g_cockpitPanels[i]);
		g_cockpitPanels[i]->m_setLightUpTime(g_cockpitPanels[i], g_cockpitPanelLightUpTimes[i]);
		g_cockpitPanels[i]->m_setTarget(g_cockpitPanels[i], &g_cockpitPanelPanes[i]);
		g_cockpitPanels[i]->m_setTextOrigin(g_cockpitPanels[i], &g_cockpitPanelTextOrigins[i]);
		g_cockpitPanels[i]->m_setTransition(g_cockpitPanels[i], g_cockpitPanelTransitions[i]);
		if (g_cockpitPanelEnabled[i]) {
			g_cockpitPanels[i]->m_enable(g_cockpitPanels[i]);
		}
		else {
			g_cockpitPanels[i]->m_disable(g_cockpitPanels[i]);
		}
	}

	LayoutWeaponPanels(mech);
	g_cockpitPanels[c_panelMechView]->m_draw = DrawMechViewPanel;
	g_cockpitPanels[c_panelMechView]->m_drawStatic = DrawMechViewStatic;
	g_cockpitPanels[c_panelMechView]->m_drawStartup = DrawMechViewStartup;
	g_cockpitPanels[c_panelMechView]->m_drawShutdown = DrawMechViewShutdown;
	g_cockpitPanels[c_panelTarget]->m_draw = DrawTargetPanel;
	g_cockpitPanels[c_panelTarget]->m_drawStatic = DrawTargetStatic;
	g_cockpitPanels[c_panelTargetText]->m_draw = DrawTargetPanelText;
	g_cockpitPanels[c_panelTarget]->m_drawStartup = DrawTargetPanelStartup;
	g_cockpitPanels[c_panelTarget]->m_drawShutdown = DrawTargetPanelShutdown;
	g_cockpitPanels[c_panelObjectives]->m_draw = DrawObjectivesPanel;
	g_cockpitPanels[c_panelNetwork]->m_draw = DrawNetworkPanel;
	g_cockpitPanels[c_panelAutopilot]->m_draw = DrawAutopilotPanel;
	g_cockpitPanels[c_panelSpeed]->m_draw = DrawSpeedPanel;
	g_cockpitPanels[c_panelMasc]->m_draw = DrawMascPanel;
	g_cockpitPanels[c_panelHeat]->m_draw = DrawHeatPanel;
	g_cockpitPanels[c_panelHeatRate]->m_draw = DrawHeatRatePanel;
	g_cockpitPanels[c_panelJets]->m_draw = DrawJetsPanel;
	g_cockpitPanels[c_panelKills]->m_draw = DrawKillsPanel;
	ScaleBarGauges();
	InitDamagePanel();
	g_eyeHeightOffset = &mech->m_cockpitHeight;
	g_eyeTwist = &mech->m_torsoTwist.m_value;
	InitHudGauges();
	if (g_difficulty->m_radar) {
		InitCockpitViews();
	}
}

// Lays out the local mech's weapon panels again and resets every panel to its settings.
// FUNCTION: MW2 0x1006ff7b
// FUNCTION: MW2MATROX 0x10079fd3
void ResetCockpitPanels(void)
{
	Mech* mech;
	MechS32 i;

	mech = g_players[g_localPlayerId]->m_mech;
	LayoutWeaponPanels(mech);
	for (i = 0; i < c_panelCount; i++) {
		g_cockpitPanels[i]->m_setLightUpTime(g_cockpitPanels[i], g_cockpitPanelLightUpTimes[i]);
		g_cockpitPanels[i]->m_setTransition(g_cockpitPanels[i], g_cockpitPanelTransitions[i]);
		g_cockpitPanels[i]->m_setDamage(g_cockpitPanels[i], 0);
		if (g_cockpitPanelEnabled[i]) {
			g_cockpitPanels[i]->m_enable(g_cockpitPanels[i]);
		}
	}
}

// Updates the cockpit for the local mech's frame: the heading and the torso twist the panels show,
// the target's bearing relative to both and its distance, a one-time warning sound, and the
// panels' handlers for the view mode (Mech::m_powerState: 1, 2 or the rest).
// Stack-slot permutation: pitch, twistBearing, i, distance and bearing.
// FUNCTION: MW2 0x1007005a
void UpdateCockpit(Mech* p_mech)
{
	MechS32 pitch;
	MechS32 twistBearing;
	MechS32 i;
	MechS32 distance;
	MechS32 bearing;

	if (p_mech->m_player->m_index != g_localPlayerId) {
		return;
	}

	g_cockpitPowerState = p_mech->m_powerState;
	if (g_cockpitPowerState != 2 && g_cockpitPanels[c_panelNetwork]->m_draw) {
		g_cockpitPanels[c_panelNetwork]->m_draw(g_cockpitPanels[c_panelNetwork]);
	}

	if (g_cockpitPowerState == 4 || g_cockpitPowerState == 5) {
		return;
	}

	if (!g_showHud) {
		return;
	}

	g_headingDegrees = (FIXED_TO_INT(p_mech->m_player->m_heading) % 360 % 360 + 360) % 360;
	g_torsoTwistDegrees = FIXED_TO_INT(p_mech->m_torsoTwist.m_value) % 360 % 360;
	pitch = FIXED_MOD360(p_mech->m_player->m_targetInfo.m_pitch + p_mech->m_torsoPitch.m_value);
	bearing = FIXED_TO_INT(p_mech->m_player->m_targetInfo.m_heading) % 360 - g_headingDegrees;
	if (bearing > 180) {
		bearing -= 360;
	}
	else if (bearing < -180) {
		bearing += 360;
	}

	twistBearing = bearing - g_torsoTwistDegrees;
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
	if (g_punchInAutoHeadingRequested) {
		if (g_cockpitPowerState == 2) {
			PunchInAutoHeading(p_mech);
		}

		g_punchInAutoHeadingRequested = 0;
	}

	if (g_overrideShutdown && g_cockpitPowerState != 3 && (p_mech->m_flags & 4) && !(p_mech->m_flags & 8)) {
		PlaySoundEffect(0xcd, 100, 0x40, 5, 0x32);
		PlayCockpitSound(2, -1);
		p_mech->m_flags |= 8;
		g_overrideShutdown = 0;
	}
	else {
		g_overrideShutdown = 0;
	}

	PlayCockpitWarnings(p_mech);
	switch (g_cockpitPowerState) {
	case 2:
		if (g_difficulty->m_radar) {
			RunMapView();
		}

		for (i = 0; i < c_panelCount; i++) {
			if (g_cockpitPanels[i]->m_draw) {
				g_cockpitPanels[i]->m_draw(g_cockpitPanels[i]);
			}
		}

		DrawHud(
			p_mech,
			g_headingDegrees,
			g_torsoTwistDegrees,
			bearing,
			twistBearing,
			pitch,
			distance,
			g_cockpitEyeSteady
		);
		UpdateEngineNote(p_mech->m_throttle.m_value);
		break;
	case 1:
		if (g_difficulty->m_radar) {
			PowerUpMapView();
		}

		for (i = 0; i < c_panelCount; i++) {
			if (g_cockpitPanels[i]->m_drawStartup) {
				g_cockpitPanels[i]->m_drawStartup(g_cockpitPanels[i]);
			}
		}

		StartEngineNote();
		break;
	default:
		if (g_difficulty->m_radar) {
			PowerDownMapView();
		}

		for (i = 0; i < c_panelCount; i++) {
			if (g_cockpitPanels[i]->m_drawShutdown) {
				g_cockpitPanels[i]->m_drawShutdown(g_cockpitPanels[i]);
			}
		}

		StopEngineNote();
		break;
	}

	if (!g_inCockpitView) {
		MuteEngineNote();
	}
}

// Shuts the cockpit panels down: ResetMapView with the radar on (DifficultyCfg::m_radar),
// each panel's m_shutdown hook, then every 2D animation.
// FUNCTION: MW2 0x100704c1
// FUNCTION: MW2MATROX 0x1007a53c
void ShutdownCockpitPanels(void)
{
	MechS32 i;

	if (g_difficulty->m_radar) {
		ResetMapView();
	}

	for (i = 0; i < c_panelCount; i++) {
		if (g_cockpitPanels[i]->m_shutdown) {
			g_cockpitPanels[i]->m_shutdown(g_cockpitPanels[i]);
		}
	}

	FreeAnim2ds(-1);
}

// Knocks the cockpit panels about when the local player's mech is hit: each panel has a two
// (p_heavy: five) in ten chance of stepping its damage.
// FUNCTION: MW2 0x1007053d
// FUNCTION: MW2MATROX 0x1007a5b8
void DamageCockpitPanels(Mech* p_mech, MechS32 p_heavy)
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

	for (i = 0; i < c_panelCount; i++) {
		if (RandomIntBelow(10) < chance) {
			g_cockpitPanels[i]->m_setDamage(g_cockpitPanels[i], g_cockpitPanels[i]->m_damage + 1);
		}
	}
}

// Plays the cockpit's warning sounds for the local mech as its state and flags change.
// FUNCTION: MW2 0x100705dd
// FUNCTION: MW2MATROX 0x1007a658
void PlayCockpitWarnings(Mech* p_mech)
{
	if (g_hitFadePending) {
		g_hitFadeCount++;
		FlashZappedPaletteLevel(g_hitFadeCount * 3);
		g_hitFadePending = 0;
	}

	if (g_inCockpitView) {
		if (p_mech->m_flags & 0x80) {
			if (!g_lockedTonePlayed && p_mech->m_weapons[p_mech->m_selectedWeapon].m_state == 1 &&
				p_mech->m_powerState == 2) {
				g_lockedTonePlayed = 1;
				PlaySoundEffect(0xfe, 100, 0x5f, 5, 0x32);
			}
		}
		else {
			g_lockedTonePlayed = 0;
			if (p_mech->m_flags & 0x40) {
				if (!g_lockingTonePlayed && p_mech->m_powerState == 2) {
					g_lockingTonePlayed = 1;
					PlaySoundEffect(0xcf, 100, 0x1f, 5, 0x32);
				}
			}
			else {
				g_lockingTonePlayed = 0;
			}
		}

		if (p_mech->m_powerState != g_lastWarningPowerState) {
			switch (p_mech->m_powerState) {
			case 2:
				PlaySoundEffect(0xce, 100, 0x2f, 5, 0x32);
				break;
			case 0:
			case 4:
				PlaySoundEffect(0xf6, 100, 0x40, 5, 0x32);
				break;
			default:
				break;
			}
		}
	}

	g_lastWarningPowerState = p_mech->m_powerState;
}

// FUNCTION: MW2 0x1007079d
// FUNCTION: MW2MATROX 0x1007a818
void DrawPanelAnim(PANE* p_target, MechS32 p_index, MechS32 p_x, MechS32 p_y)
{
	DrawAnim2d(p_target, p_index, p_x, p_y);
#ifdef MW2_MATROX
	FUN_10088280(p_target);
#endif
}
